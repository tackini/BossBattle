// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;

UCLASS()
class BOSSBATTLE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
		
	virtual void BeginPlay() override;

protected:

	// BB‚ðƒZƒbƒg
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FName TargetActorKeyName = "TargetActor";

};
