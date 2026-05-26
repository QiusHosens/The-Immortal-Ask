// Copyright The Immortal Ask Team. All Rights Reserved.

#include "TheImmortalAskGameMode.h"

#include "UI/AuthWidgetHost.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ATheImmortalAskGameMode::ATheImmortalAskGameMode()
{
	bStartPlayersAsSpectators = false;
}

void ATheImmortalAskGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetInputMode(FInputModeUIOnly());

	if (UAuthWidgetHost* AuthWidget = CreateWidget<UAuthWidgetHost>(PlayerController, UAuthWidgetHost::StaticClass()))
	{
		AuthWidget->AddToViewport(100);
	}
}
