import argparse
import json
import shutil
from pathlib import Path
from PIL import Image

SPLITS = ["train", "valid", "test"]
IMG_EXTS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}


def safe_name(name: str) -> str:
    return str(name)


def find_images(folder: Path):
    return sorted([p for p in folder.rglob("*") if p.suffix.lower() in IMG_EXTS])


def coco_to_yolo(dataset_dir: Path, out_dir: Path):
    out_img = out_dir / "images"
    out_lbl = out_dir / "labels"

    class_names = None

    for split in SPLITS:
        split_dir = dataset_dir / split
        ann_path = split_dir / "_annotations.coco.json"

        if not ann_path.exists():
            print(f"Skipping {split}: no COCO json")
            continue

        with open(ann_path, "r", encoding="utf-8") as f:
            coco = json.load(f)

        categories = sorted(coco["categories"], key=lambda c: c["id"])
        id_to_idx = {cat["id"]: i for i, cat in enumerate(categories)}
        names = [safe_name(cat["name"]) for cat in categories]

        if class_names is None:
            class_names = names

        images = {img["id"]: img for img in coco["images"]}

        anns_by_img = {img_id: [] for img_id in images}
        for ann in coco.get("annotations", []):
            anns_by_img.setdefault(ann["image_id"], []).append(ann)

        (out_img / split).mkdir(parents=True, exist_ok=True)
        (out_lbl / split).mkdir(parents=True, exist_ok=True)

        for img_id, img in images.items():
            filename = img["file_name"]
            src = split_dir / filename

            if not src.exists():
                matches = list(split_dir.rglob(Path(filename).name))
                if not matches:
                    print(f"Missing image: {src}")
                    continue
                src = matches[0]

            dst = out_img / split / src.name
            shutil.copy2(src, dst)

            w = img.get("width")
            h = img.get("height")

            if not w or not h:
                with Image.open(src) as im:
                    w, h = im.size

            label_path = out_lbl / split / f"{src.stem}.txt"

            lines = []
            for ann in anns_by_img.get(img_id, []):
                x, y, bw, bh = ann["bbox"]

                cx = (x + bw / 2) / w
                cy = (y + bh / 2) / h
                nw = bw / w
                nh = bh / h

                cls = id_to_idx[ann["category_id"]]

                lines.append(f"{cls} {cx:.6f} {cy:.6f} {nw:.6f} {nh:.6f}")

            # Empty file is correct for negative/background images.
            label_path.write_text("\n".join(lines), encoding="utf-8")

    if class_names is None:
        raise RuntimeError("No COCO annotations found.")

    yaml_lines = [
        f"path: {out_dir.resolve().as_posix()}",
        "train: images/train",
        "val: images/valid",
        "test: images/test",
        "",
        "names:",
    ]

    for i, name in enumerate(class_names):
        yaml_lines.append(f"  {i}: '{name}'")

    (out_dir / "data.yaml").write_text("\n".join(yaml_lines), encoding="utf-8")

    print(f"YOLO dataset written to: {out_dir}")


def yolo_to_coco(dataset_dir: Path, out_dir: Path):
    yaml_path = dataset_dir / "data.yaml"

    if not yaml_path.exists():
        raise FileNotFoundError("data.yaml not found. Need class names.")

    names = []
    in_names = False

    for line in yaml_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()

        if stripped == "names:":
            in_names = True
            continue

        if in_names and ":" in stripped:
            _, value = stripped.split(":", 1)
            names.append(value.strip().strip("'").strip('"'))

    if not names:
        raise RuntimeError("Could not read names from data.yaml.")

    categories = [
        {"id": i + 1, "name": name, "supercategory": "object"}
        for i, name in enumerate(names)
    ]

    for split in SPLITS:
        img_dir = dataset_dir / "images" / split
        lbl_dir = dataset_dir / "labels" / split

        if not img_dir.exists():
            print(f"Skipping {split}: no images folder")
            continue

        out_split = out_dir / split
        out_split.mkdir(parents=True, exist_ok=True)

        coco = {
            "images": [],
            "annotations": [],
            "categories": categories,
        }

        image_id = 1
        ann_id = 1

        for img_path in find_images(img_dir):
            shutil.copy2(img_path, out_split / img_path.name)

            with Image.open(img_path) as im:
                w, h = im.size

            coco["images"].append(
                {
                    "id": image_id,
                    "file_name": img_path.name,
                    "width": w,
                    "height": h,
                }
            )

            label_path = lbl_dir / f"{img_path.stem}.txt"

            if label_path.exists():
                for line in label_path.read_text(encoding="utf-8").splitlines():
                    if not line.strip():
                        continue

                    parts = line.split()
                    cls = int(parts[0])
                    cx, cy, nw, nh = map(float, parts[1:5])

                    bw = nw * w
                    bh = nh * h
                    x = cx * w - bw / 2
                    y = cy * h - bh / 2

                    coco["annotations"].append(
                        {
                            "id": ann_id,
                            "image_id": image_id,
                            "category_id": cls + 1,
                            "bbox": [x, y, bw, bh],
                            "area": bw * bh,
                            "iscrowd": 0,
                            "segmentation": [],
                        }
                    )
                    ann_id += 1

            # Missing/empty txt means negative image. Correct. Civilization survives.

            image_id += 1

        with open(out_split / "_annotations.coco.json", "w", encoding="utf-8") as f:
            json.dump(coco, f, indent=2)

    print(f"COCO dataset written to: {out_dir}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=["coco2yolo", "yolo2coco"])
    parser.add_argument("dataset_dir")
    parser.add_argument("--out", default=None)

    args = parser.parse_args()

    dataset_dir = Path(args.dataset_dir).resolve()

    if args.out:
        out_dir = Path(args.out).resolve()
    else:
        suffix = "_yolo" if args.mode == "coco2yolo" else "_coco"
        out_dir = dataset_dir.parent / f"{dataset_dir.name}{suffix}"

    if args.mode == "coco2yolo":
        coco_to_yolo(dataset_dir, out_dir)
    else:
        yolo_to_coco(dataset_dir, out_dir)


if __name__ == "__main__":
    main()