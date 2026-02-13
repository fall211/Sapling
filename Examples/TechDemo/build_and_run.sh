#!/usr/bin/env bash
#
# build_and_run.sh — Generate assets, build, and run the Sapling TechDemo
#
# Usage:
#   ./build_and_run.sh          # Debug build (default)
#   ./build_and_run.sh release  # Release build
#   ./build_and_run.sh clean    # Remove build directory and regenerate
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_TYPE="Debug"
CLEAN=false

for arg in "$@"; do
    case "$arg" in
        release|Release)
            BUILD_TYPE="Release"
            ;;
        clean)
            CLEAN=true
            ;;
        -h|--help)
            echo "Usage: $0 [debug|release] [clean]"
            echo ""
            echo "  debug    Build in Debug mode (default)"
            echo "  release  Build in Release mode"
            echo "  clean    Remove build directory before building"
            echo ""
            exit 0
            ;;
    esac
done

BUILD_DIR="build/${BUILD_TYPE}"

# ── Clean if requested ───────────────────────────────────
if $CLEAN; then
    echo "==> Cleaning build directory: ${BUILD_DIR}"
    rm -rf "$BUILD_DIR"
fi

# ── Step 1: Generate assets ──────────────────────────────
echo ""
echo "========================================"
echo "  Step 1: Generating Assets"
echo "========================================"
echo ""

if [ ! -f "Assets/Sprites/player.png" ] || [ ! -f "Assets/Audio/bgm.wav" ] || [ ! -f "Assets/Fonts/game_font.ttf" ]; then
    if command -v python3 &> /dev/null; then
        python3 generate_assets.py
    elif command -v python &> /dev/null; then
        python generate_assets.py
    else
        echo "ERROR: Python is required to generate assets."
        echo "Please install Python 3 and try again."
        exit 1
    fi
else
    echo "Assets already generated. Skipping. (Use 'clean' to regenerate)"
fi

# Verify critical assets exist
if [ ! -f "Assets/Sprites/player.png" ]; then
    echo "ERROR: Asset generation failed - player.png not found."
    exit 1
fi

if [ ! -f "Assets/Fonts/game_font.ttf" ]; then
    echo ""
    echo "WARNING: No font file found at Assets/Fonts/game_font.ttf"
    echo "The game requires a TrueType font to render text."
    echo ""
    echo "Please copy a .ttf font file to: ${SCRIPT_DIR}/Assets/Fonts/game_font.ttf"
    echo ""
    echo "On macOS you can try:"
    echo "  cp /System/Library/Fonts/Supplemental/Arial.ttf Assets/Fonts/game_font.ttf"
    echo ""
    echo "On Linux:"
    echo "  cp /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf Assets/Fonts/game_font.ttf"
    echo ""
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# ── Step 2: CMake Configure ─────────────────────────────
echo ""
echo "========================================"
echo "  Step 2: CMake Configure (${BUILD_TYPE})"
echo "========================================"
echo ""

if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    cmake -B "$BUILD_DIR" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -S "$SCRIPT_DIR"
else
    echo "CMake already configured. Skipping. (Use 'clean' to reconfigure)"
fi

# ── Step 3: Build ────────────────────────────────────────
echo ""
echo "========================================"
echo "  Step 3: Building TechDemo"
echo "========================================"
echo ""

# Detect number of CPU cores for parallel build
if command -v nproc &> /dev/null; then
    NUM_CORES=$(nproc)
elif command -v sysctl &> /dev/null; then
    NUM_CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    NUM_CORES=4
fi

cmake --build "$BUILD_DIR" --parallel "$NUM_CORES"

echo ""
echo "Build complete!"

# ── Step 4: Run ──────────────────────────────────────────
echo ""
echo "========================================"
echo "  Step 4: Running TechDemo"
echo "========================================"
echo ""

# Find the executable
EXECUTABLE=""

if [ "$BUILD_TYPE" = "Release" ] && [ "$(uname)" = "Darwin" ]; then
    # macOS release builds create an .app bundle
    APP_BUNDLE="${BUILD_DIR}/TechDemo.app/Contents/MacOS/TechDemo"
    if [ -f "$APP_BUNDLE" ]; then
        EXECUTABLE="$APP_BUNDLE"
    fi
fi

if [ -z "$EXECUTABLE" ]; then
    # Look for the executable in common locations
    if [ -f "${BUILD_DIR}/TechDemo" ]; then
        EXECUTABLE="${BUILD_DIR}/TechDemo"
    elif [ -f "${BUILD_DIR}/Debug/TechDemo" ]; then
        EXECUTABLE="${BUILD_DIR}/Debug/TechDemo"
    elif [ -f "${BUILD_DIR}/Release/TechDemo" ]; then
        EXECUTABLE="${BUILD_DIR}/Release/TechDemo"
    elif [ -f "${BUILD_DIR}/TechDemo.exe" ]; then
        EXECUTABLE="${BUILD_DIR}/TechDemo.exe"
    else
        # Search for it
        EXECUTABLE=$(find "$BUILD_DIR" -name "TechDemo" -o -name "TechDemo.exe" 2>/dev/null | head -1)
    fi
fi

if [ -z "$EXECUTABLE" ] || [ ! -f "$EXECUTABLE" ]; then
    echo "ERROR: Could not find TechDemo executable in ${BUILD_DIR}"
    echo "Build may have failed. Check the output above for errors."
    exit 1
fi

echo "Running: $EXECUTABLE"
echo "─────────────────────────────────────────"
echo ""

exec "$EXECUTABLE"
