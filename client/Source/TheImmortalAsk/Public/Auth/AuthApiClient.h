// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UObject;

using FAuthMessageCallback = TFunction<void(bool bSuccess, const FString& Message)>;
using FLoginCallback = TFunction<void(bool bSuccess, const FString& Message, int64 AccountId, const FString& AccessToken, int64 ExpiresIn)>;

class THEIMMORTALASK_API FAuthApiClient
{
public:
	static void Register(
		const UObject* WorldContext,
		const FString& Username,
		const FString& Password,
		const FString& Email,
		FAuthMessageCallback Callback);

	static void Login(
		const UObject* WorldContext,
		const FString& Username,
		const FString& Password,
		FLoginCallback Callback);

private:
	static FString GetGatewayBaseUrl();
};
