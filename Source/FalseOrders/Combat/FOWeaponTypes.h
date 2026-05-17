#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#include "FOWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EFOWeaponSlot : uint8
{
	Primary,
	Secondary
};

UENUM(BlueprintType)
enum class EFOWeaponFireMode : uint8
{
	SemiAutomatic,
	FullAutomatic
};

UENUM(BlueprintType)
enum class EFOWeaponAnimationType : uint8
{
	None,
	Rifle,
	Pistol
};

USTRUCT(BlueprintType)
struct FALSEORDERS_API FFOWeaponAmmoConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1"))
	int32 MagazineSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0"))
	int32 InitialReserveAmmo = 90;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	bool bUnlimitedReserveAmmo = false;
};

USTRUCT(BlueprintType)
struct FALSEORDERS_API FFOWeaponSpreadConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float HipFireSpreadAngleDegrees = 1.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AimingSpreadAngleDegrees = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "1.0"))
	float MovementSpreadMultiplier = 1.5f;
};

USTRUCT(BlueprintType)
struct FALSEORDERS_API FFOWeaponFireConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	EFOWeaponFireMode FireMode = EFOWeaponFireMode::FullAutomatic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "1.0"))
	float RoundsPerMinute = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "0.0"))
	float ReloadDuration = 1.8f;
};

USTRUCT(BlueprintType)
struct FALSEORDERS_API FFOHitscanWeaponConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "0.0"))
	float Damage = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan", meta = (ClampMin = "100.0"))
	float TraceDistance = 20000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitscan")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};
