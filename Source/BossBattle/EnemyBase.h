// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "EnemyBase.generated.h"


USTRUCT(BlueprintType)
struct FEnemyStatus
{
	GENERATED_BODY()

	// 敵のHP変数
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float MaxHP = 100.0f;

	// 死亡モンタージュ
	UPROPERTY(EditAnywhere, Category = "Enemy");
	UAnimMontage* DeadMontage = nullptr;

	// 死亡後に消えるまでの時間
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float DeathDestroyDelay = 4.0f;

	// 敵の被ダメージ時の無敵時間
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float InvincibleDuration = 0.5f;

};


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
	FVector HitBoxExtent = FVector(20.0f, 20.0f, 20.0f);
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
	UPROPERTY(EditAnywhere, Category = "Enemy")
	FEnemyStatus Enemy;

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

	// 現在のHPを外から読めるようにする
	UFUNCTION(BlueprintPure)
	float GetCurrentHP() const { return CurrentHP; }

	UPROPERTY()
	AActor* Player;


private:
	// 敵の現在HP変数
	float CurrentHP;

	// 敵が無敵かどうか
	bool bIsInvincible = false;

	// 敵の無敵タイマー
	FTimerHandle InvincibleTimerHandle;

	// 無敵時間の終了時に呼び出し
	void EndInvincible();
};
