#include "mbedtls/base64.h"
#include <Arduino.h>
#include <TJpg_Decoder.h>
#include <Seeed_Arduino_SSCMA.h>
#include <HardwareSerial.h>
#include <math.h>
#include "driver/spi_slave.h"

// ── SPI pins ─────────────────────────────────────────────────────────────────
#define PIN_MOSI 10
#define PIN_MISO 9
#define PIN_SCLK 8
#define PIN_CS 2
#define PIN_READY 3
#define PIN_MODE 4   // input from FPGA: 0=inference, 1=live stream

// ── Payload geometry ─────────────────────────────────────────────────────────
#define IMG_W 208
#define IMG_H 208
#define SRC_W 240  // AI.last_image() native size
#define SRC_H 240
#define FB_SIZE (IMG_W * IMG_H * 3 / 2)  // 12bpp packed = 64896 bytes
#define TEXT_BUF_LEN 30                  // configurable
#define PAYLOAD_SIZE (FB_SIZE + TEXT_BUF_LEN)

// ── Globals ──────────────────────────────────────────────────────────────────
HardwareSerial atSerial(0);
SSCMA AI;

#define DEBUG 0

// Single contiguous payload: [framebuffer | text]
static uint8_t payload[PAYLOAD_SIZE];
static uint8_t *const framebuffer = payload;
static uint8_t *const textbuffer = payload + FB_SIZE;

// Intermediate buffer for JPEG decode: 240x240 RGB888
static uint8_t rgb240[SRC_W * SRC_H * 3];

// ── Tracker config ───────────────────────────────────────────────────────────
const char *LABELS[] = {
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
  "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
  "U", "V", "W", "X", "Y", "Z",
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};
const int NUM_LABELS = sizeof(LABELS) / sizeof(LABELS[0]);

const uint8_t CONF_THRESH = 70;
const int COMMIT_X = 80;
const int OFF_SCREEN_X = 40;
const int SPACE_GAP_MS = 600;
const int WORD_TIMEOUT_MS = 1500;
const float MATCH_DIST = 60.0f;
const int MAX_TRACKS = 16;
const int TRACK_TIMEOUT_MS = 800;
const int MAX_DETECTIONS = 16;

// ── Tracker state ────────────────────────────────────────────────────────────
struct Track {
  bool active;
  int label;
  float x;
  float prevX;
  uint32_t lastSeen;
  uint32_t id;
};

struct Detection {
  int label;
  float x;
  uint8_t score;
  bool matched;
};

Track tracks[MAX_TRACKS];
uint32_t nextTrackId = 1;
static uint32_t lastCommitTime = 0;

// Sentence assembly. Filled by commitCrossings(); flushed into textbuffer
// only on the frame where sentence completion is detected.
static char sentenceBuf[TEXT_BUF_LEN + 1];
static uint8_t sentenceLen = 0;
static bool sentenceReady = false;  // set true on the frame timeout fires

static bool spi_busy = false;
static spi_slave_transaction_t spi_trans;   // must outlive queue call

// ── Forward decls ────────────────────────────────────────────────────────────
int collectDetections(Detection dets[]);
void matchAndUpdate(Detection dets[], int detCount);
void spawnUnmatched(Detection dets[], int detCount);
void commitCrossings();
void expireTracks();
void checkSentenceTimeout();
void appendToSentence(char c);

bool decodeBase64(const String &input, uint8_t **outBuf, size_t *outLen);
bool tjpgDrawCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
void buildFramebuffer();
bool sendPayloadSPI();

// ─────────────────────────────────────────────────────────────────────────────
// Setup
// ─────────────────────────────────────────────────────────────────────────────
void initSPI() {
  pinMode(PIN_READY, OUTPUT);
  digitalWrite(PIN_READY, LOW);

  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = PIN_MOSI;
  buscfg.miso_io_num = PIN_MISO;
  buscfg.sclk_io_num = PIN_SCLK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = PAYLOAD_SIZE;

  spi_slave_interface_config_t slvcfg = {};
  slvcfg.mode = 0;
  slvcfg.spics_io_num = PIN_CS;
  slvcfg.queue_size = 1;
  slvcfg.flags = 0;

  esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    Serial.print("SPI init failed: ");
    Serial.println(ret);
    while (1) delay(1000);
  }
  Serial.println("SPI slave ready");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  while (!AI.begin(&atSerial)) {
    Serial.println("Waiting for Grove Vision AI V2...");
    delay(500);
  }
  TJpgDec.setSwapBytes(false);
