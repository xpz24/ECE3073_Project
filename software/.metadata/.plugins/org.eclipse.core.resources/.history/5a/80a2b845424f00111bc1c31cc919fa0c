// game.h
// Tile definitions, map layout, player state, and game state machine.

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

// TILE TYPES
#define TILE_EMPTY    0   // walkable floor
#define TILE_WALL     1   // impassable wall
#define TILE_DOOR     2   // locked — requires camera letter to open
#define TILE_CHEST    3   // collectible item
#define TILE_EXIT     4   // proceed to next floor
#define TILE_TRAP     5   // damages player on step
#define TILE_ENEMY    6   // static enemy, requires gesture to defeat
#define TILE_KEY      7   // key item — unlocks TILE_DOOR on same floor

// TILE COLOURS (12-bit RGB444)
#define COL_WALL      0x555   // dark grey
#define COL_FLOOR     0x222   // very dark — subtle grid feel
#define COL_DOOR      0xA50   // amber
#define COL_CHEST     0xFA0   // gold
#define COL_EXIT      0x0F8   // bright green
#define COL_TRAP      0x800   // dark red
#define COL_ENEMY     0xF04   // red
#define COL_KEY       0xFF0   // yellow
#define COL_PLAYER    0x0AF   // cyan
#define COL_BLACK     0x000
#define COL_UI_BG     0x111   // status bar background
#define COL_UI_FG     0xFFF   // status bar text
#define COL_UI_HP     0x0F4   // HP bar green
#define COL_UI_HP_LO  0xF20   // HP bar red (low health)
#define COL_DIVIDER   0x444

// MAP DIMENSIONS
// Play area: 320 wide, 200 tall (top 200px), leaving 40px for status bar below.
#define MAP_COLS      20      // number of tile columns
#define MAP_ROWS      12      // number of tile rows  (fit in 200px play area)
#define TILE_W        16      // pixels per tile (20 * 16 = 320)
#define TILE_H        16      // pixels per tile (12 * 16 = 192, padded to 200)

#define PLAY_AREA_H   (MAP_ROWS * TILE_H)   // 192
#define STATUS_BAR_Y  PLAY_AREA_H           // 192 — status bar starts here
#define STATUS_BAR_H  (240 - PLAY_AREA_H)   // 48px status bar

// DOOR UNLOCK CHARACTERS
// Each door tile has a required camera character stored in door_keys[].
// Up to 4 doors per floor.
#define MAX_DOORS     4
typedef struct {
    int col;        // tile column of door
    int row;        // tile row of door
    char req;       // required character (e.g. 'O', 'A', 'U')
    int  open;      // 1 = already opened
} DoorInfo;

// FLOOR MAPS
// 'P' marks player start — converted to TILE_EMPTY at runtime.
// Format: MAP_ROWS rows × MAP_COLS cols, stored as flat array.

#define NUM_FLOORS    3

// Floor 1 — simple intro layout
static const uint8_t floor1_map[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,1,0,1,1,1,0,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,2,0,0,0,1,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,0,1,1,1,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,1,0,1},
    {1,1,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,1,0,1},
    {1,7,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1},
    {1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1},
};

// Player start positions per floor
static const int floor_start_col[NUM_FLOORS] = { 1, 1, 2 };
static const int floor_start_row[NUM_FLOORS] = { 1, 2, 1 };

// Door definitions per floor (col, row, required_char, open)
static const DoorInfo floor_doors[NUM_FLOORS][MAX_DOORS] = {
    // Floor 1: one door at (6,3) requires 'O'
    { {6, 3, 'O', 0}, {-1,-1,0,0}, {-1,-1,0,0}, {-1,-1,0,0} },
    // Floor 2: two doors
    { {5, 4, 'A', 0}, {12,7, 'U', 0}, {-1,-1,0,0}, {-1,-1,0,0} },
    // Floor 3: one door
    { {9, 5, 'I', 0}, {-1,-1,0,0}, {-1,-1,0,0}, {-1,-1,0,0} },
};

// GAME STATE MACHINE
typedef enum {
    GAME_TITLE,         // title screen
    GAME_PLAYING,       // normal gameplay
    GAME_WAIT_CAMERA,   // paused, waiting for camera input to unlock door
    GAME_CHEST,         // showing chest reward message
    GAME_NEXT_FLOOR,    // transitioning to next floor
    GAME_OVER,          // player died
    GAME_WIN            // all floors cleared
} GameState;

// PLAYER
typedef struct {
    int col;            // current tile column
    int row;            // current tile row
    int hp;             // current health points
    int hp_max;         // maximum health points
    int keys;           // number of keys held
    int score;          // score
    int floor;          // current floor index (0-based)
} Player;

