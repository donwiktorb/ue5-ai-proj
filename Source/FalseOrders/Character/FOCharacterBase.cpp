#include "Character/FOCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "Combat/FOWeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Core/FOLogChannels.h"
#include "Core/FOPlayerController.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AFOCharacterBase::AFOCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(100.0f);
	SetMinNetUpdateFrequency(33.0f);

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
	ThirdPersonSpringArm->SetupAttachment(GetCapsuleComponent());
	ThirdPersonSpringArm->TargetArmLength = 280.0f;
	ThirdPersonSpringArm->bUsePawnControlRotation = true;
	ThirdPersonSpringArm->bEnableCameraLag = true;
	ThirdPersonSpringArm->CameraLagSpeed = 12.0f;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonSpringArm, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->bUsePawnControlRotation = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	WalkSpeed = 260.0f;
	SprintSpeed = 460.0f;
	CrouchSpeed = 180.0f;
	AimingWalkSpeed = 210.0f;
	DownedLookRateMultiplier = 0.4f;
	ThirdPersonArmLength = 280.0f;
	FirstPersonCameraOffset = FVector(18.0f, 0.0f, 64.0f);
	FirstPersonHiddenBones = { TEXT("head") };
	EquippedWeaponSocketName = TEXT("WeaponSocket");
	PrimaryHolsterSocketName = TEXT("PrimaryHolsterSocket");
	SecondaryHolsterSocketName = TEXT("SecondaryHolsterSocket");

	MaxHealth = 100.0f;
	Health = MaxHealth;
	bCanBeDowned = true;
	BleedoutDuration = 20.0f;
	DownedStartServerWorldTime = 0.0f;

	CharacterStance = EFOCharacterStance::Standing;
	MovementState = EFOMovementState::Idle;
	ViewMode = EFOViewMode::ThirdPerson;
	LifeState = EFOCharacterLifeState::Alive;
	bIsAiming = false;
	bIsSprinting = false;
	CurrentSpeed = 0.0f;
	MovementInputAmount = 0.0f;
	bHasBeenDownedThisLife = false;
	PrimaryWeapon = nullptr;
	SecondaryWeapon = nullptr;
	CurrentWeapon = nullptr;
}

void AFOCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
	ThirdPersonSpringArm->TargetArmLength = ThirdPersonArmLength;
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);

	ApplyViewModeInternal();
	ApplyLifeStateMovementRestrictions();
	RefreshMovementTuning();
	ApplyInputMappingContext();

	if (HasAuthority())
	{
		SpawnDefaultWeapons();
	}
}

void AFOCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ApplyInputMappingContext();
}

void AFOCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	ApplyInputMappingContext();
}

void AFOCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFOCharacterBase::Move);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFOCharacterBase::Look);
		}

		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputStartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFOCharacterBase::InputStopSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AFOCharacterBase::InputStopSprint);
		}

		if (CrouchAction)
		{
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputToggleCrouch);
		}

		if (AimAction)
		{
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputStartAim);
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AFOCharacterBase::InputStopAim);
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &AFOCharacterBase::InputStopAim);
		}

		if (TogglePerspectiveAction)
		{
			EnhancedInput->BindAction(TogglePerspectiveAction, ETriggerEvent::Started, this, &AFOCharacterBase::ToggleViewMode);
		}

		if (FireAction)
		{
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputStartFire);
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AFOCharacterBase::InputStopFire);
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Canceled, this, &AFOCharacterBase::InputStopFire);
		}

		if (ReloadAction)
		{
			EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputReload);
		}

		if (EquipPrimaryWeaponAction)
		{
			EnhancedInput->BindAction(EquipPrimaryWeaponAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputEquipPrimaryWeapon);
		}

		if (EquipSecondaryWeaponAction)
		{
			EnhancedInput->BindAction(EquipSecondaryWeaponAction, ETriggerEvent::Started, this, &AFOCharacterBase::InputEquipSecondaryWeapon);
		}
	}
}

void AFOCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAnimationState();

	if (HasAuthority())
	{
		UpdateMovementState();

		if (LifeState == EFOCharacterLifeState::Downed && BleedoutDuration > 0.0f && GetBleedoutTimeRemaining() <= 0.0f)
		{
			HandleDeath();
		}
	}
}

void AFOCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFOCharacterBase, CharacterStance);
	DOREPLIFETIME(AFOCharacterBase, MovementState);
	DOREPLIFETIME(AFOCharacterBase, ViewMode);
	DOREPLIFETIME(AFOCharacterBase, LifeState);
	DOREPLIFETIME(AFOCharacterBase, bIsAiming);
	DOREPLIFETIME(AFOCharacterBase, bIsSprinting);
	DOREPLIFETIME(AFOCharacterBase, Health);
	DOREPLIFETIME(AFOCharacterBase, DownedStartServerWorldTime);
	DOREPLIFETIME(AFOCharacterBase, PrimaryWeapon);
	DOREPLIFETIME(AFOCharacterBase, SecondaryWeapon);
	DOREPLIFETIME(AFOCharacterBase, CurrentWeapon);
}

float AFOCharacterBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return ApplyDamageAmount(DamageAmount, DamageCauser);
}

void AFOCharacterBase::ToggleViewMode()
{
	const EFOViewMode NextViewMode = ViewMode == EFOViewMode::ThirdPerson
		? EFOViewMode::FirstPerson
		: EFOViewMode::ThirdPerson;

	SetViewMode(NextViewMode);
}

void AFOCharacterBase::SetViewMode(EFOViewMode NewViewMode)
{
	if (ViewMode == NewViewMode)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetViewMode(NewViewMode);
	}

	ViewMode = NewViewMode;
	OnRep_ViewMode();
}

void AFOCharacterBase::SetAiming(bool bNewAiming)
{
	if (!CanAim())
	{
		return;
	}

	if (bIsAiming == bNewAiming)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetAiming(bNewAiming);
	}

	bIsAiming = bNewAiming;

	if (bIsAiming)
	{
		bIsSprinting = false;
	}

	OnRep_Aiming();
	OnRep_Sprinting();
	RefreshMovementTuning();
}

void AFOCharacterBase::SetSprinting(bool bNewSprinting)
{
	const bool bResolvedSprinting = bNewSprinting && CanSprint();

	if (bIsSprinting == bResolvedSprinting)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetSprinting(bResolvedSprinting);
	}

	bIsSprinting = bResolvedSprinting;
	OnRep_Sprinting();
	RefreshMovementTuning();
}

void AFOCharacterBase::SetCharacterStance(EFOCharacterStance NewStance)
{
	if (!IsAlive())
	{
		return;
	}

	if (CharacterStance == NewStance)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetCharacterStance(NewStance);
	}

	CharacterStance = NewStance;

	if (CharacterStance == EFOCharacterStance::Crouching)
	{
		Crouch();
		bIsSprinting = false;
	}
	else
	{
		UnCrouch();
	}

	OnRep_CharacterStance();
	OnRep_Sprinting();
	RefreshMovementTuning();
}

float AFOCharacterBase::ApplyDamageAmount(float DamageAmount, AActor* DamageInstigatorActor)
{
	if (!HasAuthority() || IsDead())
	{
		return 0.0f;
	}

	const float ClampedDamage = FMath::Max(0.0f, DamageAmount);
	if (ClampedDamage <= 0.0f)
	{
		return 0.0f;
	}

	if (LifeState == EFOCharacterLifeState::Downed)
	{
		UE_LOG(LogFalseOrders, Verbose, TEXT("%s took damage while downed and will die"), *GetName());
		HandleDeath();
		return ClampedDamage;
	}

	const float PreviousHealth = Health;
	const float NewHealth = FMath::Clamp(Health - ClampedDamage, 0.0f, MaxHealth);
	Health = NewHealth;

	UE_LOG(LogFalseOrders, Verbose, TEXT("%s received %.2f damage from %s"), *GetName(), ClampedDamage, *GetNameSafe(DamageInstigatorActor));

	HandleHealthChanged(PreviousHealth);

	if (Health <= 0.0f)
	{
		if (bCanBeDowned && !bHasBeenDownedThisLife)
		{
			EnterDownedState();
		}
		else
		{
			HandleDeath();
		}
	}

	return PreviousHealth - Health;
}

bool AFOCharacterBase::IsAlive() const
{
	return LifeState == EFOCharacterLifeState::Alive;
}

bool AFOCharacterBase::IsDowned() const
{
	return LifeState == EFOCharacterLifeState::Downed;
}

bool AFOCharacterBase::IsDead() const
{
	return LifeState == EFOCharacterLifeState::Dead;
}

