"""
generate_crnn_ctc_dataset.py

Synthetic dataset generator for CRNN/CTC OCR.

Output format:
    dataset_crnn_ctc_scroll_192/
      train/
        img_000000.jpg
        labels.csv
      valid/
        labels.csv
      test/
        labels.csv

Each labels.csv row is:
    filename,text

The label is ONLY the text that is fully visible on the monitor screen in that
single frame. This is important for CTC training.

Install:
    pip install pillow opencv-python numpy

Run:
    python generate_crnn_ctc_dataset.py
"""

import csv
import os
import random
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont


# ============================================================
# CONFIG
# ============================================================

OUTPUT_DIR = Path("CRNN_CTC_Scrolling_Text_192")

NUM_IMAGES = 100
FINAL_W, FINAL_H = 192, 192

SPLIT_RATIOS = {
    "train": 0.70,
    "valid": 0.20,
    "test": 0.10,
}

# Camera sees full scene. No external crop is assumed.
# Monitor itself is drawn somewhere inside the image.
MONITOR_ASPECT = 16 / 9
MONITOR_W_RANGE = (150, 186)
MONITOR_CENTER_JITTER = 8
MONITOR_ROTATION_DEG = (-3.0, 3.0)

# Text settings.
CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 "
# Random alphanumeric text is generated so all characters appear often.
ALNUM_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
BALANCED_CHAR_POOL: list[str] = []
# CTC training labels should not include empty strings. Empty frames are skipped.
ALLOW_EMPTY_LABELS = False
TEXT_VERTICAL_JITTER = 2
CHAR_VISIBLE_TOL = 1
TEXT_MOTION_BLUR_PROB = 0.35

# Visual effects. Start simple. Increase later after training works.
BACKGROUND_NOISE_PROB = 0.70
CAMERA_BLUR_PROB = 0.35
JPEG_PROB = 0.70
ROTATE_SCENE = True

# Put your actual lab/display font first if known.
FONT_PATHS = [
    r"C:\Windows\Fonts\OCRAEXT.ttf",
    r"C:\Windows\Fonts\consola.ttf",
    r"C:\Windows\Fonts\arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation2/LiberationMono-Bold.ttf",
]

# Phrases are used as templates. Random phrases are also generated.
# BASE_PHRASES = [
#     "HELLO WORLD.",
#     "ECE-3073=OK.",
#     "AI VISION MODULE.",
#     "FPGA UART READY.",
#     "NIOS RECEIVED DATA.",
#     "TEXT RECOGNITION TEST.",
#     "SCROLLING TEXT SPEED 1.",
#     "SCROLLING TEXT SPEED 2.",
#     "TEMP=25.4 C.",
#     "STATUS=OK.",
#     "NO DEFECT.",
#     "SYSTEM READY.",
#     "EDGE AI OCR DEMO.",
#     "GROVE VISION AI V2.",
#     "DE10 LITE FPGA BOARD.",
#     "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.",
# ]

# WORDS = [
#     "THE", "SYSTEM", "IS", "READY", "FPGA", "RECEIVED", "UART", "DATA",
#     "GROVE", "VISION", "MODULE", "TEXT", "SCROLLING", "SPEED", "DEMO",
#     "AI", "MODEL", "OCR", "EDGE", "CAMERA", "MONITOR", "SCREEN", "NIOS",
#     "CONTROL", "SIGNAL", "BUFFER", "DISPLAY", "LAB", "TEST", "RESULT",
#     "VALID", "ERROR", "STATUS", "TEMP", "VALUE", "COUNT", "CLOCK", "FRAME",
# ]

FONT_CACHE: dict[int, ImageFont.FreeTypeFont] = {}
MEASURE_IMG = Image.new("RGB", (8, 8), (0, 0, 0))
MEASURE_DRAW = ImageDraw.Draw(MEASURE_IMG)


# ============================================================
# BASIC HELPERS
# ============================================================

def make_dirs() -> None:
    for split in SPLIT_RATIOS:
        (OUTPUT_DIR / split).mkdir(parents=True, exist_ok=True)


def choose_split() -> str:
    r = random.random()
    acc = 0.0
    for split, ratio in SPLIT_RATIOS.items():
        acc += ratio
        if r <= acc:
            return split
    return "test"


def load_font(size: int) -> ImageFont.FreeTypeFont:
    size = int(size)
    if size in FONT_CACHE:
        return FONT_CACHE[size]

    for path in FONT_PATHS:
        if os.path.exists(path):
            font = ImageFont.truetype(path, size)
            FONT_CACHE[size] = font
            return font

    raise FileNotFoundError(
        "No usable font found. Edit FONT_PATHS to point to a .ttf font on your machine."
    )


