// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHUDWidget.generated.h"

class AEnemyBase;

UCLASS()
class BOSSBATTLE_API UBossHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// UpdatePlayerHPÇBPì‡Ç≈çÏÇÈ
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void UpdatePlayerHP(float HPPercent);

	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void UpdateEnemyHP(float HPPercent);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void UpdateEnemyName(const FText& EnemyName);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void OnEnemyDead(AEnemyBase* DeadEnemy);
};
