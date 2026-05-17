#include "Combat/FOWeaponBase.h"

#include "Character/FOCharacterBase.h"
#include "Core/FOLogChannels.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AFOWeaponBase::AFOWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(66.0f);
	SetMinNetUpdateFrequency(22.0f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	WeaponSlot = EFOWeaponSlot::Primary;
	WeaponAnimationType = EFOWeaponAnimationType::Rifle;
	AmmoInMagazine = 0;
	ReserveAmmo = 0;
	bIsEquipped = false;
	bIsReloading = false;
	bWantsToFire = false;
	LastFireTimeSeconds = -1000.0f;
}

void AFOWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeAmmo();
	}
}

void AFOWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFOWeaponBase, OwningCharacter);
	DOREPLIFETIME(AFOWeaponBase, WeaponSlot);
	DOREPLIFETIME(AFOWeaponBase, AmmoInMagazine);
	DOREPLIFETIME(AFOWeaponBase, ReserveAmmo);
	DOREPLIFETIME(AFOWeaponBase, bIsEquipped);
	DOREPLIFETIME(AFOWeaponBase, bIsReloading);
}

void AFOWeaponBase::StartFiring()
{
	if (!HasAuthority())
	{
		return;
	}

	bWantsToFire = true;

	if (!CanFire())
	{
		return;
	}

	HandleFireShot();

	if (FireConfig.FireMode == EFOWeaponFireMode::FullAutomatic && CanFire())
	{
		GetWorldTimerManager().SetTimer(
			AutomaticFireTimerHandle,
			this,
			&AFOWeaponBase::HandleAutomaticFireTick,
			GetFireInterval(),
			true,
			GetFireInterval());
	}
}

void AFOWeaponBase::StopFiring()
{
	bWantsToFire = false;
	GetWorldTimerManager().ClearTimer(AutomaticFireTimerHandle);
}

void AFOWeaponBase::StartReload()
{
	if (!HasAuthority() || !CanReload())
	{
		return;
	}

	StopFiring();
	bIsReloading = true;
	OnRep_Reloading();

	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AFOWeaponBase::FinishReload,
		FireConfig.ReloadDuration,
		false);
}

void AFOWeaponBase::InterruptReload()
{
	if (!HasAuthority() || !bIsReloading)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	bIsReloading = false;
	OnRep_Reloading();
}

void AFOWeaponBase::SetOwningCharacter(AFOCharacterBase* NewOwningCharacter)
{
	if (!HasAuthority())
	{
		return;
	}

	OwningCharacter = NewOwningCharacter;
	SetOwner(NewOwningCharacter);
	SetInstigator(NewOwningCharacter);
	OnRep_OwningCharacter();
}

void AFOWeaponBase::SetWeaponSlot(EFOWeaponSlot NewWeaponSlot)
{
	WeaponSlot = NewWeaponSlot;
}

void AFOWeaponBase::SetEquipped(bool bNewEquipped)
{
	if (bIsEquipped == bNewEquipped)
	{
		return;
	}

	bIsEquipped = bNewEquipped;
	OnRep_Equipped();
}

bool AFOWeaponBase::CanFire() const
{
	if (OwningCharacter == nullptr)
	{
		return false;
	}

	if (!bIsEquipped || bIsReloading || AmmoInMagazine <= 0)
	{
		return false;
	}

	return OwningCharacter->IsAlive();
}

bool AFOWeaponBase::CanReload() const
{
	if (OwningCharacter == nullptr)
	{
		return false;
	}

	if (!bIsEquipped || bIsReloading || !OwningCharacter->IsAlive())
	{
		return false;
	}

	if (AmmoInMagazine >= AmmoConfig.MagazineSize)
	{
		return false;
	}

	return AmmoConfig.bUnlimitedReserveAmmo || ReserveAmmo > 0;
}

float AFOWeaponBase::GetFireInterval() const
{
	return FireConfig.RoundsPerMinute > 0.0f
		? 60.0f / FireConfig.RoundsPerMinute
		: 0.1f;
}

float AFOWeaponBase::GetCurrentSpreadAngleDegrees() const
{
	float SpreadAngle = OwningCharacter && OwningCharacter->IsAiming()
		? SpreadConfig.AimingSpreadAngleDegrees
		: SpreadConfig.HipFireSpreadAngleDegrees;

	if (OwningCharacter && OwningCharacter->GetMovementState() != EFOMovementState::Idle)
	{
		SpreadAngle *= SpreadConfig.MovementSpreadMultiplier;
	}

	return SpreadAngle;
}

