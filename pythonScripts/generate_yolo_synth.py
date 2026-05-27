import os
import random
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

# ============================================================
# CONFIG
# ============================================================

OUTPUT_DIR = Path("YOLO_Character_Scrolling_Text_192")

NUM_IMAGES = 2000
DEBUG = False

CANVAS_W, CANVAS_H = 640, 640
FINAL_W, FINAL_H = 192, 192

SPLIT_RATIOS = {
    "train": 0.70,
    "valid": 0.20,
    "test": 0.10,
}

CLASS_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
CHAR_TO_ID = {ch: i for i, ch in enumerate(CLASS_CHARS)}

FONT_PATHS = [
    r"C:\Windows\Fonts\OCRAEXT.ttf",
    r"C:\Windows\Fonts\consola.ttf",
    r"C:\Windows\Fonts\arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation2/LiberationMono-Bold.ttf",
]

MONITOR_ASPECT = 16 / 9
MONITOR_W_RANGE = (430, 610)
MONITOR_ROTATION_DEG = (-6.0, 6.0)
MONITOR_CENTER_JITTER = 35

TEXT_VERTICAL_JITTER = 7
TEXT_MOTION_BLUR_PROB = 0.30

ALLOW_EMPTY_LABELS = False
MIN_BOX_W = 2.0
MIN_BOX_H = 3.0
MASK_THRESHOLD = 18
MIN_VISIBLE_RATIO = 0.99
MIN_VISIBLE_PIXELS = 10

BACKGROUND_NOISE_PROB = 0.70
CAMERA_BLUR_PROB = 0.35
JPEG_PROB = 0.70

FONT_CACHE: dict[int, ImageFont.FreeTypeFont] = {}
MEASURE_IMG = Image.new("RGB", (8, 8), (0, 0, 0))
MEASURE_DRAW = ImageDraw.Draw(MEASURE_IMG)

BALANCED_CHAR_POOL: list[str] = []


# ============================================================
# FILE / DATA HELPERS
# ============================================================


def make_dirs() -> None:
    for split in SPLIT_RATIOS:
        (OUTPUT_DIR / "images" / split).mkdir(parents=True, exist_ok=True)
        (OUTPUT_DIR / "labels" / split).mkdir(parents=True, exist_ok=True)
    if DEBUG:
        for split in SPLIT_RATIOS:
            (OUTPUT_DIR / "debug" / split).mkdir(parents=True, exist_ok=True)


def choose_split() -> str:
    r = random.random()
    acc = 0.0
    for split, ratio in SPLIT_RATIOS.items():
        acc += ratio
        if r <= acc:
            return split
    return "test"


def write_data_yaml() -> None:
    lines = [
        f"path: {OUTPUT_DIR.resolve()}",
        "train: images/train",
        "val: images/valid",
        "test: images/test",
        "",
        f"nc: {len(CLASS_CHARS)}",
        "names:",
    ]
    lines.extend(f"  {i}: '{ch}'" for i, ch in enumerate(CLASS_CHARS))
    (OUTPUT_DIR / "data.yaml").write_text("\n".join(lines) + "\n", encoding="utf-8")


# ============================================================
# TEXT / FONT HELPERS
# ============================================================


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
        "No usable font found. Add a valid .ttf path to FONT_PATHS."
    )


def text_bbox(text: str, font: ImageFont.FreeTypeFont) -> tuple[int, int, int, int]:
    return MEASURE_DRAW.textbbox((0, 0), text, font=font)


def text_size(text: str, font: ImageFont.FreeTypeFont) -> tuple[int, int]:
    x1, y1, x2, y2 = text_bbox(text, font)
    return x2 - x1, y2 - y1


def fit_font(
    text: str, max_h: int, min_size: int = 28, max_size: int = 90
) -> ImageFont.FreeTypeFont:
    # Same result as the old downward linear scan, but with far fewer textbbox calls.
    lo, hi = min_size, max_size
    best = min_size

    while lo <= hi:
        mid = (lo + hi) // 2
        font = load_font(mid)
        _, h = text_size(text, font)

        if h <= max_h:
            best = mid
            lo = mid + 1
        else:
            hi = mid - 1

    return load_font(best)


def refill_balanced_pool() -> None:
    global BALANCED_CHAR_POOL
    BALANCED_CHAR_POOL = list(CLASS_CHARS * 5)
    random.shuffle(BALANCED_CHAR_POOL)


def next_balanced_char() -> str:
    if not BALANCED_CHAR_POOL:
        refill_balanced_pool()
    return BALANCED_CHAR_POOL.pop()


