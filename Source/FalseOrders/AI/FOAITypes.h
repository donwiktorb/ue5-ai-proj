#pragma once

#include "CoreMinimal.h"
#include "FOAITypes.generated.h"

UENUM(BlueprintType)
enum class EFOAIAlertState : uint8
{
	Passive UMETA(DisplayName = "Passive"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Alerted UMETA(DisplayName = "Alerted"),
	Searching UMETA(DisplayName = "Searching"),
	Combat UMETA(DisplayName = "Combat")
};

inline const UEnum* GetFOAIAlertStateEnum()
{
	return StaticEnum<EFOAIAlertState>();
}
