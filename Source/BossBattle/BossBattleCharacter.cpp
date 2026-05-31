// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossBattleCharacter.h"
#include "EnemyBase.h"
#include "BossBattleProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/Gameplaystatics.h"
#include "BossHUDWidget.h"
#include "TimerManager.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ABossBattleCharacter

ABossBattleCharacter::ABossBattleCharacter()
{
	// SwordSwingPivot、SwordRollPivot、SwordMeshの実体を作成
	SwordSwingPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SwordSwingPivot"));
	SwordRollPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SwordRollPivot"));
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	
	// Mesh->SwordSwingPivot->SwordRollPivot->SwordMeshと接続
	SwordSwingPivot->SetupAttachment(GetMesh(), TEXT("hand_r_Socket"));
	SwordRollPivot->SetupAttachment(SwordSwingPivot);
	SwordMesh->SetupAttachment(SwordRollPivot);

	// 剣の当たり判定Boxを作成
	SwordHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SwordHitBox"));
	SwordHitBox->SetupAttachment(SwordMesh);
	SwordHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SwordHitBox->SetBoxExtent(FVector(10.0f, 5.0f, 40.0f));
	SwordHitBox->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));


	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.0f, 0.0f, 60.0f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.0f, 0.0f, -150.0f));

}

void ABossBattleCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	PlayerStatus.CurrentHP = PlayerStatus.MaxHP;

	// SwordHitBoxに触れた時、OnSwordHitを呼び出す
	if (SwordHitBox)
	{
		SwordHitBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&ABossBattleCharacter::OnSwordHit
		);
	}

	// Tagをつける
	SwordMesh->ComponentTags.Add("PlayerSword");
	SwordHitBox->ComponentTags.Add("PlayerSword");

	// デバック
	UE_LOG(LogTemp, Warning, TEXT("Start"));

	// レベル上にいる敵の取得
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AEnemyBase::StaticClass(),
		FoundEnemies
	);
	if (FoundEnemies.Num() > 0)
	{
		CurrentEnemy = Cast<AEnemyBase>(FoundEnemies[0]);
	}

	// HUDWidgetの作成
	if (HUDWidgetClass)
	{
		// UIの作成
		HUDWidget = CreateWidget<UBossHUDWidget>(GetWorld(), HUDWidgetClass);

		if (HUDWidget)
		{
			// Uの表示
			HUDWidget->AddToViewport();

			// プレイヤーのHPPercentの計算
			float PlayerHPPercent = PlayerStatus.MaxHP > 0.0f
				? PlayerStatus.CurrentHP / PlayerStatus.MaxHP
				: 0.0f;

			// PlayerHPPercentの更新
			HUDWidget->UpdatePlayerHP(PlayerHPPercent);

			if (CurrentEnemy)
			{
				// 敵のHPPercentの計算
				float EnemyHPPercent = CurrentEnemy->GetMaxHP() > 0.0f
					? CurrentEnemy->GetCurrentHP() / CurrentEnemy->GetMaxHP()
					: 0.0f;

				// EnemyHPPercentの更新
				HUDWidget->UpdateEnemyHP(EnemyHPPercent);

				// EnemyNameの更新
				HUDWidget->UpdateEnemyName(CurrentEnemy->GetEnemyName());

				// HUDWodgetのOnEnemyDeadを呼ぶ
				CurrentEnemy->OnEnemyDead.AddDynamic(HUDWidget, &UBossHUDWidget::OnEnemyDead);
			}
		}
	}
}


//	右クリックを押すと攻撃開始
void ABossBattleCharacter::OnAttackStart()
{
	SwordOffset = FVector2D(0, 0);
	bIsAttacking = true;

	// 当たり判定をON
	SwordHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (SwordSwingPivot)
	{
		// 手から剣を外す
		SwordSwingPivot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// カメラの前に配置
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			LockedCameraRot = PC->GetControlRotation();
			FVector CamLoc;
			FRotator CamRot;
			// 視点の位置と向きを取得
			PC->GetPlayerViewPoint(CamLoc, CamRot);

			FVector Forward = CamRot.Vector();
			FVector TargetPos = CamLoc + Forward * 50.0f;

			SwordSwingPivot->SetWorldLocation(TargetPos);
		}
	}
}

// 右クリックを離すと攻撃中止
void ABossBattleCharacter::OnAttackEnd()
{
	bIsAttacking = false;

	// 当たり判定OFF
	SwordHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (SwordSwingPivot)
	{
		SwordSwingPivot->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			TEXT("hand_r_Socket")
		);
	}
}

