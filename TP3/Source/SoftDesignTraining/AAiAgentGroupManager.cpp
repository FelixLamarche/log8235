// Fill out your copyright notice in the Description page of Project Settings.


#include "AAiAgentGroupManager.h"

AAiAgentGroupManager* AAiAgentGroupManager::m_instance;

AAiAgentGroupManager::AAiAgentGroupManager()
{
    m_instance = this;
}

AAiAgentGroupManager* AAiAgentGroupManager::GetInstance()
{
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
    float angleBetweenAgents = 2 * PI / m_registeredAgents.Num();

    // Set the radius of the circle
    float radius = 100.0f; // Change this to the desired radius

    // Assign each agent a position around the player
    int i = 0;
    for (ASDTAIController* agent : m_registeredAgents)
    {
        // Calculate the agent's position
        FVector agentPosition;
        agentPosition.X = m_playerLKP.X + radius * cos(i * angleBetweenAgents);
        agentPosition.Y = m_playerLKP.Y + radius * sin(i * angleBetweenAgents);
        agentPosition.Z = m_playerLKP.Z; // Change this if you want the agents to be at a different height

        // Assign the calculated position to the agent
        agent->positioning = agentPosition;

        i++;
    }
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

    for(auto  agent: m_registeredAgents)
    {
        // R�cup�ration du NPC
        if (ASDTAIController* ai = Cast<ASDTAIController>(agent))
        {
            // Si l'agent est sur la cam�ra
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