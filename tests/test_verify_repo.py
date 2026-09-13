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
        (self.fixture_root / "OpenDriveCity.uproject").write_text(
            json.dumps(descriptor), encoding="utf-8"
        )

    def tearDown(self):
        self.temp.cleanup()

    def test_clean_fixture_passes(self):
        self.assertEqual([], verify(self.fixture_root))

    def test_generated_directory_is_rejected(self):
        (self.fixture_root / "Binaries").mkdir()
        self.assertIn("forbidden generated directory: Binaries", verify(self.fixture_root))


if __name__ == "__main__":
    unittest.main()
