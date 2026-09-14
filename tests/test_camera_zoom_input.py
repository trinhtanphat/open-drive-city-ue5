import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class CameraZoomInputTests(unittest.TestCase):
    def test_vehicle_pawn_exposes_bounded_camera_zoom_and_reset(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveVehiclePawn.cpp").read_text(encoding="utf-8")
        self.assertIn("AdjustCameraZoom", header)
        for token in ["AdjustCameraZoom", "TargetArmLength", "FMath::Clamp", "350.0f", "900.0f", "650.0f"]:
            self.assertIn(token, source)
        self.assertGreaterEqual(source.count("CameraBoom->TargetArmLength = 650.0f;"), 2)

    def test_mouse_wheel_maps_to_camera_zoom(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDrivePlayerController.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for token in ["CameraZoomAction", "ApplyCameraZoom"]:
            self.assertIn(token, header)
        for token in [
            "CameraZoomAction = NewAxisAction",
            "RuntimeContext->MapKey(CameraZoomAction, EKeys::MouseWheelAxis)",
            "Enhanced->BindAction(CameraZoomAction, ETriggerEvent::Triggered",
            "ApplyCameraZoom",
            "AdjustCameraZoom",
        ]:
            self.assertIn(token, source)

    def test_gamepad_shoulders_map_to_camera_zoom(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        self.assertIn("RuntimeContext->MapKey(CameraZoomAction, EKeys::Gamepad_LeftShoulder)", source)
        self.assertIn("RuntimeContext->MapKey(CameraZoomAction, EKeys::Gamepad_RightShoulder)", source)
        self.assertIn("ZoomOut.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeContext))", source)

    def test_readme_and_verifier_keep_camera_zoom(self):
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("Mouse wheel", readme)
        self.assertIn("shoulder", readme)
        self.assertIn("tests/test_camera_zoom_input.py", set(REQUIRED_PATHS))


if __name__ == "__main__":
    unittest.main()
