// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 最大移動速度を設定
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 現在HPのセット
	CurrentHP = MaxHP;

	// ゲーム開始時のプレイヤーの位置の取得
	Player = GetWorld()->GetFirstPlayerController()->GetPawn();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// プレイヤーの位置に移動
	if (Player)
	{

		// Playerとの距離によって追跡、攻撃の決定
		float Distance = FVector::Dist(Player->GetActorLocation(), GetActorLocation());
		if (Distance > AttackRange)
		{
			// Playerを追跡
			FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			AddMovementInput(Direction, 1.0f);
		}
		else 
		{
			// 追跡を中止して攻撃
			TryAttack();
		}

	}

}


void AEnemyBase::TryAttack()
{
	if (bCanAttack)
	{
		bCanAttack = false;
		
		Attack();

		GetWorldTimerManager().SetTimer(
			AttackTimerHandle,
			this,
			&AEnemyBase::ResetAttack,
			AttackCooldown,
			false
		);

	}
}

// 攻撃モーションの再生
void AEnemyBase::Attack()
{
	PlayAnimMontage(PunchMontage);
}

// 攻撃のクールダウンリセット
void AEnemyBase::ResetAttack()
{
	bCanAttack = true;
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

