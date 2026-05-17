#include "AI/BTTask_FindRandomPatrolPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindRandomPatrolPoint::UBTTask_FindRandomPatrolPoint()
{
	NodeName = TEXT("Find Random Patrol Point");
	SearchRadius = 1200.0f;
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomPatrolPoint, BlackboardKey));
}

EBTNodeResult::Type UBTTask_FindRandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (ControlledPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(ControlledPawn->GetWorld());
	if (NavigationSystem == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomPatrolPoint;
	const bool bFoundPoint = NavigationSystem->GetRandomReachablePointInRadius(
		ControlledPawn->GetActorLocation(),
		SearchRadius,
		RandomPatrolPoint);

	if (!bFoundPoint)
	{
		return EBTNodeResult::Failed;
	}

	if (UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsVector(BlackboardKey.SelectedKeyName, RandomPatrolPoint.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