// LIVE GAME DATA
// Runtime copy of current floor map (mutable — doors open, items collected)
static uint8_t  cur_map[MAP_ROWS][MAP_COLS];
static DoorInfo cur_doors[MAX_DOORS];
static Player   player;
static GameState game_state = GAME_TITLE;

// which door index the player is currently trying to unlock (-1 = none)
static int pending_door = -1;

// GAME INIT
static void game_load_floor(int floor_idx) {
    int r, c, d;
    // copy floor map into mutable buffer
    for (r = 0; r < MAP_ROWS; r++)
        for (c = 0; c < MAP_COLS; c++)
            cur_map[r][c] = floor1_map[r][c]; // extend for multi-floor later

    // copy door info
    for (d = 0; d < MAX_DOORS; d++)
        cur_doors[d] = floor_doors[floor_idx][d];

    // place player
    player.col   = floor_start_col[floor_idx];
    player.row   = floor_start_row[floor_idx];
    player.floor = floor_idx;
}

static void game_init(void) {
    player.hp     = 5;
    player.hp_max = 5;
    player.keys   = 0;
    player.score  = 0;
    player.floor  = 0;
    game_state    = GAME_PLAYING;
    pending_door  = -1;
    game_load_floor(0);
}

// TILE QUERY
static int tile_is_passable(int col, int row) {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return 0;
    uint8_t t = cur_map[row][col];
    return (t == TILE_EMPTY   ||
            t == TILE_CHEST   ||
            t == TILE_EXIT    ||
            t == TILE_TRAP    ||
            t == TILE_KEY);
    // TILE_DOOR and TILE_WALL and TILE_ENEMY are impassable until resolved
}

// PLAYER MOVEMENT
// Returns 1 if move was accepted (or triggered an event), 0 if blocked.
static int game_move(int dcol, int drow) {
    if (game_state != GAME_PLAYING) return 0;

    int nc = player.col + dcol;
    int nr = player.row + drow;

    if (nc < 0 || nc >= MAP_COLS || nr < 0 || nr >= MAP_ROWS) return 0;

    uint8_t tile = cur_map[nr][nc];

    // --- wall: blocked
    if (tile == TILE_WALL) return 0;

    // --- door: check for key or camera input
    if (tile == TILE_DOOR) {
        // find which door this is
        int d;
        for (d = 0; d < MAX_DOORS; d++) {
            if (cur_doors[d].col == nc && cur_doors[d].row == nr) {
                if (cur_doors[d].open) break; // already open, fall through
                // try key item first
                if (player.keys > 0) {
                    player.keys--;
                    cur_doors[d].open = 1;
                    cur_map[nr][nc]   = TILE_EMPTY;
                    break;
                }
                // otherwise need camera
                pending_door = d;
                game_state   = GAME_WAIT_CAMERA;
                return 1; // consumed the move, waiting for camera
            }
        }
        // if door is open, passable - fall through to move below
        if (tile == TILE_DOOR) return 0; // still closed and no key
    }

    // --- enemy: blocked until gesture (not yet implemented)
    if (tile == TILE_ENEMY) return 0;

    // --- move accepted
    player.col = nc;
    player.row = nr;

    // --- landing events
    switch (cur_map[nr][nc]) {
        case TILE_CHEST:
            player.score += 50;
            cur_map[nr][nc] = TILE_EMPTY;
            game_state = GAME_CHEST;
            break;
        case TILE_KEY:
            player.keys++;
            cur_map[nr][nc] = TILE_EMPTY;
            break;
        case TILE_TRAP:
            player.hp--;
            if (player.hp <= 0) game_state = GAME_OVER;
            break;
        case TILE_EXIT:
            if (player.floor + 1 < NUM_FLOORS)
                game_state = GAME_NEXT_FLOOR;
            else
                game_state = GAME_WIN;
            break;
        default:
            break;
    }

    return 1;
}

// CAMERA INPUT HANDLER
// Call this whenever ai_detected_char is non-zero.
static void game_handle_camera(char c) {
    if (game_state == GAME_WAIT_CAMERA && pending_door >= 0) {
        if (c == cur_doors[pending_door].req) {
            // correct character — unlock door
            cur_map[cur_doors[pending_door].row][cur_doors[pending_door].col] = TILE_EMPTY;
            cur_doors[pending_door].open = 1;
            player.score += 20;
            pending_door = -1;
            game_state   = GAME_PLAYING;
        }
        // wrong character: stay in GAME_WAIT_CAMERA, prompt again
    }
}

#endif // GAME_H
