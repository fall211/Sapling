---
name: verify-sapling
description: "Drive and prove the Sapling TechDemo native Sokol window (title, arena, results, WASD/Space/Esc). Use for /verify-sapling, after engine/example changes, or when there is no other scripted way to prove the desktop demo."
---

# Verify Sapling TechDemo

Sapling is a C++20 2D ECS engine. The user-facing app is **TechDemo**, a Sokol (`sokol_app`) desktop window titled `Sapling TechDemo` (1280x720 client from a 320x180 world scaled 4x). It is not a web app and has no HTTP port. Drive the real binary with keyboard actions from `Assets/manifest.json`. Do not call engine setters or scene factories as a substitute for that path.

This skill is for the next agent. Run **Doctor** before driving. If doctor is unhealthy, stop; do not invent a Linux-only stub, a headless renderer, or a test-only entry point.

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
./build/Debug/TechDemo              # macOS/Windows Debug; see blockers below
```

**Ready when:** stdout contains `[INFO] Engine init completed` and a window titled `Sapling TechDemo` is owned by the PID you started. Also expect `[INFO] Input init completed`. `AssetManager::initialize` currently prints `[INFO] AudioEngine init completed` as well (same string as audio init); do not treat a second copy of that line as a second subsystem.

**Harness launch** (records PID, log path, run id; does not steal an already-open demo):

```bash
.cursor/skills/verify-sapling/helpers/control-sapling launch
```

Optional: `control-sapling launch --build-only` configures/builds without `exec`. Isolation env: `SAPLING_VERIFY_RUN_ID` (default: timestamp). State file: `/tmp/sapling-verify-$RUN_ID/state.env`. Two Sokol windows can exist, but they share the display and have no data-dir isolation — never send keys to a window this run did not create.

**Teardown:** `control-sapling cleanup` (kills only the recorded PID). Do not `pkill TechDemo`.

### Platform reality (do not invent a working Linux binary)

Interviewed from this checkout (`CMakeLists.txt` at repo root and `Examples/TechDemo/CMakeLists.txt`, `third_party/fmod/`):

- CMake links FMOD only under `if(APPLE)` (`libfmod.dylib` / `libfmodstudio.dylib`) or `elseif(WIN32)` (`fmod_vc.lib` + DLLs). There is no UNIX/Linux branch.
- `third_party/fmod` ships macOS dylibs and Windows x86/x64/arm64 DLLs/libs. There is **no** `libfmod.so` / `libfmodstudio.so`.
- `src/Renderer/Sprout.cpp` selects `SOKOL_GLCORE` on non-Apple/non-Win32, so a Linux binary would also need X11, Xi, Xcursor, Xkb, and OpenGL. CMake does not add those libraries either.
- libstdc++ GCC also fails `src/Renderer/Texture.cpp` (`std::memcpy` without `<cstring>`) before link. Apple/MSVC may hide that.

On Linux this box, after installing cmake/g++ and adding `<cstring>` locally (not kept in-tree), the Debug target compiled then failed link with undefined FMOD C++ API, `X*`/`XI*`/`Xcursor*`/`Xkb*`/`Xrm*`, and `gl*` symbols. **There is no working Linux launch path in this repo.** Do not stub FMOD or add unverified GL/X11 lines just to get a window.

**Supported verification hosts:** macOS (Metal + FMOD dylibs) and Windows (D3D11 + FMOD DLLs), matching CMake. Debug copies assets next to the executable (`ASSETS_PATH` compile def). Release on macOS is `build/Release/TechDemo.app`.

## Doctor

Read-only. Answers: is *this* instance worth driving?

```bash
.cursor/skills/verify-sapling/helpers/control-sapling doctor
```

Require:

- Repo is the Sapling checkout (CMake + `Examples/TechDemo/Assets/manifest.json`).
- Host is Apple or Win32 **or** doctor explicitly reports Linux as blocked (then do not drive).
- Platform FMOD files exist under `third_party/fmod/`.
- `Assets/Fonts/game_font.ttf`, `Assets/Sprites/player.png`, `Assets/Audio/bgm.wav` exist.
- After launch: recorded PID is alive, log contains `[INFO] Engine init completed`, window title is `Sapling TechDemo`.

Exit `0` only when the running instance is healthy. Exit `2` for a known platform/link blocker. Exit `1` for missing tools/assets/process.

Never drive a TechDemo that doctor did not attribute to this run's PID.

## Drive

Harness: `control-sapling` for process/log lifecycle. Sokol has no ARIA tree and no debug port. Stable handles are **window title**, **manifest action names**, and **on-screen text** baked by the scenes. `control-sapling key` / `screenshot` record intent and the target path; they do not inject input or grab pixels. On a host where TechDemo actually opened, focus the `Sapling TechDemo` window with the desktop Computer tool, press the keys, and save PNGs to the `--path` given.

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
.cursor/skills/verify-sapling/helpers/control-sapling key space
.cursor/skills/verify-sapling/helpers/control-sapling key --hold w --ms 400
.cursor/skills/verify-sapling/helpers/control-sapling screenshot --path .cursor/skills/verify-sapling/evidence/<feature>/shot.png
.cursor/skills/verify-sapling/helpers/control-sapling log-grep --pattern "Engine init completed"
```

Focus the TechDemo window before keys. Prefer action names in recipes (`confirm`, not raw scan codes). Recipes live in `features/`.

## Evidence

Keep proof under `.cursor/skills/verify-sapling/evidence/<feature-id>/` (survives cleanup). Do not use `*.log` filenames; repo `.gitignore` drops them. Use `.txt` and `.png`.

Standards:

- Exercise the Sokol window with the same keys a player uses. Do not `changeScene` from a helper.
- Capture **before** and **after** screenshots plus the matching stdout excerpt. Title vs arena vs results are visually distinct (`SAPLING TECH DEMO` / HUD `Score:` `Time:` `Wave` / `RESULTS`).
- Side effects: scene change SFX is FMOD-only (not observable as a file). Observable proof is the window contents and Debug/INFO lines. Game-over `Debug::log` is `Logger::debug` and only prints when the binary was compiled with `DEBUG`.
- Record feature ID, run id, PID, host OS, and git rev in `meta.txt`.
- If launch is impossible (Linux FMOD/X11/GL as above), store doctor + build transcript and **do not** claim the feature verified via a different path.

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

`helpers/control-sapling` is the only supported driver. Put `control-sapling` on `PATH` only if you wrap that script; recipes below assume repo-relative invocation.

Feature map: `features/README.md`.
