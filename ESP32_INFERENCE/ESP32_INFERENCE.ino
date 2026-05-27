#include <Seeed_Arduino_SSCMA.h>
#include <HardwareSerial.h>
#include <math.h>

HardwareSerial atSerial(0);
SSCMA AI;

#define DEBUG 0

const char* LABELS[] = {
  "A","B","C","D","E","F","G","H","I","J",
  "K","L","M","N","O","P","Q","R","S","T",
  "U","V","W","X","Y","Z",
  "0","1","2","3","4","5","6","7","8","9"
};
const int NUM_LABELS = sizeof(LABELS) / sizeof(LABELS[0]);

// ── Tuning ───────────────────────────────────────────────────────────────────
const uint8_t CONF_THRESH      = 70;
const int     COMMIT_X         = 80;     // crossing line
const int     OFF_SCREEN_X     = 40;     // track has scrolled off, expire it
const int     SPACE_GAP_MS     = 600;
const int     WORD_TIMEOUT_MS  = 1500;
const float   ALPHA            = 1.0f;   // EMA smoothing for track position
const float   MATCH_DIST       = 60.0f;  // max px between frames to call it the same letter
const int     MAX_TRACKS       = 16;
const int     TRACK_TIMEOUT_MS = 800;
const int     MAX_DETECTIONS   = 16;

// ── State ────────────────────────────────────────────────────────────────────
struct Track {
  bool     active;
  int      label;
  float    x;         // current smoothed position
  float    prevX;     // position last frame (used for crossing detection)
  uint32_t lastSeen;
  uint32_t id;        // debug-friendly identifier
};

Track    tracks[MAX_TRACKS];
uint32_t nextTrackId = 1;

struct Detection {
  int     label;
  float   x;
  uint8_t score;   // ← NEW
  bool    matched;
};


static uint32_t lastCommitTime = 0;

// ── Forward declarations ─────────────────────────────────────────────────────
int  collectDetections(Detection dets[]);
void matchAndUpdate(Detection dets[], int detCount);
void spawnUnmatched(Detection dets[], int detCount);
void commitCrossings();
void expireTracks();
void printSpacing(uint32_t now);
void debugFrame(Detection dets[], int detCount, const char* phase);

// ── Setup / Loop ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  while (!AI.begin(&atSerial)) {
    Serial.println("Waiting for Grove Vision AI V2...");
    delay(500);
  }
  Serial.println("Grove Vision AI V2 started");
}

void loop() {
  if (AI.invoke(1, true, false)) {
    expireTracks();
    return;
  }

  Detection dets[MAX_DETECTIONS];
  int detCount = collectDetections(dets);

  // Save prevX for every active track BEFORE updating positions.
  // Crossings are detected from (prevX > COMMIT_X) → (x <= COMMIT_X).
  for (int t = 0; t < MAX_TRACKS; t++) {
    if (tracks[t].active) tracks[t].prevX = tracks[t].x;
  }

  matchAndUpdate(dets, detCount);
  spawnUnmatched(dets, detCount);

  if (DEBUG) debugFrame(dets, detCount, "after-update");

  commitCrossings();
  expireTracks();
}

// ── Step 1: gather valid detections from this frame ──────────────────────────
int collectDetections(Detection dets[]) {
  int n = 0;
  for (int i = 0; i < (int)AI.boxes().size() && n < MAX_DETECTIONS; i++) {
    auto box = AI.boxes()[i];
    if (box.score < CONF_THRESH)  continue;
    if (box.target >= NUM_LABELS) continue;
    dets[n].label   = box.target;
    dets[n].x       = box.x;
    dets[n].score   = box.score;    // ← NEW
    dets[n].matched = false;
    n++;
  }
  return n;
}

