// Fill out your copyright notice in the Description page of Project Settings.

#include "AAiAgentGroupManager.h"
#include <NavigationSystem.h>

AAiAgentGroupManager *AAiAgentGroupManager::m_instance;

AAiAgentGroupManager::AAiAgentGroupManager()
{
    m_instance = this;
}

void AAiAgentGroupManager::CheckGroupLOS()
{
    for (auto agent : m_registeredAgents) {
        if (agent->HasLoSOnPlayer) {
            return;
		}
    }
    Disband();
}

AAiAgentGroupManager *AAiAgentGroupManager::GetInstance()
{
    return m_instance;
}

void AAiAgentGroupManager::Destroy()
{
    delete m_instance;
    m_instance = nullptr;
}

FVector AAiAgentGroupManager::GetPlayerLKP()
{
	return m_playerLKP;
}

void AAiAgentGroupManager::RegisterAIAgent(ASDTAIController *aiAgent)
{
    m_registeredAgents.Add(aiAgent);
    aiAgent->IsInPursuitGroup = true;
    float angleBetweenAgents = 2 * PI / m_registeredAgents.Num();

    float radius = 250.0f; 

    float totalRadiusRequired = m_registeredAgents.Num() * aiAgent->GetSimpleCollisionRadius() * 2;

    if (totalRadiusRequired > radius)
    {
        radius = totalRadiusRequired;
    }

    int i = 0;
    for (ASDTAIController *agent : m_registeredAgents)
    {
        agent->positioning = CalculateAgentPosition(i, radius, angleBetweenAgents, aiAgent->GetSimpleCollisionRadius());
        i++;
    }

    aiAgent->UpdateTickRateMovementComponent();
    aiAgent->UpdateTickRateSKinMeshComponent();
}

FVector AAiAgentGroupManager::CalculateAgentPosition(int i, float radius, float angleBetweenAgents, float agentRadius)
{
    FVector agentPosition;
    agentPosition.X = m_playerLKP.X + radius * cos(i * angleBetweenAgents);
    agentPosition.Y = m_playerLKP.Y + radius * sin(i * angleBetweenAgents);
    agentPosition.Z = m_playerLKP.Z;

    agentPosition += CalculateOffsetVector(agentPosition, agentRadius);
    agentPosition = HandleLineTrace(agentPosition,agentRadius);
    agentPosition = HandleAgentProximity(agentPosition, agentRadius);
    agentPosition = HandleWallProximity(agentPosition, agentRadius);
    agentPosition = ProjectToNavigation(agentPosition, agentRadius);

    return agentPosition;
}

FVector AAiAgentGroupManager::CalculateOffsetVector(FVector agentPosition, float agentRadius)
{
    return (agentPosition - m_playerLKP).GetSafeNormal() * agentRadius;
}

FVector AAiAgentGroupManager::HandleLineTrace(FVector agentPosition, float agentRadius)
{
    FHitResult hitResult;
    bool isHit = this->GetWorld()->LineTraceSingleByChannel(
        hitResult,
        m_playerLKP,
        agentPosition,
        ECC_Visibility
    );

    if (isHit)
    {
        FVector direction = (hitResult.ImpactPoint - agentPosition).GetSafeNormal();
        agentPosition = hitResult.ImpactPoint - direction * agentRadius;
    }

    return agentPosition;
}

FVector AAiAgentGroupManager::HandleAgentProximity(FVector agentPosition, float agentRadius)
{
    for (ASDTAIController* otherAgent : m_registeredAgents)
    {
        if (FVector::Dist(agentPosition, otherAgent->positioning) < agentRadius)
        {
            FVector direction = (otherAgent->positioning - m_playerLKP).GetSafeNormal();
            agentPosition = otherAgent->positioning - direction * agentRadius;
        }
    }

    return agentPosition;
}

FVector AAiAgentGroupManager::HandleWallProximity(FVector agentPosition, float agentRadius)
{
    FHitResult wallHitResult;
    bool isWallHit = GetWorld()->LineTraceSingleByChannel(
        wallHitResult,
        agentPosition,
        agentPosition + FVector(0, 0, -1) * agentRadius,
        ECC_WorldStatic
    );

    if (isWallHit)
    {
        FVector direction = (wallHitResult.ImpactPoint - m_playerLKP).GetSafeNormal();
        agentPosition = wallHitResult.ImpactPoint - direction * agentRadius;
    }

    return agentPosition;
}

FVector AAiAgentGroupManager::ProjectToNavigation(FVector agentPosition, float agentRadius)
{
    UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this->GetWorld());

    FNavLocation navLocation;
    bool isPointFound = navSystem->ProjectPointToNavigation(agentPosition, navLocation, FVector(agentRadius));

    if (isPointFound)
    {
        agentPosition = navLocation.Location;
    }
    else
    {
        FVector direction = (navLocation.Location - m_playerLKP).GetSafeNormal();
        agentPosition = navLocation.Location - direction * agentRadius;
    }

    return agentPosition;
}

void AAiAgentGroupManager::UnregisterAIAgent(ASDTAIController *aiAgent)
{
    aiAgent->IsInPursuitGroup = false;
    aiAgent->positioning = FVector::ZeroVector;
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
    m_registeredAgents.Empty();
}

void AAiAgentGroupManager::DrawDebugGroup(UWorld *world)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(AAiAgentGroupManager::DrawDebugGroup);

    for (auto agent : m_registeredAgents)
    {
        // R�cup�ration du NPC
        if (ASDTAIController *ai = Cast<ASDTAIController>(agent))
        {
            // Si l'agent est sur la cam�ra
            if (ai->IsActorOnCamera)
            {
                FVector head;
                FRotator rotation;
                ai->GetPawn()->GetActorEyesViewPoint(head, rotation);
                DrawDebugSphere(world, head, 30.f, 8, FColor::Purple);
            }
        }
    }

    if (m_registeredAgents.Num() > 0)
    {
        DrawDebugSphere(world, m_playerLKP, 30.f, 8, FColor::Red);
    }
}