---
name: verify-sapling
description: "Drive and prove the Sapling TechDemo native Sokol window (title, arena, results, WASD/Space/Esc). Use for /verify-sapling, after engine/example changes, or when there is no other scripted way to prove the desktop demo."
---

# Verify Sapling TechDemo

Sapling is a C++20 2D ECS engine. The user-facing app is **TechDemo**, a Sokol (`sokol_app`) desktop window titled `Sapling TechDemo` (1280x720 client from a 320x180 world scaled 4x). It is not a web app and has no HTTP port. Drive the real binary. Debug: `sapling-ctl` verbs go through `Input` and the Sokol framebuffer. Do not call engine setters or scene factories as a substitute.

This skill is for the next agent. Run **Doctor** before driving. If doctor is unhealthy, stop; do not invent a headless renderer or a test-only entry point.

## Launch

Primary documented command (from `Examples/TechDemo/`):

```bash
./build_and_run.sh
```

That script generates missing sprites/audio if needed, copies a TTF when Python can find one, configures CMake Debug, builds `TechDemo`, and `exec`s the binary. Manual equivalent:

```bash
cd Examples/TechDemo
python3 generate_assets.py          # only if sprites/audio/font are missing
cmake -B build/Debug -DCMAKE_BUILD_TYPE=Debug -S .
cmake --build build/Debug --parallel
./build/Debug/TechDemo              # Debug on macOS, Windows, or Linux (Linux audio is stubbed)
```

**Ready when:** stdout contains `[INFO] Engine init completed` and a window titled `Sapling TechDemo` is owned by the PID you started. Also expect `[INFO] Input init completed`. `AssetManager::initialize` currently prints `[INFO] AudioEngine init completed` as well (same string as audio init); do not treat a second copy of that line as a second subsystem.

**Harness launch** (records PID, log path, run id; does not steal an already-open demo):

```bash
.cursor/skills/verify-sapling/helpers/control-sapling launch
```

Optional: `control-sapling launch --build-only` configures/builds without `exec`. Isolation env: `SAPLING_VERIFY_RUN_ID` (default: timestamp). State file: `/tmp/sapling-verify-$RUN_ID/state.env`. Two Sokol windows can exist, but they share the display and have no data-dir isolation — never send keys to a window this run did not create.

**Teardown:** `control-sapling cleanup` (kills only the recorded PID). Do not `pkill TechDemo`.

### Platform reality

- macOS: Metal + real FMOD dylibs.
- Windows: D3D11 + FMOD DLLs.
- Linux Debug: Sokol `SOKOL_GLCORE`, CMake links X11/Xi/Xcursor/OpenGL, audio uses `third_party/fmod_stub` (silent). `Texture.cpp` includes `<cstring>`.

Debug copies assets next to the executable (`ASSETS_PATH`). Release on macOS is `build/Release/TechDemo.app`. Linux audio is a stub; do not treat missing SFX as a scene-change failure.

Debug TechDemo also listens on `127.0.0.1:17321` (`SAPLING_AGENT_PORT` overrides) for `sapling-ctl`.

## Doctor

Read-only. Answers: is *this* instance worth driving?

```bash
.cursor/skills/verify-sapling/helpers/control-sapling doctor
```

Require:

- Repo is the Sapling checkout (CMake + `Examples/TechDemo/Assets/manifest.json`).
- Host is Apple, Win32, or Linux Debug with the FMOD stub.
- Platform FMOD files exist under `third_party/fmod/` (or `third_party/fmod_stub` on Linux).
- `Assets/Fonts/game_font.ttf`, `Assets/Sprites/player.png`, `Assets/Audio/bgm.wav` exist.
- After launch: recorded PID is alive, log contains `[INFO] Engine init completed`, window title is `Sapling TechDemo`.

Exit `0` only when the running instance is healthy. Exit `2` for a known platform/link blocker. Exit `1` for missing tools/assets/process.

Never drive a TechDemo that doctor did not attribute to this run's PID.

## Drive

Harness: `control-sapling` for process/log lifecycle. Debug builds also expose `sapling-ctl` (JSONL on localhost). Prefer that path. It injects through `Input` (named actions) and writes framebuffer PNGs. Do not use OS key injection or a desktop screenshot tool when `sapling-ctl` replies `ok`.

Built next to TechDemo: `Examples/TechDemo/build/Debug/sapling-ctl`.

Manifest actions (`Examples/TechDemo/Assets/manifest.json`):

