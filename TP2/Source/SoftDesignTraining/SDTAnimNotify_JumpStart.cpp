// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpStart.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

void USDTAnimNotify_JumpStart::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
    //Notify that the NPC has launched
    // get animation end time
    ASDTAIController* aiController = Cast<ASDTAIController>(MeshComp->GetOwner());
    if (aiController != nullptr)
    {
		aiController->InAir = true;
	}
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("USDTAnimNotify_JumpStart::Notify: aiController is nullptr"));
	}
}
