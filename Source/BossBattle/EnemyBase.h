// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

UCLASS()
class BOSSBATTLE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

public:
	// 剣から呼び出す
	UFUNCTION(BlueprintCallable)
	void ReceiveSwordDamage(float Damage);

	// 現在のHPを外から読めるようにする
	UFUNCTION(BlueprintPure)
	float GetCurrentHP() const { return CurrentHP; }

private:
	// HPの変数を設定
	UPROPERTY(EditAnywhere, Category="Enemy")
	float MaxHP = 100.f;
	float CurrentHP;

	// 無敵時間の設定
	UPROPERTY(EditAnywhere, Category="Enemy")
	float InvincibleDuration = 0.5f;
	bool bIsInvincible = false;

	// 無敵タイマーの管理用
	FTimerHandle InvincibleTimerHandle;

	// 無敵時間が終わったときに呼び出し
	void EndInvincible();
};
