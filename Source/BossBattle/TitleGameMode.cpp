// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleGameMode.h"
#include "GameFramework/PlayerController.h"

ATitleGameMode::ATitleGameMode() : Super()
{
	
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(
			EMouseLockMode::DoNotLock
		);
		PC->SetInputMode(InputMode);
	}
}

