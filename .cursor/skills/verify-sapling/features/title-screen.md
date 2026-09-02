# Title screen

The TechDemo opens on a title scene that shows `SAPLING TECH DEMO`, a floating player preview, blinking `Press SPACE to Start`, and a controls hint. Space or Enter leaves the title for the arena; Escape closes the window.

## Sub-features

- `title-first-paint` shows the title copy and preview after launch.
- `title-confirm-space` starts gameplay on Space release.
- `title-confirm-enter` starts gameplay on Enter release.
- `title-quit-esc` exits the process on Escape.

## How to get to it (user POV)

- Launch TechDemo; `initialScene` is `title`.
- From the arena, press Escape to return to title (does not quit).
- From results, press Escape to return to title.

## Driving it with sapling-ctl

Preconditions:

- `control-sapling doctor` reports a healthy `Sapling TechDemo` PID from this run.
- The window is on the title scene (just launched, or Esc from gameplay).
- Evidence directory `evidence/title-start/` exists.

- **First paint.** Look at the window after launch. Run `control-sapling screenshot --path .cursor/skills/verify-sapling/evidence/title-start/before-confirm.png`. The PNG shows `SAPLING TECH DEMO` and `Press SPACE to Start` (the prompt blinks; wait up to 1s and retry once if the prompt is transparent).
- **Start with confirm.** Run `control-sapling ctl action confirm press`. The window leaves the title copy; HUD text `Score:`, `Time:`, and `Wave` appears. `helpers/control-sapling prove` is this step with hashes and scene checks.
- **Proof after start.** Run `control-sapling screenshot --path .cursor/skills/verify-sapling/evidence/title-start/after-confirm.png`. The PNG is the arena, not the title.
- **Return.** Run `control-sapling ctl action quit press`. Title copy returns. Screenshot `evidence/title-start/back-to-title.png`.
- **Enter alias.** From title, Run `control-sapling key Return`. Arena HUD appears again (same `confirm` action as Space).
- **Quit.** Return to title, then Run `control-sapling ctl action quit press`. The recorded PID exits. Doctor then reports the process down. Do not treat this last step as the only proof of title; keep the start-game screenshots.

## Gotchas

- Confirm is `isActionUp`. Holding Space without a release does not start the game.
- Esc on title quits; Esc in the arena returns to title. Do not mix those proofs.
- The prompt color toggles every 0.6s. A screenshot with an invisible prompt is not a failed title if the header `SAPLING TECH DEMO` is visible.
- BGM starts on title enable via FMOD. Absence of a wav file on disk after play is not a bug; there is no audio dump to inspect.
- Linux Debug launches with `fmod_stub` and X11. GLCORE framebuffer screenshots work. Metal screenshot is not this recipe.
