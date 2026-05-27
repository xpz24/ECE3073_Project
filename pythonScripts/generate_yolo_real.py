import zipfile
import random
import shutil
import cv2
import numpy as np
from pathlib import Path

# =========================
# CONFIG
# =========================

INPUT_ZIP_DIR = Path("input_zips")
OUTPUT_DIR = Path("yolo_dataset_real")

SIZE = 192
DEBUG = False

SPLITS = {
    "train": 0.7,
    "val": 0.2,
    "test": 0.1,
}

IMAGE_EXTS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}

CLASSES = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ") + [str(i) for i in range(10)]

CLASS_TO_ID = {
    cls: idx for idx, cls in enumerate(CLASSES)
}

RANDOM_SEED = 42


# =========================
# HELPERS
# =========================

def zip_name_to_class(zip_path: Path) -> str:
    """
    A.zip -> A
    n0.zip -> 0
    n9.zip -> 9
    """
    name = zip_path.stem

    if len(name) == 1 and name.isalpha():
        return name.upper()

    if len(name) == 2 and name[0].lower() == "n" and name[1].isdigit():
        return name[1]

    raise ValueError(f"Cannot infer class from zip name: {zip_path.name}")


def detect_character_bbox(img_bgr):
    """
    Finds bright white character on darker screen.
    Avoids bright background objects by focusing near the central screen area.
    """

    gray = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2GRAY)
    h, w = gray.shape

    # Ignore outer border/background.
    # Character is expected on the monitor, usually near centre.
    roi_x1 = int(w * 0.05)
    roi_y1 = int(h * 0.20)
    roi_x2 = int(w * 0.95)
    roi_y2 = int(h * 0.85)

    roi = gray[roi_y1:roi_y2, roi_x1:roi_x2]

    blur = cv2.GaussianBlur(roi, (3, 3), 0)

    # Strict bright threshold for white character
    _, mask = cv2.threshold(
        blur,
        180,
        255,
        cv2.THRESH_BINARY
    )

    kernel = np.ones((3, 3), np.uint8)

    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    contours, _ = cv2.findContours(
        mask,
        cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE
    )

    candidates = []

    for c in contours:
        x, y, bw, bh = cv2.boundingRect(c)
        area = bw * bh

        if area < 30:
            continue

        # Character should not be absurdly huge
        if area > 0.15 * w * h:
            continue

        aspect = bw / max(bh, 1)

        # Characters are usually not extremely flat/wide
        if aspect < 0.15 or aspect > 2.5:
            continue

        # Convert ROI coords back to image coords
        gx = x + roi_x1
        gy = y + roi_y1

        # Prefer objects closer to image centre
        cx = gx + bw / 2
        cy = gy + bh / 2

        dist_to_center = ((cx - w / 2) ** 2 + (cy - h / 2) ** 2) ** 0.5

        score = area - dist_to_center * 2.0

        candidates.append((score, gx, gy, bw, bh))

    if not candidates:
        return None

    _, x, y, bw, bh = max(candidates, key=lambda item: item[0])

    pad = 3

    x1 = max(0, x - pad)
    y1 = max(0, y - pad)
    x2 = min(w - 1, x + bw + pad)
    y2 = min(h - 1, y + bh + pad)

    return x1, y1, x2, y2


def resize_bbox(bbox, old_w, old_h, new_size):
    x1, y1, x2, y2 = bbox

    sx = new_size / old_w
    sy = new_size / old_h

    return (
        x1 * sx,
        y1 * sy,
        x2 * sx,
        y2 * sy,
    )


def bbox_to_yolo(bbox, img_w, img_h):
    x1, y1, x2, y2 = bbox

    cx = ((x1 + x2) / 2) / img_w
    cy = ((y1 + y2) / 2) / img_h
    bw = (x2 - x1) / img_w
    bh = (y2 - y1) / img_h

    return cx, cy, bw, bh


def make_dirs():
    for split in SPLITS:
        (OUTPUT_DIR / "images" / split).mkdir(parents=True, exist_ok=True)
        (OUTPUT_DIR / "labels" / split).mkdir(parents=True, exist_ok=True)

        if DEBUG:
            (OUTPUT_DIR / "debug" / split).mkdir(parents=True, exist_ok=True)


