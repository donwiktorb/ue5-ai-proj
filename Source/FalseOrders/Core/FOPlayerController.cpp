#include "Core/FOPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AFOPlayerController::AFOPlayerController()
{
	bReplicates = true;
}

void AFOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyDefaultInputContext();
}

void AFOPlayerController::SetDefaultInputMappingContext(UInputMappingContext* NewMappingContext)
{
	DefaultInputMappingContext = NewMappingContext;
	ApplyDefaultInputContext();
}

void AFOPlayerController::ApplyDefaultInputContext()
{
	if (!IsLocalController())
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultInputMappingContext)
			{
				InputSubsystem->RemoveMappingContext(DefaultInputMappingContext);
				InputSubsystem->AddMappingContext(DefaultInputMappingContext, 0);
			}
		}
	}
}
