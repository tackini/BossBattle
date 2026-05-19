// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "EnemyBase.h"
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
		// Enemy‚ÌŽæ“¾
		AEnemyBase* EnemyPawn = Cast<AEnemyBase>(GetPawn());

		if (PlayerPawn && EnemyPawn)
		{
			// BB‚ÌTargetActor‚ÉPlayer‚ð•Û‘¶
			GetBlackboardComponent()->SetValueAsObject(
				"TargetActor",
				PlayerPawn
			);

			float Distance = FVector::Dist(EnemyPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
			GetBlackboardComponent()->SetValueAsFloat(
				"DistanceToPlayer",
				Distance
			);
		}
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
		// Player‚ÌŽæ“¾
		APawn*PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		// Enemy‚ÌŽæ“¾
		AEnemyBase* EnemyPawn = Cast<AEnemyBase>(GetPawn());

		if (PlayerPawn && EnemyPawn)
		{
			float Distance = FVector::Dist(EnemyPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
			GetBlackboardComponent()->SetValueAsFloat(
				"DistanceToPlayer",
				Distance
			);
		}
}

