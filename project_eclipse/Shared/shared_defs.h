#ifndef SHARED_DEF_H
#define SHARED_DEF_H

#include "system.h"

/* Shared payload in SDRAM. Layout:
 *   [0 .. ESP32_FB_SIZE - 1]                       framebuffer (image)
 *   [ESP32_FB_SIZE .. ESP32_PAYLOAD_SIZE - 1]      text buffer
 */
#define SHARED_FB_BASE_ADDR   SDRAM_BASE
#define SHARED_TEXT_BASE_ADDR (SHARED_FB_BASE_ADDR + ESP32_FB_SIZE)

/* ESP32 framebuffer format:
 * 208 x 208, RGB444 packed 12 bits per pixel.
 * Byte layout per 2 pixels (P0, P1):
 *   B0: [R0 G0]   R0 high nibble, G0 low nibble
 *   B1: [B0 R1]
 *   B2: [G1 B1]
 */
#define ESP32_FB_WIDTH        208
#define ESP32_FB_HEIGHT       208
#define ESP32_FB_BPP          12
#define ESP32_FB_SIZE         (ESP32_FB_WIDTH * ESP32_FB_HEIGHT * 3 / 2)   /* 64896 */

/* Text buffer: fixed-length, null-padded. Non-zero on the frame where a
 * sentence completes; all zeros otherwise.
 */
#define ESP32_TEXT_BUF_LEN    30

/* Full SPI payload size (must match ESP32 side) */
#define ESP32_PAYLOAD_SIZE    (ESP32_FB_SIZE + ESP32_TEXT_BUF_LEN)         /* 64926 */

/* VGA display format — leave at old dimensions for now; updated later. */
#define VGA_WIDTH             320
#define VGA_HEIGHT            240
#define VGA_PIXELS            (VGA_WIDTH * VGA_HEIGHT)

/* Hardware mutex owner IDs */
#define FRAME_SPI_OWNER_ID    0
#define FRAME_VGA_OWNER_ID    1

#endif
