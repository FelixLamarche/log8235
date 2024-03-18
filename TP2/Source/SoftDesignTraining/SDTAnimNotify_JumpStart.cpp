// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpStart.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
void USDTAnimNotify_JumpStart::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
    // Get the owner of the MeshComp
    APawn* PawnOwner = Cast<APawn>(MeshComp->GetOwner());

    if (PawnOwner)
    {
        // Get the controller of the pawn
        AController* Controller = PawnOwner->GetController();

        // Cast the controller to ASDTAIController
        ASDTAIController* AIController = Cast<ASDTAIController>(Controller);

        if (AIController)
        {
            // The controller is an ASDTAIController, you can now use it
            AIController->InAir = true;
        }

    }
}