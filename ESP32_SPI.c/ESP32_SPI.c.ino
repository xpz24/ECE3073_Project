#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include "driver/spi_slave.h"

#define PIN_MOSI 10   // D10 -> GPIO10
#define PIN_MISO 9    // D9  -> GPIO9
#define PIN_SCLK 8    // D8  -> GPIO8
#define PIN_CS   20   // D7  -> GPIO20

#define BUF_SIZE 64

#define CMD_WRITE_MSG  0x01
#define CMD_READ_REPLY 0x02

uint8_t rx_buf[BUF_SIZE];
uint8_t tx_buf[BUF_SIZE];

char last_received_msg[BUF_SIZE];
uint32_t esp32_rx_count = 0;

bool read_armed = false;

void prepare_tx_message(uint32_t count) {
  memset(tx_buf, 0, BUF_SIZE);

  // Put a valid reply packet into tx_buf
  tx_buf[0] = CMD_READ_REPLY;
  snprintf((char*)&tx_buf[1], BUF_SIZE - 1, "Hi I am ESP32 count: %lu",
           (unsigned long)count);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  memset(rx_buf, 0, BUF_SIZE);
  memset(tx_buf, 0, BUF_SIZE);
  memset(last_received_msg, 0, BUF_SIZE);

  // Initial reply
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
}

void loop() {
  memset(rx_buf, 0, BUF_SIZE);

  spi_slave_transaction_t t = {};
  t.length = BUF_SIZE * 8;
  t.tx_buffer = tx_buf;
  t.rx_buffer = rx_buf;

  esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
  if (ret != ESP_OK) {
    Serial.print("SPI transaction failed: ");
    Serial.println(ret);
    return;
  }

  int actual_len = t.trans_len / 8;
  if (actual_len <= 0) {
    return;
  }

  uint8_t cmd = rx_buf[0];

if (cmd == CMD_WRITE_MSG) {
  int msg_len = actual_len - 1;
  if (msg_len < 0) msg_len = 0;
  if (msg_len >= BUF_SIZE) msg_len = BUF_SIZE - 1;

  memcpy(last_received_msg, &rx_buf[1], msg_len);
  last_received_msg[msg_len] = '\0';

  Serial.println("Received WRITE command from Nios II");
  Serial.print("Message: ");
  Serial.println(last_received_msg);
  Serial.println("");

  prepare_tx_message(esp32_rx_count);
  esp32_rx_count++;

  // Serial.print("Prepared reply for next READ: ");
  // Serial.println((char*)&tx_buf[1]);

  read_armed = false;
}
else if (cmd == CMD_READ_REPLY) {
  Serial.println("Received READ command from Nios II");
  read_armed = true;
}
else if (cmd == 0x00 && read_armed) {
  Serial.println("Response sent to Nios II");
  Serial.print("Sent reply: ");
  Serial.println((char*)&tx_buf[1]);

  read_armed = false;
}
else {
  Serial.print("Unknown command received: 0x");
  Serial.println(cmd, HEX);
}
}