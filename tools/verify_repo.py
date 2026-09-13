from __future__ import annotations

import json
import pathlib
import sys

REQUIRED_PATHS = [
    "OpenDriveCity.uproject",
    "Source/OpenDriveCity/OpenDriveCity.Build.cs",
    "Source/OpenDriveCity/OpenDriveCity.cpp",
    "Source/OpenDriveCity.Target.cs",
    "Source/OpenDriveCityEditor.Target.cs",
    ".gitignore",
    "LICENSE",
    "README.md",
]
FORBIDDEN_DIRS = ["Binaries", "DerivedDataCache", "Intermediate", "Saved"]
FORBIDDEN_BINARY_EXTENSIONS = {".uasset", ".umap", ".pak"}
REQUIRED_PLUGINS = {"ChaosVehicles", "EnhancedInput"}


def verify(root: pathlib.Path) -> list[str]:
    root = pathlib.Path(root)
    errors: list[str] = []
    for rel in REQUIRED_PATHS:
        if not (root / rel).exists():
            errors.append(f"missing required path: {rel}")

    for name in FORBIDDEN_DIRS:
        if (root / name).exists():
            errors.append(f"forbidden generated directory: {name}")

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
        if not path.is_file():
            continue
        if ".git" in path.parts or ".worktrees" in path.parts:
            continue
        if path.suffix.lower() in FORBIDDEN_BINARY_EXTENSIONS:
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
