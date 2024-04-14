// Fill out your copyright notice in the Description page of Project Settings.

#include "AAiAgentGroupManager.h"
#include <NavigationSystem.h>

AAiAgentGroupManager *AAiAgentGroupManager::m_instance;

AAiAgentGroupManager::AAiAgentGroupManager()
{
    m_instance = this;
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

// FVector AAiAgentGroupManager::CalculateAgentPosition(int i, float radius, float angleBetweenAgents, float agentRadius)
// {
//     // Calculate the agent's position
//     FVector agentPosition;
//     agentPosition.X = m_playerLKP.X + radius * cos(i * angleBetweenAgents);
//     agentPosition.Y = m_playerLKP.Y + radius * sin(i * angleBetweenAgents);
//     agentPosition.Z = m_playerLKP.Z; // Change this if you want the agents to be at a different height

//     // Calculate the offset vector from the player's last known position to the agent's position
//     FVector offsetVector = (agentPosition - m_playerLKP).GetSafeNormal() * agentRadius;

//     // Add the offset vector to the agent's position
//     agentPosition += offsetVector;

//     // Perform a line trace from the player's position to the agent's position
//     FHitResult hitResult;
//     bool isHit = GetWorld()->LineTraceSingleByChannel(
//         hitResult,     // The result of the line trace
//         m_playerLKP,   // The start position of the line trace
//         agentPosition, // The end position of the line trace
//         ECC_Visibility // The collision channel to use for the line trace
//     );

//     // If the line trace hit something, offset the agent's position by the obstacle's width
//     if (isHit)
//     {
//         // Get the obstacle's width
//         float obstacleWidth = hitResult.GetActor()->GetSimpleCollisionHalfHeight() * 2;

//         // Offset the agent's position by the obstacle's width
//         agentPosition += hitResult.ImpactNormal * obstacleWidth;
//     }

//     // If the agent's position is within a 250 unit radius around the player, move it further away
//     if (FVector::Dist(agentPosition, m_playerLKP) < 250.0f)
//     {
//         FVector direction = (agentPosition - m_playerLKP).GetSafeNormal();
//         agentPosition = m_playerLKP + direction * 250.0f;
//     }

//     // Check if the agent's position is too close to any other agent
//     for (ASDTAIController* otherAgent : m_registeredAgents)
//     {
//         if (FVector::Dist(agentPosition, otherAgent->positioning) < agentRadius)
//         {
//             FVector direction = (agentPosition - otherAgent->positioning).GetSafeNormal();
//             agentPosition = otherAgent->positioning + direction * agentRadius;
//         }
//     }

//     // Check if the agent's position is too close to a wall
//     FHitResult wallHitResult;
//     bool isWallHit = GetWorld()->LineTraceSingleByChannel(
//         wallHitResult,     // The result of the line trace
//         agentPosition,     // The start position of the line trace
//         agentPosition + FVector(0, 0, -1) * agentRadius, // The end position of the line trace
//         ECC_WorldStatic    // The collision channel to use for the line trace
//     );

//     // If the line trace hit a wall, move the agent's position further away from the wall
//     if (isWallHit)
//     {
//         FVector direction = (agentPosition - wallHitResult.ImpactPoint).GetSafeNormal();
//         agentPosition = wallHitResult.ImpactPoint + direction * agentRadius;
//     }

//     return agentPosition;
// }

FVector AAiAgentGroupManager::CalculateAgentPosition(int i, float radius, float angleBetweenAgents, float agentRadius)
{
    // Calculate the agent's position
    FVector agentPosition;
    agentPosition.X = m_playerLKP.X + radius * cos(i * angleBetweenAgents);
    agentPosition.Y = m_playerLKP.Y + radius * sin(i * angleBetweenAgents);
    agentPosition.Z = m_playerLKP.Z; // Change this if you want the agents to be at a different height

    // Calculate the offset vector from the player's last known position to the agent's position
    FVector offsetVector = (agentPosition - m_playerLKP).GetSafeNormal() * agentRadius;

    // Add the offset vector to the agent's position
    agentPosition += offsetVector;

    // Get the navigation system
    UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this->GetWorld());

    // Project the agent's position to the nearest navigable area within a radius
    FNavLocation navLocation;
    bool isPointFound = navSystem->ProjectPointToNavigation(agentPosition, navLocation, FVector(agentRadius));

    // If a navigable point is found, set the agent's position to it
    if (isPointFound)
    {
        agentPosition = navLocation.Location;
    }
    else
    {
        // If the point is pushed outside the navigable area, set the agent's position to the edge of the navigable area
        isPointFound = navSystem->ProjectPointToNavigation(agentPosition, navLocation, FVector::ZeroVector);
        if (isPointFound)
        {
            agentPosition = navLocation.Location;
        }
    }

    return agentPosition;
}

void AAiAgentGroupManager::RegisterAIAgent(ASDTAIController *aiAgent)
{
    m_registeredAgents.Add(aiAgent);
    aiAgent->IsInPursuitGroup = true;
    float angleBetweenAgents = 2 * PI / m_registeredAgents.Num();

    float radius = 250.0f; 

    // Calculate the total radius required for all agents
    float totalRadiusRequired = m_registeredAgents.Num() * aiAgent->GetSimpleCollisionRadius() * 2;

    // If the total radius required is greater than the current radius, increase the current radius
    if (totalRadiusRequired > radius)
    {
        radius = totalRadiusRequired;
    }

    // Assign each agent a position around the player
    int i = 0;
    for (ASDTAIController *agent : m_registeredAgents)
    {
        // Calculate and assign the agent's position
        agent->positioning = CalculateAgentPosition(i, radius, angleBetweenAgents, aiAgent->GetSimpleCollisionRadius());

        i++;
    }

    // Update tick rate of the agent
    aiAgent->UpdateTickRateMovementComponent();
    aiAgent->UpdateTickRateSKinMeshComponent();
}

void AAiAgentGroupManager::UnregisterAIAgent(ASDTAIController *aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
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
                DrawDebugSphere(world, head, 30.f, 32, FColor::Purple);
            }
        }
    }

    if (m_registeredAgents.Num() > 0)
    {
        DrawDebugSphere(world, m_playerLKP, 30.f, 32, FColor::Red);
    }

    if (m_CirclePositions.Num() > 0)
    {
        for (auto pos : m_CirclePositions)
        {
            DrawDebugSphere(world, pos, 30.f, 32, FColor::Green);
        }
    }
}