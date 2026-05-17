#include "AI/BTTask_ClearBlackboardKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearBlackboardKey::UBTTask_ClearBlackboardKey()
{
	NodeName = TEXT("Clear Blackboard Key");
}

EBTNodeResult::Type UBTTask_ClearBlackboardKey::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComponent->ClearValue(KeyToClear.SelectedKeyName);
	return EBTNodeResult::Succeeded;
}
