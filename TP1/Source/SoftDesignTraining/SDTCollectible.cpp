// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTCollectible.h"
#include "SoftDesignTraining.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;

    m_AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    m_AudioComponent->SetupAttachment(RootComponent);
    if (m_PickUpSoundCue != nullptr)
    {
        m_AudioComponent->SetSound(m_PickUpSoundCue);
    }

    m_ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    m_ParticleSystemComponent->SetupAttachment(RootComponent);
    m_ParticleSystemComponent->bAutoActivate = false;
    m_ParticleSystemComponent->SetTemplate(m_ParticleSystem);
}

void ASDTCollectible::BeginPlay()
{
    Super::BeginPlay();
}

void ASDTCollectible::Collect()
{
    if (IsOnCooldown())
        return;

    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);
    // Play pick up soundeffect
    m_AudioComponent->Play();
    // Play VFX
    m_ParticleSystemComponent->Activate();
    GetStaticMeshComponent()->SetVisibility(false);
}

void ASDTCollectible::OnCooldownDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);
    m_ParticleSystemComponent->Deactivate();

    GetStaticMeshComponent()->SetVisibility(true);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}

void ASDTCollectible::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
}

void ASDTCollectible::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    // Check the property being changed
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ASDTCollectible, m_PickUpSoundCue))
    {
        m_AudioComponent->SetSound(m_PickUpSoundCue);
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASDTCollectible, m_ParticleSystem))
    {
        m_ParticleSystemComponent->SetTemplate(m_ParticleSystem);
    }
}