// ── Step 2: globally greedy matching ─────────────────────────────────────────
// Each round, find the single closest (track, detection) pair across ALL
// unmatched candidates of the same label. Assign it. Repeat until no pair
// is within MATCH_DIST. This avoids the order-dependency bug where the
// first iterated track grabs a detection that was actually closer to another.
void matchAndUpdate(Detection dets[], int detCount) {
  uint32_t now = millis();

  while (true) {
    int   bestT = -1;
    int   bestD = -1;
    float bestDist = MATCH_DIST;

    for (int t = 0; t < MAX_TRACKS; t++) {
      if (!tracks[t].active)         continue;
      if (tracks[t].lastSeen == now) continue;

      for (int d = 0; d < detCount; d++) {
        if (dets[d].matched)                  continue;
        if (dets[d].label != tracks[t].label) continue;
        
        // Reject matches that would move the track rightward by more than 
        // a small jitter tolerance. Scrolling text only moves leftward.
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

    dets[bestD].matched   = true;
    tracks[bestT].x       = dets[bestD].x;
    tracks[bestT].lastSeen = now;
  }
}

// ── Step 3: spawn tracks for unmatched detections ────────────────────────────
// No spawn guard needed. A ghost detection that spawns a new track will have
// prevX == x (both ≤ COMMIT_X), so it won't trigger a crossing event.
void spawnUnmatched(Detection dets[], int detCount) {
  uint32_t now = millis();

  for (int d = 0; d < detCount; d++) {
    if (dets[d].matched) continue;

    int slot = -1;
    for (int t = 0; t < MAX_TRACKS; t++) {
      if (!tracks[t].active) { slot = t; break; }
    }
    if (slot < 0) continue;

    tracks[slot].active   = true;
    tracks[slot].label    = dets[d].label;
    tracks[slot].x        = dets[d].x;
    tracks[slot].prevX    = dets[d].x;   // prevX = x → no spurious crossing
    tracks[slot].lastSeen = now;
    tracks[slot].id       = nextTrackId++;
  }
}

// ── Step 4: detect crossings and commit ──────────────────────────────────────
// A crossing is an edge-triggered event: prevX > COMMIT_X AND x <= COMMIT_X.
// Commit leftmost-first so multiple crossings in one frame come out in order.
void commitCrossings() {
  uint32_t now = millis();

  while (true) {
    int   best = -1;
    float bestX = COMMIT_X + 1.0f;

    for (int t = 0; t < MAX_TRACKS; t++) {
      if (!tracks[t].active) continue;
      if (!(tracks[t].prevX > COMMIT_X && tracks[t].x <= COMMIT_X)) continue;
      if (tracks[t].x < bestX) { bestX = tracks[t].x; best = t; }
    }

    if (best < 0) break;

    printSpacing(now);

    if (DEBUG) {
      Serial.print("[T");
      Serial.print(tracks[best].id);
      Serial.print(":");
      Serial.print(LABELS[tracks[best].label]);
      Serial.print(" ");
      Serial.print((int)tracks[best].prevX);
      Serial.print("→");
      Serial.print((int)tracks[best].x);
      Serial.print("]");
    } else {
      Serial.print(LABELS[tracks[best].label]);
    }

    // Mark crossing as "consumed" by setting prevX = x.
    // The track stays active and continues matching/scrolling until off-screen.
    tracks[best].prevX = tracks[best].x;
    lastCommitTime = now;
  }
}

// ── Step 5: expire tracks ────────────────────────────────────────────────────
void expireTracks() {
  uint32_t now = millis();
  for (int t = 0; t < MAX_TRACKS; t++) {
    if (!tracks[t].active) continue;

    // Scrolled off the left side of the frame
    if (tracks[t].x < OFF_SCREEN_X) {
      if (DEBUG) {
        Serial.print(" exp-off T");
        Serial.print(tracks[t].id);
        Serial.print(":");
        Serial.print(LABELS[tracks[t].label]);
        Serial.println();
      }
      tracks[t].active = false;
      continue;
    }

    // No matching detection within TRACK_TIMEOUT_MS
    if (now - tracks[t].lastSeen > TRACK_TIMEOUT_MS) {
      if (DEBUG) {
        Serial.print(" exp-timeout T");
        Serial.print(tracks[t].id);
        Serial.print(":");
        Serial.print(LABELS[tracks[t].label]);
        Serial.print("@");
        Serial.println((int)tracks[t].x);
      }
      tracks[t].active = false;
    }
  }
}

// ── Helpers ──────────────────────────────────────────────────────────────────
void printSpacing(uint32_t now) {
  if (lastCommitTime == 0) return;
  uint32_t gap = now - lastCommitTime;
  if      (gap > WORD_TIMEOUT_MS) Serial.println(".");
  else if (gap > SPACE_GAP_MS)    Serial.print(" ");
}

void debugFrame(Detection dets[], int detCount, const char* phase) {
  if (detCount == 0) return;

  Serial.print("DET: ");
  for (int d = 0; d < detCount; d++) {
    Serial.print(LABELS[dets[d].label]);
    Serial.print("=");
    Serial.print((int)dets[d].x);
    Serial.print("(");
    Serial.print(dets[d].score);  // ← confidence
    Serial.print(") ");
  }
  Serial.println();

  Serial.print("TRK: ");
  for (int t = 0; t < MAX_TRACKS; t++) {
    if (!tracks[t].active) continue;
    Serial.print(tracks[t].id);
    Serial.print(":");
    Serial.print(LABELS[tracks[t].label]);
    Serial.print(" ");
    Serial.print((int)tracks[t].prevX);
    Serial.print("→");
    Serial.print((int)tracks[t].x);
    Serial.print(" ");
  }
  Serial.println();
}