def generate_word(min_len: int = 2, max_len: int = 10) -> str:
    return "".join(
        next_balanced_char() for _ in range(random.randint(min_len, max_len))
    )


def random_phrase() -> str:
    return " ".join(generate_word() for _ in range(random.randint(3, 12)))


# ============================================================
# IMAGE EFFECTS
# ============================================================


def random_background() -> np.ndarray:
    base = np.full((CANVAS_H, CANVAS_W, 3), random.randint(35, 115), dtype=np.uint8)

    x_grad = np.linspace(random.randint(-25, 25), random.randint(-15, 40), CANVAS_W)
    y_grad = np.linspace(random.randint(-25, 25), random.randint(-15, 40), CANVAS_H)
    grad = x_grad[None, :] + y_grad[:, None]
    base = np.clip(base.astype(np.float32) + grad[:, :, None], 0, 255).astype(np.uint8)

    if random.random() < 0.45:
        for _ in range(random.randint(2, 8)):
            x1 = random.randint(0, CANVAS_W - 80)
            y1 = random.randint(0, CANVAS_H - 80)
            x2 = random.randint(x1 + 30, min(CANVAS_W, x1 + 200))
            y2 = random.randint(y1 + 30, min(CANVAS_H, y1 + 170))
            color = random.randint(25, 140)
            cv2.rectangle(base, (x1, y1), (x2, y2), (color, color, color), -1)

    if random.random() < BACKGROUND_NOISE_PROB:
        noise = np.random.normal(0, random.uniform(2, 8), base.shape)
        base = np.clip(base.astype(np.float32) + noise, 0, 255).astype(np.uint8)

    return base


def apply_screen_effects(
    rgb: np.ndarray, alpha: np.ndarray, screen_rect: tuple[int, int, int, int]
) -> Image.Image:
    x1, y1, x2, y2 = screen_rect
    x1, y1 = max(0, x1), max(0, y1)
    x2, y2 = min(CANVAS_W, x2), min(CANVAS_H, y2)

    if x2 <= x1 + 2 or y2 <= y1 + 2:
        return Image.fromarray(np.dstack([rgb, alpha]).astype(np.uint8), mode="RGBA")

    screen = rgb[y1:y2, x1:x2].astype(np.float32)
    h, w = screen.shape[:2]
    yy, xx = np.indices((h, w))

    if random.random() < 0.75:
        period = random.uniform(5.0, 12.0)
        strength = random.uniform(0.015, 0.050)
        scan = 1.0 - strength * (0.5 + 0.5 * np.sin(2 * np.pi * yy / period))
        screen *= scan[:, :, None]

    if random.random() < 0.45:
        gx = random.uniform(-0.2 * w, 1.2 * w)
        gy = random.uniform(-0.1 * h, 0.9 * h)
        rx = random.uniform(0.18 * w, 0.60 * w)
        ry = random.uniform(0.05 * h, 0.22 * h)
        blob = np.exp(
            -(((xx - gx) ** 2) / (2 * rx * rx) + ((yy - gy) ** 2) / (2 * ry * ry))
        )
        screen += blob[:, :, None] * random.uniform(5, 28)

    rgb[y1:y2, x1:x2] = np.clip(screen, 0, 255).astype(np.uint8)
    return Image.fromarray(np.dstack([rgb, alpha]).astype(np.uint8), mode="RGBA")


def apply_camera_effects(rgb: np.ndarray) -> np.ndarray:
    out = rgb.astype(np.float32)

    out = out * random.uniform(0.82, 1.22) + random.uniform(-10, 12)
    out *= np.array(
        [
            random.uniform(0.93, 1.08),
            random.uniform(0.95, 1.05),
            random.uniform(0.93, 1.08),
        ],
        dtype=np.float32,
    )
    out = np.clip(out, 0, 255).astype(np.uint8)

    if random.random() < 0.75:
        gray = cv2.cvtColor(out, cv2.COLOR_RGB2GRAY)
        mask = (gray > random.randint(185, 225)).astype(np.uint8) * 255
        glow = (
            cv2.GaussianBlur(mask, (0, 0), sigmaX=random.uniform(1.5, 4.5)).astype(
                np.float32
            )
            / 255.0
        )
        out = np.clip(
            out.astype(np.float32) + glow[:, :, None] * random.uniform(8, 35), 0, 255
        ).astype(np.uint8)

    if random.random() < CAMERA_BLUR_PROB:
        sigma = random.uniform(0.35, 1.15)
        out = cv2.GaussianBlur(out, (0, 0), sigmaX=sigma, sigmaY=sigma)

    if random.random() < 0.65:
        noise = np.random.normal(0, random.uniform(1.5, 5.0), out.shape)
        out = np.clip(out.astype(np.float32) + noise, 0, 255).astype(np.uint8)

    if random.random() < JPEG_PROB:
        bgr = cv2.cvtColor(out, cv2.COLOR_RGB2BGR)
        ok, enc = cv2.imencode(
            ".jpg", bgr, [int(cv2.IMWRITE_JPEG_QUALITY), random.randint(62, 94)]
        )
        if ok:
            out = cv2.cvtColor(cv2.imdecode(enc, cv2.IMREAD_COLOR), cv2.COLOR_BGR2RGB)

    return out


