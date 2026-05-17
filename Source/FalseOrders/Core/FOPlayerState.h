#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FOPlayerState.generated.h"

UCLASS(Blueprintable)
class FALSEORDERS_API AFOPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFOPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void SetSquadId(int32 NewSquadId);

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void SetReadyForRespawn(bool bNewReadyForRespawn);

	UFUNCTION(BlueprintPure, Category = "Player State")
	int32 GetSquadId() const
	{
		return SquadId;
	}

	UFUNCTION(BlueprintPure, Category = "Player State")
	bool IsReadyForRespawn() const
	{
		return bReadyForRespawn;
	}

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player State")
	int32 SquadId;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player State")
	bool bReadyForRespawn;
};

