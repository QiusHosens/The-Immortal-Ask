// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FRealmInfo
{
	int32 Id = 0;
	FString Code;
	FString Name;
	int32 MaxCharacters = 3;
};

struct FCharacterSummary
{
	FString PublicId;
	FString Name;
	int32 Level = 1;
	int32 CombatPower = 0;
	FString RealmStageName;
	FString SectName;
};

struct FSectOption
{
	int32 Id = 0;
	FString Name;
	FString Element;
};

struct FSpiritRootOption
{
	int32 Id = 0;
	FString Name;
	int32 Rarity = 1;
};

struct FCreationOptions
{
	TArray<FSectOption> Sects;
	TArray<FSpiritRootOption> SpiritRoots;
};

struct FEnterWorldResult
{
	FString PublicId;
	FString Name;
	int32 Level = 1;
	int32 MapId = 1001;
	float PosX = 0.f;
	float PosY = 0.f;
	float PosZ = 0.f;
	float RotationYaw = 0.f;
	FString RealmStageName;
	FString SectName;
};

using FCharacterListCallback = TFunction<void(bool bSuccess, const FString& Message, const TArray<FCharacterSummary>& Characters)>;
using FRealmListCallback = TFunction<void(bool bSuccess, const FString& Message, const TArray<FRealmInfo>& Realms)>;
using FCreationOptionsCallback = TFunction<void(bool bSuccess, const FString& Message, const FCreationOptions& Options)>;
using FCreateCharacterCallback = TFunction<void(bool bSuccess, const FString& Message, const FString& PublicId, const FString& Name)>;
using FEnterWorldCallback = TFunction<void(bool bSuccess, const FString& Message, const FEnterWorldResult& Result)>;

class THEIMMORTALASK_API FCharacterApiClient
{
public:
	static void ListRealms(const UObject* WorldContext, FRealmListCallback Callback);
	static void GetCreationOptions(const UObject* WorldContext, FCreationOptionsCallback Callback);

	static void ListCharacters(
		const UObject* WorldContext,
		const FString& AccessToken,
		int32 RealmId,
		FCharacterListCallback Callback);

	static void CreateCharacter(
		const UObject* WorldContext,
		const FString& AccessToken,
		int32 RealmId,
		const FString& Name,
		int32 Gender,
		int32 SectId,
		int32 SpiritRootId,
		FCreateCharacterCallback Callback);

	static void EnterWorld(
		const UObject* WorldContext,
		const FString& AccessToken,
		const FString& PublicId,
		FEnterWorldCallback Callback);

private:
	static FString GetGatewayBaseUrl();
	static void SendAuthorizedGet(
		const FString& Url,
		const FString& AccessToken,
		TFunction<void(bool bSuccess, const FString& Message, int32 ResponseCode, const FString& Body)> Callback);
	static void SendAuthorizedPost(
		const FString& Url,
		const FString& AccessToken,
		const FString& JsonBody,
		TFunction<void(bool bSuccess, const FString& Message, int32 ResponseCode, const FString& Body)> Callback);
};
