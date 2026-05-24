/*
 * Framebuffer backend for memtestppc+ (PPC-native; see display.h).
 *
 * Maintains the 80x25 char+attribute buffer (vga_buf, the 0xb8000/SCREEN_ADR
 * analog) and blits cells to the Open Firmware framebuffer using the embedded
 * IBM VGA 8x16 font. This is the hardware-replacement layer; the text-drawing
 * primitives (cprint/dprint/scroll/...) live in the ported lib.c and reach the
 * screen by routing through fb_render_cell() (via lib.c's ttyprint()).
 */

#include "display.h"
#include "ofw.h"
#include "font_vga.h"

/* The virtual VGA text buffer (80*25*2 = 4000 bytes). */
unsigned char vga_buf[SCREEN_HEIGHT * SCREEN_WIDTH * 2];

/* Framebuffer state. */
static unsigned char *fb_addr;
static int fb_width, fb_height, fb_depth, fb_linesize;

/* Offset to center the 640x400 text area within the framebuffer. */
static int text_offset_x;
static int text_offset_y;

/*
 * Standard VGA 16-color palette (RGB888, stored as 0x00RRGGBB) — the exact
 * CGA/VGA colors.
 */
const unsigned long vga_palette[16] = {
    0x00000000,  /* 0 black */
    0x000000aa,  /* 1 blue */
    0x0000aa00,  /* 2 green */
    0x0000aaaa,  /* 3 cyan */
    0x00aa0000,  /* 4 red */
    0x00aa00aa,  /* 5 magenta */
    0x00aa5500,  /* 6 brown */
    0x00aaaaaa,  /* 7 light gray */
    0x00555555,  /* 8 dark gray */
    0x005555ff,  /* 9 light blue */
    0x0055ff55,  /* A light green */
    0x0055ffff,  /* B light cyan */
    0x00ff5555,  /* C light red */
    0x00ff55ff,  /* D light magenta */
    0x00ffff55,  /* E yellow */
    0x00ffffff,  /* F white */
};

static void fb_putpixel(int x, int y, unsigned long rgb)
{
    unsigned char *p;

    if (x < 0 || x >= fb_width || y < 0 || y >= fb_height)
        return;

    p = fb_addr + y * fb_linesize + x * (fb_depth / 8);

    switch (fb_depth) {
    case 8:
        /* 8-bit indexed: map RGB to nearest VGA color index. */
        {
            int best = 0;
            int best_dist = 0x7fffffff;
            int i;
            for (i = 0; i < 16; i++) {
                int dr = (int)((rgb >> 16) & 0xff) - (int)((vga_palette[i] >> 16) & 0xff);
                int dg = (int)((rgb >> 8) & 0xff) - (int)((vga_palette[i] >> 8) & 0xff);
                int db = (int)(rgb & 0xff) - (int)(vga_palette[i] & 0xff);
                int dist = dr*dr + dg*dg + db*db;
                if (dist < best_dist) {
                    best_dist = dist;
                    best = i;
                }
            }
            *p = (unsigned char)best;
        }
        break;
    case 15:
    case 16:
        {
            unsigned short val;
            if (fb_depth == 15) {
                val = ((rgb >> 9) & 0x7c00) | ((rgb >> 6) & 0x03e0) | ((rgb >> 3) & 0x001f);
            } else {
                val = ((rgb >> 8) & 0xf800) | ((rgb >> 5) & 0x07e0) | ((rgb >> 3) & 0x001f);
            }
            *(unsigned short *)p = val;
        }
        break;
    case 32:
        *(unsigned long *)p = rgb;
        break;
    }
}

static void fb_fillrect(int x, int y, int w, int h, unsigned long rgb)
{
    int ix, iy;
    for (iy = y; iy < y + h; iy++) {
        for (ix = x; ix < x + w; ix++) {
            fb_putpixel(ix, iy, rgb);
        }
    }
}

/*
 * Set up the OF framebuffer color map for 8-bit mode by calling the screen
 * device's "color!" method per palette entry.
 */
