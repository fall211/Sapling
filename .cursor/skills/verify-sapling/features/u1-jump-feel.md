# Side-view jump feel

Game scene is a side-view walker: A/D walk, gravity onto the existing floor row, Space jumps with coyote time and variable height. W/S does not fly.

## Sub-features

- `grounded-idle` stands on the floor with no key held.
- `walk-right` moves on `moveRight`, then stops on release, still grounded.
- `jump-arc` raises the player (world +Y is down, so transform.y decreases at apex) then returns to the floor.
- `no-fly` ignores `moveUp`/`moveDown`.

## How to get to it (user POV)

- From the title, press Space.
- In the arena, press A/D to walk and Space to jump.

## Driving it with sapling-ctl

Preconditions:

- Healthy TechDemo PID on the title scene.
- `control-sapling doctor` is clean.
- Evidence dir `.cursor/skills/verify-sapling/evidence/u1-jump-feel/`.

- **Title.** Screenshot `title.png`. Dump `ctl state` to `title-state.json`.
- **Enter game.** Run `control-sapling ctl action confirm press`.
- **Grounded idle.** Sleep ~200ms. Screenshot `grounded.png`. Dump `grounded-state.json`. Player named `player` has `transform.y` on the floor (~9.625).
- **Walk.** Run `control-sapling key --hold d --ms 400`. Screenshot `walked.png`. Dump `walked-state.json`. `transform.x` increased. `transform.y` still on the floor.
- **Jump.** Run `control-sapling ctl action jump down`, sleep 120ms, screenshot `midair.png`, dump `midair-state.json`, then `ctl action jump up`. Mid-air `transform.y` is less than grounded y (upward on screen).
- **Land.** Sleep 400ms. Screenshot `landed.png`. Dump `landed-state.json`. `transform.y` matches grounded floor again.
- **No fly.** From grounded, `control-sapling key --hold w --ms 300`. y must not decrease.

Fail if grounded vs mid-air PNG hashes match, if mid-air y is not below grounded y, or if landed y is not back on the floor.

## Gotchas

- World +Y is down. A jump is a smaller `transform.y`, not a larger one.
- `ctl action jump press` is down+up in one frame and cuts the jump (variable height). Hold with `jump down` then `jump up`.
- `jump` and `confirm` both bind Space. Title still starts on `confirm`. Use `jump` only after the game scene is live.
- Horizontal walls are still AABB clamps (`ArenaBounds` minX/maxX), not per-tile colliders. Walking into the side wall stops via that clamp.
- Enemies still patrol in 2D; they are not part of this unit.
