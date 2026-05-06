#include "mbedtls/base64.h"
#include <Arduino.h>
#include <TJpg_Decoder.h>
#include <Seeed_Arduino_SSCMA.h>
#include <HardwareSerial.h>
#include "driver/spi_slave.h"

#define PIN_MOSI 10  // D10 -> GPIO10
#define PIN_MISO 9   // D9  -> GPIO9
#define PIN_SCLK 8   // D8  -> GPIO8
#define PIN_CS 2     // D0  -> GPIO02
#define PIN_READY 3  // D1 -> GPIO03

HardwareSerial atSerial(0);
SSCMA AI;

// Framebuffer (4bpp packed)
#define W 320
#define H 240
#define FB_SIZE (W * H / 2)

bool once = true;

uint8_t framebuffer[FB_SIZE];

// Temporary decoded image buffer (240x240 grayscale)
uint8_t gray240[240 * 240];

void initSPI() {
  pinMode(PIN_READY, OUTPUT);
  digitalWrite(PIN_READY, LOW);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = PIN_MOSI;
  buscfg.miso_io_num = PIN_MISO;
  buscfg.sclk_io_num = PIN_SCLK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = FB_SIZE;

  spi_slave_interface_config_t slvcfg = {};
  slvcfg.mode = 0;
  slvcfg.spics_io_num = PIN_CS;
  slvcfg.queue_size = 1;
  slvcfg.flags = 0;

  esp_err_t ret = spi_slave_initialize(
    SPI2_HOST,
    &buscfg,
    &slvcfg,
    SPI_DMA_CH_AUTO);

  if (ret != ESP_OK) {
    Serial.print("SPI init failed: ");
    Serial.println(ret);
    while (1) delay(1000);
  }

  Serial.println("SPI slave ready");
}

void sendFramebufferSPI() {
  // digitalWrite(PIN_READY, LOW);
  // delay(10);
  // digitalWrite(PIN_READY, HIGH);
  spi_slave_transaction_t t = {};
  t.length = FB_SIZE * 8;
  t.tx_buffer = framebuffer;
  t.rx_buffer = NULL;

  digitalWrite(PIN_READY, HIGH);

  esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);

  digitalWrite(PIN_READY, LOW);

  if (ret == ESP_OK) {
    Serial.println("Framebuffer sent over SPI");
  } else {
    Serial.print("SPI framebuffer send failed: ");
    Serial.println(ret);
  }
}

// Send FrameBuffer to PC
void sendFramebufferToPC() {
  Serial.println("BEGIN_FB4");
  Serial.println(FB_SIZE);  // should be 38400

  Serial.write(framebuffer, FB_SIZE);

  Serial.println();
  Serial.println("END_FB4");
}
//

// SEND JPEG TO PC OVER SERIAL
void sendJpegToPC(uint8_t *jpgBuf, size_t jpgLen) {
  Serial.println("BEGIN_JPEG");
  Serial.println(jpgLen);
  Serial.write(jpgBuf, jpgLen);
  Serial.println();
  Serial.println("END_JPEG");
}
//

// BASE64
bool decodeBase64(const String &input, uint8_t **outBuf, size_t *outLen) {
  size_t requiredLen = 0;

  // First pass: get required buffer size
  int ret = mbedtls_base64_decode(
    NULL, 0, &requiredLen,
    (const unsigned char *)input.c_str(),
    input.length());

  if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
    Serial.println("Base64 size calc failed");
    return false;
  }

  *outBuf = (uint8_t *)malloc(requiredLen);
  if (!(*outBuf)) {
    Serial.println("Malloc failed");
    return false;
  }

  // Second pass: actual decode
  ret = mbedtls_base64_decode(
    *outBuf, requiredLen, outLen,
    (const unsigned char *)input.c_str(),
    input.length());

  if (ret != 0) {
    Serial.printf("Base64 decode failed: %d\n", ret);
    free(*outBuf);
    return false;
  }

  return true;
}
//////////

