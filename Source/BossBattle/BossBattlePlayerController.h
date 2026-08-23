// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BossBattlePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *
 */
UCLASS()
class BOSSBATTLE_API ABossBattlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	// Begin Actor interface


protected:

	virtual void BeginPlay() override;

	// End Actor interface

	/* UI */

	// ÉQÅ[ÉÄèIóπéûÇÃâÊñ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> EndWidgetClass;

	UPROPERTY()
	UUserWidget* EndWidget;

public:

	/* Function */

	void ShowEndScreen();
};
