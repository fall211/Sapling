# Presentable

Title names the goal. Pit shows YOU FELL then retries. In-game HUD is gems plus an EXIT label. Far ledge is a doorway.

## Sub-features

- `title-goal` subtitle is 3 gems then the exit. Controls stay on the title.
- `pit-tell` dropping below y 9.90 shows YOU FELL, then snap to 2.5, 9.625.
- `quiet-hud` Gems n/3 and EXIT. No A/D sticker.
- `gem-lock` touching the heart at <3 gems stays in game and shows Need 3 gems. Third gem is on the mid ledge so the heart can be reached at 2/3.
- `slime-off-spawn` enemy patrols the left of the far ledge, not the start tile.

## Driving it with sapling-ctl

- Screenshot title: 3 gems. Then the exit.
- Confirm. Start shot: player 2.5,9.625, slime on far ledge, EXIT in HUD.
- Collect start gem and one mid gem, skip the other mid gem, reach the heart. State Gems 2/3, scene game, Need 3 gems.
- Collect the last mid gem, touch heart, YOU WON.
- Walk off start to the right. YOU FELL then retry 2.5,9.625.

## Gotchas

- Gems at 4.0 start, 7.4 and 9.0 mid. Heart at 17.6. Enemy at 12.8, 7.625.
- Do not walk A at spawn; the slime is no longer there.
