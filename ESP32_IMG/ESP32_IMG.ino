// #include <Seeed_Arduino_SSCMA.h>
// #include <HardwareSerial.h>

// HardwareSerial atSerial(0);

// SSCMA AI;

// void setup() {
//     Serial.begin(115200);

//     while (!AI.begin(&atSerial)) {
//         Serial.println("AI_BEGIN_FAILED");
//         delay(1000);
//     }

//     Serial.println("READY");
// }

// void loop() {

//     if (!AI.invoke(1, false, true)) {

//         String img = AI.last_image();

//         if (img.length() > 0) {

//             Serial.println("BEGIN_IMAGE");
//             Serial.println(img);
//             Serial.println("END_IMAGE");

//         }
//     }

//     delay(50);
// }
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

  // invoke once, no filter, include image
  if (!AI.invoke(1, true, false)) {

    String base64Str = AI.last_image();
    

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