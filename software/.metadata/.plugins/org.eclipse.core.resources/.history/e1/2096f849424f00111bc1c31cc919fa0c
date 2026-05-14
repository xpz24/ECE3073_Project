// game_render.h
// Tile map, player, and status bar rendering for the dungeon game.
// Depends on: game.h, vga_text_render.h (for fill_rect / draw_char / draw_string)

#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "game.h"

// TILE PIXEL ORIGINS
// Top-left pixel of tile (col, row)
#define TILE_X(col) ((col) * TILE_W)
#define TILE_Y(row) ((row) * TILE_H)

// DRAW SINGLE TILE
// Fills the tile area with a base colour then draws a simple icon on top.

static void draw_tile(int col, int row) {
    int x = TILE_X(col);
    int y = TILE_Y(row);
    uint8_t tile = cur_map[row][col];

    // base colour fill
    uint16_t base;
    switch (tile) {
        case TILE_WALL:   base = COL_WALL;   break;
        case TILE_FLOOR:
        case TILE_EMPTY:  base = COL_FLOOR;  break;
        case TILE_DOOR:   base = COL_DOOR;   break;
        case TILE_CHEST:  base = COL_CHEST;  break;
        case TILE_EXIT:   base = COL_EXIT;   break;
        case TILE_TRAP:   base = COL_TRAP;   break;
        case TILE_ENEMY:  base = COL_ENEMY;  break;
        case TILE_KEY:    base = COL_KEY;    break;
        default:          base = COL_BLACK;  break;
    }
    fill_rect(x, y, TILE_W, TILE_H, base);

    // inner border for walls — gives a chunky block feel
    if (tile == TILE_WALL) {
        fill_rect(x + 1, y + 1, TILE_W - 2, TILE_H - 2, 0x777);
        fill_rect(x + 3, y + 3, TILE_W - 6, TILE_H - 6, COL_WALL);
        return;
    }

    // floor: faint dot at centre for grid visibility
    if (tile == TILE_EMPTY) {
        fill_rect(x + TILE_W/2 - 1, y + TILE_H/2 - 1, 2, 2, 0x333);
        return;
    }

    // door: vertical bars suggesting iron bars
    if (tile == TILE_DOOR) {
        int bx;
        for (bx = x + 3; bx < x + TILE_W - 2; bx += 4)
            fill_rect(bx, y + 1, 2, TILE_H - 2, 0x840);
        // draw required character (tiny) centred
        int d;
        for (d = 0; d < MAX_DOORS; d++) {
            if (cur_doors[d].col == col && cur_doors[d].row == row && !cur_doors[d].open) {
                draw_char(cur_doors[d].req,
                          x + TILE_W/2 - FONT_W/2,
                          y + TILE_H/2 - FONT_H/2,
                          1, 0xFFF, COL_DOOR);
                break;
            }
        }
        return;
    }

    // chest: small box icon
    if (tile == TILE_CHEST) {
        fill_rect(x + 3, y + 4, TILE_W - 6, TILE_H - 7, 0x840);
        fill_rect(x + 3, y + 4, TILE_W - 6, 3,          0xFA0);
        fill_rect(x + TILE_W/2 - 1, y + 6, 2, 4, 0xFFF);
        return;
    }

    // exit: bright downward arrow
    if (tile == TILE_EXIT) {
        // shaft
        fill_rect(x + TILE_W/2 - 1, y + 2, 2, TILE_H - 6, 0x0FF);
        // arrowhead
        fill_rect(x + TILE_W/2 - 3, y + TILE_H - 7, 6, 2, 0x0FF);
        fill_rect(x + TILE_W/2 - 2, y + TILE_H - 5, 4, 2, 0x0FF);
        fill_rect(x + TILE_W/2 - 1, y + TILE_H - 3, 2, 2, 0x0FF);
        return;
    }

    // trap: red X
    if (tile == TILE_TRAP) {
        int i;
        for (i = 2; i < TILE_W - 2; i++) {
            write_pixel((y + i)           * FB_WIDTH + (x + i),           0xF44);
            write_pixel((y + i)           * FB_WIDTH + (x + TILE_W-1-i),  0xF44);
            write_pixel((y + i + 1)       * FB_WIDTH + (x + i),           0xF44);
            write_pixel((y + i + 1)       * FB_WIDTH + (x + TILE_W-1-i),  0xF44);
        }
        return;
    }

    // enemy: simple skull-ish cross shape
    if (tile == TILE_ENEMY) {
        fill_rect(x + TILE_W/2 - 1, y + 2, 2, TILE_H - 4, 0xF88);
        fill_rect(x + 2, y + TILE_H/2 - 1, TILE_W - 4, 2, 0xF88);
        return;
    }

    // key: small key icon (handle + shaft)
    if (tile == TILE_KEY) {
        // ring part
        fill_rect(x + 3, y + 3, 6, 6, 0xFF0);
        fill_rect(x + 4, y + 4, 4, 4, COL_KEY);
        fill_rect(x + 5, y + 5, 2, 2, COL_FLOOR); // hole
        // shaft
        fill_rect(x + 9, y + 6,  6, 2, 0xFF0);
        fill_rect(x + 12, y + 8, 2, 2, 0xFF0);
        return;
    }
}

