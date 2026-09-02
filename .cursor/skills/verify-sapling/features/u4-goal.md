# Finish tape

A heart marker sits on the rightmost ledge. At Gems 3/3, touching it goes to a YOU WON screen. Below 3/3 it does not win. Collecting the third gem is not a win.

## Sub-features

- `exit-marker` is a heart entity named exit on the far ledge.
- `win-at-three` confirm, collect three, touch exit, scene score, text YOU WON.
- `need-three` touching exit with fewer gems stays in game.
- `gem-off-slime` last gem is on the far ledge; slime patrols the left of the start ledge. Collecting the far gem does not overlap the enemy bbox.
- `contact-kill` walking into the slime retries after a short freeze (player still overlapping).
- `pit-tell` dropping below y 9.90 shows YOU FELL for a beat, then snap-retry.

## How to get to it (user POV)

- Collect the three gems left to right, walk into the heart.
- Walk left into the slime to die. Walk off a ledge to see YOU FELL.

## Driving it with sapling-ctl

- Confirm. Screenshot `start.png` showing the heart on the right ledge.
- Collect all three (walk/jump). State `Gems: 3/3`. Screenshot `three.png`.
- Walk into the heart. Screenshot `won.png`. State scene is `score`, text includes YOU WON.
- From spawn, hold A into the slime. Screenshot `contact.png` while still overlapping, then `contact-retry.png` back at spawn.
- Walk off the start ledge. Screenshot `fell.png` with YOU FELL, then `pit-retry.png` back at spawn.

## Gotchas

- Exit radius is 0.7. Far gem is at 16.2, heart at 17.6, so 3/3 happens before the tape.
- Enemy patrols the left of the start ledge (1.55, patrol 0.35). Damage radius 0.875. Last gem is not on that tile.
- Pit still retries below y 9.90 after the YOU FELL hold.
