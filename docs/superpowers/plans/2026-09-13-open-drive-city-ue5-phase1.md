# Open Drive City UE5 Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create a source-complete Unreal Engine 5 C++ driving sandbox skeleton that can be authored and statically verified on VPS 182 now, then compiled and GPU-validated later on a proper workstation.

**Architecture:** Keep player input, driver intent, vehicle simulation integration, camera, telemetry, city smoke-test generation, and rendering presets separated behind small C++ classes/config files. Enhanced Input mappings are created from C++ at runtime so Phase 1 does not depend on binary assets.

**Tech Stack:** Unreal Engine 5 C++, Chaos Vehicles, Enhanced Input, Python 3 repository verification, GitHub Actions.

**Spec:** `docs/superpowers/specs/2026-09-13-open-drive-city-ue5-design.md`

## Global Constraints

- Public repository: `trinhtanphat/open-drive-city-ue5`; default branch `main`.
- Do not commit Unreal Engine binaries, Marketplace/Fab assets, generated build output, DerivedDataCache, Intermediate, Saved, or IDE caches.
- Phase 1 must not require proprietary art assets.
- Host 182 is source/static-verification only until a discrete GPU and Unreal Engine toolchain are available.
- Project-owned code is MIT licensed.
- Runtime failures for missing required engine plugins must be explicit rather than silently switching to unrelated movement logic.
- Primitive test-city dimensions and object counts must be bounded.

---

## File Structure
- `OpenDriveCity.uproject`: project descriptor and required plugins.
- `Source/OpenDriveCity/OpenDriveCity.Build.cs`: module dependencies.
- `Source/OpenDriveCity/OpenDriveCity.cpp`: module entry point.
- `Source/OpenDriveCity/Public/OpenDriveDriverIntent.h`: normalized driver-intent value object.
- `Source/OpenDriveCity/Public/OpenDriveVehicleControllerComponent.h` + `.cpp`: clamped intent state.
- `Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h` + `.cpp`: Chaos vehicle, camera, telemetry, and intent application.
- `Source/OpenDriveCity/Public/OpenDrivePlayerController.h` + `.cpp`: runtime Enhanced Input mappings and possession routing.
- `Source/OpenDriveCity/Public/OpenDriveGameMode.h` + `.cpp`: default gameplay classes.
- `Source/OpenDriveCity/Public/OpenDriveTestCityBuilder.h` + `.cpp`: bounded primitive road/building generator.
- `Config/DefaultEngine.ini`: maps/game mode/rendering defaults.
- `Config/DefaultGame.ini`: project metadata.
- `Config/DefaultScalability.ini`: Low/Medium/High/Cinematic render profiles.
- `tools/verify_repo.py`: deterministic source/static verification.
- `tests/test_verify_repo.py`: verifier unit tests.
- `.github/workflows/verify.yml`: CI that runs without Unreal Engine.
- `.gitignore`, `LICENSE`, `README.md`: repository hygiene, licensing, setup/runbook.

### Task 1: Repository Skeleton and Verifier

**Files:**
- Create: `.gitignore`, `LICENSE`, `README.md`, `OpenDriveCity.uproject`
- Create: `Source/OpenDriveCity/OpenDriveCity.Build.cs`, `Source/OpenDriveCity/OpenDriveCity.cpp`
- Create: `Source/OpenDriveCity.Target.cs`, `Source/OpenDriveCityEditor.Target.cs`
- Create: `tools/verify_repo.py`, `tests/test_verify_repo.py`, `.github/workflows/verify.yml`

**Interfaces:**
- Produces: `tools.verify_repo.verify(root: pathlib.Path) -> list[str]`; empty list means PASS.
- [ ] **Step 1: Write failing verifier tests**

```python
class VerifyRepoTests(unittest.TestCase):
    def test_clean_fixture_passes(self):
        self.assertEqual([], verify(self.fixture_root))

    def test_generated_directory_is_rejected(self):
        (self.fixture_root / "Binaries").mkdir()
        self.assertIn("forbidden generated directory: Binaries", verify(self.fixture_root))
```

- [ ] **Step 2: Run test and confirm RED**

Run: `python -m unittest tests.test_verify_repo -v`
Expected: FAIL because `tools.verify_repo` does not exist.

- [ ] **Step 3: Implement minimal project skeleton and verifier**

Verifier must check required paths, parse `OpenDriveCity.uproject` as JSON, require `ChaosVehicles` and `EnhancedInput` enabled, reject forbidden generated directories, and reject tracked large binary extensions (`.uasset`, `.umap`, `.pak`) in Phase 1.

- [ ] **Step 4: Run verifier tests and repository verifier**

