
#include "SDTAIController.h"
#include "SoftDesignTraining.h"

ASDTAIController::ASDTAIController()
{
}

void ASDTAIController::Tick(float deltaTime)
{
	APawn* pawn = GetPawn();

	const FVector dir = pawn->GetActorForwardVector();
	const FVector right = pawn->GetActorRightVector();

	Move(pawn, dir, m_acceleration, deltaTime);
	Rotate(pawn, right);

	//auto debug_string = FString::Printf(TEXT("Speed: %s"), *m_speed.ToString());
	//GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Green, debug_string);
}

void ASDTAIController::Move(APawn* pawn, FVector dir, float acceleration, float deltaTime)
{
	auto speed = FMath::Min(m_speed.Size() + deltaTime * acceleration, m_max_speed);
	m_speed = dir * speed;
	pawn->AddMovementInput(m_speed);
}

void ASDTAIController::Rotate(APawn* pawn, FVector dir, float factor)
{
	const FRotator current_rot = pawn->GetActorRotation();
	const FRotator result = FMath::Lerp(current_rot, dir.Rotation(), factor);
	pawn->SetActorRotation(result);
}