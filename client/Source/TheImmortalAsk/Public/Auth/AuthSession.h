// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Auth/CharacterApiClient.h"
#include "AuthSession.generated.h"

UCLASS()
class THEIMMORTALASK_API UAuthSession : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SetSession(int64 InAccountId, const FString& InAccessToken, int64 InExpiresIn);
	void ClearSession();

	void SetSelectedRealm(int32 InRealmId, const FString& InRealmName);
	void SetActiveCharacter(const FEnterWorldResult& Result);
	void ClearActiveCharacter();
	void SetInMainWorld(bool bInMainWorld);

	bool IsLoggedIn() const { return AccountId > 0 && !AccessToken.IsEmpty(); }
	bool HasActiveCharacter() const { return !CharacterPublicId.IsEmpty(); }
	bool IsInMainWorld() const { return bInMainWorld; }

	int64 GetAccountId() const { return AccountId; }
	const FString& GetAccessToken() const { return AccessToken; }
	int32 GetSelectedRealmId() const { return SelectedRealmId; }
	const FString& GetSelectedRealmName() const { return SelectedRealmName; }
	const FString& GetCharacterPublicId() const { return CharacterPublicId; }
	const FString& GetCharacterName() const { return CharacterName; }
	int32 GetCharacterLevel() const { return CharacterLevel; }
	int32 GetCharacterMapId() const { return CharacterMapId; }
	const FString& GetRealmStageName() const { return RealmStageName; }
	const FString& GetSectName() const { return SectName; }
	FVector GetSpawnLocation() const { return SpawnLocation; }
	FRotator GetSpawnRotation() const { return SpawnRotation; }

private:
	int64 AccountId = 0;
	FString AccessToken;
	int64 ExpiresIn = 0;

	int32 SelectedRealmId = 1;
	FString SelectedRealmName;

	FString CharacterPublicId;
	FString CharacterName;
	int32 CharacterLevel = 1;
	int32 CharacterMapId = 1001;
	FString RealmStageName;
	FString SectName;
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	bool bInMainWorld = false;
};