void AFOWeaponBase::InitializeAmmo()
{
	if (AmmoInMagazine <= 0)
	{
		AmmoInMagazine = AmmoConfig.MagazineSize;
	}

	if (ReserveAmmo <= 0)
	{
		ReserveAmmo = AmmoConfig.InitialReserveAmmo;
	}
}

void AFOWeaponBase::HandleAutomaticFireTick()
{
	if (!bWantsToFire || !CanFire())
	{
		StopFiring();
		return;
	}

	HandleFireShot();
}

void AFOWeaponBase::HandleFireShot()
{
	if (!CanFire() || OwningCharacter == nullptr)
	{
		return;
	}

	const float CurrentTimeSeconds = GetWorld()->GetTimeSeconds();
	if ((CurrentTimeSeconds - LastFireTimeSeconds) + KINDA_SMALL_NUMBER < GetFireInterval())
	{
		return;
	}

	FVector TraceStart = OwningCharacter->GetPawnViewLocation();
	FRotator AimRotation = OwningCharacter->GetBaseAimRotation();

	const float SpreadHalfAngleRadians = FMath::DegreesToRadians(GetCurrentSpreadAngleDegrees());
	const FVector ShotDirection = FMath::VRandCone(AimRotation.Vector(), SpreadHalfAngleRadians);
	const FVector TraceEnd = TraceStart + ShotDirection * HitscanConfig.TraceDistance;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(FalseOrdersHitscanTrace), false, OwningCharacter);
	TraceParams.AddIgnoredActor(this);

	FHitResult HitResult;
	const bool bHitSomething = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		HitscanConfig.TraceChannel,
		TraceParams);

	const FVector FinalTraceEnd = bHitSomething ? HitResult.ImpactPoint : TraceEnd;
	AController* InstigatorController = OwningCharacter->GetController();

	if (bHitSomething && HitResult.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			HitscanConfig.Damage,
			ShotDirection,
			HitResult,
			InstigatorController,
			this,
			nullptr);
	}

	AmmoInMagazine = FMath::Max(AmmoInMagazine - 1, 0);
	LastFireTimeSeconds = CurrentTimeSeconds;
	NotifyAmmoChanged();
	MulticastPlayFireCosmetics(FinalTraceEnd, bHitSomething);

	if (AmmoInMagazine <= 0)
	{
		StopFiring();
	}
}

void AFOWeaponBase::FinishReload()
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 MissingAmmo = FMath::Max(AmmoConfig.MagazineSize - AmmoInMagazine, 0);
	int32 AmmoToTransfer = MissingAmmo;

	if (!AmmoConfig.bUnlimitedReserveAmmo)
	{
		AmmoToTransfer = FMath::Min(ReserveAmmo, MissingAmmo);
		ReserveAmmo -= AmmoToTransfer;
	}

	AmmoInMagazine += AmmoToTransfer;
	bIsReloading = false;

	NotifyAmmoChanged();
	OnRep_Reloading();
}

void AFOWeaponBase::NotifyAmmoChanged()
{
	OnAmmoChanged.Broadcast(AmmoInMagazine, ReserveAmmo);
	BP_OnAmmoChanged(AmmoInMagazine, ReserveAmmo);
}

void AFOWeaponBase::BroadcastReloadStarted()
{
	BP_OnReloadStarted();

	if (OwningCharacter)
	{
		OwningCharacter->NotifyWeaponReloadStarted(this);
	}
}

void AFOWeaponBase::BroadcastReloadFinished()
{
	BP_OnReloadFinished();

	if (OwningCharacter)
	{
		OwningCharacter->NotifyWeaponReloadFinished(this);
	}
}

void AFOWeaponBase::ApplyEquippedPresentation()
{
	SetActorEnableCollision(bIsEquipped);
}

void AFOWeaponBase::MulticastPlayFireCosmetics_Implementation(FVector_NetQuantize TraceEnd, bool bHitSomething)
{
	BP_OnWeaponFired(TraceEnd, bHitSomething);

	if (OwningCharacter)
	{
		OwningCharacter->NotifyWeaponFired(this, TraceEnd);
	}
}

void AFOWeaponBase::OnRep_OwningCharacter()
{
}

void AFOWeaponBase::OnRep_AmmoInMagazine()
{
	NotifyAmmoChanged();
}

void AFOWeaponBase::OnRep_ReserveAmmo()
{
	NotifyAmmoChanged();
}

void AFOWeaponBase::OnRep_Equipped()
{
	ApplyEquippedPresentation();
	BP_OnEquippedChanged(bIsEquipped);
}

void AFOWeaponBase::OnRep_Reloading()
{
	if (bIsReloading)
	{
		BroadcastReloadStarted();
	}
	else
	{
		BroadcastReloadFinished();
	}
}
