# Collect-and-retry loop

Three gems on the U2 course, one patrolling enemy on the high ledge. Collecting updates `Gems: n/3`. Enemy contact or falling in the pit reloads the course from the start.

## Sub-features

- `three-gems` start state has three collectible entities.
- `collect-one` walking the start gem increments HUD to `Gems: 1/3`.
- `pit-retry` walking the first gap returns the player near x=2.5 y=9.625 and restores three gems.

## How to get to it (user POV)

- Space from title. Walk into the first gem. Walk off the start ledge to retry.

## Driving it with sapling-ctl

- Title, confirm. Screenshot `start.png`. Count collectible tags == 3 and one enemy.
- Hold D ~250ms. Screenshot `collected.png`. HUD `Gems: 1/3`.
- Hold D ~800ms without jump. Screenshot `retry.png`. Player back at start, gems 0/3.

## Gotchas

- Pit threshold is player y > 10.2. Standing on the start ledge is 9.625.
- The enemy patrols the right platform; pit is the reliable death proof.
- No score-screen fail state. No exit.
