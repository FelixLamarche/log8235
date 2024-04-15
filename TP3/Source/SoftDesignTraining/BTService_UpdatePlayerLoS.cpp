// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdatePlayerLoS.h"
#include "SDTAIController.h"
#include "BehaviorTree/BlackboardComponent.h" 
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

UBTService_UpdatePlayerLoS::UBTService_UpdatePlayerLoS()
{
    bCreateNodeInstance = true;
}

void UBTService_UpdatePlayerLoS::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        //aiController->UpdateLoSOnPlayer();

        UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();

        //write to bb that the player is seen
        const FName& hasLoSOnPlayerBBKey = aiController->GetBBKeyHasLoSOnPlayer();
        const bool hasLoSOnPlayer = aiController->HasLoSOnPlayer;
        OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(hasLoSOnPlayerBBKey, hasLoSOnPlayer);

        if (hasLoSOnPlayer)
        {
            //write to bb the position of the target
            const FName& targetLocationBBKey = aiController->GetBBKeyTargetLocation();
            OwnerComp.GetBlackboardComponent()->SetValueAsVector(targetLocationBBKey, aiController->TargetLocation);
        }
    }
}