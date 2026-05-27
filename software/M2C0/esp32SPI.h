#ifndef ESP32_SPI_H
#define ESP32_SPI_H

#include "alt_types.h"
#include "altera_avalon_mutex.h"
#include "system.h"

extern volatile int esp32_frame_ready;
extern alt_mutex_dev *frame_mutex;

void init_frame_mutex(void);
void init_esp32_ready_irq(void);

/* Read the full payload (framebuffer + text) in a single SPI transaction.
 * rx_data must point to a buffer of at least ESP32_PAYLOAD_SIZE bytes.
 */
int esp32_spi_read_payload(alt_u8 *rx_data);

/* Legacy: read just the framebuffer portion. Kept for compatibility. */
int esp32_spi_read_framebuffer(alt_u8 *rx_data);

int esp32_spi_write(const alt_u8 *tx_data, alt_u32 length);
int esp32_spi_read(alt_u8 *rx_data, alt_u32 length);
int esp32_spi_exchange(const alt_u8 *tx_data, alt_u32 tx_len, alt_u8 *rx_data,
                       alt_u32 rx_len);

#endif
