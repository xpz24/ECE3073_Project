#ifndef SHARED_DEF_H
#define SHARED_DEF_H

#include "system.h"

/* Shared framebuffer in SDRAM */
#define SHARED_FB_BASE_ADDR   SDRAM_BASE

/* ESP32 framebuffer format:
 * 320 x 240, 4 bits per pixel, packed 2 pixels per byte
 */
#define ESP32_FB_WIDTH        320
#define ESP32_FB_HEIGHT       240
#define ESP32_FB_BPP          4
#define ESP32_FB_SIZE         (ESP32_FB_WIDTH * ESP32_FB_HEIGHT / 2)

/* VGA display format */
#define VGA_WIDTH             ESP32_FB_WIDTH
#define VGA_HEIGHT            ESP32_FB_HEIGHT
#define VGA_PIXELS            (VGA_WIDTH * VGA_HEIGHT)

/* Hardware mutex owner IDs */
#define FRAME_SPI_OWNER_ID    0
#define FRAME_VGA_OWNER_ID    1

#endif