def choose_splits(items):

    random.shuffle(items)

    n = len(items)

    n_train = max(1, round(n * SPLITS["train"]))
    n_val = max(1, round(n * SPLITS["val"]))

    n_test = n - n_train - n_val

    # Ensure test has at least 2 if possible
    if n >= 4 and n_test == 1 and n_train > 1:
        n_train -= 1
        n_test += 1

    # Safety correction
    while n_train + n_val + n_test > n:
        if n_train > n_val:
            n_train -= 1
        else:
            n_val -= 1

    return {
        "train": items[:n_train],
        "val": items[n_train:n_train+n_val],
        "test": items[n_train+n_val:],
    }


def write_data_yaml():
    yaml_path = OUTPUT_DIR / "data.yaml"

    names = "\n".join(
        f"  {idx}: {name}" for idx, name in enumerate(CLASSES)
    )

    content = f"""path: {OUTPUT_DIR.resolve()}
train: images/train
val: images/val
test: images/test

names:
{names}
"""

    yaml_path.write_text(content, encoding="utf-8")


# =========================
# MAIN
# =========================

def main():
    random.seed(RANDOM_SEED)

    if OUTPUT_DIR.exists():
        shutil.rmtree(OUTPUT_DIR)

    make_dirs()

    zip_files = sorted(INPUT_ZIP_DIR.glob("*.zip"))

    if not zip_files:
        raise FileNotFoundError(f"No zip files found in {INPUT_ZIP_DIR}")

    total_images = 0
    skipped = 0

    for zip_path in zip_files:
        cls = zip_name_to_class(zip_path)
        class_id = CLASS_TO_ID[cls]

        extracted_images = []

        with zipfile.ZipFile(zip_path, "r") as z:
            image_names = [
                name for name in z.namelist()
                if Path(name).suffix.lower() in IMAGE_EXTS
            ]

            for image_name in image_names:
                data = z.read(image_name)
                arr = np.frombuffer(data, np.uint8)
                img = cv2.imdecode(arr, cv2.IMREAD_COLOR)

                if img is None:
                    skipped += 1
                    continue

                extracted_images.append((image_name, img))

        split_items = choose_splits(extracted_images)

        for split, items in split_items.items():
            for image_name, img in items:
                old_h, old_w = img.shape[:2]

                bbox = detect_character_bbox(img)

                if bbox is None:
                    print(f"Skipped, no bbox found: {zip_path.name}/{image_name}")
                    skipped += 1
                    continue

                resized = cv2.resize(
                    img,
                    (SIZE, SIZE),
                    interpolation=cv2.INTER_AREA
                )

                resized_bbox = resize_bbox(
                    bbox,
                    old_w,
                    old_h,
                    SIZE
                )

                yolo_bbox = bbox_to_yolo(
                    resized_bbox,
                    SIZE,
                    SIZE
                )

                safe_stem = Path(image_name).stem.replace(" ", "_")
                out_name = f"{cls}_{safe_stem}.jpg"

                image_out = OUTPUT_DIR / "images" / split / out_name
                label_out = OUTPUT_DIR / "labels" / split / f"{Path(out_name).stem}.txt"

                cv2.imwrite(str(image_out), resized)

                label_line = (
                    f"{class_id} "
                    f"{yolo_bbox[0]:.6f} "
                    f"{yolo_bbox[1]:.6f} "
                    f"{yolo_bbox[2]:.6f} "
                    f"{yolo_bbox[3]:.6f}\n"
                )

                label_out.write_text(label_line, encoding="utf-8")

                if DEBUG:
                    debug_img = resized.copy()

                    x1, y1, x2, y2 = map(int, resized_bbox)

                    cv2.rectangle(
                        debug_img,
                        (x1, y1),
                        (x2, y2),
                        (0, 255, 0),
                        2
                    )

                    cv2.putText(
                        debug_img,
                        cls,
                        (x1, max(15, y1 - 5)),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.6,
                        (0, 255, 0),
                        2,
                        cv2.LINE_AA
                    )

                    debug_out = OUTPUT_DIR / "debug" / split / out_name
                    cv2.imwrite(str(debug_out), debug_img)

                total_images += 1

    write_data_yaml()

    print("Done.")
    print(f"Created images: {total_images}")
    print(f"Skipped images: {skipped}")
    print(f"Dataset path: {OUTPUT_DIR.resolve()}")


if __name__ == "__main__":
    main()