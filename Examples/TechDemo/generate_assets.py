#!/usr/bin/env python3
"""
Generate placeholder assets for the Sapling TechDemo.
Creates simple PNG sprites and WAV audio files using only the Python standard library.
"""

import struct
import zlib
import os
import math
import random

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SPRITES_DIR = os.path.join(SCRIPT_DIR, "Assets", "Sprites")
AUDIO_DIR = os.path.join(SCRIPT_DIR, "Assets", "Audio")
FONTS_DIR = os.path.join(SCRIPT_DIR, "Assets", "Fonts")


def write_png(filename, width, height, pixels):
    """Write a minimal RGBA PNG file. pixels is a list of (r, g, b, a) tuples, row-major."""

    def make_chunk(chunk_type, data):
        chunk = chunk_type + data
        crc = struct.pack(">I", zlib.crc32(chunk) & 0xFFFFFFFF)
        return struct.pack(">I", len(data)) + chunk + crc

    # PNG signature
    sig = b"\x89PNG\r\n\x1a\n"

    # IHDR: width, height, bit depth 8, color type 6 (RGBA)
    ihdr_data = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    ihdr = make_chunk(b"IHDR", ihdr_data)

    # IDAT: raw image data with filter byte 0 (None) per row
    raw = b""
    for y in range(height):
        raw += b"\x00"  # filter byte
        for x in range(width):
            idx = y * width + x
            r, g, b, a = pixels[idx]
            raw += struct.pack("BBBB", r, g, b, a)

    compressed = zlib.compress(raw)
    idat = make_chunk(b"IDAT", compressed)

    # IEND
    iend = make_chunk(b"IEND", b"")

    filepath = os.path.join(SPRITES_DIR, filename)
    with open(filepath, "wb") as f:
        f.write(sig + ihdr + idat + iend)
    print(f"  Created {filepath}")


def write_wav(filename, samples, sample_rate=22050):
    """Write a mono 16-bit WAV file. samples is a list of floats in [-1, 1]."""
    filepath = os.path.join(AUDIO_DIR, filename)
    num_samples = len(samples)
    data_size = num_samples * 2  # 16-bit = 2 bytes per sample

    with open(filepath, "wb") as f:
        # RIFF header
        f.write(b"RIFF")
        f.write(struct.pack("<I", 36 + data_size))
        f.write(b"WAVE")

        # fmt chunk
        f.write(b"fmt ")
        f.write(struct.pack("<I", 16))          # chunk size
        f.write(struct.pack("<H", 1))           # PCM format
        f.write(struct.pack("<H", 1))           # mono
        f.write(struct.pack("<I", sample_rate))  # sample rate
        f.write(struct.pack("<I", sample_rate * 2))  # byte rate
        f.write(struct.pack("<H", 2))           # block align
        f.write(struct.pack("<H", 16))          # bits per sample

        # data chunk
        f.write(b"data")
        f.write(struct.pack("<I", data_size))
        for s in samples:
            val = max(-1.0, min(1.0, s))
            f.write(struct.pack("<h", int(val * 32767)))

    print(f"  Created {filepath}")


# ── Pixel art helper ──────────────────────────────────────

def filled_rect(w, h, color):
    """Return a list of pixels for a filled rectangle."""
    return [color] * (w * h)


def draw_pixel(pixels, w, x, y, color):
    """Set a single pixel in a pixel buffer."""
    if 0 <= x < w and 0 <= y:
        idx = y * w + x
        if idx < len(pixels):
            pixels[idx] = color


def draw_rect_outline(pixels, w, h, x0, y0, x1, y1, color):
    """Draw a rectangle outline into the pixel buffer."""
    for x in range(x0, x1 + 1):
        draw_pixel(pixels, w, x, y0, color)
        draw_pixel(pixels, w, x, y1, color)
    for y in range(y0, y1 + 1):
        draw_pixel(pixels, w, x0, y, color)
        draw_pixel(pixels, w, x1, y, color)


def draw_filled_rect(pixels, w, h, x0, y0, x1, y1, color):
    """Draw a filled rectangle into the pixel buffer."""
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            draw_pixel(pixels, w, x, y, color)


# ── Sprite generation ────────────────────────────────────

