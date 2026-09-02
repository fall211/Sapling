# Agent CLI

## Problem

Agents that make games or engine changes need to drive a live Debug TechDemo: send input, read scene state, grab the real framebuffer, change scenes. Sokol owns the OS window and event loop, so a competing stdin REPL fights `sapp_run`. Games already go through named `Input` actions, not raw OS keys.

## Usage

Debug TechDemo listens on `127.0.0.1:17321` (override `SAPLING_AGENT_PORT`).

```
sapling-ctl ping
sapling-ctl state
sapling-ctl screenshot /tmp/frame.png
sapling-ctl action confirm press
sapling-ctl key W down
sapling-ctl scene game
sapling-ctl quit
```

Each command writes one JSONL `AgentRequest` and prints one `AgentReply`.

## Shape

Wire types: `AgentRequest { id, verb, args }` and `AgentReply { id, ok, error?, payload }`.

Verbs live in a table mapping string to a game-thread handler. A background thread accepts TCP, parses JSONL, and only enqueues requests / writes replies. `Engine::update` drains the queue. Input injection builds the same `sapp_event` path `Input::update` already handles. Screenshot is a pending capture after `sg_commit`, written with `stb_image_write`. Engine public surface is bind/start/drain/afterPresent plus `Input::injectKey`.

## Synthesis decision

Picked A (localhost JSONL + `sapling-ctl`) over B (stdin REPL) and C (embedded HTTP). A keeps Sokol as the only event loop, hides sockets behind a small CLI, and adds no HTTP library. B races `sapp_run` for stdin. C is a heavier public surface for the same verbs.

## Tradeoffs accepted

- We accept TCP localhost (not a unix socket) so Windows can compile the same protocol without a second transport.
- We accept GLCORE `glReadPixels` as the proven screenshot path on Linux. Metal/D3D reply with an error until a Sokol readback exists there.
- We accept a Linux FMOD header stub and X11/GL link so Debug TechDemo can run on this box. Release audio on macOS/Windows is unchanged.

## Alternatives considered

- B stdin REPL: caller must share stdin with Sokol. Hides nothing; exposes loop ownership to the agent.
- C HTTP server: richer tooling surface, extra dependency, same verbs.

## Next implementation step

Add protocol types, the verb table, POSIX listen loop, and a Debug-only Engine hook.
