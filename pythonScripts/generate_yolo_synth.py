import os
import json
import random
import math
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# ============================================================
# CONFIG
# ============================================================
SCREEN_TYPES = ["lcd", "oled_woled"]
NEGATIVE_SAMPLE_PROB = 0.02
TEXT_VERTICAL_JITTER = 3
TEXT_MOTION_BLUR_PROB = 0.45
CHAR_VISIBLE_TOL = 3

OUTPUT_DIR = Path("Character_Detection_Swift-YOLO_192") / "dataset_8k"

DEBUG_DRAW_BBOX = False
DEBUG_BBOX_DIR = OUTPUT_DIR / "debug_bbox"

FINAL_W, FINAL_H = 192, 192
CROP_SIZE = 640

# Large 4:3 background world.
# 1280x960 is enough for a centered 640x640 crop after +/-45 deg rotation.
WORLD_W, WORLD_H = 1280, 960

SCALE = FINAL_W / CROP_SIZE

# NUM_PER_CLASS = 200
NUM_PHRASE_IMAGES = 8000

CHARS = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") + ["-", "=", "."]
SAFE_NAMES = {"-": "dash", "=": "equals", ".": "dot"}

MEASURE_IMG = Image.new("RGBA", (10, 10), (0, 0, 0, 0))
MEASURE_DRAW = ImageDraw.Draw(MEASURE_IMG)
TEXT_SIZE_CACHE = {}

PHRASES = [
    "HELLO WORLD.",
    "A-B=3",
    "TEST 123.",
    "ECE-3073=OK.",
    "AI MODEL.",
    "NO DEFECT.",
    "CRACK-1.",
    "TEMP=25.",
    "ATTACK",
    "DEFEND",
    "JUMP",
    "BLOCK",
    "RUN",
    "ATTACK NOW.",
    "DEFEND BASE.",
    "JUMP-1",
    "BLOCK=ON",
    "RUN FAST.",
    "ATTACK=TRUE",
    "DEFEND-2",
    "RUN 123.",
    "JUMP HIGH.",
    "BLOCK ATTACK.",
]

SPLIT_RATIOS = {
    "train": 0.7,
    "valid": 0.2,
    "test": 0.1,
}

FONT_PATHS = [
    r"C:\Windows\Fonts\consolab.ttf",
    r"C:\Windows\Fonts\consola.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
]

FONT_CACHE = {}

# Monitor size settings in the 640x640 camera crop.
MONITOR_ASPECT = 16 / 9
MONITOR_MAX_W_CAP = 640

# Minimum monitor size as a ratio of the largest monitor that can fit
# at the chosen rotation angle.
MONITOR_MIN_FIT_RATIO = 0.72

# Tiny math safety only. Prevents anti-aliased monitor/screen pixels clipping.
GEOMETRY_EPS = 6

# Camera/background rotation.
MAX_CAMERA_ROLL_DEG = 45.0

# COCO state.
coco_data = {}
image_id_counter = 1
annotation_id_counter = 1


# ============================================================
# SETUP / COCO
# ============================================================


def make_dirs():
    for split in SPLIT_RATIOS:
        (OUTPUT_DIR / split).mkdir(parents=True, exist_ok=True)


def init_coco():
    global coco_data

    categories = [
        {"id": i + 1, "name": char, "supercategory": "character"}
        for i, char in enumerate(CHARS)
    ]

    coco_data = {}
    for split in SPLIT_RATIOS:
        coco_data[split] = {
            "info": {
                "description": "Synthetic monitor character detection dataset",
                "version": "2.0",
                "year": 2026,
            },
            "licenses": [],
            "images": [],
            "annotations": [],
            "categories": categories,
        }


def choose_split():
    r = random.random()
    if r < SPLIT_RATIOS["train"]:
        return "train"
    if r < SPLIT_RATIOS["train"] + SPLIT_RATIOS["valid"]:
        return "valid"
    return "test"


def add_coco_image(split, filename):
    global image_id_counter

    image_id = image_id_counter
    image_id_counter += 1

    coco_data[split]["images"].append(
        {
            "id": image_id,
            "file_name": filename,
            "width": FINAL_W,
            "height": FINAL_H,
        }
    )

    return image_id


def add_coco_annotation(split, image_id, char, bbox_xywh):
    global annotation_id_counter

    x, y, w, h = bbox_xywh
    if w <= 1 or h <= 1:
        return

    coco_data[split]["annotations"].append(
        {
            "id": annotation_id_counter,
            "image_id": image_id,
            "category_id": CHARS.index(char) + 1,
            "bbox": [
                round(float(x), 2),
                round(float(y), 2),
                round(float(w), 2),
                round(float(h), 2),
            ],
            "area": round(float(w * h), 2),
            "iscrowd": 0,
            "segmentation": [],
        }
    )

    annotation_id_counter += 1


def save_coco_files():
    for split in SPLIT_RATIOS:
        json_path = OUTPUT_DIR / split / "_annotations.coco.json"
        with open(json_path, "w", encoding="utf-8") as f:
            json.dump(coco_data[split], f, indent=2)
        print(f"Saved {json_path}")


# ============================================================
# FONT / COLOR
# ============================================================


def load_font(size):
    size = int(size)
    if size in FONT_CACHE:
        return FONT_CACHE[size]

    for path in FONT_PATHS:
        if os.path.exists(path):
            font = ImageFont.truetype(path, size)
            FONT_CACHE[size] = font
            return font

    raise FileNotFoundError("Could not find a usable monospace font.")


# def random_phrase():
#     length = random.randint(1, 16)
#     chars = CHARS + [" "]

#     text = "".join(random.choice(chars) for _ in range(length)).strip()

#     if not text:
#         text = random.choice(CHARS)

#     return text

