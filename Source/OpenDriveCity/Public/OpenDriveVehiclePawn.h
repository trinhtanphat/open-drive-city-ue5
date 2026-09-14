#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "OpenDriveVehicleTelemetry.h"
#include "OpenDriveVehiclePawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UOpenDriveVehicleControllerComponent;

UCLASS()
class OPENDRIVECITY_API AOpenDriveVehiclePawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    AOpenDriveVehiclePawn();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="OpenDrive|Camera")
    void ResetChaseCamera();

    UFUNCTION(BlueprintCallable, Category="OpenDrive|Camera")
    void AdjustCameraYaw(float DeltaDegrees);

    UFUNCTION(BlueprintCallable, Category="OpenDrive|Camera")
    void AdjustCameraPitch(float DeltaDegrees);

    UFUNCTION(BlueprintCallable, Category="OpenDrive|Camera")
    void AdjustCameraZoom(float DeltaUnits);

    UFUNCTION(BlueprintCallable, Category="OpenDrive|Vehicle")
    void RecoverVehicle();

    UFUNCTION(BlueprintPure, Category="OpenDrive|Telemetry")
    float GetSpeedKph() const;

    UFUNCTION(BlueprintPure, Category="OpenDrive|Telemetry")
    int32 GetCurrentGear() const;

    UFUNCTION(BlueprintPure, Category="OpenDrive|Telemetry")
    FOpenDriveVehicleTelemetry GetTelemetry() const;

    UFUNCTION(BlueprintPure, Category="OpenDrive|Input")
    UOpenDriveVehicleControllerComponent* GetDriverController() const { return DriverController; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OpenDrive|Input")
    TObjectPtr<UOpenDriveVehicleControllerComponent> DriverController;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OpenDrive|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OpenDrive|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;
};
