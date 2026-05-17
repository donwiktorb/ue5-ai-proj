#pragma once

#include "CoreMinimal.h"

#include "FOCharacterTypes.generated.h"

UENUM(BlueprintType)
enum class EFOCharacterStance : uint8
{
	Standing,
	Crouching
};

UENUM(BlueprintType)
enum class EFOMovementState : uint8
{
	Idle,
	Walking,
	Sprinting
};

UENUM(BlueprintType)
enum class EFOViewMode : uint8
{
	ThirdPerson,
	FirstPerson
};

UENUM(BlueprintType)
enum class EFOCharacterLifeState : uint8
{
	Alive,
	Downed,
	Dead
};
