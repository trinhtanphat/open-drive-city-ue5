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
