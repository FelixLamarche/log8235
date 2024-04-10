// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SOFTDESIGNTRAINING_API LoadBalancerManager
{
private:
	LoadBalancerManager();

	static LoadBalancerManager* m_Instance;
	static FDelegateHandle m_DelegateHandle;

	void TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds);

	int32 m_UpdateIndex;

public:
	TArray<AActor*> m_NPCList;
	TArray<AActor*> m_PlayerInstance;

public:
	static LoadBalancerManager* GetInstance();
	static void Initialize();
	static void Destroy();

	void RegisterPlayer(AActor* npcCharacter);
	void UnregisterPlayer(AActor* npcCharacter);
	//tmp
	AActor* GetPlayer();
	//tmp
	void RegisterNPC(AActor* npcCharacter);
	void UnregisterNPC(AActor* npcCharacter);
	
	// Temps maximum alloué pour la mise à jour des PNJ par frame (en secondes)
	double m_MaxTimeUpdateNPC;
};
