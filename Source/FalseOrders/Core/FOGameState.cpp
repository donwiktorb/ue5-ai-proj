#include "Core/FOGameState.h"

#include "Net/UnrealNetwork.h"

AFOGameState::AFOGameState()
{
	MissionFlowState = EFOMissionFlowState::Lobby;
}

void AFOGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFOGameState, MissionFlowState);
}

int32 AFOGameState::GetConnectedPlayerCount() const
{
	return PlayerArray.Num();
}

void AFOGameState::SetMissionFlowState(EFOMissionFlowState NewState)
{
	if (HasAuthority())
	{
		MissionFlowState = NewState;
		OnRep_MissionFlowState();
	}
}

void AFOGameState::OnRep_MissionFlowState()
{
	// Blueprint subclasses can react to replicated mission flow changes for UI or sequencing.
    // As of time writing this ... there is no ui or sequencing lol
    // WILL BE NEEDED LATER
}

