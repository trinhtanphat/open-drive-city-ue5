#pragma once

#include "CoreMinimal.h"
#include "OpenDriveVehicleTelemetry.generated.h"

USTRUCT(BlueprintType)
struct OPENDRIVECITY_API FOpenDriveVehicleTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="OpenDrive|Telemetry")
    float SpeedKph = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="OpenDrive|Telemetry")
    int32 CurrentGear = 0;

    UPROPERTY(BlueprintReadOnly, Category="OpenDrive|Telemetry")
    bool bHandbrake = false;

    UPROPERTY(BlueprintReadOnly, Category="OpenDrive|Telemetry")
    bool bMoving = false;
};
