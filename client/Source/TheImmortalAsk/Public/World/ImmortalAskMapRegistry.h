// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FImmortalAskMapInfo
{
	int32 MapId = 1001;
	FString DisplayName;
	FVector WorldOrigin = FVector::ZeroVector;
};

class THEIMMORTALASK_API FImmortalAskMapRegistry
{
public:
	static FImmortalAskMapInfo GetMapInfo(int32 MapId);
	static FVector ResolveSpawnLocation(int32 MapId, float PosX, float PosY, float PosZ);
};
