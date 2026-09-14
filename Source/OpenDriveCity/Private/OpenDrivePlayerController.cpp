#include "OpenDrivePlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "OpenDriveVehicleControllerComponent.h"
#include "OpenDriveVehiclePawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogOpenDriveInput, Log, All);

AOpenDrivePlayerController::AOpenDrivePlayerController()
{
    bShowMouseCursor = false;
}

void AOpenDrivePlayerController::BeginPlay()
{
    Super::BeginPlay();
    BuildRuntimeInputMap();
    RegisterRuntimeInputMap();
}

void AOpenDrivePlayerController::OnUnPossess()
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        if (UOpenDriveVehicleControllerComponent* DriverController = Vehicle->GetDriverController())
        {
            DriverController->ResetIntent();
        }
    }

    bMouseLookHeld = false;
    Super::OnUnPossess();
}

void AOpenDrivePlayerController::BuildRuntimeInputMap()
{
    if (RuntimeContext)
    {
        return;
    }

    RuntimeContext = NewObject<UInputMappingContext>(this, TEXT("OpenDriveRuntimeContext"));
    auto NewAxisAction = [this](const TCHAR* Name)
    {
        UInputAction* Action = NewObject<UInputAction>(this, Name);
        Action->ValueType = EInputActionValueType::Axis1D;
        return Action;
    };
    auto NewBoolAction = [this](const TCHAR* Name)
    {
        UInputAction* Action = NewObject<UInputAction>(this, Name);
        Action->ValueType = EInputActionValueType::Boolean;
        return Action;
    };

    ThrottleAction = NewAxisAction(TEXT("Throttle"));
    BrakeAction = NewAxisAction(TEXT("Brake"));
    SteeringAction = NewAxisAction(TEXT("Steering"));
    HandbrakeAction = NewBoolAction(TEXT("Handbrake"));
    ResetCameraAction = NewBoolAction(TEXT("ResetCamera"));
    RecoverVehicleAction = NewBoolAction(TEXT("RecoverVehicle"));
    MouseLookHoldAction = NewBoolAction(TEXT("MouseLookHold"));
    MouseLookYawAction = NewAxisAction(TEXT("MouseLookYaw"));
    MouseLookPitchAction = NewAxisAction(TEXT("MouseLookPitch"));
    GamepadLookYawAction = NewAxisAction(TEXT("GamepadLookYaw"));
    GamepadLookPitchAction = NewAxisAction(TEXT("GamepadLookPitch"));
    CameraZoomAction = NewAxisAction(TEXT("CameraZoom"));

    RuntimeContext->MapKey(ThrottleAction, EKeys::W);
    RuntimeContext->MapKey(ThrottleAction, EKeys::Up);
    RuntimeContext->MapKey(ThrottleAction, EKeys::Gamepad_RightTriggerAxis);
    RuntimeContext->MapKey(BrakeAction, EKeys::S);
    RuntimeContext->MapKey(BrakeAction, EKeys::Down);
    RuntimeContext->MapKey(BrakeAction, EKeys::Gamepad_LeftTriggerAxis);

    FEnhancedActionKeyMapping& Left = RuntimeContext->MapKey(SteeringAction, EKeys::A);
    Left.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeContext));
    RuntimeContext->MapKey(SteeringAction, EKeys::D);
    RuntimeContext->MapKey(SteeringAction, EKeys::Gamepad_LeftX);
    FEnhancedActionKeyMapping& ArrowLeft = RuntimeContext->MapKey(SteeringAction, EKeys::Left);
    ArrowLeft.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeContext));
    RuntimeContext->MapKey(SteeringAction, EKeys::Right);

    RuntimeContext->MapKey(HandbrakeAction, EKeys::SpaceBar);
    RuntimeContext->MapKey(HandbrakeAction, EKeys::Gamepad_FaceButton_Left);
    RuntimeContext->MapKey(ResetCameraAction, EKeys::R);
    RuntimeContext->MapKey(ResetCameraAction, EKeys::Gamepad_RightThumbstick);
    RuntimeContext->MapKey(RecoverVehicleAction, EKeys::BackSpace);
    RuntimeContext->MapKey(RecoverVehicleAction, EKeys::Gamepad_Special_Right);
    RuntimeContext->MapKey(MouseLookHoldAction, EKeys::RightMouseButton);
    RuntimeContext->MapKey(MouseLookYawAction, EKeys::MouseX);
    RuntimeContext->MapKey(MouseLookPitchAction, EKeys::MouseY);
    RuntimeContext->MapKey(GamepadLookYawAction, EKeys::Gamepad_RightX);
    RuntimeContext->MapKey(GamepadLookPitchAction, EKeys::Gamepad_RightY);
    RuntimeContext->MapKey(CameraZoomAction, EKeys::MouseWheelAxis);
    FEnhancedActionKeyMapping& ZoomOut = RuntimeContext->MapKey(CameraZoomAction, EKeys::Gamepad_LeftShoulder);
    ZoomOut.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeContext));
    RuntimeContext->MapKey(CameraZoomAction, EKeys::Gamepad_RightShoulder);

}

