# Presentable

Title names the goal. Pit shows YOU FELL then retries. In-game HUD is gems only.

## Sub-features

- `title-goal` subtitle is 3 gems then the heart. Controls stay on the title.
- `pit-tell` dropping below y 9.90 shows YOU FELL, then snap to 2.5, 9.625.
- `quiet-hud` in-game HUD is Gems n/3. No A/D sticker.

## Driving it with sapling-ctl

- Screenshot title before confirm. Text includes 3 gems and heart.
- Confirm. Walk off the start ledge. Screenshot `fell.png` with YOU FELL. After the beat, state player at 2.5, 9.625.
- Collect three, touch heart. Scene score, YOU WON.

## Gotchas

- Spawn is still 2.5, 9.625. Walk D off the start ledge (tiles 1-5) to pit. Do not walk A into the slime first.
- Win path is unchanged from U4.
