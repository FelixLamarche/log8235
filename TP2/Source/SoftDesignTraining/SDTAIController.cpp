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
    // Move to target depending on current behavior
    // This function is called while m_ReachedTarget is true.
    // Check void ASDTBaseAIController::Tick for how it works.
    
    UWorld* world = GetWorld();

    // Recherche du collectible le plus proche (en terme de distance de parcours)
    TArray<AActor*> collectibles;
    UGameplayStatics::GetAllActorsOfClass(world, ASDTCollectible::StaticClass(), collectibles);
    FVector pathStart= GetPawn()->GetActorLocation();

    float bestPathLenght = INFINITY;
    AActor* bestCollectible = nullptr;

    for (int i=0; i < collectibles.Num();i++) //iterate through collectibles and find the closest one
    {
        // Si le collectible est en cooldown, on ne le prend pas en compte
        if (Cast<ASDTCollectible>(collectibles[i])->IsOnCooldown())
        {
            continue;
        }

        // Calcul de la distance de parcours
        FVector pathEnd = collectibles[i]->GetActorLocation();
        float pathLenght = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(),pathStart,pathEnd,GetPawn())->GetPathLength();
        // Si le collectible est plus proche que le meilleur collectible trouvé jusqu'à présent, on le prend en compte
        if (bestCollectible == nullptr || pathLenght < bestPathLenght)
        {
         bestPathLenght = pathLenght;
         bestCollectible = collectibles[i];
        }
	}

    // Si aucun collectible n'est trouvé, on return sans rien faire
    if (bestCollectible == nullptr)
    {
        return;
    }
    // Sinon on va vers ce collectible
    MoveToActor(bestCollectible);
    OnMoveToTarget();

    // Debug
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