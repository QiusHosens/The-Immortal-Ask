// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuthSettings.generated.h"

UCLASS(Config = Game, DefaultConfig)
class THEIMMORTALASK_API UAuthSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Auth")
	FString GatewayBaseUrl = TEXT("http://127.0.0.1:8080");

	static const UAuthSettings* Get();
};
