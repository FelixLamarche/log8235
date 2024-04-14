// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "BTTask_SetFleeLocAsTarget.h"

EBTNodeResult::Type UBTTask_SetFleeLocAsTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->SetBestFleeLocationAsTarget();
        const FVector targetLocation = aiController->TargetLocation;
        const FName hasLoSOnPlayerBBKey = aiController->GetBBKeyTargetLocation();

        const FName& targetLocationBBKey = aiController->GetBBKeyTargetLocation();
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(targetLocationBBKey, targetLocation);

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}