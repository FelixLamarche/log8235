// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "LoadBalancerManager.h"
#include "AiAgentGroupManager.h"
#include "BehaviorTree/BlackboardComponent.h" 


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
    const float SIGHT_MAX_DISTANCE = 1000.0f;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    const FVector playerLocation = playerCharacter->GetActorLocation();
    const FVector selfLocation = GetPawn()->GetActorLocation();
    bool isAgentCloseEnough = FVector::Dist(selfLocation, playerLocation) < SIGHT_MAX_DISTANCE;

    GetWorld()->LineTraceSingleByObjectType(losHit, selfLocation, playerLocation, TraceObjectTypes);

    bool LoSOnPlayer = false;
    AAiAgentGroupManager* groupManager = AAiAgentGroupManager::GetInstance();
    if (losHit.GetComponent() && losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER && isAgentCloseEnough)
    {
        /*GetBlackboardComponent()->
        behaviorTree->GetBlackboardComponent()*/
        LoSOnPlayer = true;
        groupManager->UpdatePlayerLKP(playerCharacter->GetActorLocation());
        groupManager->RegisterAIAgent(this);
        TargetLocation = playerCharacter->GetActorLocation();
    }

    if (IsInPursuitGroup)
    {
        const float MIN_DIST_TO_PLAYER = 300.0f;
        bool tryToCatchPlayer = LoSOnPlayer && (FVector::Dist(selfLocation, playerLocation) <= MIN_DIST_TO_PLAYER);

        if (tryToCatchPlayer)
        {
			TargetLocation = playerCharacter->GetActorLocation();
            GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("Catching player at %s"), *TargetLocation.ToString()));
        }
        else if (groupManager->HasGroupLoSOnPlayer())
        {
            groupManager->RegisterAIAgent(this);
            TargetLocation = positioning;
            GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("Positioning player at %s"), *TargetLocation.ToString()));
        }
        else
        {
            TargetLocation = groupManager->GetPlayerLKP();
            if (FVector::Dist(selfLocation, TargetLocation) < 100.0f)
            {
                groupManager->CheckIfDisband();
                GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("Check disband at %s"), *TargetLocation.ToString()));
			}
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("player LKP at %s"), *TargetLocation.ToString()));
            }
        }
    }

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

