#include "OpenDriveVehicleControllerComponent.h"

UOpenDriveVehicleControllerComponent::UOpenDriveVehicleControllerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UOpenDriveVehicleControllerComponent::SetThrottle(float Value)
{
    Intent.Throttle = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UOpenDriveVehicleControllerComponent::SetBrake(float Value)
{
    Intent.Brake = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UOpenDriveVehicleControllerComponent::SetSteering(float Value)
{
    Intent.Steering = FMath::Clamp(Value, -1.0f, 1.0f);
}

void UOpenDriveVehicleControllerComponent::SetHandbrake(bool bEnabled)
{
    Intent.bHandbrake = bEnabled;
}

void UOpenDriveVehicleControllerComponent::ResetIntent()
{
    Intent = FOpenDriveDriverIntent{};
}