bool AFOCharacterBase::CanSprint() const
{
	return IsAlive() && CharacterStance == EFOCharacterStance::Standing && !bIsAiming;
}

bool AFOCharacterBase::CanAim() const
{
	return IsAlive();
}

bool AFOCharacterBase::CanUseCrouch() const
{
	return IsAlive();
}

bool AFOCharacterBase::CanBeRevived() const
{
	return IsDowned();
}

float AFOCharacterBase::GetBleedoutTimeRemaining() const
{
	if (!IsDowned())
	{
		return 0.0f;
	}

	if (const AGameStateBase* WorldGameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		const float ServerTime = WorldGameState->GetServerWorldTimeSeconds();
		return FMath::Max(0.0f, BleedoutDuration - (ServerTime - DownedStartServerWorldTime));
	}

	return BleedoutDuration;
}

bool AFOCharacterBase::CanFireWeapon() const
{
	return IsAlive() && CurrentWeapon && CurrentWeapon->CanFire();
}

bool AFOCharacterBase::CanReloadWeapon() const
{
	return IsAlive() && CurrentWeapon && CurrentWeapon->CanReload();
}

EFOWeaponAnimationType AFOCharacterBase::GetEquippedWeaponAnimationType() const
{
	return CurrentWeapon
		? CurrentWeapon->GetWeaponAnimationType()
		: EFOWeaponAnimationType::None;
}

void AFOCharacterBase::EquipWeaponSlot(EFOWeaponSlot DesiredSlot)
{
	if (!IsAlive())
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerEquipWeaponSlot(DesiredSlot);
		return;
	}

	SetCurrentWeapon(GetWeaponForSlot(DesiredSlot));
}

void AFOCharacterBase::NotifyWeaponFired(AFOWeaponBase* FiredWeapon, FVector TraceEnd)
{
	BP_OnWeaponFired(FiredWeapon, TraceEnd);
}

void AFOCharacterBase::NotifyWeaponReloadStarted(AFOWeaponBase* ReloadingWeapon)
{
	BP_OnWeaponReloadStarted(ReloadingWeapon);
}

void AFOCharacterBase::NotifyWeaponReloadFinished(AFOWeaponBase* ReloadingWeapon)
{
	BP_OnWeaponReloadFinished(ReloadingWeapon);
}

void AFOCharacterBase::ApplyInputMappingContext() const
{
	if (AFOPlayerController* FOPlayerController = Cast<AFOPlayerController>(Controller))
	{
		FOPlayerController->SetDefaultInputMappingContext(DefaultInputMappingContext);
	}
}

void AFOCharacterBase::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr || !IsAlive())
	{
		return;
	}

	const FVector2D MoveVector = Value.Get<FVector2D>();
	const FRotator ControlRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveVector.Y);
	AddMovementInput(RightDirection, MoveVector.X);
}

void AFOCharacterBase::Look(const FInputActionValue& Value)
{
	if (Controller == nullptr || IsDead())
	{
		return;
	}

	const FVector2D LookAxis = Value.Get<FVector2D>();
	const float LookScale = IsDowned() ? DownedLookRateMultiplier : 1.0f;
	AddControllerYawInput(LookAxis.X * LookScale);
	AddControllerPitchInput(LookAxis.Y * LookScale);
}

void AFOCharacterBase::InputStartSprint()
{
	SetSprinting(true);
}

void AFOCharacterBase::InputStopSprint()
{
	SetSprinting(false);
}

void AFOCharacterBase::InputToggleCrouch()
{
	if (!CanUseCrouch())
	{
		return;
	}

	const EFOCharacterStance NextStance = CharacterStance == EFOCharacterStance::Standing
		? EFOCharacterStance::Crouching
		: EFOCharacterStance::Standing;

	SetCharacterStance(NextStance);
}

void AFOCharacterBase::InputStartAim()
{
	SetAiming(true);
}

void AFOCharacterBase::InputStopAim()
{
	SetAiming(false);
}

void AFOCharacterBase::InputStartFire()
{
	StartWeaponFire();
}

void AFOCharacterBase::InputStopFire()
{
	StopWeaponFire();
}

void AFOCharacterBase::InputReload()
{
	ReloadCurrentWeapon();
}

void AFOCharacterBase::InputEquipPrimaryWeapon()
{
	EquipWeaponSlot(EFOWeaponSlot::Primary);
}

