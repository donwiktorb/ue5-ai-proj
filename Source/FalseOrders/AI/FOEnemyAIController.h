#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/FOAITypes.h"
#include "FOEnemyAIController.generated.h"

class AFOEnemyCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS(Blueprintable)
class FALSEORDERS_API AFOEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFOEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	void InitializeBlackboardForEnemy(AFOEnemyCharacter* EnemyCharacter);
	void SetAlertState(EFOAIAlertState NewAlertState);
	bool IsValidPlayerTarget(const AActor* Actor) const;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> FOAIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "0.0"))
	float SightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionAngleDegrees;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
	FName TargetActorKeyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
	FName LastKnownLocationKeyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
	FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
	FName CanSeeTargetKeyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
	FName AlertStateKeyName;
};
