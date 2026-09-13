#include "OpenDriveGameMode.h"

#include "OpenDrivePlayerController.h"
#include "OpenDriveVehiclePawn.h"

AOpenDriveGameMode::AOpenDriveGameMode()
{
    PlayerControllerClass = AOpenDrivePlayerController::StaticClass();
    DefaultPawnClass = AOpenDriveVehiclePawn::StaticClass();
}
