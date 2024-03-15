// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpEnd.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
void USDTAnimNotify_JumpEnd::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
    //Notify that our NPC has landed
    ASDTAIController* aiController = dynamic_cast<ASDTAIController*>(MeshComp->GetOwner());


    if (aiController != nullptr)
    {
        aiController->InAir = false;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USDTAnimNotify_JumpEnd::Notify: aiController is nullptr"));
    }
}