| Action | Keys |
|---|---|
| `moveLeft` | LEFT, A |
| `moveRight` | RIGHT, D |
| `moveUp` | UP, W |
| `moveDown` | DOWN, S |
| `confirm` | SPACE, ENTER |
| `quit` | ESCAPE |

Scenes: `title` (initial) → Space starts `game` → timer/death then Space → `score` → Space replays `game`, Esc returns `title`. Esc on `title` requests quit (`sapp_request_quit`). Esc during `game` returns to `title` (does not quit). Confirm uses `Input::isActionUp` (key **release**).

```bash
Examples/TechDemo/build/Debug/sapling-ctl ping
Examples/TechDemo/build/Debug/sapling-ctl state
Examples/TechDemo/build/Debug/sapling-ctl action confirm press
Examples/TechDemo/build/Debug/sapling-ctl key W down
Examples/TechDemo/build/Debug/sapling-ctl screenshot .cursor/skills/verify-sapling/evidence/<feature>/shot.png
.cursor/skills/verify-sapling/helpers/control-sapling key space
.cursor/skills/verify-sapling/helpers/control-sapling key --hold w --ms 400
.cursor/skills/verify-sapling/helpers/control-sapling screenshot --path .cursor/skills/verify-sapling/evidence/<feature>/shot.png
.cursor/skills/verify-sapling/helpers/control-sapling log-grep --pattern "Engine init completed"
```

`control-sapling key` / `screenshot` call `sapling-ctl` when that binary exists. Confirm uses `Input::isActionUp` (`press` is down then up in one frame). Prefer action names (`confirm`). Recipes live in `features/`.

## Evidence

Keep proof under `.cursor/skills/verify-sapling/evidence/<feature-id>/` (survives cleanup). Do not use `*.log` filenames; repo `.gitignore` drops them. Use `.txt` and `.png`.

Standards:

- Exercise the Sokol window with the same keys a player uses. Do not `changeScene` from a helper.
- Capture **before** and **after** screenshots plus the matching stdout excerpt. Title vs arena vs results are visually distinct (`SAPLING TECH DEMO` / HUD `Score:` `Time:` `Wave` / `RESULTS`).
- Side effects: scene change SFX is FMOD-only (not observable as a file). Observable proof is the window contents and Debug/INFO lines. Game-over `Debug::log` is `Logger::debug` and only prints when the binary was compiled with `DEBUG`.
- Record feature ID, run id, PID, host OS, and git rev in `meta.txt`.
- If the window cannot open, store doctor + build transcript and do not claim pixels from another surface.

## Cleanup

```bash
.cursor/skills/verify-sapling/helpers/control-sapling cleanup
```

Sends SIGTERM then SIGKILL to the PID in `state.env` only. Removes `/tmp/sapling-verify-$RUN_ID/` scratch (stdout copy is already in evidence if you copied it). Leaves `.cursor/skills/verify-sapling/evidence/`. Does not delete `Examples/TechDemo/build/` (rebuild cache) or `Assets/`.

After cleanup, confirm evidence files still exist before reporting.

## Helpers

All helpers are executable. Invoke from repo root as shown.

| Command | What it does |
|---|---|
| `helpers/control-sapling launch` | CMake Debug build of TechDemo if needed; starts binary; writes PID/log to `/tmp/sapling-verify-$RUN_ID/state.env`; waits for `[INFO] Engine init completed` or fails with the compiler/linker error. |
| `helpers/control-sapling launch --build-only` | Configure/build only. |
| `helpers/control-sapling doctor` | Read-only health / platform blocker report. |
| `helpers/control-sapling key space` | Logs confirm intent; on Linux exits 2. On a live window, press Space (key-up) via the Computer tool. |
| `helpers/control-sapling key --hold w --ms 400` | Logs a hold intent; hold W ~400ms on the live window via the Computer tool. |
| `helpers/control-sapling screenshot --path <png>` | Logs the evidence path; save the Sokol window PNG there with the Computer tool. |
| `helpers/control-sapling log-grep --pattern <re>` | Grep the run's stdout file. |
| `helpers/control-sapling cleanup` | Kill recorded PID; delete scratch dir; keep evidence. |

`control-sapling` owns process lifecycle. `sapling-ctl` owns input, state, and framebuffer capture in Debug. Put either on `PATH` only as a wrap of the built/repo path.

Feature map: `features/README.md`.
