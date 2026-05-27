// Copyright The Immortal Ask Team. All Rights Reserved.

#include "World/ImmortalAskWorldBuilder.h"

#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "World/ImmortalAskMapRegistry.h"

namespace
{
	UStaticMesh* LoadBasicMesh(const FName MeshName)
	{
		return LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"), *MeshName.ToString(), *MeshName.ToString()));
	}

	UMaterialInterface* LoadBaseMaterial()
	{
		return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
}

AImmortalAskWorldBuilder::AImmortalAskWorldBuilder()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AImmortalAskWorldBuilder::BuildForMap(const int32 MapId)
{
	if (BuiltMapId == MapId)
	{
		return;
	}

	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Reset();

	const FImmortalAskMapInfo Info = FImmortalAskMapRegistry::GetMapInfo(MapId);
	switch (MapId)
	{
	case 1100:
		BuildGenericOutpost(Info.WorldOrigin, Info.DisplayName, FLinearColor(0.18f, 0.42f, 0.32f));
		break;
	case 1200:
		BuildGenericOutpost(Info.WorldOrigin, Info.DisplayName, FLinearColor(0.12f, 0.24f, 0.48f));
		break;
	case 1300:
		BuildGenericOutpost(Info.WorldOrigin, Info.DisplayName, FLinearColor(0.55f, 0.28f, 0.12f));
		break;
	case 1001:
	default:
		BuildLuoxiaVillage(Info.WorldOrigin);
		break;
	}

	BuiltMapId = MapId;
}

void AImmortalAskWorldBuilder::BuildLuoxiaVillage(const FVector& Origin)
{
	EnsureLighting(Origin, FLinearColor(1.f, 0.86f, 0.62f));
	SpawnLabel(Origin + FVector(0.f, 0.f, 520.f), TEXT("落霞村"));

	const TArray<FPropDefinition> Props = {
		{TEXT("Plane"), FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, FVector(55.f, 55.f, 1.f), FLinearColor(0.12f, 0.28f, 0.12f)},
		{TEXT("Plane"), FVector(0.f, 0.f, 2.f), FRotator::ZeroRotator, FVector(18.f, 18.f, 1.f), FLinearColor(0.18f, 0.34f, 0.16f)},
		{TEXT("Cube"), FVector(-900.f, -500.f, 90.f), FRotator(0.f, 15.f, 0.f), FVector(3.6f, 2.8f, 1.8f), FLinearColor(0.45f, 0.28f, 0.18f)},
		{TEXT("Cube"), FVector(-300.f, -700.f, 90.f), FRotator(0.f, -10.f, 0.f), FVector(3.2f, 2.6f, 1.7f), FLinearColor(0.42f, 0.26f, 0.16f)},
		{TEXT("Cube"), FVector(350.f, -650.f, 90.f), FRotator(0.f, 25.f, 0.f), FVector(3.4f, 2.7f, 1.9f), FLinearColor(0.48f, 0.30f, 0.18f)},
		{TEXT("Cube"), FVector(900.f, -450.f, 90.f), FRotator(0.f, -18.f, 0.f), FVector(3.8f, 3.0f, 2.0f), FLinearColor(0.40f, 0.24f, 0.14f)},
		{TEXT("Cube"), FVector(-650.f, 450.f, 90.f), FRotator(0.f, 8.f, 0.f), FVector(3.0f, 2.4f, 1.6f), FLinearColor(0.44f, 0.27f, 0.17f)},
		{TEXT("Cube"), FVector(250.f, 550.f, 90.f), FRotator(0.f, -6.f, 0.f), FVector(3.3f, 2.5f, 1.7f), FLinearColor(0.43f, 0.26f, 0.16f)},
		{TEXT("Cylinder"), FVector(0.f, 250.f, 160.f), FRotator::ZeroRotator, FVector(1.2f, 1.2f, 3.2f), FLinearColor(0.55f, 0.42f, 0.24f)},
		{TEXT("Cube"), FVector(0.f, 250.f, 320.f), FRotator(0.f, 45.f, 0.f), FVector(2.4f, 2.4f, 0.8f), FLinearColor(0.62f, 0.48f, 0.28f)},
		{TEXT("Cube"), FVector(-1200.f, 1200.f, 260.f), FRotator(0.f, 30.f, 0.f), FVector(8.f, 8.f, 5.f), FLinearColor(0.22f, 0.24f, 0.28f)},
		{TEXT("Cube"), FVector(1300.f, -1100.f, 320.f), FRotator(0.f, -20.f, 0.f), FVector(10.f, 10.f, 6.f), FLinearColor(0.20f, 0.22f, 0.26f)},
		{TEXT("Cube"), FVector(1500.f, 1400.f, 280.f), FRotator(0.f, 12.f, 0.f), FVector(9.f, 9.f, 5.f), FLinearColor(0.24f, 0.26f, 0.30f)},
		{TEXT("Cube"), FVector(-1500.f, -1300.f, 300.f), FRotator(0.f, -35.f, 0.f), FVector(11.f, 11.f, 6.f), FLinearColor(0.18f, 0.20f, 0.24f)},
	};

	for (const FPropDefinition& Prop : Props)
	{
		SpawnProp(Origin, Prop);
	}

	const TArray<FVector> TreeLocations = {
		FVector(-500.f, 900.f, 0.f),
		FVector(700.f, 850.f, 0.f),
		FVector(-950.f, -950.f, 0.f),
		FVector(1050.f, 700.f, 0.f),
		FVector(-200.f, 1100.f, 0.f),
		FVector(450.f, -1000.f, 0.f),
	};

	for (const FVector& TreeLocation : TreeLocations)
	{
		SpawnProp(Origin, {TEXT("Cylinder"), TreeLocation + FVector(0.f, 0.f, 80.f), FRotator::ZeroRotator, FVector(0.35f, 0.35f, 1.6f), FLinearColor(0.34f, 0.22f, 0.12f)});
		SpawnProp(Origin, {TEXT("Sphere"), TreeLocation + FVector(0.f, 0.f, 220.f), FRotator::ZeroRotator, FVector(1.6f), FLinearColor(0.10f, 0.42f, 0.18f)});
	}
}

void AImmortalAskWorldBuilder::BuildGenericOutpost(const FVector& Origin, const FString& Label, const FLinearColor& AccentColor)
{
	EnsureLighting(Origin, AccentColor);
	SpawnLabel(Origin + FVector(0.f, 0.f, 480.f), Label);

	const TArray<FPropDefinition> Props = {
		{TEXT("Plane"), FVector::ZeroVector, FRotator::ZeroRotator, FVector(40.f, 40.f, 1.f), FLinearColor(0.14f, 0.18f, 0.14f)},
		{TEXT("Cube"), FVector(0.f, 0.f, 120.f), FRotator::ZeroRotator, FVector(4.f, 4.f, 2.4f), AccentColor * 0.8f},
		{TEXT("Cylinder"), FVector(-700.f, -500.f, 120.f), FRotator::ZeroRotator, FVector(1.5f, 1.5f, 3.f), AccentColor},
		{TEXT("Cylinder"), FVector(700.f, 500.f, 120.f), FRotator::ZeroRotator, FVector(1.5f, 1.5f, 3.f), AccentColor},
	};

	for (const FPropDefinition& Prop : Props)
	{
		SpawnProp(Origin, Prop);
	}
}

void AImmortalAskWorldBuilder::SpawnProp(const FVector& Origin, const FPropDefinition& Prop)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UStaticMesh* Mesh = LoadBasicMesh(Prop.MeshName);
	if (!Mesh)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		Origin + Prop.Location,
		Prop.Rotation,
		Params);

	if (!MeshActor)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetWorldScale3D(Prop.Scale);
	MeshComponent->SetMobility(EComponentMobility::Movable);

	if (UMaterialInterface* BaseMaterial = LoadBaseMaterial())
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshActor))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Prop.Color);
			MeshComponent->SetMaterial(0, DynamicMaterial);
		}
	}

	SpawnedActors.Add(MeshActor);
}

