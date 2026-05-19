// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystem.h"
#include "EnemyBase.generated.h"

class AEnemyBase;

// 全体に死亡通知を渡す
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDead, AEnemyBase*, DeadEnemy);

// 敵のステータス構造体
USTRUCT(BlueprintType)
struct FEnemyStatus
{
	GENERATED_BODY()

	// 敵の名前
	UPROPERTY(EditAnywhere, Category = "Status")
	FText EnemyName;

	// 最大HP変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHP = 100.0f;

	// 現在HP変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHP;

	// バックジャンプモンタージュ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	UAnimMontage* JumpMontage = nullptr;

	// 死亡モンタージュ
	UPROPERTY(EditAnywhere, Category = "Status");
	UAnimMontage* DeadMontage = nullptr;

	// 死亡後に消えるまでの時間
	UPROPERTY(EditAnywhere, Category = "Status")
	float DeathDestroyDelay = 2.9f;

	// 敵の被ダメージ時の無敵時間
	UPROPERTY(EditAnywhere, Category = "Status")
	float InvincibleDuration = 0.1f;
	
};

// 敵の攻撃構造体
USTRUCT(BlueprintType)
struct FEnemyAttackData
{
	GENERATED_BODY()

	// 攻撃モンタージュ
	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* Montage = nullptr;

	// 攻撃ダメージ
	UPROPERTY(EditAnywhere, Category = "Attack")
	float Damage = 20.0f;

	// 攻撃可能距離
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 300.0f;

	// 攻撃クールタイム
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackCooldown = 2.0f;

	// 攻撃判定の場所
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName AttackSocketName = "hand_r";

	// 攻撃判定の位置(調整用)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HitBoxOffset = FVector::ZeroVector;

	// 攻撃判定の回転(調整用)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator HitBoxRotation = FRotator::ZeroRotator;

	// 攻撃判定の大きさ
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HitBoxExtent = FVector(50.0f, 80.0f, 80.0f);

	// 攻撃音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	USoundBase* AttackSound = nullptr;
};


UCLASS()
class BOSSBATTLE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()


public:
	AEnemyBase();

	// 敵死亡通知
	UPROPERTY(BlueprintAssignable)
	FOnEnemyDead OnEnemyDead;


protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


	/* Combat Runtime */

	// 現在攻撃可能か
	bool bCanAttack = true;

	// 敵が死んでいるかどうか
	bool bIsDead = false;

	// 隙中かどうか
	UPROPERTY(BlueprintReadOnly)
	bool bIsRecovery = false;

	// 被攻撃ヒット回数
	UPROPERTY(BlueprintReadOnly)
	int32 HitCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BackJumpHitThreshould = 5;

	UFUNCTION(BlueprintCallable)
	void ResetCount();

	// 攻撃クールタイマー
	FTimerHandle AttackTimerHandle;

	// 攻撃データ
	FEnemyAttackData CurrentAttackData;

	// 攻撃の当たり判定コリジョン
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* AttackHitBox;

	/* AttackTrace */
	// 前フレームのAttackHitBoxの位置
	UPROPERTY()
	FVector PreviousAttackHitBoxLocation;

	// AttackHitBoxに一度触れたActorを入れる変数
	UPROPERTY()
	TSet<AActor*> HitActors;


	/* Combat Fuctions */

	// 攻撃出来るか
	UFUNCTION(BlueprintCallable)
	void TryAttack(const FEnemyAttackData& AttackData);

	// 攻撃処理
	void Attack(const FEnemyAttackData& AttackData);
	
	// 当たり判定のセット
	UFUNCTION(BlueprintCallable)
	void EnableAttackHitBox();

	UFUNCTION(BlueprintCallable)
	void DisableAttackHitBox();

	// AttackTrace
	UFUNCTION(BlueprintCallable)
	void AttackTrace(FEnemyAttackData& AttackData);

	// 攻撃可能処理
	void ResetAttack();
	
	// 敵の削除
	void DestroyEnemy();
	
	void Die();

	// 後隙のセット
	UFUNCTION(BlueprintCallable)
	void StartRecovery();

	UFUNCTION(BlueprintCallable)
	void EndRecovery();

	// BackJump
	UFUNCTION(BlueprintCallable)
	void BackJump();
	UFUNCTION(BlueprintCallable)
	void LaunchBackJump();

	// Color
	void FlashRed();
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;


	/* Struct */

	// EnemyStatus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	FEnemyStatus EnemyStatus;

	// PunchAttack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|AttackData")
	FEnemyAttackData PunchAttack;

	// JumpAttack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|AttackData")
	FEnemyAttackData JumpAttack;


public:
	// 剣から呼び出す
	UFUNCTION(BlueprintCallable)
	void ReceiveSwordDamage(float Damage);

	UPROPERTY()
	AActor* Player;

	// 死亡時の爆発
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* DeathExplosion;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* DeathExplosionSound;


	/* Getter */
	UFUNCTION(BlueprintCallable, Category = "Status")
	float GetCurrentHP() const;

	UFUNCTION(BlueprintCallable, Category = "Status")
	float GetMaxHP() const;

	FText GetEnemyName() const
	{
		return EnemyStatus.EnemyName;
	}

	bool GetbIsInvincible() const;


private:
	// 敵が無敵かどうか
	bool bIsInvincible = false;

	// 削除タイマー
	FTimerHandle EnemyDestroyTimerHandle;

	// 敵の無敵タイマー
	FTimerHandle InvincibleTimerHandle;

	// 無敵時間の終了時に呼び出し
	void EndInvincible();

	// 敵の色タイマー
	FTimerHandle FlashTimerHandle;

	// 敵の色のリセット
	void ResetColor();
};