# WEAK_CHARS = list("FNSTW9-")

# def random_phrase(required_char=None):
#     length = random.randint(4, 18)

#     chars = []
#     for _ in range(length):
#         if random.random() < 0.25:
#             chars.append(random.choice(WEAK_CHARS))
#         else:
#             chars.append(random.choice(CHARS))

#     if required_char is not None:
#         chars[random.randrange(len(chars))] = required_char

#     out = []
#     for i, char in enumerate(chars):
#         out.append(char)
#         if i < len(chars) - 1 and random.random() < 0.18:
#             out.append(" ")

#     return "".join(out)

def random_phrase():
    """
    Generate random non-word phrases where each target character has
    approximately equal probability.

    Spaces are inserted separately, so they do not steal probability
    from actual labelled classes.
    """
    length = random.randint(4, 18)

    chars = []
    for i in range(length):
        chars.append(random.choice(CHARS))

        # Optional spaces between characters, but not too often
        if i < length - 1 and random.random() < 0.18:
            chars.append(" ")

    return "".join(chars)


def random_text_color():
    r = random.random()

    # Main target: white or near-white text
    if r < 0.85:
        v = random.randint(210, 255)
        tint = random.randint(-8, 8)
        return (
            max(0, min(255, v + tint)),
            max(0, min(255, v + tint)),
            max(0, min(255, v)),
            255,
        )

    # Slight LCD/app variation
    if r < 0.97:
        return random.choice(
            [
                (180, 220, 255, 255),  # cool white
                (220, 255, 220, 255),  # greenish white
                (255, 245, 190, 255),  # warm white
                (180, 255, 180, 255),  # pale green
            ]
        )

    # Rare odd cases
    return random.choice(
        [
            (255, 255, 0, 255),
            (0, 255, 255, 255),
            (0, 255, 0, 255),
        ]
    )


# ============================================================
# BACKGROUND WORLD
# ============================================================


def random_background_world():
    bg_type = random.choice(["dark_gray", "gradient", "noisy", "lab_like"])

    if bg_type == "dark_gray":
        val = random.randint(30, 120)
        return np.full((WORLD_H, WORLD_W, 3), val, dtype=np.uint8)

    if bg_type == "gradient":
        x = np.linspace(
            random.randint(20, 70),
            random.randint(90, 190),
            WORLD_W,
            dtype=np.uint8,
        )
        grad = np.tile(x, (WORLD_H, 1))
        return cv2.merge([grad, grad, grad])

    if bg_type == "lab_like":
        bg = np.full(
            (WORLD_H, WORLD_W, 3),
            random.randint(45, 130),
            dtype=np.uint8,
        )

        for _ in range(random.randint(10, 24)):
            x1 = random.randint(0, WORLD_W - 40)
            y1 = random.randint(0, WORLD_H - 40)
            x2 = random.randint(x1 + 20, min(WORLD_W, x1 + 220))
            y2 = random.randint(y1 + 20, min(WORLD_H, y1 + 180))

            color = (
                random.randint(20, 190),
                random.randint(20, 190),
                random.randint(20, 190),
            )

            cv2.rectangle(bg, (x1, y1), (x2, y2), color, -1)

        return bg

    return np.random.randint(20, 110, (WORLD_H, WORLD_W, 3), dtype=np.uint8)


def rotated_center_crop_background():
    """
    Create a large 4:3 background world, rotate it to simulate camera roll,
    then crop the centered 640x640 camera view.

    No border artifacts appear because the world is larger than the rotated crop footprint.
    """
    world = random_background_world()

    angle = random.uniform(-MAX_CAMERA_ROLL_DEG, MAX_CAMERA_ROLL_DEG)
    center = (WORLD_W / 2, WORLD_H / 2)
    M = cv2.getRotationMatrix2D(center, angle, 1.0)

    rotated = cv2.warpAffine(
        world,
        M,
        (WORLD_W, WORLD_H),
        flags=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT,
        borderValue=(0, 0, 0),
    )

    x1 = WORLD_W // 2 - CROP_SIZE // 2
    y1 = WORLD_H // 2 - CROP_SIZE // 2

    crop = rotated[y1 : y1 + CROP_SIZE, x1 : x1 + CROP_SIZE].copy()
    return crop


# ============================================================
# GEOMETRY HELPERS
# ============================================================