void AImmortalAskWorldBuilder::EnsureLighting(const FVector& Origin, const FLinearColor& SunColor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(Origin + FVector(0.f, 0.f, 1200.f), FRotator(-42.f, 35.f, 0.f), Params))
	{
		if (UDirectionalLightComponent* LightComponent = Sun->GetComponent())
		{
			LightComponent->SetIntensity(7.f);
			LightComponent->SetLightColor(SunColor);
			LightComponent->SetMobility(EComponentMobility::Movable);
		}
		SpawnedActors.Add(Sun);
	}

	if (ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(Origin + FVector(0.f, 0.f, 600.f), FRotator::ZeroRotator, Params))
	{
		if (USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent())
		{
			SkyLightComponent->SetIntensity(1.1f);
			SkyLightComponent->SetMobility(EComponentMobility::Movable);
			SkyLightComponent->RecaptureSky();
		}
		SpawnedActors.Add(SkyLight);
	}

	if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(Origin, FRotator::ZeroRotator, Params))
	{
		if (UExponentialHeightFogComponent* FogComponent = Fog->GetComponent())
		{
			FogComponent->SetFogDensity(0.018f);
			FogComponent->SetFogInscatteringColor(FLinearColor(0.45f, 0.58f, 0.72f));
			FogComponent->SetMobility(EComponentMobility::Movable);
		}
		SpawnedActors.Add(Fog);
	}
}

void AImmortalAskWorldBuilder::SpawnLabel(const FVector& Origin, const FString& Label)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* LabelActor = World->SpawnActor<AActor>(AActor::StaticClass(), Origin, FRotator(0.f, 180.f, 0.f), Params);
	if (!LabelActor)
	{
		return;
	}

	USceneComponent* LabelRoot = NewObject<USceneComponent>(LabelActor, TEXT("LabelRoot"));
	LabelActor->SetRootComponent(LabelRoot);
	LabelRoot->RegisterComponent();

	UTextRenderComponent* TextComponent = NewObject<UTextRenderComponent>(LabelActor, TEXT("MapLabel"));
	TextComponent->SetupAttachment(LabelActor->GetRootComponent());
	TextComponent->RegisterComponent();
	TextComponent->SetHorizontalAlignment(EHTA_Center);
	TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	TextComponent->SetWorldSize(120.f);
	TextComponent->SetTextRenderColor(FColor(255, 220, 140));
	TextComponent->SetText(FText::FromString(Label));

	SpawnedActors.Add(LabelActor);
}
