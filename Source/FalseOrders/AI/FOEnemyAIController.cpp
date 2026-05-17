#include "AI/FOEnemyAIController.h"

#include "AI/FOEnemyCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Character/FOCharacterBase.h"
#include "Core/FOLogChannels.h"

AFOEnemyAIController::AFOEnemyAIController()
{
	(void)GetFOAIAlertStateEnum();

	bAttachToPawn = true;

	SightRadius = 2500.0f;
	LoseSightRadius = 3000.0f;
	PeripheralVisionAngleDegrees = 75.0f;

	TargetActorKeyName = TEXT("TargetActor");
	LastKnownLocationKeyName = TEXT("LastKnownLocation");
	PatrolLocationKeyName = TEXT("PatrolLocation");
	CanSeeTargetKeyName = TEXT("CanSeeTarget");
	AlertStateKeyName = TEXT("AlertState");

	FOAIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*FOAIPerceptionComponent);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
	SightConfig->SetMaxAge(3.0f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 300.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	FOAIPerceptionComponent->ConfigureSense(*SightConfig);
	FOAIPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
	FOAIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AFOEnemyAIController::HandleTargetPerceptionUpdated);
}

void AFOEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFOEnemyCharacter* EnemyCharacter = Cast<AFOEnemyCharacter>(InPawn);
	if (EnemyCharacter == nullptr)
	{
		return;
	}

	InitializeBlackboardForEnemy(EnemyCharacter);

	if (UBehaviorTree* BehaviorTreeAsset = EnemyCharacter->GetBehaviorTreeAsset())
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	SetAlertState(EFOAIAlertState::Passive);
}

void AFOEnemyAIController::OnUnPossess()
{
	ClearFocus(EAIFocusPriority::Gameplay);

	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetActorKeyName);
		BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, false);
	}

	Super::OnUnPossess();
}

void AFOEnemyAIController::InitializeBlackboardForEnemy(AFOEnemyCharacter* EnemyCharacter)
{
	UBehaviorTree* BehaviorTreeAsset = EnemyCharacter ? EnemyCharacter->GetBehaviorTreeAsset() : nullptr;
	if (BehaviorTreeAsset == nullptr || BehaviorTreeAsset->BlackboardAsset == nullptr)
	{
		return;
	}

	UBlackboardComponent* BlackboardComponent = nullptr;
	UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComponent);

	if (BlackboardComponent)
	{
		BlackboardComponent->ClearValue(TargetActorKeyName);
		BlackboardComponent->SetValueAsVector(LastKnownLocationKeyName, EnemyCharacter->GetActorLocation());
		BlackboardComponent->SetValueAsVector(PatrolLocationKeyName, EnemyCharacter->GetActorLocation());
		BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, false);
		BlackboardComponent->SetValueAsEnum(AlertStateKeyName, static_cast<uint8>(EFOAIAlertState::Passive));
	}
}

void AFOEnemyAIController::SetAlertState(EFOAIAlertState NewAlertState)
{
	if (AFOEnemyCharacter* EnemyCharacter = GetPawn<AFOEnemyCharacter>())
	{
		EnemyCharacter->SetAlertState(NewAlertState);
	}

	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsEnum(AlertStateKeyName, static_cast<uint8>(NewAlertState));
	}
}

bool AFOEnemyAIController::IsValidPlayerTarget(const AActor* Actor) const
{
	const AFOCharacterBase* CharacterTarget = Cast<AFOCharacterBase>(Actor);
	if (CharacterTarget == nullptr)
	{
		return false;
	}

	if (CharacterTarget == GetPawn())
	{
		return false;
	}

	return true;
}

void AFOEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UE_LOG(
		LogFalseOrders,
		Warning,
		TEXT("Perception update: Actor=%s Class=%s Sensed=%s HasAuthority=%s"),
		*GetNameSafe(Actor),
		*GetNameSafe(Actor ? Actor->GetClass() : nullptr),
		Stimulus.WasSuccessfullySensed() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"));

	if (!HasAuthority())
	{
		UE_LOG(LogFalseOrders, Warning, TEXT("Perception rejected: no authority on %s"), *GetName());
		return;
	}

	if (!IsValidPlayerTarget(Actor))
	{
		UE_LOG(LogFalseOrders, Warning, TEXT("Perception rejected: invalid target %s"), *GetNameSafe(Actor));
		return;
	}

	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	AFOEnemyCharacter* EnemyCharacter = GetPawn<AFOEnemyCharacter>();
	if (BlackboardComponent == nullptr || EnemyCharacter == nullptr)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BlackboardComponent->SetValueAsObject(TargetActorKeyName, Actor);
		BlackboardComponent->SetValueAsVector(LastKnownLocationKeyName, Actor->GetActorLocation());
		BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, true);
		SetAlertState(EFOAIAlertState::Combat);
		SetFocus(Actor, EAIFocusPriority::Gameplay);
		EnemyCharacter->NotifyTargetSpotted(Actor);

		UE_LOG(LogFalseOrders, Verbose, TEXT("%s acquired target %s"), *GetName(), *GetNameSafe(Actor));
		return;
	}

	BlackboardComponent->SetValueAsVector(LastKnownLocationKeyName, Stimulus.StimulusLocation);
	BlackboardComponent->ClearValue(TargetActorKeyName);
	BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, false);
	SetAlertState(EFOAIAlertState::Searching);
	ClearFocus(EAIFocusPriority::Gameplay);
	EnemyCharacter->NotifyTargetLost(Actor);

	UE_LOG(LogFalseOrders, Verbose, TEXT("%s lost target %s"), *GetName(), *GetNameSafe(Actor));
}
