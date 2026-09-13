import json
import pathlib
import tempfile
import unittest

from tools.verify_repo import REQUIRED_PATHS, verify

ROOT = pathlib.Path(__file__).resolve().parents[1]


class FinalContractTests(unittest.TestCase):
    def make_fixture(self):
        temp = tempfile.TemporaryDirectory()
        root = pathlib.Path(temp.name)
        for rel in REQUIRED_PATHS:
            if rel == "OpenDriveCity.uproject":
                continue
            path = root / rel
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("fixture", encoding="utf-8")
        descriptor = {
            "FileVersion": 3,
            "Plugins": [
                {"Name": "ChaosVehicles", "Enabled": True},
                {"Name": "EnhancedInput", "Enabled": True},
            ],
        }
        (root / "OpenDriveCity.uproject").write_text(
            json.dumps(descriptor), encoding="utf-8"
        )
        return temp, root

    def test_phase1_required_paths_are_enforced(self):
        expected = {
            "Config/DefaultEngine.ini",
            "Config/DefaultGame.ini",
            "Config/DefaultScalability.ini",
            ".github/workflows/verify.yml",
            "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h",
            "Source/OpenDriveCity/Public/OpenDriveTestCityBuilder.h",
            "tests/test_input_lifecycle.py",
        }
        self.assertTrue(expected.issubset(set(REQUIRED_PATHS)))

    def test_nested_generated_directory_is_rejected(self):
        temp, root = self.make_fixture()
        try:
            nested = root / "Source" / "Scratch" / "Binaries"
            nested.mkdir(parents=True)
            errors = verify(root)
            self.assertIn(
                "forbidden generated directory: Source/Scratch/Binaries", errors
            )
        finally:
            temp.cleanup()
    def test_readme_documents_phase1_runbook(self):
        text = (ROOT / "README.md").read_text(encoding="utf-8")
        required = [
            "Prerequisites",
            "Clone and setup",
            "GPU-runtime-unverified",
            "WASD",
            "Space",
            "Low",
            "Medium",
            "High",
            "Cinematic",
            "Development Editor",
            "spawn and possess",
            "speed and gear telemetry",
            "MIT License",
        ]
        for token in required:
            self.assertIn(token, text)

    def test_ci_runs_tests_and_verifier(self):
        text = (ROOT / ".github/workflows/verify.yml").read_text(encoding="utf-8")
        self.assertIn("python -m unittest discover -s tests -v", text)
        self.assertIn("python tools/verify_repo.py", text)


if __name__ == "__main__":
    unittest.main()
