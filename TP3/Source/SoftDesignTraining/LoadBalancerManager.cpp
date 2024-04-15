// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadBalancerManager.h"
#include "SDTAIController.h"
#include "SDTUtils.h"
#include "AAiAgentGroupManager.h"

LoadBalancerManager* LoadBalancerManager::m_Instance;
FDelegateHandle LoadBalancerManager::m_DelegateHandle;

LoadBalancerManager::LoadBalancerManager()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LoadBalancerManager::LoadBalancerManager);
	m_UpdateIndex = 0;
	m_MaxTimeUpdateNPC = 0.002f;
}

LoadBalancerManager* LoadBalancerManager::GetInstance()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LoadBalancerManager::GetInstance);
	if (!m_Instance)
	{
		LoadBalancerManager::Initialize();
	}

	return m_Instance;
}

void LoadBalancerManager::Initialize()
{
	m_Instance = new LoadBalancerManager();
	m_DelegateHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(m_Instance, &LoadBalancerManager::TickWorld);
}

void LoadBalancerManager::Destroy()
{
	FWorldDelegates::OnWorldPostActorTick.Remove(m_DelegateHandle);
	delete m_Instance;
	m_Instance = nullptr;
}

void LoadBalancerManager::RegisterNPC(AActor* npcCharacter)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LoadBalancerManager::RegisterNPC);
	m_NPCList.Add(npcCharacter);

}

void LoadBalancerManager::UnregisterNPC(AActor* npcCharacter)
{
	m_NPCList.Remove(npcCharacter);

}

AActor* LoadBalancerManager::GetPlayer()
{
	if (m_PlayerInstance.Num() != 0)
	{
		return m_PlayerInstance[0];
	}

	return NULL;
}

void LoadBalancerManager::RegisterPlayer(AActor* player)
{
	m_PlayerInstance.Add(player);
}

void LoadBalancerManager::UnregisterPlayer(AActor* player)
{
	m_PlayerInstance.Remove(player);
	this->Destroy();
}

void LoadBalancerManager::TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LoadBalancerManager::TickWorld);
	//Début du timer
	double StartTime = FPlatformTime::Seconds();
	// Temps écoulé
	double ElapsedTime = 0;

	// Affichage des PNJ dans le group de poursuite
	AAiAgentGroupManager* groupManager = AAiAgentGroupManager::GetInstance();
	if (groupManager)
	{
		groupManager->DrawDebugGroup(World);
	}

	ElapsedTime = FPlatformTime::Seconds() - StartTime;

	while (ElapsedTime < m_MaxTimeUpdateNPC)
	{
		// Récupération du NPC
		if (ASDTAIController* ai = Cast<ASDTAIController>(m_NPCList[m_UpdateIndex]))
		{	
			// Update de l'interaction avec le joueur
			//ai->UpdateLoSOnPlayer();
			ai->ShowNavigationPath();

		}

		m_UpdateIndex++;

		// Si on a parcouru tous les PNJ, on recommence
		if (m_UpdateIndex >= m_NPCList.Num())
		{
			m_UpdateIndex = 0;
		}

		// Mise à jour du temps écoulé
		ElapsedTime = FPlatformTime::Seconds() - StartTime;
	}
	
}