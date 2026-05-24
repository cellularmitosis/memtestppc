/*
 * WAVE 2 CHECKPOINT STUB — throwaway. Replaced by the real ported memtest86+
 * v2.00 main.c in Wave 5.
 *
 * Purpose: prove the Wave-2 rendering core end to end — that the VERBATIM
 * upstream print primitives (cprint/dprint/hprint/aprint/footer in lib.c) reach
 * the framebuffer through the unchanged path cprint -> tty_print_line
 * (screen_buffer.c) -> ttyprint (lib.c, the one rewritten leaf) -> fb_render_cell
 * (display.c). Also exercises the attr-poke -> explicit fb_render_cell path used
 * by init.c/error.c for the title/footer bars.
 *
 * Defines `v` (struct vars) here because lib.c references it; the real main.c
 * owns this global in Wave 5.
 */

#include "test.h"
#include "display.h"
#include "ofw.h"

struct vars variables;
struct vars * const v = &variables;

int main(void)
{
    int i;

    if (display_init() < 0) {
        ofw_puts("display_init failed: no framebuffer found\r\n");
        ofw_exit();
        for (;;) ;
    }

    /* Green title bar: poke attr 0x20 across the row, render it (attr-only
     * change needs an explicit render), then cprint the title over it. */
    for (i = 0; i < SCREEN_WIDTH; i++)
        vga_buf[(0 * SCREEN_WIDTH + i) * 2 + 1] = 0x20;
    for (i = 0; i < SCREEN_WIDTH; i++)
        fb_render_cell(0, i);
    cprint(0, 2, "memtestppc+ : Wave 2 lib.c render-path check");

    /* The actual proof: verbatim cprint/dprint/hprint/aprint via ttyprint. */
    cprint(2, 2, "cprint -> tty_print_line -> ttyprint -> fb_render_cell:");
    cprint(3, 4, "If you can read this, the VERBATIM print path works.");

    cprint(5, 2, "dprint 12345 =");
    dprint(5, 18, 12345, 5, 0);
    cprint(6, 2, "hprint 0xdeadbeef =");
    hprint(6, 22, 0xdeadbeef);
    cprint(7, 2, "aprint 0x4000 pages =");
    aprint(7, 24, 0x4000);

    /* Reverse-video footer bar (attr 0x71) + the verbatim v2.00 footer() text. */
    for (i = 0; i < SCREEN_WIDTH; i++)
        vga_buf[(24 * SCREEN_WIDTH + i) * 2 + 1] = 0x71;
    for (i = 0; i < SCREEN_WIDTH; i++)
        fb_render_cell(24, i);
    footer();

    for (;;) ;
    return 0;
}
