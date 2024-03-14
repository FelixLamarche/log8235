// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h" 
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
    APawn* pawn = GetPawn();
    if (!world || !pawn) // Check if pawn or world is null
    {
        return;
    }

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (BehaviourState == AIState::ChasingPlayer && playerCharacter)
    {
        PathFollowingResult = MoveToActor(playerCharacter, -1, false);
    }
    else
    {
        PathFollowingResult = MoveToLocation(BestTargetLocation);
    }
    OnMoveToTarget();

    if (showDebug)
    {
        DrawDebugSphere(world, BestTargetLocation, 50, 50, FColor::Red, false);
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
    
    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    CheckPlayerVisibility();
  
    // Prioritize chasing the player
    if (IsPlayerVisible && BehaviourState != AIState::ChasingPlayer && playerCharacter)
    {
        AIStateInterrupted();
        BestTargetLocation = playerCharacter->GetActorLocation();
        BehaviourState = AIState::ChasingPlayer;
        GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, TEXT("chasing"));
    }
    // Then start chasing the last known player location when he is no longer visible
    else if (!IsPlayerVisible && BehaviourState == AIState::ChasingPlayer)
    {
        AIStateInterrupted();
        BehaviourState = AIState::ChasingPlayerLastLocation;
        BestTargetLocation = LastKnownPlayerLocation;
        
        GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Blue, TEXT("chasing location"));
    }
    // Else if we do not have a goal, we search for a collectible
    // Examining path request to solve a bug which avoids locking the state of the agent
    else if (m_ReachedTarget || BehaviourState == AIState::Idle || 
        PathFollowingResult == EPathFollowingRequestResult::Failed || 
        PathFollowingResult == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        ASDTCollectible* bestCollectible = GetBestCollectible();
        BestTargetLocation = bestCollectible->GetActorLocation();
        BehaviourState = AIState::GettingCollectible;
        GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, TEXT("finding collectible"));
    }

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
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


// Calculates and returns the nearest collectible to retrieve
ASDTCollectible* ASDTAIController::GetBestCollectible()
{
    UWorld* world = GetWorld();
    APawn* pawn = GetPawn();
    if (!world || !pawn)
        return nullptr;

    TArray<AActor*> collectibles;
    UGameplayStatics::GetAllActorsOfClass(world, ASDTCollectible::StaticClass(), collectibles);

    FVector pathStart = pawn->GetActorLocation();
    float bestPathLength = INFINITY;
    ASDTCollectible* bestCollectible = nullptr;

    for (int i = 0; i < collectibles.Num(); i++)
    {
        ASDTCollectible* collectible = Cast<ASDTCollectible>(collectibles[i]);
        if (!collectible || collectible->IsOnCooldown()) // Check if collectible is null or on cooldown
        {
            continue;
        }

        FVector pathEnd = collectible->GetActorLocation();
        // If calculating the path for each collectible is too long, just use distance
        UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(world, pathStart, pathEnd, pawn);
        if (!path) // Check if path is null
        {
            continue;
        }

        float pathLength = path->GetPathLength();
        if (bestCollectible == nullptr || pathLength < bestPathLength)
        {
            bestPathLength = pathLength;
            bestCollectible = collectible;
        }
    }

    return bestCollectible;
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

// Checks if player is visible and sets their last known position
void ASDTAIController::CheckPlayerVisibility() 
{
    const AActor* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    UWorld* world = GetWorld();
    const APawn* selfPawn = GetPawn();

    if (!player || !world || !selfPawn)
    {
        IsPlayerVisible = false;
        return;
    }

    const FVector selfPosition = selfPawn->GetActorLocation();
    const FVector playerLocation = player->GetActorLocation();

    // Raycast to the left and right of the player for better accuracy of the visibility
    const float playerHalfWidth = 50.0f;
    const FVector playerLeftSide = playerLocation + playerHalfWidth * FVector::LeftVector;
    const FVector playerRightSide = playerLocation + playerHalfWidth * FVector::RightVector;
    IsPlayerVisible = !SDTUtils::Raycast(world, selfPosition, playerLocation) &&
        !SDTUtils::Raycast(world, selfPosition,  playerLeftSide) &&
        !SDTUtils::Raycast(world, selfPosition, playerRightSide);

    if (IsPlayerVisible)
    {
        LastKnownPlayerLocation = playerLocation;
    }
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