#!/usr/bin/env python3
"""
Image to VGA Framebuffer Array Converter
=========================================
Converts any image to a 320x240 RGB444 12-bit C array
for use with the NIOS2 VGA pixel buffer (RAM2Port).

Each pixel is stored as uint16_t with format: 0x0RGB
  bits [11:8] = R (4-bit)
  bits [7:4]  = G (4-bit)
  bits [3:0]  = B (4-bit)

Usage:
    python image_to_vga_array.py <input_image> [output_header]

Example:
    python image_to_vga_array.py photo.jpg image_data.h

Requirements:
    pip install Pillow numpy
"""

import sys
import os
from PIL import Image
import numpy as np

TARGET_WIDTH  = 320
TARGET_HEIGHT = 240

def convert_image(input_path, output_path=None):
    if output_path is None:
        base = os.path.splitext(os.path.basename(input_path))[0]
        output_path = base + "_vga.h"

    print(f"Loading image: {input_path}")
    img = Image.open(input_path)

    # Convert to RGB (drop alpha if present)
    img = img.convert("RGB")
    print(f"Original size: {img.width}x{img.height}")

    # Resize to 320x240
    img = img.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.LANCZOS)
    print(f"Resized to: {TARGET_WIDTH}x{TARGET_HEIGHT}")

    # Convert to numpy array (H x W x 3), values 0-255
    pixels = np.array(img, dtype=np.uint8)

    # Quantize each channel from 8-bit to 4-bit (0-15)
    r4 = (pixels[:, :, 0] >> 4).astype(np.uint16)  # bits [11:8]
    g4 = (pixels[:, :, 1] >> 4).astype(np.uint16)  # bits [7:4]
    b4 = (pixels[:, :, 2] >> 4).astype(np.uint16)  # bits [3:0]

    # Pack into 12-bit RGB444 stored in uint16_t: 0x0RGB
    rgb444 = ((r4 << 8) | (g4 << 4) | b4).astype(np.uint16)

    # Flatten to 1D row-major
    flat = rgb444.flatten()
    total_pixels = TARGET_WIDTH * TARGET_HEIGHT  # 76800
    total_bytes  = total_pixels * 2              # 153600 bytes (2 bytes per pixel)

    print(f"Total pixels : {total_pixels}")
    print(f"Total bytes  : {total_bytes} (2 bytes per pixel, uint16_t)")

    array_name = os.path.splitext(os.path.basename(output_path))[0].replace("-", "_").replace(" ", "_")

    with open(output_path, "w") as f:
        f.write("/*\n")
        f.write(f" * VGA Framebuffer Image Data\n")
        f.write(f" * Source     : {os.path.basename(input_path)}\n")
        f.write(f" * Resolution : {TARGET_WIDTH}x{TARGET_HEIGHT} (display: 640x480 2x scaled)\n")
        f.write(f" * Format     : RGB444 12-bit packed in uint16_t (0x0RGB)\n")
        f.write(f" *               bits [11:8] = R, [7:4] = G, [3:0] = B\n")
        f.write(f" * Pixels     : {total_pixels}\n")
        f.write(f" * Total bytes: {total_bytes}\n")
        f.write(" */\n\n")
        f.write("#ifndef IMAGE_DATA_H\n")
        f.write("#define IMAGE_DATA_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define IMG_WIDTH  {TARGET_WIDTH}\n")
        f.write(f"#define IMG_HEIGHT {TARGET_HEIGHT}\n")
        f.write(f"#define IMG_SIZE   {total_pixels}  /* pixels */\n")
        f.write(f"#define IMG_BYTES  {total_bytes}   /* bytes  */\n\n")
        f.write(f"static const uint16_t {array_name}[{total_pixels}] = {{\n")

        # Write 12 values per line for readability
        for i in range(0, total_pixels, 12):
            chunk = flat[i:i+12]
            line = ", ".join(f"0x{v:03X}" for v in chunk)
            if i + 12 < total_pixels:
                f.write(f"    {line},\n")
            else:
                f.write(f"    {line}\n")

        f.write("};\n\n")
        f.write("#endif /* IMAGE_DATA_H */\n")

    print(f"\nDone! Header written to: {output_path}")
    print(f"Array: {array_name}[{total_pixels}] (uint16_t)")

    return output_path, array_name, total_pixels


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python image_to_vga_array.py <input_image> [output_header.h]")
        print("Example: python image_to_vga_array.py photo.jpg image_data.h")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(input_path):
        print(f"Error: File not found: {input_path}")
        sys.exit(1)

    convert_image(input_path, output_path)