def draw_debug_bboxes(final_img, labels):
    """
    Draw final 192x192 COCO bboxes for visual checking.
    labels contains bbox in COCO xywh format.
    """
    debug = final_img.copy()

    for item in labels:
        x, y, w, h = item["bbox"]
        char = item["char"]

        x1 = int(round(x))
        y1 = int(round(y))
        x2 = int(round(x + w))
        y2 = int(round(y + h))

        cv2.rectangle(
            debug,
            (x1, y1),
            (x2, y2),
            (0, 255, 0),
            1,
        )

        cv2.putText(
            debug,
            char,
            (x1, max(0, y1 - 2)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.35,
            (0, 255, 0),
            1,
            cv2.LINE_AA,
        )

    return debug


def rotated_bbox_size(w, h, angle_deg):
    rad = math.radians(angle_deg)
    c = abs(math.cos(rad))
    s = abs(math.sin(rad))
    return w * c + h * s, w * s + h * c


def transform_bbox_affine_xyxy(bbox, M):
    x1, y1, x2, y2 = bbox

    pts = np.array(
        [
            [x1, y1, 1.0],
            [x2, y1, 1.0],
            [x2, y2, 1.0],
            [x1, y2, 1.0],
        ],
        dtype=np.float32,
    )

    M3 = np.vstack([M, [0, 0, 1]]).astype(np.float32)
    out = (M3 @ pts.T).T

    xs = out[:, 0]
    ys = out[:, 1]

    return float(xs.min()), float(ys.min()), float(xs.max()), float(ys.max())


def scale_and_clamp_bbox_to_final(bbox, pad=1):
    x1, y1, x2, y2 = bbox

    # Clamp in 640x640 first.
    x1 = max(0, min(CROP_SIZE - 1, x1))
    y1 = max(0, min(CROP_SIZE - 1, y1))
    x2 = max(0, min(CROP_SIZE - 1, x2))
    y2 = max(0, min(CROP_SIZE - 1, y2))

    if x2 <= x1 or y2 <= y1:
        return None

    # Resize bbox to 192x192.
    x1 *= SCALE
    y1 *= SCALE
    x2 *= SCALE
    y2 *= SCALE

    x1 = max(0, x1 - pad)
    y1 = max(0, y1 - pad)
    x2 = min(FINAL_W - 1, x2 + pad)
    y2 = min(FINAL_H - 1, y2 + pad)

    w = max(1, x2 - x1)
    h = max(1, y2 - y1)

    return x1, y1, w, h


def choose_monitor_geometry():
    """
    Choose monitor angle, size, and center so the rotated monitor bbox
    fits inside the 640x640 crop.

    GEOMETRY_EPS is only to prevent anti-aliasing/rounding crop artifacts.
    """
    for _ in range(100):
        angle = random.uniform(0, 360) if random.random() < 0.85 else 0.0

        rad = math.radians(angle)
        c = abs(math.cos(rad))
        s = abs(math.sin(rad))

        # monitor_h = monitor_w / MONITOR_ASPECT
        rot_w_coeff = c + (1 / MONITOR_ASPECT) * s
        rot_h_coeff = s + (1 / MONITOR_ASPECT) * c

        max_w_by_fit = min(
            (CROP_SIZE - 2 * GEOMETRY_EPS) / rot_w_coeff,
            (CROP_SIZE - 2 * GEOMETRY_EPS) / rot_h_coeff,
        )

        max_w = int(min(MONITOR_MAX_W_CAP, max_w_by_fit))
        min_w = int(max_w * MONITOR_MIN_FIT_RATIO)

        if min_w >= max_w:
            continue

        monitor_w = random.randint(min_w, max_w)
        monitor_h = int(round(monitor_w / MONITOR_ASPECT))

        rot_w, rot_h = rotated_bbox_size(monitor_w, monitor_h, angle)

        cx_min = int(math.ceil(rot_w / 2 + GEOMETRY_EPS))
        cx_max = int(math.floor(CROP_SIZE - rot_w / 2 - GEOMETRY_EPS))
        cy_min = int(math.ceil(rot_h / 2 + GEOMETRY_EPS))
        cy_max = int(math.floor(CROP_SIZE - rot_h / 2 - GEOMETRY_EPS))

        if cx_min > cx_max or cy_min > cy_max:
            continue

        cx = random.randint(cx_min, cx_max)
        cy = random.randint(cy_min, cy_max)

        return monitor_w, monitor_h, angle, cx, cy

    raise RuntimeError("Could not place monitor. Check monitor size constraints.")


# ============================================================
# MONITOR + TEXT LAYER
# ============================================================
def apply_oled_screen_effects(layer, screen_rect):
    """
    OLED / LG WOLED-like screen effects:
    - deeper blacks
    - high contrast
    - weaker LCD-style glow
    - subtle WRGB/subpixel texture
    - possible OLED banding
    - reflections still possible
    """
    sx1, sy1, sx2, sy2 = screen_rect

    arr = np.array(layer).astype(np.float32)
    screen = arr[sy1:sy2, sx1:sx2, :]

    h, w = screen.shape[:2]
    if h <= 4 or w <= 4:
        return layer

    yy, xx = np.indices((h, w))

    # Deeper black floor, OLED-style contrast
    rgb = screen[:, :, :3]
    gray = np.mean(rgb, axis=2)

    dark_mask = gray < 35
    rgb[dark_mask] *= random.uniform(0.25, 0.65)

    # Slight OLED near-black unevenness / banding
    if random.random() < 0.55:
        band = np.random.normal(0, 1, (h, 1)).astype(np.float32)
        band = cv2.GaussianBlur(band, (0, 0), sigmaX=0, sigmaY=random.uniform(5, 18))
        band -= band.min()
        band /= max(band.max(), 1e-6)

        strength = random.uniform(2, 10)
        darkness = 1.0 - np.clip(gray / 80.0, 0, 1)

        rgb += band[:, :, None] * darkness[:, :, None] * strength

    # WRGB-ish subpixel texture: weaker than LCD RGB stripes
    if random.random() < 0.35:
        cols = np.arange(w)
        stripe = np.ones((h, w, 3), dtype=np.float32)

        boost = random.uniform(1.003, 1.018)
        dim = random.uniform(0.990, 0.999)

        # Approximate WRGB behavior. Not physically perfect, but useful.
        stripe[:, cols % 4 == 0, :] *= boost  # white subpixel impression
        stripe[:, cols % 4 == 1, 2] *= boost  # red
        stripe[:, cols % 4 == 2, 1] *= boost  # green
        stripe[:, cols % 4 == 3, 0] *= boost  # blue

        stripe[:, cols % 4 != 0, :] *= dim

        rgb *= stripe

    # OLED is glossy: reflections are sharper, less washed-out than LCD
    if random.random() < 0.50:
        glare = np.zeros((h, w), dtype=np.float32)

        for _ in range(random.randint(1, 3)):
            gx = random.uniform(-0.2 * w, 1.2 * w)
            gy = random.uniform(-0.2 * h, 0.9 * h)

            rx = random.uniform(0.08 * w, 0.38 * w)
            ry = random.uniform(0.025 * h, 0.16 * h)

            blob = np.exp(
                -(((xx - gx) ** 2) / (2 * rx * rx) + ((yy - gy) ** 2) / (2 * ry * ry))
            )

            glare += blob * random.uniform(0.3, 1.0)

        glare = cv2.GaussianBlur(glare, (0, 0), sigmaX=random.uniform(2, 8))
        glare /= max(glare.max(), 1e-6)

        rgb += glare[:, :, None] * random.uniform(4, 18)

    # Very mild PWM / camera interaction
    if random.random() < 0.35:
        period = random.uniform(12, 38)
        phase = random.uniform(0, 2 * math.pi)

        pwm = 1.0 + random.uniform(0.015, 0.06) * np.sin(
            yy * 2 * math.pi / period + phase
        )

        rgb *= pwm[:, :, None]

    screen[:, :, :3] = np.clip(rgb, 0, 255)
    arr[sy1:sy2, sx1:sx2, :] = screen

    return Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), mode="RGBA")


