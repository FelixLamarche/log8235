// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTUtils.h"
#include  "PhysicsHelpers.h"

#include "SDTAIController.generated.h"

/**
 *
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    ASDTAIController();

    virtual void Tick(float deltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = AI)
    FVector m_speed = FVector(0.0, 0.0, 0.0);
    UPROPERTY(VisibleAnywhere, Category = AI)
    float m_speed_value = 0.f;
    UPROPERTY(EditAnywhere, Category = AI)
    float m_max_speed = 0.7f;
    UPROPERTY(EditAnywhere, Category = AI)
    double m_max_acceleration = 1.5;
    UPROPERTY(EditAnywhere, Category = AI)
    double m_distance_vision = 400.0;
    UPROPERTY(EditAnywhere, Category = AI)
    double m_angle_vision = 25.0;

private:
    struct HitInfoWall
    {
        bool hitCenter = false;
        bool hitLeft = false;
        bool hitRight = false;
        bool hitTooClose = false;
    };
    void Move(APawn* pawn, FVector dir, float acceleration, float deltaTime, float rotFactor = 0.007f);
    bool DetectWall(APawn* pawn, float distance, HitInfoWall &hitInfo, PhysicsHelpers pHelper, bool debug = false);
    void avoidTheWall(APawn* pawn, HitInfoWall hitInfo, float deltaTime);
    bool DetectPlayer(APawn*player ,APawn* pawn, PhysicsHelpers pHelper, bool debug);
    void Pursuite(APawn* player, UWorld* world, APawn* pawn, PhysicsHelpers pHelper, float deltaTime);
};
