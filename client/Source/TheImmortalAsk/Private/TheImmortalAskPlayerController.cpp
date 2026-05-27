// Copyright The Immortal Ask Team. All Rights Reserved.

#include "TheImmortalAskPlayerController.h"

#include "Auth/AuthSession.h"
#include "UI/AuthWidgetHost.h"
#include "UI/CharacterSelectWidgetHost.h"
#include "UI/MainSceneHudWidget.h"
#include "World/ImmortalAskCharacter.h"
#include "World/ImmortalAskWorldBuilder.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogTheImmortalAskAuth, Log, All);

void ATheImmortalAskPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (const UGameInstance* GameInstance = GetGameInstance())
			{
				if (const UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>())
				{
					if (Session->IsInMainWorld() && Session->HasActiveCharacter())
					{
						EnterMainWorld();
						return;
					}
				}
			}

			ShowAuthUI();
		});
	}
}

void ATheImmortalAskPlayerController::ShowAuthUI()
{
	HideWidget(CharacterSelectWidget);
	HideWidget(MainHudWidget);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetInputMode(FInputModeUIOnly());

	if (!AuthWidget)
	{
		AuthWidget = CreateWidget<UAuthWidgetHost>(this, UAuthWidgetHost::StaticClass());
		if (AuthWidget)
		{
			AuthWidget->OnAuthFlowCompleted.AddDynamic(this, &ATheImmortalAskPlayerController::HandleAuthCompleted);
		}
	}

	if (AuthWidget)
	{
		AuthWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
		AuthWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
		AuthWidget->SetVisibility(ESlateVisibility::Visible);
		if (!AuthWidget->IsInViewport())
		{
			AuthWidget->AddToViewport(1000);
		}
		UE_LOG(LogTheImmortalAskAuth, Log, TEXT("Auth UI shown"));
	}
}

void ATheImmortalAskPlayerController::ShowCharacterSelectUI()
{
	HideWidget(AuthWidget);
	HideWidget(MainHudWidget);

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	if (!CharacterSelectWidget)
	{
		CharacterSelectWidget = CreateWidget<UCharacterSelectWidgetHost>(this, UCharacterSelectWidgetHost::StaticClass());
		if (CharacterSelectWidget)
		{
			CharacterSelectWidget->OnCharacterFlowCompleted.AddDynamic(this, &ATheImmortalAskPlayerController::HandleCharacterFlowCompleted);
		}
	}

	if (CharacterSelectWidget)
	{
		CharacterSelectWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
		CharacterSelectWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
		CharacterSelectWidget->SetVisibility(ESlateVisibility::Visible);
		if (!CharacterSelectWidget->IsInViewport())
		{
			CharacterSelectWidget->AddToViewport(1000);
		}
		UE_LOG(LogTheImmortalAskAuth, Log, TEXT("Character select UI shown"));
	}
}

void ATheImmortalAskPlayerController::ShowMainSceneHud()
{
	if (!MainHudWidget)
	{
		MainHudWidget = CreateWidget<UMainSceneHudWidget>(this, UMainSceneHudWidget::StaticClass());
	}

	if (MainHudWidget)
	{
		MainHudWidget->RefreshFromSession();
		MainHudWidget->SetAnchorsInViewport(FAnchors(0.f, 0.f, 1.f, 1.f));
		MainHudWidget->SetAlignmentInViewport(FVector2D(0.f, 0.f));
		MainHudWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (!MainHudWidget->IsInViewport())
		{
			MainHudWidget->AddToViewport(500);
		}
	}
}

void ATheImmortalAskPlayerController::EnterMainWorld()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>();
	if (!Session || !Session->HasActiveCharacter())
	{
		ShowAuthUI();
		return;
	}

	HideWidget(AuthWidget);
	HideWidget(CharacterSelectWidget);

	EnsureWorldBuilder(Session->GetCharacterMapId());

	if (APawn* ExistingPawn = GetPawn())
	{
		ExistingPawn->Destroy();
	}

	PlayerCharacter = SpawnPlayerCharacter(Session->GetSpawnLocation(), Session->GetSpawnRotation());
	if (PlayerCharacter)
	{
		Possess(PlayerCharacter);
		SetControlRotation(Session->GetSpawnRotation());
	}

	Session->SetInMainWorld(true);

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetInputMode(FInputModeGameOnly());
	ShowMainSceneHud();

	UE_LOG(
		LogTheImmortalAskAuth,
		Log,
		TEXT("Entered main world map=%d at %s"),
		Session->GetCharacterMapId(),
		*Session->GetSpawnLocation().ToString());
}

AImmortalAskWorldBuilder* ATheImmortalAskPlayerController::EnsureWorldBuilder(const int32 MapId)
{
	if (IsValid(WorldBuilder))
	{
		WorldBuilder->BuildForMap(MapId);
		return WorldBuilder;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	WorldBuilder = World->SpawnActor<AImmortalAskWorldBuilder>(AImmortalAskWorldBuilder::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (WorldBuilder)
	{
		WorldBuilder->BuildForMap(MapId);
	}

	return WorldBuilder;
}

AImmortalAskCharacter* ATheImmortalAskPlayerController::SpawnPlayerCharacter(const FVector& Location, const FRotator& Rotation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	return World->SpawnActor<AImmortalAskCharacter>(AImmortalAskCharacter::StaticClass(), Location, Rotation, Params);
}

void ATheImmortalAskPlayerController::HideWidget(UUserWidget* Widget)
{
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ATheImmortalAskPlayerController::HandleAuthCompleted()
{
	ShowCharacterSelectUI();
}

void ATheImmortalAskPlayerController::HandleCharacterFlowCompleted()
{
	EnterMainWorld();
}
