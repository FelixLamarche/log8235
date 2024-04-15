// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "LoadBalancerManager.h"
#include "AAiAgentGroupManager.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
}

void ASDTAIController::BeginPlay()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::BeginPlay);
    Super::BeginPlay();
    playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    LoadBalancerManager* loadBalancerManager = LoadBalancerManager::GetInstance();
    if (loadBalancerManager)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::RegisterNPC);
        loadBalancerManager->RegisterNPC(this);
    }
}

void ASDTAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    LoadBalancerManager* loadBalancerManager = LoadBalancerManager::GetInstance();
    if (loadBalancerManager)
    {
        loadBalancerManager->UnregisterNPC(this);
    }


}

void ASDTAIController::UpdateLoSOnPlayer()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::UpdateLoSOnPlayer);

    if (!playerCharacter)
        return;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);


    bool LoSOnPlayer = false;
    HasLoSOnPlayer = false;

    AAiAgentGroupManager* groupManager = AAiAgentGroupManager::GetInstance();
    if (losHit.GetComponent())
    {
        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            LoSOnPlayer = true;
            groupManager->UpdatePlayerLKP(playerCharacter->GetActorLocation());
            groupManager->RegisterAIAgent(this);
            TargetLocation = playerCharacter->GetActorLocation();
        }
    }

    if (!LoSOnPlayer && IsInPursuitGroup)
    {
        // Set target as the last known position of the player
        TargetLocation = groupManager->GetPlayerLKP();
    }

    if (IsInPursuitGroup) {
        TargetLocation = positioning;
    }

    //bool justObtainedLoS = LoSOnPlayer && (LoSOnPlayer != HasLoSOnPlayer);
    //bool justLostLoS = !LoSOnPlayer && HasLoSOnPlayer;
    //if (justObtainedLoS)
    //{
    //    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", GetPawn(), FColor::Red, 5.f, false);
    //}
    //else if (justLostLoS)
    //{
    //    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", GetPawn(), FColor::Red, 5.f, false);
    //}

    // Update the LoS status
    HasLoSOnPlayer = LoSOnPlayer;
}


void ASDTAIController::SetClosestCollectibleAsTarget()
{
    float closestSqrCollectibleDistance = 9999999999999.9f;
    ASDTCollectible* closestCollectible = nullptr;

    TArray<AActor*> foundCollectibles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

    // Find the closest collectible
    for (int i = 0; i < foundCollectibles.Num(); i++)
    {
        ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[i]);
		if (!collectibleActor)
			continue;

        if (!collectibleActor->IsOnCooldown())
        {
			float sqrDist = FVector::DistSquared(foundCollectibles[i]->GetActorLocation(), GetPawn()->GetActorLocation());
            if (sqrDist < closestSqrCollectibleDistance)
            {
				closestSqrCollectibleDistance = sqrDist;
				closestCollectible = collectibleActor;
			}
		}
    }

    // Move to the collectible
    if (closestCollectible)
    {
        DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), TEXT("Collect"), GetPawn(), FColor::Orange, 1.f, false);
        TargetLocation = closestCollectible->GetActorLocation();
	}
}


void ASDTAIController::SetBestFleeLocationAsTarget()
{
    float bestLocationScore = 0.f;
    ASDTFleeLocation* bestFleeLocation = nullptr;

    //ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
    {
        ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
        if (fleeLocation)
        {
            float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());

            FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToPlayer.Normalize();

            FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToFleeLocation.Normalize();

            float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
            float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

            if (locationScore > bestLocationScore)
            {
                bestLocationScore = locationScore;
                bestFleeLocation = fleeLocation;
            }

            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);
        }
    }

    if (bestFleeLocation)
    {
        DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), TEXT("Flee"), GetPawn(), FColor::Orange, 1.f, false);
        TargetLocation = bestFleeLocation->GetActorLocation();
    }
}


void ASDTAIController::RotateTowards(const FVector& targetLocation)
{
    if (!targetLocation.IsZero())
    {
        FVector direction = targetLocation - GetPawn()->GetActorLocation();
        FRotator targetRotation = direction.Rotation();

        targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

        SetControlRotation(targetRotation);
    }
}

void ASDTAIController::SetActorLocation(const FVector& targetLocation)
{
    GetPawn()->SetActorLocation(targetLocation);
}


void ASDTAIController::ShowNavigationPath()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::ShowNavigationPath)
    if (UPathFollowingComponent* pathFollowingComponent = GetPathFollowingComponent())
    {
        if (pathFollowingComponent->HasValidPath())
        {
            const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
            TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

            for (int i = 0; i < pathPoints.Num(); ++i)
            {
                DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

                if (i != 0)
                {
                    DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
                }
            }
        }
    }
}


void ASDTAIController::UpdateIsActorOnCamera()
{   
    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::UpdateIsActorOnCamera);
	AActor* selfPawn = GetPawn();
    
    if (!selfPawn)
        return;

    // Su l'acteur a �t� rendu r�cemment, on met � jour la variable IsActorOnCamera � true
    IsActorOnCamera = selfPawn->WasRecentlyRendered(1.5f);
}

