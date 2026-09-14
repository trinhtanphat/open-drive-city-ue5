import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class CameraOrbitInputTests(unittest.TestCase):
    def test_vehicle_pawn_exposes_bounded_camera_orbit(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveVehiclePawn.cpp").read_text(encoding="utf-8")
        for token in ["AdjustCameraYaw", "AdjustCameraPitch"]:
            self.assertIn(token, header)
            self.assertIn(token, source)
        for token in ["GetRelativeRotation", "FRotator::NormalizeAxis", "FMath::Clamp", "-70.0f", "25.0f"]:
            self.assertIn(token, source)

    def test_mouse_orbit_requires_right_mouse_hold(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDrivePlayerController.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for token in ["MouseLookHoldAction", "MouseLookYawAction", "MouseLookPitchAction", "bMouseLookHeld"]:
            self.assertIn(token, header)
        for token in ["EKeys::RightMouseButton", "EKeys::MouseX", "EKeys::MouseY", "BeginMouseLook", "EndMouseLook"]:
            self.assertIn(token, source)
        self.assertIn("ETriggerEvent::Canceled, this, &AOpenDrivePlayerController::EndMouseLook", source)

    def test_gamepad_right_stick_orbits_without_mouse_hold(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for token in ["GamepadLookYawAction", "GamepadLookPitchAction", "EKeys::Gamepad_RightX", "EKeys::Gamepad_RightY", "ApplyGamepadLookYaw", "ApplyGamepadLookPitch"]:
            self.assertIn(token, source)

    def test_readme_and_verifier_keep_camera_controls(self):
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("Right Mouse Button", readme)
        self.assertIn("right stick", readme)
        self.assertIn("R`", readme)
        self.assertIn("tests/test_camera_orbit_input.py", set(REQUIRED_PATHS))


if __name__ == "__main__":
    unittest.main()