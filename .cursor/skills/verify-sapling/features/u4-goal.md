# Finish tape

A heart marker sits on the rightmost ledge. At Gems 3/3, touching it goes to a YOU WON screen. Below 3/3 it does not win.

## Sub-features

- `exit-marker` is a heart entity named exit on the far ledge.
- `win-at-three` confirm, collect three, touch exit, scene score, text YOU WON.
- `need-three` touching exit with fewer gems stays in game.

## How to get to it (user POV)

- Collect the three gems left to right, walk into the heart.

## Driving it with sapling-ctl

- Confirm. Screenshot `start.png` showing the heart on the right ledge.
- Collect all three (walk/jump). State `Gems: 3/3`. Screenshot `three.png`.
- Walk into the heart. Screenshot `won.png`. State scene is `score`, text includes YOU WON.

## Gotchas

- Exit radius is 0.7. Far gem is left of the heart so 3/3 happens before the tape.
- Enemy patrols the left of the start ledge, away from the route.
- Pit still retries below y 9.90.
