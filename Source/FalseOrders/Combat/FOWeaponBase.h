#pragma once

#include "CoreMinimal.h"
#include "Combat/FOWeaponTypes.h"
#include "GameFramework/Actor.h"
#include "FOWeaponBase.generated.h"

class AFOCharacterBase;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFOWeaponAmmoChangedSignature, int32, NewAmmoInMagazine, int32, NewReserveAmmo);

UCLASS(Blueprintable)
class FALSEORDERS_API AFOWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AFOWeaponBase();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void StartFiring();
	void StopFiring();
	void StartReload();
	void InterruptReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetOwningCharacter(AFOCharacterBase* NewOwningCharacter);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponSlot(EFOWeaponSlot NewWeaponSlot);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetEquipped(bool bNewEquipped);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanReload() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AFOCharacterBase* GetOwningCharacter() const
	{
		return OwningCharacter;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EFOWeaponSlot GetWeaponSlot() const
	{
		return WeaponSlot;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EFOWeaponAnimationType GetWeaponAnimationType() const
	{
		return WeaponAnimationType;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsEquipped() const
	{
		return bIsEquipped;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const
	{
		return bIsReloading;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetAmmoInMagazine() const
	{
		return AmmoInMagazine;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetReserveAmmo() const
	{
		return ReserveAmmo;
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetFireInterval() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentSpreadAngleDegrees() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_OnWeaponFired(FVector TraceEnd, bool bHitSomething);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_OnReloadStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_OnReloadFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_OnEquippedChanged(bool bNewEquipped);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void BP_OnAmmoChanged(int32 NewAmmoInMagazine, int32 NewReserveAmmo);

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FFOWeaponAmmoChangedSignature OnAmmoChanged;

protected:
	void InitializeAmmo();
	void HandleAutomaticFireTick();
	void HandleFireShot();
	void FinishReload();
	void NotifyAmmoChanged();
	void BroadcastReloadStarted();
	void BroadcastReloadFinished();
	void ApplyEquippedPresentation();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireCosmetics(FVector_NetQuantize TraceEnd, bool bHitSomething);

	UFUNCTION()
	void OnRep_OwningCharacter();

	UFUNCTION()
	void OnRep_AmmoInMagazine();

	UFUNCTION()
	void OnRep_ReserveAmmo();

	UFUNCTION()
	void OnRep_Equipped();

	UFUNCTION()
	void OnRep_Reloading();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Replicated owner reference so remote machines can resolve weapon attachment and presentation without trusting actor owner state alone. */
    UPROPERTY(ReplicatedUsing = OnRep_OwningCharacter, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AFOCharacterBase> OwningCharacter;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EFOWeaponSlot WeaponSlot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EFOWeaponAnimationType WeaponAnimationType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FFOWeaponAmmoConfig AmmoConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FFOWeaponFireConfig FireConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FFOWeaponSpreadConfig SpreadConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FFOHitscanWeaponConfig HitscanConfig;

	UPROPERTY(ReplicatedUsing = OnRep_AmmoInMagazine, BlueprintReadOnly, Category = "Weapon")
	int32 AmmoInMagazine;

	UPROPERTY(ReplicatedUsing = OnRep_ReserveAmmo, BlueprintReadOnly, Category = "Weapon")
	int32 ReserveAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_Equipped, BlueprintReadOnly, Category = "Weapon")
	bool bIsEquipped;

	UPROPERTY(ReplicatedUsing = OnRep_Reloading, BlueprintReadOnly, Category = "Weapon")
	bool bIsReloading;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bWantsToFire;

	float LastFireTimeSeconds;
	FTimerHandle AutomaticFireTimerHandle;
	FTimerHandle ReloadTimerHandle;
};