// ---------- 4bpp write ----------
void setPixel4(int x, int y, uint8_t val) {
  if (x < 0 || x >= W || y < 0 || y >= H) return;

  uint32_t idx = y * W + x;
  uint32_t byteIdx = idx >> 1;

  if (idx & 1) {
    framebuffer[byteIdx] = (framebuffer[byteIdx] & 0xF0) | (val & 0x0F);
  } else {
    framebuffer[byteIdx] = (framebuffer[byteIdx] & 0x0F) | (val << 4);
  }
}

// ---------- JPEG callback ----------
// Converts decoded RGB → grayscale buffer (240x240)
bool tjpgDrawCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {

      uint16_t rgb565 = bitmap[j * w + i];

      uint8_t r = ((rgb565 >> 11) & 0x1F) * 255 / 31;
      uint8_t g = ((rgb565 >> 5) & 0x3F) * 255 / 63;
      uint8_t b = (rgb565 & 0x1F) * 255 / 31;

      uint8_t gray8 = (77 * r + 150 * g + 29 * b) >> 8;

      int px = x + i;
      int py = y + j;

      if (px < 240 && py < 240) {
        gray240[py * 240 + px] = gray8;
      }
    }
  }
  return true;
}

// ---------- Convert 240x240 → 320x240 with letterbox ----------
void buildFramebuffer() {
  // Clear framebuffer
  memset(framebuffer, 0, FB_SIZE);

  int offsetX = 40;  // (320 - 240)/2

  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 240; x++) {

      uint8_t gray4 = gray240[y * 240 + x] >> 4;

      setPixel4(x + offsetX, y, gray4);
    }
  }
}

// ---------- Process one frame ----------
void processFrame(String base64Str) {
  // if(once) {
uint8_t *jpgBuf = NULL;
  size_t jpgLen = 0;

  if (!decodeBase64(base64Str, &jpgBuf, &jpgLen)) {
    Serial.println("Decode failed");
    return;
  }

  // sendJpegToPC(jpgBuf, jpgLen);

  // JPEG decode
  TJpgDec.setCallback(tjpgDrawCallback);
  TJpgDec.setJpgScale(1);

  memset(gray240, 0, sizeof(gray240));

  TJpgDec.drawJpg(0, 0, jpgBuf, jpgLen);

  free(jpgBuf);

  buildFramebuffer();
  Serial.println("Frame ready, waiting for Nios SPI read");
  once = false;
  // }
  
  sendFramebufferSPI();

  // sendFramebufferToPC();

  //Serial.println("Frame ready (320x240 4bpp)");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  while (!AI.begin(&atSerial)) {
    Serial.println("Waiting for AI.begin...");
    delay(10);
  }

  TJpgDec.setSwapBytes(false);
  initSPI();

  Serial.println("Starting capture...");
  delay(1000);
}

// ---------- Loop ----------
void loop() {
  // if (!AI.invoke(1, false, true)){
  //       if (AI.last_image().length() > 0){
  //           Serial.print("Last image:");
  //           Serial.println(AI.last_image());
  //       }
  //       delay(1000);
  //   }
  // uint32_t t0 = millis();

  // invoke once, no filter, include image
  if (!AI.invoke(1, false, true)) {
  // if(true){

    String base64Str = AI.last_image();
    // String base64Str = "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";

    if (base64Str.length() > 0) {
      Serial.print("Base64 length: ");
      Serial.println(base64Str.length());

      processFrame(base64Str);
    } else {
      Serial.println("No image returned");
    }
  } else {
    Serial.println("AI.invoke failed");
  }
}

// #include <Seeed_Arduino_SSCMA.h>

// #ifdef ESP32
// #include <HardwareSerial.h>

// // Define two Serial devices mapped to the two internal UARTs
// HardwareSerial atSerial(0);

// #else
// #define atSerial Serial1
// #endif

// SSCMA AI;

// void setup()
// {
//    AI.begin(&atSerial);
//    Serial.begin(9600);
// }

// void loop()
// {
//     // invoke once, no filter, get image
//     if (!AI.invoke(1, false, true)){
//         if (AI.last_image().length() > 0){
//             Serial.print("Last image:");
//             Serial.println(AI.last_image());
//         }
//     } else {
//       Serial.println("AI Invoke failed");
//     }
// }