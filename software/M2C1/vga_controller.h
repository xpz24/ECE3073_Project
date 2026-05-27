#ifndef VGA_CONTROLLER_H
#define VGA_CONTROLLER_H

#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "shared_defs.h"

#define PIXEL_ADDR_BASE  PIO_PIXEL_ADDR_BASE
#define PIXEL_DATA_BASE  PIO_PIXEL_DATA_BASE   /* must be widened to 12 bits in Qsys */
#define PIXEL_WREN_BASE  PIO_PIXEL_WREN_BASE
#define FRAME_READY_BASE PIO_FRAME_READY_BASE
#define SWAP_DONE_BASE   PIO_SWAP_DONE_BASE

void present_frame(void);
void init_vga_mutex(void);
void draw_esp32_framebuffer(void);
void write_pixel(unsigned int addr, unsigned int color);  /* color now 12-bit */
void draw_horizontal_stripes(void);
void draw_vertical_stripes(void);

#endif
