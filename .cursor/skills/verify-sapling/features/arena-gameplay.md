# Arena gameplay

Gameplay is a top-down collector: a player sprite in a tiled arena, spinning gems, patrolling enemies, and a HUD for score, timer, wave, hearts, and gem count. WASD or arrows move; touching gems scores; touching enemies costs hearts; Escape returns to the title.

## Sub-features

- `arena-hud` shows `Score:`, `Time:`, `Wave`, hearts, and `Gems:` after confirm from title.
- `arena-move` moves the player with `moveUp`/`moveDown`/`moveLeft`/`moveRight`.
- `arena-collect` increases score/gem count when walking over a gem.
- `arena-esc-title` returns to the title without ending the process.

## How to get to it (user POV)

- From the title, press Space or Enter.
- From results, press Space to play again (resets the arena).

## Driving it with sapling-ctl

Preconditions:

- Healthy TechDemo PID on the title scene.
- `control-sapling doctor` is clean.

- **Enter arena.** Run `control-sapling ctl action confirm press`. Screenshot `evidence/arena-gameplay/hud.png`. HUD includes `Score: 0`, a `Time:` value near 60, and `Wave 1`.
- **Move.** Hold W then D. Run `control-sapling key --hold w --ms 400` and `control-sapling key --hold d --ms 400`. The player sprite leaves center; capture `evidence/arena-gameplay/moved.png` and compare to `hud.png`.
- **Collect (best effort).** Keep walking toward visible gems with WASD holds. When `Gems:` or `Score:` on the HUD increases, screenshot `evidence/arena-gameplay/scored.png`. If no gem is reached in ~10s, record that miss; do not poke collectible components.
- **Esc to title.** Run `control-sapling ctl action quit press`. Window shows `SAPLING TECH DEMO` again; PID still alive. Screenshot `evidence/arena-gameplay/esc-title.png`.

## Gotchas

- Movement is normalized on diagonals and clamped to arena bounds. A hold against a wall is not a failed move.
- Enemies damage on contact with a cooldown; walking into red sprites can end the run before a collect proof.
- The timer starts at 60s and waves spawn on a 20s clock. Do not use a 60s wall-clock wait as the primary gameplay proof.
- HUD is screen-space GUI transforms; window size is 1280x720 from a 320x180 world times 4.
- `Time's up! Game over.` and `Player died! Game over.` only print as `[DEBUG]` in Debug builds.
