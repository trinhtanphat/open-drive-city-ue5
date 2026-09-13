#pragma once

#include "CoreMinimal.h"
#include "OpenDriveDriverIntent.generated.h"

USTRUCT(BlueprintType)
struct OPENDRIVECITY_API FOpenDriveDriverIntent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float Throttle = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Brake = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Steering = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bHandbrake = false;
};
