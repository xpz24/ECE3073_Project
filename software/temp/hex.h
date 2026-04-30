#ifndef HEX_DISPLAY_H
#define HEX_DISPLAY_H

#define DISPLAY_WIDTH 6

typedef struct {
    char c;
    int dp;
} display_char;

unsigned char seg_char(char c, int dp);

void write_hex(unsigned char s5, unsigned char s4, unsigned char s3,
               unsigned char s2, unsigned char s1, unsigned char s0);

void clear_hex(void);
void show_all_dash(void);

void process_string(const char *text, display_char *processed, int *processed_len);
void scroll_step(int pos, display_char *processed, int processed_len, int cpuFlag);

#endif
