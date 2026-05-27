// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TheImmortalAskPlayerController.generated.h"

class UAuthWidgetHost;
class UCharacterSelectWidgetHost;
class UMainSceneHudWidget;
class AImmortalAskCharacter;
class AImmortalAskWorldBuilder;

UCLASS()
class THEIMMORTALASK_API ATheImmortalAskPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	void ShowAuthUI();
	void ShowCharacterSelectUI();
	void ShowMainSceneHud();
	void EnterMainWorld();
	void HideWidget(UUserWidget* Widget);
	AImmortalAskWorldBuilder* EnsureWorldBuilder(int32 MapId);
	AImmortalAskCharacter* SpawnPlayerCharacter(const FVector& Location, const FRotator& Rotation);

	UFUNCTION()
	void HandleAuthCompleted();

	UFUNCTION()
	void HandleCharacterFlowCompleted();

	UPROPERTY()
	TObjectPtr<UAuthWidgetHost> AuthWidget;

	UPROPERTY()
	TObjectPtr<UCharacterSelectWidgetHost> CharacterSelectWidget;

	UPROPERTY()
	TObjectPtr<UMainSceneHudWidget> MainHudWidget;

	UPROPERTY()
	TObjectPtr<AImmortalAskWorldBuilder> WorldBuilder;

	UPROPERTY()
	TObjectPtr<AImmortalAskCharacter> PlayerCharacter;
};
