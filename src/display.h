#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * Framebuffer backend for memtestppc+ (PPC-native; no memtest86+ counterpart).
 *
 * x86 memtest86+ relies on VGA text-mode hardware: it writes char+attr cells to
 * the text buffer at 0xb8000 and the hardware paints glyphs automatically. PPC
 * has only a linear Open Firmware framebuffer, so this backend stands in for
 * that hardware:
 *
 *   - vga_buf[] is our 0xb8000 analog: an 80x25 char+attribute buffer in the
 *     exact x86 text-mode format (2 bytes/cell: char, attr). The ported upstream
 *     code writes into it verbatim via SCREEN_ADR.
 *   - fb_render_cell(y, x) reads one cell from vga_buf and blits its glyph to the
 *     framebuffer using the embedded IBM VGA 8x16 font. This is the single hook
 *     the ported lib.c routes all screen output through (its ttyprint() — the
 *     former serial-console echo — becomes a loop over fb_render_cell).
 *
 * VGA attribute byte: bits 0-3 = foreground, 4-6 = background, 7 = blink.
 */

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define CHAR_WIDTH    8
#define CHAR_HEIGHT   16

/* The virtual VGA text buffer — the 0xb8000 / SCREEN_ADR analog. */
extern unsigned char vga_buf[SCREEN_HEIGHT * SCREEN_WIDTH * 2];
#define SCREEN_ADR ((unsigned long)vga_buf)

/* Standard VGA 16-color palette (RGB888, 0x00RRGGBB). */
extern const unsigned long vga_palette[16];

/* Bring up the OF framebuffer hardware: discover it, set the palette, black the
 * physical screen. <0 on failure. (Named fb_init, not display_init, because the
 * ported init.c owns the upstream display_init() that draws the TUI content.) */
int fb_init(void);

/* Blit one text cell (char+attr from vga_buf) to the framebuffer. */
void fb_render_cell(int y, int x);

/* Re-blit the entire text buffer to the framebuffer. */
void display_refresh(void);

#endif /* DISPLAY_H */
