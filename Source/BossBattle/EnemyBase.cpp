// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "TimerManager.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	// 現在HPのセット
	CurrentHP = MaxHP;
}

// 敵の被ダメージ処理
void AEnemyBase::ReceiveSwordDamage(float Damage)
{
	// 無敵中か現在HPが0なら実行しない
	if (bIsInvincible || CurrentHP <= 0.f) return;

	// ダメージ計算
	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);

	// 現在HPが0なら敵を削除
	if (CurrentHP <= 0.f)
	{
		Destroy();
		return;
	}

	// ダメージを受けると短時間無敵
	bIsInvincible = true;
	GetWorld()->GetTimerManager().SetTimer(
		InvincibleTimerHandle,
		this,
		&AEnemyBase::EndInvincible,
		InvincibleDuration,
		false
	);
}

// 無敵時間の終了
void AEnemyBase::EndInvincible()
{
	bIsInvincible = false;
}

