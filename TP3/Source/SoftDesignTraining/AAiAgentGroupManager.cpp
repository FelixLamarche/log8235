// Fill out your copyright notice in the Description page of Project Settings.


#include "AAiAgentGroupManager.h"

AAiAgentGroupManager* AAiAgentGroupManager::m_instance;

AAiAgentGroupManager::AAiAgentGroupManager()
{
}

AAiAgentGroupManager* AAiAgentGroupManager::GetInstance()
{
    if (!m_instance)
    {
        m_instance = new AAiAgentGroupManager();
    }

    return m_instance;
}

void AAiAgentGroupManager::Destroy()
{
	delete m_instance;
    m_instance = nullptr;
}

void AAiAgentGroupManager::RegisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Add(aiAgent);
    aiAgent->IsInPursuitGroup = true;
    // Update tick rate of the agent
    aiAgent->UpdateTickRateMovementComponent();
    aiAgent->UpdateTickRateSKinMeshComponent();

}

void AAiAgentGroupManager::UnregisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
    aiAgent->IsInPursuitGroup = false;
    // Update tick rate of the agent
    aiAgent->UpdateTickRateMovementComponent();
    aiAgent->UpdateTickRateSKinMeshComponent();
}

void AAiAgentGroupManager::UpdatePlayerLKP(FVector lkp)
{
    m_playerLKP = lkp;
}

bool AAiAgentGroupManager::AgentAtLKP()
{
    for (auto agent : m_registeredAgents)
    {
        if ((agent->GetPawn()->GetActorLocation() - m_playerLKP).Size2D() < 50.f)
        {
            return true;
        }
    }
    return false;
}

void AAiAgentGroupManager::Disband()
{
    for (auto agent : m_registeredAgents)
    {
        UnregisterAIAgent(agent);
    }
}

void AAiAgentGroupManager::DrawDebugGroup(UWorld* world)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(AAiAgentGroupManager::DrawDebugGroup);

    for(int i=0; i < m_registeredAgents.Num(); i++)
    {
        // Récupération du NPC
        if (ASDTAIController* ai = Cast<ASDTAIController>(m_registeredAgents[i]))
        {
            // Si l'agent est sur la caméra
            if (ai->IsActorOnCamera) 
            {
                FVector head;
                FRotator rotation;
                ai->GetPawn()->GetActorEyesViewPoint(head, rotation);
                DrawDebugSphere(world, head, 30.f, 32, FColor::Purple);
            }
        }
    
    }

    if (m_registeredAgents.Num() > 0)
    {
        DrawDebugSphere(world, m_playerLKP, 30.f, 32, FColor::Red);
    }
}