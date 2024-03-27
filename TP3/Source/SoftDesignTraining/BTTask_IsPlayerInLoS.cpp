// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsPlayerInLoS.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

EBTNodeResult::Type UBTTask_IsPlayerInLoS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        const FName hasLoSOnPlayerBBKey = TEXT("HasLoSOnPlayer");
        if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>(hasLoSOnPlayerBBKey))
        {
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}