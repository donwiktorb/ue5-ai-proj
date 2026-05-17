#include "Core/FOPlayerState.h"

#include "Net/UnrealNetwork.h"

AFOPlayerState::AFOPlayerState()
{
	SquadId = 0;
	bReadyForRespawn = false;
}

void AFOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFOPlayerState, SquadId);
	DOREPLIFETIME(AFOPlayerState, bReadyForRespawn);
}

void AFOPlayerState::SetSquadId(int32 NewSquadId)
{
	if (HasAuthority())
	{
		SquadId = NewSquadId;
	}
}

void AFOPlayerState::SetReadyForRespawn(bool bNewReadyForRespawn)
{
	if (HasAuthority())
	{
		bReadyForRespawn = bNewReadyForRespawn;
	}
}

