#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_ClearBlackboardKey.generated.h"

UCLASS()
class FALSEORDERS_API UBTTask_ClearBlackboardKey : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ClearBlackboardKey();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector KeyToClear;
};