void AFOCharacterBase::InputEquipSecondaryWeapon()
{
	EquipWeaponSlot(EFOWeaponSlot::Secondary);
}

void AFOCharacterBase::UpdateAnimationState()
{
	CurrentSpeed = GetVelocity().Size2D();

	if (const UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
	{
		const float MaxAcceleration = FMath::Max(1.0f, MoveComponent->GetMaxAcceleration());
		MovementInputAmount = FMath::Clamp(MoveComponent->GetCurrentAcceleration().Size2D() / MaxAcceleration, 0.0f, 1.0f);
	}
	else
	{
		MovementInputAmount = 0.0f;
	}
}

void AFOCharacterBase::UpdateMovementState()
{
	const float Speed2D = GetVelocity().Size2D();
	EFOMovementState NewMovementState = EFOMovementState::Idle;

	if (IsAlive() && Speed2D > KINDA_SMALL_NUMBER)
	{
		NewMovementState = bIsSprinting ? EFOMovementState::Sprinting : EFOMovementState::Walking;
	}

	SetMovementState(NewMovementState);
}

void AFOCharacterBase::RefreshMovementTuning()
{
	if (UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
	{
		float TargetSpeed = 0.0f;

		if (IsAlive())
		{
			TargetSpeed = WalkSpeed;

			if (CharacterStance == EFOCharacterStance::Crouching)
			{
				TargetSpeed = CrouchSpeed;
			}
			else if (bIsSprinting)
			{
				TargetSpeed = SprintSpeed;
			}
			else if (bIsAiming)
			{
				TargetSpeed = AimingWalkSpeed;
			}
		}

		MoveComponent->MaxWalkSpeed = TargetSpeed;
		MoveComponent->MaxWalkSpeedCrouched = CrouchSpeed;
		MoveComponent->bOrientRotationToMovement = IsAlive() && !bIsAiming && ViewMode == EFOViewMode::ThirdPerson;
	}

	bUseControllerRotationYaw = !IsDead() && (bIsAiming || ViewMode == EFOViewMode::FirstPerson || IsDowned());
}

void AFOCharacterBase::ApplyLifeStateMovementRestrictions()
{
	if (UCharacterMovementComponent* MoveComponent = GetCharacterMovement())
	{
		if (IsAlive())
		{
			if (MoveComponent->MovementMode == MOVE_None)
			{
				MoveComponent->SetMovementMode(MOVE_Walking);
			}
		}
		else
		{
			MoveComponent->StopMovementImmediately();
			MoveComponent->DisableMovement();
		}
	}

	RefreshMovementTuning();

	if (!IsAlive() && CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
		CurrentWeapon->InterruptReload();
	}
}

void AFOCharacterBase::SpawnDefaultWeapons()
{
	if (PrimaryWeapon == nullptr)
	{
		PrimaryWeapon = SpawnDefaultWeaponForSlot(DefaultPrimaryWeaponClass, EFOWeaponSlot::Primary);
	}

	if (SecondaryWeapon == nullptr)
	{
		SecondaryWeapon = SpawnDefaultWeaponForSlot(DefaultSecondaryWeaponClass, EFOWeaponSlot::Secondary);
	}

	if (CurrentWeapon == nullptr)
	{
		if (PrimaryWeapon)
		{
			SetCurrentWeapon(PrimaryWeapon);
		}
		else if (SecondaryWeapon)
		{
			SetCurrentWeapon(SecondaryWeapon);
		}
		else
		{
			RefreshWeaponAttachments();
		}
	}
	else
	{
		RefreshWeaponAttachments();
	}
}

AFOWeaponBase* AFOCharacterBase::SpawnDefaultWeaponForSlot(TSubclassOf<AFOWeaponBase> WeaponClass, EFOWeaponSlot WeaponSlot)
{
	if (!HasAuthority() || WeaponClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFOWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AFOWeaponBase>(WeaponClass, FTransform::Identity, SpawnParameters);
	if (SpawnedWeapon)
	{
		SpawnedWeapon->SetOwningCharacter(this);
		SpawnedWeapon->SetWeaponSlot(WeaponSlot);
		SpawnedWeapon->SetEquipped(false);
	}

	return SpawnedWeapon;
}

AFOWeaponBase* AFOCharacterBase::GetWeaponForSlot(EFOWeaponSlot WeaponSlot) const
{
	switch (WeaponSlot)
	{
	case EFOWeaponSlot::Primary:
		return PrimaryWeapon;

	case EFOWeaponSlot::Secondary:
		return SecondaryWeapon;

	default:
		return nullptr;
	}
}

void AFOCharacterBase::SetCurrentWeapon(AFOWeaponBase* NewWeapon)
{
	if (CurrentWeapon == NewWeapon)
	{
		return;
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
		CurrentWeapon->InterruptReload();
		CurrentWeapon->SetEquipped(false);
	}

	CurrentWeapon = NewWeapon;

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeapon->SetEquipped(true);
	}

	OnRep_CurrentWeapon();
}

void AFOCharacterBase::RefreshWeaponAttachments()
{
	if (PrimaryWeapon)
	{
		UpdateWeaponAttachment(PrimaryWeapon, PrimaryWeapon == CurrentWeapon);
	}

	if (SecondaryWeapon)
	{
		UpdateWeaponAttachment(SecondaryWeapon, SecondaryWeapon == CurrentWeapon);
	}
}

void AFOCharacterBase::UpdateWeaponAttachment(AFOWeaponBase* WeaponToUpdate, bool bIsEquippedWeapon) const
{
	if (WeaponToUpdate == nullptr)
	{
		return;
	}

	const FName DesiredSocketName = bIsEquippedWeapon
		? EquippedWeaponSocketName
		: GetHolsterSocketNameForWeapon(WeaponToUpdate);

	const bool bAttachedToDesiredSocket = TryAttachWeaponToSocket(WeaponToUpdate, DesiredSocketName);
	if (!bAttachedToDesiredSocket)
	{
		AttachWeaponFallback(WeaponToUpdate);
	}

	const bool bShouldBeVisible = bIsEquippedWeapon || bAttachedToDesiredSocket;
	ApplyWeaponVisibility(WeaponToUpdate, bShouldBeVisible);
}

bool AFOCharacterBase::TryAttachWeaponToSocket(AFOWeaponBase* WeaponToAttach, FName SocketName) const
{
	if (WeaponToAttach == nullptr)
	{
		return false;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!IsValid(CharacterMesh))
	{
		return false;
	}

	if (!IsValid(CharacterMesh->GetSkeletalMeshAsset()))
	{
		USceneComponent* WeaponRoot = WeaponToAttach->GetRootComponent();
		if (WeaponRoot && WeaponRoot->GetAttachParent() != GetRootComponent())
		{
			UE_LOG(
				LogFalseOrders,
				Log,
				TEXT("%s has no skeletal mesh asset assigned. Using fallback weapon attachment for %s."),
				*GetName(),
				*GetNameSafe(WeaponToAttach));
		}
		return false;
	}

	if (SocketName.IsNone() || !CharacterMesh->DoesSocketExist(SocketName))
	{
		USceneComponent* WeaponRoot = WeaponToAttach->GetRootComponent();
		const bool bAlreadyUsingFallback =
			WeaponRoot &&
			WeaponRoot->GetAttachParent() == GetRootComponent() &&
			WeaponRoot->GetAttachSocketName().IsNone();

		if (!bAlreadyUsingFallback)
		{
			UE_LOG(
				LogFalseOrders,
				Log,
				TEXT("%s is missing socket %s on mesh %s. Using fallback weapon attachment for %s."),
				*GetName(),
				*SocketName.ToString(),
				*GetNameSafe(CharacterMesh->GetSkeletalMeshAsset()),
				*GetNameSafe(WeaponToAttach));
		}
		return false;
	}

	USceneComponent* WeaponRoot = WeaponToAttach->GetRootComponent();
	const bool bAlreadyAttachedToSocket =
		WeaponRoot &&
		WeaponRoot->GetAttachParent() == CharacterMesh &&
		WeaponRoot->GetAttachSocketName() == SocketName;

	if (!bAlreadyAttachedToSocket)
	{
		WeaponToAttach->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

		UE_LOG(
			LogFalseOrders,
			Log,
			TEXT("%s attached %s to socket %s."),
			*GetName(),
			*GetNameSafe(WeaponToAttach),
			*SocketName.ToString());
	}

	return true;
}

void AFOCharacterBase::AttachWeaponFallback(AFOWeaponBase* WeaponToAttach) const
{
	if (WeaponToAttach == nullptr || GetRootComponent() == nullptr)
	{
		return;
	}

	USceneComponent* WeaponRoot = WeaponToAttach->GetRootComponent();
	const bool bAlreadyUsingFallback =
		WeaponRoot &&
		WeaponRoot->GetAttachParent() == GetRootComponent() &&
		WeaponRoot->GetAttachSocketName().IsNone();

	if (!bAlreadyUsingFallback)
	{
		WeaponToAttach->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		UE_LOG(
			LogFalseOrders,
			Log,
			TEXT("%s attached %s to root component as a temporary fallback."),
			*GetName(),
			*GetNameSafe(WeaponToAttach));
	}
}

void AFOCharacterBase::ApplyWeaponVisibility(AFOWeaponBase* WeaponToUpdate, bool bShouldBeVisible) const
{
	if (WeaponToUpdate == nullptr)
	{
		return;
	}

	if (WeaponToUpdate->IsHidden() != !bShouldBeVisible)
	{
		WeaponToUpdate->SetActorHiddenInGame(!bShouldBeVisible);

		UE_LOG(
			LogFalseOrders,
			Verbose,
			TEXT("%s set %s visibility to %s."),
			*GetName(),
			*GetNameSafe(WeaponToUpdate),
			bShouldBeVisible ? TEXT("visible") : TEXT("hidden"));
	}
}

FName AFOCharacterBase::GetHolsterSocketNameForWeapon(const AFOWeaponBase* Weapon) const
{
	if (Weapon && Weapon->GetWeaponSlot() == EFOWeaponSlot::Secondary)
	{
		return SecondaryHolsterSocketName;
	}

	return PrimaryHolsterSocketName;
}

void AFOCharacterBase::StartWeaponFire()
{
	if (!CanFireWeapon())
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerStartWeaponFire();
		return;
	}

	CurrentWeapon->StartFiring();
}

void AFOCharacterBase::StopWeaponFire()
{
	if (CurrentWeapon == nullptr)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerStopWeaponFire();
		return;
	}

	CurrentWeapon->StopFiring();
}

