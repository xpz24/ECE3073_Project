#ifndef VGA_CONTROLLER_H
#define VGA_CONTROLLER_H

#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "shared_defs.h"

/* Base addresses for custom VGA pixel buffer interface */
#define PIXEL_ADDR_BASE PIO_PIXEL_ADDR_BASE
#define PIXEL_DATA_BASE PIO_PIXEL_DATA_BASE
#define PIXEL_WREN_BASE PIO_PIXEL_WREN_BASE
#define FRAME_READY_BASE  PIO_FRAME_READY_BASE
#define SWAP_DONE_BASE    PIO_SWAP_DONE_BASE
// #define FRAME_VGA_OWNER_ID   1

/* Display geometry */
//#define VGA_WIDTH   320
//#define VGA_HEIGHT  240
//#define VGA_PIXELS  (VGA_WIDTH * VGA_HEIGHT)

void present_frame(void);
void init_vga_mutex(void);
void draw_esp32_framebuffer(void);
void write_pixel(unsigned int addr, unsigned char color);
void draw_horizontal_stripes(void);
void draw_vertical_stripes(void);

#endif
