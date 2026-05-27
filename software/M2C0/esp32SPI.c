#include "esp32SPI.h"
#include "system.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_mutex.h"
#include "sys/alt_irq.h"
#include <stdio.h> // Comment out when done debugging
#include "shared_defs.h"

#define ESP32_SPI_BASE   ESP32_BASE
#define ESP32_SPI_SLAVE  0

volatile int esp32_frame_ready = 0;
alt_mutex_dev *frame_mutex = NULL;

void init_frame_mutex(void) {
	frame_mutex = altera_avalon_mutex_open(MUTEX_NAME);

	// Comment out when done debugging
	if (frame_mutex == NULL) {
		printf("[ESP32 SPI] Hardware mutex open failed\n");
	} else {
		printf("[ESP32 SPI] Hardware mutex opened\n");
	}
}

/* Write-only transaction: send data to ESP32 */
int esp32_spi_write(const alt_u8 *tx_data, alt_u32 length) {
	int rc;

	if (tx_data == 0 || length == 0) {
		return -1;
	}

	rc = alt_avalon_spi_command(
	ESP32_SPI_BASE,
	ESP32_SPI_SLAVE, length, tx_data, 0, 0, 0);

	if (rc != 0) {
		return -1;
	}

	return 0;
}

/* Read-only transaction: receive data from ESP32 */
int esp32_spi_read(alt_u8 *rx_data, alt_u32 length) {
	int rc;

	if (rx_data == 0 || length == 0) {
		return -1;
	}

	rc = alt_avalon_spi_command(
	ESP32_SPI_BASE,
	ESP32_SPI_SLAVE, 0, 0, length, rx_data, 0);

	if (rc != length) {
		return -1;
	}

	return 0;
}

/* Full exchange: write then read (two transactions) */
int esp32_spi_exchange(const alt_u8 *tx_data, alt_u32 tx_len, alt_u8 *rx_data,
		alt_u32 rx_len) {
	if (esp32_spi_write(tx_data, tx_len) != 0) {
		return -1;
	}

	if (esp32_spi_read(rx_data, rx_len) != 0) {
		return -1;
	}

	return 0;
}

int esp32_spi_read_framebuffer(alt_u8 *rx_data) {
	return esp32_spi_read(rx_data, ESP32_FB_SIZE);
}

int esp32_spi_read_payload(alt_u8 *rx_data) {
    return esp32_spi_read(rx_data, ESP32_PAYLOAD_SIZE);
}

static void esp32_ready_isr(void *context) {
	// Clear edge capture
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_SPI_READY_BASE, 0x1);

	esp32_frame_ready = 1;
}

void init_esp32_ready_irq() {
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_SPI_READY_BASE, 0x0);

    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_SPI_READY_BASE, 0x1);

    alt_ic_isr_register(
        PIO_SPI_READY_IRQ_INTERRUPT_CONTROLLER_ID,
        PIO_SPI_READY_IRQ,
        esp32_ready_isr,
        NULL,
        NULL
    );

    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_SPI_READY_BASE, 0x1);

    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_SPI_READY_BASE, 0x1);
}
