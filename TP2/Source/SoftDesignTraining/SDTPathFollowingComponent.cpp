// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}

/**
* This function is called every frame while the AI is following a path.
* MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
*/
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	const TArray<FNavPathPoint>& points = Path->GetPathPoints();

	ASDTAIController* controller = dynamic_cast<ASDTAIController*>(GetOwner());
    FVector segmentStart = FVector::ZeroVector;
    //FVector startPoint = segmentStart.Location;
	//FVector endPoint = segmentEnd.Location;

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        // Update jump along path / nav link proxy
    }
    else
    {
        // Update navigation along path (move along)
        Super::FollowPathSegment(DeltaTime);
        controller->Landing = false;
    }
}

/**
* This function is called every time the AI has reached a new point on the path.
* If you need to do something at a given point in the path, this is the place.
*/
void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

    ASDTAIController* controller = dynamic_cast<ASDTAIController*>(GetOwner());

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    if (!controller->AtJumpSegment && SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        controller->AtJumpSegment = true;
    }
    else
    {
		controller->AtJumpSegment = false;
    }
}
