/* lib.c - MemTest-86  Version 3.0
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 * ----------------------------------------------------
 * MemTest86+ V2.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.canardplus.com - http://www.memtest.org
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 lib.c to
 * PowerPC / Open Firmware (single-CPU). Discipline (see
 * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md): start from
 * the verbatim v2.00 file; comment out x86 leaf code with a one-line reason;
 * add PPC code inline marked "PPC:"; never delete; keep buildable. Nothing
 * from the old attic/src-v0.01 hack is inherited.
 *
 * THE ONE KEY EDIT in this file is ttyprint(): upstream it emits an ANSI
 * cursor escape + text to a serial console; here it becomes the framebuffer
 * renderer (loops fb_render_cell over the run), so cprint/scroll/etc. stay
 * byte-verbatim and "just work".
*/

/* x86: io.h = x86 port-I/O (inb/outb) primitives; serial.h = x86 16550 UART
 * register defs (UART_LSR etc.) — N/A on PPC/OF. The only consumers were the
 * PC-keyboard get_key(), the serial_echo_* console, and beep — all commented
 * out below. Replaced by ofw.h (the OF client interface = the io.h/BIOS
 * replacement). */
/* #include "io.h" */
/* #include "serial.h" */
#include "test.h"
#include "config.h"
#include "screen_buffer.h"
/* PPC: OF client interface for check_input()'s stdin poll and reboot(). */
#include "ofw.h"


int slock = 0, lsr = 0;
short serial_cons = SERIAL_CONSOLE_DEFAULT;

#if SERIAL_TTY != 0 && SERIAL_TTY != 1
#error Bad SERIAL_TTY. Only ttyS0 and ttyS1 are supported.
#endif
short serial_tty = SERIAL_TTY;
const short serial_base_ports[] = {0x3f8, 0x2f8};

#if ((115200%SERIAL_BAUD_RATE) != 0)
#error Bad default baud rate
#endif
int serial_baud_rate = SERIAL_BAUD_RATE;
unsigned char serial_parity = 0;
unsigned char serial_bits = 8;

char buf[18];

struct ascii_map_str {
	int ascii;
	int keycode;
};

/* x86: codes[] is the x86 exception-vector name table consumed only by
 * inter() (the x86 interrupt handler) — N/A on PPC/OF (we install no x86 IDT).
 * Commented out alongside inter() below to keep the diff legible. */
/*
char *codes[] = {
	"  Divide",
	"   Debug",
	"     NMI",
	"  Brkpnt",
	"Overflow",
	"   Bound",
	"  Inv_Op",
	" No_Math",
	"Double_Fault",
	"Seg_Over",
	" Inv_TSS",
	"  Seg_NP",
	"Stack_Fault",
	"Gen_Prot",
	"Page_Fault",
	"   Resvd",
	"     FPE",
	"Alignment",
	" Mch_Chk",
	"SIMD FPE"
};
*/

/* x86: struct eregs is the x86 trap-frame (esp/ebp/.../eflag) pushed by the
 * IDT stubs and consumed only by inter() — N/A on PPC/OF. Commented out with
 * inter() below. test.h forward-declares "struct eregs;" so the (also
 * commented) prototype stays resolvable. */
/*
struct eregs {
	ulong esp;
	ulong ebp;
	ulong esi;
	ulong edi;
	ulong edx;
	ulong ecx;
	ulong ebx;
	ulong eax;
	ulong vect;
	ulong code;
	ulong eip;
	ulong cs;
	ulong eflag;
};
*/

int memcmp(const void *s1, const void *s2, ulong count)
{
	const unsigned char *src1 = s1, *src2 = s2;
	int i;
	for(i = 0; i < count; i++) {
		if (src1[i] != src2[i]) {
			return (int)src1[i] - (int)src2[i];
		}
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, ulong n) {
	signed char res = 0;
	while (n) {
		res = *s1 - *s2;
		if (res != 0)
			return res;
		if (*s1 == '\0')
			return 0;
		++s1, ++s2;
		--n;
	}
	return res;
}

void *memmove(void *dest, const void *src, ulong n)
{
	long i;
	char *d = (char *)dest, *s = (char *)src;

	/* If src == dest do nothing */
	if (dest < src) {
		for(i = 0; i < n; i++) {
			d[i] = s[i];
		}
	}
	else if (dest > src) {
		for(i = n -1; i >= 0; i--) {
			d[i] = s[i];
		}
	}
	return dest;
}

char toupper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c + 'A' -'a';
	else
		return c;
}

int isdigit(char c)
{
	return c >= '0' && c <= '9';
}

