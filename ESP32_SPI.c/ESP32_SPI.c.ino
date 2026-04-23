#include <string.h>
#include "driver/spi_slave.h"

#define PIN_MOSI 10   // D10 -> GPIO10
#define PIN_MISO 9    // D9  -> GPIO9
#define PIN_SCLK 8    // D8  -> GPIO8
#define PIN_CS   20   // D7  -> GPIO20

#define BUF_SIZE 300

uint8_t rx_buf[BUF_SIZE];
uint8_t tx_buf[BUF_SIZE];

char last_received_msg[BUF_SIZE];
uint32_t esp32_rx_count = 0;

// 0 = waiting for Nios write transaction
// 1 = waiting for Nios read transaction
uint8_t spi_phase = 0;

void prepare_tx_message(uint32_t count) {
  memset(tx_buf, 0, BUF_SIZE);
  snprintf((char*)tx_buf, BUF_SIZE, "Hi I am ESP32 count: %lu", (unsigned long)count);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  memset(rx_buf, 0, BUF_SIZE);
  memset(tx_buf, 0, BUF_SIZE);
  memset(last_received_msg, 0, BUF_SIZE);

  // Initial response, used when Nios performs its first read transaction
  prepare_tx_message(0);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = PIN_MOSI;
  buscfg.miso_io_num = PIN_MISO;
  buscfg.sclk_io_num = PIN_SCLK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = BUF_SIZE;

  spi_slave_interface_config_t slvcfg = {};
  slvcfg.mode = 0;
  slvcfg.spics_io_num = PIN_CS;
  slvcfg.queue_size = 1;
  slvcfg.flags = 0;

  esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    Serial.print("SPI init failed: ");
    Serial.println(ret);
    while (1) {
      delay(1000);
    }
  }

  Serial.println("ESP32 SPI slave ready");
  Serial.println("Type 'p' in Serial Monitor to print the last saved received message.");
}

void loop() {
  // Local terminal command to print the saved message
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'p' || c == 'P') {
      Serial.println("----- ESP32 saved received message -----");
      if (strlen(last_received_msg) > 0) {
        Serial.println(last_received_msg);
      } else {
        Serial.println("(none)");
      }
      Serial.println("----------------------------------------");
    }
  }

  memset(rx_buf, 0, BUF_SIZE);

  spi_slave_transaction_t t = {};
  t.length = BUF_SIZE * 8;
  t.tx_buffer = tx_buf;
  t.rx_buffer = rx_buf;

  esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, 10 / portTICK_PERIOD_MS);

  if (ret != ESP_OK) {
    Serial.print("SPI transaction failed: ");
    Serial.println(ret);
    return;
  }

  int actual_len = t.trans_len / 8;

  if (spi_phase == 0) {
    // Phase 0: Nios is expected to WRITE a message to ESP32
    int msg_len = actual_len;
    if (msg_len >= BUF_SIZE) {
      msg_len = BUF_SIZE - 1;
    }

    memcpy(last_received_msg, rx_buf, msg_len);
    last_received_msg[msg_len] = '\0';

    Serial.println("Received message from Nios II:");
    Serial.println(last_received_msg);

    esp32_rx_count++;

    // Prepare the response for the NEXT transaction
    prepare_tx_message(esp32_rx_count - 1);

    Serial.print("Prepared response for Nios II: ");
    Serial.println((char*)tx_buf);

    // Next transaction should be Nios reading our prepared response
    spi_phase = 1;
  } else {
    // Phase 1: Nios is expected to READ the already-prepared response
    Serial.println("Response sent to Nios II.");

    // Go back to waiting for the next Nios write transaction
    spi_phase = 0;
  }

  memset(rx_buf, 0, BUF_SIZE);
}