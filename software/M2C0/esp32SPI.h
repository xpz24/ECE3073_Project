#ifndef ESP32_SPI_H
#define ESP32_SPI_H

#include "alt_types.h"
#include "altera_avalon_mutex.h"
#include "system.h"

// ESP32 framebuffer format:
// 320 x 240, 4 bits per pixel, packed 2 pixels per byte
//#define ESP32_FB_WIDTH      320
//#define ESP32_FB_HEIGHT     240
//#define ESP32_FB_BPP        4
//#define ESP32_FB_SIZE       (ESP32_FB_WIDTH * ESP32_FB_HEIGHT / 2)  // 38400 bytes
//#define FRAME_SPI_OWNER_ID   0

extern volatile int esp32_frame_ready;
extern alt_mutex_dev *frame_mutex;

void init_frame_mutex(void);
void init_esp32_ready_irq();
int esp32_spi_read_framebuffer(alt_u8 *rx_data);
int esp32_spi_write(const alt_u8 *tx_data, alt_u32 length);
int esp32_spi_read(alt_u8 *rx_data, alt_u32 length);
int esp32_spi_exchange(const alt_u8 *tx_data, alt_u32 tx_len, alt_u8 *rx_data,
		alt_u32 rx_len);

#endif
