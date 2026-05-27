import serial
from pathlib import Path

PORT = "COM6"      # change this
BAUD = 115200

SAVE_DIR = Path("captured_base64")
SAVE_DIR.mkdir(exist_ok=True)

ser = serial.Serial(PORT, BAUD, timeout=2)

buffer = []
capturing = False
image_num = 0

print("Listening...")

while True:

    line = ser.readline().decode(
        "utf-8",
        errors="ignore"
    ).strip()

    if line == "BEGIN_IMAGE":

        buffer = []
        capturing = True

    elif line == "END_IMAGE" and capturing:

        capturing = False

        filename = SAVE_DIR / f"img{image_num}.txt"

        with open(filename, "w") as f:
            f.write("".join(buffer))

        print("Saved", filename)

        image_num += 1

    elif capturing:

        buffer.append(line)