// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsInPursuitGroup.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

EBTNodeResult::Type UBTTask_IsInPursuitGroup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        const bool IsInPursuitGroup = aiController->IsInPursuitGroup;
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(aiController->GetBBKeyIsInPursuitGroup(), IsInPursuitGroup);
        if (IsInPursuitGroup)
        {
            return EBTNodeResult::Succeeded;
        }
        else
        {
			return EBTNodeResult::Failed;
		}
    }

    return EBTNodeResult::Failed;
}
