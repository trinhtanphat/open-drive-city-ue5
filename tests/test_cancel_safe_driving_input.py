import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class CancelSafeDrivingInputTests(unittest.TestCase):
    def test_axis_actions_use_explicit_release_handlers(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDrivePlayerController.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for action, handler in [
            ("ThrottleAction", "ReleaseThrottle"),
            ("BrakeAction", "ReleaseBrake"),
            ("SteeringAction", "ReleaseSteering"),
        ]:
            self.assertIn(f"void {handler}(const FInputActionValue& Value);", header)
            self.assertIn(f"BindAction({action}, ETriggerEvent::Completed, this, &AOpenDrivePlayerController::{handler})", source)
            self.assertIn(f"BindAction({action}, ETriggerEvent::Canceled, this, &AOpenDrivePlayerController::{handler})", source)

    def test_axis_release_handlers_write_explicit_zero(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for handler, setter in [
            ("ReleaseThrottle", "SetThrottle(0.0f)"),
            ("ReleaseBrake", "SetBrake(0.0f)"),
            ("ReleaseSteering", "SetSteering(0.0f)"),
        ]:
            self.assertIn(f"void AOpenDrivePlayerController::{handler}", source)
            self.assertIn(setter, source)

    def test_handbrake_releases_when_input_is_canceled(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        self.assertIn(
            "BindAction(HandbrakeAction, ETriggerEvent::Canceled, this, &AOpenDrivePlayerController::ReleaseHandbrake)",
            source,
        )

    def test_readme_and_verifier_keep_cancel_safe_contract(self):
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("Canceled", readme)
        self.assertIn("stuck driving input", readme)
        self.assertIn("tests/test_cancel_safe_driving_input.py", set(REQUIRED_PATHS))


if __name__ == "__main__":
    unittest.main()
