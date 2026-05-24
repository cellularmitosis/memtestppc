/*
 * WAVE 0 SMOKE TEST — throwaway. Replaced by the real ported memtest86+ v2.00
 * main.c in Wave 5.
 *
 * Purpose: prove the PPC framebuffer substrate (OF framebuffer discovery,
 * palette setup, 8x16 font blit, the vga_buf -> fb_render_cell contract) lights
 * up a real screen under QEMU/OF before any memtest code exists. It writes
 * directly into vga_buf (cprint/lib.c don't exist yet) and refreshes.
 */

#include "display.h"
#include "ofw.h"

/* Local cell writer — stands in for cprint until lib.c is ported in Wave 2. */
static void put(int row, int col, const char *s, unsigned char attr)
{
    int i;
    for (i = 0; s[i] && (col + i) < SCREEN_WIDTH; i++) {
        unsigned char *cell = &vga_buf[(row * SCREEN_WIDTH + col + i) * 2];
        cell[0] = (unsigned char)s[i];
        cell[1] = attr;
    }
}

int main(void)
{
    int i;

    if (display_init() < 0) {
        ofw_puts("display_init failed: no framebuffer found\r\n");
        ofw_exit();
        for (;;) ;
    }

    /* Green title bar (attr 0x20 = black on green), like the real TUI. */
    for (i = 0; i < SCREEN_WIDTH; i++)
        vga_buf[(0 * SCREEN_WIDTH + i) * 2 + 1] = 0x20;
    put(0, 2, "memtestppc+ : Wave 0 framebuffer smoke test", 0x20);

    /* Body text in memtest86+'s light-gray-on-blue (0x17). */
    put(2, 2, "If you can read this, the OF framebuffer + 8x16 font work.", 0x17);
    put(3, 2, "vga_buf -> fb_render_cell contract is live. Substrate OK.", 0x17);

    /* Palette check: 16 cells, fg color 0..F on a blue background. */
    put(5, 2, "Palette 0-F:", 0x17);
    for (i = 0; i < 16; i++) {
        unsigned char *cell = &vga_buf[(5 * SCREEN_WIDTH + 15 + i) * 2];
        cell[0] = "0123456789ABCDEF"[i];
        cell[1] = (unsigned char)(0x10 | i); /* blue bg, fg = i */
    }

    /* Reverse-video bottom line (attr 0x71) to exercise that path. */
    for (i = 0; i < SCREEN_WIDTH; i++)
        vga_buf[(24 * SCREEN_WIDTH + i) * 2 + 1] = 0x71;
    put(24, 2, "<this row is reverse-video 0x71>", 0x71);

    display_refresh();

    for (;;) ;
    return 0;
}