# ============================================================
# MASK / BOX HELPERS
# ============================================================


def clip_mask_to_rect_np(
    mask_np: np.ndarray, rect: tuple[int, int, int, int]
) -> np.ndarray:
    # Use the same half-open rectangle convention as NumPy slicing:
    # [x1:x2, y1:y2]. This avoids one-pixel disagreement at the screen edge.
    x1, y1, x2, y2 = rect
    x1 = max(0, min(CANVAS_W, int(x1)))
    y1 = max(0, min(CANVAS_H, int(y1)))
    x2 = max(0, min(CANVAS_W, int(x2)))
    y2 = max(0, min(CANVAS_H, int(y2)))

    clipped = np.zeros_like(mask_np)

    if x2 > x1 and y2 > y1:
        clipped[y1:y2, x1:x2] = mask_np[y1:y2, x1:x2]

    return clipped


def rotate_rgba(img: Image.Image, angle: float, center: tuple[int, int]) -> Image.Image:
    return img.rotate(
        angle,
        resample=Image.Resampling.BICUBIC,
        center=center,
        fillcolor=(0, 0, 0, 0),
    )


def rotate_mask_np(
    mask_np: np.ndarray, angle: float, center: tuple[int, int]
) -> np.ndarray:
    # OpenCV is much faster than rotating one full-size PIL image per glyph.
    # The operation is the same idea: bicubic rotate around the monitor centre,
    # transparent/zero-filled outside the canvas.
    matrix = cv2.getRotationMatrix2D(center, angle, 1.0)
    return cv2.warpAffine(
        mask_np,
        matrix,
        (CANVAS_W, CANVAS_H),
        flags=cv2.INTER_CUBIC,
        borderMode=cv2.BORDER_CONSTANT,
        borderValue=0,
    )



def resize_rgb_to_final(rgb: np.ndarray) -> np.ndarray:
    return cv2.resize(rgb, (FINAL_W, FINAL_H), interpolation=cv2.INTER_AREA)


def resize_mask_to_final(mask_np: np.ndarray) -> np.ndarray:
    return cv2.resize(mask_np, (FINAL_W, FINAL_H), interpolation=cv2.INTER_AREA)


def bbox_from_mask(mask_np: np.ndarray) -> tuple[float, float, float, float] | None:
    ys, xs = np.where(mask_np > MASK_THRESHOLD)
    if xs.size == 0:
        return None

    x1 = float(xs.min())
    y1 = float(ys.min())
    x2 = float(xs.max() + 1)
    y2 = float(ys.max() + 1)

    x1 = max(0.0, min(FINAL_W, x1))
    y1 = max(0.0, min(FINAL_H, y1))
    x2 = max(0.0, min(FINAL_W, x2))
    y2 = max(0.0, min(FINAL_H, y2))

    if (x2 - x1) < MIN_BOX_W or (y2 - y1) < MIN_BOX_H:
        return None

    return x1, y1, x2, y2


def bbox_to_yolo(
    box: tuple[float, float, float, float],
) -> tuple[float, float, float, float]:
    x1, y1, x2, y2 = box
    return (
        ((x1 + x2) / 2.0) / FINAL_W,
        ((y1 + y2) / 2.0) / FINAL_H,
        (x2 - x1) / FINAL_W,
        (y2 - y1) / FINAL_H,
    )


# ============================================================
# RENDERING
# ============================================================


