/*
 * Framebuffer display layer for memtestppc+
 *
 * Maintains an 80x25 character+attribute buffer identical to the x86
 * VGA text mode format, and renders it to the Open Firmware framebuffer
 * using the embedded IBM VGA 8x16 font.
 */

#include "display.h"
#include "memtest.h"
#include "ofw.h"
#include "font_vga.h"

/* The virtual VGA text buffer (80*25*2 = 4000 bytes) */
unsigned char vga_buf[SCREEN_HEIGHT * SCREEN_WIDTH * 2];

/* Framebuffer state */
static unsigned char *fb_addr;
static int fb_width, fb_height, fb_depth, fb_linesize;

/* Text area offset within framebuffer (to center the 640x400 text area) */
static int text_offset_x;
static int text_offset_y;

/*
 * Standard VGA 16-color palette (RGB888, stored as 0x00RRGGBB)
 * These are the exact CGA/VGA colors.
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
        /* 8-bit indexed: map RGB to nearest VGA color index */
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
 * Set up the OF framebuffer color map for 8-bit mode.
 * We call the screen device's "color!" method to set palette entries.
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
     * Apple OF color! signature: ( index r g b -- )
     * call-method args go onto the Forth stack with args[2] at bottom,
     * args[N] at top. So we need: args[2]=index, args[3]=r, args[4]=g, args[5]=b
     * which gives Forth stack (top-first): b g r index — i.e. ( index r g b -- )
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

void display_render_cell(int row, int col)
{
    int buf_offset = (row * SCREEN_WIDTH + col) * 2;
    unsigned char ch = vga_buf[buf_offset];
    unsigned char attr = vga_buf[buf_offset + 1];

    int fg_idx = attr & 0x0f;
    int bg_idx = (attr >> 4) & 0x07;

    unsigned long fg_rgb = vga_palette[fg_idx];
    unsigned long bg_rgb = vga_palette[bg_idx];

    const unsigned char *glyph = &font_vga_8x16[(unsigned int)ch * CHAR_HEIGHT];

    int px = text_offset_x + col * CHAR_WIDTH;
    int py = text_offset_y + row * CHAR_HEIGHT;

    int y, x;

    if (fb_depth == 8) {
        for (y = 0; y < CHAR_HEIGHT; y++) {
            unsigned char bits = glyph[y];
            unsigned char *dst = fb_addr + (py + y) * fb_linesize + px;
            for (x = 0; x < CHAR_WIDTH; x++) {
                *dst++ = (bits & (0x80 >> x)) ? fg_idx : bg_idx;
            }
        }
    } else if (fb_depth == 32) {
        for (y = 0; y < CHAR_HEIGHT; y++) {
            unsigned char bits = glyph[y];
            unsigned long *dst = (unsigned long *)(fb_addr + (py + y) * fb_linesize + px * 4);
            for (x = 0; x < CHAR_WIDTH; x++) {
                *dst++ = (bits & (0x80 >> x)) ? fg_rgb : bg_rgb;
            }
        }
    } else {
        for (y = 0; y < CHAR_HEIGHT; y++) {
            unsigned char bits = glyph[y];
            for (x = 0; x < CHAR_WIDTH; x++) {
                fb_putpixel(px + x, py + y,
                    (bits & (0x80 >> x)) ? fg_rgb : bg_rgb);
            }
        }
    }
}

int display_init(void)
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

    /* Center the 640x400 text area in the framebuffer */
    text_offset_x = (fb_width - SCREEN_WIDTH * CHAR_WIDTH) / 2;
    text_offset_y = (fb_height - SCREEN_HEIGHT * CHAR_HEIGHT) / 2;
    if (text_offset_x < 0) text_offset_x = 0;
    if (text_offset_y < 0) text_offset_y = 0;

    /* Set up 8-bit palette if needed */
    fb_setup_palette();

    /* Initialize text buffer: spaces on blue background, white foreground */
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga_buf[i * 2] = ' ';
        vga_buf[i * 2 + 1] = 0x17; /* white on blue */
    }

    /*
     * Fill entire framebuffer with black first, then render text area on top.
     * Done after palette setup so index 0 = black is in effect.
     */
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
    int row, col;
    for (row = 0; row < SCREEN_HEIGHT; row++) {
        for (col = 0; col < SCREEN_WIDTH; col++) {
            display_render_cell(row, col);
        }
    }
}

/*
 * The original memtest86+ display functions, adapted.
 * These write to our vga_buf[] and immediately render the affected cells.
 */

void cplace(int y, int x, const char c)
{
    if (y < 0 || y >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH) return;
    vga_buf[(y * SCREEN_WIDTH + x) * 2] = c;
    display_render_cell(y, x);
}

