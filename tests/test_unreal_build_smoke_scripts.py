import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class UnrealBuildSmokeScriptTests(unittest.TestCase):
    def test_build_script_contract(self):
        path = ROOT / "tools/build_unreal.ps1"
        self.assertTrue(path.exists(), "build_unreal.ps1 is required")
        text = path.read_text(encoding="utf-8")
        for token in [
            "[string]$UnrealRoot", "Find-UnrealRoot", "UE_5.6", "Build.bat",
            "OpenDriveCityEditor", "Win64", "Development",
            "-Project=", "-WaitMutex", "BUILD_UNREAL=PASS",
        ]:
            self.assertIn(token, text)

    def test_smoke_script_contract(self):
        path = ROOT / "tools/run_unreal_smoke.ps1"
        self.assertTrue(path.exists(), "run_unreal_smoke.ps1 is required")
        text = path.read_text(encoding="utf-8")
        for token in [
            "[string]$UnrealRoot", "UE_5.6", "UnrealEditor-Cmd.exe", "-NullRHI",
            "-Unattended", "-NoSplash", "-NoSound", "-ExecCmds=Quit",
            "UNREAL_SMOKE=PASS",
        ]:
            self.assertIn(token, text)

    def test_scripts_are_required_and_documented(self):
        required = set(REQUIRED_PATHS)
        self.assertIn("tools/build_unreal.ps1", required)
        self.assertIn("tools/run_unreal_smoke.ps1", required)
        self.assertIn("tests/test_unreal_build_smoke_scripts.py", required)

        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        for token in [
            "tools/build_unreal.ps1",
            "tools/run_unreal_smoke.ps1",
            "-NullRHI",
            "OpenDriveCityEditor",
        ]:
            self.assertIn(token, readme)


if __name__ == "__main__":
    unittest.main()
