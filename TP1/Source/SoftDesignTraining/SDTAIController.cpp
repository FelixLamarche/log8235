
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"


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

	if (pawn == nullptr || world == nullptr)
		return;
	
	bool isPoweredUp = SDTUtils::IsPlayerPoweredUp(world);
	// Fin Debug
	HitInfoWall hitInfoW;
	TArray<FOverlapResult> overlapInfo;
	ASDTCollectible* collectible;

	// Detection d'un death floor
	if (DetectDeathFloor(pawn, m_distance_vision, overlapInfo, world, true))
	{
		// Evitement du death floor
		AvoidDeathFloor(pawn, overlapInfo, deltaTime);
	}
	else if (DetectPlayer(world, pawn, true))
	{
		// Si on ne detecte pas de death floor et qu'il y a un joueur, on le poursuit
		Pursuite(world, pawn, deltaTime);
	}
	else if ( (collectible = DetectCollectible(pawn, m_distance_vision, overlapInfo, world, true)) != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Green, FString::Printf(TEXT("Collectible detected")));
		// Si on ne detecte pas de death floor et qu'il n'y a pas de joueur et qu'on detecte un bonus, on le ramasse
		MoveToCollectible(pawn, collectible, deltaTime);
		
	}
	else if (DetectWall(pawn, m_distance_vision, hitInfoW, true))
	{
		// Si on ne detecte pas de death floor, de joueur et de bonus et qu'on detecte un mur, on l'évite
		AvoidTheWall(pawn, hitInfoW, deltaTime);
	}
	else
	{
		// Si on ne detecte pas de death floor, de joueur, de bonus et de mur, on accélère jusqu'à la vitesse max et on avance tout droit
		Move(pawn, pawn->GetActorForwardVector(), m_max_acceleration/2, deltaTime);
	}
	/*{
		
		// Detection d'un mur
		if (DetectWall(pawn, m_distance_vision, hitInfoW, true))
		{
			// Evitement du mur
			AvoidTheWall(pawn, hitInfoW, deltaTime);
		}
		if (DetectPlayer(world, pawn, true))
		{
			// Si on ne detecte pas de mur et qu'il y a un joueur, on le poursuit
			Pursuite(world, pawn, deltaTime);
		}
		else
		{
			// Si on ne detecte pas de mur et qu'il n'y a pas de joueur, on accélère jusqu'à la vitesse max et on avance tout droit
			
		}
	} else
	{
		Move(pawn, pawn->GetActorForwardVector(),m_max_acceleration/2, deltaTime);
	}*/
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

bool ASDTAIController::DetectDeathFloor(APawn* pawn, float distance, TArray<FOverlapResult>& OverlapInfo, UWorld* world, bool debug)
{
	bool hasBeenHit = SpherOverlap(pawn->GetActorLocation(), distance, world, OverlapInfo, debug);
	
	bool deathFloorDetected = false;
	// Detection des death floors
	for (int32 i = 0; i < OverlapInfo.Num(); ++i)
	{
		if (OverlapInfo[i].GetActor() && OverlapInfo[i].GetActor()->ActorHasTag(FName(TEXT("DeathFloor"))))
		{
			
			// Si on detecte un death floor, on vérifie qu'il n'y a pas de mur entre le joueur et le death floor
			if (!SDTUtils::Raycast(world, pawn->GetActorLocation(), OverlapInfo[i].GetActor()->GetActorLocation()))
			{
				// On vérifie aussi que le death floor est bien devant le joueur
				if (isInFrontOf(pawn, OverlapInfo[i].GetActor()->GetActorLocation()))
				{
					deathFloorDetected = true;
				}
			}

			if (debug)
			{
				DrawDebugLine(world, pawn->GetActorLocation(), OverlapInfo[i].GetActor()->GetActorLocation(), FColor::White, false);
			}
		}
	}

	if (debug)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Green, FString::Printf(TEXT("Death floor detected : %d"), deathFloorDetected));
	}

	return deathFloorDetected;
}

ASDTCollectible* ASDTAIController::DetectCollectible(APawn* pawn, float distance, TArray<FOverlapResult>& OverlapInfo, UWorld* world, bool debug)
{
	bool hasBeenHit = SpherOverlap(pawn->GetActorLocation(), distance, world, OverlapInfo, debug);
	
	// Detection des bonus
	float minDistance = distance;
	ASDTCollectible* collectible = nullptr;
	for (int32 i = 0; i < OverlapInfo.Num(); ++i)
	{
		if (OverlapInfo[i].GetActor() && OverlapInfo[i].GetActor()->ActorHasTag(FName(TEXT("Collectible"))))
		{
			// Verification que le bonus est bien devant le joueur
			if (isInFrontOf(pawn, OverlapInfo[i].GetActor()->GetActorLocation()))
			{
				// Si on detecte un bonus, on vérifie qu'il n'y a pas de mur entre le joueur et le bonus et on retroune le bonus le plus pret
				if (!SDTUtils::Raycast(world, pawn->GetActorLocation(), OverlapInfo[i].GetActor()->GetActorLocation()))
				{
					// Si la distance est plus petite, on prend ce bonus
					if (FVector::Dist(pawn->GetActorLocation(), OverlapInfo[i].GetActor()->GetActorLocation()) < minDistance)
					{
						// si le bonus est bien disponible, on le selectionne
						if (!Cast<ASDTCollectible>(OverlapInfo[i].GetActor())->IsOnCooldown())
						{
							minDistance = FVector::Dist(pawn->GetActorLocation(), OverlapInfo[i].GetActor()->GetActorLocation());
							collectible = Cast<ASDTCollectible>(OverlapInfo[i].GetActor());
						}
					}
				}
			}

			if (debug)
			{
				DrawDebugLine(world, pawn->GetActorLocation(), OverlapInfo[i].GetActor()->GetActorLocation(), FColor::White, false);
			}
		}
	}

	return collectible;
}