def apply_lcd_screen_effects(layer, screen_rect):
    """
    Apply LCD/monitor effects only inside the screen area.

    More realistic than one perfect glare blob:
    - faint scanlines
    - subtle moire
    - uneven brightness falloff
    - multi-blob/streak glare
    - mild smudge/reflection haze
    """
    sx1, sy1, sx2, sy2 = screen_rect

    arr = np.array(layer).astype(np.float32)
    screen = arr[sy1:sy2, sx1:sx2, :]

    h, w = screen.shape[:2]

    if h <= 4 or w <= 4:
        return layer

    yy, xx = np.indices((h, w))

    # Slight screen brightness gradient, like viewing angle / LCD unevenness.
    if random.random() < 0.75:
        gx = random.uniform(-0.5 * w, 1.5 * w)
        gy = random.uniform(-0.5 * h, 1.5 * h)

        dist = np.sqrt((xx - gx) ** 2 + (yy - gy) ** 2)
        dist /= max(dist.max(), 1)

        strength = random.uniform(0.04, 0.12)
        gradient = 1.0 - strength * dist

        screen[:, :, :3] *= gradient[:, :, None]

    # Faint horizontal scanlines.
    if random.random() < 0.70:
        period = random.uniform(2.0, 4.5)
        phase = random.uniform(0, 2 * math.pi)
        strength = random.uniform(0.008, 0.035)

        scan = 1.0 - strength * (0.5 + 0.5 * np.sin(2.0 * np.pi * yy / period + phase))

        screen[:, :, :3] *= scan[:, :, None]

    # Very subtle diagonal moire / interference.
    if random.random() < 0.35:
        period = random.uniform(9.0, 28.0)
        angle = random.uniform(-1.0, 1.0)
        phase = random.uniform(0, 2 * math.pi)

        pattern = np.sin(
            (xx * math.cos(angle) + yy * math.sin(angle)) * 2.0 * np.pi / period + phase
        )

        strength = random.uniform(0.004, 0.018)
        factor = 1.0 + strength * pattern

        screen[:, :, :3] *= factor[:, :, None]

    # Subtle RGB/subpixel vertical texture.
    # Keep weak. At 192x192 this can become aliasing garbage fast.
    if random.random() < 0.25:
        stripe = np.ones((h, w, 3), dtype=np.float32)

        boost = random.uniform(1.005, 1.025)
        dim = random.uniform(0.985, 0.998)

        cols = np.arange(w)

        stripe[:, cols % 3 == 0, 0] *= boost
        stripe[:, cols % 3 == 1, 1] *= boost
        stripe[:, cols % 3 == 2, 2] *= boost

        stripe[:, cols % 3 != 0, 0] *= dim
        stripe[:, cols % 3 != 1, 1] *= dim
        stripe[:, cols % 3 != 2, 2] *= dim

        screen[:, :, :3] *= stripe

    # Irregular glare / reflection.
    # Multiple weak blobs + optional streak, not one suspicious perfect egg.
    if random.random() < 0.45:
        glare = np.zeros((h, w), dtype=np.float32)

        num_blobs = random.randint(1, 4)

        for _ in range(num_blobs):
            gx = random.uniform(-0.2 * w, 1.2 * w)
            gy = random.uniform(-0.2 * h, 0.9 * h)

            rx = random.uniform(0.12 * w, 0.55 * w)
            ry = random.uniform(0.04 * h, 0.28 * h)

            blob = np.exp(
                -(((xx - gx) ** 2) / (2 * rx * rx) + ((yy - gy) ** 2) / (2 * ry * ry))
            )

            glare += blob * random.uniform(0.25, 1.0)

        # Optional diagonal/linear reflection streak.
        if random.random() < 0.55:
            angle = random.uniform(-0.9, 0.9)
            offset = random.uniform(-0.4 * w, 1.2 * w)
            width = random.uniform(0.035 * w, 0.12 * w)

            line_coord = xx * math.cos(angle) + yy * math.sin(angle)
            streak = np.exp(-((line_coord - offset) ** 2) / (2 * width * width))

            glare += streak * random.uniform(0.25, 0.85)

        # Make glare imperfect/noisy.
        glare = cv2.GaussianBlur(glare, (0, 0), sigmaX=random.uniform(5, 18))
        glare = glare / max(glare.max(), 1e-6)

        glare_strength = random.uniform(5, 24)

        tint = np.array(
            [
                random.uniform(0.85, 1.05),
                random.uniform(0.90, 1.08),
                random.uniform(0.95, 1.18),
            ],
            dtype=np.float32,
        )

        screen[:, :, :3] += glare[:, :, None] * glare_strength * tint

    # Smudge / dirty screen haze.
    if random.random() < 0.30:
        smudge = np.random.normal(0, 1, (h, w)).astype(np.float32)
        smudge = cv2.GaussianBlur(
            smudge,
            (0, 0),
            sigmaX=random.uniform(12, 32),
            sigmaY=random.uniform(8, 26),
        )

        smudge -= smudge.min()
        smudge /= max(smudge.max(), 1e-6)

        strength = random.uniform(2, 10)

        # More visible on dark areas.
        gray = np.mean(screen[:, :, :3], axis=2)
        darkness = 1.0 - np.clip(gray / 255.0, 0, 1)

        screen[:, :, :3] += smudge[:, :, None] * darkness[:, :, None] * strength

    screen[:, :, :3] = np.clip(screen[:, :, :3], 0, 255)
    arr[sy1:sy2, sx1:sx2, :] = screen

    return Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), mode="RGBA")


