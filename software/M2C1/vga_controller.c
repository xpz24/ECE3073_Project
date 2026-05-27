#include "vga_controller.h"
#include "altera_avalon_mutex.h"
#include "sys/alt_stdio.h"
#include "shared_defs.h"
#include <unistd.h>

static alt_mutex_dev *frame_mutex = NULL;

void init_vga_mutex(void) {
    frame_mutex = altera_avalon_mutex_open(MUTEX_NAME);
    if (frame_mutex == NULL) alt_putstr("[VGA] Mutex open failed\n");
    else                     alt_putstr("[VGA] Mutex opened\n");
}

/* Unpack 3 bytes of packed RGB444 into two 12-bit RGB444 words.
 * ESP32 natural packing:
 *   b0: [R0 G0]   (R0 high nibble, G0 low nibble)
 *   b1: [B0 R1]
 *   b2: [G1 B1]
 * Output 12-bit word format expected by RAM_VGA / vga_controller:
 *   bits[11:8] = R, [7:4] = G, [3:0] = B
 */
static inline void unpack_pair(alt_u8 b0, alt_u8 b1, alt_u8 b2,
                               alt_u16 *p0, alt_u16 *p1) {
    alt_u8 r0 = (b0 >> 4) & 0x0F;
    alt_u8 g0 =  b0       & 0x0F;
    alt_u8 b0n = (b1 >> 4) & 0x0F;
    alt_u8 r1 =  b1       & 0x0F;
    alt_u8 g1 = (b2 >> 4) & 0x0F;
    alt_u8 b1n =  b2       & 0x0F;
    *p0 = ((alt_u16)r0  << 8) | ((alt_u16)g0  << 4) | b0n;
    *p1 = ((alt_u16)r1  << 8) | ((alt_u16)g1  << 4) | b1n;
}

/* Read 208x208 packed RGB444 from SDRAM, unpack, and place at the top-left
 * of the 320x240 VGA framebuffer. Pixels outside the 208x208 region are
 * untouched — they keep whatever junk is in RAM_VGA from the previous swap
 * (or random startup garbage). That's fine per the agreed spec.
 */
void draw_esp32_framebuffer(void) {
    volatile alt_u8 *shared_fb = (volatile alt_u8 *) SHARED_FB_BASE_ADDR;

    if (frame_mutex != NULL) {
        altera_avalon_mutex_lock(frame_mutex, FRAME_VGA_OWNER_ID);
    }

    /* 208 wide is even, so every row is exactly 208/2 = 104 byte-triplets,
     * = 312 bytes per row, no straggler pixel.
     */
    for (int y = 0; y < 208; y++) {
        const volatile alt_u8 *row = shared_fb + (y * 208 * 3 / 2);  /* 312 */
        unsigned int fb_row_offset = y * VGA_WIDTH;                  /* 320 */
        for (int x = 0; x < 208; x += 2) {
            alt_u8 b0 = row[(x >> 1) * 3 + 0];
            alt_u8 b1 = row[(x >> 1) * 3 + 1];
            alt_u8 b2 = row[(x >> 1) * 3 + 2];
            alt_u16 p0, p1;
            unpack_pair(b0, b1, b2, &p0, &p1);
            write_pixel(fb_row_offset + x,     p0);
            write_pixel(fb_row_offset + x + 1, p1);
        }
    }

    present_frame();

    if (frame_mutex != NULL) {
        altera_avalon_mutex_unlock(frame_mutex);
    }
}

void present_frame(void) {
    IOWR_ALTERA_AVALON_PIO_DATA(FRAME_READY_BASE, 1);
    while (IORD_ALTERA_AVALON_PIO_DATA(SWAP_DONE_BASE) == 0) { }
    IOWR_ALTERA_AVALON_PIO_DATA(FRAME_READY_BASE, 0);
    while (IORD_ALTERA_AVALON_PIO_DATA(SWAP_DONE_BASE) != 0) { }
}

/* Write one 12-bit RGB444 pixel. PIO_PIXEL_DATA must be widened to 12 bits. */
void write_pixel(unsigned int addr, unsigned int color) {
    if (addr >= VGA_PIXELS) return;
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_ADDR_BASE, addr);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_DATA_BASE, color & 0xFFF);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 1);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 0);
}

/* Test patterns updated for RGB444. Useful for smoke-testing the pipeline. */
void draw_horizontal_stripes(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            unsigned int addr  = (y * VGA_WIDTH) + x;
            unsigned int shade = (x / 20) & 0x0F;
            unsigned int color = (shade << 8) | (shade << 4) | shade;
            write_pixel(addr, color);
        }
    }
    present_frame();
}

void draw_vertical_stripes(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        unsigned int shade = (y / 15) & 0x0F;
        unsigned int color = (shade << 8) | (shade << 4) | shade;
        for (int x = 0; x < VGA_WIDTH; x++) {
            unsigned int addr = (y * VGA_WIDTH) + x;
            write_pixel(addr, color);
        }
    }
    present_frame();
}
