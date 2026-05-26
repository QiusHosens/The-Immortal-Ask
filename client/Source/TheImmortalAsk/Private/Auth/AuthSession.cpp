// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/AuthSession.h"

void UAuthSession::SetSession(int64 InAccountId, const FString& InAccessToken, int64 InExpiresIn)
{
	AccountId = InAccountId;
	AccessToken = InAccessToken;
	ExpiresIn = InExpiresIn;
}

void UAuthSession::ClearSession()
{
	AccountId = 0;
	AccessToken.Reset();
	ExpiresIn = 0;
}