pinMode(PIN_MODE, INPUT_PULLDOWN);
  initSPI();
  memset(payload, 0, PAYLOAD_SIZE);
  Serial.println("Grove Vision AI V2 started");
}

// ─────────────────────────────────────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  bool liveStream = (digitalRead(PIN_MODE) == HIGH);

  if (liveStream) {
    // ─── LIVE STREAM MODE ──────────────────────────────────────────────────
    // Grab a fresh image. No tracker work this frame.
    if (AI.sample(1) == CMD_OK) {
      String b64 = AI.last_image();
      if (b64.length() > 0) {
        uint8_t *jpgBuf = NULL;
        size_t   jpgLen = 0;
        if (decodeBase64(b64, &jpgBuf, &jpgLen)) {
          TJpgDec.setCallback(tjpgDrawCallback);
          TJpgDec.setJpgScale(1);
          memset(rgb240, 0, sizeof(rgb240));
          TJpgDec.drawJpg(0, 0, jpgBuf, jpgLen);
          free(jpgBuf);
          buildFramebuffer();
        }
      }
    }

    // Text field zeroed in this mode — no inference happening.
    memset(textbuffer, 0, TEXT_BUF_LEN);

  } else {
    // ─── INFERENCE MODE ────────────────────────────────────────────────────
    if (AI.invoke(1, false, true)) {
      // failed — still run timing-based logic so sentence flushes work
      expireTracks();
      checkSentenceTimeout();
    } else {
      // Tracker pipeline
      Detection dets[MAX_DETECTIONS];
      int detCount = collectDetections(dets);
      for (int t = 0; t < MAX_TRACKS; t++) {
        if (tracks[t].active) tracks[t].prevX = tracks[t].x;
      }
      matchAndUpdate(dets, detCount);
      spawnUnmatched(dets, detCount);
      commitCrossings();
      expireTracks();
      checkSentenceTimeout();
    }

    // Image field zeroed in this mode — FPGA can choose to show
    // "INFERENCE MODE" placeholder or just blank/random pixels.
    memset(framebuffer, 0, FB_SIZE);

    // Text field carries the sentence (or zeros if none ready)
    memset(textbuffer, 0, TEXT_BUF_LEN);
    if (sentenceReady) {
      uint8_t n = sentenceLen < TEXT_BUF_LEN ? sentenceLen : TEXT_BUF_LEN;
      memcpy(textbuffer, sentenceBuf, n);
    }
  }

  // ─── Ship payload (both modes) ──────────────────────────────────────────
  if (sendPayloadSPI()) {
    // On a successful queue in inference mode, consume the sentence
    if (!liveStream && sentenceReady) {
      sentenceReady = false;
      sentenceLen   = 0;
      sentenceBuf[0] = 0;
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tracker — unchanged logic
// ─────────────────────────────────────────────────────────────────────────────
int collectDetections(Detection dets[]) {
  int n = 0;
  for (int i = 0; i < (int)AI.boxes().size() && n < MAX_DETECTIONS; i++) {
    auto box = AI.boxes()[i];
    if (box.score < CONF_THRESH) continue;
    if (box.target >= NUM_LABELS) continue;
    dets[n].label = box.target;
    dets[n].x = box.x;
    dets[n].score = box.score;
    dets[n].matched = false;
    n++;
  }
  return n;
}

void matchAndUpdate(Detection dets[], int detCount) {
  uint32_t now = millis();
  while (true) {
    int bestT = -1, bestD = -1;
    float bestDist = MATCH_DIST;
    for (int t = 0; t < MAX_TRACKS; t++) {
      if (!tracks[t].active) continue;
      if (tracks[t].lastSeen == now) continue;
      for (int d = 0; d < detCount; d++) {
        if (dets[d].matched) continue;
        if (dets[d].label != tracks[t].label) continue;
        const float BACKWARD_TOLERANCE = 8.0f;
        if (dets[d].x > tracks[t].x + BACKWARD_TOLERANCE) continue;
        float dist = fabsf(dets[d].x - tracks[t].x);
        if (dist < bestDist) {
          bestDist = dist;
          bestT = t;
          bestD = d;
        }
      }
    }
    if (bestT < 0) break;
    dets[bestD].matched = true;
    tracks[bestT].x = dets[bestD].x;
    tracks[bestT].lastSeen = now;
  }
}

void spawnUnmatched(Detection dets[], int detCount) {
  uint32_t now = millis();
  for (int d = 0; d < detCount; d++) {
    if (dets[d].matched) continue;
    int slot = -1;
    for (int t = 0; t < MAX_TRACKS; t++) {
      if (!tracks[t].active) {
        slot = t;
        break;
      }
    }
    if (slot < 0) continue;
    tracks[slot].active = true;
    tracks[slot].label = dets[d].label;
    tracks[slot].x = dets[d].x;
    tracks[slot].prevX = dets[d].x;
    tracks[slot].lastSeen = now;
    tracks[slot].id = nextTrackId++;
  }
}

void commitCrossings() {
  uint32_t now = millis();
  while (true) {
    int best = -1;
    float bestX = COMMIT_X + 1.0f;
    for (int t = 0; t < MAX_TRACKS; t++) {
      if (!tracks[t].active) continue;
      if (!(tracks[t].prevX > COMMIT_X && tracks[t].x <= COMMIT_X)) continue;
      if (tracks[t].x < bestX) {
        bestX = tracks[t].x;
        best = t;
      }
    }
    if (best < 0) break;

    // Spacing: insert ' ' if there was a long-ish gap between commits.
    if (lastCommitTime != 0) {
      uint32_t gap = now - lastCommitTime;
      if (gap > SPACE_GAP_MS && gap <= WORD_TIMEOUT_MS) {
        appendToSentence(' ');
      }
      // gap > WORD_TIMEOUT_MS is handled by checkSentenceTimeout(),
      // which fires the previous sentence BEFORE this letter arrives.
      // By the time we get here, sentenceBuf is empty again.
    }

    appendToSentence(LABELS[tracks[best].label][0]);
    Serial.print(LABELS[tracks[best].label]);

    if (DEBUG) {
      Serial.print(LABELS[tracks[best].label]);
    }
    tracks[best].prevX = tracks[best].x;
    lastCommitTime = now;
  }
}

void expireTracks() {
  uint32_t now = millis();
  for (int t = 0; t < MAX_TRACKS; t++) {
    if (!tracks[t].active) continue;
    if (tracks[t].x < OFF_SCREEN_X) {
      tracks[t].active = false;
      continue;
    }
    if (now - tracks[t].lastSeen > TRACK_TIMEOUT_MS) {
      tracks[t].active = false;
    }
  }
}

// Called every frame. If we've committed at least one letter and nothing new
// has crossed within WORD_TIMEOUT_MS, mark the sentence ready for shipment.
void checkSentenceTimeout() {
  if (sentenceReady) return;     // already armed, waiting to be sent
  if (sentenceLen == 0) return;  // nothing to flush
  if (lastCommitTime == 0) return;
  uint32_t now = millis();
  if (now - lastCommitTime > WORD_TIMEOUT_MS) {
    sentenceBuf[sentenceLen] = 0;
    Serial.println(".");   // ← add this
    sentenceReady = true;
    if (DEBUG) {
      Serial.print(" [SENT] ");
      Serial.println(sentenceBuf);
    }
  }
}

void appendToSentence(char c) {
  if (sentenceReady) return;  // don't append while a completed sentence is pending send
  if (sentenceLen >= TEXT_BUF_LEN) return;
  sentenceBuf[sentenceLen++] = c;
}

// ─────────────────────────────────────────────────────────────────────────────
// Image pipeline
// ─────────────────────────────────────────────────────────────────────────────
bool decodeBase64(const String &input, uint8_t **outBuf, size_t *outLen) {
  size_t requiredLen = 0;
  int ret = mbedtls_base64_decode(
    NULL, 0, &requiredLen,
    (const unsigned char *)input.c_str(), input.length());
  if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) return false;
  *outBuf = (uint8_t *)malloc(requiredLen);
  if (!*outBuf) return false;
  ret = mbedtls_base64_decode(
    *outBuf, requiredLen, outLen,
    (const unsigned char *)input.c_str(), input.length());
  if (ret != 0) {
    free(*outBuf);
    return false;
  }
  return true;
}

// TJpg_Decoder callback. Writes RGB888 into rgb240[].
bool tjpgDrawCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  for (int j = 0; j < h; j++) {
    int py = y + j;
    if (py >= SRC_H) break;
    for (int i = 0; i < w; i++) {
      int px = x + i;
      if (px >= SRC_W) break;
      uint16_t rgb565 = bitmap[j * w + i];
      uint8_t r = ((rgb565 >> 11) & 0x1F) * 255 / 31;
      uint8_t g = ((rgb565 >> 5) & 0x3F) * 255 / 63;
      uint8_t b = (rgb565 & 0x1F) * 255 / 31;
      uint32_t idx = (py * SRC_W + px) * 3;
      rgb240[idx + 0] = r;
      rgb240[idx + 1] = g;
      rgb240[idx + 2] = b;
    }
  }
  return true;
}

