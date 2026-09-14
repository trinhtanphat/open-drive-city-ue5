# Open Drive City UE5

Source-first Unreal Engine 5 C++ driving sandbox focused on a compact modern-city driving prototype with legally clean project-owned source.

## Current status

Phase 1 is **source-complete/static-verified** on VPS 182 and remains **GPU-runtime-unverified** until it is compiled and exercised in Unreal Editor on a discrete-GPU workstation.

The repository contains project source, configuration, verification tests, CI, and documentation only. Unreal Engine binaries, Marketplace/Fab assets, generated build products, and proprietary vehicle/city assets are not committed.

## Prerequisites

- Windows 11 or another UE5-supported development OS.
- Unreal Engine 5 with the Chaos Vehicles and Enhanced Input plugins available.
- Visual Studio 2022 with Desktop development with C++ and Game development with C++ workloads on Windows.
- Python 3.12+ for repository verification.
- Recommended GPU for later runtime validation: 12-16 GB VRAM; 24 GB preferred for heavier city assets and aggressive Lumen/Nanite settings.
- Recommended system RAM: 32 GB minimum, 64 GB preferred for larger content workflows.
- Recommended free SSD space: 150 GB or more once Unreal Engine, caches, and assets are installed.

## Clone and setup

```powershell
git clone https://github.com/trinhtanphat/open-drive-city-ue5.git
cd open-drive-city-ue5
python -m unittest discover -s tests -v
python tools/verify_repo.py
```

## Build on a GPU workstation

1. Install Unreal Engine 5 and the required Visual Studio C++ toolchain.
2. Right-click `OpenDriveCity.uproject` and generate project files, or use the matching Unreal Build Tool command for your engine installation.
3. Open the generated solution and compile the **Development Editor** target.
4. Launch `OpenDriveCity.uproject` in Unreal Editor.
5. Confirm Chaos Vehicles and Enhanced Input are enabled if the editor requests plugin confirmation.

VPS 182 is intentionally used only for source/static verification because it currently has no suitable discrete GPU exposed to Windows and does not have the full Unreal toolchain installed.

## Controls

Keyboard driving uses **WASD** or the matching arrow keys.

- `W` / Up Arrow: throttle.
- `S` / Down Arrow: brake/reverse intent.
- `A` / Left Arrow and `D` / Right Arrow: steering.
- `Space`: handbrake.
- `Backspace`: recover vehicle upright and clear driving input.
- Hold **Right Mouse Button** and move the mouse: orbit the chase camera.
- Gamepad **right stick**: orbit the chase camera without a mouse-look hold.
- `R`: reset the chase camera behind the vehicle.
- Gamepad left stick: steering.
- Gamepad triggers: throttle/brake.
- Camera reset action: restore the chase camera behind the vehicle.

## Graphics tiers

`Config/DefaultScalability.ini` provides **Low**, **Medium**, **High**, and **Cinematic** profiles. Lower tiers reduce or disable expensive rendering features, while High/Cinematic retain the Lumen/Nanite-ready visual direction where hardware permits.

## GPU validation checklist

On the first suitable GPU workstation, perform this gate before calling the runtime verified:

1. Generate project files from `OpenDriveCity.uproject`.
2. Compile the **Development Editor** target successfully.
3. Open the project in Unreal Editor without plugin or module errors.
4. Create/open a test level, place the primitive city builder if needed, then **spawn and possess** `AOpenDriveVehiclePawn`.
5. Verify throttle, brake, steering, and handbrake behavior.
6. Reset the chase camera and confirm camera lag/FOV behavior remains independent of vehicle physics.
7. Verify **speed and gear telemetry** while driving.
8. Exercise **Low**, **Medium**, **High**, and **Cinematic** graphics tiers.
9. Save/reopen the project and repeat a short drive smoke test.

Until every item above passes, status remains **GPU-runtime-unverified** even when all repository/static checks are green.

## Verification and CI

```powershell
python -m unittest discover -s tests -v
python tools/verify_repo.py
git diff --check
```

GitHub Actions runs the Python unit suite and repository verifier on pushes and pull requests without requiring Unreal Engine.

A separate `windows-latest` job parses the PowerShell helpers with the real PowerShell parser and verifies that the Unreal build/smoke helpers fail closed with exit 2 when the engine is missing.

## Licensing

Project-owned code and documentation are provided under the **MIT License** in `LICENSE`. Unreal Engine, Epic content, Fab/Marketplace assets, Megascans content, and any future third-party assets remain governed by their own applicable licenses and are not redistributed here unless their license explicitly permits it.

## Workstation preflight

Before attempting an Unreal build, run the workstation gate:

```powershell
powershell -ExecutionPolicy Bypass -File tools/check_workstation.ps1
```

On a CPU-only or virtualized host such as VPS 182, temporarily skip the GPU requirement while still checking the compiler, SDK, build tools, Unreal installation, and disk:

```powershell
powershell -ExecutionPolicy Bypass -File tools/check_workstation.ps1 -SkipGpu
```

Exit code `0` means every required non-skipped prerequisite passed. Exit code `2` means one or more blockers remain. Skipping GPU never changes the runtime status: it remains **GPU-runtime-unverified** until a discrete-GPU smoke test is completed.

## Unreal build and headless smoke

After Unreal Engine 5.6 is installed, build the editor target from PowerShell. You may omit `-UnrealRoot` to auto-detect `UE_5.6` from the standard Epic install folders:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_unreal.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.6"
```

The build helper invokes `Build.bat` for `OpenDriveCityEditor Win64 Development` and fails closed when the engine, project, or build entry point is missing.

After a successful build, run the CPU/headless startup smoke test:

```powershell
powershell -ExecutionPolicy Bypass -File tools/run_unreal_smoke.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.6"
```

The smoke helper launches `UnrealEditor-Cmd.exe` with `-NullRHI`, `-Unattended`, no splash/audio, and a bounded timeout. Passing this proves project/module startup without a GPU; it does **not** replace the later discrete-GPU driving/render validation.
