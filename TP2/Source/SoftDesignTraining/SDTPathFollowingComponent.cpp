// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer &ObjectInitializer)
{
}

/**
 * This function is called every frame while the AI is following a path.
 * MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
 */
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    ASDTAIController *controller = dynamic_cast<ASDTAIController *>(GetOwner());

    const TArray<FNavPathPoint> &points = Path->GetPathPoints();
    const FNavPathPoint &segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint &segmentEnd = points[MoveSegmentStartIndex + 1];

    if (!controller->AtJumpSegment)
        controller->JumpCurveTime = 0;
    GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("IsInAir: %f"), controller->InAir));
    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        // Update jump along path / nav link proxy
        APawn *pawn = controller->GetPawn();
        FVector pawnPos = pawn->GetActorLocation();
        controller->Landing = false;
        //Get controller velocity
        FVector velocity = pawn->GetVelocity();
        GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Green, FString::Printf(TEXT("Velocity: %f"), velocity.Size()));
        const auto toTarget = (segmentEnd.Location - pawnPos).GetSafeNormal();

        const double totalDistance = FVector::Dist2D(segmentStart.Location, segmentEnd.Location);
        const double coveredDistance = FVector::Dist2D(pawnPos, segmentStart.Location);
        controller->JumpCurveTime = coveredDistance / totalDistance;

        UCurveFloat *jumpCurve = controller->JumpCurve;
        float value = jumpCurve->GetFloatValue(controller->JumpCurveTime);

        pawnPos.Z = segmentStart.Location.Z + controller->JumpApexHeight * value;

        if (controller->JumpCurveTime > 0.9)
            controller->Landing = true;

        const auto displacement = toTarget * DeltaTime;
        pawn->SetActorLocation(pawnPos + displacement, true);
        pawn->AddMovementInput(toTarget, controller->JumpSpeed);
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

    ASDTAIController *controller = dynamic_cast<ASDTAIController *>(GetOwner());

    const TArray<FNavPathPoint> &points = Path->GetPathPoints();
    const FNavPathPoint &segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint &segmentEnd = points[MoveSegmentStartIndex + 1];

    if (!controller->AtJumpSegment && SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        controller->AtJumpSegment = true;
        Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
    }
    else
    {
        controller->AtJumpSegment = false;
        Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
    }
}