def render_monitor_scene(
    text: str,
) -> tuple[np.ndarray, list[tuple[int, tuple[float, float, float, float]]]]:
    bg = Image.fromarray(random_background(), mode="RGB").convert("RGBA")
    layer = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)

    mon_w = random.randint(*MONITOR_W_RANGE)
    mon_h = int(round(mon_w / MONITOR_ASPECT))

    cx = CANVAS_W // 2 + random.randint(-MONITOR_CENTER_JITTER, MONITOR_CENTER_JITTER)
    cy = CANVAS_H // 2 + random.randint(-MONITOR_CENTER_JITTER, MONITOR_CENTER_JITTER)

    x1 = int(cx - mon_w / 2)
    y1 = int(cy - mon_h / 2)
    x2 = x1 + mon_w
    y2 = y1 + mon_h

    bezel = random.randint(8, 35)
    draw.rounded_rectangle(
        (x1, y1, x2, y2), radius=max(12, mon_h // 22), fill=(bezel, bezel, bezel, 255)
    )

    pad = max(16, int(mon_w * random.uniform(0.045, 0.065)))
    sx1, sy1, sx2, sy2 = x1 + pad, y1 + pad, x2 - pad, y2 - pad
    screen_rect = (sx1, sy1, sx2, sy2)
    draw.rectangle(screen_rect, fill=(0, 0, 0, 255))

    screen_w = sx2 - sx1
    usable_h = max(35, sy2 - sy1 - 18)
    font = fit_font(text, max_h=int(usable_h * 0.78))

    full_bbox = text_bbox(text, font)
    text_w = full_bbox[2] - full_bbox[0]
    text_h = full_bbox[3] - full_bbox[1]

    progress = random.uniform(0.0, 1.0)
    tx = int(sx2 - progress * (text_w + screen_w))
    ty = int(
        (sy1 + sy2) / 2
        - text_h / 2
        - full_bbox[1]
        + random.randint(-TEXT_VERTICAL_JITTER, TEXT_VERTICAL_JITTER)
    )

    text_layer = Image.new("RGBA", (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    text_draw = ImageDraw.Draw(text_layer)

    glyph_masks: list[tuple[int, np.ndarray]] = []
    cursor_x = tx

    for ch in text:
        ch_bbox = text_draw.textbbox((cursor_x, ty), ch, font=font)
        ch_w = ch_bbox[2] - ch_bbox[0]

        if ch == " ":
            cursor_x += max(ch_w, int(font.size * 0.45))
            continue

        if ch in CHAR_TO_ID:
            text_draw.text((cursor_x, ty), ch, font=font, fill=(255, 255, 255, 255))

            mask = Image.new("L", (CANVAS_W, CANVAS_H), 0)
            mask_draw = ImageDraw.Draw(mask)
            mask_draw.text((cursor_x, ty), ch, font=font, fill=255)
            glyph_masks.append((CHAR_TO_ID[ch], np.array(mask, dtype=np.uint8)))

        cursor_x += ch_w

    if random.random() < TEXT_MOTION_BLUR_PROB:
        k = random.choice([3, 5, 7])
        kernel = np.zeros((k, k), dtype=np.float32)
        kernel[k // 2, :] = 1.0
        kernel /= kernel.sum()

        text_layer = Image.fromarray(
            cv2.filter2D(np.array(text_layer), -1, kernel), mode="RGBA"
        )
        glyph_masks = [
            (class_id, cv2.filter2D(mask_np, -1, kernel))
            for class_id, mask_np in glyph_masks
        ]

    # Clip text to the screen BEFORE rotating. This means boxes come only from visible pixels.
    text_np = np.array(text_layer)
    screen_mask = np.zeros((CANVAS_H, CANVAS_W), dtype=np.uint8)
    screen_mask[max(0, sy1) : min(CANVAS_H, sy2), max(0, sx1) : min(CANVAS_W, sx2)] = (
        255
    )
    text_np[:, :, 3] = (text_np[:, :, 3].astype(np.uint16) * screen_mask // 255).astype(
        np.uint8
    )
    text_layer = Image.fromarray(text_np, mode="RGBA")

    # Keep both full masks and screen-clipped masks.
    # The full mask is used only to measure how much of the character survived clipping.
    # The clipped mask is used for the actual YOLO bbox.
    glyph_masks = [
        (class_id, mask_np, clip_mask_to_rect_np(mask_np, screen_rect))
        for class_id, mask_np in glyph_masks
    ]

    layer.alpha_composite(text_layer)

    layer_np = np.array(layer)
    layer = apply_screen_effects(
        layer_np[:, :, :3].copy(), layer_np[:, :, 3].copy(), screen_rect
    )

    angle = random.uniform(*MONITOR_ROTATION_DEG)
    center = (cx, cy)

    layer = rotate_rgba(layer, angle, center)
    glyph_masks = [
        (
            class_id,
            rotate_mask_np(full_mask, angle, center),
            rotate_mask_np(clipped_mask, angle, center),
        )
        for class_id, full_mask, clipped_mask in glyph_masks
    ]

    bg.alpha_composite(layer)
    rgb = np.array(bg.convert("RGB"))
    rgb = apply_camera_effects(rgb)
    rgb_final = resize_rgb_to_final(rgb)

    annotations: list[tuple[int, tuple[float, float, float, float]]] = []
    for class_id, full_mask, clipped_mask in glyph_masks:
        full_final = resize_mask_to_final(full_mask)
        clipped_final = resize_mask_to_final(clipped_mask)

        full_area = int(np.count_nonzero(full_final > MASK_THRESHOLD))
        visible_area = int(np.count_nonzero(clipped_final > MASK_THRESHOLD))

        if full_area <= 0 or visible_area < MIN_VISIBLE_PIXELS:
            continue

        visible_ratio = visible_area / full_area

        # Drop characters where only a small sliver is visible at the screen edge.
        # These boxes are geometrically correct but bad training targets.
        if visible_ratio < MIN_VISIBLE_RATIO:
            continue

        box = bbox_from_mask(clipped_final)
        if box is not None:
            annotations.append((class_id, box))

    return rgb_final, annotations


# ============================================================
# WRITING
# ============================================================


def write_label_file(
    path: Path, annotations: list[tuple[int, tuple[float, float, float, float]]]
) -> None:
    lines = []
    for class_id, box in annotations:
        xc, yc, bw, bh = bbox_to_yolo(box)
        if 0.0 <= xc <= 1.0 and 0.0 <= yc <= 1.0 and bw > 0 and bh > 0:
            lines.append(f"{class_id} {xc:.6f} {yc:.6f} {bw:.6f} {bh:.6f}")
    path.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")


def save_debug_image(
    image_rgb: np.ndarray,
    annotations: list[tuple[int, tuple[float, float, float, float]]],
    path: Path,
) -> None:
    debug = image_rgb.copy()

    for class_id, box in annotations:
        x1, y1, x2, y2 = [int(round(v)) for v in box]
        ch = CLASS_CHARS[class_id]
        cv2.rectangle(debug, (x1, y1), (x2, y2), (0, 255, 0), 1)
        cv2.putText(
            debug,
            ch,
            (x1, max(12, y1 - 4)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.40,
            (0, 255, 0),
            1,
            cv2.LINE_AA,
        )

    cv2.imwrite(str(path), cv2.cvtColor(debug, cv2.COLOR_RGB2BGR))


# ============================================================
# MAIN
# ============================================================


def main() -> None:
    random.seed()
    np.random.seed(None)

    make_dirs()
    write_data_yaml()

    existing_indices = []

    for split in SPLIT_RATIOS:
        image_dir = OUTPUT_DIR / "images" / split

        if image_dir.exists():
            for f in image_dir.glob("img_*.jpg"):
                try:
                    idx = int(f.stem.split("_")[1])
                    existing_indices.append(idx)
                except:
                    pass

    start_index = max(existing_indices, default=-1) + 1

    generated = start_index
    attempts = 0
    max_attempts = NUM_IMAGES * 12

    print(f"Generating {NUM_IMAGES} images...")

    target_total = NUM_IMAGES

    while generated < target_total and attempts < max_attempts:
        attempts += 1

        image_rgb, annotations = render_monitor_scene(random_phrase())
        if not annotations and not ALLOW_EMPTY_LABELS:
            continue

        split = choose_split()
        stem = f"img_{generated:06d}"

        img_path = OUTPUT_DIR / "images" / split / f"{stem}.jpg"
        label_path = OUTPUT_DIR / "labels" / split / f"{stem}.txt"

        cv2.imwrite(str(img_path), cv2.cvtColor(image_rgb, cv2.COLOR_RGB2BGR))
        write_label_file(label_path, annotations)

        if DEBUG:
            save_debug_image(
                image_rgb, annotations, OUTPUT_DIR / "debug" / split / f"{stem}.jpg"
            )

        generated += 1
        if generated % 250 == 0:
            print(f"Generated {generated}/{NUM_IMAGES}")

    print("\nDone.")
    print(f"Dataset saved to: {OUTPUT_DIR.resolve()}")
    print(f"Generated images: {generated}/{NUM_IMAGES}")
    if generated < NUM_IMAGES:
        print(
            "Warning: stopped early because too many samples had no visible characters."
        )


if __name__ == "__main__":
    main()

