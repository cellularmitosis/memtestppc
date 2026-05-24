#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * VGA text-mode compatible display layer for memtestppc+
 *
 * We maintain an 80x25 character+attribute buffer identical in format
 * to the x86 VGA text buffer (2 bytes per cell: char, attr).
 * After writes, we render changed cells to the framebuffer using
 * the embedded IBM VGA 8x16 font.
 *
 * VGA attribute byte: bits 0-3 = foreground, 4-6 = background, 7 = blink
 */

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define CHAR_WIDTH    8
#define CHAR_HEIGHT   16

/* Our virtual "VGA text buffer" — replaces SCREEN_ADR from memtest86+ */
extern unsigned char vga_buf[SCREEN_HEIGHT * SCREEN_WIDTH * 2];
#define SCREEN_ADR ((unsigned long)vga_buf)

/* VGA color attribute values */
#define VGA_BLACK       0x0
#define VGA_BLUE        0x1
#define VGA_GREEN       0x2
#define VGA_CYAN        0x3
#define VGA_RED         0x4
#define VGA_MAGENTA     0x5
#define VGA_BROWN       0x6
#define VGA_LIGHT_GRAY  0x7
#define VGA_DARK_GRAY   0x8
#define VGA_LIGHT_BLUE  0x9
#define VGA_LIGHT_GREEN 0xA
#define VGA_LIGHT_CYAN  0xB
#define VGA_LIGHT_RED   0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW      0xE
#define VGA_WHITE       0xF

/* Standard VGA 16-color palette (RGB888) */
extern const unsigned long vga_palette[16];

/* Initialize display: discover framebuffer, clear screen */
int display_init(void);

/* Render the entire text buffer to the framebuffer */
void display_refresh(void);

/* Render a single character cell at text position (col, row) */
void display_render_cell(int row, int col);

/* Print functions — write to the text buffer AND render */
void cprint(int y, int x, const char *text);
void cplace(int y, int x, const char c);
void hprint(int y, int x, unsigned long val);
void hprint2(int y, int x, unsigned long val, int digits);
void dprint(int y, int x, unsigned long val, int len, int right);
void aprint(int y, int x, unsigned long page);
void xprint(int y, int x, unsigned long val);

/* Scroll the error message area */
void scroll(void);
void clear_scroll(void);
void footer(void);

#endif
