// Copyright The Immortal Ask Team. All Rights Reserved.

#include "TheImmortalAskGameMode.h"

#include "TheImmortalAskPlayerController.h"

ATheImmortalAskGameMode::ATheImmortalAskGameMode()
{
	bStartPlayersAsSpectators = false;
	PlayerControllerClass = ATheImmortalAskPlayerController::StaticClass();
}
