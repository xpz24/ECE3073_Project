import subprocess
import urllib.request
from pathlib import Path
import ultralytics

ultralytics.checks()

# ============================================================
# CONFIG
# ============================================================

SCRIPT_DIR = Path(__file__).resolve().parent

DATASET = (
    SCRIPT_DIR / "Character_Detection_Swift-YOLO_192" / "dataset_8k_yolo" / "data.yaml"
)

MODEL = "yolov8n.pt"

IMG_SIZE = 192
EPOCHS = 100
BATCH = 256
DEVICE = 0
WORKERS = 4

RUN_NAME = "character_yolo_edge"

PROJECT_DIR = SCRIPT_DIR / "runs" / "detect"
RUN_DIR = PROJECT_DIR / RUN_NAME

BEST_MODEL = RUN_DIR / "weights" / "best.pt"

# Export / Vela
NO_POST = False  # set True if invoke fails because of YOLO postprocess ops

HIMAX_VELA_URL = (
    "https://raw.githubusercontent.com/HimaxWiseEyePlus/"
    "ML_FVP_EVALUATION/main/vela/himax_vela.ini"
)

VELA_CONFIG = SCRIPT_DIR / "himax_vela.ini"
VELA_OUTPUT_DIR = RUN_DIR / "weights" / "best_saved_model"

FULL_INT_MODEL = VELA_OUTPUT_DIR / "best_full_integer_quant.tflite"
VELA_MODEL = VELA_OUTPUT_DIR / "best_full_integer_quant_vela.tflite"


# ============================================================
# HELPERS
# ============================================================


def run(cmd):
    print("\nRunning:")
    print(" ".join(map(str, cmd)))

    subprocess.run(
        cmd,
        check=True,
        cwd=SCRIPT_DIR,
    )


def download_if_missing(url, dst):
    if dst.exists():
        print(f"\nUsing existing config:\n{dst}")
        return

    print(f"\nDownloading:\n{url}")
    print(f"To:\n{dst}")

    urllib.request.urlretrieve(url, dst)


# ============================================================
# VALIDATION
# ============================================================

if not DATASET.exists():
    raise FileNotFoundError(f"Missing data.yaml:\n{DATASET}")

PROJECT_DIR.mkdir(parents=True, exist_ok=True)


# ============================================================
# TRAIN
# ============================================================

run(
    [
        "yolo",
        "detect",
        "train",
        f"model={MODEL}",
        f"data={DATASET}",
        f"imgsz={IMG_SIZE}",
        f"epochs={EPOCHS}",
        f"batch={BATCH}",
        f"device={DEVICE}",
        f"workers={WORKERS}",
        "cache=disk",
        "amp=True",
        "plots=True",
        "exist_ok=True",
        f"name={RUN_NAME}",
        f"project={PROJECT_DIR}",
    ]
)


# ============================================================
# VERIFY BEST MODEL
# ============================================================

if not BEST_MODEL.exists():
    raise FileNotFoundError(
        "\nTraining completed but best.pt was not found.\n" f"Expected:\n{BEST_MODEL}\n"
    )


# ============================================================
# EXPORT FULL INTEGER TFLITE
# ============================================================

run(
    [
        "yolo",
        "export",
        f"model={BEST_MODEL}",
        "format=tflite",
        "int8=True",
        f"imgsz={IMG_SIZE}",
        f"data={DATASET}",
    ]
)


# ============================================================
# VERIFY FULL INTEGER MODEL
# ============================================================

if not FULL_INT_MODEL.exists():
    matches = list(BEST_MODEL.parent.rglob("*full_integer_quant*.tflite"))

    if matches:
        FULL_INT_MODEL = matches[0]
        VELA_OUTPUT_DIR = FULL_INT_MODEL.parent
        VELA_MODEL = VELA_OUTPUT_DIR / f"{FULL_INT_MODEL.stem}_vela.tflite"
    else:
        raise FileNotFoundError(
            "\nExport completed but full integer quant model was not found.\n"
            f"Expected:\n{FULL_INT_MODEL}\n"
        )


# ============================================================
# INSTALL / RUN VELA
# ============================================================

run(
    [
        "python",
        "-m",
        "pip",
        "install",
        "ethos-u-vela",
    ]
)

download_if_missing(HIMAX_VELA_URL, VELA_CONFIG)

run(
    [
        "vela",
        "--accelerator-config",
        "ethos-u55-64",
        # "--optimise",
        # "Size",
        "--config",
        str(VELA_CONFIG),
        "--system-config",
        "My_Sys_Cfg",
        "--memory-mode",
        "My_Mem_Mode_Parent",
        "--output-dir",
        str(VELA_OUTPUT_DIR),
        str(FULL_INT_MODEL),
    ]
)


# ============================================================
# DONE
# ============================================================

if not VELA_MODEL.exists():
    matches = list(VELA_OUTPUT_DIR.glob("*_vela.tflite"))

    if matches:
        VELA_MODEL = matches[0]
    else:
        raise FileNotFoundError(
            "\nVela finished but no *_vela.tflite model was found.\n"
            f"Checked:\n{VELA_OUTPUT_DIR}\n"
        )

size_mb = VELA_MODEL.stat().st_size / (1024 * 1024)

print("\nDone.")

print("\nBest PyTorch model:")
print(BEST_MODEL)

print("\nFull integer TFLite:")
print(FULL_INT_MODEL)

print("\nVela deploy model:")
print(VELA_MODEL)

print(f"\nVela model size: {size_mb:.2f} MB")

if size_mb <= 2.4:
    print("Model is within the 2.4 MB Grove Vision AI V2 limit.")
else:
    print("Model exceeds 2.4 MB. Try smaller imgsz or smaller model.")
