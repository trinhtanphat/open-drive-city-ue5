import json
import pathlib
import tempfile
import unittest

from tools.verify_repo import REQUIRED_PATHS, verify

ROOT = pathlib.Path(__file__).resolve().parents[1]


class VerifyRepoTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.fixture_root = pathlib.Path(self.temp.name)
        required = [rel for rel in REQUIRED_PATHS if rel != "OpenDriveCity.uproject"]

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


class InputAndGameModeContractTests(unittest.TestCase):
    def test_player_controller_uses_enhanced_input(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text(encoding="utf-8")
        for token in [
            "UInputMappingContext", "UInputAction", "UEnhancedInputLocalPlayerSubsystem",
            "EKeys::W", "EKeys::S", "EKeys::A", "EKeys::D",
            "EKeys::SpaceBar", "EKeys::Gamepad_LeftX"
        ]:
            self.assertIn(token, source)

    def test_game_mode_owns_project_classes(self):
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveGameMode.cpp").read_text(encoding="utf-8")
        self.assertIn("PlayerControllerClass = AOpenDrivePlayerController::StaticClass()", source)
        self.assertIn("DefaultPawnClass = AOpenDriveVehiclePawn::StaticClass()", source)


class CityAndRenderingContractTests(unittest.TestCase):
    def test_city_builder_is_bounded_and_asset_free(self):
        header = (ROOT / "Source/OpenDriveCity/Public/OpenDriveTestCityBuilder.h").read_text(encoding="utf-8")
        source = (ROOT / "Source/OpenDriveCity/Private/OpenDriveTestCityBuilder.cpp").read_text(encoding="utf-8")
        for token in ["GridX", "GridY", "BlockSize", "RoadWidth", "MaxBuildings"]:
            self.assertIn(token, header)
        self.assertGreaterEqual(source.count("FMath::Clamp"), 3)
        self.assertIn("/Engine/BasicShapes/Cube.Cube", source)
        self.assertIn("/Engine/BasicShapes/Plane.Plane", source)
        self.assertIn("400", source)

    def test_rendering_config_has_scalable_lumen_nanite_defaults(self):
        engine = (ROOT / "Config/DefaultEngine.ini").read_text(encoding="utf-8")
        scalability = (ROOT / "Config/DefaultScalability.ini").read_text(encoding="utf-8")
        for token in ["r.DynamicGlobalIlluminationMethod=1", "r.ReflectionMethod=1", "r.Nanite.ProjectEnabled=True"]:
            self.assertIn(token, engine)
        for token in ["@0]", "@1]", "@2]", "@Cine]", "r.DynamicGlobalIlluminationMethod=0"]:
            self.assertIn(token, scalability)
        self.assertIn("EnhancedInputComponent", engine)
