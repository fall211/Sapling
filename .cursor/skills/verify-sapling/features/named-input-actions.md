# Named input actions

Players drive TechDemo through named actions loaded from `Assets/manifest.json`: movement on WASD and arrows, confirm on Space and Enter, quit on Escape. The same action names are what the scenes call; the keys are data, not hard-coded scene logic.

## Sub-features

- `input-confirm-space` and `input-confirm-enter` both start the game from title.
- `input-move-wasd` and `input-move-arrows` both move the player.
- `input-quit-title` exits from title; `input-quit-game` returns to title from the arena.

## How to get to it (user POV)

- Launch the demo and use the keyboard; there is no remapping UI.
- Read the title hint `WASD / Arrows to Move  |  ESC to Quit`.

## Driving it with sapling-ctl

Preconditions:

- Healthy TechDemo PID on title.
- Manifest still lists `confirm` keys `SPACE`,`ENTER`; `quit` key `ESCAPE`; movement `W/A/S/D` and arrows.

- **Space confirm.** Run `control-sapling ctl action confirm press`. Arena HUD appears.
- **Reset.** Run `control-sapling ctl action quit press` to title.
- **Enter confirm.** Run `control-sapling key Return`. Arena HUD appears again.
- **WASD.** Run `control-sapling key --hold a --ms 300`. Screenshot `evidence/named-input/wasd.png`.
- **Arrows.** Run `control-sapling key --hold Right --ms 300`. Screenshot `evidence/named-input/arrows.png` showing further displacement vs `wasd.png`.
- **Quit vs back.** From arena, Run `control-sapling ctl action quit press` (title, PID alive). From title, Run `control-sapling ctl action quit press` (PID exits).

## Gotchas

- Sokol key names in the helper are X11/mac keysyms (`space`, `Return`, `Escape`, `w`, `Left`). Manifest names are `SPACE`, `ENTER`, `ESCAPE`, `W`, `LEFT`. Prefer action names.
- `Input::update` ignores `key_repeat`. Holding a key is one down plus one up; `--hold` is the supported way to move.
- Unmapped keys do nothing; do not use them as negative proof unless doctor already confirmed the manifest.
- Changing `manifest.json` without relaunch does not hot-reload input.
