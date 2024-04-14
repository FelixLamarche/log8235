// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTAIController.h"
#include "GameFramework/Actor.h"
#include "AAiAgentGroupManager.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API AAiAgentGroupManager: AActor
{
	GENERATED_BODY()

public:
	AAiAgentGroupManager();

	static AAiAgentGroupManager* GetInstance();
	static void Destroy();

	void RegisterAIAgent(ASDTAIController* aiAgent);
	void UnregisterAIAgent(ASDTAIController* aiAgent);

	void UpdatePlayerLKP(FVector lkp);
	bool AgentAtLKP();

	void Disband();

	void DrawDebugGroup(UWorld* World);

private:
	static AAiAgentGroupManager* m_instance;

	TArray<ASDTAIController*> m_registeredAgents;
	FVector m_playerLKP;
};
