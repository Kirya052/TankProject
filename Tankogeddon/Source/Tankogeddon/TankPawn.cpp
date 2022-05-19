// Fill out your copyright notice in the Description page of Project Settings.


#include "TankPawn.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <Camera/CameraComponent.h>
#include "TankController.h"
#include "Kismet/KismetMathLibrary.h"
#include <Components/ArrowComponent.h>
#include "Cannon.h"



ATankPawn::ATankPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body Mesh"));
	RootComponent = BodyMesh;

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret mesh"));
	TurretMesh->SetupAttachment(BodyMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(BodyMesh);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	CannonSetupPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("CannonSetupPoint"));
	CannonSetupPoint->AttachToComponent(TurretMesh, FAttachmentTransformRules::KeepRelativeTransform);

}

void ATankPawn::MoveForward(float Value)
{
	TargetForwardAxisValue = Value;
}

void ATankPawn::RotateRight(float Value)
{
	TargetRotateAxisValue = Value;
}

void ATankPawn::BeginPlay()
{
	Super::BeginPlay();

	Controller = Cast<ATankController>(GetController());

	SetupCannon();
}

void ATankPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Tank movement
	FVector ActorLocation = GetActorLocation();
	FVector forwardVector = GetActorForwardVector();
	FVector moveposition = ActorLocation + forwardVector * MoveSpeed * TargetForwardAxisValue * DeltaSeconds;
	SetActorLocation(moveposition, true);

	//Tank rotation
	float CurrentTargetRotateAxisValue = FMath::Lerp(CurrentTargetRotateAxisValue, TargetRotateAxisValue, RotateInterpolationKey);
	float YawRotation = CurrentTargetRotateAxisValue * RotationSpeed * DeltaSeconds;
	//UE_LOG(LogTemp, Warning, TEXT("CurrentTargetRotateAxisValue : %f, TargetRotateAxisValue: %f "), CurrentTargetRotateAxisValue, TargetRotateAxisValue );
	FRotator ActorRotation = GetActorRotation();
	YawRotation = ActorRotation.Yaw + YawRotation;
	//YawRotation += ActorRotation.Yaw;
	FRotator NewRotation = FRotator(0.0f, YawRotation, 0.0f);
	SetActorRotation(NewRotation);

	//TurretRotation
	if (Controller)
	{
		FVector mousePosition = Controller->GetMousePosition();
		FRotator currRotation = TurretMesh->GetComponentRotation();
		FRotator targetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), mousePosition);
		targetRotation.Pitch = NewRotation.Pitch;
		targetRotation.Roll = NewRotation.Roll;

		TurretMesh->SetWorldRotation(FMath::Lerp(currRotation, targetRotation, TurretRotateInterpolationKey));
	}
}

void ATankPawn::Fire()
{
	if (Cannon)
	{
		Cannon->Fire();
	}
}

void ATankPawn::SetupCannon()
{
	if (Cannon)
	{
		Cannon->Destroy();
	}
	FActorSpawnParameters spawnParams;
	spawnParams.Instigator = this;
	spawnParams.Owner = this;

	Cannon = GetWorld()->SpawnActor<ACannon>(CannonClass, spawnParams);
	Cannon->AttachToComponent(CannonSetupPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

