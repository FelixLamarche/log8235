// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsPlayerPoweredUp.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "SDTUtils.h"

EBTNodeResult::Type UBTTask_IsPlayerPoweredUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UWorld* world = GetWorld();

	ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
	if(SDTUtils::IsPlayerPoweredUp(world) && aiController)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(aiController->GetBBKeyIsPlayerPoweredUp(), true);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}