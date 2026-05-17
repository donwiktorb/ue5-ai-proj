#pragma once

#include "CoreMinimal.h"
#include "AI/FOAITypes.h"
#include "Character/FOCharacterBase.h"
#include "FOEnemyCharacter.generated.h"

class UBehaviorTree;

UCLASS(Blueprintable)
class FALSEORDERS_API AFOEnemyCharacter : public AFOCharacterBase
{
	GENERATED_BODY()

public:
	AFOEnemyCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAlertState(EFOAIAlertState NewAlertState);

	UFUNCTION(BlueprintPure, Category = "AI")
	EFOAIAlertState GetAlertState() const
	{
		return AlertState;
	}

	UFUNCTION(BlueprintCallable, Category = "AI")
	void NotifyTargetSpotted(AActor* SeenActor);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void NotifyTargetLost(AActor* LostActor);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartAttack();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopAttack();

	UFUNCTION(BlueprintPure, Category = "AI")
	UBehaviorTree* GetBehaviorTreeAsset() const
	{
		return BehaviorTreeAsset;
	}

	UFUNCTION(BlueprintPure, Category = "AI")
	int32 GetTeamId() const
	{
		return TeamId;
	}

	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsAttackActive() const
	{
		return bIsAttackActive;
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnAlertStateChanged(EFOAIAlertState NewAlertState);

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnTargetSpotted(AActor* SeenActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnTargetLost(AActor* LostActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnAttackStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnAttackStopped();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void BP_OnEnemyDeathStarted();

protected:
	UFUNCTION()
	void OnRep_AlertState();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	/** Reserved for co-op faction checks once friend/foe logic expands beyond player-controlled pawns. */
    // WILL BE NEEDED LATER
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	int32 TeamId;

	UPROPERTY(ReplicatedUsing = OnRep_AlertState, BlueprintReadOnly, Category = "AI")
	EFOAIAlertState AlertState;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsAttackActive;

	EFOCharacterLifeState CachedLifeState;
};

