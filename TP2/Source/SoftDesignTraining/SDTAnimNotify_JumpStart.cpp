// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpStart.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
void USDTAnimNotify_JumpStart::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
    //Notify that the NPC has launched
    ASDTAIController* aiController = dynamic_cast<ASDTAIController*>(MeshComp->GetOwner());

    if (aiController != nullptr)
    {
		aiController->InAir = true;
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("USDTAnimNotify_JumpStart::Notify: aiController is nullptr"));
	}
}
