/* --*- C -*--
 *
 * By Jani Averbach, Jaa@iki.fi, 2001
 *
 * Released under version 2 of the Gnu Public License.
 *
 * $Author: jaa $
 * $Revision: 1.6 $
 * $Date: 2001/03/29 09:00:30 $
 * $Source: /home/raid/cvs/memtest86/screen_buffer.h,v $  (for CVS)
 *
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 screen_buffer.h to
 * PowerPC / Open Firmware. This file is platform-neutral plumbing (a char-only
 * shadow of the screen + change-detection helpers) and is imported VERBATIM —
 * nothing here is x86-specific. The only display leaf, ttyprint(), is declared
 * in test.h and defined in lib.c (where it blits to the framebuffer). See
 * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md.
 */
#ifndef SCREEN_BUFFER_H_1D10F83B_INCLUDED
#define SCREEN_BUFFER_H_1D10F83B_INCLUDED

#include "config.h"

char get_scrn_buf(const int y, const int x);
void set_scrn_buf(const int y, const int x, const char val);
void clear_screen_buf(void);
void tty_print_region(const int pi_top,const int pi_left, const int pi_bottom,const int pi_right);
void tty_print_line(int y, int x, const char *text);
void tty_print_screen(void);
void print_error(char *pstr);
#endif /* SCREEN_BUFFER_H_1D10F83B_INCLUDED */