def gen_player():
    """16x16 player character - a little green dude facing down."""
    w, h = 16, 16
    transparent = (0, 0, 0, 0)
    body = (50, 180, 80, 255)       # green body
    dark = (30, 120, 50, 255)       # darker green
    eye = (255, 255, 255, 255)      # white eyes
    pupil = (20, 20, 40, 255)       # dark pupils
    skin = (240, 200, 160, 255)     # skin tone

    pixels = [transparent] * (w * h)

    # Body (torso) - rows 6-12, cols 4-11
    draw_filled_rect(pixels, w, h, 4, 6, 11, 12, body)
    # Darker sides
    draw_filled_rect(pixels, w, h, 4, 6, 4, 12, dark)
    draw_filled_rect(pixels, w, h, 11, 6, 11, 12, dark)

    # Head - rows 1-5, cols 4-11
    draw_filled_rect(pixels, w, h, 4, 1, 11, 5, body)
    draw_filled_rect(pixels, w, h, 5, 0, 10, 0, body)  # top of head

    # Eyes
    draw_filled_rect(pixels, w, h, 5, 2, 6, 3, eye)
    draw_filled_rect(pixels, w, h, 9, 2, 10, 3, eye)
    draw_pixel(pixels, w, 6, 3, pupil)
    draw_pixel(pixels, w, 9, 3, pupil)

    # Mouth
    draw_filled_rect(pixels, w, h, 6, 5, 9, 5, dark)

    # Arms
    draw_filled_rect(pixels, w, h, 2, 7, 3, 11, body)
    draw_filled_rect(pixels, w, h, 12, 7, 13, 11, body)

    # Legs
    draw_filled_rect(pixels, w, h, 5, 13, 6, 15, dark)
    draw_filled_rect(pixels, w, h, 9, 13, 10, 15, dark)

    write_png("player.png", w, h, pixels)


def gen_player_walk():
    """64x16 player walk animation - 4 frames showing leg movement."""
    frame_w, frame_h = 16, 16
    total_w = frame_w * 4
    transparent = (0, 0, 0, 0)
    body = (50, 180, 80, 255)
    dark = (30, 120, 50, 255)
    eye = (255, 255, 255, 255)
    pupil = (20, 20, 40, 255)

    pixels = [transparent] * (total_w * frame_h)

    # Leg positions for each frame (left_x_offset, right_x_offset)
    leg_offsets = [
        ((-1, 0), (1, 0)),   # frame 0: legs spread
        ((0, 0), (0, 0)),    # frame 1: legs together
        ((1, 0), (-1, 0)),   # frame 2: legs spread other way
        ((0, 0), (0, 0)),    # frame 3: legs together
    ]

    for frame in range(4):
        ox = frame * frame_w  # x offset for this frame

        def dp(x, y, color):
            draw_pixel(pixels, total_w, ox + x, y, color)

        def dfr(x0, y0, x1, y1, color):
            for yy in range(y0, y1 + 1):
                for xx in range(x0, x1 + 1):
                    dp(xx, yy, color)

        # Body
        dfr(4, 6, 11, 12, body)
        dfr(4, 6, 4, 12, dark)
        dfr(11, 6, 11, 12, dark)

        # Head
        dfr(4, 1, 11, 5, body)
        dfr(5, 0, 10, 0, body)
        dfr(5, 2, 6, 3, eye)
        dfr(9, 2, 10, 3, eye)
        dp(6, 3, pupil)
        dp(9, 3, pupil)
        dfr(6, 5, 9, 5, dark)

        # Arms with swing
        arm_swing = [0, 1, 0, -1]
        ay = arm_swing[frame]
        dfr(2, 7 + ay, 3, 11 + ay, body)
        dfr(12, 7 - ay, 13, 11 - ay, body)

        # Legs with walk animation
        lx, ly = leg_offsets[frame][0]
        rx, ry = leg_offsets[frame][1]
        dfr(5 + lx, 13, 6 + lx, 15, dark)
        dfr(9 + rx, 13, 10 + rx, 15, dark)

    write_png("player_walk.png", total_w, frame_h, pixels)


def gen_gem():
    """12x12 gem / collectible - yellow diamond shape."""
    w, h = 12, 12
    transparent = (0, 0, 0, 0)
    gem_bright = (255, 220, 50, 255)
    gem_mid = (230, 180, 30, 255)
    gem_dark = (180, 140, 20, 255)
    gem_shine = (255, 255, 200, 255)

    pixels = [transparent] * (w * h)

    # Diamond shape
    center_x, center_y = 5, 5
    for y in range(h):
        for x in range(w):
            dx = abs(x - center_x)
            dy = abs(y - center_y)
            if dx + dy <= 5:
                if dx + dy <= 3:
                    pixels[y * w + x] = gem_bright
                elif dx + dy <= 4:
                    pixels[y * w + x] = gem_mid
                else:
                    pixels[y * w + x] = gem_dark

    # Shine highlight
    draw_pixel(pixels, w, 4, 3, gem_shine)
    draw_pixel(pixels, w, 3, 4, gem_shine)
    draw_pixel(pixels, w, 5, 2, gem_shine)

    write_png("gem.png", w, h, pixels)


