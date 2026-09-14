import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class VehicleRecoveryTelemetryTests(unittest.TestCase):
    def test_telemetry_contract(self):
        header = ROOT / "Source/OpenDriveCity/Public/OpenDriveVehicleTelemetry.h"
        self.assertTrue(header.exists(), "telemetry contract header is required")
        text = header.read_text(encoding="utf-8")
        for token in [
            "FOpenDriveVehicleTelemetry",
            "SpeedKph",
            "CurrentGear",
            "bHandbrake",
            "bMoving",
        ]:
            self.assertIn(token, text)

    def test_vehicle_pawn_exposes_telemetry_and_recovery(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveVehiclePawn.cpp").read_text(encoding="utf-8")
        for token in ["GetTelemetry", "RecoverVehicle", "FOpenDriveVehicleTelemetry"]:
            self.assertIn(token, header)
        for token in [
            "SetActorLocationAndRotation",
            "ETeleportType::TeleportPhysics",
            "SetPhysicsLinearVelocity(FVector::ZeroVector)",
            "SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector)",
            "SetThrottle(0.0f)",
            "SetBrake(0.0f)",
            "SetSteering(0.0f)",
            "SetHandbrake(false)",
            "Movement->SetThrottleInput(0.0f)",
            "Movement->SetBrakeInput(0.0f)",
            "Movement->SetSteeringInput(0.0f)",
            "Movement->SetHandbrakeInput(false)",
            "ResetChaseCamera()",
        ]:
            self.assertIn(token, source)

    def test_recovery_input_mapping(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDrivePlayerController.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for token in ["RecoverVehicleAction", "RecoverVehicle"]:
            self.assertIn(token, header)
        for token in [
            "EKeys::BackSpace",
            "EKeys::Gamepad_Special_Right",
            "BindAction(RecoverVehicleAction",
            "Vehicle->RecoverVehicle()",
        ]:
            self.assertIn(token, source)
    def test_new_contract_files_are_required(self):
        required = set(REQUIRED_PATHS)
        self.assertIn(
            "Source/OpenDriveCity/Public/OpenDriveVehicleTelemetry.h",
            required,
        )
        self.assertIn("tests/test_vehicle_recovery_telemetry.py", required)

    def test_readme_documents_recovery_control(self):
        text = (ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("Backspace", text)
        self.assertIn("recover vehicle", text.lower())


if __name__ == "__main__":
    unittest.main()
