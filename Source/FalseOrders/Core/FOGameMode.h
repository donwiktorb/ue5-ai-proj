#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FOGameMode.generated.h"

class AFOCharacterBase;

UCLASS(Blueprintable)
class FALSEORDERS_API AFOGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFOGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

protected:
	/** Kept editable so we can swap the playable pawn with a Blueprint subclass. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode")
	TSubclassOf<AFOCharacterBase> DefaultCharacterClass;
};

