
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ASDTAIController::ASDTAIController()
{
}

void ASDTAIController::Tick(float deltaTime)
{
	APawn* pawn = GetPawn();
	UWorld* world = GetWorld();
	PhysicsHelpers pHelper(world);

	HitInfoWall hitInfoW;
	
	// Detection d'un mur
	if (DetectWall(pawn, m_distance_vision, hitInfoW, pHelper, true))
	{
		// Evitement du mur
		avoidTheWall(pawn, hitInfoW, deltaTime);
	}
	else
	{
		// Si on ne detecte pas de mur, on accélère jusqu'à la vitesse max et on avance tout droit
		Move(pawn, pawn->GetActorForwardVector(),m_max_acceleration/2, deltaTime);
	}
}

void ASDTAIController::Move(APawn* pawn, FVector dir, float acceleration, float deltaTime, float rotFactor)
{
	// Calcul de la vitesse
	m_speed_value = FMath::Min(m_speed_value + deltaTime * acceleration, m_max_speed);
	m_speed = pawn->GetActorForwardVector() * m_speed_value;

	// Déplacement
	pawn->AddMovementInput(m_speed);

	// Orientation du personnage vers la direction du mouvement
	const FRotator current_rot = pawn->GetActorRotation();
	const FRotator result = FMath::Lerp(current_rot, dir.Rotation(), rotFactor);
	pawn->SetActorRotation(result);
}

bool ASDTAIController::DetectWall(APawn* pawn, float distance, HitInfoWall &hitInfo, PhysicsHelpers pHelper, bool debug)
{
	FVector sourcePoint = pawn->GetActorLocation();
	// Calcul de 4 points d'arrivée représentant un cone de vision d'angle m_angle_vision et un mur trop proche
	FVector targetPointCenter = sourcePoint + pawn->GetActorForwardVector() * distance;
	FVector targetPointLeft = sourcePoint + pawn->GetActorForwardVector().RotateAngleAxis(-m_angle_vision, FVector::UpVector) * distance * 0.8f;
	FVector targetPointRight = sourcePoint + pawn->GetActorForwardVector().RotateAngleAxis(m_angle_vision, FVector::UpVector) * distance * 0.8f;
	FVector targetPointTooClose = sourcePoint + pawn->GetActorForwardVector() * distance * 0.35f;
	
	// On tire 4 rayons et on vérifie si l'un d'entre eux touche un mur
	hitInfo.hitCenter = SDTUtils::Raycast(GetWorld(), sourcePoint, targetPointCenter);
	hitInfo.hitLeft = SDTUtils::Raycast(GetWorld(), sourcePoint, targetPointLeft);
	hitInfo.hitRight = SDTUtils::Raycast(GetWorld(), sourcePoint, targetPointRight);
	hitInfo.hitTooClose = SDTUtils::Raycast(GetWorld(), sourcePoint, targetPointTooClose);

	if (debug)
	{
	FColor colorCenter = hitInfo.hitTooClose ? FColor::Blue : (hitInfo.hitCenter ? FColor::Green : FColor::Red);
	FColor colorLeft = hitInfo.hitLeft ? FColor::Green : FColor::Red;
	FColor colorRight = hitInfo.hitRight ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), sourcePoint, targetPointCenter, colorCenter);
	DrawDebugLine(GetWorld(), sourcePoint, targetPointLeft, colorLeft);
	DrawDebugLine(GetWorld(), sourcePoint, targetPointRight, colorRight);
	
	GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Green, FString::Printf(TEXT("Hit detected : Center %d  Left %d  Right %d"), hitInfo.hitCenter, hitInfo.hitLeft, hitInfo.hitRight));
	}

	return hitInfo.hitCenter || hitInfo.hitLeft || hitInfo.hitRight || hitInfo.hitTooClose;	
}

void ASDTAIController::avoidTheWall(APawn* pawn, HitInfoWall hitInfo, float deltaTime)
{	
	float acceleration;
	if (hitInfo.hitTooClose)
	{
		// Si on a un mur trop proche, on recule
		acceleration = m_speed_value > -0.2f ? -m_max_acceleration : 0.f;
	}
	else
	{
		// Reduction la vitesse jusqu'à 30% si on a un mur en face
		acceleration = hitInfo.hitCenter ? (m_speed_value > 0.3f ? -m_max_acceleration : 0.f) : m_max_acceleration/2;
	}

	// Calcul de la nouvelle direction
	FVector newDir = pawn->GetActorForwardVector();
	if (hitInfo.hitRight)
	{
		newDir = -pawn->GetActorRightVector();
	}
	else if (hitInfo.hitLeft && !hitInfo.hitRight)
	{
		newDir = pawn->GetActorRightVector();
	}
	Move(pawn, newDir,acceleration, deltaTime);
}