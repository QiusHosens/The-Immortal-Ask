// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TheImmortalAskPlayerController.generated.h"

class UAuthWidgetHost;

UCLASS()
class THEIMMORTALASK_API ATheImmortalAskPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	void ShowAuthUI();

	UPROPERTY()
	TObjectPtr<UAuthWidgetHost> AuthWidget;
};
