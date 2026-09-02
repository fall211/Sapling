# Presentable (stacked on U4)

Title names 3 gems then the heart. In-game HUD is Gems n/3. YOU FELL already lives on feat/u4-goal. World EXIT uses game_font at the heart. No HUD sticker.

Linux GLCORE: sapling-ctl holds must persist. Unfocused X11 KEY_UP used to wipe injectKey; Input now tracks m_injectedKeys and ignores those KEY_UPs.

## Layout

- Spawn (3.4, 9.625). Gems: start 4.6, mid 9.0, skippable left 1.90.
- Platforms: (1,5,10), (7,11,9), (12,18,9). Same-height mid/far; a 1-tile drop tunneled through the floor.
- Heart (17.6, 8.15). EXIT label (16.7, 7.50) size 2.
- Slime in the first pit (6.50, 11.15), EnemyAIBounds keep it in the hole. Not on the spawn tile.

## Driving (Linux, named actions)

- Doctor, launch, ping, confirm.
- `moveRight down` ~0.20s: player x changes (3.4 → ~5.56).
- Longer hold off the start ledge: YOU FELL, then retry at 3.4.
- Jump start → mid (D 0.25, jump 0.55): Gems 2/3 if the left gem was skipped.
- Walk D on the long ledge into the heart: Need 3 gems, scene still game.
- Collect 1.90 first, same jump, walk heart: scene score, YOU WON.

Evidence: `.cursor/skills/verify-sapling/evidence/u5-stack/`
