#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FOGameState.generated.h"

UENUM(BlueprintType)
enum class EFOMissionFlowState : uint8
{
	Lobby,
	InProgress,
	Completed,
	Failed
};

UCLASS(Blueprintable)
class FALSEORDERS_API AFOGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFOGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Mission")
	int32 GetConnectedPlayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void SetMissionFlowState(EFOMissionFlowState NewState);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MissionFlowState, BlueprintReadOnly, Category = "Mission")
	EFOMissionFlowState MissionFlowState;

	UFUNCTION()
	void OnRep_MissionFlowState();
};

