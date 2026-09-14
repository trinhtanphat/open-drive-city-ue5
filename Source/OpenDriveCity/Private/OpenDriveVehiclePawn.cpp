#include "OpenDriveVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

void AOpenDriveVehiclePawn::AdjustCameraYaw(float DeltaDegrees)
{
    if (!CameraBoom)
    {
        return;
    }

    FRotator Rotation = CameraBoom->GetRelativeRotation();
    Rotation.Yaw = FRotator::NormalizeAxis(Rotation.Yaw + DeltaDegrees);
    CameraBoom->SetRelativeRotation(Rotation);
}

void AOpenDriveVehiclePawn::AdjustCameraPitch(float DeltaDegrees)
{
    if (!CameraBoom)
    {
        return;
    }

    FRotator Rotation = CameraBoom->GetRelativeRotation();
    Rotation.Pitch = FMath::Clamp(Rotation.Pitch + DeltaDegrees, -70.0f, 25.0f);
    CameraBoom->SetRelativeRotation(Rotation);
}

void AOpenDriveVehiclePawn::RecoverVehicle()
{
    const float Yaw = GetActorRotation().Yaw;
    const FVector LiftedLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
    SetActorLocationAndRotation(
        LiftedLocation, FRotator(0.0f, Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);

    if (USkeletalMeshComponent* VehicleMesh = GetMesh())
    {
        VehicleMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        VehicleMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }

    if (UChaosWheeledVehicleMovementComponent* Movement =
        Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
    {
        Movement->SetThrottleInput(0.0f);
        Movement->SetBrakeInput(0.0f);
        Movement->SetSteeringInput(0.0f);
        Movement->SetHandbrakeInput(false);
    }

    if (DriverController)
    {
        DriverController->SetThrottle(0.0f);
        DriverController->SetBrake(0.0f);
        DriverController->SetSteering(0.0f);
        DriverController->SetHandbrake(false);
    }

    ResetChaseCamera();
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

FOpenDriveVehicleTelemetry AOpenDriveVehiclePawn::GetTelemetry() const
{
    FOpenDriveVehicleTelemetry Telemetry;
    Telemetry.SpeedKph = GetSpeedKph();
    Telemetry.CurrentGear = GetCurrentGear();
    Telemetry.bMoving = Telemetry.SpeedKph > 0.5f;
    Telemetry.bHandbrake = DriverController ? DriverController->GetIntent().bHandbrake : false;
    return Telemetry;
}
