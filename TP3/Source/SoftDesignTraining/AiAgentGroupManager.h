// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTAIController.h"

/**
 * 
 */
class SOFTDESIGNTRAINING_API AiAgentGroupManager
{
public:
	AiAgentGroupManager();

	static AiAgentGroupManager* GetInstance();
	static void Destroy();

	void RegisterAIAgent(ASDTAIController* aiAgent);
	void UnregisterAIAgent(ASDTAIController* aiAgent);

	void UpdatePlayerLKP(FVector lkp);
	FVector GetPlayerLKP() const { return m_playerLKP; }
	bool AgentAtLKP();

	void Disband();

	void DrawDebugGroup(UWorld* World);

private:
	static AiAgentGroupManager* m_instance;

	TArray<ASDTAIController*> m_registeredAgents;
	FVector m_playerLKP;
};
