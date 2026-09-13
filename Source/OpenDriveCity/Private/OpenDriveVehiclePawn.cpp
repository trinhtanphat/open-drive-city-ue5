#include "OpenDriveVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "OpenDriveVehicleControllerComponent.h"

AOpenDriveVehiclePawn::AOpenDriveVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    DriverController = CreateDefaultSubobject<UOpenDriveVehicleControllerComponent>(TEXT("DriverController"));

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 650.0f;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
    CameraBoom->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 7.5f;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    ChaseCamera->FieldOfView = 90.0f;
}

void AOpenDriveVehiclePawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
    if (!Movement || !DriverController)
    {
        return;
    }

    const FOpenDriveDriverIntent& Intent = DriverController->GetIntent();
    Movement->SetThrottleInput(Intent.Throttle);
    Movement->SetBrakeInput(Intent.Brake);
    Movement->SetSteeringInput(Intent.Steering);
    Movement->SetHandbrakeInput(Intent.bHandbrake);
}

void AOpenDriveVehiclePawn::ResetChaseCamera()
{
    if (CameraBoom)
    {
        CameraBoom->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
    }
}

float AOpenDriveVehiclePawn::GetSpeedKph() const
{
    return GetVelocity().Size() * 0.036f;
}

int32 AOpenDriveVehiclePawn::GetCurrentGear() const
{
    const UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
    return Movement ? Movement->GetCurrentGear() : 0;
}