int isxdigit(char c)
{
	return isdigit(c) || (toupper(c) >= 'A' && toupper(c) <= 'F'); }

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {
	unsigned long result = 0, value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if (toupper(*cp) == 'X' && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	} else if (base == 16) {
		if (cp[0] == '0' && toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit(*cp) &&
		(value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}


/*
 * Scroll the error message area of the screen as needed
 * Starts at line LINE_SCROLL and ends at line 23
 */
void scroll(void)
{
	int i, j;
	char *s, tmp;

	/* Only scroll if at the bottom of the screen */
	if (v->msg_line < 23) {
		v->msg_line++;
	} else {
		/* If scroll lock is on, loop till it is cleared */
		while (slock) {
			check_input();
		}
		for (i=LINE_SCROLL; i<23; i++) {
			s = (char *)(SCREEN_ADR + ((i+1) * 160));
			for (j=0; j<160; j+=2, s+=2) {
				*(s-160) = *s;
				tmp = get_scrn_buf(i+1, j/2);
				set_scrn_buf(i, j/2, tmp);
			}
		}
		/* Clear the newly opened line */
		s = (char *)(SCREEN_ADR + (23 * 160));
		for (j=0; j<80; j++) {
			*s = ' ';
			set_scrn_buf(23, j, ' ');
			s += 2;
		}
		tty_print_region(LINE_SCROLL, 0, 23, 79);
	}
}

/*
 * Clear scroll region
 */
void clear_scroll(void)
{
	int i;
	char *s;

	s = (char*)(SCREEN_ADR+LINE_HEADER*160);
        for(i=0; i<80*(24-LINE_HEADER); i++) {
                *s++ = ' ';
                *s++ = 0x17;
        }
}

/*
 * Print characters on screen
 */
void cprint(int y, int x, const char *text)
{
	register int i;
	char *dptr;

	dptr = (char *)(SCREEN_ADR + (160*y) + (2*x));
	for (i=0; text[i]; i++) {
		*dptr = text[i];
		dptr += 2;
	}
	tty_print_line(y, x, text);
}

void itoa(char s[], int n)
{
	int i, sign;

	if((sign = n) < 0)
		n = -n;
	i=0;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if(sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

void reverse(char s[])
{
	int c, i, j;
	for(j = 0; s[j] != 0; j++)
		;

	for(i=0, j = j - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/*
 * Print a people friendly address
 */
void aprint(int y, int x, ulong page)
{
	/* page is in multiples of 4K */
	if ((page << 2) < 9999) {
		dprint(y, x, page << 2, 4, 0);
		cprint(y, x+4, "K");
	}
	else if ((page >>8) < 9999) {
		dprint(y, x, (page  + (1 << 7)) >> 8, 4, 0);
		cprint(y, x+4, "M");
	}
	else if ((page >>18) < 9999) {
		dprint(y, x, (page + (1 << 17)) >> 18, 4, 0);
		cprint(y, x+4, "G");
	}
	else {
		dprint(y, x, (page + (1 << 27)) >> 28, 4, 0);
		cprint(y, x+4, "T");
	}
}

/*
 * Print a decimal number on screen
 */
void dprint(int y, int x, ulong val, int len, int right)
{
	ulong j, k;
	int i, flag=0;

	if (val > 999999999 || len > 9) {
		return;
	}
	for(i=0, j=1; i<len-1; i++) {
		j *= 10;
	}
	if (!right) {
		for (i=0; j>0; j/=10) {
			k = val/j;
			if (k > 9) {
				j *= 100;
				continue;
			}
			if (flag || k || j == 1) {
				buf[i++] = k + '0';
				flag++;
			} else {
				buf[i++] = ' ';
			}
			val -= k * j;
		}
	} else {
		for(i=0; i<len; j/=10) {
			if (j) {
				k = val/j;
					if (k > 9) {
					j *= 100;
					len++;
					continue;
				}
				if (k == 0 && flag == 0) {
					continue;
				}
				buf[i++] = k + '0';
				val -= k * j;
			} else {
				if (flag == 0 &&  i < len-1) {
					buf[i++] = '0';
				} else {
					buf[i++] = ' ';
				}
			}
			flag++;
		}
	}
	buf[i] = 0;
	cprint(y,x,buf);
}

/*
 * Print a hex number on screen at least digits long
 */
void hprint2(int y,int x, unsigned long val, int digits)
{
        unsigned long j;
        int i, idx, flag = 0;

        for (i=0, idx=0; i<8; i++) {
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
        if (digits > 8) {
                digits = 8;
        }
        if (flag > digits) {
                digits = flag;
        }
        buf[idx] = 0;
        cprint(y,x,buf + (idx - digits));
}

/*
 * Print a hex number on screen exactly digits long
 */
void hprint3(int y,int x, unsigned long val, int digits)
{
	unsigned long j;
	int i, idx, flag = 0;


	for (i=0, idx=0; i<digits; i++) {
		j = 0xf & val;
		val /= 16;

		if (j < 10) {
			if (flag || j || i == 7) {
				buf[digits - ++idx] = j + '0';
				flag++;
			} else {
				buf[digits - ++idx] = '0';
			}
		} else {
			buf[digits - ++idx] = j + 'a' - 10;
			flag++;
		}
	}
	buf[idx] = 0;
	cprint(y,x,buf);
}

/*
 * Print a hex number on screen
 */
void hprint(int y, int x, unsigned long val)
{
	return hprint2(y, x, val, 8);
}

/*
 * Print an address in 0000m0000k0000 notation
 */
void xprint(int y,int x, ulong val)
{
	ulong j;

	j = (val & 0xffc00000) >> 20;
	dprint(y, x, j, 4, 0);
	cprint(y, x+4, "m");
	j = (val & 0xffc00) >> 10;
	dprint(y, x+5, j, 4, 0);
	cprint(y, x+9, "k");
	j = val & 0x3ff;
	dprint(y, x+10, j, 4, 0);
}

/* x86: inter() is the x86 interrupt/exception handler. It is invoked from the
 * IDT stubs with a struct eregs trap-frame, reads CR2 for page-fault address
 * via inline asm, and dumps x86 registers (eax/ebx/.../eflag) and the x86 PC
 * (eip:cs) — N/A on PPC/OF: we install no x86 IDT, and the PPC exception model
 * (DSI/ISI/program via the OF-owned vectors) is entirely different. A faithful
 * PPC exception dump would be its own workstream; for now we have no caller
 * (head.S does not route traps here), so this is commented out wholesale. The
 * codes[] table and struct eregs it relied on are commented out above. */
#if 0
/* Handle an interrupt */
void inter(struct eregs *trap_regs)
{
	int i, line;
	unsigned char *pp;
	ulong address = 0;

	/* Get the page fault address */
	if (trap_regs->vect == 14) {
		__asm__("movl %%cr2,%0":"=r" (address));
	}
#ifdef PARITY_MEM

	/* Check for a parity error */
	if (trap_regs->vect == 2) {
		parity_err(trap_regs->edi, trap_regs->esi);
		return;
	}
#endif

	/* clear scrolling region */
	pp=(unsigned char *)(SCREEN_ADR+(2*80*(LINE_SCROLL-2)));
	for(i=0; i<2*80*(24-LINE_SCROLL-2); i++, pp+=2) {
		*pp = ' ';
	}
	line = LINE_SCROLL-2;

	cprint(line, 0, "Unexpected Interrupt - Halting");
	cprint(line+2, 0, " Type: ");
	if (trap_regs->vect <= 19) {
		cprint(line+2, 7, codes[trap_regs->vect]);
	} else {
		hprint(line+2, 7, trap_regs->vect);
	}
	cprint(line+3, 0, "   PC: ");
	hprint(line+3, 7, trap_regs->eip);
	cprint(line+4, 0, "   CS: ");
	hprint(line+4, 7, trap_regs->cs);
	cprint(line+5, 0, "Eflag: ");
	hprint(line+5, 7, trap_regs->eflag);
	cprint(line+6, 0, " Code: ");
	hprint(line+6, 7, trap_regs->code);
	if (trap_regs->vect == 14) {
		/* Page fault address */
		cprint(line+7, 0, " Addr: ");
		hprint(line+7, 7, address);
	}

	cprint(line+2, 20, "eax: ");
	hprint(line+2, 25, trap_regs->eax);
	cprint(line+3, 20, "ebx: ");
	hprint(line+3, 25, trap_regs->ebx);
	cprint(line+4, 20, "ecx: ");
	hprint(line+4, 25, trap_regs->ecx);
	cprint(line+5, 20, "edx: ");
	hprint(line+5, 25, trap_regs->edx);
	cprint(line+6, 20, "edi: ");
	hprint(line+6, 25, trap_regs->edi);
	cprint(line+7, 20, "esi: ");
	hprint(line+7, 25, trap_regs->esi);
	cprint(line+8, 20, "ebp: ");
	hprint(line+8, 25, trap_regs->ebp);
	cprint(line+9, 20, "esp: ");
	hprint(line+9, 25, trap_regs->esp);

	cprint(line+1, 38, "Stack:");
	for (i=0; i<12; i++) {
		hprint(line+2+i, 38, trap_regs->esp+(4*i));
		hprint(line+2+i, 47, *(ulong*)(trap_regs->esp+(4*i)));
		hprint(line+2+i, 57, trap_regs->esp+(4*(i+12)));
		hprint(line+2+i, 66, *(ulong*)(trap_regs->esp+(4*(i+12))));
	}

	cprint(line+11, 0, "CS:EIP:                          ");
	pp = (unsigned char *)trap_regs->eip;
	for(i = 0; i < 10; i++) {
		hprint2(line+11, 8+(3*i), pp[i], 2);
	}

	while(1) {
		check_input();
	}
}
#endif

void set_cache(int val)
{
	/* x86: the original tested cpu_id (CPUID/type) to special-case the 386
	 * (no cache), then toggled the x86 cache-disable bit in cr0 via
	 * cache_off()/cache_on() (test.h) — N/A on PPC/OF. struct cpu_ident is
	 * x86 CPUID state (commented out in test.h), so its use here is gone too.
	 *   extern struct cpu_ident cpu_id;
	 *   if ((cpu_id.cpuid < 1) && (cpu_id.type == 3)) {
	 *       cprint(LINE_INFO, COL_CACHE, "none");
	 *       return;
	 *   }
	 *   switch(val) {
	 *   case 0:
	 *       cache_off();
	 *       cprint(LINE_INFO, COL_CACHE, "off");
	 *       break;
	 *   case 1:
	 *       cache_on();
	 *       cprint(LINE_INFO, COL_CACHE, " on");
	 *       break;
	 *   }
	 */
	(void)val;
	/* PPC: cache control via HID0[DCE]/[ICE] (and dcbf+sync from ppc.h) is not
	 * yet implemented. The tseq table (main.c, Wave 5) has cache=0 (uncached)
	 * and cache=1 (cached) entries; this stub lets them link. The cache stays
	 * in whatever state OF left it. TO VERIFY: the "uncached" tseq rows do not
	 * actually run uncached until HID0 control lands — flagged in the report. */
}

/* x86: get_key() polled the PS/2 controller (inb 0x64 status / inb 0x60 data)
 * with a serial-console fallback — N/A on PPC/OF (no PS/2, no UART; OF owns
 * input). The x86 original is preserved (#if 0) below for fidelity.
 *
 * PPC: read one byte from the OF stdin console (non-blocking) and map it
 * (ASCII) to the PC keycode the config-menu code switches on, via
 * ascii_to_keycode()/ser_map[] (re-enabled below). Returns 0 when no key is
 * waiting — get_config()/getval() busy-poll get_key() in a loop, so a 0 just
 * means "spin again". This is the OF-keyboard substrate that makes the
 * (c)configuration menu (Decision #6) usable. */
int get_key(void)
{
	static ofw_ihandle_t kbd_ih = 0;
	static int kbd_init = 0;
	unsigned char ch;

	if (!kbd_init) {
		kbd_ih = ofw_get_stdin();
		kbd_init = 1;
	}
	if (kbd_ih == -1 || kbd_ih == 0) {
		return 0;
	}
	if (ofw_read(kbd_ih, &ch, 1) <= 0) {
		return 0;
	}
	return ascii_to_keycode((int)ch);
}

#if 0  /* x86 PS/2 original, preserved for fidelity */
int get_key() {
	int c;

	c = inb(0x64);
	if ((c & 1) == 0) {
		if (serial_cons) {
			int comstat;
			comstat = serial_echo_inb(UART_LSR);
			if (comstat & UART_LSR_DR) {
				c = serial_echo_inb(UART_RX);
				/* Pressing '.' has same effect as 'c'
				   on a keyboard.
				   Oct 056   Dec 46   Hex 2E   Ascii .
				*/
				return (ascii_to_keycode(c));
			}
		}
		return(0);
	}
	c = inb(0x60);
	return((c));
}
#endif

/*
 * Poll for user input and act on it.
 *
 * x86: the original called get_key() (PS/2 scancodes) and switched on the
 * scancode (ESC=1 -> warm-boot via 0x472/0x64, 46='c' -> get_config,
 * 28=CR/57=SP toggle scroll lock, 0x26=^L redraw). That whole path is
 * commented out below.
 *
 * PPC: poll Open Firmware's stdin (the OF console — ADB/USB keyboard on real
 * Macs, the QEMU console under OpenBIOS). One non-blocking byte; ESC (ASCII 27)
 * reboots via ofw_reset(). The scroll-lock and config hooks are left as
 * commented stubs for Wave 6 (the footer advertises them; see HANDOFF "Footer
 * honesty"). check_input() is called from scroll() and do_tick(), so it must
 * exist and be cheap — a single ofw_read of one byte is.
 */
/* PPC: read one more byte from the OF console if one is (about to be) pending.
 * Used to disambiguate a lone ESC from a multi-byte escape sequence (arrow /
 * nav / function keys arrive as ESC '[' <final> etc.). The sequence bytes are
 * co-queued with the ESC, so this normally returns on the first read; the short
 * wall-clock deadline (via the OF timebase) only bounds the lone-ESC case so a
 * genuine ESC reboot isn't delayed by more than ~20 ms. Latency-independent —
 * does not assume a fixed per-read cost. */
static int read_pending_byte(ofw_ihandle_t ih, unsigned char *c)
{
	unsigned long freq = ofw_get_timebase_freq();
	unsigned long deadline = freq / 50;	/* ~20 ms in timebase ticks */
	unsigned long start = mftb();

	for (;;) {
		if (ofw_read(ih, c, 1) > 0) {
			return 1;
		}
		if ((mftb() - start) > deadline) {
			return 0;
		}
	}
}

void check_input(void)
{
	/* PPC: cache the stdin ihandle across calls (it never changes). */
	static ofw_ihandle_t stdin_ih = 0;
	static int stdin_init = 0;
	unsigned char ch;
	int n;

	if (!stdin_init) {
		stdin_ih = ofw_get_stdin();
		stdin_init = 1;
	}
	if (stdin_ih == -1 || stdin_ih == 0) {
		return;
	}

	/* OF "read" is non-blocking on a console: returns the bytes available
	 * (0 if none). Read a single byte. */
	n = ofw_read(stdin_ih, &ch, 1);
	if (n <= 0) {
		return;
	}

	if (ch == 27) {
		/* A lone ESC means "reboot" (the footer advertises it). But the arrow
		 * keys (and other navigation/function keys) arrive over the OF console
		 * as an ANSI escape sequence — ESC '[' <final> (CSI) or ESC 'O' <final>
		 * (SS3) — so the first byte of e.g. Down-arrow is ESC. Without this,
		 * pressing an arrow rebooted the machine. If a sequence follows the ESC,
		 * drain and ignore it; only a *bare* ESC reboots. */
		unsigned char c2;
		if (read_pending_byte(stdin_ih, &c2)) {
			if (c2 == '[' || c2 == 'O') {
				/* CSI/SS3 — consume through the final byte (0x40..0x7e),
				 * which also covers longer forms like ESC '[' '3' '~'. */
				unsigned char cf;
				int k;
				for (k = 0; k < 8; k++) {
					if (!read_pending_byte(stdin_ih, &cf)) {
						break;
					}
					if (cf >= 0x40 && cf <= 0x7e) {
						break;
					}
				}
			}
			return;	/* navigation/function key — not a reboot */
		}
		/* Bare ESC -> reboot. */
		cprint(LINE_RANGE, COL_MID+23, "Halting... ");
		ofw_reset();
		/* ofw_reset() should not return; loop just in case. */
		while (1) { }
	}

	switch (ch) {
	/* PPC: the footer advertises these keys (Wave 6 wires them up — config.c
	 * is now ported and get_key() is OF-backed). 'c' opens the config menu;
	 * SP/CR toggle scroll-lock; ^L redraws. */
	case 'c':
	case 'C':
		get_config();
		break;
	case ' ':
		slock = 1;
		footer();
		break;
	case '\r':
	case '\n':
		slock = 0;
		footer();
		break;
	case 0x0c:
		/* ^L - redraw the display */
		tty_print_screen();
		break;
	default:
		break;
	}

	/* x86 original (PS/2 scancodes), preserved for reference:
	 *
	 * unsigned char c;
	 * if ((c = get_key())) {
	 *     switch(c & 0x7f) {
	 *     case 1:
	 *         // "ESC" key was pressed, bail out.
	 *         cprint(LINE_RANGE, COL_MID+23, "Halting... ");
	 *         // tell the BIOS to do a warm start
	 *         *((unsigned short *)0x472) = 0x1234;
	 *         outb(0xfe,0x64);
	 *         break;
	 *     case 46:
	 *         // c - Configure
	 *         get_config();
	 *         break;
	 *     case 28:
	 *         // CR - clear scroll lock
	 *         slock = 0;
	 *         footer();
	 *         break;
	 *     case 57:
	 *         // SP - set scroll lock
	 *         slock = 1;
	 *         footer();
	 *         break;
	 *     case 0x26:
	 *         // ^L/L - redraw the display
	 *         tty_print_screen();
	 *         break;
	 *     }
	 * }
	 */
}

void footer()
{
	cprint(24, 0, "(ESC)Reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock");
	if (slock) {
		cprint(24, 74, "LOCKED");
	} else {
		cprint(24, 74, "      ");
	}
}

/* getval() reads a number interactively from the config menu (the Address Range
 * limits). It busy-polls get_key() — now OF-backed — which returns PC keycodes;
 * the keycode->char switch, BS/CR handling, base detection, and p/g/m/k/x suffix
 * math are all platform-neutral and enabled verbatim. Reused unchanged on PPC
 * now that get_key() is wired to the OF console (Wave 6). */
ulong getval(int x, int y, int result_shift)
{
	unsigned long val;
	int done;
	int c;
	int i, n;
	int base;
	int shift;
	char buf[16];

	for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
		buf[i] = ' ';
	}
	buf[sizeof(buf)/sizeof(buf[0]) -1] = '\0';

	wait_keyup();
	done = 0;
	n = 0;
	base = 10;
	while(!done) {
		/* Read a new character and process it */
		c = get_key();
		switch(c) {
		case 0x26: /* ^L/L - redraw the display */
			tty_print_screen();
			break;
		case 0x1c: /* CR */
			/* If something has been entered we are done */
			if(n) done = 1;
			break;
		case 0x19: /* p */ buf[n] = 'p'; break;
		case 0x22: /* g */ buf[n] = 'g'; break;
		case 0x32: /* m */ buf[n] = 'm'; break;
		case 0x25: /* k */ buf[n] = 'k'; break;
		case 0x2d: /* x */
			/* Only allow 'x' after an initial 0 */
			if (n == 1 && (buf[0] == '0')) {
				buf[n] = 'x';
			}
			break;
		case 0x0e: /* BS */
			if (n > 0) {
				n -= 1;
				buf[n] = ' ';
			}
			break;
		/* Don't allow entering a number not in our current base */
		case 0x0B: if (base >= 1) buf[n] = '0'; break;
		case 0x02: if (base >= 2) buf[n] = '1'; break;
		case 0x03: if (base >= 3) buf[n] = '2'; break;
		case 0x04: if (base >= 4) buf[n] = '3'; break;
		case 0x05: if (base >= 5) buf[n] = '4'; break;
		case 0x06: if (base >= 6) buf[n] = '5'; break;
		case 0x07: if (base >= 7) buf[n] = '6'; break;
		case 0x08: if (base >= 8) buf[n] = '7'; break;
		case 0x09: if (base >= 9) buf[n] = '8'; break;
		case 0x0A: if (base >= 10) buf[n] = '9'; break;
		case 0x1e: if (base >= 11) buf[n] = 'a'; break;
		case 0x30: if (base >= 12) buf[n] = 'b'; break;
		case 0x2e: if (base >= 13) buf[n] = 'c'; break;
		case 0x20: if (base >= 14) buf[n] = 'd'; break;
		case 0x12: if (base >= 15) buf[n] = 'e'; break;
		case 0x21: if (base >= 16) buf[n] = 'f'; break;
		default:
			break;
		}
		/* Don't allow anything to be entered after a suffix */
		if (n > 0 && (
			(buf[n-1] == 'p') || (buf[n-1] == 'g') ||
			(buf[n-1] == 'm') || (buf[n-1] == 'k'))) {
			buf[n] = ' ';
		}
		/* If we have entered a character increment n */
		if (buf[n] != ' ') {
			n++;
		}
		buf[n] = ' ';
		/* Print the current number */
		cprint(x, y, buf);

		/* Find the base we are entering numbers in */
		base = 10;
		if ((buf[0] == '0') && (buf[1] == 'x')) {
			base = 16;
		}
		else if (buf[0] == '0') {
			base = 8;
		}
	}
	/* Compute our current shift */
	shift = 0;
	switch(buf[n-1]) {
	case 'g': /* gig */  shift = 30; break;
	case 'm': /* meg */  shift = 20; break;
	case 'p': /* page */ shift = 12; break;
	case 'k': /* kilo */ shift = 10; break;
	}
	shift -= result_shift;

	/* Compute our current value */
	val = simple_strtoul(buf, NULL, base);
	if (shift > 0) {
		if (shift >= 32) {
			val = 0xffffffff;
		} else {
			val <<= shift;
		}
	} else {
		if (-shift >= 32) {
			val = 0;
		}
		else {
			val >>= -shift;
		}
	}
	return val;
}

/*
 * THE ONE KEY EDIT.
 *
 * x86: ttyprint() emitted an ANSI cursor-position escape ("\e[<y>;<x>H") plus
 * the text to the serial console via serial_echo_print() — that mirrored the
 * VGA text buffer onto a serial terminal. On PPC/OF there is no serial console;
 * instead vga_buf[] (= SCREEN_ADR) is our char+attr buffer and a linear
 * framebuffer is the only display. cprint()/scroll() already wrote the run into
 * vga_buf and routed (y,x,text) here through tty_print_line/tty_print_region,
 * so the faithful move is: keep the exact signature, and make ttyprint() the
 * framebuffer renderer. For each character position i in p, blit the cell at
 * (y, x+i) from vga_buf via the display backend's fb_render_cell() (display.h).
 * Result: cprint/scroll render verbatim with no edits to themselves.
 *
 * Note we loop over the text length, not the buffer width, matching exactly the
 * run of cells cprint() just wrote. (The char read by fb_render_cell comes from
 * vga_buf, not from p — but p and vga_buf agree because cprint wrote p first.)
 */
void ttyprint(int y, int x, const char *p)
{
	int i;

	/* PPC: render each cell of the run from vga_buf to the framebuffer. */
	for (i = 0; p[i]; i++) {
		fb_render_cell(y, x + i);
	}

	/* x86 original (serial-console cursor escape + echo):
	 *
	 * static char sx[3];
	 * static char sy[3];
	 * sx[0]='\0';
	 * sy[0]='\0';
	 * x++; y++;
	 * itoa(sx, x);
	 * itoa(sy, y);
	 * serial_echo_print("[");
	 * serial_echo_print(sy);
	 * serial_echo_print(";");
	 * serial_echo_print(sx);
	 * serial_echo_print("H");
	 * serial_echo_print(p);
	 */
}


/* x86: serial_echo_init() programmed the 16550 UART (divisor latch, line
 * control, baud rate) via serial_echo_inb/outb port I/O — N/A on PPC/OF (no
 * UART; ttyprint no longer drives a serial console). Stubbed: it also called
 * clear_screen_buf(), so we keep that one platform-neutral call (the screen
 * shadow still needs clearing if anything invokes init), and drop the UART
 * programming. TO VERIFY: confirm whether any caller (init.c/main.c, Wave 3-5)
 * still calls serial_echo_init() for the clear_screen_buf() side effect; if so
 * this stub preserves it. */
void serial_echo_init(void)
{
	/* x86 UART programming removed:
	 *   int comstat, hi, lo, serial_div;
	 *   unsigned char lcr;
	 *   comstat = serial_echo_inb(UART_LCR);
	 *   serial_echo_outb(comstat | UART_LCR_DLAB, UART_LCR);
	 *   hi = serial_echo_inb(UART_DLM);
	 *   lo = serial_echo_inb(UART_DLL);
	 *   serial_echo_outb(comstat, UART_LCR);
	 *   lcr = serial_parity | (serial_bits - 5);
	 *   serial_echo_outb(lcr, UART_LCR);
	 *   serial_div = 115200 / serial_baud_rate;
	 *   serial_echo_outb(0x80|lcr, UART_LCR);
	 *   serial_echo_outb(serial_div & 0xff, UART_DLL);
	 *   serial_echo_outb((serial_div >> 8) & 0xff, UART_DLM);
	 *   serial_echo_outb(lcr, UART_LCR);
	 *   comstat = serial_echo_inb(UART_LSR);
	 *   comstat = serial_echo_inb(UART_RX);
	 *   serial_echo_outb(0x00, UART_IER);
	 */
	clear_screen_buf();
	return;
}

/* x86: serial_echo_print() wrote each char out the UART transmit register
 * (WAIT_FOR_XMITR / serial_echo_outb) — N/A on PPC/OF. ttyprint no longer
 * calls it. Stubbed to a no-op so any straggler caller links; it is gated on
 * serial_cons (always 0 on PPC, see config.h) anyway. */
void serial_echo_print(const char *p)
{
	(void)p;
	/* x86 original:
	 * if (!serial_cons) {
	 *     return;
	 * }
	 * while (*p) {
	 *     WAIT_FOR_XMITR;
	 *     serial_echo_outb(*p, UART_TX);
	 *     if(*p==10) {
	 *         WAIT_FOR_XMITR;
	 *         serial_echo_outb(13, UART_TX);
	 *     }
	 *     p++;
	 * }
	 */
	return;
}

/* ser_map[] maps an input ASCII byte to the PC keycode the menu/getval switches
 * on; ascii_to_keycode() looks it up. Upstream this served the x86 serial-console
 * keyboard emulation; on PPC it is exactly what we need to turn OF stdin (which
 * delivers ASCII) into the keycodes get_config()/getval() expect — so it is
 * ENABLED on PPC (the substrate that makes get_key() work over the OF console).
 * Pure data + a linear lookup, platform-neutral. */
/* Except for multi-character key sequences this mapping
 * table is complete.  So it should not need to be updated
 * when new keys are searched for.  However the key handling
 * should really be turned around and only in get_key should
 * we worry about the exact keycode that was pressed.  Everywhere
 * else we should switch on the character...
 */
struct ascii_map_str ser_map[] =
/*ascii keycode     ascii  keycode*/
{
  /* Special cases come first so I can leave
   * their "normal" mapping in the table,
   * without it being activated.
   */
  {  27,   0x01}, /* ^[/ESC -> ESC  */
  { 127,   0x0e}, /*    DEL -> BS   */
  {   8,   0x0e}, /* ^H/BS  -> BS   */
  {  10,   0x1c}, /* ^L/NL  -> CR   */
  {  13,   0x1c}, /* ^M/CR  -> CR   */
  {   9,   0x0f}, /* ^I/TAB -> TAB  */
  {  19,   0x39}, /* ^S     -> SP   */
  {  17,     28}, /* ^Q     -> CR   */

  { ' ',   0x39}, /*     SP -> SP   */
  { 'a',   0x1e},
  { 'A',   0x1e},
  {   1,   0x1e}, /* ^A      -> A */
  { 'b',   0x30},
  { 'B',   0x30},
  {   2,   0x30}, /* ^B      -> B */
  { 'c',   0x2e},
  { 'C',   0x2e},
  {   3,   0x2e}, /* ^C      -> C */
  { 'd',   0x20},
  { 'D',   0x20},
  {   4,   0x20}, /* ^D      -> D */
  { 'e',   0x12},
  { 'E',   0x12},
  {   5,   0x12}, /* ^E      -> E */
  { 'f',   0x21},
  { 'F',   0x21},
  {   6,   0x21}, /* ^F      -> F */
  { 'g',   0x22},
  { 'G',   0x22},
  {   7,   0x22}, /* ^G      -> G */
  { 'h',   0x23},
  { 'H',   0x23},
  {   8,   0x23}, /* ^H      -> H */
  { 'i',   0x17},
  { 'I',   0x17},
  {   9,   0x17}, /* ^I      -> I */
  { 'j',   0x24},
  { 'J',   0x24},
  {  10,   0x24}, /* ^J      -> J */
  { 'k',   0x25},
  { 'K',   0x25},
  {  11,   0x25}, /* ^K      -> K */
  { 'l',   0x26},
  { 'L',   0x26},
  {  12,   0x26}, /* ^L      -> L */
  { 'm',   0x32},
  { 'M',   0x32},
  {  13,   0x32}, /* ^M      -> M */
  { 'n',   0x31},
  { 'N',   0x31},
  {  14,   0x31}, /* ^N      -> N */
  { 'o',   0x18},
  { 'O',   0x18},
  {  15,   0x18}, /* ^O      -> O */
  { 'p',   0x19},
  { 'P',   0x19},
  {  16,   0x19}, /* ^P      -> P */
  { 'q',   0x10},
  { 'Q',   0x10},
  {  17,   0x10}, /* ^Q      -> Q */
  { 'r',   0x13},
  { 'R',   0x13},
  {  18,   0x13}, /* ^R      -> R */
  { 's',   0x1f},
  { 'S',   0x1f},
  {  19,   0x1f}, /* ^S      -> S */
  { 't',   0x14},
  { 'T',   0x14},
  {  20,   0x14}, /* ^T      -> T */
  { 'u',   0x16},
  { 'U',   0x16},
  {  21,   0x16}, /* ^U      -> U */
  { 'v',   0x2f},
  { 'V',   0x2f},
  {  22,   0x2f}, /* ^V      -> V */
  { 'w',   0x11},
  { 'W',   0x11},
  {  23,   0x11}, /* ^W      -> W */
  { 'x',   0x2d},
  { 'X',   0x2d},
  {  24,   0x2d}, /* ^X      -> X */
  { 'y',   0x15},
  { 'Y',   0x15},
  {  25,   0x15}, /* ^Y      -> Y */
  { 'z',   0x2c},
  { 'Z',   0x2c},
  {  26,   0x2c}, /* ^Z      -> Z */
  { '-',   0x0c},
  { '_',   0x0c},
  {  31,   0x0c}, /* ^_      -> _ */
  { '=',   0x0c},
  { '+',   0x0c},
  { '[',   0x1a},
  { '{',   0x1a},
  {  27,   0x1a}, /* ^[      -> [ */
  { ']',   0x1b},
  { '}',   0x1b},
  {  29,   0x1b}, /* ^]      -> ] */
  { ';',   0x27},
  { ':',   0x27},
  { '\'',  0x28},
  { '"',   0x28},
  { '`',   0x29},
  { '~',   0x29},
  { '\\',  0x2b},
  { '|',   0x2b},
  {  28,   0x2b}, /* ^\      -> \ */
  { ',',   0x33},
  { '<',   0x33},
  { '.',   0x34},
  { '>',   0x34},
  { '/',   0x35},
  { '?',   0x35},
  { '1',   0x02},
  { '!',   0x02},
  { '2',   0x03},
  { '@',   0x03},
  { '3',   0x04},
  { '#',   0x04},
  { '4',   0x05},
  { '$',   0x05},
  { '5',   0x06},
  { '%',   0x06},
  { '6',   0x07},
  { '^',   0x07},
  {  30,   0x07}, /* ^^      -> 6 */
  { '7',   0x08},
  { '&',   0x08},
  { '8',   0x09},
  { '*',   0x09},
  { '9',   0x0a},
  { '(',   0x0a},
  { '0',   0x0b},
  { ')',   0x0b},
  {   0,      0}
};

/*
 * Given an ascii character, return the keycode
 *
 * Uses ser_map definition above.
 *
 * It would be more efficient to use an array of 255 characters
 * and directly index into it.
 */
int ascii_to_keycode (int in)
{
	struct ascii_map_str *p;
	for (p = ser_map; p->ascii; p++) {
		if (in ==p->ascii)
			return p->keycode;
	}
	return 0;
}

/*
 * Call this when you want to wait for the user to lift the
 * finger off of a key.  It is a noop if you are using a
 * serial console.
 */
/* PPC: the OF stdin console (like the x86 serial console) delivers key-DOWN
 * bytes only — there is no key-up event to wait for. So this is a no-op, exactly
 * as upstream short-circuited for serial_cons. (The x86 PS/2 version below
 * busy-waited for the 0x80 key-release scancode, which would loop forever on OF
 * — never re-enable it as-is.) */
void wait_keyup(void)
{
}

#if 0  /* x86 PS/2 original — waits for the 0x80 key-release scancode */
void wait_keyup( void ) {
	/* Check to see if someone lifted the keyboard key */
	while (1) {
		if ((get_key() & 0x80) != 0) {
			return;
		}
		/* Trying to simulate waiting for a key release with
		 * the serial port is to nasty to let live.
		 * In particular some menus don't even display until
		 * you release the key that caused to to get there.
		 * With the serial port this results in double pressing
		 * or something worse for just about every key.
		 */
		if (serial_cons) {
			return;
		}
	}
}
#endif


/*
 * Handles "console=<param>" command line option
 *
 * x86: serial_console_setup() parsed a "console=ttyS0,115200e8" kernel cmdline
 * option and configured the x86 UART (serial_tty/baud/parity/bits) — N/A on
 * PPC/OF: there is no serial console and OF does not pass a Linux-style cmdline.
 * Body wrapped in #if 0; test.h's prototype stays so any straggler links. The
 * parsing is pure string math but has nothing to configure on PPC. (The only
 * caller is main.c's cmdline handling, which the Wave-5 main.c port comments
 * out per HANDOFF: defs.h cmdline/segment defs are #if 0'd.)
 */
#if 0
void serial_console_setup(char *param)
{
	char *option, *end;
	unsigned long tty;
	unsigned long baud_rate;
	unsigned char parity, bits;

	if (strncmp(param, "ttyS", 4))
		return;   /* not a serial port */

	param += 4;

	tty = simple_strtoul(param, &option, 10);

	if (option == param)
		return;   /* there were no digits */

	if (tty > 1)
		return;   /* only ttyS0 and ttyS1 supported */

	if (*option == '\0')
		goto save_tty; /* no options given, just ttyS? */

	if (*option != ',')
		return;  /* missing the comma separator */

	/* baud rate must follow */
	option++;
	baud_rate = simple_strtoul(option, &end, 10);

	if (end == option)
		return;  /* no baudrate after comma */

	if (baud_rate == 0 || (115200 % baud_rate) != 0)
		return;  /* wrong baud rate */

	if (*end == '\0')
		goto save_baud_rate;  /* no more options given */

	switch (toupper(*end)) {
		case 'N':
			parity = 0;
			break;
		case 'O':
			parity = UART_LCR_PARITY;
			break;
		case 'E':
			parity = UART_LCR_PARITY | UART_LCR_EPAR;
			break;
		default:
			/* Unknown parity */
			return;
	}

	end++;
	if (*end == '\0')
		goto save_parity;

	/* word length (bits) */
	if (*end < '7' || *end > '8')
		return;  /* invalid number of bits */

	bits = *end - '0';

	end++;

	if (*end != '\0')
		return;  /* garbage at the end */

	serial_bits = bits;
	save_parity:
	serial_parity = parity;
	save_baud_rate:
	serial_baud_rate = (int) baud_rate;
  save_tty:
	serial_tty = (short) tty;
	serial_cons = 1;
}
#endif


#ifdef LP
#define DATA            0x00
#define STATUS          0x01
#define CONTROL         0x02

#define LP_PBUSY        0x80  /* inverted input, active high */
#define LP_PERRORP      0x08  /* unchanged input, active low */

#define LP_PSELECP      0x08  /* inverted output, active low */
#define LP_PINITP       0x04  /* unchanged output, active low */
#define LP_PSTROBE      0x01  /* short high output on raising edge */

#define DELAY 0x10c6ul

void lp_wait(ulong xloops)
{
	int d0;
	__asm__("mull %0"
		:"=d" (xloops), "=&a" (d0)
		:"1" (xloops),"0" (current_cpu_data.loops_per_sec));
	__delay(xloops);
}
static void __delay(ulong loops)
{
	int d0;
	__asm__ __volatile__(
		"\tjmp 1f\n"
		".align 16\n"
		"1:\tjmp 2f\n"
		".align 16\n"
		"2:\tdecl %0\n\tjns 2b"
		:"=&a" (d0)
		:"0" (loops));
}

put_lp(char c, short port)
{
	unsigned char status;

	/* Wait for printer to be ready */
	while (1) {
		status = inb(STATUS(port));
		if (status & LP_PERRORP) {
			if (status & LP_PBUSY) {
				break;
			}
		}
	}

	outb(d, DATA(c));
	lp_wait(DELAY);
	outb((LP_PSELECP | LP_PINITP | LP_PSTROBE), CONTROL(port));
	lp_wait(DELAY);
	outb((LP_PSELECP | LP_PINITP), CONTROL(port));
	lp_wait(DELAY);
}
#endif
/* Note: the LP (line-printer over x86 parallel port) block above is already
 * gated by #ifdef LP, which is never defined — x86 leaf, N/A on PPC/OF. Left
 * verbatim inside its dormant #ifdef. */
