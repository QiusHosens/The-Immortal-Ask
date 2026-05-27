// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/AuthSession.h"

#include "World/ImmortalAskMapRegistry.h"

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
	ClearActiveCharacter();
}

void UAuthSession::SetSelectedRealm(const int32 InRealmId, const FString& InRealmName)
{
	SelectedRealmId = InRealmId;
	SelectedRealmName = InRealmName;
}

void UAuthSession::SetActiveCharacter(const FEnterWorldResult& Result)
{
	CharacterPublicId = Result.PublicId;
	CharacterName = Result.Name;
	CharacterLevel = Result.Level;
	CharacterMapId = Result.MapId;
	RealmStageName = Result.RealmStageName;
	SectName = Result.SectName;
	SpawnLocation = FImmortalAskMapRegistry::ResolveSpawnLocation(Result.MapId, Result.PosX, Result.PosY, Result.PosZ);
	SpawnRotation = FRotator(0.f, Result.RotationYaw, 0.f);
}

void UAuthSession::ClearActiveCharacter()
{
	CharacterPublicId.Reset();
	CharacterName.Reset();
	CharacterLevel = 1;
	CharacterMapId = 1001;
	RealmStageName.Reset();
	SectName.Reset();
	SpawnLocation = FVector::ZeroVector;
	SpawnRotation = FRotator::ZeroRotator;
	bInMainWorld = false;
}

void UAuthSession::SetInMainWorld(const bool bInMainWorldValue)
{
	bInMainWorld = bInMainWorldValue;
}
