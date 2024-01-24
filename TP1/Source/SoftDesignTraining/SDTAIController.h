// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

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

private:
    void Move(APawn* pawn, FVector dir, float acceleration, float deltaTime);
    void Rotate(APawn* pawn, FVector dir, float factor = 0.07f);

    UPROPERTY(VisibleAnywhere, Category = AI)
    FVector m_speed = FVector(0.0, 0.0, 0.0);
    UPROPERTY(EditAnywhere, Category = AI)
    double m_max_speed = 150.0;
    UPROPERTY(EditAnywhere, Category = AI)
    double m_acceleration = 100.0;
};
