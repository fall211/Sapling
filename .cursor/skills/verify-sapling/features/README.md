# Sapling TechDemo verification map

This directory is the maintained source for verifying user-facing behavior of the Sapling TechDemo Sokol window. Read this index, then use the matching feature file. The app is native, not a browser.

## Baseline preconditions

- Host is macOS or Windows with the FMOD binaries already in `third_party/fmod/`, or Linux Debug with `third_party/fmod_stub` plus X11/GL (see skill Launch).
- Work from the Sapling repo root. TechDemo lives at `Examples/TechDemo/`.
- Debug build so `ASSETS_PATH` points at `Examples/TechDemo/Assets/` and `[DEBUG]` logs exist.
- `Examples/TechDemo/Assets/manifest.json` is the live input/asset/scene table. `initialScene` is `title`.
- Start with `helpers/control-sapling launch`. Record `SAPLING_VERIFY_RUN_ID`.
- Run `helpers/control-sapling doctor` and require PID, `AgentCli listening`, and `sapling-ctl ping` ok.
- Never send keys to a TechDemo this run did not start.

## Driving conventions

- Start every recipe from the title screen unless the feature says otherwise (Esc from gameplay returns to title; Esc from title quits the process).
- Treat action names as literal: `confirm`, `quit`, `jump`, `moveLeft`, `moveRight`, `moveUp`, `moveDown`.
- `confirm` is edge-triggered on **key up**. A held Space does not start the game until release.
- Drive through `control-sapling ctl` and `screenshot`. Do not use engine APIs or a Computer-tool capture as the user path.
- Cleanup must not delete proof under `evidence/`.

## Proof and skip reporting

- Capture the key action and the resulting scene, not only a final screenshot.
- Window proof is a PNG that shows the scene's distinctive text (`SAPLING TECH DEMO`, HUD `Score:`/`Time:`/`Wave`, or `RESULTS`).
- Log proof is the command, stdout/stderr excerpt, and exit code.
- Record feature ID, entry point, PID, OS, and git rev in `evidence/<id>/meta.txt`.
- If FMOD or the window is missing, keep the doctor/build transcript and do not mark title/arena/results as verified. Wrong-surface pixels are inconclusive, not a pass.

## Feature entry contract

Each feature file starts with an H1 and one paragraph, then exactly four H2s: `Sub-features`, `How to get to it (user POV)`, `Driving it with sapling-ctl`, `Gotchas`.

## Features

- [Title screen](./title-screen.md) covers first paint, Space/Enter to start, Esc to quit.
- [Arena gameplay](./arena-gameplay.md) covers WASD/arrows, gems, enemies, HUD, Esc back to title.
- [Results screen](./results-screen.md) covers game-over continue, score handoff, replay, Esc to title.
- [Named input actions](./named-input-actions.md) covers manifest bindings for confirm, quit, jump, and movement.
- [Side-view jump feel](./u1-jump-feel.md) covers grounded idle, A/D walk, Space jump, coyote/variable height.
- [One-screen course](./u2-course.md) covers raised platforms, a jump gap, and no survival timer.
- [Collect-and-retry loop](./u3-loop.md) covers three gems, one enemy, and pit/enemy retry.
