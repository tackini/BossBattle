// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossBattleCharacter.h"
#include "EnemyBase.h"
#include "BossBattleProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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
	SwordHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SwordHitBox->SetBoxExtent(FVector(10.0f, 5.0f, 40.0f));
	SwordHitBox->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

	SwordHitBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SwordHitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SwordHitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void ABossBattleCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// SwordHitBoxに触れた時、OnSwordHitを呼び出す
	if (SwordHitBox)
	{
		SwordHitBox->OnComponentBeginOverlap.AddDynamic(
			this,
			&ABossBattleCharacter::OnSwordHit
		);
	}
}


//	右クリックを押すと攻撃開始
void ABossBattleCharacter::OnAttackStart()
{
	SwordOffset = FVector2D(0, 0);
	bIsAttacking = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Attack Start"));

	if (SwordSwingPivot)
	{
		// 手から剣を外す
		SwordSwingPivot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// カメラの前に配置
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			FVector CamLoc;
			FRotator CamRot;
			PC->GetPlayerViewPoint(CamLoc, CamRot);		// 視点の位置と向きを取得

			FVector Forward = CamRot.Vector();
			FVector TargetPos = CamLoc + Forward * 100.0f;

			SwordSwingPivot->SetWorldLocation(TargetPos);
		}
	}
}

// 右クリックを離すと攻撃中止
void ABossBattleCharacter::OnAttackEnd()
{
	bIsAttacking = false;
	UE_LOG(LogTemp, Warning, TEXT("Attack End"));

	if (SwordSwingPivot)
	{
		SwordSwingPivot->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			TEXT("hand_r_Socket")
		);
	}
}


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
		// 剣の速度が一定以上ならダメージを与える
		if (SwingVelocity.Size() >= DamageSpeedThreshould)
		{

			Enemy->ReceiveSwordDamage(SwordDamage);

			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				FString::Printf(TEXT("Hit! HP: %.1f"), Enemy->GetCurrentHP())
			);

		}
	}
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


	if (bIsAttacking) {
	
		float MaxRange_X = 30.0f, MaxRange_Y = 10.0f;
		SwordOffset.X += -LookAxisVector.X * 3.0f;
		SwordOffset.Y += -LookAxisVector.Y * 3.0f;
		SwordOffset.X = FMath::Clamp(SwordOffset.X, -MaxRange_X, MaxRange_X);
		SwordOffset.Y = FMath::Clamp(SwordOffset.Y, -MaxRange_Y, MaxRange_Y);

	} else if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void ABossBattleCharacter::Tick(float DeltaTime)
{
	// Super;;Tick(DeltaTime);

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
			
			// 剣の移動制限
			float MaxRange_X = 40.0f, MaxRange_Y = 15.0f;

			// 剣が中心にあるほど奥にセットされる
			float OffsetSize = SwordOffset.Size();
			float MaxOffsetSize = 35;
			float NormalizedOffset = 1.0f - (OffsetSize / MaxOffsetSize);

			// カメラ前に操作用の空間を作る(空間の中心点を決める)
			FVector BasePos = CamLoc + Forward * (80.0f + NormalizedOffset * 60.0f);

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

			FVector MoveDir = TargetPos - CurrentPos;
			SwingVelocity = FVector2D(MoveDir.Y, MoveDir.Z);

			FVector NewPos = FMath::VInterpTo(
				CurrentPos,
				TargetPos,
				DeltaTime,
				6.0f
			);
			// 剣の位置をセット
			SwordSwingPivot->SetWorldLocation(NewPos);



			// 縦に剣を振る
			float NormalizedY = SwordOffset.Y / MaxRange_Y;
			FRotator SwingRot = CamRot;
			SwingRot.Pitch = CamRot.Pitch + (-90.0f + NormalizedY * 60.0f);
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
					DeltaTime, 5.0f
				);
				SwordRollPivot->SetRelativeRotation(NewRollRot);
			}

			// 剣の振る速度が一定より大きいなら攻撃
			if (MoveDir.Size() > DamageSpeedThreshould)
			{
				if (SwordHitBox)
				{
					SwordHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

				}
			} else
			{
				// 速度が足りないときはダメージ判定をOFF
				if (SwordHitBox)
				{
					SwordHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}

		}

	}

}