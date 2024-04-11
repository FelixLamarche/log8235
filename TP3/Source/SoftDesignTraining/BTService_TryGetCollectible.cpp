// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryGetCollectible.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

UBTService_TryGetCollectible::UBTService_TryGetCollectible()
{
    bCreateNodeInstance = true;
}

void UBTService_TryGetCollectible::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();

        //write to bb the position of the target
        const FName& targetLocationBBKey = aiController->GetBBKeyTargetLocation();
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(targetLocationBBKey, aiController->GetTargetLocation());
    }
}