void AFOCharacterBase::ReloadCurrentWeapon()
{
	if (!CanReloadWeapon())
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerReloadCurrentWeapon();
		return;
	}

	CurrentWeapon->StartReload();
}

void AFOCharacterBase::HandleHealthChanged(float OldHealth)
{
	OnHealthChanged.Broadcast(OldHealth, Health, MaxHealth);
	BP_OnHealthChanged(OldHealth, Health, MaxHealth);
}

void AFOCharacterBase::SetMovementState(EFOMovementState NewMovementState)
{
	if (MovementState == NewMovementState)
	{
		return;
	}

	MovementState = NewMovementState;
	OnRep_MovementState();
}

void AFOCharacterBase::EnterDownedState()
{
	if (LifeState == EFOCharacterLifeState::Downed || LifeState == EFOCharacterLifeState::Dead)
	{
		return;
	}

	const EFOCharacterLifeState PreviousLifeState = LifeState;

	LifeState = EFOCharacterLifeState::Downed;
	bHasBeenDownedThisLife = true;
	bIsAiming = false;
	bIsSprinting = false;
	CharacterStance = EFOCharacterStance::Standing;
	SetMovementState(EFOMovementState::Idle);

	if (const AGameStateBase* WorldGameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		DownedStartServerWorldTime = WorldGameState->GetServerWorldTimeSeconds();
	}
	else
	{
		DownedStartServerWorldTime = 0.0f;
	}

	OnRep_CharacterStance();
	OnRep_Aiming();
	OnRep_Sprinting();
	OnRep_LifeState(PreviousLifeState);
}

