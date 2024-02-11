// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "SDTCollectible.generated.h"


/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API ASDTCollectible : public AStaticMeshActor
{
	GENERATED_BODY()
public:
    ASDTCollectible();

    void Collect();
    void OnCooldownDone();
    bool IsOnCooldown();
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_CollectCooldownDuration = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
        bool isMoveable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
        USoundCue* m_PickUpSoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Audio)
        UAudioComponent* m_AudioComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
        UParticleSystem* m_ParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
        UParticleSystemComponent* m_ParticleSystemComponent = nullptr;

    virtual void Tick(float deltaTime) override;
    virtual void BeginPlay() override;
    // This function will be called whenever a property changes in the Blueprint
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

    FVector initialPosition;

protected:
    FTimerHandle m_CollectCooldownTimer;
	
};
