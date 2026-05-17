#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindRandomPatrolPoint.generated.h"

UCLASS(Blueprintable)
class FALSEORDERS_API UBTTask_FindRandomPatrolPoint : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FindRandomPatrolPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (ClampMin = "0.0"))
	float SearchRadius;
};

