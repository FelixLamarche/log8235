// Fill out your copyright notice in the Description page of Project Settings.


#include "AiAgentGroupManager.h"

AiAgentGroupManager* AiAgentGroupManager::m_instance;

AiAgentGroupManager::AiAgentGroupManager()
{
}

AiAgentGroupManager* AiAgentGroupManager::GetInstance()
{
    if (!m_instance)
    {
        m_instance = new AiAgentGroupManager();
    }

    return m_instance;
}

void AiAgentGroupManager::Destroy()
{
	delete m_instance;
    m_instance = nullptr;
}

void AiAgentGroupManager::RegisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Add(aiAgent);
    aiAgent->IsInPursuitGroup = true;
}

void AiAgentGroupManager::UnregisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
    aiAgent->IsInPursuitGroup = false;
}

void AiAgentGroupManager::DrawDebugGroup()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(AiAgentGroupManager::DrawDebugGroup);

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
                DrawDebugSphere(
                    ai->GetWorld(),
                    head,
                    30.f,
                    32,
                    FColor::Purple
                    );
            }
        }
    
    }
}