def text_size(text: str, font: ImageFont.FreeTypeFont) -> tuple[int, int]:
    bbox = MEASURE_DRAW.textbbox((0, 0), text, font=font)
    return bbox[2] - bbox[0], bbox[3] - bbox[1]


def fit_font(text: str, max_h: int, min_size: int = 10, max_size: int = 42) -> ImageFont.FreeTypeFont:
    # Width is intentionally not constrained because the text scrolls horizontally.
    for size in range(max_size, min_size - 1, -1):
        font = load_font(size)
        _, h = text_size(text, font)
        if h <= max_h:
            return font
    return load_font(min_size)


def refill_balanced_pool() -> None:
    global BALANCED_CHAR_POOL

    repeats = 4
    BALANCED_CHAR_POOL = list(ALNUM_CHARS * repeats)
    random.shuffle(BALANCED_CHAR_POOL)


def next_balanced_char() -> str:
    if not BALANCED_CHAR_POOL:
        refill_balanced_pool()

    return BALANCED_CHAR_POOL.pop()


def generate_balanced_word(min_len: int = 2, max_len: int = 12) -> str:
    length = random.randint(min_len, max_len)

    return "".join(next_balanced_char() for _ in range(length))


def random_phrase() -> str:
    # Generates random uppercase alphanumeric "words".
    # Spaces separate words. No punctuation.
    num_words = random.randint(3, 14)

    words = [
        generate_balanced_word()
        for _ in range(num_words)
    ]

    return " ".join(words)

# def random_phrase() -> str:
#     # Mix fixed realistic phrases with random sentence-like strings.
#     if random.random() < 0.45:
#         return random.choice(BASE_PHRASES)

#     num_words = random.randint(3, 14)
#     parts: list[str] = []
#     for _ in range(num_words):
#         if random.random() < 0.18:
#             parts.append(str(random.randint(0, 999)))
#         elif random.random() < 0.12:
#             parts.append(f"TEMP={random.randint(18, 35)}.{random.randint(0, 9)}")
#         elif random.random() < 0.10:
#             parts.append(f"CH{random.randint(0, 9)}={random.randint(0, 255)}")
#         else:
#             parts.append(random.choice(WORDS))

#     text = " ".join(parts)
#     if random.random() < 0.75:
#         text += "."
#     return text


# ============================================================
# IMAGE EFFECTS
# ============================================================

def random_background() -> np.ndarray:
    base = np.full((FINAL_H, FINAL_W, 3), random.randint(35, 115), dtype=np.uint8)

    # Gentle gradient.
    if random.random() < 0.7:
        x = np.linspace(random.randint(-20, 20), random.randint(-10, 35), FINAL_W)
        y = np.linspace(random.randint(-20, 20), random.randint(-10, 35), FINAL_H)
        grad = x[None, :] + y[:, None]
        base = np.clip(base.astype(np.float32) + grad[:, :, None], 0, 255).astype(np.uint8)

    # Lab-ish random rectangles in the background.
    if random.random() < 0.45:
        for _ in range(random.randint(2, 8)):
            x1 = random.randint(0, FINAL_W - 20)
            y1 = random.randint(0, FINAL_H - 20)
            x2 = random.randint(x1 + 8, min(FINAL_W, x1 + 60))
            y2 = random.randint(y1 + 8, min(FINAL_H, y1 + 50))
            color = random.randint(25, 140)
            cv2.rectangle(base, (x1, y1), (x2, y2), (color, color, color), -1)

    if random.random() < BACKGROUND_NOISE_PROB:
        noise = np.random.normal(0, random.uniform(2, 8), base.shape)
        base = np.clip(base.astype(np.float32) + noise, 0, 255).astype(np.uint8)

    return base


def apply_screen_effects(arr: np.ndarray, screen_rect: tuple[int, int, int, int]) -> np.ndarray:
    x1, y1, x2, y2 = screen_rect
    screen = arr[y1:y2, x1:x2].astype(np.float32)
    h, w = screen.shape[:2]
    if h <= 2 or w <= 2:
        return arr

    yy, xx = np.indices((h, w))

    # Faint scanlines.
    if random.random() < 0.70:
        period = random.uniform(2.0, 4.5)
        strength = random.uniform(0.015, 0.045)
        scan = 1.0 - strength * (0.5 + 0.5 * np.sin(2 * np.pi * yy / period))
        screen *= scan[:, :, None]

    # Mild glare/unevenness.
    if random.random() < 0.45:
        gx = random.uniform(-0.3 * w, 1.3 * w)
        gy = random.uniform(-0.2 * h, 1.0 * h)
        rx = random.uniform(0.20 * w, 0.65 * w)
        ry = random.uniform(0.06 * h, 0.25 * h)
        blob = np.exp(-(((xx - gx) ** 2) / (2 * rx * rx) + ((yy - gy) ** 2) / (2 * ry * ry)))
        screen += blob[:, :, None] * random.uniform(4, 22)

    arr[y1:y2, x1:x2] = np.clip(screen, 0, 255).astype(np.uint8)
    return arr


