# One-screen course

The game scene is a short left-to-right platformer: start ledge, gap, raised floor tiles, another gap, higher ledge. No 60s timer and no waves.

## Sub-features

- `title-copy` no longer says top-down collector.
- `start-ledge` stands at the old floor height without a key.
- `raised-platform` lands on a floor tile whose standing Y is not 9.625.
- `gap-fall` walking into the first gap increases Y (down) without jump.

## How to get to it (user POV)

- From the title, press Space.
- Walk right, jump the gap, stand on the raised tiles.

## Driving it with sapling-ctl

Preconditions: healthy PID on title, doctor clean.

- Title screenshot `title.png` must not contain `Top-Down Collector`.
- `ctl action confirm press`. Screenshot `start.png`. `player.y` near 9.625.
- Hold D into the gap without jump. State `gap-fall.json` shows y > 9.7 (in the pit) or still falling.
- Reset via quit then confirm. From start, jump right onto the mid platform. State `raised.json` shows y about 8.625, two dumps 200ms apart match, no jump held.
- Fail if raised y is 9.625, if title still says top-down, or if HUD still shows `Time:` / `Wave`.

## Gotchas

- World +Y is down. Raised platforms have *smaller* standing y.
- Jump height is about 1.6 units; platforms are one tile apart vertically.
- Bottom wall is a visible pit, not a fail state.
