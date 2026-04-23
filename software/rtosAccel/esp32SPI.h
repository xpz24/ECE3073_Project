#ifndef ESP32_SPI_H
#define ESP32_SPI_H

#include "alt_types.h"

#define ESP32_MSG_SIZE 64

int esp32_spi_write(const alt_u8 *tx_data, alt_u32 length);
int esp32_spi_read(alt_u8 *rx_data, alt_u32 length);
int esp32_spi_exchange(const alt_u8 *tx_data, alt_u32 tx_len,
                       alt_u8 *rx_data, alt_u32 rx_len);

#endif