def apply_camera_effects(img: np.ndarray) -> np.ndarray:
    out = img.astype(np.float32)

    # Auto exposure / white balance.
    out = out * random.uniform(0.82, 1.22) + random.uniform(-10, 12)
    wb = np.array([
        random.uniform(0.93, 1.08),
        random.uniform(0.95, 1.05),
        random.uniform(0.93, 1.08),
    ], dtype=np.float32)
    out *= wb
    out = np.clip(out, 0, 255).astype(np.uint8)

    # Bloom around bright text.
    if random.random() < 0.80:
        gray = cv2.cvtColor(out, cv2.COLOR_RGB2GRAY)
        mask = (gray > random.randint(185, 225)).astype(np.uint8) * 255
        glow = cv2.GaussianBlur(mask, (0, 0), sigmaX=random.uniform(0.8, 2.2)).astype(np.float32) / 255.0
        out = np.clip(out.astype(np.float32) + glow[:, :, None] * random.uniform(8, 35), 0, 255).astype(np.uint8)

    if random.random() < CAMERA_BLUR_PROB:
        sigma = random.uniform(0.25, 0.90)
        out = cv2.GaussianBlur(out, (0, 0), sigmaX=sigma, sigmaY=sigma)

    if random.random() < 0.65:
        noise = np.random.normal(0, random.uniform(1.5, 5.0), out.shape)
        out = np.clip(out.astype(np.float32) + noise, 0, 255).astype(np.uint8)

    if random.random() < JPEG_PROB:
        bgr = cv2.cvtColor(out, cv2.COLOR_RGB2BGR)
        ok, enc = cv2.imencode(".jpg", bgr, [int(cv2.IMWRITE_JPEG_QUALITY), random.randint(62, 94)])
        if ok:
            bgr = cv2.imdecode(enc, cv2.IMREAD_COLOR)
            out = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)

    return out


# ============================================================
# RENDERING
# ============================================================

