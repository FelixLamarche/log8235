
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SoftDesignTrainingMainCharacter.h"


ASDTAIController::ASDTAIController()
{
	m_speed_value = 0.f;
	m_max_speed = 0.7f;
	m_max_acceleration = 1.5;
	m_distance_vision = 400.0;
	m_angle_vision = 25.0;
}

void ASDTAIController::Tick(float deltaTime)
{
	APawn* pawn = GetPawn();
	UWorld* world = GetWorld();
	
	bool isPoweredUp = SDTUtils::IsPlayerPoweredUp(world);
	// Fin Debug
	HitInfoWall hitInfoW;

	DetectDeathFloor(pawn, m_distance_vision, world, true);
	
	// Detection d'un mur
	if (DetectWall(pawn, m_distance_vision, hitInfoW, true))
	{
		// Evitement du mur
		avoidTheWall(pawn, hitInfoW, deltaTime);
	}
	if (DetectPlayer(world, pawn, true))
	{
		// Si on ne detecte pas de mur et qu'il y a un joueur, on le poursuit
		Pursuite(world, pawn, deltaTime);
	}
	else
	{
		// Si on ne detecte pas de mur et qu'il n'y a pas de joueur, on accélère jusqu'à la vitesse max et on avance tout droit
		Move(pawn, pawn->GetActorForwardVector(),m_max_acceleration/2, deltaTime);
	}
}

void ASDTAIController::Move(APawn* pawn, FVector dir, float acceleration, float deltaTime, float rotFactor)
{
	// Calcul de la vitesse
	m_speed_value = FMath::Min(m_speed_value + deltaTime * acceleration, m_max_speed);
	// Débug
	// GEngine->AddOnScreenDebugMessage(-1, 0.16f, FColor::Red, FString::Printf(TEXT("Speed : %f"), m_speed_value));
	m_speed = pawn->GetActorForwardVector() * m_speed_value;

	// Déplacement
	pawn->AddMovementInput(pawn->GetActorForwardVector().GetSafeNormal(), m_speed_value);

	// Orientation progressive du personnage vers la direction du mouvement
	const FRotator current_rot = pawn->GetActorRotation();
	const FRotator result = FMath::Lerp(current_rot, dir.Rotation(), rotFactor);
	pawn->SetActorRotation(result);
}

bool ASDTAIController::DetectWall(APawn* pawn, float distance, HitInfoWall &hitInfo, bool debug)
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

bool ASDTAIController::DetectDeathFloor(APawn* pawn, float distance, UWorld* world, bool debug)
{
	FHitResult hitData;
	FCollisionObjectQueryParams objectQueryParams;
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_COLLECTIBLE);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_DEATH_OBJECT);
	queryParams.AddIgnoredActor(GetPawn());

	FVector sourcePoint = pawn->GetActorLocation();
	FVector targetPointCenter = sourcePoint + pawn->GetActorForwardVector() * distance + FVector::UpVector * -100.f;

	bool hasBeenHit = world->LineTraceSingleByObjectType(hitData, sourcePoint, targetPointCenter, objectQueryParams, queryParams);
	
	if (debug)
	{	
		FColor color = FColor::White;
		if (hasBeenHit && hitData.GetActor() != nullptr)
		{
			color = hitData.GetActor()->ActorHasTag(FName(TEXT("DeathFloor"))) ? FColor::White : FColor::Red;
		}
		DrawDebugLine(world, sourcePoint, targetPointCenter, color);
		DrawDebugPoint(world, hitData.ImpactPoint, 10.f, FColor::Red, false);
	}

	return false;
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
		// Reduction la vitesse jusqu'à 30% si on detecte un mur
		acceleration = m_speed_value > 0.3f ? -m_max_acceleration : 0.f;
	}

	//Debug
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("Acceleration : %f - Speed %f"), acceleration, m_speed_value));

	// Calcul de la nouvelle direction
	FVector newDir = pawn->GetActorForwardVector();
	if (hitInfo.hitRight)
	{
		newDir = -pawn->GetActorRightVector();
	}
	else if (hitInfo.hitLeft)
	{
		newDir = pawn->GetActorRightVector();
	}
	Move(pawn, newDir,acceleration, deltaTime);
}

bool ASDTAIController::DetectPlayer(UWorld* world,APawn* pawn, bool debug)
{
	ASoftDesignTrainingMainCharacter* player = GetPlayer(world);
	if (!player)
		return false;
	
	bool detected = false;

	// Si le joueur est dans le "champ de vision"
	if (FVector::Dist(pawn->GetActorLocation(), player->GetActorLocation()) < m_distance_vision)
	{
		// Raycast pour vérifier que le joueur n'est pas derrière un mur
		detected = !SDTUtils::Raycast(GetWorld(), pawn->GetActorLocation(), player->GetActorLocation());
		if (debug) {
			DrawDebugLine(GetWorld(), pawn->GetActorLocation(), player->GetActorLocation(), FColor::Yellow);
			DrawDebugSphere(GetWorld(), pawn->GetActorLocation(), m_distance_vision, 12, FColor::Yellow);
		}
		return detected;
	}

	// Si le joueur est hors de portée, on ne le détecte pas
	return detected;

}

void ASDTAIController::Pursuite(UWorld* world, APawn* pawn, float deltaTime)
{
	ASoftDesignTrainingMainCharacter* player = GetPlayer(world);
	if (!player)
		return;
	
	FVector playerPos = player->GetActorLocation();
	FVector pawnPos = pawn->GetActorLocation();

	if (playerPos == pawnPos)
	{
		return;
	}
	FVector dir = (playerPos - pawnPos);
	dir.Normalize(0.1f);
	bool isPoweredUp = Cast<ASoftDesignTrainingMainCharacter>(player)->IsPoweredUp();


	if (isPoweredUp)
	{
		dir = -dir;
	}
	// pawn->SetActorRotation(dir.Rotation());
	Move(pawn, dir, m_max_acceleration, deltaTime, 0.05f);
}

ASoftDesignTrainingMainCharacter * ASDTAIController::GetPlayer(UWorld* world)
{
	if (world != nullptr)
	{
		ACharacter* player_ = UGameplayStatics::GetPlayerCharacter(world, 0);
		if (!player_)
			return nullptr;

		ASoftDesignTrainingMainCharacter* player = Cast<ASoftDesignTrainingMainCharacter>(player_);
		if (!player)
			return nullptr;
		return player;
	}
	return nullptr;
}