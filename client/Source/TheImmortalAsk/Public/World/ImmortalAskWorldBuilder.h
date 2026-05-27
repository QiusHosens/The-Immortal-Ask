// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImmortalAskWorldBuilder.generated.h"

UCLASS()
class THEIMMORTALASK_API AImmortalAskWorldBuilder : public AActor
{
	GENERATED_BODY()

public:
	AImmortalAskWorldBuilder();

	void BuildForMap(int32 MapId);

private:
	struct FPropDefinition
	{
		FName MeshName;
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FVector Scale = FVector(1.f);
		FLinearColor Color = FLinearColor::White;
	};

	void BuildLuoxiaVillage(const FVector& Origin);
	void BuildGenericOutpost(const FVector& Origin, const FString& Label, const FLinearColor& AccentColor);
	void SpawnProp(const FVector& Origin, const FPropDefinition& Prop);
	void EnsureLighting(const FVector& Origin, const FLinearColor& SunColor);
	void SpawnLabel(const FVector& Origin, const FString& Label);

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors;

	int32 BuiltMapId = INDEX_NONE;
};