void cprint(int y, int x, const char *text)
{
    int i;
    for (i = 0; text[i] && (x + i) < SCREEN_WIDTH; i++) {
        vga_buf[(y * SCREEN_WIDTH + x + i) * 2] = text[i];
        display_render_cell(y, x + i);
    }
}

void dprint(int y, int x, unsigned long val, int len, int right)
{
    unsigned long j, k;
    int i, flag = 0;
    char buf[18];

    if (val > 999999999 || len > 9) return;

    for (i = 0, j = 1; i < len - 1; i++) j *= 10;

    if (!right) {
        for (i = 0; j > 0; j /= 10) {
            k = val / j;
            if (k > 9) { j *= 100; continue; }
            if (flag || k || j == 1) {
                buf[i++] = k + '0';
                flag++;
            } else {
                buf[i++] = ' ';
            }
            val -= k * j;
        }
    } else {
        for (i = 0; j > 0; j /= 10) {
            k = val / j;
            if (k > 9) { j *= 100; continue; }
            buf[i++] = (flag || k || j == 1) ? k + '0' : '0';
            if (k) flag = 1;
            val -= k * j;
        }
    }
    buf[i] = 0;
    cprint(y, x, buf);
}

void hprint(int y, int x, unsigned long val)
{
    hprint2(y, x, val, 8);
}

void hprint2(int y, int x, unsigned long val, int digits)
{
    unsigned long j;
    int i, idx, flag = 0;
    char buf[18];

    for (i = 0, idx = 0; i < 8; i++) {
        j = val >> (28 - (4 * i));
        j &= 0xf;
        if (j < 10) {
            if (flag || j || i == 7) {
                buf[idx++] = j + '0';
                flag++;
            } else {
                buf[idx++] = '0';
            }
        } else {
            buf[idx++] = j + 'a' - 10;
            flag++;
        }
    }
    if (digits > 8) digits = 8;
    if (flag > digits) digits = flag;
    buf[idx] = 0;
    cprint(y, x, buf + (idx - digits));
}

void aprint(int y, int x, unsigned long page)
{
    if ((page << 2) < 9999) {
        dprint(y, x, page << 2, 4, 0);
        cprint(y, x + 4, "K");
    } else if ((page >> 8) < 9999) {
        dprint(y, x, (page + (1 << 7)) >> 8, 4, 0);
        cprint(y, x + 4, "M");
    } else if ((page >> 18) < 9999) {
        dprint(y, x, (page + (1 << 17)) >> 18, 4, 0);
        cprint(y, x + 4, "G");
    } else {
        dprint(y, x, (page + (1 << 27)) >> 28, 4, 0);
        cprint(y, x + 4, "T");
    }
}

void xprint(int y, int x, unsigned long val)
{
    unsigned long j;
    j = (val & 0xffc00000) >> 20;
    dprint(y, x, j, 4, 0);
    cprint(y, x + 4, "m");
    j = (val & 0xffc00) >> 10;
    dprint(y, x + 5, j, 4, 0);
    cprint(y, x + 9, "k");
    j = val & 0x3ff;
    dprint(y, x + 10, j, 4, 0);
}

void scroll(void)
{
    int i, j;

    if (v_msg_line < 23) {
        v_msg_line++;
    } else {
        /* Scroll lines LINE_SCROLL..22 up by one */
        for (i = LINE_SCROLL; i < 23; i++) {
            for (j = 0; j < SCREEN_WIDTH * 2; j++) {
                vga_buf[i * SCREEN_WIDTH * 2 + j] =
                    vga_buf[(i + 1) * SCREEN_WIDTH * 2 + j];
            }
        }
        /* Clear line 23 */
        for (j = 0; j < SCREEN_WIDTH; j++) {
            vga_buf[23 * SCREEN_WIDTH * 2 + j * 2] = ' ';
        }
        /* Re-render the scrolled region */
        for (i = LINE_SCROLL; i <= 23; i++) {
            for (j = 0; j < SCREEN_WIDTH; j++) {
                display_render_cell(i, j);
            }
        }
    }
}

void clear_scroll(void)
{
    int i, j;
    for (i = LINE_HEADER; i < 24; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            vga_buf[i * SCREEN_WIDTH * 2 + j * 2] = ' ';
            vga_buf[i * SCREEN_WIDTH * 2 + j * 2 + 1] = 0x17;
        }
    }
    for (i = LINE_HEADER; i < 24; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            display_render_cell(i, j);
        }
    }
}

void footer(void)
{
    cprint(24, 0, "(ESC)reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock");
}
