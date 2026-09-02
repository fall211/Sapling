# Presentable (stacked on U4)

Title names 3 gems then the heart. In-game HUD is Gems n/3. YOU FELL already lives on feat/u4-goal. World EXIT is a small caption on the heart (transform.scale 0.18, font size 1). Not a billboard.

Linux GLCORE: sapling-ctl holds persist via m_injectedKeys.

## Layout

- Spawn (3.4, 9.625). Gems: start 4.6, mid 9.0, skippable left 1.90.
- Platforms: (1,5,10), (7,11,9), (12,18,9).
- Heart (17.6, 8.15). EXIT label (17.6, 7.62).
- Slime in the first pit (6.50, 11.15).
- Pit fail at y > 10.50 so the YOU FELL beat shows the player in the hole, then snap to spawn.

## Driving (Linux, named actions)

- Doctor, launch, ping, confirm.
- Walk off the start ledge, wait until y > 10.50: YOU FELL, then retry at 3.4 Gems 0/3.
- Jump start → mid: Gems 2/3 if the left gem was skipped. Walk into the heart: Need 3 gems.
- Collect 1.90 first, same jump, walk heart: scene score, YOU WON.

Evidence: `.cursor/skills/verify-sapling/evidence/u5-stack/`
