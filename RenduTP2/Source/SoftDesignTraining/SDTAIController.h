// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTAIController.generated.h"

/**
 * 
 */

UENUM()
enum class AIState
{
    Idle,
    GettingCollectible,
    ChasingPlayer,
    ChasingPlayerLastLocation,
    FleeingPlayer
};

UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()

public:
    ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    AIState BehaviourState = AIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleHalfLength = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleRadius = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UCurveFloat* JumpCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpApexHeight = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpSpeed = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool AtJumpSegment = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool InAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool Landing = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool IsPlayerVisible = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    FVector BestTargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    float JumpCurveTime = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    float jumpProgression;

public:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void AIStateInterrupted();

protected:
    void OnMoveToTarget();
    void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
    void UpdatePlayerInteraction(float deltaTime);
    void CheckPlayerVisibility();
    ASDTCollectible* GetBestCollectible();
    ASDTFleeLocation* GetBestFleeLocation();


    EPathFollowingRequestResult::Type PathFollowingResult = EPathFollowingRequestResult::Type::AlreadyAtGoal;

private:
    virtual void GoToBestTarget(float deltaTime) override;
    virtual void ChooseBehavior(float deltaTime) override;
    virtual void ShowNavigationPath() override;
};
