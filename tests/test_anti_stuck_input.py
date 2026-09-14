import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class AntiStuckInputTests(unittest.TestCase):
    def test_driver_controller_exposes_reset_intent(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehicleControllerComponent.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveVehicleControllerComponent.cpp").read_text(encoding="utf-8")
        self.assertIn("void ResetIntent();", header)
        self.assertIn("UOpenDriveVehicleControllerComponent::ResetIntent()", source)
        self.assertIn("Intent = FOpenDriveDriverIntent{};", source)

    def test_player_controller_resets_old_pawn_before_unpossess(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDrivePlayerController.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        self.assertIn("virtual void OnUnPossess() override;", header)
        self.assertIn("void AOpenDrivePlayerController::OnUnPossess()", source)
        self.assertIn("void AOpenDrivePlayerController::OnUnPossess()", source)
        start = source.index("void AOpenDrivePlayerController::OnUnPossess()")
        body = source[start:start + 900]
        self.assertIn("GetDriverController()", body)
        self.assertIn("ResetIntent();", body)
        self.assertIn("Super::OnUnPossess();", body)
        self.assertLess(body.index("ResetIntent();"), body.index("Super::OnUnPossess();"))

    def test_unpossess_clears_mouse_look_state(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        self.assertIn("void AOpenDrivePlayerController::OnUnPossess()", source)
        start = source.index("void AOpenDrivePlayerController::OnUnPossess()")
        body = source[start:start + 900]
        self.assertIn("bMouseLookHeld = false;", body)

    def test_readme_and_verifier_keep_anti_stuck_contract(self):
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("losing possession", readme)
        self.assertIn("resets driving input", readme)
        self.assertIn("tests/test_anti_stuck_input.py", set(REQUIRED_PATHS))


if __name__ == "__main__":
    unittest.main()
