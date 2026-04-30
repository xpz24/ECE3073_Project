#include "esp32SPI.h"
#include "system.h"
#include "altera_avalon_spi.h"
#include <string.h>

#define ESP32_SPI_BASE   ESP32_BASE
#define ESP32_SPI_SLAVE  0

/* Write-only transaction: send data to ESP32 */
int esp32_spi_write(const alt_u8 *tx_data, alt_u32 length)
{
    int rc;

    if (tx_data == 0 || length == 0) {
        return -1;
    }

    rc = alt_avalon_spi_command(
        ESP32_SPI_BASE,
        ESP32_SPI_SLAVE,
        length,
        tx_data,
        0,
        0,
        0
    );

    if (rc != 0) {
        return -1;
    }

    return 0;
}

/* Read-only transaction: receive data from ESP32 */
int esp32_spi_read(alt_u8 *rx_data, alt_u32 length)
{
    int rc;

    if (rx_data == 0 || length == 0) {
        return -1;
    }

    rc = alt_avalon_spi_command(
        ESP32_SPI_BASE,
        ESP32_SPI_SLAVE,
        0,
        0,
        length,
        rx_data,
        0
    );

    if (rc != length) {
        return -1;
    }

    return 0;
}

/* Full exchange: write then read (two transactions) */
int esp32_spi_exchange(const alt_u8 *tx_data, alt_u32 tx_len,
                       alt_u8 *rx_data, alt_u32 rx_len)
{
    if (esp32_spi_write(tx_data, tx_len) != 0) {
        return -1;
    }

    if (esp32_spi_read(rx_data, rx_len) != 0) {
        return -1;
    }

    return 0;
}
