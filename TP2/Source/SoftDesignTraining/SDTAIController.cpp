// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    bool showDebug = false;
    
    UWorld* world = GetWorld();
    if (!world) // Check if world is null
    {
        return;
    }

    TArray<AActor*> collectibles;
    UGameplayStatics::GetAllActorsOfClass(world, ASDTCollectible::StaticClass(), collectibles);
    APawn* pawn = GetPawn();
    if (!pawn) // Check if pawn is null
    {
        return;
    }

    FVector pathStart= pawn->GetActorLocation();

    float bestPathLenght = INFINITY;
    AActor* bestCollectible = nullptr;

    for (int i=0; i < collectibles.Num();i++)
    {
        ASDTCollectible* collectible = Cast<ASDTCollectible>(collectibles[i]);
        if (!collectible || collectible->IsOnCooldown()) // Check if collectible is null or on cooldown
        {
            continue;
        }

        FVector pathEnd = collectibles[i]->GetActorLocation();
        UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(world, pathStart, pathEnd, pawn);
        if (!path) // Check if path is null
        {
            continue;
        }

        float pathLenght = path->GetPathLength();
        if (bestCollectible == nullptr || pathLenght < bestPathLenght)
        {
            bestPathLenght = pathLenght;
            bestCollectible = collectibles[i];
        }
    }

    if (bestCollectible == nullptr)
    {
        return;
    }

    MoveToActor(bestCollectible);
    OnMoveToTarget();

    if (showDebug)
    {
        DrawDebugSphere(world, bestCollectible->GetActorLocation(), 50, 50, FColor::Red, false);
    }
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    // Show current navigation path DrawDebugLine and DrawDebugSphere
    // Use the UPathFollowingComponent of the AIController to get the path
    // This function is called while m_ReachedTarget is false 
    // Check void ASDTBaseAIController::Tick for how it works.
    
    USDTPathFollowingComponent* pathFollowingComponent = Cast<USDTPathFollowingComponent>(GetPathFollowingComponent());
    // Récupération du path
    FNavPathSharedPtr path = pathFollowingComponent->GetPath();
    if (path == nullptr)
    {
        return;
    }
    // Récupération des points du path
    const TArray<FNavPathPoint>& points = path->GetPathPoints();
    // Affichage des points du path
    for (int i = 0; i < points.Num(); i++)
    {
        if (i != points.Num() - 1)
        {
            DrawDebugLine(GetWorld(), points[i].Location, points[i + 1].Location, FColor::Green, false);
        }
        DrawDebugPoint(GetWorld(), points[i].Location, 10, FColor::Red, false);
    }
    
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    // Set behavior based on hit
    // This is the place where you decide to switch your path towards a new path
    // Check ASDTAIController::AIStateInterrupted to stop your current path

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}