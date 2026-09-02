# Sapling Engine — Tech Demo

A one-screen side-view course that showcases the core features of the **Sapling Engine**.

## Features Demonstrated

| Feature | How It's Used |
|---|---|
| **Multiple Scenes** | Title screen → Gameplay → Score screen, with transitions |
| **Input System** | A/D walk, Space jump in game, Space/Enter confirm, ESC to quit — all via named actions |
| **Data Manifest** | Assets, input bindings, startup scenes, and initial scene are loaded from `Assets/manifest.json` |
| **Sprites (Static)** | Player idle, wall tiles, floor tiles, hearts |
| **Sprites (Animated)** | Player walk cycle (4 frames), gem spin (4 frames), enemy bounce (4 frames) |
| **Text Rendering** | HUD score, timer, wave counter, title text, score screen rating |
| **Audio** | Background music (looping), collect SFX, hurt SFX, scene transition, victory jingle |
| **Custom Components** | `PlayerController` (walk/jump/coyote), `Collectible`, `FloatMotion`, `EnemyAI` |
| **Custom Systems** | `sPlayerMovement` (gravity, jump, floor), `sCollectibles`, `sFloatMotion`, `sEnemyAI` |
| **Inter-Scene Messaging** | Final score is sent from GameScene → ScoreScene via `Engine::sendToScene` |
| **Entity Tags & Queries** | Entities tagged as `"player"`, `"collectible"`, `"enemy"`, `"hud"`, etc. |
| **Bounding Box Collision** | Player has a `BBox` component; distance-based overlap checks for gems and enemies |
| **GUI Transforms** | HUD elements positioned in screen-space with anchored pivots |
| **Sprite Layers** | Background tiles, Midground gems/enemies, Player layer, UI layer |
| **Entity Lifecycle** | Gems are spawned dynamically and destroyed on collection |

## How to Build & Run

### Prerequisites

- **CMake** 3.10+
- **C++20** compatible compiler (Clang, GCC, MSVC)
- **Python 3** (for generating placeholder assets)
- **FMOD** libraries (included in the engine's `third_party/fmod/` directory)

### Quick Start

From the `Examples/TechDemo/` directory:

```bash
./build_and_run.sh
```

This script will:
1. Generate placeholder sprite and audio assets via Python
2. Copy a system font for text rendering
3. Configure CMake (Debug build)
4. Build the project
5. Run the game

### Build Options

```bash
./build_and_run.sh              # Debug build (default)
./build_and_run.sh release      # Release build
./build_and_run.sh clean        # Clean rebuild
./build_and_run.sh clean release  # Clean release build
```

### Manual Build

```bash
# 1. Generate assets
python3 generate_assets.py

# 2. Configure
cmake -B build/Debug -DCMAKE_BUILD_TYPE=Debug -S .

# 3. Build
cmake --build build/Debug

# 4. Run
./build/Debug/TechDemo
```

## Controls

| Key | Action |
|---|---|
| `A` / `←` | Walk left |
| `D` / `→` | Walk right |
| `Space` | Jump (in game) / Start (title) |
| `Enter` | Confirm / Start |
| `Escape` | Quit / Back to title |

## Gameplay

- Walk with A/D. Space jumps. Cross the gap onto raised floor tiles.
- Each gem is worth 10, 20, or 50 points
- Avoid red enemies — they take away health on contact
- New enemy waves spawn every 20 seconds, increasing difficulty
- The game ends when the 60-second timer runs out or you lose all 3 hearts
- Try to get the highest score!

## Project Structure

```
Examples/TechDemo/
├── main.cpp                  # Entry point — engine init, scene type registration, manifest load
├── CMakeLists.txt            # Build configuration
├── build_and_run.sh          # One-step build & run script
├── generate_assets.py        # Generates placeholder PNGs and WAVs
├── README.md                 # This file
├── Scenes/
│   ├── TitleScene.hpp/.cpp   # Title screen with animated preview
│   ├── GameScene.hpp/.cpp    # Main gameplay — arena, player, gems, enemies, HUD
│   └── ScoreScene.hpp/.cpp   # Results screen with score and rating
├── Components/
│   ├── PlayerController.hpp  # Player movement speed, score, health, facing
│   ├── Collectible.hpp       # Point value and collection radius
│   ├── FloatMotion.hpp    # Sine-wave bobbing effect
│   └── EnemyAI.hpp           # Patrol (horizontal/vertical) and chase behaviors
├── Systems/
│   ├── sPlayerMovement.hpp   # Input → velocity, wall clamping, animation switching
│   ├── sCollectibles.hpp     # Distance-based gem collection and scoring
│   ├── sFloatMotion.hpp    # Applies float animation offsets to sprites
│   └── sEnemyAI.hpp          # Enemy movement, patrol reversal, player damage
└── Assets/
    ├── manifest.json         # Data-driven asset, input, and startup scene registration
    ├── Sprites/              # Generated PNG sprite sheets
    ├── Audio/                # Generated WAV sound effects and music
    └── Fonts/                # TTF font (copied from system)
```

## Troubleshooting

### No font file found
The asset generator tries to copy a system font (Arial, DejaVuSans, etc.). If it can't find one, manually copy any `.ttf` file to `Assets/Fonts/game_font.ttf`.

### FMOD not found
Make sure the Sapling engine's `third_party/fmod/` directory contains the FMOD SDK libraries for your platform.

### Build errors about C++20
Ensure your compiler supports C++20. On macOS, use a recent Xcode Command Line Tools. On Linux, GCC 10+ or Clang 12+.