def estimate_phrase_size(text, font, spacing=0):
    key = (text, font.size, spacing)

    if key in TEXT_SIZE_CACHE:
        return TEXT_SIZE_CACHE[key]

    total_w = 0
    max_h = 0

    for i, char in enumerate(text):
        bbox = MEASURE_DRAW.textbbox((0, 0), char, font=font)

        char_w = bbox[2] - bbox[0]
        char_h = bbox[3] - bbox[1]

        if char == " ":
            char_w = max(6, char_w)

        total_w += char_w

        if i < len(text) - 1:
            total_w += spacing

        max_h = max(max_h, char_h)

    TEXT_SIZE_CACHE[key] = (total_w, max_h)
    return total_w, max_h


def make_empty_rgba():
    return Image.new("RGBA", (CROP_SIZE, CROP_SIZE), (0, 0, 0, 0))


def fit_font_for_text(text, max_w, max_h, min_size=10, max_size=90):
    for size in range(max_size, min_size - 1, -1):
        font = load_font(size)

        text_w, text_h = estimate_phrase_size(
            text,
            font,
            spacing=0,
        )

        if text_w <= max_w and text_h <= max_h:
            return font

    return load_font(min_size)


def draw_monitor_and_text(text):
    """
    Draw a monitor with a single horizontally scrolling text row.

    - Text is vertically centered.
    - Text scrolls right-to-left.
    - Characters may be partially clipped by screen edges.
    - Only fully visible characters get labels.
    - Some samples are negative: monitor/screen but no text.
    """
    monitor_w, monitor_h, angle, cx, cy = choose_monitor_geometry()

    local_side = (
        int(math.ceil(math.sqrt(monitor_w**2 + monitor_h**2))) + 2 * GEOMETRY_EPS + 20
    )

    local_w = local_side
    local_h = local_side

    layer = Image.new("RGBA", (local_w, local_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    x1 = int(round(local_w / 2 - monitor_w / 2))
    y1 = int(round(local_h / 2 - monitor_h / 2))
    x2 = x1 + monitor_w
    y2 = y1 + monitor_h

    bezel_color = (
        random.randint(5, 45),
        random.randint(5, 45),
        random.randint(5, 45),
        255,
    )

    draw.rounded_rectangle(
        (x1, y1, x2, y2),
        radius=max(4, monitor_h // 28),
        fill=bezel_color,
    )

    bezel_pad = max(6, int(random.uniform(0.035, 0.065) * monitor_w))

    sx1 = x1 + bezel_pad
    sy1 = y1 + bezel_pad
    sx2 = x2 - bezel_pad
    sy2 = y2 - bezel_pad

    # Deployment target: black screen with white text.
    screen_color = random.choice(
        [
            (0, 0, 0, 255),
            (2, 2, 2, 255),
            (4, 4, 5, 255),
            (8, 8, 10, 255),
        ]
    )

    draw.rectangle((sx1, sy1, sx2, sy2), fill=screen_color)

    usable_w = max(8, sx2 - sx1 - 10)
    usable_h = max(8, sy2 - sy1 - 10)

    if len(text) == 1:
        max_font = int(usable_h * random.uniform(0.55, 0.95))
        min_font = max(12, int(usable_h * 0.25))
        font = load_font(random.randint(min_font, max(min_font, max_font)))
    else:
        font = fit_font_for_text(
            text,
            int(usable_w * 2.5),  # scrolling text can be wider than screen
            int(usable_h * 0.72),
            min_size=10,
            max_size=int(usable_h * 0.60),
        )

    screen_type = random.choices(
        ["lcd", "oled_woled"],
        weights=[0.65, 0.35],
        k=1,
    )[0]

    labels = []
    is_negative = random.random() < NEGATIVE_SAMPLE_PROB

    if not is_negative:
        color = random_text_color()

        full_bbox = draw.textbbox((0, 0), text, font=font)
        text_w = full_bbox[2] - full_bbox[0]

        # Tkinter-like: canvas y = screen center, anchor="w".
        # PIL draws from top-left, so compensate using bbox vertical center.
        screen_center_y = (sy1 + sy2) / 2

        bbox_center_y = (full_bbox[1] + full_bbox[3]) / 2

        ty = int(
            screen_center_y
            - bbox_center_y
            + random.randint(-TEXT_VERTICAL_JITTER, TEXT_VERTICAL_JITTER)
        )

        # Simulate one frame of right-to-left scrolling:
        # starts off-screen right, exits off-screen left.
        screen_w = sx2 - sx1

        # Tkinter starts at x = WIDTH, anchor="w".
        # In screen coordinates, that means x starts at sx2.
        # It disappears when the text bbox right edge is < sx1.
        scroll_progress = random.uniform(0.0, 1.0)

        tx = int(sx2 - scroll_progress * (text_w + screen_w))

        text_layer = Image.new("RGBA", (local_w, local_h), (0, 0, 0, 0))
        text_draw = ImageDraw.Draw(text_layer)

        cursor_x = tx

        for char in text:
            if char == " ":
                space_bbox = text_draw.textbbox((0, 0), " ", font=font)
                cursor_x += max(6, space_bbox[2] - space_bbox[0])
                continue

            if char not in CHARS:
                continue

            bbox = text_draw.textbbox((cursor_x, ty), char, font=font)
            text_draw.text((cursor_x, ty), char, font=font, fill=color)

            fully_visible = (
                bbox[0] >= sx1 - CHAR_VISIBLE_TOL
                and bbox[1] >= sy1 - CHAR_VISIBLE_TOL
                and bbox[2] <= sx2 + CHAR_VISIBLE_TOL
                and bbox[3] <= sy2 + CHAR_VISIBLE_TOL
            )

            if fully_visible:
                labels.append(
                    {
                        "char": char,
                        "bbox": bbox,
                    }
                )

            char_w = bbox[2] - bbox[0]
            cursor_x += char_w  # + random.randint(0, 2)

        # Text-only horizontal motion blur.
        if random.random() < TEXT_MOTION_BLUR_PROB:
            if screen_type == "lcd":
                kernel_size = random.choice([3, 5, 7])
            else:
                kernel_size = random.choice([3, 3, 5])

            kernel = np.zeros((kernel_size, kernel_size), dtype=np.float32)
            kernel[kernel_size // 2, :] = 1.0
            kernel /= kernel.sum()

            text_np = np.array(text_layer)
            text_np = cv2.filter2D(text_np, -1, kernel)
            text_layer = Image.fromarray(text_np, mode="RGBA")

        # Crop text to screen bounds so off-screen scrolling text is hidden
        text_np = np.array(text_layer)

        mask = np.zeros((local_h, local_w), dtype=np.uint8)
        mask[sy1:sy2, sx1:sx2] = 255

        text_np[:, :, 3] = (text_np[:, :, 3].astype(np.uint16) * mask // 255).astype(
            np.uint8
        )

        text_layer = Image.fromarray(text_np, mode="RGBA")

        layer.alpha_composite(text_layer)

    if screen_type == "lcd":
        layer = apply_lcd_screen_effects(layer, (sx1, sy1, sx2, sy2))
    else:
        layer = apply_oled_screen_effects(layer, (sx1, sy1, sx2, sy2))

    local_center = (local_w / 2, local_h / 2)

    M = cv2.getRotationMatrix2D(
        local_center,
        angle,
        1.0,
    )

    layer_np = np.array(layer)

    rotated_np = cv2.warpAffine(
        layer_np,
        M,
        (local_w, local_h),
        flags=cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_CONSTANT,
        borderValue=(0, 0, 0, 0),
    )

    final_layer = np.zeros((CROP_SIZE, CROP_SIZE, 4), dtype=np.uint8)

    paste_x = int(round(cx - local_w / 2))
    paste_y = int(round(cy - local_h / 2))

    src_x1 = max(0, -paste_x)
    src_y1 = max(0, -paste_y)
    src_x2 = min(local_w, CROP_SIZE - paste_x)
    src_y2 = min(local_h, CROP_SIZE - paste_y)

    dst_x1 = max(0, paste_x)
    dst_y1 = max(0, paste_y)
    dst_x2 = dst_x1 + (src_x2 - src_x1)
    dst_y2 = dst_y1 + (src_y2 - src_y1)

    if src_x2 > src_x1 and src_y2 > src_y1:
        final_layer[dst_y1:dst_y2, dst_x1:dst_x2] = rotated_np[
            src_y1:src_y2,
            src_x1:src_x2,
        ]

    out_labels = []

    for item in labels:
        rotated_bbox = transform_bbox_affine_xyxy(
            item["bbox"],
            M,
        )

        crop_bbox = (
            rotated_bbox[0] + paste_x,
            rotated_bbox[1] + paste_y,
            rotated_bbox[2] + paste_x,
            rotated_bbox[3] + paste_y,
        )

        final_xywh = scale_and_clamp_bbox_to_final(
            crop_bbox,
            pad=random.randint(1, 2),
        )

        if final_xywh is not None:
            out_labels.append(
                {
                    "char": item["char"],
                    "bbox": final_xywh,
                }
            )

    return final_layer, out_labels


def alpha_composite_bgr_with_rgba(background_bgr, overlay_rgba):
    overlay_rgb = overlay_rgba[:, :, :3].astype(np.float32)
    alpha = overlay_rgba[:, :, 3:4].astype(np.float32) / 255.0

    bg_rgb = cv2.cvtColor(background_bgr, cv2.COLOR_BGR2RGB).astype(np.float32)
    out_rgb = overlay_rgb * alpha + bg_rgb * (1.0 - alpha)

    out_bgr = cv2.cvtColor(out_rgb.astype(np.uint8), cv2.COLOR_RGB2BGR)
    return out_bgr


# ============================================================
# FINAL CAMERA/IMAGE EFFECTS
# ============================================================
def distort_bbox_192_xywh_to_192_xywh(bbox, k1):
    """
    Approximate bbox update for radial lens distortion.

    Input/output bbox is in final 192x192 COCO xywh format.
    Internally converts to 640-space, distorts bbox corners,
    then converts back to 192-space.
    """
    x, y, bw, bh = bbox

    # 192 -> 640
    inv_scale = CROP_SIZE / FINAL_W

    x1 = x * inv_scale
    y1 = y * inv_scale
    x2 = (x + bw) * inv_scale
    y2 = (y + bh) * inv_scale

    pts = np.array(
        [
            [x1, y1],
            [x2, y1],
            [x2, y2],
            [x1, y2],
        ],
        dtype=np.float32,
    )

    cx = CROP_SIZE / 2
    cy = CROP_SIZE / 2

    out_pts = []

    for px, py in pts:
        nx = (px - cx) / cx
        ny = (py - cy) / cy

        r2 = nx * nx + ny * ny

        # Approximate inverse of the remap distortion.
        scale = 1.0 / max(1e-6, 1.0 + k1 * r2)

        dx = cx + nx * scale * cx
        dy = cy + ny * scale * cy

        out_pts.append([dx, dy])

    out_pts = np.array(out_pts)

    nx1 = np.clip(out_pts[:, 0].min(), 0, CROP_SIZE - 1)
    ny1 = np.clip(out_pts[:, 1].min(), 0, CROP_SIZE - 1)
    nx2 = np.clip(out_pts[:, 0].max(), 0, CROP_SIZE - 1)
    ny2 = np.clip(out_pts[:, 1].max(), 0, CROP_SIZE - 1)

    if nx2 <= nx1 or ny2 <= ny1:
        return None

    # 640 -> 192
    nx1 *= SCALE
    ny1 *= SCALE
    nx2 *= SCALE
    ny2 *= SCALE

    nx1 = np.clip(nx1, 0, FINAL_W - 1)
    ny1 = np.clip(ny1, 0, FINAL_H - 1)
    nx2 = np.clip(nx2, 0, FINAL_W - 1)
    ny2 = np.clip(ny2, 0, FINAL_H - 1)

    return (
        float(nx1),
        float(ny1),
        float(max(1, nx2 - nx1)),
        float(max(1, ny2 - ny1)),
    )


def apply_final_camera_effects(img, labels=None):
    """
    Camera-only effects after monitor/background compositing.

    If lens distortion is enabled, labels are updated too.
    labels must be final 192x192 COCO-style:
    [{"char": ..., "bbox": (x, y, w, h)}, ...]
    """
    if labels is None:
        labels = []

    h, w = img.shape[:2]
    out = img.astype(np.float32)

    # Auto exposure / gain
    gain = random.uniform(0.82, 1.28)
    bias = random.uniform(-12, 14)
    out = out * gain + bias

    # Auto white balance, BGR order
    wb = np.array(
        [
            random.uniform(0.92, 1.08),
            random.uniform(0.95, 1.05),
            random.uniform(0.92, 1.10),
        ],
        dtype=np.float32,
    )
    out *= wb
    out = np.clip(out, 0, 255).astype(np.uint8)

    # Local exposure / tone mapping
    if random.random() < 0.55:
        gray = cv2.cvtColor(out, cv2.COLOR_BGR2GRAY).astype(np.float32)

        local = cv2.GaussianBlur(
            gray,
            (0, 0),
            sigmaX=random.uniform(25, 70),
            sigmaY=random.uniform(25, 70),
        )

        correction = (np.mean(local) - local) * random.uniform(0.08, 0.20)

        out = np.clip(
            out.astype(np.float32) + correction[:, :, None],
            0,
            255,
        ).astype(np.uint8)

    # Focus softness
    blur_roll = random.random()

    if blur_roll < 0.50:
        sigma = random.uniform(0.35, 1.00)
        out = cv2.GaussianBlur(out, (0, 0), sigmaX=sigma, sigmaY=sigma)

    elif blur_roll < 0.58:
        sigma = random.uniform(1.10, 1.80)
        out = cv2.GaussianBlur(out, (0, 0), sigmaX=sigma, sigmaY=sigma)

    # Rare motion blur
    if random.random() < 0.08:
        kernel_size = random.choice([3, 5])
        kernel = np.zeros((kernel_size, kernel_size), dtype=np.float32)

        direction = random.choice(["horizontal", "vertical", "diagonal"])

        if direction == "horizontal":
            kernel[kernel_size // 2, :] = 1.0
        elif direction == "vertical":
            kernel[:, kernel_size // 2] = 1.0
        else:
            np.fill_diagonal(kernel, 1.0)

        kernel /= kernel.sum()
        out = cv2.filter2D(out, -1, kernel)

    # Mild chromatic aberration
    if random.random() < 0.30:
        b, g, r = cv2.split(out)

        dx = random.choice([-1, 1])
        dy = random.choice([-1, 0, 1])

        M_r = np.float32([[1, 0, dx], [0, 1, dy]])
        M_b = np.float32([[1, 0, -dx], [0, 1, -dy]])

        r = cv2.warpAffine(r, M_r, (w, h), borderMode=cv2.BORDER_REPLICATE)
        b = cv2.warpAffine(b, M_b, (w, h), borderMode=cv2.BORDER_REPLICATE)

        out = cv2.merge([b, g, r])

    # Sensor noise
    if random.random() < 0.90:
        gray = cv2.cvtColor(out, cv2.COLOR_BGR2GRAY).astype(np.float32)
        darkness = 1.0 - gray / 255.0

        lum_noise = np.random.normal(
            0,
            random.uniform(2, 6),
            (h, w, 1),
        )

        chroma_noise = np.random.normal(
            0,
            random.uniform(1, 4),
            out.shape,
        )

        noise = lum_noise + chroma_noise * 0.6
        noise *= 0.4 + darkness[:, :, None]

        out = np.clip(
            out.astype(np.float32) + noise,
            0,
            255,
        ).astype(np.uint8)

    # Vignetting, weakened
    if random.random() < 0.35:
        yy, xx = np.indices((h, w))
        cx = w / 2
        cy = h / 2

        dist = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2)
        dist /= dist.max()

        strength = random.uniform(0.03, 0.12)
        power = random.uniform(1.5, 2.5)

        vignette = 1.0 - strength * (dist**power)

        out = np.clip(
            out.astype(np.float32) * vignette[:, :, None],
            0,
            255,
        ).astype(np.uint8)

    # Demosaic-like softness / color bleed approximation
    if random.random() < 0.45:
        soft = cv2.resize(
            out,
            (max(1, w // 2), max(1, h // 2)),
            interpolation=cv2.INTER_LINEAR,
        )

        soft = cv2.resize(
            soft,
            (w, h),
            interpolation=cv2.INTER_LINEAR,
        )

        out = cv2.addWeighted(
            out,
            0.78,
            soft,
            0.22,
            0,
        )

    # Rolling shutter brightness variation
    if random.random() < 0.40:
        yy = np.linspace(0, 2 * np.pi, h)

        band = 1.0 + random.uniform(0.01, 0.04) * np.sin(
            yy * random.uniform(1.0, 3.0) + random.uniform(0, 6)
        )

        out = np.clip(
            out.astype(np.float32) * band[:, None, None],
            0,
            255,
        ).astype(np.uint8)

    # Lens distortion with bbox update
    if random.random() < 0.20:
        k1 = random.uniform(-0.08, -0.025)

        cx = w / 2
        cy = h / 2

        yy, xx = np.indices((h, w), dtype=np.float32)

        nx = (xx - cx) / cx
        ny = (yy - cy) / cy
        r2 = nx * nx + ny * ny

        scale = 1.0 + k1 * r2

        mapx = cx + nx * scale * cx
        mapy = cy + ny * scale * cy

        out = cv2.remap(
            out,
            mapx.astype(np.float32),
            mapy.astype(np.float32),
            cv2.INTER_LINEAR,
            borderMode=cv2.BORDER_REPLICATE,
        )

        fixed_labels = []

        for item in labels:
            new_bbox = distort_bbox_192_xywh_to_192_xywh(
                item["bbox"],
                k1,
            )

            if new_bbox is not None:
                fixed_labels.append(
                    {
                        "char": item["char"],
                        "bbox": new_bbox,
                    }
                )

        labels = fixed_labels

    # Resolution path realism
    if random.random() < 0.45:
        scale = random.choice([1.5, 2.0, 2.5])

        hi = cv2.resize(
            out,
            (int(w * scale), int(h * scale)),
            interpolation=cv2.INTER_CUBIC,
        )

        if random.random() < 0.70:
            hi = cv2.GaussianBlur(
                hi,
                (0, 0),
                sigmaX=random.uniform(0.4, 1.2),
                sigmaY=random.uniform(0.4, 1.2),
            )

        out = cv2.resize(
            hi,
            (w, h),
            interpolation=cv2.INTER_AREA,
        )

    # ISP sharpening, only once
    if random.random() < 0.50:
        blurred = cv2.GaussianBlur(out, (0, 0), sigmaX=1.0)
        amount = random.uniform(0.15, 0.45)

        out = cv2.addWeighted(
            out,
            1.0 + amount,
            blurred,
            -amount,
            0,
        )

    # JPEG compression
    if random.random() < 0.80:
        quality = random.randint(60, 92)

        ok, enc = cv2.imencode(
            ".jpg",
            out,
            [int(cv2.IMWRITE_JPEG_QUALITY), quality],
        )

        if ok:
            out = cv2.imdecode(enc, cv2.IMREAD_COLOR)

    return out, labels


# ============================================================
# SAMPLE GENERATION
# ============================================================


def create_sample(text, filename):

    for split in SPLIT_RATIOS:

        image_file = OUTPUT_DIR / split / filename

        if image_file.exists():

            print(f"Dataset image exists: {image_file}")
            print("Stopping generation.")

            raise SystemExit(0)

        if DEBUG_DRAW_BBOX:

            debug_file = DEBUG_BBOX_DIR / split / filename

            if debug_file.exists():

                print(f"Debug image exists: {debug_file}")
                print("Stopping generation.")

                raise SystemExit(0)

    bg_crop = rotated_center_crop_background()

    monitor_rgba, labels = draw_monitor_and_text(text)

    composed = alpha_composite_bgr_with_rgba(
        bg_crop,
        monitor_rgba,
    )

    composed, labels = apply_final_camera_effects(
        composed,
        labels,
    )

    final_img = cv2.resize(
        composed,
        (FINAL_W, FINAL_H),
        interpolation=cv2.INTER_AREA,
    )

    split = choose_split()

    image_path = OUTPUT_DIR / split / filename

    cv2.imwrite(
        str(image_path),
        final_img,
    )

    if DEBUG_DRAW_BBOX:
        debug_split_dir = DEBUG_BBOX_DIR / split
        debug_split_dir.mkdir(parents=True, exist_ok=True)

        debug_img = draw_debug_bboxes(final_img, labels)

        cv2.imwrite(
            str(debug_split_dir / filename),
            debug_img,
        )

    image_id = add_coco_image(split, filename)

    for item in labels:
        add_coco_annotation(split, image_id, item["char"], item["bbox"])


def create_single_char_sample(char, sample_index):
    safe_char = SAFE_NAMES.get(char, char)
    filename = f"{safe_char}_{sample_index:05d}.jpg"
    create_sample(char, filename)


def create_phrase_sample(text, sample_index):
    filename = f"phrase_{sample_index:05d}.jpg"
    create_sample(text, filename)


# ============================================================
# MAIN
# ============================================================


def main():
    make_dirs()
    init_coco()

    # for class_index, char in enumerate(CHARS):
    #     print(f"Generating class {class_index}: {char}")
    #     for i in range(NUM_PER_CLASS):
    #         create_single_char_sample(char, i)

    print("\nGenerating phrase samples...")
    for i in range(NUM_PHRASE_IMAGES):
        # if random.random() < 0.30:
        #     text = random.choice(PHRASES)
        # else:
        text = random_phrase()

        create_phrase_sample(text, i)
        if i % 100 == 0:
            print(f"Generated {i} images.")

    save_coco_files()

    print("\nDone.")
    print(f"Dataset saved to: {OUTPUT_DIR.resolve()}")
    print(f"Classes: {len(CHARS)}")


if __name__ == "__main__":
    main()