void AFOCharacterBase::ApplyViewModeInternal()
{
	if (ThirdPersonSpringArm)
	{
		ThirdPersonSpringArm->TargetArmLength = ThirdPersonArmLength;
	}

	const bool bFirstPerson = ViewMode == EFOViewMode::FirstPerson;

	if (IsLocallyControlled())
	{
		FirstPersonCamera->SetActive(bFirstPerson);
		ThirdPersonCamera->SetActive(!bFirstPerson);
	}
	else
	{
		FirstPersonCamera->SetActive(false);
		ThirdPersonCamera->SetActive(false);
	}

	UpdateFirstPersonMeshVisibility();
	RefreshMovementTuning();
	OnViewModeChanged.Broadcast(ViewMode);
	BP_OnViewModeChanged(ViewMode);
}

void AFOCharacterBase::UpdateFirstPersonMeshVisibility()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!IsValid(CharacterMesh))
	{
		return;
	}

	const bool bHideHeadForLocalFirstPerson = IsLocallyControlled() && ViewMode == EFOViewMode::FirstPerson;

	for (const FName& BoneName : FirstPersonHiddenBones)
	{
		if (BoneName.IsNone())
		{
			continue;
		}

		if (bHideHeadForLocalFirstPerson)
		{
			CharacterMesh->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);
		}
		else
		{
			CharacterMesh->UnHideBoneByName(BoneName);
		}
	}
}

