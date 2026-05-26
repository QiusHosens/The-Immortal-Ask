// Copyright The Immortal Ask Team. All Rights Reserved.

#include "TheImmortalAskGameMode.h"

#include "UI/AuthWidgetHost.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATheImmortalAskGameMode::ATheImmortalAskGameMode()
{
	bStartPlayersAsSpectators = false;
}

void ATheImmortalAskGameMode::BeginPlay()
{
	Super::BeginPlay();

	// BeginPlay 时 PlayerController 可能尚未生成，下一帧再尝试一次。
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
			{
				EnsureAuthUI(PlayerController);
			}
		});
	}
}

void ATheImmortalAskGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer && NewPlayer->IsLocalPlayerController())
	{
		EnsureAuthUI(NewPlayer);
	}
}

void ATheImmortalAskGameMode::EnsureAuthUI(APlayerController* PlayerController)
{
	if (!PlayerController || AuthWidgetInstance.IsValid())
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetInputMode(FInputModeUIOnly());

	if (UAuthWidgetHost* AuthWidget = CreateWidget<UAuthWidgetHost>(PlayerController, UAuthWidgetHost::StaticClass()))
	{
		AuthWidget->AddToViewport(100);
		AuthWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
		AuthWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
		AuthWidgetInstance = AuthWidget;
	}
}
