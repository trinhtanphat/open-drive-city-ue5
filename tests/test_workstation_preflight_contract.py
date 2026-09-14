import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class WorkstationPreflightContractTests(unittest.TestCase):
    def test_script_checks_build_and_runtime_prerequisites(self):
        script = ROOT / "tools" / "check_workstation.ps1"
        self.assertTrue(script.exists(), "workstation preflight script is required")
        text = script.read_text(encoding="utf-8")
        for token in [
            "Unreal Engine",
            "Visual Studio",
            "MSBuild",
            "rc.exe",
            "cmake",
            "ninja",
            "Win32_VideoController",
            "FreeGB",
            "Build.bat",
            "Epic Games Launcher",
            "C++ compile smoke",
            "VsDevCmd.bat",
            "smoke.cpp",
            "GPU-runtime-unverified",
            "exit 2",
        ]:
            self.assertIn(token, text)
