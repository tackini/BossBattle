// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "BossBattleCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 進路方向への回転速度を設定
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

	// 攻撃判定の生成
	AttackHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitBox"));
	AttackHitBox->SetupAttachment(GetMesh());

}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 現在HPのセット
	EnemyStatus.CurrentHP = EnemyStatus.MaxHP;

	// ゲーム開始時のプレイヤーの位置の取得
	Player = GetWorld()->GetFirstPlayerController()->GetPawn();

	// 攻撃判定ボックスにタグを追加
	AttackHitBox->ComponentTags.Add("EnemyAttack");
}


void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// プレイヤーがいるかどうか
	if (!Player) return;

	// Playerとの距離によって追跡、攻撃の決定
	float Distance = FVector::Dist(Player->GetActorLocation(), GetActorLocation());
	if (Distance < JumpAttack.AttackRange && Distance >= PunchAttack.AttackRange + 100)
	{
		TryAttack(JumpAttack);
	}
	else if (Distance < PunchAttack.AttackRange)
	{
		// 追跡を中止して攻撃
		TryAttack(PunchAttack);
	}
	else 
	{
		// Playerを追跡
		FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);
	}
}


void AEnemyBase::TryAttack(const FEnemyAttackData& AttackData)
{
	// 死んでいないか
	if (bIsDead) return;
	
	// 攻撃可能か 
	if (!bCanAttack) return;

	bCanAttack = false;
		
	Attack(AttackData);

	// 攻撃のクールダウンタイマー
	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&AEnemyBase::ResetAttack,
		AttackData.AttackCooldown,
		false
	);

}

// 攻撃モーションの再生
void AEnemyBase::Attack(const FEnemyAttackData& AttackData)
{
	CurrentAttackData = AttackData;
	PlayAnimMontage(CurrentAttackData.Montage);
}

// 攻撃判定の生成
void AEnemyBase::EnableAttackHitBox()
{
	// AttackHitBox, MeshComがあるか
	if (!AttackHitBox)
	{
		return;
	}
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	// 攻撃判定を特定の位置に付与
	AttackHitBox->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentAttackData.AttackSocketName
	);

	// 攻撃判定を設定
	AttackHitBox->SetRelativeLocation(CurrentAttackData.HitBoxOffset);
	AttackHitBox->SetRelativeRotation(CurrentAttackData.HitBoxRotation);
	AttackHitBox->SetBoxExtent(CurrentAttackData.HitBoxExtent);

	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackHitBox->UpdateOverlaps();

	TArray<AActor*> OverlappingActors;
	AttackHitBox->GetOverlappingActors(OverlappingActors);

	// 攻撃ヒット処理
	for (AActor* Actor : OverlappingActors)
	{
		ABossBattleCharacter* HitPlayer = Cast<ABossBattleCharacter>(Actor);
		if (HitPlayer)
		{
			// ダメージ処理
			HitPlayer->ReceiveEnemyDamage(CurrentAttackData.Damage);
			
			// 被ダメージ音
			if (CurrentAttackData.AttackSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					CurrentAttackData.AttackSound,
					GetActorLocation()
				);
			}

			// 攻撃判定の無効
			DisableAttackHitBox();
		}
	}
}

// 攻撃判定の削除
void AEnemyBase::DisableAttackHitBox()
{
	if (!AttackHitBox)
	{
		return;
	}
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	if (bIsInvincible || EnemyStatus.CurrentHP <= 0.0f) return;

	// ダメージ計算
	EnemyStatus.CurrentHP = FMath::Max(0.0f, EnemyStatus.CurrentHP - Damage);

	// 現在HPが0かどうか
	if (EnemyStatus.CurrentHP <= 0.0f)
	{
		Die();
	}

	// ダメージを受けると短時間無敵
	bIsInvincible = true;
	GetWorld()->GetTimerManager().SetTimer(
		InvincibleTimerHandle,
		this,
		&AEnemyBase::EndInvincible,
		EnemyStatus.InvincibleDuration,
		false
	);
}

// 敵の死亡時
void AEnemyBase::Die()
{
	bIsDead = true;

	// 死亡アニメを再生
	PlayAnimMontage(EnemyStatus.DeadMontage);

	// 遅延して削除
	SetLifeSpan(EnemyStatus.DeathDestroyDelay);
}

// 敵を削除
void AEnemyBase::DestroyEnemy()
{
	Destroy();
}

// 無敵時間の終了
void AEnemyBase::EndInvincible()
{
	bIsInvincible = false;
}

// 現在HPの読み込み
float AEnemyBase::GetCurrentHP() const
{
	return EnemyStatus.CurrentHP;
}

// 最大HPの読み込み
float AEnemyBase::GetMaxHP() const
{
	return EnemyStatus.MaxHP;
}
