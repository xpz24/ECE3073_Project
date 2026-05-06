// LIBRARIES
#include <ctype.h>
#include "system.h"
#include "alt_types.h"
#include "altera_avalon_pio_regs.h"
#include "includes.h"

// BASE ADDRESSES
#define HEX_LO_BASE PIO_HEX_LO_BASE
#define HEX_HI_BASE PIO_HEX_HI_BASE
#define SW_BASE PIO_SWITCH_BASE
#define LED_BASE PIO_LED_BASE
#define KEY_BASE PIO_BUTTON_BASE

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

// DEFINITIONS
#define DISPLAY_WIDTH 6

// CHARACTER LOOKUP TABLE
unsigned char seg_char(char c, int dp) {
	c = toupper((unsigned char )c);
	static const unsigned char table[128] = { ['0'] = SEG_0, ['1'] = SEG_1, ['2'
			] = SEG_2, ['3'] = SEG_3, ['4'] = SEG_4, ['5'] = SEG_5, ['6'
			] = SEG_6, ['7'] = SEG_7, ['8'] = SEG_8, ['9'] = SEG_9, ['A'
			] = SEG_A, ['B'] = SEG_B, ['C'] = SEG_C, ['D'] = SEG_D, ['E'
			] = SEG_E, ['F'] = SEG_F, ['G'] = SEG_G, ['H'] = SEG_H, ['I'
			] = SEG_I, ['J'] = SEG_J, ['L'] = SEG_L, ['N'] = SEG_N, ['O'
			] = SEG_O, ['P'] = SEG_P, ['R'] = SEG_R, ['S'] = SEG_S, ['T'
			] = SEG_T, ['U'] = SEG_U, ['Y'] = SEG_Y, ['-'] = SEG_DASH, [' '
			] = SEG_BLANK, ['='] = SEG_EQUAL };
	unsigned char seg = table[(unsigned char) c];
	if (table[(unsigned char) c] == 0 && c != '8') {
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
void write_hex(unsigned char s5, unsigned char s4, unsigned char s3,
		unsigned char s2, unsigned char s1, unsigned char s0) {
	IOWR_ALTERA_AVALON_PIO_DATA(HEX_LO_BASE,
			((alt_u32 )s2 << 16) | ((alt_u32 )s1 << 8) | (alt_u32 )s0);
	IOWR_ALTERA_AVALON_PIO_DATA(HEX_HI_BASE,
			((alt_u32 )s5 << 16) | ((alt_u32 )s4 << 8) | (alt_u32 )s3);
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
// display_char processed[100];
// int processed_len = 0;

// PROCESS STRING FUNCTION
void process_string(const char *text, display_char *processed,
		int *processed_len) {
	*processed_len = 0;

	for (int i = 0; text[i] != '\0'; i++) {
		// skip '.' itself
		if (text[i] == '.')
			continue;
		processed[*processed_len].c = text[i];
		// check if next char is '.'
		if (text[i + 1] == '.') {
			processed[*processed_len].dp = 1;
		} else {
			processed[*processed_len].dp = 0;
		}
		(*processed_len)++;
	}
}

// SCROLLING STATE
// int scroll_pos = 0;
// int scroll_counter = 0;

// SCROLL STRING FUNCTION
void scroll_step(int pos, display_char *processed, int processed_len,
		int cpuFlag) {
	int len = processed_len;
	unsigned char display[6];

	int offset = 0;
	char cpu_str[4];

	INT8U cpu = OSCPUUsage;

	if (cpuFlag) {
		offset = 2;
		if (cpu > 99) {
			cpu = 99;
		}
		cpu_str[0] = (cpu / 100) + '0';
		cpu_str[1] = ((cpu / 10) % 10) + '0';
		cpu_str[2] = (cpu % 10) + '0';
		cpu_str[3] = '\0';

		display[0] = seg_char(cpu_str[1], 0);
		display[1] = seg_char(cpu_str[2], 0);
	}

	// Fill 6 display slots
	for (int j = 0 + offset; j < DISPLAY_WIDTH; j++) {
		int char_idx = pos + j - DISPLAY_WIDTH - offset;

		if (char_idx >= 0 && char_idx < len) {
			display[j] = seg_char(processed[char_idx].c,
					processed[char_idx].dp);
		} else {
			display[j] = seg_char(' ', 0);
		}
	}
	// Write to displays (HEX5 is leftmost = display[5])
	write_hex(display[0], display[1], display[2], display[3], display[4],
			display[5]);
}

//void run_scroll(display_char *processed, int processed_len) {
//	int scroll_pos = 0;
//	int scroll_counter = 0;
//
//	int total = processed_len + DISPLAY_WIDTH;
//
//	while (1) {
//		int sw = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE);
//
//		// mirror switches to LEDs (debugging sanity saver)
//		IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, sw);
//
//		// SW0 OFF -> clear display
//		if (!(sw & 0x1)) {
//			clear_hex();
//			scroll_pos = 0;
//			scroll_counter = 0;
//		} else {
//			// always display current frame
//			scroll_step(scroll_pos, processed, &processed_len);
//
//			// SW1 controls scrolling
//			if (sw & 0x2) {
//				scroll_counter++;
//
//				// speed control (you can tweak this)
//				int speed = 100;
//
//				// example: use more switches for speed control
//				if (sw & 0x4)
//					speed = 50;
//				if (sw & 0x8)
//					speed = 20;
//
//				if (scroll_counter >= speed) {
//					scroll_counter = 0;
//					scroll_pos++;
//
//					if (scroll_pos >= total) {
//						scroll_pos = 0;
//					}
//				}
//			}
//			// else: paused (just keeps displaying current position)
//		}
//
//		// small delay (very scientific, I know)
//		for (volatile int i = 0; i < 1000; i++)
//			;
//	}
//}

// MAIN
//int led_hex_main(char *text) {
//	// adjust to change scroll speed
//    int scroll_speed = 100;
//    process_string(text);
//    clear_hex();
//    int total = processed_len + DISPLAY_WIDTH; // Total scroll positions
//    printf("System ready. Flip switches to display messages.\n");
//
//    while (1) {
//        int sw = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE);
//        // Mirror switches to LEDs to see what's active
//		IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, sw);
//        // SW0 off   everything off
//        if (!(sw & 0x1)) {
//            clear_hex();
//            scroll_pos = 0;
//            scroll_counter = 0;
//        }
//        // SW0 on - scroll/pause message
//        else {
//        	// always display current position
//        	scroll_step(scroll_pos);
//        	// SW0 + SW1 - scroll message
//        	if (sw & 0x2) {
//        		scroll_counter++;
//        		if (scroll_counter >= scroll_speed) {
//        			scroll_counter = 0;
//        			scroll_pos++;
//        			// wrap around
//					if (scroll_pos >= total) {
//						scroll_pos = 0;
//					}
//        		}
//        	}
//        	// SW0 + !SW1 - pause display
//        }
//        // small delay to prevent cpu hogging
//        volatile int delay;
//        for (delay = 0; delay < 1000; delay++);
//    }
//    return 0;
//}
