from __future__ import annotations

import json
import pathlib
import sys

REQUIRED_PATHS = [
    "OpenDriveCity.uproject",
    ".gitignore",
    "LICENSE",
    "README.md",
    ".github/workflows/verify.yml",
    "Config/DefaultEngine.ini",
    "Config/DefaultGame.ini",
    "Config/DefaultScalability.ini",
    "Source/OpenDriveCity.Target.cs",
    "Source/OpenDriveCityEditor.Target.cs",
    "Source/OpenDriveCity/OpenDriveCity.Build.cs",
    "Source/OpenDriveCity/OpenDriveCity.cpp",
    "Source/OpenDriveCity/Public/OpenDriveDriverIntent.h",
    "Source/OpenDriveCity/Public/OpenDriveVehicleTelemetry.h",
    "Source/OpenDriveCity/Public/OpenDriveVehicleControllerComponent.h",
    "Source/OpenDriveCity/Private/OpenDriveVehicleControllerComponent.cpp",
    "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h",
    "Source/OpenDriveCity/Private/OpenDriveVehiclePawn.cpp",
    "Source/OpenDriveCity/Public/OpenDrivePlayerController.h",
    "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp",
    "Source/OpenDriveCity/Public/OpenDriveGameMode.h",
    "Source/OpenDriveCity/Private/OpenDriveGameMode.cpp",
    "Source/OpenDriveCity/Public/OpenDriveTestCityBuilder.h",
    "Source/OpenDriveCity/Private/OpenDriveTestCityBuilder.cpp",
    "tools/verify_repo.py",
    "tests/test_verify_repo.py",
    "tests/test_final_contract.py",
    "tests/test_input_lifecycle.py",
    "tools/check_workstation.ps1",
    "tests/test_workstation_preflight_contract.py",
    "tests/test_vehicle_recovery_telemetry.py",
    "tests/test_camera_orbit_input.py",
    "tests/test_camera_zoom_input.py",
    "tests/test_anti_stuck_input.py",
    "tests/test_cancel_safe_driving_input.py",
    "tools/build_unreal.ps1",
    "tools/run_unreal_smoke.ps1",
    "tests/test_unreal_build_smoke_scripts.py",
    "tests/test_windows_powershell_ci.py",
]

FORBIDDEN_DIRS = {"Binaries", "DerivedDataCache", "Intermediate", "Saved"}
FORBIDDEN_BINARY_EXTENSIONS = {".uasset", ".umap", ".pak"}
REQUIRED_PLUGINS = {"ChaosVehicles", "EnhancedInput"}


def _relative_parts(root: pathlib.Path, path: pathlib.Path) -> tuple[str, ...]:
    return path.relative_to(root).parts


def _is_ignored_internal_path(parts: tuple[str, ...]) -> bool:
    return ".git" in parts or ".worktrees" in parts


def verify(root: pathlib.Path) -> list[str]:
    root = pathlib.Path(root)
    errors: list[str] = []
    for rel in REQUIRED_PATHS:
        if not (root / rel).exists():
            errors.append(f"missing required path: {rel}")

    descriptor_path = root / "OpenDriveCity.uproject"
    if descriptor_path.exists():
        try:
            descriptor = json.loads(descriptor_path.read_text(encoding="utf-8"))
        except (json.JSONDecodeError, OSError) as exc:
            errors.append(f"invalid project descriptor: {exc}")
        else:
            enabled = {
                item.get("Name")
                for item in descriptor.get("Plugins", [])
                if item.get("Enabled") is True
            }
            for plugin in sorted(REQUIRED_PLUGINS - enabled):
                errors.append(f"required plugin not enabled: {plugin}")

    for path in root.rglob("*"):
        parts = _relative_parts(root, path)
        if _is_ignored_internal_path(parts):
            continue
        if path.is_dir() and path.name in FORBIDDEN_DIRS:
            rel = path.relative_to(root).as_posix()
            errors.append(f"forbidden generated directory: {rel}")
            continue
        if path.is_file() and path.suffix.lower() in FORBIDDEN_BINARY_EXTENSIONS:
            rel = path.relative_to(root).as_posix()
            errors.append(f"forbidden phase-1 binary asset: {rel}")

    return errors

def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    errors = verify(root)
    if errors:
        for error in errors:
            print(f"VERIFY FAIL: {error}")
        return 1
    print("VERIFY PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