// DRAW PLAYER
// Simple cyan filled circle-ish shape with a direction indicator.

static void draw_player(void) {
    int x = TILE_X(player.col);
    int y = TILE_Y(player.row);
    // body
    fill_rect(x + 3, y + 2, TILE_W - 6, TILE_H - 4, COL_PLAYER);
    // eyes (two white dots)
    fill_rect(x + 5,  y + 5, 2, 2, 0xFFF);
    fill_rect(x + 9,  y + 5, 2, 2, 0xFFF);
    // mouth
    fill_rect(x + 5,  y + 10, 6, 1, 0x000);
}

// Erase player tile back to map tile (call before moving)
static void erase_player(void) {
    draw_tile(player.col, player.row);
}

// FULL MAP REDRAW
// Call once on floor load or after emergency clear.
// Draws every tile then draws the player on top.

static void game_draw_map(void) {
    int r, c;
    for (r = 0; r < MAP_ROWS; r++)
        for (c = 0; c < MAP_COLS; c++)
            draw_tile(c, r);
    draw_player();
}

// STATUS BAR
// 48px strip below the play area.
// Shows: HP bar | FLOOR number | KEY count | SCORE

static void game_draw_status(void) {
    int sy = STATUS_BAR_Y;
    int sh = STATUS_BAR_H;

    fill_rect(0, sy, FB_WIDTH, sh, COL_UI_BG);
    fill_rect(0, sy, FB_WIDTH, 1,  COL_DIVIDER); // top divider line

    // HP bar
    // Label "HP"
    draw_char('H', 4,  sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('P', 12, sy + 6, 1, COL_UI_FG, COL_UI_BG);

    // bar background
    fill_rect(22, sy + 7, 60, 10, 0x333);
    // bar fill - colour depends on HP level
    int hp_w = (player.hp * 60) / player.hp_max;
    uint16_t hp_col = (player.hp <= player.hp_max / 3) ? COL_UI_HP_LO : COL_UI_HP;
    if (hp_w > 0)
        fill_rect(22, sy + 7, hp_w, 10, hp_col);
    // hp fraction text
    draw_char('0' + player.hp,     86, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('/',                 94, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('0' + player.hp_max, 102,sy + 6, 1, COL_UI_FG, COL_UI_BG);

    // FLOOR
    draw_char('F', 120, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('L', 128, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('R', 136, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char(':', 144, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('1' + player.floor,  152, sy + 6, 1, 0x0FF, COL_UI_BG);

    // KEY count
    draw_char('K', 168, sy + 6, 1, COL_KEY,   COL_UI_BG);
    draw_char(':', 176, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('0' + player.keys,   184, sy + 6, 1, COL_KEY, COL_UI_BG);

    // SCORE
    // up to 999
    int sc = player.score;
    if (sc > 999) sc = 999;
    draw_char('S',        200, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('C',        208, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char(':',        216, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('0' + sc/100,      224, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('0' + (sc/10)%10,  232, sy + 6, 1, COL_UI_FG, COL_UI_BG);
    draw_char('0' + sc%10,       240, sy + 6, 1, COL_UI_FG, COL_UI_BG);

    // PROMPT (bottom row of status bar)
    int py = sy + 24;
    fill_rect(0, py, FB_WIDTH, 1, 0x222); // inner divider

    if (game_state == GAME_WAIT_CAMERA && pending_door >= 0) {
        char req = cur_doors[pending_door].req;
        // "SHOW [X] TO OPEN"
        draw_string("SHOW",   4, py + 6, 1, 0xFA0, COL_UI_BG);
        draw_char(req,       36, py + 6, 1, 0xFFF, 0x500);
        draw_string("TO OPEN",48, py + 6, 1, 0xFA0, COL_UI_BG);
    }
    else if (game_state == GAME_CHEST) {
        draw_string("+50 SCORE!",  4, py + 6, 1, COL_CHEST, COL_UI_BG);
    }
    else if (game_state == GAME_OVER) {
        draw_string("GAME OVER - KEY0 TO RETRY", 4, py + 6, 1, 0xF00, COL_UI_BG);
    }
    else if (game_state == GAME_WIN) {
        draw_string("YOU ESCAPED!  WELL DONE", 4, py + 6, 1, COL_EXIT, COL_UI_BG);
    }
    else if (game_state == GAME_NEXT_FLOOR) {
        draw_string("NEXT FLOOR...", 4, py + 6, 1, 0x0FF, COL_UI_BG);
    }
    else {
        // idle hints
        draw_string("TILT:MOVE  CAM:UNLOCK  KEY0:CLEAR", 4, py + 6, 1, 0x555, COL_UI_BG);
    }
}

// GAME TICK
// Call from vga_task every loop iteration when game_active == 1.
// Handles tilt movement, camera input, state transitions.
// tilt_flag and ai_detected_char are read from interrupts.h globals.

static int  game_tilt_timer   = 0;
#define     GAME_TILT_DELAY   8    // ticks between tilt-triggered moves
static int  game_chest_timer  = 0;
#define     GAME_CHEST_SHOW   60   // ticks to display chest message

static void game_tick(void) {

    // camera input (any state)
    char cam = ai_detected_char;
    if (cam != 0x00) {
        ai_detected_char = 0x00;
        game_handle_camera(cam);
        game_draw_status();
        if (game_state == GAME_PLAYING) game_draw_map(); // redraw if door opened
    }

    // per-state logic
    switch (game_state) {

        case GAME_PLAYING: {
            int tilt = tilt_flag;
            int moved = 0;

            if (tilt == TILT_LEFT || tilt == TILT_RIGHT ||
                tilt == TILT_FORWARD || tilt == TILT_BACKWARD) {
                game_tilt_timer++;
                if (game_tilt_timer >= GAME_TILT_DELAY) {
                    game_tilt_timer = 0;
                    int dc = 0, dr = 0;
                    if      (tilt == TILT_LEFT)     dc = -1;
                    else if (tilt == TILT_RIGHT)     dc = +1;
                    else if (tilt == TILT_FORWARD)   dr = -1;
                    else if (tilt == TILT_BACKWARD)  dr = +1;

                    erase_player();             // wipe old position
                    moved = game_move(dc, dr);
                    draw_tile(player.col - dc, player.row - dr); // restore old tile
                    draw_player();              // draw at new position
                    game_draw_status();
                }
            } else {
                game_tilt_timer = 0;
            }
            break;
        }

        case GAME_CHEST:
            game_chest_timer++;
            if (game_chest_timer >= GAME_CHEST_SHOW) {
                game_chest_timer = 0;
                game_state = GAME_PLAYING;
                game_draw_status();
            }
            break;

        case GAME_NEXT_FLOOR:
            // brief pause then load next floor
            game_chest_timer++;
            if (game_chest_timer >= GAME_CHEST_SHOW) {
                game_chest_timer = 0;
                game_load_floor(player.floor + 1);
                game_state = GAME_PLAYING;
                game_draw_map();
                game_draw_status();
            }
            break;

        case GAME_WAIT_CAMERA:
        case GAME_OVER:
        case GAME_WIN:
            // no tick logic - wait for KEY0 or camera
            break;

        default:
            break;
    }
}

#endif // GAME_RENDER_H
