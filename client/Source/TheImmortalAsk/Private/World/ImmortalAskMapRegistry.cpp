// Copyright The Immortal Ask Team. All Rights Reserved.

#include "World/ImmortalAskMapRegistry.h"

FImmortalAskMapInfo FImmortalAskMapRegistry::GetMapInfo(const int32 MapId)
{
	switch (MapId)
	{
	case 1100:
		return {1100, TEXT("青云山"), FVector(20000.f, 0.f, 0.f)};
	case 1200:
		return {1200, TEXT("玄冥泽"), FVector(0.f, 20000.f, 0.f)};
	case 1300:
		return {1300, TEXT("烈阳荒漠"), FVector(-20000.f, 0.f, 0.f)};
	case 1001:
	default:
		return {1001, TEXT("落霞村"), FVector::ZeroVector};
	}
}

FVector FImmortalAskMapRegistry::ResolveSpawnLocation(const int32 MapId, const float PosX, const float PosY, const float PosZ)
{
	const FImmortalAskMapInfo Info = GetMapInfo(MapId);
	const FVector ServerLocation(PosX, PosY, PosZ);

	if (!ServerLocation.IsNearlyZero(1.f))
	{
		return Info.WorldOrigin + ServerLocation + FVector(0.f, 0.f, 120.f);
	}

	return Info.WorldOrigin + FVector(0.f, -600.f, 120.f);
}