void AOpenDrivePlayerController::RegisterRuntimeInputMap()
{
    if (!RuntimeContext)
    {
        BuildRuntimeInputMap();
    }

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
        {
            Subsystem->AddMappingContext(RuntimeContext, 0);
        }
    }
}

void AOpenDrivePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    BuildRuntimeInputMap();

    UEnhancedInputComponent* Enhanced = Cast<UEnhancedInputComponent>(InputComponent);
    if (!Enhanced)
    {
        UE_LOG(LogOpenDriveInput, Error, TEXT("Enhanced Input component is required."));
        return;
    }

    Enhanced->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyThrottle);
    Enhanced->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AOpenDrivePlayerController::ApplyThrottle);
    Enhanced->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyBrake);
    Enhanced->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AOpenDrivePlayerController::ApplyBrake);
    Enhanced->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplySteering);
    Enhanced->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AOpenDrivePlayerController::ApplySteering);
    Enhanced->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AOpenDrivePlayerController::ApplyHandbrake);
    Enhanced->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AOpenDrivePlayerController::ReleaseHandbrake);
    Enhanced->BindAction(ResetCameraAction, ETriggerEvent::Started, this, &AOpenDrivePlayerController::ResetCamera);
    Enhanced->BindAction(RecoverVehicleAction, ETriggerEvent::Started, this, &AOpenDrivePlayerController::RecoverVehicle);
    Enhanced->BindAction(MouseLookHoldAction, ETriggerEvent::Started, this, &AOpenDrivePlayerController::BeginMouseLook);
    Enhanced->BindAction(MouseLookHoldAction, ETriggerEvent::Completed, this, &AOpenDrivePlayerController::EndMouseLook);
    Enhanced->BindAction(MouseLookHoldAction, ETriggerEvent::Canceled, this, &AOpenDrivePlayerController::EndMouseLook);
    Enhanced->BindAction(MouseLookYawAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyMouseLookYaw);
    Enhanced->BindAction(MouseLookPitchAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyMouseLookPitch);
    Enhanced->BindAction(GamepadLookYawAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyGamepadLookYaw);
    Enhanced->BindAction(GamepadLookPitchAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyGamepadLookPitch);
    Enhanced->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AOpenDrivePlayerController::ApplyCameraZoom);
}

void AOpenDrivePlayerController::ApplyThrottle(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->GetDriverController()->SetThrottle(Value.Get<float>());
    }
}

void AOpenDrivePlayerController::ApplyBrake(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->GetDriverController()->SetBrake(Value.Get<float>());
    }
}

void AOpenDrivePlayerController::ApplySteering(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->GetDriverController()->SetSteering(Value.Get<float>());
    }
}

void AOpenDrivePlayerController::ApplyHandbrake(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->GetDriverController()->SetHandbrake(Value.Get<bool>());
    }
}

void AOpenDrivePlayerController::ReleaseHandbrake(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->GetDriverController()->SetHandbrake(false);
    }
}

void AOpenDrivePlayerController::ResetCamera(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->ResetChaseCamera();
    }
}

void AOpenDrivePlayerController::RecoverVehicle(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->RecoverVehicle();
    }
}

void AOpenDrivePlayerController::BeginMouseLook(const FInputActionValue& Value)
{
    bMouseLookHeld = Value.Get<bool>();
}

void AOpenDrivePlayerController::EndMouseLook(const FInputActionValue& Value)
{
    bMouseLookHeld = false;
}

void AOpenDrivePlayerController::ApplyMouseLookYaw(const FInputActionValue& Value)
{
    if (!bMouseLookHeld)
    {
        return;
    }
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->AdjustCameraYaw(Value.Get<float>() * 0.15f);
    }
}

void AOpenDrivePlayerController::ApplyMouseLookPitch(const FInputActionValue& Value)
{
    if (!bMouseLookHeld)
    {
        return;
    }
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->AdjustCameraPitch(-Value.Get<float>() * 0.15f);
    }
}

void AOpenDrivePlayerController::ApplyGamepadLookYaw(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->AdjustCameraYaw(Value.Get<float>() * 2.0f);
    }
}

void AOpenDrivePlayerController::ApplyGamepadLookPitch(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->AdjustCameraPitch(Value.Get<float>() * 2.0f);
    }
}

void AOpenDrivePlayerController::ApplyCameraZoom(const FInputActionValue& Value)
{
    if (AOpenDriveVehiclePawn* Vehicle = Cast<AOpenDriveVehiclePawn>(GetPawn()))
    {
        Vehicle->AdjustCameraZoom(Value.Get<float>() * 75.0f);
    }
}