// 攻撃ヒット処理
void ABossBattleCharacter::OnSwordHit(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{

	// 振れたActorが敵かどうか確認
	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	if (Enemy)
	{
		// 剣の速度が一定以上かつ無敵でないならダメージを与える
		if (SwingVelocity.Size() >= DamageSpeedThreshould && Enemy->GetbIsInvincible() == false)
		{
			// ダメージ処理
			Enemy->ReceiveSwordDamage(SwordDamage);

			// 敵のHPPercentの更新
			if (HUDWidget)
			{
				float HPPercent = Enemy->GetMaxHP() > 0.0f
					? Enemy->GetCurrentHP() / Enemy->GetMaxHP()
					: 0.0f;

				HUDWidget->UpdateEnemyHP(HPPercent);
			}

			// 剣ヒット音
			if (SwordHitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					SwordHitSound,
					Enemy->GetActorLocation()
				);
			}

			// ヒットストップ
			StartHitStop(0.013f, 0.1f);

		}
	}


	// 剣が敵の攻撃に当たったか
	if (OtherComp && OtherComp->ComponentHasTag("EnemyAttack"))
	{
		// 敵の攻撃方向を正規化して取得
		FVector EnemyAttackDir = (GetActorLocation() - OtherComp->GetComponentLocation()).GetSafeNormal();

		// 敵の攻撃と剣の攻撃の向きの内積を計算
		float Dot = FVector::DotProduct(CurrentSwordSwingDir, EnemyAttackDir);

		// デバック
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("Dot: %.1f"), Dot)
		);

		// 弾くと一瞬無敵
		bIsInvincible = true;

		GetWorldTimerManager().SetTimer(
			InvincibleTimerHandle,
			this,
			&ABossBattleCharacter::EndInvincible,
			InvincibleDuration,
			false
		);

		// 敵の攻撃判定を無効化
		OtherComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (Dot <= ParryThreshould)
		{
			// パリィ音
			if (SwordParrySound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					SwordParrySound,
					OtherComp->GetComponentLocation()
				);
			}

			if (ParrySparkSystem)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ParrySparkSystem,
					OtherComp->GetComponentLocation()
					);
			}

			// ヒットストップ
			StartHitStop(0.03, 0.1);
		}
		else
		{
			Enemy->AttackDeflected();
		}
	}

}

void ABossBattleCharacter::EndInvincible()
{
	bIsInvincible = false;
}

// 被ダメージ処理
void ABossBattleCharacter::ReceiveEnemyDamage(float Damage)
{
	if (bIsInvincible || PlayerStatus.CurrentHP <= 0.0f) return;

	PlayerStatus.CurrentHP = FMath::Max(0.0f, PlayerStatus.CurrentHP - Damage);

	// HPPercentの計算と更新
	if (HUDWidget)
	{
		float HPPercent = PlayerStatus.MaxHP > 0.0f
			? PlayerStatus.CurrentHP / PlayerStatus.MaxHP
			: 0.0f;

		HUDWidget->UpdatePlayerHP(HPPercent);
	}
}

// ヒットストップ開始
void ABossBattleCharacter::StartHitStop(float Duration, float TimeScale)
{
	// どのくらい遅くするか
	UGameplayStatics::SetGlobalTimeDilation(
		GetWorld(),
		TimeScale
	);

	// タイマーのリセット
	GetWorldTimerManager().ClearTimer(
		HitStopTimerHandle
	);

	// 遅延してヒットストップ終了
	GetWorldTimerManager().SetTimer(
		HitStopTimerHandle,
		this,
		&ABossBattleCharacter::EndHitStop,
		Duration,
		false
	);
}

// ヒットストップ終了
void ABossBattleCharacter::EndHitStop()
{
	UGameplayStatics::SetGlobalTimeDilation(
		GetWorld(),
		1.0f
	);
}


//////////////////////////////////////////////////////////////////////////// Input

void ABossBattleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABossBattleCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABossBattleCharacter::Look);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABossBattleCharacter::OnAttackStart);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ABossBattleCharacter::OnAttackEnd);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ABossBattleCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ABossBattleCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 剣の操作に変更
	if (bIsAttacking) 
	{
		// 剣の移動制限
		SwordOffset.X += -LookAxisVector.X * 3.0f;
		SwordOffset.Y += -LookAxisVector.Y * 3.0f;
		SwordOffset.X = FMath::Clamp(SwordOffset.X, -MaxSwordRangeX, MaxSwordRangeX);
		SwordOffset.Y = FMath::Clamp(SwordOffset.Y, -MaxSwordRangeY, MaxSwordRangeY);

	} else if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void ABossBattleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAttacking && SwordSwingPivot) {

		APlayerController* PC = Cast<APlayerController>(GetController());

		if (PC) {

			// 視点の位置と向きを取得
			FVector CamLoc;
			FRotator CamRot;
			PC->GetPlayerViewPoint(CamLoc, CamRot);

			
			FVector Forward = CamRot.Vector();
			FVector Right = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
			FVector Up = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Z);
			
			// 剣が中心にあるほど奥にセットされる
			float OffsetSize = SwordOffset.Size();
			float MaxOffsetSize = 60;
			float NormalizedOffset = 1.0f - (OffsetSize / MaxOffsetSize);

			// カメラ前に操作用の空間を作る(空間の中心点を決める)
			FVector BasePos = CamLoc + Forward * (80.0f + NormalizedOffset * 50.0f);

			// 空間内で上下左右にどのくらい動かすかの値を決定(中心点からのずれ)
			FVector Offset =
				Right * -SwordOffset.X * 3.0f + Up * SwordOffset.Y * 5.0f;

			// プレイヤーの移動速度を取得
			FVector PlayerVelocity = GetVelocity();

			// 元の位置からどれくらい動いたか(プレイヤーの移動も加算)
			FVector TargetPos = BasePos + Offset + PlayerVelocity * DeltaTime;

			// 剣の動きを滑らかにする(目標地点に少しずつ移動させる)
			FVector CurrentPos = SwordSwingPivot->GetComponentLocation();
			if (!SwordSwingPivot) return;

			// 剣の移動方向を取得
			FVector MoveDir = TargetPos - CurrentPos;
			SwingVelocity = FVector2D(MoveDir.Y, MoveDir.Z);

			// 剣の移動方向を正規化して取得
			//CurrentSwordSwingDir = MoveDir.GetSafeNormal();

			FVector NewPos = FMath::VInterpTo(
				CurrentPos,
				TargetPos,
				DeltaTime,
				5.0f
			);

			// 剣を振る方向のベクトルを取得
			FVector ActualMoveDir = NewPos - CurrentPos;
			CurrentSwordSwingDir = ActualMoveDir.GetSafeNormal();

			// 剣の位置をセット
			SwordSwingPivot->SetWorldLocation(NewPos);


			// 剣を縦と横に振る
			FRotator SwingRot = CamRot;
			float NormalizedY = SwordOffset.Y / MaxSwordRangeY;
			float NormalizedX = SwordOffset.X / MaxSwordRangeX;
			SwingRot.Pitch = CamRot.Pitch + (-90.0f + NormalizedY * 40.0f);
			SwingRot.Roll = CamRot.Roll + (0.0f + NormalizedX * -30.0f);
			SwordSwingPivot->SetWorldRotation(SwingRot);


			// 剣の振る速度が一定以上なら回転
			if (MoveDir.Size() > 5.0f)
			{
				// カメラのローカル空間での移動方向を取得
				float LocalY = FVector::DotProduct(MoveDir.GetSafeNormal(), Right);
				float LocalZ = FVector::DotProduct(MoveDir.GetSafeNormal(), Up);
				float BladeAngle = FMath::RadiansToDegrees(
					FMath::Atan2(LocalY, LocalZ)
				);
				BladeAngle = FMath::Clamp(BladeAngle, -80.0f, 80.0f);

				FRotator CurrentRollRot = SwordRollPivot->GetRelativeRotation();
				FRotator TargetRollRot = FRotator(0.0f, BladeAngle, 0.0f);
				FRotator NewRollRot = FMath::RInterpTo(
					CurrentRollRot,
					TargetRollRot,
					DeltaTime,
					5.0f
				);
				SwordRollPivot->SetRelativeRotation(NewRollRot);
			}

			FRotator TargetRot = LockedCameraRot;
			TargetRot.Yaw -= NormalizedX * CameraFollowYawMax;
			TargetRot.Pitch += NormalizedY * CameraFollowPitchMax;

			FRotator NewRot = FMath::RInterpTo(
				GetControlRotation(),
				TargetRot,
				DeltaTime,
				CameraFollowInterpSpeed
			);

			GetController()->SetControlRotation(NewRot);
		}
	}
}