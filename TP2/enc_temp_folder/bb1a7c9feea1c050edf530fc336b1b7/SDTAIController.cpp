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
    // Move to target depending on current behavior
    // This function is called while m_ReachedTarget is true.
    // Check void ASDTBaseAIController::Tick for how it works.
    UWorld* world = GetWorld();
    TArray<AActor*> collectibles;
    UGameplayStatics::GetAllActorsOfClass(world, ASDTCollectible::StaticClass(), collectibles);
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Collectibles: %d"), collectibles.Num()));
    FVector pathStart= GetPawn()->GetActorLocation();
    FVector pathEnd = FVector(INFINITY,INFINITY,INFINITY);
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(world);
    UNavigationPath* path = nullptr;
    UNavigationPath* bestPath = nullptr;
    AActor* bestCollectible = nullptr;


    for (int i=0; i < collectibles.Num();i++) //iterate through collectibles and find the closest one
    {
        if (Cast<ASDTCollectible>(collectibles[i])->IsOnCooldown())
        {
            continue;
        }
        
         pathEnd = collectibles[i]->GetActorLocation();
        
         path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(),pathStart,pathEnd,GetPawn());
         if (path->GetPathLength() < bestPath->GetPathLength())
         {
			 bestPath = path;
             bestCollectible = collectibles[i];
		 }

         
       // if (FVector::Dist(pathStart, collectibles[i]->GetActorLocation()) < FVector::Dist(pathStart, pathEnd))
      //  {
		//}
	}
    DrawDebugSphere(world, bestCollectible->GetActorLocation(), 50, 50, FColor::Red, false, 5.f);



    GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::Red, FString::Printf(TEXT("Path: %d"), path->PathPoints.Num()));
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
    // auto path = UPathFollowingComponent::GetPath();
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