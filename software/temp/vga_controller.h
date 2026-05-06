#ifndef VGA_CONTROLLER_H
#define VGA_CONTROLLER_H

#include <stdio.h>
#include "includes.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"

/* Base addresses for custom VGA pixel buffer interface */
#define PIXEL_ADDR_BASE PIO_PIXEL_ADDR_BASE
#define PIXEL_DATA_BASE PIO_PIXEL_DATA_BASE
#define PIXEL_WREN_BASE PIO_PIXEL_WREN_BASE

/* Display geometry */
#define VGA_WIDTH   320
#define VGA_HEIGHT  240
#define VGA_PIXELS  (VGA_WIDTH * VGA_HEIGHT)

void write_pixel(unsigned int addr, unsigned char color);
void draw_color_stripes(void);
void draw_vertical_stripes(void);

#endif
