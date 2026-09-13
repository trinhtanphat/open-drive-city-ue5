import json
import pathlib
import tempfile
import unittest

from tools.verify_repo import verify

ROOT = pathlib.Path(__file__).resolve().parents[1]


class VerifyRepoTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.fixture_root = pathlib.Path(self.temp.name)
        required = [
            "Source/OpenDriveCity/OpenDriveCity.Build.cs",
            "Source/OpenDriveCity/OpenDriveCity.cpp",
            "Source/OpenDriveCity.Target.cs",
            "Source/OpenDriveCityEditor.Target.cs",
            ".gitignore",
            "LICENSE",
            "README.md",
        ]
        for rel in required:
            path = self.fixture_root / rel
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("fixture", encoding="utf-8")
        descriptor = {
            "FileVersion": 3,
            "Plugins": [
                {"Name": "ChaosVehicles", "Enabled": True},
                {"Name": "EnhancedInput", "Enabled": True},
            ],
        }
        (self.fixture_root / "OpenDriveCity.uproject").write_text(json.dumps(descriptor), encoding="utf-8")

    def tearDown(self):
        self.temp.cleanup()

    def test_clean_fixture_passes(self):
        self.assertEqual([], verify(self.fixture_root))

    def test_generated_directory_is_rejected(self):
        (self.fixture_root / "Binaries").mkdir()
        self.assertIn("forbidden generated directory: Binaries", verify(self.fixture_root))

    def test_gitignore_excludes_python_cache(self):
        text = (ROOT / ".gitignore").read_text(encoding="utf-8")
        self.assertIn("__pycache__/", text)
        self.assertIn("*.py[cod]", text)

    def test_driver_intent_contract(self):
        text = (ROOT / "Source/OpenDriveCity/Public/OpenDriveDriverIntent.h").read_text(encoding="utf-8")
        for token in ["FOpenDriveDriverIntent", "Throttle", "Brake", "Steering", "bHandbrake"]:
            self.assertIn(token, text)

    def test_vehicle_controller_component_contract(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehicleControllerComponent.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveVehicleControllerComponent.cpp").read_text(encoding="utf-8")
        for token in ["SetThrottle", "SetBrake", "SetSteering", "SetHandbrake", "GetIntent"]:
            self.assertIn(token, header)
        self.assertGreaterEqual(source.count("FMath::Clamp"), 3)
        self.assertNotIn("EKeys::", source)

    def test_vehicle_pawn_contract(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveVehiclePawn.cpp").read_text(encoding="utf-8")
        for token in ["AWheeledVehiclePawn", "ResetChaseCamera", "GetSpeedKph", "GetCurrentGear", "GetDriverController"]:
            self.assertIn(token, header)
        for token in ["SetThrottleInput", "SetBrakeInput", "SetSteeringInput", "SetHandbrakeInput", "USpringArmComponent", "UCameraComponent"]:
            self.assertIn(token, source)


if __name__ == "__main__":
    unittest.main()
