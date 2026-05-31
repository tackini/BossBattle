// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "BossBattleCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
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

	// 攻撃判定ボックスにタグを追加
	AttackHitBox->ComponentTags.Add("EnemyAttack");

	DynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
}


void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 攻撃判定の処理
void AEnemyBase::TryAttack(const FEnemyAttackData& AttackData)
{
	// 死んでいないか
	if (bIsDead) return;
	
	// 攻撃可能か 
	if (!bCanAttack) return;


	// Playerの取得
	if (!Player)
	{
		Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	if (!Player) return;

	// プレイヤーの方向
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.0f;

	if (ToPlayer.IsNearlyZero()) return;


	FRotator TargetRot = ToPlayer.Rotation();

	// 向きのセット
	SetActorRotation(TargetRot);

	// 攻撃した
	bCanAttack = false;
	
	// 攻撃開始
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

// 攻撃判定のコリジョンON
void AEnemyBase::EnableAttackHitBox()
{
	// 攻撃したActorの保存をリセット
	HitActors.Empty();

	// AttackHitBox, MeshComがあるか
	if (!IsValid(AttackHitBox))
	{
		return;
	}
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!IsValid(MeshComp))
	{
		return;
	}

	// AttackHitBoxをソケットに付与
	AttackHitBox->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentAttackData.AttackSocketName
	);

	// 最初の攻撃判定ボックスの所得
	PreviousAttackHitBoxLocation = AttackHitBox->GetComponentLocation();

	// 攻撃判定の位置、向き、大きさをセット
	AttackHitBox->SetRelativeLocation(CurrentAttackData.HitBoxOffset);
	AttackHitBox->SetRelativeRotation(CurrentAttackData.HitBoxRotation);
	AttackHitBox->SetBoxExtent(CurrentAttackData.HitBoxExtent);

	// コリジョンをON
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackHitBox->UpdateOverlaps();
}


// 攻撃判定のコリジョンOFF
void AEnemyBase::DisableAttackHitBox()
{
	if (!AttackHitBox)
	{
		return;
	}
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


// AttackTrace
void AEnemyBase::AttackTrace(FEnemyAttackData& AttackData)
{
	FVector CurrentLocation = AttackHitBox->GetComponentLocation();

	TArray<FHitResult> HitResults;

	GetWorld()->SweepMultiByChannel(
		HitResults,
		PreviousAttackHitBoxLocation,
		CurrentLocation,
		FQuat(AttackHitBox->GetComponentQuat()),
		ECC_Pawn,
		FCollisionShape::MakeBox(AttackHitBox->GetScaledBoxExtent())
	);

	// 当たったものを順番に確認
	for (const FHitResult& Hit : HitResults)
	{

		AActor* HitActor = Hit.GetActor();
		// 当たったものがActorか
		if (!IsValid(HitActor)) continue;

		// 一度攻撃したActorを無視
		if (HitActors.Contains(HitActor))
		{
			continue;
		}

		// 当たったActorがPlayerか
		ABossBattleCharacter* HitPlayer = Cast<ABossBattleCharacter>(HitActor);
		if (HitPlayer)
		{
			// 一度攻撃したActorを保存
			HitActors.Add(HitActor);

			// ダメージ処理
			HitPlayer->ReceiveEnemyDamage(CurrentAttackData.Damage);

			// ダメージ音
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
			break;
		}
	}

	PreviousAttackHitBoxLocation = CurrentLocation;
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

	// 受けた攻撃回数のカウント
	HitCount++;

	// 一定回数攻撃するとバックジャンプ
	if (HitCount >= BackJumpHitThreshould)
	{
		AAIController* AICon = Cast<AAIController>(GetController());

		if (AICon && AICon->GetBlackboardComponent())
		{
			AICon->GetBlackboardComponent()->SetValueAsBool(
				"ShouldBackJump",
				true
			);
		}
	}
	
	//ダメージを受けると色を赤く光らせる
	FlashRed();

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


void AEnemyBase::AttackDeflected()
{
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (CurrentAttackData.AttackDeflectedMontage)
		{
			// 今の攻撃Montageを止める
			Anim->Montage_Stop(0.1f);

			SetStun(true);

			Anim->Montage_Play(CurrentAttackData.AttackDeflectedMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(
				this,
				&AEnemyBase::OnAttackDeflectedEnded
			);

			Anim->Montage_SetEndDelegate(
				EndDelegate,
				CurrentAttackData.AttackDeflectedMontage
			);
		}
	}
}

// スタンの終了
void AEnemyBase::OnAttackDeflectedEnded(UAnimMontage* Montage, bool bInterrupted)
{
	SetStun(false);
}


// スタン状態をセット
void AEnemyBase::SetStun(bool NewStun)
{
	bIsStun = NewStun;

	if (AAIController* AICon =
		Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB =
			AICon->GetBlackboardComponent())
		{
			BB->SetValueAsBool(
				TEXT("bIsStun"),
				bIsStun
			);
		}
	}
}


// BackJump
void AEnemyBase::BackJump()
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon)
	{
		AICon->StopMovement();
	}

	// Playerの取得
	if (!IsValid(Player))
	{
		Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	if (!IsValid(Player)) return;

	// プレイヤーの方向
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.0f;

	if (ToPlayer.IsNearlyZero()) return;


	FRotator TargetRot = ToPlayer.Rotation();

	// 向きのセット
	SetActorRotation(TargetRot);

	// ジャンプモンタージュ再生
	if (EnemyStatus.JumpMontage)
	{
		PlayAnimMontage(EnemyStatus.JumpMontage);
	}
}


