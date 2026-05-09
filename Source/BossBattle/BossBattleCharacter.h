// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Components/BoxComponent.h"
#include "BossBattleCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class BOSSBATTLE_API ABossBattleCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	
public:
	ABossBattleCharacter();


protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;


	/* Components */

	// SwordSwingPivotをUEで見えるようにする
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SwordSwingPivot;

	// SwordRollPivotをUEで見えるようにする
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SwordRollPivot;

	// SwordMeshをUEで見えるようにする
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SwordMesh;


	/* Sword Settings */

	// X軸の剣の移動制限
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sword")
	float MaxSwordRangeX = 40.0f;

	// Y軸の剣の移動制限
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sword")
	float MaxSwordRangeY = 15.0f;
	
	// ダメージ判定に必要な最低速度
	UPROPERTY(EditAnywhere, Category = "Sword")
	float DamageSpeedThreshould = 50.0f;

	// 剣のダメージ量
	UPROPERTY(EditAnywhere, Category = "Sword")
	float SwordDamage = 30.0f;


	/* Audio Setting */

	// 剣のヒット音
	UPROPERTY(EditAnywhere, Category = "Sword")
	USoundBase* SwordHitSound = nullptr;

	// 剣のパリィ音
	UPROPERTY(EditAnywhere, Category = "Sword")
	USoundBase* SwordParrySound = nullptr;


	/* Sword Runtime */

	FVector2D SwordOffset;			// 剣の現在位置
	FVector2D PreviousSwordOffset;	// 前フレームの剣の位置
	FVector2D SwingVelocity;		// 剣の移動速度
	FVector CurrentSwordSwingDir;	// 剣の向き
	bool bIsAttacking = false;		// 攻撃中かどうか
	

	/* Sword Functions */

	void OnAttackStart();
	void OnAttackEnd();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// 剣の衝突判定処理
	UFUNCTION()
	void OnSwordHit(
		UPrimitiveComponent* OverlappedComp,	// 触れた自分側のコンポーネント
		AActor* OtherActor,						// 触れた相手のActor
		UPrimitiveComponent* OtherComp,			// 触れた相手のコンポーネント
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// 剣のヒットストップ開始
	UFUNCTION()
	void StartHitStop(float Duration, float TimeScale);


	/* Input Action */

	// 攻撃アクションの入力
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* AttackAction;

	// 視点操作入力
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;



	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


	UFUNCTION(BlueprintCallable)
	void ReceiveEnemyDamage(float Damage);

	// 現在のHPを外から読めるようにする
	UFUNCTION(BlueprintPure)
	float GetCurrentHP() const { return CurrentHP; }


private:
	// 剣の当たり判定コリジョン
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* SwordHitBox;


	UPROPERTY(EditAnywhere, Category = "Player")
	float MaxHP = 100;
	float CurrentHP;

	// プレイヤーの被ダメージ時の無敵時間
	UPROPERTY(EditAnywhere, Category = "Player")
	float InvincibleDuration = 0.5f;
	bool bIsInvincible = false;

	// プレイヤーの無敵タイマー
	FTimerHandle InvincibleTimerHandle;

	// 剣のヒットストップタイマー
	FTimerHandle HitStopTimerHandle;

	// 無敵時間の終了
	void EndInvincible();

	// 剣のヒットストップ終了
	UFUNCTION()
	void EndHitStop();

};

