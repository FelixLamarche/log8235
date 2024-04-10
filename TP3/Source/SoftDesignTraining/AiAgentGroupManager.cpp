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
    for (auto agent : m_registeredAgents)
    {
        FVector head;
        FRotator rotation;
        agent->GetPawn()->GetActorEyesViewPoint(head, rotation);
        DrawDebugSphere(
            agent->GetWorld(),
            head,
            30.f,
            32,
            FColor::Purple
        );
    }
}