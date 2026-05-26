// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TheImmortalAskGameMode.generated.h"

class UAuthWidgetHost;

UCLASS()
class THEIMMORTALASK_API ATheImmortalAskGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATheImmortalAskGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	void EnsureAuthUI(APlayerController* PlayerController);

	TWeakObjectPtr<UAuthWidgetHost> AuthWidgetInstance;
};