static void fb_setup_palette(void)
{
    ofw_ihandle_t screen_ih;
    int i;
    struct {
        const char *service;
        int nargs;
        int nret;
        unsigned long args[7];
    } cargs;
    typedef int (*ofw_entry_t)(void *);

    if (fb_depth != 8)
        return;

    screen_ih = ofw_open("screen");
    if (screen_ih == -1 || screen_ih == 0)
        return;

    /*
     * Apple OF color! signature: ( index r g b -- ). call-method args go onto
     * the Forth stack with args[2] at bottom, args[N] at top, so we pass
     * args[2]=index, args[3]=b, args[4]=g, args[5]=r to get the empirical BGR
     * order the iBook's ATI Rage wants (see docs/sessions/004).
     */
    for (i = 0; i < 16; i++) {
        int r = (vga_palette[i] >> 16) & 0xff;
        int g = (vga_palette[i] >> 8) & 0xff;
        int b = vga_palette[i] & 0xff;
        cargs.service = "call-method";
        cargs.nargs = 6;
        cargs.nret = 1;
        cargs.args[0] = (unsigned long)"color!";
        cargs.args[1] = (unsigned long)screen_ih;
        cargs.args[2] = (unsigned long)i;
        cargs.args[3] = (unsigned long)b;
        cargs.args[4] = (unsigned long)g;
        cargs.args[5] = (unsigned long)r;
        cargs.args[6] = 0;
        ((ofw_entry_t)_ofw_cif_store)(&cargs);
    }
}

void fb_render_cell(int y, int x)
{
    int buf_offset = (y * SCREEN_WIDTH + x) * 2;
    unsigned char ch = vga_buf[buf_offset];
    unsigned char attr = vga_buf[buf_offset + 1];

    int fg_idx = attr & 0x0f;
    int bg_idx = (attr >> 4) & 0x07;

    unsigned long fg_rgb = vga_palette[fg_idx];
    unsigned long bg_rgb = vga_palette[bg_idx];

    const unsigned char *glyph = &font_vga_8x16[(unsigned int)ch * CHAR_HEIGHT];

    int px = text_offset_x + x * CHAR_WIDTH;
    int py = text_offset_y + y * CHAR_HEIGHT;

    int row, col;

    if (fb_depth == 8) {
        for (row = 0; row < CHAR_HEIGHT; row++) {
            unsigned char bits = glyph[row];
            unsigned char *dst = fb_addr + (py + row) * fb_linesize + px;
            for (col = 0; col < CHAR_WIDTH; col++) {
                *dst++ = (bits & (0x80 >> col)) ? fg_idx : bg_idx;
            }
        }
    } else if (fb_depth == 32) {
        for (row = 0; row < CHAR_HEIGHT; row++) {
            unsigned char bits = glyph[row];
            unsigned long *dst = (unsigned long *)(fb_addr + (py + row) * fb_linesize + px * 4);
            for (col = 0; col < CHAR_WIDTH; col++) {
                *dst++ = (bits & (0x80 >> col)) ? fg_rgb : bg_rgb;
            }
        }
    } else {
        for (row = 0; row < CHAR_HEIGHT; row++) {
            unsigned char bits = glyph[row];
            for (col = 0; col < CHAR_WIDTH; col++) {
                fb_putpixel(px + col, py + row,
                    (bits & (0x80 >> col)) ? fg_rgb : bg_rgb);
            }
        }
    }
}

int fb_init(void)
{
    struct ofw_fb_info info;
    int i;

    if (ofw_get_fb_info(&info) < 0)
        return -1;

    fb_addr = (unsigned char *)info.addr;
    fb_width = info.width;
    fb_height = info.height;
    fb_depth = info.depth;
    fb_linesize = info.linesize;

    /* Center the 640x400 text area in the framebuffer. */
    text_offset_x = (fb_width - SCREEN_WIDTH * CHAR_WIDTH) / 2;
    text_offset_y = (fb_height - SCREEN_HEIGHT * CHAR_HEIGHT) / 2;
    if (text_offset_x < 0) text_offset_x = 0;
    if (text_offset_y < 0) text_offset_y = 0;

    /* Set up 8-bit palette if needed. */
    fb_setup_palette();

    /* Initialize text buffer: spaces, light-gray on blue (memtest86+'s 0x17). */
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga_buf[i * 2] = ' ';
        vga_buf[i * 2 + 1] = 0x17;
    }

    /* Black the whole framebuffer, then render the text area on top. */
    {
        unsigned char *p = fb_addr;
        int total = fb_height * fb_linesize;
        if (fb_depth == 8) {
            for (i = 0; i < total; i++)
                *p++ = 0;
        } else {
            fb_fillrect(0, 0, fb_width, fb_height, 0x00000000);
        }
    }

    display_refresh();
    return 0;
}

void display_refresh(void)
{
    int y, x;
    for (y = 0; y < SCREEN_HEIGHT; y++) {
        for (x = 0; x < SCREEN_WIDTH; x++) {
            fb_render_cell(y, x);
        }
    }
}