def gen_gem_spin():
    """48x12 gem spin animation - 4 frames."""
    frame_w, frame_h = 12, 12
    total_w = frame_w * 4
    transparent = (0, 0, 0, 0)
    gem_bright = (255, 220, 50, 255)
    gem_mid = (230, 180, 30, 255)
    gem_dark = (180, 140, 20, 255)
    gem_shine = (255, 255, 200, 255)

    pixels = [transparent] * (total_w * frame_h)

    # Each frame has the diamond at different "squeeze" to simulate spin
    squeeze_factors = [1.0, 0.6, 0.3, 0.6]

    for frame in range(4):
        ox = frame * frame_w
        squeeze = squeeze_factors[frame]
        center_x, center_y = 5, 5

        for y in range(frame_h):
            for x in range(frame_w):
                dx = abs(x - center_x) / max(squeeze, 0.1)
                dy = abs(y - center_y)
                dist = dx + dy
                if dist <= 5:
                    if dist <= 3:
                        color = gem_bright
                    elif dist <= 4:
                        color = gem_mid
                    else:
                        color = gem_dark
                    draw_pixel(pixels, total_w, ox + x, y, color)

        # Shine on non-squeezed frames
        if squeeze > 0.5:
            draw_pixel(pixels, total_w, ox + 4, 3, gem_shine)
            draw_pixel(pixels, total_w, ox + 3, 4, gem_shine)

    write_png("gem_spin.png", total_w, frame_h, pixels)


def gen_wall():
    """16x16 wall tile - stone/brick pattern."""
    w, h = 16, 16
    base = (100, 100, 110, 255)
    light = (120, 120, 130, 255)
    dark = (70, 70, 80, 255)
    mortar = (60, 60, 65, 255)

    pixels = [base] * (w * h)

    # Brick pattern
    # Row 1 of bricks (rows 0-6)
    draw_filled_rect(pixels, w, h, 0, 0, 15, 0, mortar)
    draw_filled_rect(pixels, w, h, 0, 7, 15, 7, mortar)
    draw_filled_rect(pixels, w, h, 7, 0, 7, 7, mortar)

    # Row 2 of bricks (rows 8-14)
    draw_filled_rect(pixels, w, h, 0, 8, 15, 8, mortar)
    draw_filled_rect(pixels, w, h, 0, 15, 15, 15, mortar)
    draw_filled_rect(pixels, w, h, 3, 8, 3, 15, mortar)
    draw_filled_rect(pixels, w, h, 11, 8, 11, 15, mortar)

    # Pale edges on bricks
    draw_filled_rect(pixels, w, h, 1, 1, 6, 1, light)
    draw_filled_rect(pixels, w, h, 9, 1, 15, 1, light)
    draw_filled_rect(pixels, w, h, 1, 9, 2, 9, light)
    draw_filled_rect(pixels, w, h, 5, 9, 10, 9, light)
    draw_filled_rect(pixels, w, h, 13, 9, 15, 9, light)

    # Dark edges on bricks
    draw_filled_rect(pixels, w, h, 1, 6, 6, 6, dark)
    draw_filled_rect(pixels, w, h, 9, 6, 15, 6, dark)
    draw_filled_rect(pixels, w, h, 1, 14, 2, 14, dark)
    draw_filled_rect(pixels, w, h, 5, 14, 10, 14, dark)
    draw_filled_rect(pixels, w, h, 13, 14, 15, 14, dark)

    write_png("wall.png", w, h, pixels)


