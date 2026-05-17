#pragma once

#include "CoreMinimal.h"
#include "Character/FOCharacterTypes.h"
#include "Combat/FOWeaponTypes.h"
#include "GameFramework/Character.h"
#include "FOCharacterBase.generated.h"

class AFOWeaponBase;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFOViewModeChangedSignature, EFOViewMode, NewViewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFOStanceChangedSignature, EFOCharacterStance, NewStance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFOMovementStateChangedSignature, EFOMovementState, NewMovementState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFOLifeStateChangedSignature, EFOCharacterLifeState, NewLifeState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFOHealthChangedSignature, float, OldHealth, float, NewHealth, float, MaxHealth);

UCLASS(Blueprintable)
class FALSEORDERS_API AFOCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AFOCharacterBase();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Character|View")
	void ToggleViewMode();

	UFUNCTION(BlueprintCallable, Category = "Character|View")
	void SetViewMode(EFOViewMode NewViewMode);

	UFUNCTION(BlueprintCallable, Category = "Character|State")
	void SetAiming(bool bNewAiming);

	UFUNCTION(BlueprintCallable, Category = "Character|State")
	void SetSprinting(bool bNewSprinting);

	UFUNCTION(BlueprintCallable, Category = "Character|State")
	void SetCharacterStance(EFOCharacterStance NewStance);

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	float ApplyDamageAmount(float DamageAmount, AActor* DamageInstigatorActor);

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	bool IsDowned() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Character|State")
	bool CanSprint() const;

	UFUNCTION(BlueprintPure, Category = "Character|State")
	bool CanAim() const;

	UFUNCTION(BlueprintPure, Category = "Character|State")
	bool CanUseCrouch() const;

	UFUNCTION(BlueprintPure, Category = "Character|Health")
	bool CanBeRevived() const;

	UFUNCTION(BlueprintPure, Category = "Character|Health")
	float GetBleedoutTimeRemaining() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnViewModeChanged(EFOViewMode NewViewMode);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnStanceChanged(EFOCharacterStance NewStance);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnLifeStateChanged(EFOCharacterLifeState NewLifeState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnMovementStateChanged(EFOMovementState NewMovementState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnAimingChanged(bool bNewIsAiming);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnSprintChanged(bool bNewIsSprinting);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnHealthChanged(float OldHealth, float NewHealth, float InMaxHealth);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnDownedStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void BP_OnDeathStarted();

	UPROPERTY(BlueprintAssignable, Category = "Character")
	FFOViewModeChangedSignature OnViewModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character")
	FFOStanceChangedSignature OnStanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character")
	FFOMovementStateChangedSignature OnMovementStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character")
	FFOLifeStateChangedSignature OnLifeStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character")
	FFOHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintPure, Category = "Character")
	EFOCharacterStance GetCharacterStance() const
	{
		return CharacterStance;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	EFOMovementState GetMovementState() const
	{
		return MovementState;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	EFOViewMode GetViewMode() const
	{
		return ViewMode;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	EFOCharacterLifeState GetLifeState() const
	{
		return LifeState;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsAiming() const
	{
		return bIsAiming;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	bool IsSprinting() const
	{
		return bIsSprinting;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	float GetHealth() const
	{
		return Health;
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	float GetMaxHealth() const
	{
		return MaxHealth;
	}

	UFUNCTION(BlueprintPure, Category = "Character|Animation")
	float GetCurrentSpeed() const
	{
		return CurrentSpeed;
	}

	UFUNCTION(BlueprintPure, Category = "Character|Animation")
	float GetMovementInputAmount() const
	{
		return MovementInputAmount;
	}

	UFUNCTION(BlueprintPure, Category = "Character|Animation")
	bool IsFirstPersonView() const
	{
		return ViewMode == EFOViewMode::FirstPerson;
	}

	UFUNCTION(BlueprintPure, Category = "Character|Animation")
	EFOWeaponAnimationType GetEquippedWeaponAnimationType() const;

	UFUNCTION(BlueprintPure, Category = "Character|Weapon")
	AFOWeaponBase* GetCurrentWeapon() const
	{
		return CurrentWeapon;
	}

	UFUNCTION(BlueprintPure, Category = "Character|Weapon")
	bool CanFireWeapon() const;

	UFUNCTION(BlueprintPure, Category = "Character|Weapon")
	bool CanReloadWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void EquipWeaponSlot(EFOWeaponSlot DesiredSlot);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Weapon")
	void BP_OnWeaponEquipped(AFOWeaponBase* NewWeapon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Weapon")
	void BP_OnWeaponFired(AFOWeaponBase* FiredWeapon, FVector TraceEnd);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Weapon")
	void BP_OnWeaponReloadStarted(AFOWeaponBase* ReloadingWeapon);

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Weapon")
	void BP_OnWeaponReloadFinished(AFOWeaponBase* ReloadingWeapon);

	void NotifyWeaponFired(AFOWeaponBase* FiredWeapon, FVector TraceEnd);
	void NotifyWeaponReloadStarted(AFOWeaponBase* ReloadingWeapon);
	void NotifyWeaponReloadFinished(AFOWeaponBase* ReloadingWeapon);

protected:
	void ApplyInputMappingContext() const;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void InputStartSprint();
	void InputStopSprint();
	void InputToggleCrouch();
	void InputStartAim();
	void InputStopAim();
	void InputStartFire();
	void InputStopFire();
	void InputReload();
	void InputEquipPrimaryWeapon();
	void InputEquipSecondaryWeapon();

	void UpdateAnimationState();
	void UpdateMovementState();
	void RefreshMovementTuning();
	void ApplyViewModeInternal();
	void UpdateFirstPersonMeshVisibility();
	void ApplyLifeStateMovementRestrictions();
	void SpawnDefaultWeapons();
	AFOWeaponBase* SpawnDefaultWeaponForSlot(TSubclassOf<AFOWeaponBase> WeaponClass, EFOWeaponSlot WeaponSlot);
	AFOWeaponBase* GetWeaponForSlot(EFOWeaponSlot WeaponSlot) const;
	void SetCurrentWeapon(AFOWeaponBase* NewWeapon);
	void RefreshWeaponAttachments();
	void UpdateWeaponAttachment(AFOWeaponBase* WeaponToUpdate, bool bIsEquippedWeapon) const;
	bool TryAttachWeaponToSocket(AFOWeaponBase* WeaponToAttach, FName SocketName) const;
	void AttachWeaponFallback(AFOWeaponBase* WeaponToAttach) const;
	void ApplyWeaponVisibility(AFOWeaponBase* WeaponToUpdate, bool bShouldBeVisible) const;
	FName GetHolsterSocketNameForWeapon(const AFOWeaponBase* Weapon) const;
	void StartWeaponFire();
	void StopWeaponFire();
	void ReloadCurrentWeapon();
	void HandleHealthChanged(float OldHealth);
	void SetMovementState(EFOMovementState NewMovementState);
	void EnterDownedState();
	void HandleDeath();

	UFUNCTION(Server, Reliable)
	void ServerSetViewMode(EFOViewMode NewViewMode);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterStance(EFOCharacterStance NewStance);

	UFUNCTION(Server, Reliable)
	void ServerStartWeaponFire();

	UFUNCTION(Server, Reliable)
	void ServerStopWeaponFire();

	UFUNCTION(Server, Reliable)
	void ServerReloadCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeaponSlot(EFOWeaponSlot DesiredSlot);

	UFUNCTION()
	void OnRep_ViewMode();

	UFUNCTION()
	void OnRep_CharacterStance();

	UFUNCTION()
	void OnRep_MovementState();

	UFUNCTION()
	void OnRep_Aiming();

	UFUNCTION()
	void OnRep_Sprinting();

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_LifeState(EFOCharacterLifeState PreviousLifeState);

	UFUNCTION()
	void OnRep_PrimaryWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> TogglePerspectiveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> EquipPrimaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Input")
	TObjectPtr<UInputAction> EquipSecondaryWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	float CrouchSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	float AimingWalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	float DownedLookRateMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	float ThirdPersonArmLength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
	FVector FirstPersonCameraOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|View")
	TArray<FName> FirstPersonHiddenBones;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weapon")
	TSubclassOf<AFOWeaponBase> DefaultPrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weapon")
	TSubclassOf<AFOWeaponBase> DefaultSecondaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weapon")
	FName EquippedWeaponSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weapon")
	FName PrimaryHolsterSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Weapon")
	FName SecondaryHolsterSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health")
	float MaxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health")
	bool bCanBeDowned;

	/** Bleedout is tracked now so revive can hook in later without restructuring the character state flow. */
    // WILL BE NEEDED LATER
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health", meta = (ClampMin = "0.0"))
	float BleedoutDuration;

	/** These replicated values are the authoritative gameplay state that simulated proxies animate from. */
    // WILL BE NEEDED LATER
	UPROPERTY(ReplicatedUsing = OnRep_CharacterStance, BlueprintReadOnly, Category = "Character|State")
	EFOCharacterStance CharacterStance;

	UPROPERTY(ReplicatedUsing = OnRep_MovementState, BlueprintReadOnly, Category = "Character|State")
	EFOMovementState MovementState;

	UPROPERTY(ReplicatedUsing = OnRep_ViewMode, BlueprintReadOnly, Category = "Character|State")
	EFOViewMode ViewMode;

	UPROPERTY(ReplicatedUsing = OnRep_LifeState, BlueprintReadOnly, Category = "Character|State")
	EFOCharacterLifeState LifeState;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming, BlueprintReadOnly, Category = "Character|State")
	bool bIsAiming;

	UPROPERTY(ReplicatedUsing = OnRep_Sprinting, BlueprintReadOnly, Category = "Character|State")
	bool bIsSprinting;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Character|Health")
	float Health;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character|Health")
	float DownedStartServerWorldTime;

	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon, BlueprintReadOnly, Category = "Character|Weapon")
	TObjectPtr<AFOWeaponBase> PrimaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon, BlueprintReadOnly, Category = "Character|Weapon")
	TObjectPtr<AFOWeaponBase> SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Character|Weapon")
	TObjectPtr<AFOWeaponBase> CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Animation")
	float CurrentSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Animation")
	float MovementInputAmount;

	UPROPERTY()
	bool bHasBeenDownedThisLife;
};
