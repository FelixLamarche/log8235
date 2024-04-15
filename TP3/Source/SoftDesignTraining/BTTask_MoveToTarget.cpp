// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToTarget.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "SDTAIController.h"

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        const FVector targetLocation = aiController->TargetLocation;
        const FName hasLoSOnPlayerBBKey = aiController->GetBBKeyTargetLocation();

        const FName& targetLocationBBKey = aiController->GetBBKeyTargetLocation();
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(targetLocationBBKey, targetLocation);

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}