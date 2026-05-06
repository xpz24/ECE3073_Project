#include "vga_controller.h"
#include "altera_avalon_mutex.h"
#include "sys/alt_stdio.h"
#include "shared_defs.h"
#include <unistd.h>

static alt_mutex_dev *frame_mutex = NULL;

void init_vga_mutex(void) {
	frame_mutex = altera_avalon_mutex_open(MUTEX_NAME);

	if (frame_mutex == NULL) {
		alt_putstr("[VGA] Mutex open failed\n");
	} else {
		alt_putstr("[VGA] Mutex opened\n");
	}
}

void draw_esp32_framebuffer(void) {

	volatile alt_u8 *shared_fb = (volatile alt_u8 *) SHARED_FB_BASE_ADDR;

	if (frame_mutex != NULL) {
		altera_avalon_mutex_lock(frame_mutex,
		FRAME_VGA_OWNER_ID);
	}

	for (int y = 0; y < VGA_HEIGHT; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {

			unsigned int pixel_index = y * VGA_WIDTH + x;
			unsigned int byte_index = pixel_index >> 1;

			alt_u8 packed = shared_fb[byte_index];
			alt_u8 color;

			if (pixel_index & 1) {
				color = packed & 0x0F;
			} else {
				color = (packed >> 4) & 0x0F;
			}

			write_pixel(pixel_index, color);
		}
	}

	present_frame();

	if (frame_mutex != NULL) {
		altera_avalon_mutex_unlock(frame_mutex);
	}


}


void present_frame(void) {
    IOWR_ALTERA_AVALON_PIO_DATA(FRAME_READY_BASE, 1);

    while (IORD_ALTERA_AVALON_PIO_DATA(SWAP_DONE_BASE) == 0) {
        // wait for swap
    }

    IOWR_ALTERA_AVALON_PIO_DATA(FRAME_READY_BASE, 0);

    while (IORD_ALTERA_AVALON_PIO_DATA(SWAP_DONE_BASE) != 0) {
        // wait for hardware to clear ack
    }
}

/* Write one 4-bit pixel to the pixel buffer */
void write_pixel(unsigned int addr, unsigned char color) {
	if (addr >= VGA_PIXELS)
		return;
	IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_ADDR_BASE, addr);
	IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_DATA_BASE, color & 0x0F);
	IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 1);
	IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 0);
}

/* Horizontal color stripes: color changes with x */
void draw_horizontal_stripes(void) {
	for (int y = 0; y < VGA_HEIGHT; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			unsigned int addr = (y * VGA_WIDTH) + x;
			unsigned char color = (x / 20) & 0x0F;
			write_pixel(addr, color);
		}
	}
	present_frame();
}

/* Vertical color stripes: color changes with y */
void draw_vertical_stripes(void) {
	for (int y = 0; y < VGA_HEIGHT; y++) {
		unsigned char color = (y / 15) & 0x0F;
		for (int x = 0; x < VGA_WIDTH; x++) {
			unsigned int addr = (y * VGA_WIDTH) + x;
			write_pixel(addr, color);
		}
	}
	present_frame();
}