Run: `python -m unittest tests.test_verify_repo -v` and `python tools/verify_repo.py`
Expected: PASS / `VERIFY PASS`.

- [ ] **Step 5: Commit**

```bash
git add .
git commit -m "chore: scaffold UE5 source-first project"
```

### Task 2: Driver Intent and Vehicle Control Component
**Files:**
- Create: `Source/OpenDriveCity/Public/OpenDriveDriverIntent.h`
- Create: `Source/OpenDriveCity/Public/OpenDriveVehicleControllerComponent.h`
- Create: `Source/OpenDriveCity/Private/OpenDriveVehicleControllerComponent.cpp`
- Modify: `tests/test_verify_repo.py`

**Interfaces:**
- Produces: `FOpenDriveDriverIntent { float Throttle, Brake, Steering; bool bHandbrake; }`.
- Produces: `UOpenDriveVehicleControllerComponent::SetThrottle(float)`, `SetBrake(float)`, `SetSteering(float)`, `SetHandbrake(bool)`, `GetIntent() const`.

- [ ] **Step 1: Add RED source-contract tests**

```python
def test_driver_intent_contract(self):
    text = (ROOT / "Source/OpenDriveCity/Public/OpenDriveDriverIntent.h").read_text()
    for token in ["Throttle", "Brake", "Steering", "bHandbrake"]:
        self.assertIn(token, text)
```

Also assert the component exposes all setters and clamps analog values with `FMath::Clamp`.

- [ ] **Step 2: Run RED test**

Run: `python -m unittest tests.test_verify_repo -v`
Expected: FAIL because the source files are absent.

- [ ] **Step 3: Implement intent and controller component**

Use clamp ranges `Throttle/Brake: 0..1`, `Steering: -1..1`; handbrake remains boolean. No keyboard-specific code is allowed in this component.

- [ ] **Step 4: Run tests and verifier**

Run: `python -m unittest tests.test_verify_repo -v && python tools/verify_repo.py`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add Source tests tools
git commit -m "feat: add normalized vehicle driver intent"
```
### Task 3: Chaos Vehicle Pawn, Chase Camera, and Telemetry

**Files:**
- Create: `Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h`
- Create: `Source/OpenDriveCity/Private/OpenDriveVehiclePawn.cpp`
- Modify: `tests/test_verify_repo.py`

**Interfaces:**
- Consumes: `UOpenDriveVehicleControllerComponent::GetIntent() const`.
- Produces: `AOpenDriveVehiclePawn::ResetChaseCamera()`, `GetSpeedKph() const`, `GetCurrentGear() const`.

- [ ] **Step 1: Add RED source-contract tests**

```python
def test_vehicle_pawn_contract(self):
    text = (ROOT / "Source/OpenDriveCity/Public/OpenDriveVehiclePawn.h").read_text()
    for token in ["AWheeledVehiclePawn", "ResetChaseCamera", "GetSpeedKph", "GetCurrentGear"]:
        self.assertIn(token, text)
```

Assert `.cpp` calls Chaos movement setters for throttle, brake, steering, and handbrake and owns `USpringArmComponent` plus `UCameraComponent`.

- [ ] **Step 2: Run RED test**

Run: `python -m unittest tests.test_verify_repo -v`
Expected: FAIL because vehicle pawn files are absent.

- [ ] **Step 3: Implement pawn**

Use `AWheeledVehiclePawn` as the base class, apply current driver intent every tick to `UChaosWheeledVehicleMovementComponent`, configure a rear spring arm and chase camera, and derive speed from velocity in km/h.

- [ ] **Step 4: Run tests and verifier**

Run: `python -m unittest tests.test_verify_repo -v && python tools/verify_repo.py`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add Source tests
git commit -m "feat: add Chaos vehicle pawn and chase camera"
```
### Task 4: Enhanced Input Player Controller and Game Mode

**Files:**
- Create: `Source/OpenDriveCity/Public/OpenDrivePlayerController.h`
- Create: `Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp`
- Create: `Source/OpenDriveCity/Public/OpenDriveGameMode.h`
- Create: `Source/OpenDriveCity/Private/OpenDriveGameMode.cpp`
- Modify: `tests/test_verify_repo.py`

**Interfaces:**
- Consumes: vehicle intent setters and `AOpenDriveVehiclePawn::ResetChaseCamera()`.
- Produces: runtime Enhanced Input mappings for WASD/arrows, Space handbrake, gamepad sticks/triggers, and camera reset.

- [ ] **Step 1: Add RED input/game-mode tests**

```python
def test_player_controller_uses_enhanced_input(self):
    text = (ROOT / "Source/OpenDriveCity/Private/OpenDrivePlayerController.cpp").read_text()
    for token in ["UInputMappingContext", "UInputAction", "UEnhancedInputLocalPlayerSubsystem", "EKeys::W", "EKeys::Gamepad_LeftX"]:
        self.assertIn(token, text)
```

