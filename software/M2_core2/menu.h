// menu.h
// Main menu screen for mode selection.
// Navigation: KEY1 = cycle selection, KEY0 = confirm.
//
// Depends on: vga_gfx.h (draw_string, fill_rect, FB_WIDTH, FB_HEIGHT)

#ifndef MENU_H
#define MENU_H

#include "vga_gfx.h"

typedef enum {
    MODE_NONE    = -1,
    MODE_GAME    = 0,
    MODE_DEMO    = 1,
    MODE_SYSINFO = 2,
} MenuMode;

#define MENU_ITEMS 3

static const char *menu_labels[MENU_ITEMS] = {
    "DUNGEON GAME",
    "DEMO / BANNER",
    "SYSTEM INFO",
};

static const char *menu_desc[MENU_ITEMS] = {
    "TILT TO MOVE  CAMERA TO ACT",
    "SCROLL CAPTURED TEXT ON VGA",
    "LIVE CPU USAGE + CORE INFO",
};

int      menu_active = 1;
int      menu_sel    = 0;

// Draws a single menu item row — selected or deselected state.
// Call this for both the old and new selection when cycling.
static void menu_draw_item(int i, int is_sel) {
    int item_y = 50 + i * 46;
    uint16_t row_bg = is_sel ? 0x113 : 0x000;

    // Clear the entire item row to its background colour first
    fill_rect(4,          item_y - 2, FB_WIDTH - 8, 38, row_bg);

    // Left accent bar (only when selected)
    if (is_sel)
        fill_rect(4, item_y - 2, 3, 38, 0x0FF);

    // Arrow / blank
    draw_string(is_sel ? ">" : " ", 10, item_y + 4, 1, 0xFF0, row_bg);

    // Number + label
    char num[3] = { '1' + i, '.', '\0' };
    draw_string(num,            22, item_y + 4,  1, is_sel ? 0xFF0 : 0x555, row_bg);
    draw_string(menu_labels[i], 40, item_y + 4,  1, is_sel ? 0xFFF : 0x888, row_bg);

    // Description sub-line
    draw_string(menu_desc[i],   22, item_y + 18, 1, is_sel ? 0x0FF : 0x444, row_bg);
}

// Full redraw — only called once at startup or after emergency restore.
static void menu_draw(void) {
    int i;

    // Background
    fill_rect(0, 0, FB_WIDTH, FB_HEIGHT, 0x000);

    // Header bar
    fill_rect(0, 0, FB_WIDTH, 18, 0x113);
    draw_string("FPGA MULTI-CORE SYSTEM", 8, 4, 1, 0x0FF, 0x113);

    // Subtitle
    draw_string("SELECT A MODE TO CONTINUE", 8, 26, 1, 0x777, 0x000);

    fill_rect(0, 38, FB_WIDTH, 1, 0x333);

    // All items
    for (i = 0; i < MENU_ITEMS; i++)
        menu_draw_item(i, i == menu_sel);

    // Footer
    fill_rect(0, FB_HEIGHT - 22, FB_WIDTH, 1, 0x333);
    draw_string("KEY0:CYCLE MENU   KEY1:SELECT", 8, FB_HEIGHT - 15, 1, 0x444, 0x000);
}

// Partial redraw — only touches the two rows that changed.
static void menu_cycle(void) {
    int prev  = menu_sel;
    menu_sel  = (menu_sel + 1) % MENU_ITEMS;

    menu_draw_item(prev,     0);   // deselect old row
    menu_draw_item(menu_sel, 1);   // select new row
}

// Returns the selected mode and sets menu_active = 0.
static MenuMode menu_confirm(void) {
    menu_active = 0;
    return (MenuMode)menu_sel;
}

#endif // MENU_H
