// Copyright Epic Games, Inc. All Rights Reserved.


#include "BossBattlePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"

void ABossBattlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}

// ÉNÉäÉA,éÄñSéûÇÃUIï\é¶
void ABossBattlePlayerController::ShowEndScreen()
{
    if (EndWidgetClass)
    {
        EndWidget = CreateWidget<UUserWidget>(this, EndWidgetClass);
        EndWidget->AddToViewport();

        // UIëÄçÏ
        bShowMouseCursor = true;

        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(
            EMouseLockMode::DoNotLock
        );

        SetInputMode(InputMode);
        FlushPressedKeys();
    }
}