void ASDTAIController::MoveToCollectible(APawn* pawn, ASDTCollectible* collectible, float deltaTime)
{
	FVector dir = (collectible->GetActorLocation() - pawn->GetActorLocation());
	dir.Normalize(0.1f);
	Move(pawn, dir, m_max_acceleration, deltaTime, 0.05f);
}

void ASDTAIController::AvoidTheWall(APawn* pawn, HitInfoWall hitInfo, float deltaTime)
{	
	float acceleration;
	if (hitInfo.hitTooClose)
	{
		// Si on a un mur trop proche, on recule
		acceleration = m_speed_value > -0.2f ? -m_max_acceleration : 0.f;
	}
	else if (hitInfo.hitCenter)
	{
		// Reduction la vitesse si on detecte un mur
		acceleration = m_speed_value > 0.45f ? -m_max_acceleration : 0.f;
	}
	else
	{
		// Reduction la vitesse si on detecte un mur
		acceleration = m_speed_value > m_max_speed ? -m_max_acceleration : 0.f;
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
	else if (hitInfo.hitTooClose)
	{
		// Si on a un mur trop proche et qu'auncun mur n'est détecté sur les cotés, on tourne à droite (par défaut)
		newDir = pawn->GetActorRightVector();
	}
	Move(pawn, newDir,acceleration, deltaTime);
}

void ASDTAIController::AvoidDeathFloor(APawn* pawn, TArray<FOverlapResult>& OverlapInfo, float deltaTime)
{
	// On récupère la position du death floor
	FVector deathFloorPos = FVector::ZeroVector;
	for (int32 i = 0; i < OverlapInfo.Num(); ++i)
	{
		if (OverlapInfo[i].GetActor() && OverlapInfo[i].GetActor()->ActorHasTag(FName(TEXT("DeathFloor"))))
		{
			deathFloorPos = OverlapInfo[i].GetActor()->GetActorLocation();
		}
	}

	if (deathFloorPos != FVector::ZeroVector)
	{
		// On se dirige à l'opposé du death floor
		FVector dir = (deathFloorPos - pawn->GetActorLocation());
		dir.Normalize(0.1f);
		dir = -dir;
		Move(pawn, dir, m_max_acceleration, deltaTime, 0.05f);
	}
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
	bool isPoweredUp = player->IsPoweredUp();

	if (playerPos == pawnPos)
	{
		return;
	}
	FVector dir = (playerPos - pawnPos);
	dir.Normalize(0.1f);
	
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

bool ASDTAIController::SpherOverlap(FVector position, float radius, UWorld* world, TArray<FOverlapResult>& outOverlaps, bool debug)
{
	FCollisionObjectQueryParams objectQueryParams;
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_COLLECTIBLE);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_DEATH_OBJECT);
	queryParams.AddIgnoredActor(GetPawn());

	bool hasBeenHit = world->OverlapMultiByObjectType(outOverlaps, position, FQuat::Identity, objectQueryParams, FCollisionShape::MakeSphere(radius), queryParams);

	if (debug)
	{
		FColor color = hasBeenHit ? FColor::Red : FColor::Green;
		DrawDebugSphere(world, position, radius, 12, color);
		// Affichage des acteurs touchés
		for (int32 i = 0; i < outOverlaps.Num(); ++i)
		{
			if (outOverlaps[i].GetComponent()){
				DrawDebugPrimitive(*(outOverlaps[i].GetComponent()), FColor::Purple);
			}
		}
	}

	return hasBeenHit;
}

void ASDTAIController::DrawDebugPrimitive(const UPrimitiveComponent& primitive, FColor color)
{
	FVector center = primitive.Bounds.Origin;
	FVector extent = primitive.Bounds.BoxExtent;
	UWorld * world = GetWorld();
	if (world == nullptr)
		return;
	DrawDebugBox(world, center, extent, color);
}

bool ASDTAIController::isInFrontOf(APawn* pawn, FVector targetPosition)
{
	FVector dir = (targetPosition - pawn->GetActorLocation()).GetSafeNormal();
	float angle = FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(dir, pawn->GetActorForwardVector()))));
	return angle < 90.f;
}