void ASDTAIController::UpdateTickRateMovementComponent()
{   
    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::UpdateTickRateMovementComponent);
    APawn* selfPawn = GetPawn();

    if (!selfPawn)
		return;

    UCharacterMovementComponent* movementComponent = selfPawn->FindComponentByClass<UCharacterMovementComponent>();
    if (movementComponent)
    {   
        // Si l'acteur est sur la cam�ra ou qu'il est dans le groupe de poursuite, on met � jour le tick rate du mouvement component � 0
        if (IsActorOnCamera || IsInPursuitGroup)
        {
			movementComponent->SetComponentTickInterval(0.f);
		}
        else
        {
			movementComponent->SetComponentTickInterval(2.f);
		}
	}
}

void ASDTAIController::UpdateTickRateSKinMeshComponent()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::UpdateTickRateSKinMeshComponent);
    APawn* selfPawn = GetPawn();

    if (!selfPawn)
        return;

	USkeletalMeshComponent* skinMeshComponent = selfPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (skinMeshComponent)
    {
        if (IsActorOnCamera)
        {
			skinMeshComponent->SetComponentTickInterval(0.f);
		}
        else
        {
			skinMeshComponent->SetComponentTickInterval(2.f);
		}
	}
}

void ASDTAIController::Tick(float deltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::Tick);
	Super::Tick(deltaTime);

    bool oldIsActorOnCamera = IsActorOnCamera; // Added to store the old value of IsActorOnCamera

    UpdateIsActorOnCamera(); // Added to update the IsActorOnCamera variable

    if (oldIsActorOnCamera != IsActorOnCamera) // Added to update the tick rate of the movement component only when the value of IsActorOnCamera changes
    {		
        UpdateTickRateMovementComponent(); // Added to update the tick rate of the movement component
        UpdateTickRateSKinMeshComponent(); // Added to update the tick rate of the skin mesh component
	}
    APawn* selfPawn = GetPawn();
    float radius = 50.0f; // default radius

    radius = selfPawn->GetSimpleCollisionRadius();
    DrawDebugSphere(
        GetWorld(), 
        positioning,
        radius,
        12,
        FColor::Blue,
        false, 
        -1,
        0, 
        1
    );

    //ShowNavigationPath();
    // if (m_ReachedTarget)
    // {
	//	//GoToBestTarget(deltaTime);
	// }
     if (IsActorOnCamera) // 'else if' added to avoid calling ShowNavigationPath() when the actor is not on camera
    {
		ShowNavigationPath();
	}
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    AAiAgentGroupManager* groupManager = AAiAgentGroupManager::GetInstance();
    if (groupManager)
	{
		groupManager->UnregisterAIAgent(this);
	}
    m_ReachedTarget = true;
}


//////////////////////////////////////////////////////////////////////////
//
// OLD CODE
//////////////////////////////////////////////////////////////////////////

