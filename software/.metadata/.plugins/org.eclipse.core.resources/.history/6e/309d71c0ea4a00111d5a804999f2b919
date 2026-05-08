// libraries
#include <stdio.h>
#include <string.h>
#include "../M2_core2_bsp/system.h"
#include "../M2_core2_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../M2_core2_bsp/HAL/inc/includes.h"
#include "../M2_core2_bsp/HAL/inc/io.h"
#include "interrupts.h"
#include "image_data.h"

// base addresses
#define PIXEL_ADDR_BASE CORE_2_PIXEL_ADDR_BASE
#define PIXEL_DATA_BASE CORE_2_PIXEL_DATA_BASE
#define PIXEL_WREN_BASE CORE_2_PIXEL_WREN_BASE

// frame buffer dimensions
#define FB_WIDTH  320
#define FB_HEIGHT 240
#define FB_SIZE   (FB_WIDTH * FB_HEIGHT)  // 76800 pixels

// sdram buffer address
#define SDRAM_FB_ADDR 0x3ED000UL
static uint8_t *sdram_fb = (uint8_t *)SDRAM_FB_ADDR;
#define EMERGENCY_FB_ADDR 0x3DA000
static uint8_t *emergency_fb = (uint8_t *)EMERGENCY_FB_ADDR;

// task config
#define VGA_STACKSIZE 4096
#define VGA_PRIORITY  2
OS_STK vga_stk[VGA_STACKSIZE];

// write_pixel()
// addr  : 0 to 76799 (row-major: addr = x + y * FB_WIDTH)
// color : 4-bit grayscale value (0x0 = black, 0xF = white)
void write_pixel(unsigned int addr, unsigned char color) {
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_ADDR_BASE, addr);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_DATA_BASE, color & 0xF);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 1);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 0);
}

// write_framebuffer()
// Reads from SDRAM, pushes all pixels to RAM2Port.
// VGA controller then displays it continuously on its own.
static void write_framebuffer(void) {
    unsigned int i;
    for (i = 0; i < FB_SIZE; i++) {
        write_pixel(i, sdram_fb[i]);
    }
}

static void write_emergency(void) {
    unsigned int i;
    for (i = 0; i < FB_SIZE; i++) {
        write_pixel(i, emergency_fb[i]);
    }
}

// clear_framebuffer()
// Fills display with black.
static void clear_framebuffer(void) {
    unsigned int i;
    for (i = 0; i < FB_SIZE; i++) {
        write_pixel(i, 0x0);
    }
}

// vga_task()
void vga_task(void *pdata) {
	printf("[VGA] Start task\n");

	// copy image from flash into sdram
	printf("[VGA] Copying image to SDRAM at 0x%08X...\n", (unsigned int)SDRAM_FB_ADDR);
	memcpy(sdram_fb, baby_image_data, IMG_SIZE);
	memcpy(emergency_fb, emergency_image_data, IMG_SIZE);
	printf("[VGA] Image ready in SDRAM (%d bytes).\n", IMG_SIZE);

	// start with black screen
	clear_framebuffer();
	printf("[VGA] Display cleared. Waiting for KEY1 from Core 3...\n");

	// main loop
	while (1) {
		if (emergency_flag) {
			printf("\n[VGA] EMERGENCY STOP - HALTING OPERATION\n");
			write_emergency();
			while (emergency_flag) {
				OSTimeDly(5);
			}
			key_flags = 0;
			write_framebuffer();
			printf("[VGA] EMERGENCY CLEARED - CONTINUING OPERATION\n\n");
		}
		// key0 simulates capturing data - save into sdram
		if (key_flags & (1 << 0)) {
			key_flags &= ~(1 << 0);
			printf("[VGA] Clearing image\n");
			clear_framebuffer();
		}
		// key1 reads data from sdram and display onto vga
		if (key_flags & (1 << 1)) {
			key_flags &= ~(1 << 1);
			printf("[VGA] Displaying image from SDRAM\n");
			write_framebuffer();
		}
		OSTimeDly(10);
    }
}

// vga_init()
void vga_init(void) {
    OSTaskCreateExt(vga_task,
                    NULL,
                    &vga_stk[VGA_STACKSIZE - 1],
                    VGA_PRIORITY,
                    VGA_PRIORITY,
                    vga_stk,
                    VGA_STACKSIZE,
                    NULL,
                    0);
}
