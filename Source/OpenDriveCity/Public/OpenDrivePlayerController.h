#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OpenDrivePlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class OPENDRIVECITY_API AOpenDrivePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AOpenDrivePlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void BuildRuntimeInputMap();
    void RegisterRuntimeInputMap();
    void ApplyThrottle(const FInputActionValue& Value);
    void ApplyBrake(const FInputActionValue& Value);
    void ApplySteering(const FInputActionValue& Value);
    void ApplyHandbrake(const FInputActionValue& Value);
    void ReleaseHandbrake(const FInputActionValue& Value);
    void ResetCamera(const FInputActionValue& Value);
    void RecoverVehicle(const FInputActionValue& Value);
    void BeginMouseLook(const FInputActionValue& Value);
    void EndMouseLook(const FInputActionValue& Value);
    void ApplyMouseLookYaw(const FInputActionValue& Value);
    void ApplyMouseLookPitch(const FInputActionValue& Value);
    void ApplyGamepadLookYaw(const FInputActionValue& Value);
    void ApplyGamepadLookPitch(const FInputActionValue& Value);
    void ApplyCameraZoom(const FInputActionValue& Value);

    UPROPERTY(Transient) TObjectPtr<UInputMappingContext> RuntimeContext;
    UPROPERTY(Transient) TObjectPtr<UInputAction> ThrottleAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> BrakeAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> SteeringAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> HandbrakeAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> ResetCameraAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> RecoverVehicleAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MouseLookHoldAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MouseLookYawAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> MouseLookPitchAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> GamepadLookYawAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> GamepadLookPitchAction;
    UPROPERTY(Transient) TObjectPtr<UInputAction> CameraZoomAction;

    bool bMouseLookHeld = false;
};
