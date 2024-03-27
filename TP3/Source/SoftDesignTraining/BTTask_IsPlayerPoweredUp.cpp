// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsPlayerPoweredUp.h"
#include "SDTUtils.h"

EBTNodeResult::Type UBTTask_IsPlayerPoweredUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UWorld* world = GetWorld();

	if(SDTUtils::IsPlayerPoweredUp(world))
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}