void AFOCharacterBase::HandleDeath()
{
	if (IsDead())
	{
		return;
	}

	const EFOCharacterLifeState PreviousLifeState = LifeState;
	const float PreviousHealth = Health;

	LifeState = EFOCharacterLifeState::Dead;
	Health = 0.0f;
	DownedStartServerWorldTime = 0.0f;
	bIsAiming = false;
	bIsSprinting = false;
	CharacterStance = EFOCharacterStance::Standing;
	SetMovementState(EFOMovementState::Idle);

	OnRep_Health(PreviousHealth);
	OnRep_CharacterStance();
	OnRep_LifeState(PreviousLifeState);
	OnRep_Aiming();
	OnRep_Sprinting();
}

void AFOCharacterBase::ServerSetViewMode_Implementation(EFOViewMode NewViewMode)
{
	SetViewMode(NewViewMode);
}

void AFOCharacterBase::ServerSetAiming_Implementation(bool bNewAiming)
{
	SetAiming(bNewAiming);
}

void AFOCharacterBase::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

void AFOCharacterBase::ServerSetCharacterStance_Implementation(EFOCharacterStance NewStance)
{
	SetCharacterStance(NewStance);
}

void AFOCharacterBase::ServerStartWeaponFire_Implementation()
{
	StartWeaponFire();
}

void AFOCharacterBase::ServerStopWeaponFire_Implementation()
{
	StopWeaponFire();
}

void AFOCharacterBase::ServerReloadCurrentWeapon_Implementation()
{
	ReloadCurrentWeapon();
}

void AFOCharacterBase::ServerEquipWeaponSlot_Implementation(EFOWeaponSlot DesiredSlot)
{
	EquipWeaponSlot(DesiredSlot);
}

void AFOCharacterBase::OnRep_ViewMode()
{
	ApplyViewModeInternal();
}

void AFOCharacterBase::OnRep_CharacterStance()
{
	if (CharacterStance == EFOCharacterStance::Crouching)
	{
		Crouch(false);
	}
	else
	{
		UnCrouch(false);
	}

	OnStanceChanged.Broadcast(CharacterStance);
	BP_OnStanceChanged(CharacterStance);
}

void AFOCharacterBase::OnRep_MovementState()
{
	OnMovementStateChanged.Broadcast(MovementState);
	BP_OnMovementStateChanged(MovementState);
}

void AFOCharacterBase::OnRep_Aiming()
{
	RefreshMovementTuning();
	BP_OnAimingChanged(bIsAiming);
}

void AFOCharacterBase::OnRep_Sprinting()
{
	RefreshMovementTuning();
	BP_OnSprintChanged(bIsSprinting);
}

void AFOCharacterBase::OnRep_Health(float OldHealth)
{
	HandleHealthChanged(OldHealth);
}

void AFOCharacterBase::OnRep_LifeState(EFOCharacterLifeState PreviousLifeState)
{
	ApplyLifeStateMovementRestrictions();

	OnLifeStateChanged.Broadcast(LifeState);
	BP_OnLifeStateChanged(LifeState);

	if (LifeState == EFOCharacterLifeState::Downed && PreviousLifeState != EFOCharacterLifeState::Downed)
	{
		BP_OnDownedStarted();
	}
	else if (LifeState == EFOCharacterLifeState::Dead && PreviousLifeState != EFOCharacterLifeState::Dead)
	{
		BP_OnDeathStarted();
	}
}

void AFOCharacterBase::OnRep_PrimaryWeapon()
{
	RefreshWeaponAttachments();
}

void AFOCharacterBase::OnRep_SecondaryWeapon()
{
	RefreshWeaponAttachments();
}

void AFOCharacterBase::OnRep_CurrentWeapon()
{
	RefreshWeaponAttachments();
	BP_OnWeaponEquipped(CurrentWeapon);
}
