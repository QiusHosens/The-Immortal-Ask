// Copyright The Immortal Ask Team. All Rights Reserved.

#include "TheImmortalAskPlayerController.h"

#include "UI/AuthWidgetHost.h"

DEFINE_LOG_CATEGORY_STATIC(LogTheImmortalAskAuth, Log, All);

void ATheImmortalAskPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick([this]()
			{
				ShowAuthUI();
			});
		}
	}
}

void ATheImmortalAskPlayerController::ShowAuthUI()
{
	if (AuthWidget)
	{
		return;
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetInputMode(FInputModeUIOnly());

	AuthWidget = CreateWidget<UAuthWidgetHost>(this, UAuthWidgetHost::StaticClass());
	if (!AuthWidget)
	{
		UE_LOG(LogTheImmortalAskAuth, Error, TEXT("CreateWidget<UAuthWidgetHost> failed"));
		return;
	}

	AuthWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
	AuthWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
	AuthWidget->SetVisibility(ESlateVisibility::Visible);
	AuthWidget->AddToViewport(1000);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([AuthWidget = AuthWidget]()
		{
			const FVector2D Size = AuthWidget->GetCachedGeometry().GetLocalSize();
			UE_LOG(LogTheImmortalAskAuth, Log, TEXT("Auth UI viewport size: %s"), *Size.ToString());
		});
	}

	UE_LOG(LogTheImmortalAskAuth, Log, TEXT("Auth UI added to viewport"));
}
