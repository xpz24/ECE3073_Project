#!/usr/bin/env python3
"""
Image to VGA Framebuffer Array Converter
=========================================
Converts any image to a 320x240 grayscale 4-bit C array
for use with the NIOS2 VGA pixel buffer (RAM2Port).

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
OUTPUT_NAME   = "image_data.h"

def convert_image(input_path, output_path=None):
    if output_path is None:
        base = os.path.splitext(os.path.basename(input_path))[0]
        output_path = base + "_vga.h"

    print(f"Loading image: {input_path}")
    img = Image.open(input_path)

    # Convert to grayscale
    img = img.convert("L")
    print(f"Original size: {img.width}x{img.height}")

    # Resize to 320x240 using high-quality downsampling
    img = img.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.LANCZOS)
    print(f"Resized to: {TARGET_WIDTH}x{TARGET_HEIGHT}")

    # Convert to numpy array, quantize to 4-bit (0-15)
    pixels = np.array(img, dtype=np.uint8)
    pixels_4bit = (pixels >> 4).astype(np.uint8)  # 8-bit -> 4-bit (0-15)

    # Flatten to 1D array (row-major, left-to-right, top-to-bottom)
    flat = pixels_4bit.flatten()
    total_pixels = TARGET_WIDTH * TARGET_HEIGHT  # 76800

    print(f"Total pixels: {total_pixels}")
    print(f"Value range: {flat.min()} - {flat.max()}")

    # Write C header file
    array_name = os.path.splitext(os.path.basename(output_path))[0].replace("-", "_").replace(" ", "_")

    with open(output_path, "w") as f:
        f.write("/*\n")
        f.write(f" * VGA Framebuffer Image Data\n")
        f.write(f" * Source: {os.path.basename(input_path)}\n")
        f.write(f" * Resolution: {TARGET_WIDTH}x{TARGET_HEIGHT} (display: 640x480 2x scaled)\n")
        f.write(f" * Format: Grayscale 4-bit (0-15), stored as uint8_t\n")
        f.write(f" * Total size: {total_pixels} bytes\n")
        f.write(f" * Usage: Write pixel[i] to PIO_DATA at address i\n")
        f.write(" */\n\n")
        f.write("#ifndef IMAGE_DATA_H\n")
        f.write("#define IMAGE_DATA_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define IMG_WIDTH  {TARGET_WIDTH}\n")
        f.write(f"#define IMG_HEIGHT {TARGET_HEIGHT}\n")
        f.write(f"#define IMG_SIZE   {total_pixels}\n\n")
        f.write(f"static const uint8_t {array_name}[{total_pixels}] = {{\n")

        # Write 16 values per line for readability
        for i in range(0, total_pixels, 16):
            chunk = flat[i:i+16]
            line = ", ".join(f"0x{v:01X}" for v in chunk)
            if i + 16 < total_pixels:
                f.write(f"    {line},\n")
            else:
                f.write(f"    {line}\n")

        f.write("};\n\n")
        f.write("#endif /* IMAGE_DATA_H */\n")

    print(f"\nDone! Header written to: {output_path}")
    print(f"Array name: {array_name}[{total_pixels}]")
    print(f"\nIn your NIOS2 C code, #include \"{output_path}\" and use:")
    print(f"  write_framebuffer({array_name}, IMG_SIZE);")

    return output_path, array_name, total_pixels


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python image_to_vga_array.py <input_image> [output_header.h]")
        print("Example: python image_to_vga_array.py photo.jpg")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(input_path):
        print(f"Error: File not found: {input_path}")
        sys.exit(1)

    convert_image(input_path, output_path)