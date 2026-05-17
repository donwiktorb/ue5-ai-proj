#include "AI/FOEnemyCharacter.h"

#include "AI/FOEnemyAIController.h"
#include "Core/FOLogChannels.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Net/UnrealNetwork.h"

AFOEnemyCharacter::AFOEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AFOEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	TeamId = 1;
	AlertState = EFOAIAlertState::Passive;
	bIsAttackActive = false;
	CachedLifeState = EFOCharacterLifeState::Alive;
}

void AFOEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CachedLifeState = GetLifeState();
}

void AFOEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CachedLifeState != GetLifeState())
	{
		const EFOCharacterLifeState PreviousLifeState = CachedLifeState;
		CachedLifeState = GetLifeState();

		if (CachedLifeState == EFOCharacterLifeState::Dead && PreviousLifeState != EFOCharacterLifeState::Dead)
		{
			StopAttack();
			BP_OnEnemyDeathStarted();
		}
	}
}

void AFOEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFOEnemyCharacter, AlertState);
}

float AFOEnemyCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (AppliedDamage > 0.0f && IsAlive())
	{
		SetAlertState(EFOAIAlertState::Alerted);
	}

	return AppliedDamage;
}

void AFOEnemyCharacter::SetAlertState(EFOAIAlertState NewAlertState)
{
	if (AlertState == NewAlertState)
	{
		return;
	}

	const EFOAIAlertState OldAlertState = AlertState;
	AlertState = NewAlertState;

	const bool bShouldAim = AlertState == EFOAIAlertState::Combat;
	SetAiming(bShouldAim);

	if (!bShouldAim)
	{
		StopAttack();
	}

	UE_LOG(
		LogFalseOrders,
		Verbose,
		TEXT("%s alert state changed from %d to %d. Aiming=%s"),
		*GetName(),
		static_cast<uint8>(OldAlertState),
		static_cast<uint8>(AlertState),
		bShouldAim ? TEXT("true") : TEXT("false"));

	OnRep_AlertState();
}

void AFOEnemyCharacter::NotifyTargetSpotted(AActor* SeenActor)
{
	SetAlertState(EFOAIAlertState::Combat);
	BP_OnTargetSpotted(SeenActor);
}

void AFOEnemyCharacter::NotifyTargetLost(AActor* LostActor)
{
	if (IsAlive())
	{
		SetAlertState(EFOAIAlertState::Searching);
	}

	BP_OnTargetLost(LostActor);
}

void AFOEnemyCharacter::StartAttack()
{
	if (bIsAttackActive || !IsAlive())
	{
		return;
	}

	SetAiming(true);

	bIsAttackActive = true;
	StartWeaponFire();

	BP_OnAttackStarted();
}

void AFOEnemyCharacter::StopAttack()
{
	if (!bIsAttackActive)
	{
		return;
	}

	bIsAttackActive = false;
	StopWeaponFire();

	BP_OnAttackStopped();
}

void AFOEnemyCharacter::OnRep_AlertState()
{
	UE_LOG(LogFalseOrders, Verbose, TEXT("%s alert state changed to %d"), *GetName(), static_cast<uint8>(AlertState));
	BP_OnAlertStateChanged(AlertState);
}

