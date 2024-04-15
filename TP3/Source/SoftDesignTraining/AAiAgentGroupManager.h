// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTAIController.h"
#include "GameFramework/Actor.h"
#include "AAiAgentGroupManager.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API AAiAgentGroupManager : public AActor
{
	GENERATED_BODY()

public:
	AAiAgentGroupManager();

	static AAiAgentGroupManager *GetInstance();
	static void Destroy();

	void RegisterAIAgent(ASDTAIController *aiAgent);
	void UnregisterAIAgent(ASDTAIController *aiAgent);

	void UpdatePlayerLKP(FVector lkp);
	bool AgentAtLKP();
	FVector GetPlayerLKP();
	void Disband();
	FVector CalculateAgentPosition(int i, float radius, float angleBetweenAgents, float agentRadius);
    FVector CalculateOffsetVector(FVector agentPosition, float agentRadius);
    FVector HandleLineTrace(FVector agentPosition, float agentRadius);
    FVector HandleAgentProximity(FVector agentPosition, float agentRadius);
    FVector HandleWallProximity(FVector agentPosition, float agentRadius);
    FVector ProjectToNavigation(FVector agentPosition, float agentRadius);
	
	void DrawDebugGroup(UWorld *World);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	TSet<ASDTAIController *> m_registeredAgents;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	TArray<FVector> m_CirclePositions;
private:
	FVector m_playerLKP;
	static AAiAgentGroupManager *m_instance;
};
