// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()

public:
    ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    //virtual void UpdatePlayerInteraction(float deltaTime) override;

    virtual void Tick(float deltaTime) override;

    virtual void ShowNavigationPath() override;

    //virtual void GoToBestTarget(float deltaTime) override;

    void SetClosestCollectibleAsTarget();
    void SetBestFleeLocationAsTarget();


    void UpdateTickRateMovementComponent();
    void UpdateTickRateSKinMeshComponent();

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
    bool HasLoSOnPlayer = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool IsInPursuitGroup = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool IsActorOnCamera = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    FVector TargetLocation = FVector::ZeroVector;

    const FName& GetBBKeyHasLoSOnPlayer() const { return m_BBKeyHasLoSOnPlayer; }
    const FName& GetBBKeyTargetLocation() const { return m_BBKeyTargetLocation; }
    const FName& GetBBKeyIsInPursuitGroup() const { return m_BBKeyIsInPursuitGroup; }

protected:

    enum PlayerInteractionBehavior
    {
        PlayerInteractionBehavior_Collect,
        PlayerInteractionBehavior_Chase,
        PlayerInteractionBehavior_Flee
    };

    UPROPERTY(EditAnywhere, category = Behavior)
    UBehaviorTree* m_aiBehaviorTree;

    //void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
    //void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime);
    //PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit);
    //bool HasLoSOnHit(const FHitResult& hit);
    //void MoveToRandomCollectible();
    //void MoveToPlayer();
    //void PlayerInteractionLoSUpdate();
    //void OnPlayerInteractionNoLosDone();
    //void OnMoveToTarget();
    
    void UpdateIsActorOnCamera();

public:
    //void MoveToBestFleeLocation();
    void UpdateLoSOnPlayer();
    //virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void RotateTowards(const FVector& targetLocation);
    void SetActorLocation(const FVector& targetLocation);
    void AIStateInterrupted();

protected:
    FVector m_JumpTarget;
    FRotator m_ObstacleAvoidanceRotation;
    FTimerHandle m_PlayerInteractionNoLosTimer;
    PlayerInteractionBehavior m_PlayerInteractionBehavior;

private:
    FName m_BBKeyHasLoSOnPlayer = TEXT("HasLoSOnPlayer");
    FName m_BBKeyTargetLocation = TEXT("TargetLocation");
    FName m_BBKeyIsInPursuitGroup = TEXT("IsInPursuitGroup");
};