// Downscale 240x240 RGB888 → 208x208 RGB444, packed 12bpp (natural packing).
// Byte layout per 2 pixels (P0, P1):
//   B0: [R0 G0]   R0 high nibble, G0 low nibble
//   B1: [B0 R1]
//   B2: [G1 B1]
void buildFramebuffer() {
  // Precompute nearest-neighbor source coords for x (saves work in inner loop)
  static uint16_t srcXTable[IMG_W];
  static bool tableReady = false;
  if (!tableReady) {
    for (int x = 0; x < IMG_W; x++) {
      srcXTable[x] = (uint16_t)((x * SRC_W) / IMG_W);  // floor(x * 240/208)
    }
    tableReady = true;
  }

  uint8_t *out = framebuffer;

  for (int y = 0; y < IMG_H; y++) {
    int srcY = (y * SRC_H) / IMG_H;
    const uint8_t *row = &rgb240[srcY * SRC_W * 3];

    // Pack 2 pixels at a time into 3 bytes
    for (int x = 0; x < IMG_W; x += 2) {
      const uint8_t *p0 = &row[srcXTable[x] * 3];
      const uint8_t *p1 = &row[srcXTable[x + 1] * 3];

      uint8_t r0 = p0[0] >> 4, g0 = p0[1] >> 4, b0 = p0[2] >> 4;
      uint8_t r1 = p1[0] >> 4, g1 = p1[1] >> 4, b1 = p1[2] >> 4;

      *out++ = (r0 << 4) | g0;
      *out++ = (b0 << 4) | r1;
      *out++ = (g1 << 4) | b1;
    }
  }
}

bool sendPayloadSPI() {
  // Reap previous transaction if it finished
  if (spi_busy) {
    spi_slave_transaction_t *done = NULL;
    esp_err_t r = spi_slave_get_trans_result(SPI2_HOST, &done, 0);
    if (r == ESP_OK) {
      spi_busy = false;
      digitalWrite(PIN_READY, LOW);
    } else {
      // Previous frame still in flight (FPGA hasn't clocked it out yet).
      // Drop this frame so the tracker keeps running.
      return false;
    }
  }

  // Queue a new transaction
  spi_trans = {};
  spi_trans.length    = PAYLOAD_SIZE * 8;
  spi_trans.tx_buffer = payload;
  spi_trans.rx_buffer = NULL;

  esp_err_t r = spi_slave_queue_trans(SPI2_HOST, &spi_trans, 0);
  if (r == ESP_OK) {
    spi_busy = true;
    digitalWrite(PIN_READY, HIGH);
    return true;
  }
  return false;
}