def gen_floor():
    """16x16 floor tile - subtle checker pattern."""
    w, h = 16, 16
    light = (55, 60, 70, 255)
    dark = (45, 50, 60, 255)
    dot = (50, 55, 65, 255)

    pixels = []
    for y in range(h):
        for x in range(w):
            # Checker of 8x8 blocks
            if ((x // 8) + (y // 8)) % 2 == 0:
                pixels.append(light)
            else:
                pixels.append(dark)

    # Add a few subtle dots for texture
    for pos in [(3, 3), (11, 5), (7, 11), (13, 13), (1, 9)]:
        draw_pixel(pixels, w, pos[0], pos[1], dot)

    write_png("floor.png", w, h, pixels)


def gen_enemy():
    """16x16 enemy sprite - red slime/blob."""
    w, h = 16, 16
    transparent = (0, 0, 0, 0)
    body = (200, 50, 50, 255)
    dark = (150, 30, 30, 255)
    light = (230, 80, 80, 255)
    eye = (255, 255, 255, 255)
    pupil = (40, 10, 10, 255)

    pixels = [transparent] * (w * h)

    # Blob body - round shape
    for y in range(h):
        for x in range(w):
            dx = x - 7.5
            dy = y - 9.0
            # Elliptical shape, wider at bottom
            ry = 6.5
            rx = 6.0 + (y - 5) * 0.15 if y > 5 else 5.0
            if (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) <= 1.0:
                if (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) <= 0.5:
                    pixels[y * w + x] = light
                elif (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) <= 0.8:
                    pixels[y * w + x] = body
                else:
                    pixels[y * w + x] = dark

    # Eyes
    draw_filled_rect(pixels, w, h, 4, 6, 6, 8, eye)
    draw_filled_rect(pixels, w, h, 9, 6, 11, 8, eye)
    draw_pixel(pixels, w, 5, 7, pupil)
    draw_pixel(pixels, w, 6, 7, pupil)
    draw_pixel(pixels, w, 10, 7, pupil)
    draw_pixel(pixels, w, 11, 7, pupil)

    write_png("enemy.png", w, h, pixels)


def gen_enemy_walk():
    """64x16 enemy walk animation - 4 frames of blob bouncing."""
    frame_w, frame_h = 16, 16
    total_w = frame_w * 4
    transparent = (0, 0, 0, 0)
    body = (200, 50, 50, 255)
    dark = (150, 30, 30, 255)
    light = (230, 80, 80, 255)
    eye = (255, 255, 255, 255)
    pupil = (40, 10, 10, 255)

    pixels = [transparent] * (total_w * frame_h)

    # Squash/stretch for each frame
    configs = [
        (7.5, 9.0, 6.0, 6.5),   # normal
        (7.5, 10.0, 6.5, 5.5),  # squashed
        (7.5, 8.5, 5.5, 7.0),   # stretched up
        (7.5, 10.0, 6.5, 5.5),  # squashed
    ]

    for frame in range(4):
        ox = frame * frame_w
        cx, cy, base_rx, ry = configs[frame]

        for y in range(frame_h):
            for x in range(frame_w):
                dx = x - cx
                dy = y - cy
                rx = base_rx + (y - 5) * 0.12 if y > 5 else base_rx * 0.85
                if rx <= 0:
                    continue
                dist = (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry)
                if dist <= 1.0:
                    if dist <= 0.5:
                        color = light
                    elif dist <= 0.8:
                        color = body
                    else:
                        color = dark
                    draw_pixel(pixels, total_w, ox + x, y, color)

        # Eyes (adjusted for vertical position)
        ey = int(cy) - 3
        if ey < 1:
            ey = 1
        draw_filled_rect(pixels, total_w, frame_h, ox + 4, ey, ox + 6, ey + 2, eye)
        draw_filled_rect(pixels, total_w, frame_h, ox + 9, ey, ox + 11, ey + 2, eye)
        draw_pixel(pixels, total_w, ox + 5, ey + 1, pupil)
        draw_pixel(pixels, total_w, ox + 6, ey + 1, pupil)
        draw_pixel(pixels, total_w, ox + 10, ey + 1, pupil)
        draw_pixel(pixels, total_w, ox + 11, ey + 1, pupil)

    write_png("enemy_walk.png", total_w, frame_h, pixels)


def gen_heart():
    """12x12 heart for lives/health display."""
    w, h = 12, 12
    transparent = (0, 0, 0, 0)
    red = (220, 40, 40, 255)
    dark_red = (160, 20, 20, 255)
    bright = (255, 100, 100, 255)

    pixels = [transparent] * (w * h)

    # Heart shape using distance from two circles + triangle
    for y in range(h):
        for x in range(w):
            # Two circles at top
            d1 = math.sqrt((x - 3) ** 2 + (y - 3) ** 2)
            d2 = math.sqrt((x - 8) ** 2 + (y - 3) ** 2)
            # Triangle/point at bottom
            in_triangle = (y >= 3 and y <= 11 and
                           x >= (y - 3) * -6 / 8 + 6 and
                           x <= (y - 3) * 6 / 8 + 5)
            in_circle = d1 <= 3.5 or d2 <= 3.5

            if in_circle or in_triangle:
                if d1 <= 2 or (x < 6 and y < 5):
                    pixels[y * w + x] = bright
                elif d2 <= 2.5 or y > 7:
                    pixels[y * w + x] = dark_red
                else:
                    pixels[y * w + x] = red

    write_png("heart.png", w, h, pixels)


def gen_particle():
    """4x4 simple particle sprite."""
    w, h = 4, 4
    transparent = (0, 0, 0, 0)
    white = (255, 255, 255, 255)
    half = (255, 255, 255, 180)

    pixels = [transparent] * (w * h)
    # Cross pattern
    draw_pixel(pixels, w, 1, 0, half)
    draw_pixel(pixels, w, 2, 0, half)
    draw_pixel(pixels, w, 0, 1, half)
    draw_pixel(pixels, w, 1, 1, white)
    draw_pixel(pixels, w, 2, 1, white)
    draw_pixel(pixels, w, 3, 1, half)
    draw_pixel(pixels, w, 0, 2, half)
    draw_pixel(pixels, w, 1, 2, white)
    draw_pixel(pixels, w, 2, 2, white)
    draw_pixel(pixels, w, 3, 2, half)
    draw_pixel(pixels, w, 1, 3, half)
    draw_pixel(pixels, w, 2, 3, half)

    write_png("particle.png", w, h, pixels)


def gen_debug():
    """8x8 debug circle for bounding box visualization."""
    w, h = 8, 8
    transparent = (0, 0, 0, 0)
    red = (255, 0, 0, 255)

    pixels = [transparent] * (w * h)
    for y in range(h):
        for x in range(w):
            dx = x - 3.5
            dy = y - 3.5
            if dx * dx + dy * dy <= 12:
                pixels[y * w + x] = red

    write_png("debug.png", w, h, pixels)


# ── Audio generation ──────────────────────────────────────

def gen_collect_sound():
    """Short ascending chime for item collection."""
    sr = 22050
    duration = 0.3
    samples = []
    num = int(sr * duration)

    for i in range(num):
        t = i / sr
        # Rising frequency from 600 to 1200 Hz
        freq = 600 + (t / duration) * 600
        # Envelope: quick attack, quick decay
        env = max(0, 1.0 - t / duration) * min(1.0, t * 20)
        val = math.sin(2 * math.pi * freq * t) * env * 0.6
        # Add a harmonic
        val += math.sin(2 * math.pi * freq * 2 * t) * env * 0.2
        samples.append(val)

    write_wav("collect.wav", samples, sr)


def gen_step_sound():
    """Short footstep sound."""
    sr = 22050
    duration = 0.08
    samples = []
    num = int(sr * duration)

    random.seed(42)
    for i in range(num):
        t = i / sr
        env = max(0, 1.0 - t / duration)
        # Low noise burst
        noise = (random.random() * 2 - 1)
        freq = 120
        tone = math.sin(2 * math.pi * freq * t)
        val = (noise * 0.5 + tone * 0.5) * env * 0.4
        samples.append(val)

    write_wav("step.wav", samples, sr)


def gen_hurt_sound():
    """Short descending buzz for taking damage."""
    sr = 22050
    duration = 0.25
    samples = []
    num = int(sr * duration)

    for i in range(num):
        t = i / sr
        freq = 300 - (t / duration) * 150
        env = max(0, 1.0 - t / duration)
        val = math.sin(2 * math.pi * freq * t) * env * 0.5
        # Add buzz
        val += math.sin(2 * math.pi * freq * 3 * t) * env * 0.15
        samples.append(val)

    write_wav("hurt.wav", samples, sr)


def gen_bgm():
    """Simple looping background melody (~4 seconds)."""
    sr = 22050
    duration = 4.0
    samples = []
    num = int(sr * duration)

    # Simple melody notes (frequency, start_beat, duration_beats)
    bpm = 140
    beat_len = 60.0 / bpm

    # A simple 8-note melody pattern
    melody_notes = [
        (330, 0), (392, 1), (440, 2), (392, 3),
        (330, 4), (294, 5), (330, 6), (262, 7),
    ]

    # Bass notes
    bass_notes = [
        (131, 0), (131, 2), (165, 4), (131, 6),
    ]

    for i in range(num):
        t = i / sr
        val = 0.0

        # Melody
        for freq, beat in melody_notes:
            note_start = beat * beat_len
            note_dur = beat_len * 0.9
            if note_start <= t < note_start + note_dur:
                nt = t - note_start
                env = min(1.0, nt * 30) * max(0, 1.0 - nt / note_dur)
                val += math.sin(2 * math.pi * freq * nt) * env * 0.25
                val += math.sin(2 * math.pi * freq * 2 * nt) * env * 0.08
                break

        # Bass
        for freq, beat in bass_notes:
            note_start = beat * beat_len
            note_dur = beat_len * 1.8
            if note_start <= t < note_start + note_dur:
                nt = t - note_start
                env = min(1.0, nt * 20) * max(0, 1.0 - nt / note_dur) * 0.7
                val += math.sin(2 * math.pi * freq * nt) * env * 0.2
                break

        samples.append(val)

    write_wav("bgm.wav", samples, sr)


def gen_scene_change_sound():
    """Swoosh/transition sound."""
    sr = 22050
    duration = 0.4
    samples = []
    num = int(sr * duration)

    random.seed(99)
    for i in range(num):
        t = i / sr
        progress = t / duration
        # Sweeping noise
        freq = 200 + progress * 800
        env = math.sin(progress * math.pi) * 0.5
        val = math.sin(2 * math.pi * freq * t) * env * 0.4
        val += (random.random() * 2 - 1) * env * 0.15
        samples.append(val)

    write_wav("scene_change.wav", samples, sr)


def gen_score_jingle():
    """Short victory jingle for score screen."""
    sr = 22050
    duration = 1.0
    samples = []
    num = int(sr * duration)

    # Ascending arpeggio
    notes = [262, 330, 392, 523]  # C4 E4 G4 C5
    note_dur = 0.2

    for i in range(num):
        t = i / sr
        val = 0.0

        for idx, freq in enumerate(notes):
            ns = idx * note_dur
            nd = note_dur * 1.5
            if ns <= t < ns + nd:
                nt = t - ns
                env = min(1.0, nt * 40) * max(0, 1.0 - nt / nd)
                val += math.sin(2 * math.pi * freq * nt) * env * 0.3
                val += math.sin(2 * math.pi * freq * 2 * nt) * env * 0.1

        # Final chord sustain
        if t >= len(notes) * note_dur:
            nt = t - len(notes) * note_dur
            env = max(0, 1.0 - nt / (duration - len(notes) * note_dur))
            for freq in notes:
                val += math.sin(2 * math.pi * freq * nt) * env * 0.15

        samples.append(val)

    write_wav("score_jingle.wav", samples, sr)


# ── Font handling ─────────────────────────────────────────

def find_system_font():
    """Try to find a usable TTF font on the system."""
    candidates = [
        # macOS
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Supplemental/Verdana.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
        # Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        # Windows
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\verdana.ttf",
    ]

    for path in candidates:
        if os.path.exists(path):
            return path

    return None


def setup_font():
    """Copy a system font for use by the game."""
    font_dest = os.path.join(FONTS_DIR, "game_font.ttf")
    if os.path.exists(font_dest):
        print(f"  Font already exists: {font_dest}")
        return True

    font_src = find_system_font()
    if font_src:
        import shutil
        try:
            shutil.copy(font_src, font_dest)
        except (PermissionError, OSError):
            # Fallback: read and write the bytes manually to avoid copystat issues
            with open(font_src, "rb") as f_in:
                data = f_in.read()
            with open(font_dest, "wb") as f_out:
                f_out.write(data)
        print(f"  Copied font from {font_src} to {font_dest}")
        return True
    else:
        print("  WARNING: No system font found. Please place a .ttf font at:")
        print(f"  {font_dest}")
        return False


# ── Main ──────────────────────────────────────────────────

def main():
    os.makedirs(SPRITES_DIR, exist_ok=True)
    os.makedirs(AUDIO_DIR, exist_ok=True)
    os.makedirs(FONTS_DIR, exist_ok=True)

    print("Generating sprites...")
    gen_player()
    gen_player_walk()
    gen_gem()
    gen_gem_spin()
    gen_wall()
    gen_floor()
    gen_enemy()
    gen_enemy_walk()
    gen_heart()
    gen_particle()
    gen_debug()

    print("Generating audio...")
    gen_collect_sound()
    gen_step_sound()
    gen_hurt_sound()
    gen_bgm()
    gen_scene_change_sound()
    gen_score_jingle()

    print("Setting up font...")
    setup_font()

    print("\nAsset generation complete!")


if __name__ == "__main__":
    main()
