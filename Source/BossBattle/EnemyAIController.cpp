// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);

		// Player‚ÌŽæ“¾
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		if (PlayerPawn)
		{
			// BB‚ÌTargetActor‚ÉPlayer‚ð•Û‘¶
			GetBlackboardComponent()->SetValueAsObject(
				TargetActorKeyName,
				PlayerPawn
			);
		}
	}
}