def render_monitor_scene(text: str) -> tuple[np.ndarray, str]:
    """Return RGB image and visible text label for one frame."""
    bg = random_background()
    img = Image.fromarray(bg, mode="RGB").convert("RGBA")

    # Draw monitor on its own transparent layer so we can rotate it.
    layer = Image.new("RGBA", (FINAL_W, FINAL_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    mon_w = random.randint(*MONITOR_W_RANGE)
    mon_h = int(round(mon_w / MONITOR_ASPECT))
    cx = FINAL_W // 2 + random.randint(-MONITOR_CENTER_JITTER, MONITOR_CENTER_JITTER)
    cy = FINAL_H // 2 + random.randint(-MONITOR_CENTER_JITTER, MONITOR_CENTER_JITTER)

    x1 = int(cx - mon_w / 2)
    y1 = int(cy - mon_h / 2)
    x2 = x1 + mon_w
    y2 = y1 + mon_h

    bezel = random.randint(8, 35)
    draw.rounded_rectangle((x1, y1, x2, y2), radius=max(4, mon_h // 22), fill=(bezel, bezel, bezel, 255))

    pad = max(5, int(mon_w * random.uniform(0.045, 0.065)))
    sx1 = x1 + pad
    sy1 = y1 + pad
    sx2 = x2 - pad
    sy2 = y2 - pad

    draw.rectangle((sx1, sy1, sx2, sy2), fill=(0, 0, 0, 255))

    usable_h = max(12, sy2 - sy1 - 6)
    font = fit_font(text, max_h=int(usable_h * 0.78), min_size=10, max_size=34)

    # Compute scrolling x-position. Text moves from off-screen right to off-screen left.
    full_bbox = draw.textbbox((0, 0), text, font=font)
    text_w = full_bbox[2] - full_bbox[0]
    screen_w = sx2 - sx1
    progress = random.uniform(0.0, 1.0)
    tx = int(sx2 - progress * (text_w + screen_w))

    # Vertically centered text.
    text_h = full_bbox[3] - full_bbox[1]
    ty = int((sy1 + sy2) / 2 - text_h / 2 - full_bbox[1] + random.randint(-TEXT_VERTICAL_JITTER, TEXT_VERTICAL_JITTER))

    text_layer = Image.new("RGBA", (FINAL_W, FINAL_H), (0, 0, 0, 0))
    td = ImageDraw.Draw(text_layer)

    cursor_x = tx
    visible_chars: list[str] = []

    for ch in text:
        if ch not in CHARSET:
            # Skip unsupported chars but still advance approximately.
            ch = " "

        bbox = td.textbbox((cursor_x, ty), ch, font=font)
        ch_w = bbox[2] - bbox[0]
        if ch == " ":
            ch_w = max(ch_w, int(font.size * 0.45))
            bbox = (cursor_x, ty, cursor_x + ch_w, ty + text_h)
        else:
            td.text((cursor_x, ty), ch, font=font, fill=(255, 255, 255, 255))

        fully_visible = (
            bbox[0] >= sx1 - CHAR_VISIBLE_TOL
            and bbox[1] >= sy1 - CHAR_VISIBLE_TOL
            and bbox[2] <= sx2 + CHAR_VISIBLE_TOL
            and bbox[3] <= sy2 + CHAR_VISIBLE_TOL
        )

        if fully_visible:
            visible_chars.append(ch)

        cursor_x += ch_w

    # Motion blur the text itself, mostly horizontal like scrolling display capture.
    if random.random() < TEXT_MOTION_BLUR_PROB:
        k = random.choice([3, 5])
        kernel = np.zeros((k, k), dtype=np.float32)
        kernel[k // 2, :] = 1.0
        kernel /= kernel.sum()
        text_np = cv2.filter2D(np.array(text_layer), -1, kernel)
        text_layer = Image.fromarray(text_np, mode="RGBA")

    # Clip text to the monitor screen rectangle.
    text_np = np.array(text_layer)
    mask = np.zeros((FINAL_H, FINAL_W), dtype=np.uint8)
    mask[max(0, sy1):min(FINAL_H, sy2), max(0, sx1):min(FINAL_W, sx2)] = 255
    text_np[:, :, 3] = (text_np[:, :, 3].astype(np.uint16) * mask // 255).astype(np.uint8)
    text_layer = Image.fromarray(text_np, mode="RGBA")

    layer.alpha_composite(text_layer)

    # Apply screen effects after text is on the screen.
    layer_rgb = np.array(layer.convert("RGB"))
    layer_alpha = np.array(layer)[:, :, 3]
    layer_rgb = apply_screen_effects(layer_rgb, (max(0, sx1), max(0, sy1), min(FINAL_W, sx2), min(FINAL_H, sy2)))
    layer = Image.fromarray(np.dstack([layer_rgb, layer_alpha]).astype(np.uint8), mode="RGBA")

    if ROTATE_SCENE:
        angle = random.uniform(*MONITOR_ROTATION_DEG)
        layer = layer.rotate(angle, resample=Image.Resampling.BICUBIC, center=(cx, cy))

    img.alpha_composite(layer)
    rgb = np.array(img.convert("RGB"))
    rgb = apply_camera_effects(rgb)

    visible_text = "".join(visible_chars).strip()
    # Collapse repeated whitespace for cleaner CTC labels.
    visible_text = " ".join(visible_text.split())

    return rgb, visible_text


# ============================================================
# MAIN GENERATION
# ============================================================

def save_labels(labels: dict[str, list[tuple[str, str]]]) -> None:
    for split, rows in labels.items():
        path = OUTPUT_DIR / split / "labels.csv"
        with open(path, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["filename", "text"])
            writer.writerows(rows)
        print(f"Saved {path} with {len(rows)} rows")


def main() -> None:
    random.seed()
    make_dirs()

    labels: dict[str, list[tuple[str, str]]] = {split: [] for split in SPLIT_RATIOS}

    generated = 0
    attempts = 0
    max_attempts = NUM_IMAGES * 10

    print(f"Generating {NUM_IMAGES} CRNN/CTC OCR images...")
    while generated < NUM_IMAGES and attempts < max_attempts:
        attempts += 1
        phrase = random_phrase()
        image_rgb, visible_text = render_monitor_scene(phrase)

        if not visible_text and not ALLOW_EMPTY_LABELS:
            continue

        split = choose_split()
        filename = f"img_{generated:06d}.jpg"
        out_path = OUTPUT_DIR / split / filename

        # PIL/OpenCV disagreement tax: OpenCV writes BGR.
        image_bgr = cv2.cvtColor(image_rgb, cv2.COLOR_RGB2BGR)
        cv2.imwrite(str(out_path), image_bgr)
        labels[split].append((filename, visible_text))

        generated += 1
        if generated % 250 == 0:
            print(f"Generated {generated}/{NUM_IMAGES}")

    save_labels(labels)

    print("\nDone.")
    print(f"Dataset saved to: {OUTPUT_DIR.resolve()}")
    print(f"Generated images: {generated}")
    if generated < NUM_IMAGES:
        print("Warning: stopped early because too many samples had empty visible text.")


if __name__ == "__main__":
    main()

