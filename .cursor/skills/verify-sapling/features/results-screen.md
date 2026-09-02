# Results screen

When the 60-second timer hits zero or health reaches zero, the arena shows a game-over prompt; Space then opens a `RESULTS` scene with the final score, a rating string, and `Press SPACE to Play Again`. Space replays the arena; Escape returns to the title.

## Sub-features

- `results-gameover-prompt` shows continue copy on the arena after death or timeout.
- `results-score` shows `RESULTS` and `Your Score` after confirm from game-over.
- `results-replay` starts a fresh arena on Space from results.
- `results-esc-title` returns to title on Escape from results.

## How to get to it (user POV)

- Play until time expires (60s) or lose three hearts, then press Space.
- There is no menu item that jumps here.

## Driving it with control-sapling

Preconditions:

- Healthy TechDemo PID.
- Prefer a Debug build so `[DEBUG]` game-over lines appear.

- **Reach game-over.** From title, Run `control-sapling key space`. Either wait until `Time:` reads `0` (about 60s) or collide with enemies until hearts are gone. Grep with `control-sapling log-grep --pattern "Game over"`. Screenshot `evidence/results-screen/gameover.png` while still on the arena overlay.
- **Open results.** Run `control-sapling key space`. Screenshot `evidence/results-screen/results.png`. The PNG shows `RESULTS` and `Your Score`.
- **Replay.** Run `control-sapling key space`. HUD `Score: 0` / `Wave 1` returns. Screenshot `evidence/results-screen/replay.png`.
- **Esc from results.** Get back to results (timeout or death + Space), then Run `control-sapling key escape`. Title `SAPLING TECH DEMO` returns. Screenshot `evidence/results-screen/esc-title.png`.

## Gotchas

- The first Space after game-over is `confirm` on the **arena**, which `sendToScene("score", "final_score", ...)` then `changeScene("score")`. A second Space on results replays. Do not collapse those into one key.
- Ratings (`AMAZING!`, `Great Job!`, `Not Bad!`, `Keep Trying!`, `Better Luck Next Time`) depend on score thresholds (500/300/150/1/0). Assert the header `RESULTS`, not a specific rating, unless the run earned a known score.
- Waiting 60s is the only honest timeout path. Do not call `m_gameOver` from a helper.
- Inter-scene score is messaging, not a file. The on-screen number is the user-visible side effect.