//
//void ASDTAIController::GoToBestTarget(float deltaTime)
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::GoToBestTarget)
//        switch (m_PlayerInteractionBehavior)
//        {
//        case PlayerInteractionBehavior_Collect:
//
//            MoveToRandomCollectible();
//
//            break;
//
//        case PlayerInteractionBehavior_Chase:
//
//            MoveToPlayer();
//
//            break;
//
//        case PlayerInteractionBehavior_Flee:
//
//            MoveToBestFleeLocation();
//
//            break;
//        }
//}
//
//
//
//
//void ASDTAIController::MoveToRandomCollectible()
//{
//    float closestSqrCollectibleDistance = 18446744073709551610.f;
//    ASDTCollectible* closestCollectible = nullptr;
//
//    TArray<AActor*> foundCollectibles;
//    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);
//
//    while (foundCollectibles.Num() != 0)
//    {
//        int index = FMath::RandRange(0, foundCollectibles.Num() - 1);
//
//        ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
//        if (!collectibleActor)
//            return;
//
//        if (!collectibleActor->IsOnCooldown())
//        {
//            MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, NULL, false);
//            OnMoveToTarget();
//            return;
//        }
//        else
//        {
//            foundCollectibles.RemoveAt(index);
//        }
//    }
//}
//
//
//void ASDTAIController::MoveToBestFleeLocation()
//{
//    float bestLocationScore = 0.f;
//    ASDTFleeLocation* bestFleeLocation = nullptr;
//
//    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
//    if (!playerCharacter)
//        return;
//
//    for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
//    {
//        ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
//        if (fleeLocation)
//        {
//            float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());
//
//            FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
//            selfToPlayer.Normalize();
//
//            FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
//            selfToFleeLocation.Normalize();
//
//            float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
//            float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;
//
//            if (locationScore > bestLocationScore)
//            {
//                bestLocationScore = locationScore;
//                bestFleeLocation = fleeLocation;
//            }
//
//            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);
//        }
//    }
//
//    if (bestFleeLocation)
//    {
//        MoveToLocation(bestFleeLocation->GetActorLocation(), 0.5f, false, true, false, NULL, false);
//        OnMoveToTarget();
//    }
//}
//
//
//void ASDTAIController::MoveToPlayer()
//{
//    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
//    if (!playerCharacter)
//        return;
//
//    MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);
//    OnMoveToTarget();
//}
//
//
//void ASDTAIController::PlayerInteractionLoSUpdate()
//{
//    // REMOVE THIS FUNCTION ONCE BEHAVIOUR IS IN THE BEHAVIOUR TREE
//    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
//    if (!playerCharacter)
//        return;
//
//    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
//    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
//    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));
//
//    FHitResult losHit;
//    GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);
//
//    bool hasLosOnPlayer = false;
//
//    if (losHit.GetComponent())
//    {
//        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
//        {
//            hasLosOnPlayer = true;
//        }
//    }
//
//    if (hasLosOnPlayer)
//    {
//        if (GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
//        {
//            GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
//            m_PlayerInteractionNoLosTimer.Invalidate();
//            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", GetPawn(), FColor::Red, 5.f, false);
//        }
//    }
//    else
//    {
//        if (!GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
//        {
//            GetWorld()->GetTimerManager().SetTimer(m_PlayerInteractionNoLosTimer, this, &ASDTAIController::OnPlayerInteractionNoLosDone, 3.f, false);
//            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", GetPawn(), FColor::Red, 5.f, false);
//        }
//    }
//
//}
//
//
//void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(ASDTAIController::UpdatePlayerInteraction)
//        //finish jump before updating AI state
//        if (AtJumpSegment)
//            return;
//
//    APawn* selfPawn = GetPawn();
//    if (!selfPawn)
//        return;
//
//    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
//    if (!playerCharacter)
//        return;
//
//    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
//    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;
//
//    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
//    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));
//
//    TArray<FHitResult> allDetectionHits;
//    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));
//
//    FHitResult detectionHit;
//    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);
//
//    UpdatePlayerInteractionBehavior(detectionHit, deltaTime);
//
//    if (GetMoveStatus() == EPathFollowingStatus::Idle)
//    {
//        m_ReachedTarget = true;
//    }
//
//    FString debugString = "";
//
//    switch (m_PlayerInteractionBehavior)
//    {
//    case PlayerInteractionBehavior_Chase:
//        debugString = "Chase";
//        break;
//    case PlayerInteractionBehavior_Flee:
//        debugString = "Flee";
//        break;
//    case PlayerInteractionBehavior_Collect:
//        debugString = "Collect";
//        break;
//    }
//
//    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, GetPawn(), FColor::Orange, 0.f, false);
//
//    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
//}
//
//bool ASDTAIController::HasLoSOnHit(const FHitResult& hit)
//{
//    if (!hit.GetComponent())
//        return false;
//
//    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
//    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
//
//    FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
//    hitDirection.Normalize();
//
//    FHitResult losHit;
//    FCollisionQueryParams queryParams = FCollisionQueryParams();
//    queryParams.AddIgnoredActor(hit.GetActor());
//
//    GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);
//
//    return losHit.GetActor() == nullptr;
//}
//

//
//
//void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
//{
//    Super::OnMoveCompleted(RequestID, Result);
//
//    m_ReachedTarget = true;
//}
//
//void ASDTAIController::OnMoveToTarget()
//{
//    m_ReachedTarget = false;
//}
//
//
//
//ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult& hit)
//{
//    if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Collect)
//    {
//        if (!hit.GetComponent())
//            return PlayerInteractionBehavior_Collect;
//
//        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
//            return PlayerInteractionBehavior_Collect;
//
//        if (!HasLoSOnHit(hit))
//            return PlayerInteractionBehavior_Collect;
//
//        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
//    }
//    else
//    {
//        //PlayerInteractionLoSUpdate();
//
//        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
//    }
//}
//
//void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
//{
//    for (const FHitResult& hit : hits)
//    {
//        if (UPrimitiveComponent* component = hit.GetComponent())
//        {
//            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
//            {
//                //we can't get more important than the player
//                outDetectionHit = hit;
//                return;
//            }
//            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
//            {
//                outDetectionHit = hit;
//            }
//        }
//    }
//}
//
//
//void ASDTAIController::UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime)
//{
//    PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit);
//
//    if (currentBehavior != m_PlayerInteractionBehavior)
//    {
//        m_PlayerInteractionBehavior = currentBehavior;
//        AIStateInterrupted();
//    }
//}
//
//
//
//void ASDTAIController::OnPlayerInteractionNoLosDone()
//{
//    GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
//    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "TIMER DONE", GetPawn(), FColor::Red, 5.f, false);
//
//    if (!AtJumpSegment)
//    {
//        AIStateInterrupted();
//        m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
//    }
//}