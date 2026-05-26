// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AuthSession.generated.h"

UCLASS()
class THEIMMORTALASK_API UAuthSession : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SetSession(int64 InAccountId, const FString& InAccessToken, int64 InExpiresIn);
	void ClearSession();

	bool IsLoggedIn() const { return AccountId > 0 && !AccessToken.IsEmpty(); }
	int64 GetAccountId() const { return AccountId; }
	const FString& GetAccessToken() const { return AccessToken; }

private:
	int64 AccountId = 0;
	FString AccessToken;
	int64 ExpiresIn = 0;
};
