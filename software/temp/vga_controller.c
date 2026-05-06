#include "vga_controller.h"

/* Write one 4-bit pixel to the pixel buffer */
void write_pixel(unsigned int addr, unsigned char color) {
	if (addr >= VGA_PIXELS) return;
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_ADDR_BASE, addr);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_DATA_BASE, color & 0x0F);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 1);
    IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_WREN_BASE, 0);
}

/* Horizontal color stripes: color changes with x */
void draw_color_stripes(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            unsigned int addr = (y * VGA_WIDTH) + x;
            unsigned char color = (x / 20) & 0x0F;
            write_pixel(addr, color);
        }
    }
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
}

/* VGA Task for uC/OS-II */
//void task5(void *pdata) {
//    (void)pdata;
//
//    while (1) {
//        draw_color_stripes();
//        OSTimeDlyHMSM(0, 0, 1, 0);   // show for 1 sec
//
//        draw_vertical_stripes();
//        OSTimeDlyHMSM(0, 0, 1, 0);   // show for 1 sec
//    }
//}