Assert `AOpenDriveGameMode` sets `PlayerControllerClass` and `DefaultPawnClass` to project-owned classes.

- [ ] **Step 2: Run RED test**

Run: `python -m unittest tests.test_verify_repo -v`
Expected: FAIL because controller/game-mode files are absent.

- [ ] **Step 3: Implement runtime mappings and routing**

Create transient `UInputAction` objects and a `UInputMappingContext` in the controller, register it with the local-player Enhanced Input subsystem, bind actions, then forward values only to the possessed `AOpenDriveVehiclePawn` controller component.

- [ ] **Step 4: Run tests and verifier**

Run: `python -m unittest tests.test_verify_repo -v && python tools/verify_repo.py`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add Source tests
git commit -m "feat: add source-only enhanced input routing"
```
### Task 5: Primitive Test City and Rendering Configuration

**Files:**
- Create: `Source/OpenDriveCity/Public/OpenDriveTestCityBuilder.h`
- Create: `Source/OpenDriveCity/Private/OpenDriveTestCityBuilder.cpp`
- Create: `Config/DefaultEngine.ini`, `Config/DefaultGame.ini`, `Config/DefaultScalability.ini`
- Modify: `tests/test_verify_repo.py`

**Interfaces:**
- Produces: `AOpenDriveTestCityBuilder` with bounded `GridX`, `GridY`, `BlockSize`, `RoadWidth`, and `MaxBuildings` properties.
- Produces: rendering tiers `Low`, `Medium`, `High`, `Cinematic` through Unreal scalability groups.

- [ ] **Step 1: Add RED city/config tests**

Assert the builder clamps grid dimensions and building count and uses engine primitive meshes only. Assert config contains Lumen/Nanite-ready defaults plus lower-quality overrides that disable or reduce expensive effects.

- [ ] **Step 2: Run RED test**

Run: `python -m unittest tests.test_verify_repo -v`
Expected: FAIL because builder/config files are absent.

- [ ] **Step 3: Implement bounded city builder and config**

Use instanced/static mesh components based on `/Engine/BasicShapes/Cube.Cube` and `/Engine/BasicShapes/Plane.Plane`; cap grids at 20x20 and buildings at 400. Roads and buildings must be generated deterministically from integer coordinates, with no external asset dependency.

- [ ] **Step 4: Run tests and verifier**

Run: `python -m unittest tests.test_verify_repo -v && python tools/verify_repo.py`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add Source Config tests
git commit -m "feat: add bounded test city and graphics presets"
```

### Task 6: Documentation, CI, and Final Source Verification

**Files:**
- Modify: `README.md`, `.github/workflows/verify.yml`, `tools/verify_repo.py`, `tests/test_verify_repo.py`

**Interfaces:**
- Produces: documented GPU-workstation build gate and CI/static verification contract.
- [ ] **Step 1: Add RED final-contract tests**

Add assertions that README documents prerequisites, clone/build steps, GPU-runtime-unverified status, key bindings, graphics tiers, licensing, and the later GPU validation checklist. Add a CI workflow assertion that executes both unittest and `tools/verify_repo.py`.

- [ ] **Step 2: Run RED test**

Run: `python -m unittest tests.test_verify_repo -v`
Expected: FAIL until docs/CI contain every required contract.

- [ ] **Step 3: Complete documentation and final verifier rules**

The GPU gate must explicitly require: generate project files, compile Development Editor, open project, spawn/possess the vehicle, verify throttle/brake/steering/handbrake, reset chase camera, verify speed/gear telemetry, and exercise Low/Medium/High/Cinematic presets.

- [ ] **Step 4: Run complete verification**

Run:

```powershell
python -m unittest discover -s tests -v
python tools/verify_repo.py
git diff --check
git status --short
```

Expected: all tests PASS, `VERIFY PASS`, `git diff --check` clean, only intentional source changes staged/unstaged before final commit.

- [ ] **Step 5: Commit and push main**

```bash
git add .
git commit -m "docs: finalize phase 1 source verification runbook"
git push origin main
```

## Final Acceptance Gate

- `main` contains all Phase 1 source/config/docs/tests and no proprietary assets.
- `python -m unittest discover -s tests -v` passes on VPS 182.
- `python tools/verify_repo.py` prints `VERIFY PASS`.
- `git diff --check` is clean.
- GitHub Actions verify workflow is green if the remote runner has completed.
- Status is reported accurately as **source-complete/static-verified, GPU-runtime-unverified** until Unreal Editor is built and run on a suitable GPU workstation.
