#include "Core/FOGameMode.h"

#include "Character/FOCharacterBase.h"
#include "Core/FOGameState.h"
#include "Core/FOLogChannels.h"
#include "Core/FOPlayerController.h"
#include "Core/FOPlayerState.h"

AFOGameMode::AFOGameMode()
{
	GameStateClass = AFOGameState::StaticClass();
	PlayerControllerClass = AFOPlayerController::StaticClass();
	PlayerStateClass = AFOPlayerState::StaticClass();

	DefaultCharacterClass = AFOCharacterBase::StaticClass();
	DefaultPawnClass = DefaultCharacterClass;

	bStartPlayersAsSpectators = false;
}

void AFOGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogFalseOrdersNetwork, Log, TEXT("Player joined: %s"), *GetNameSafe(NewPlayer));
}

void AFOGameMode::Logout(AController* Exiting)
{
	UE_LOG(LogFalseOrdersNetwork, Log, TEXT("Player left: %s"), *GetNameSafe(Exiting));

	Super::Logout(Exiting);
}

void AFOGameMode::RestartPlayer(AController* NewPlayer)
{
	if (DefaultCharacterClass)
	{
		DefaultPawnClass = DefaultCharacterClass;
	}

	Super::RestartPlayer(NewPlayer);
}

