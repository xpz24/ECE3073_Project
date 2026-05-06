import serial
from pathlib import Path
from PIL import Image

PORT = "COM3"        # change this
BAUD = 921600        # must match Serial.begin(...)
W = 320
H = 240
FB_SIZE = W * H // 2

OUT_DIR = Path("captured_frames")
ORIG_DIR = OUT_DIR / "original_jpeg"
FB_DIR = OUT_DIR / "framebuffer_jpeg"

ORIG_DIR.mkdir(parents=True, exist_ok=True)
FB_DIR.mkdir(parents=True, exist_ok=True)

ser = serial.Serial(PORT, BAUD, timeout=10)

jpeg_counter = 0
fb_counter = 0

def fb4_to_gray8_image(fb: bytes) -> Image.Image:
    pixels = []

    for byte in fb:
        high = (byte >> 4) & 0x0F
        low = byte & 0x0F

        pixels.append(high * 17)
        pixels.append(low * 17)

    img = Image.new("L", (W, H))
    img.putdata(pixels[: W * H])
    return img

print("Listening...")

while True:
    line = ser.readline().decode(errors="ignore").strip()

    if line == "BEGIN_JPEG":
        size = int(ser.readline().decode(errors="ignore").strip())
        jpg_data = ser.read(size)

        end_line = ser.readline().decode(errors="ignore").strip()

        path = ORIG_DIR / f"orig_{jpeg_counter:05d}.jpg"
        path.write_bytes(jpg_data)

        print(f"Saved original JPEG: {path} ({size} bytes), end={end_line}")
        jpeg_counter += 1

    elif line == "BEGIN_FB4":
        size = int(ser.readline().decode(errors="ignore").strip())

        if size != FB_SIZE:
            print(f"Unexpected framebuffer size: {size}")
            ser.read(size)
            continue

        fb_data = ser.read(size)
        end_line = ser.readline().decode(errors="ignore").strip()

        img = fb4_to_gray8_image(fb_data)

        jpg_path = FB_DIR / f"fb_{fb_counter:05d}.jpg"
        png_path = FB_DIR / f"fb_{fb_counter:05d}.png"

        img.save(jpg_path, quality=95)
        img.save(png_path)

        print(f"Saved framebuffer image: {jpg_path} and {png_path}, end={end_line}")
        fb_counter += 1