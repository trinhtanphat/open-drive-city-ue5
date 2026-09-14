#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpenDriveDriverIntent.h"
#include "OpenDriveVehicleControllerComponent.generated.h"

UCLASS(ClassGroup=(OpenDrive), meta=(BlueprintSpawnableComponent))
class OPENDRIVECITY_API UOpenDriveVehicleControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOpenDriveVehicleControllerComponent();

    UFUNCTION(BlueprintCallable)
    void SetThrottle(float Value);

    UFUNCTION(BlueprintCallable)
    void SetBrake(float Value);

    UFUNCTION(BlueprintCallable)
    void SetSteering(float Value);

    UFUNCTION(BlueprintCallable)
    void SetHandbrake(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void ResetIntent();

    UFUNCTION(BlueprintPure)
    FOpenDriveDriverIntent GetIntent() const { return Intent; }

private:
    FOpenDriveDriverIntent Intent;
};
