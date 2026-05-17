#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FOPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS(Blueprintable)
class FALSEORDERS_API AFOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFOPlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetDefaultInputMappingContext(UInputMappingContext* NewMappingContext);

protected:
	void ApplyDefaultInputContext();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
};
