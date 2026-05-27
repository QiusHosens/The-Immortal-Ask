// Copyright The Immortal Ask Team. All Rights Reserved.

#include "World/ImmortalAskCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AImmortalAskCharacter::AImmortalAskCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->JumpZVelocity = 520.f;
	Movement->AirControl = 0.25f;
	Movement->MaxWalkSpeed = 520.f;
	Movement->MinAnalogWalkSpeed = 20.f;
	Movement->BrakingDecelerationWalking = 1800.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 420.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 70.f);
	CameraBoom->CameraLagSpeed = 12.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 72.f;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BodyMesh);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 95.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
		BodyMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 1.05f));
	}

	if (SphereMesh.Succeeded())
	{
		HeadMesh->SetStaticMesh(SphereMesh.Object);
		HeadMesh->SetRelativeScale3D(FVector(0.55f));
	}

	if (BaseMaterial.Succeeded())
	{
		if (UMaterialInstanceDynamic* BodyMaterial = UMaterialInstanceDynamic::Create(BaseMaterial.Object, this))
		{
			BodyMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.28f, 0.55f));
			BodyMesh->SetMaterial(0, BodyMaterial);
		}

		if (UMaterialInstanceDynamic* HeadMaterial = UMaterialInstanceDynamic::Create(BaseMaterial.Object, this))
		{
			HeadMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.86f, 0.72f, 0.58f));
			HeadMesh->SetMaterial(0, HeadMaterial);
		}
	}
}

void AImmortalAskCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AImmortalAskCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AImmortalAskCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AImmortalAskCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AImmortalAskCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AImmortalAskCharacter::LookUp);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AImmortalAskCharacter::StartJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AImmortalAskCharacter::StopJump);
}

void AImmortalAskCharacter::MoveForward(const float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
	}
}

void AImmortalAskCharacter::MoveRight(const float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
	}
}

void AImmortalAskCharacter::Turn(const float Value)
{
	AddControllerYawInput(Value);
}

void AImmortalAskCharacter::LookUp(const float Value)
{
	AddControllerPitchInput(Value);
}

void AImmortalAskCharacter::StartJump()
{
	Jump();
}

void AImmortalAskCharacter::StopJump()
{
	StopJumping();
}
