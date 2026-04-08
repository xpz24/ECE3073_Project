// LIBRARIES
#include <stdio.h>
#include <ctype.h>
#include "../project_M1_bsp/system.h"
#include "../project_M1_bsp/HAL/inc/alt_types.h"
#include "../project_M1_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../project_M1_bsp/drivers/inc/altera_up_avalon_accelerometer_spi.h"

// BASE ADDRESSES
#define HEX_LO_BASE 0x04009080
#define HEX_HI_BASE 0x04009070
#define SW_BASE 0x040090a0
#define LEDR_BASE 0x04009090
#define KEY_BASE 0x040090b0

// GLOBAL INTERRUPT FLAG
volatile int scroll_enable = 0;

// 7-SEGMENT DICTIONARY
#define SEG_0     0x40
#define SEG_1     0x79
#define SEG_2     0x24
#define SEG_3     0x30
#define SEG_4     0x19
#define SEG_5     0x12
#define SEG_6     0x02
#define SEG_7     0x78
#define SEG_8     0x00
#define SEG_9     0x10
#define SEG_A     0x08
#define SEG_B     0x03
#define SEG_C     0x46
#define SEG_D     0x21
#define SEG_E     0x06
#define SEG_F     0x0E
#define SEG_G     0x02
#define SEG_H     0x09
#define SEG_I     0x79
#define SEG_J     0x61
#define SEG_L     0x47
#define SEG_N     0x2B
#define SEG_O     0x40
#define SEG_P     0x0C
#define SEG_R     0x2F
#define SEG_S     0x12
#define SEG_T     0x07
#define SEG_U     0x41
#define SEG_Y     0x11
#define SEG_BLANK 0x7F
#define SEG_DASH  0x3F
#define SEG_EQUAL 0x37
#define SEG_DOT   0xBF

#define DISPLAY_WIDTH 6

// CHARACTER LOOKUP TABLE
unsigned char seg_char(char c, int dp) {
	c = toupper((unsigned char)c);
	static const unsigned char table[128] = {
		['0'] = SEG_0,
		['1'] = SEG_1,
		['2'] = SEG_2,
		['3'] = SEG_3,
		['4'] = SEG_4,
		['5'] = SEG_5,
		['6'] = SEG_6,
		['7'] = SEG_7,
		['8'] = SEG_8,
		['9'] = SEG_9,
		['A'] = SEG_A,
		['B'] = SEG_B,
		['C'] = SEG_C,
		['D'] = SEG_D,
		['E'] = SEG_E,
		['F'] = SEG_F,
		['G'] = SEG_G,
		['H'] = SEG_H,
		['I'] = SEG_I,
		['J'] = SEG_J,
		['L'] = SEG_L,
		['N'] = SEG_N,
		['O'] = SEG_O,
		['P'] = SEG_P,
		['R'] = SEG_R,
		['S'] = SEG_S,
		['T'] = SEG_T,
		['U'] = SEG_U,
		['Y'] = SEG_Y,
		['-'] = SEG_DASH,
		[' '] = SEG_BLANK,
		['='] = SEG_EQUAL
	};
	unsigned char seg = table[(unsigned char)c];
	if (table[(unsigned char)c] == 0 && c != '0' && c != '8') {
		seg = SEG_BLANK | 0x80;
	}
    seg = dp ? (seg & 0x7F) : (seg | 0x80);
    return seg;
}

// CHARACTER STRUCT
typedef struct {
    char c; // character
    int dp; // decimal point
} display_char;

// WRITE HEX FUNCTION
void write_hex(unsigned char s5, unsigned char s4, unsigned char s3, unsigned char s2, unsigned char s1, unsigned char s0) {
    IOWR_ALTERA_AVALON_PIO_DATA(HEX_LO_BASE, (s2 << 16) | (s1 << 8) | s0);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX_HI_BASE, (s5 << 16) | (s4 << 8) | s3);
}

// CLEAR HEX FUNCTION
void clear_hex() {
	unsigned char blank = SEG_BLANK | 0x80;
    write_hex(blank, blank, blank, blank, blank, blank);
}

// HEX IDLE FUNCTION
void show_all_dash(void) {
	unsigned char dash = SEG_DASH | 0x80;
    write_hex(dash, dash, dash, dash, dash, dash);
}

// PROCESSED BUFFER
display_char processed[100];
int processed_len = 0;

// PROCESS STRING FUNCTION
void process_string(const char *text) {
    processed_len = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        // skip '.' itself
        if (text[i] == '.') continue;
        processed[processed_len].c = text[i];
        // check if next char is '.'
        if (text[i + 1] == '.') {
            processed[processed_len].dp = 1;
        }
        else {
            processed[processed_len].dp = 0;
        }
        processed_len++;
    }
}

// SCROLL STRING FUNCTION
// Returns 0 if SW[0] turned off mid-scroll, 1 if completed
int scroll_string(int scroll_speed) {
    int len = processed_len;
    // Total scroll positions = message length + 6 blank lead-in
    int total = len + DISPLAY_WIDTH;
    int pos, j;

    for (pos = 0; pos < total; pos++) {
        unsigned char display[6];
        // Fill 6 display slots
        for (j = 0; j < DISPLAY_WIDTH; j++) {
            int char_idx = pos + j - DISPLAY_WIDTH;

            if (char_idx >= 0 && char_idx < len) {
                display[j] = seg_char(processed[char_idx].c, processed[char_idx].dp);
            }
            else {
                display[j] = seg_char(' ', 0);
            }
        }
        // Write to displays (HEX5 is leftmost = display[5])
        write_hex(display[0], display[1], display[2], display[3], display[4], display[5]);
        // Check if SW[0] turned off — abort scroll
        if (!(IORD_ALTERA_AVALON_PIO_DATA(SW_BASE) & 0x1)) {
            clear_hex();
            return 0;
        }
        // Scroll speed delay
        volatile int d;
        for (d = 0; d < scroll_speed; d++);
    }
    return 1;
}

// MAIN
int led_hex_main(char *text) {
	// adjust to change scroll speed
    int scroll_speed = 100000;
    process_string(text);
    clear_hex();
    printf("System ready. Flip switches to display messages.\n");

    while (1) {
        int sw = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE);
        // Mirror switches to LEDs to see what's active
		IOWR_ALTERA_AVALON_PIO_DATA(LEDR_BASE, sw);
        // SW[0] OFF — everything off
        if (!(sw & 0x1)) {
            clear_hex();
        }
        // SW[0] ON — check SW[1]
        else {
            // SW[0]+SW[1]
            if (sw & 0x2) {
                printf("Scrolling: %s\n", text);
                scroll_string(scroll_speed);
            }
            // SW[0] only — default: all dash
            else {
                show_all_dash();
            }
        }
        volatile int delay;
        for (delay = 0; delay < 10000; delay++);
    }
    return 0;
}
