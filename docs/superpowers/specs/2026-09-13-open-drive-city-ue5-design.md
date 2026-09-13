# Open Drive City UE5 — Design Specification

Date: 2026-09-13
Status: Approved direction, pre-implementation design
Target repository: `trinhtanphat/open-drive-city-ue5`

## 1. Goal

Build an open-source, source-first Unreal Engine 5 driving sandbox that can be developed without a local discrete GPU and later run on a GPU-capable machine.

The first playable target is a third-person chase-camera driving experience in a compact city test map. The visual direction is photoreal urban driving, while the repository stays legally clean and contains no proprietary vehicle or city assets.

## 2. Primary Constraints

- Development host 182 currently has no usable discrete NVIDIA GPU exposed to Windows.
- The repository must be useful before Unreal Engine is installed locally.
- All project-owned code and configuration must be committed to Git.
- Unreal Engine binaries, Marketplace content, DerivedDataCache, generated build products, and licensed third-party assets must not be committed.
- The project must remain easy to clone and build later on a GPU-capable workstation.

## 3. Technology Choice

The project will use Unreal Engine 5 C++ as the primary runtime and gameplay framework.

Core engine systems:

- Unreal Engine 5 C++ project structure.
- Chaos Vehicles for the eventual production vehicle simulation.
- Enhanced Input for keyboard, gamepad, and future wheel bindings.
- World Partition-ready map structure for later city expansion.
- Lumen/Nanite-ready rendering settings, guarded by scalable presets.
- Automation tests for project-owned non-rendering logic where practical.

The repository will not vendor Unreal Engine source. Developers may use an Epic-installed engine or a separately licensed source build.

## 4. Phase 1 Scope

Phase 1 delivers a compile-ready UE5 project skeleton with gameplay code that does not depend on proprietary art.

- `AOpenDriveVehiclePawn`: player-controlled vehicle pawn.
- `UOpenDriveVehicleControllerComponent`: throttle, brake, steering, handbrake intent.
- `AOpenDrivePlayerController`: input routing and camera reset.
- Chase camera with spring arm, configurable lag, FOV, and look controls.
- Speed/gear telemetry interface for future HUD use.
- Basic game mode and default pawn wiring.
- `AOpenDriveTestCityBuilder`: optional runtime/editor-friendly primitive city block generator for smoke testing without external assets.
- Graphics scalability profile definitions for Low, Medium, High, and Cinematic targets.
- README with setup/build instructions and hardware expectations.

Phase 1 explicitly excludes multiplayer, traffic AI, pedestrians, licensed car models, a large streamed city, accounts, economy, and online services.

## 5. Component Boundaries

### Vehicle Input

Input actions produce normalized driver intent only. Gameplay code must not hard-code specific keyboard keys inside the vehicle pawn.

### Vehicle Simulation

The vehicle pawn owns the UE/Chaos integration. Higher-level systems interact through stable throttle, brake, steering, and handbrake methods so the simulation can be replaced or tuned without rewriting input code.

### Camera

Camera behavior is isolated from vehicle physics. The camera reads vehicle transform and velocity but does not mutate simulation state.

### Test City

The primitive test-city builder exists only to make the project visually inspectable without external assets. Production city content will replace it without changing vehicle APIs.

### Rendering Configuration

Rendering quality is configuration-driven. Cinematic features must be optional so the same project can open on lower-end development machines.

## 6. Data and Control Flow

1. Enhanced Input captures keyboard/gamepad actions.
2. Player controller forwards normalized intent to the controlled vehicle.
3. Vehicle controller component clamps and stores current driver intent.
4. Vehicle pawn applies the intent to Chaos movement when the runtime plugin is available.
5. Camera follows the vehicle independently.
6. Telemetry exposes speed and drivetrain state to debug UI or a future HUD.

No network authority model is introduced in Phase 1.

## 7. Visual Direction

The visual target is a clean modern-city driving scene with realistic sunlight, reflections, road materials, building massing, and a low chase camera similar to modern open-world racing games.

Phase 1 uses legal primitives/placeholders only. Later visual upgrades may use user-owned Marketplace/Fab assets, Megascans content under its applicable license, or original assets, but those are not required for the repository to build.

Recommended future GPU baseline:

- 12–16 GB VRAM: practical development target.
- 24 GB VRAM: preferred for high-resolution city assets and aggressive Lumen/Nanite settings.
- System RAM: 32 GB minimum, 64 GB preferred for larger content workflows.
- Free SSD space: 150 GB or more once Unreal Engine, caches, and city assets are installed.

## 8. Error Handling and Degradation

The project must fail clearly when required engine plugins are missing and must avoid silent fallback to unrelated movement behavior.

Rendering presets must degrade gracefully: expensive effects may be disabled at lower quality tiers without changing gameplay behavior.

The test-city generator must use bounded dimensions and object counts to avoid accidental editor/runtime memory explosions.

## 9. Repository and Licensing

The repository is public and contains project-owned source, configuration, documentation, and tests only.

- Project-owned code will use the MIT License.
- Unreal Engine itself is not redistributed by this repository.
- Third-party assets must keep their original licenses and are excluded unless redistribution is explicitly allowed.
- `.gitignore` will exclude `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, IDE output, and local engine artifacts.
- Large binary assets will not be added during Phase 1.

## 10. Verification Strategy

Verification is split because host 182 cannot presently perform a real GPU/editor run.

- Static repository checks: project descriptor, module structure, includes, configs, and prohibited generated directories.
- Source-level tests for deterministic helper logic where Unreal-independent extraction is sensible.
- CI lint/static guards that do not require redistribution of Unreal Engine.
- Later GPU workstation gate: generate project files, compile Development Editor, open test map, possess vehicle, drive, brake, steer, reset camera, and validate all scalability presets.

## 11. Success Criteria

Phase 1 is complete when:

- The public GitHub repository exists with `main` as the default branch.
- A clean clone contains a valid UE5 C++ project skeleton and documented setup instructions.
- Input, vehicle-control abstraction, chase camera, telemetry, test-city builder, and graphics presets are represented in source/configuration.
- No proprietary game assets or Unreal Engine binaries are committed.
- Repository-level verification scripts pass on host 182.
- The project is explicitly marked as source-complete but GPU-runtime-unverified until a suitable GPU machine performs the editor/runtime gate.

## 12. Future Phases

After a GPU-capable workstation is available, the next phase may add production Chaos tuning, photoreal road/building assets, traffic, pedestrians, weather, day/night transitions, audio, photo mode, streamed districts, and multiplayer.

Those features are intentionally outside Phase 1 so the initial repository stays small, buildable, reviewable, and legally clean.
