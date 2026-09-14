import pathlib
import unittest

from tools.verify_repo import REQUIRED_PATHS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class WindowsPowerShellCiTests(unittest.TestCase):
    def test_verify_workflow_has_windows_powershell_gate(self):
        workflow = (ROOT / ".github/workflows/verify.yml").read_text(encoding="utf-8")
        for token in [
            "windows-powershell-verify",
            "runs-on: windows-latest",
            "shell: pwsh",
            "Parser]::ParseFile",
            "tools/check_workstation.ps1",
            "tools/build_unreal.ps1",
            "tools/run_unreal_smoke.ps1",
        ]:
            self.assertIn(token, workflow)
    def test_windows_gate_checks_fail_closed_exit_two(self):
        workflow = (ROOT / ".github/workflows/verify.yml").read_text(encoding="utf-8")
        for token in [
            "pwsh -NoProfile -File .\\tools\\build_unreal.ps1",
            "pwsh -NoProfile -File .\\tools\\run_unreal_smoke.ps1",
            "$LASTEXITCODE -ne 2",
        ]:
            self.assertIn(token, workflow)

    def test_readme_documents_windows_ci_gate(self):
        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        self.assertIn("windows-latest", readme)
        self.assertIn("PowerShell", readme)
        self.assertIn("exit 2", readme)
    def test_regression_test_is_required(self):
        self.assertIn("tests/test_windows_powershell_ci.py", set(REQUIRED_PATHS))


if __name__ == "__main__":
    unittest.main()