// BackJump時の後ろ移動
void AEnemyBase::LaunchBackJump()
{
	FVector BackDir = -GetActorForwardVector();
	BackDir.Z = 0.0f;
	BackDir.Normalize();

	FVector LaunchVelocity = BackDir * 800.0f;
	LaunchVelocity.Z = 500.0f;

	LaunchCharacter(LaunchVelocity, true, true);
}


// 被攻撃回数をリセット
void AEnemyBase::ResetCount()
{
	AAIController* AICon = Cast<AAIController>(GetController());
	// BackJuumoを無効
	if (AICon && AICon->GetBlackboardComponent())
	{
		AICon->GetBlackboardComponent()->SetValueAsBool(
			"ShouldBackJump",
			false
		);
		HitCount = 0;
	}
}

// 一瞬だけMeshを赤くする
void AEnemyBase::FlashRed()
{
	if (!DynamicMaterial) return;

	// 色を変化
	DynamicMaterial->SetVectorParameterValue(
		TEXT("BodyColor"),
		FLinearColor::Red
	);

	// TimerHnadleのリセット
	GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		FlashTimerHandle,
		this,
		&AEnemyBase::ResetColor,
		0.1f,
		false
	);
}

// 色を戻す
void AEnemyBase::ResetColor()
{
	if (!DynamicMaterial) return;
	// 色のリセット
	DynamicMaterial->SetVectorParameterValue(
		TEXT("BodyColor"),
		FLinearColor::White
	);
}

// 敵の死亡時
void AEnemyBase::Die()
{
	bIsDead = true;
	bCanAttack = false;

	// 敵の死亡通知
	OnEnemyDead.Broadcast(this);

	// 死亡アニメを再生
	if (EnemyStatus.DeadMontage)
	{
		PlayAnimMontage(EnemyStatus.DeadMontage);
	}

	// 遅延してDestroyEnemyを呼ぶ
	GetWorld()->GetTimerManager().SetTimer(
		EnemyDestroyTimerHandle,
		this,
		&AEnemyBase::DestroyEnemy,
		EnemyStatus.DeathDestroyDelay,
		false
	);

	AAIController* AICon = Cast<AAIController>(GetController());
	if (AICon)
	{
		// AIControllerを止める
		AICon->StopMovement();

		// BhaviorTreeを止める
		UBrainComponent* Brain = AICon->GetBrainComponent();
		if (Brain)
		{
			Brain->StopLogic(TEXT("Enemy Dead"));
		}
	}

	DisableAttackHitBox();
	GetCharacterMovement()->StopMovementImmediately();
}

// 敵を削除
void AEnemyBase::DestroyEnemy()
{
	// 爆発エフェクトの再生
	if (DeathExplosion)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DeathExplosion,
			GetActorLocation(),
			GetActorRotation(),
			GetActorScale3D()
		);
	}

	// 爆発音の再生
	if (DeathExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			AEnemyBase::DeathExplosionSound,
			GetActorLocation()
		);
	}

	// 敵の削除
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

// 無敵の読み込み
bool AEnemyBase::GetbIsInvincible() const
{
	return bIsInvincible;
}
