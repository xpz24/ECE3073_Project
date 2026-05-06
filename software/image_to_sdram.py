#!/usr/bin/env python3
"""
Image to SDRAM Framebuffer Converter
======================================
Converts any image to a 320x240 grayscale 4-bit C source file.
The array is marked for placement at a fixed SDRAM address via a linker section.

Usage:
    python image_to_sdram.py <input_image> [output.c]

Example:
    python image_to_sdram.py baby.jpg image_data.c

Requirements:
    pip install Pillow numpy
"""

import sys
import os
from PIL import Image
import numpy as np

TARGET_WIDTH  = 320
TARGET_HEIGHT = 240
FB_SIZE       = TARGET_WIDTH * TARGET_HEIGHT   # 76800

# Core 2 SDRAM region: 0x200000 to 0x3FFFFF (2MB)
# Place framebuffer at the top to avoid collision with code/heap
SDRAM_BASE    = 0x200000
SDRAM_LENGTH  = 0x200000   # 2MB
FB_BASE_ADDR  = SDRAM_BASE + SDRAM_LENGTH - FB_SIZE  # 0x3ED000

def convert_image(input_path, output_path=None):
    if output_path is None:
        base = os.path.splitext(os.path.basename(input_path))[0]
        output_path = base + "_sdram.c"

    print(f"Loading image: {input_path}")
    img = Image.open(input_path)
    img = img.convert("L")
    print(f"Original size: {img.width}x{img.height}")

    img = img.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.LANCZOS)
    print(f"Resized to: {TARGET_WIDTH}x{TARGET_HEIGHT}")

    pixels = np.array(img, dtype=np.uint8)
    pixels_4bit = (pixels >> 4).astype(np.uint8)
    flat = pixels_4bit.flatten()

    print(f"Total pixels: {FB_SIZE}")
    print(f"Value range:  {flat.min()} - {flat.max()}")
    print(f"SDRAM address: 0x{FB_BASE_ADDR:08X}")

    array_name = os.path.splitext(os.path.basename(output_path))[0].replace("-","_").replace(" ","_")

    # Write .c file — array placed in custom linker section ".sdram_framebuffer"
    with open(output_path, "w") as f:
        f.write("/*\n")
        f.write(f" * SDRAM Framebuffer Image Data\n")
        f.write(f" * Source     : {os.path.basename(input_path)}\n")
        f.write(f" * Resolution : {TARGET_WIDTH}x{TARGET_HEIGHT} (displayed 640x480 2x scaled)\n")
        f.write(f" * Format     : Grayscale 4-bit (0-15), stored as uint8_t\n")
        f.write(f" * SDRAM addr : 0x{FB_BASE_ADDR:08X}\n")
        f.write(f" * Size       : {FB_SIZE} bytes\n")
        f.write(" */\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write("/* Place this array at a fixed SDRAM address via linker section */\n")
        f.write(f'__attribute__((section(".sdram_framebuffer")))\n')
        f.write(f"const uint8_t {array_name}[{FB_SIZE}] = {{\n")

        for i in range(0, FB_SIZE, 16):
            chunk = flat[i:i+16]
            line = ", ".join(f"0x{v:01X}" for v in chunk)
            if i + 16 < FB_SIZE:
                f.write(f"    {line},\n")
            else:
                f.write(f"    {line}\n")

        f.write("};\n")

    # Write companion header
    header_path = os.path.splitext(output_path)[0] + ".h"
    with open(header_path, "w") as f:
        guard = os.path.basename(header_path).replace(".","_").upper()
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define FB_WIDTH       {TARGET_WIDTH}\n")
        f.write(f"#define FB_HEIGHT      {TARGET_HEIGHT}\n")
        f.write(f"#define FB_SIZE        {FB_SIZE}\n")
        f.write(f"#define FB_SDRAM_ADDR  0x{FB_BASE_ADDR:08X}UL\n\n")
        f.write(f"extern const uint8_t {array_name}[{FB_SIZE}];\n\n")
        f.write(f"#endif /* {guard} */\n")

    print(f"\nDone!")
    print(f"  C file  : {output_path}")
    print(f"  Header  : {header_path}")
    print(f"\nAdd both files to your NIOS2 Eclipse project (M2_core2 folder).")
    print(f"The array will be placed at SDRAM address 0x{FB_BASE_ADDR:08X}.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python image_to_sdram.py <input_image> [output.c]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(input_path):
        print(f"Error: File not found: {input_path}")
        sys.exit(1)

    convert_image(input_path, output_path)