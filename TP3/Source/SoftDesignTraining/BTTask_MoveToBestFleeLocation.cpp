// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveToBestFleeLocation.h"
#include "SDTAIController.h"
#include "SDTUtils.h"

EBTNodeResult::Type UBTTask_MoveToBestFleeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->MoveToBestFleeLocation();

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}