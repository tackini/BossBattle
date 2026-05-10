// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "EnemyBase.generated.h"


// 敵のステータス構造体
USTRUCT(BlueprintType)
struct FEnemyStatus
{
	GENERATED_BODY()

	// 最大HP変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHP = 100.0f;

	// 現在HP変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHP;

	// 死亡モンタージュ
	UPROPERTY(EditAnywhere, Category = "Status");
	UAnimMontage* DeadMontage = nullptr;

	// 死亡後に消えるまでの時間
	UPROPERTY(EditAnywhere, Category = "Status")
	float DeathDestroyDelay = 4.0f;

	// 敵の被ダメージ時の無敵時間
	UPROPERTY(EditAnywhere, Category = "Status")
	float InvincibleDuration = 0.5f;

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


protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


	/* Combat Runtime */

	// 現在攻撃可能か
	bool bCanAttack = true;

	// 敵が死んでいるかどうか
	bool bIsDead = false;

	// 攻撃クールタイマー
	FTimerHandle AttackTimerHandle;

	// 攻撃データ
	FEnemyAttackData CurrentAttackData;

	// 攻撃の当たり判定コリジョン
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* AttackHitBox;


	/* Combat Fuctions */

	void TryAttack(const FEnemyAttackData& AttackData);

	void Attack(const FEnemyAttackData& AttackData);
	
	UFUNCTION(BlueprintCallable)
	void EnableAttackHitBox();

	UFUNCTION(BlueprintCallable)
	void DisableAttackHitBox();

	void ResetAttack();
	
	void DestroyEnemy();
	
	void Die();


	/* Struct */

	// EnemyStatus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	FEnemyStatus EnemyStatus;

	// PunchAttack
	UPROPERTY(EditAnywhere, Category = "Combat|AttackData")
	FEnemyAttackData PunchAttack;

	// JumpAttack
	UPROPERTY(EditAnywhere, Category = "Combat|AttackData")
	FEnemyAttackData JumpAttack;


public:
	// 剣から呼び出す
	UFUNCTION(BlueprintCallable)
	void ReceiveSwordDamage(float Damage);

	UPROPERTY()
	AActor* Player;

	/* Getter */
	UFUNCTION(BlueprintCallable, Category = "Status")
	float GetCurrentHP() const;

	UFUNCTION(BlueprintCallable, Category = "Status")
	float GetMaxHP() const;


private:
	// 敵が無敵かどうか
	bool bIsInvincible = false;

	// 敵の無敵タイマー
	FTimerHandle InvincibleTimerHandle;

	// 無敵時間の終了時に呼び出し
	void EndInvincible();
};
