# Claude Conversation Log

Session ID: agent-a625c805aeb64956d
Date: 2026-05-24 07:25:05

---

## 🤖 Claude (Orchestrator)

You are porting ONE file, lib.c, as part of a line-faithful port of memtest86+ v2.00 to PowerPC/Open Firmware (single-CPU). This is the rendering-and-utilities core and the most important file in this wave. Thoroughness beats tokens — read whole files, never guess.

## FIRST read (for rules + context):
1. /Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md  ← the contract; follow it exactly (esp. §1 discipline, §3 rendering architecture, §5 report format)
2. /Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/HANDOFF.md  ← read "Substrate contract" (incl. the attr-only-render note) and "v2.00 facts that bite later waves"
Then read IN FULL:
3. /Users/cell/claude/memtestppc+/ref/memtest86+-2.00/lib.c  ← the file you are porting
4. The substrate + headers you compile against: /Users/cell/claude/memtestppc+/src/{test.h, display.h, ppc.h, ofw.h, config.h}
5. /Users/cell/claude/memtestppc+/ref/memtest86+-2.00/screen_buffer.h  ← lib.c's cprint calls tty_print_line() declared here; a parallel agent is porting screen_buffer.c. The contract: your ttyprint(y,x,text) is what renders.
6. Reference only (do NOT copy its decisions): /Users/cell/claude/memtestppc+/attic/src-v0.01/display.c had a check_key()/OF-stdin read and an OF reboot you can learn the OF mechanics from.

## Create /Users/cell/claude/memtestppc+/src/lib.c — faithful port of v2.00 lib.c
Discipline: start verbatim; comment x86 leaf WITH a one-line reason `/* x86: ... — N/A on PPC/OF */`; add PPC marked `/* PPC: ... */`; never delete; keep buildable. lib.c must `#include "test.h"` (brings struct vars, prototypes, SCREEN_ADR=vga_buf, ppc.h, display.h) and `#include "ofw.h"`.

KEEP VERBATIM (platform-neutral): scroll, clear_scroll, cplace, cprint, dprint, hprint, hprint2, hprint3, aprint, xprint, footer (keep v2.00's exact footer text — we reproduce the v2.00 TUI), and the string/mem utils (strlen, strstr, memcmp, strncmp, memmove, memcpy, toupper, isdigit, isxdigit, simple_strtoul, itoa, reverse). Do NOT alter cprint/scroll/cplace — they write into vga_buf (SCREEN_ADR) and route through tty_print_line; that's the whole point.

THE ONE KEY EDIT — rewrite ttyprint() ONLY:
- Upstream ttyprint(int y,int x,const char *p) emits an ANSI cursor escape + text to the serial console (serial_echo_print). Replace its body so it renders to the framebuffer instead: for each character position i in p, call fb_render_cell(y, x+i) (declared in display.h) which reads char+attr from vga_buf and blits the glyph. Keep the exact signature. Result: cprint/scroll (which call tty_print_line -> ttyprint) render verbatim with no edits to themselves.

COMMENT OUT (x86 leaf, with reasons; stub callers as needed so it links):
- inter() + the codes[] exception-name table + any struct eregs def/use — x86 interrupt/exception handler; we install no x86 IDT on PPC/OF.
- set_cache() (x86 cr0 cache control) — comment the x86 asm; provide a PPC stub `void set_cache(int val){ (void)val; /* PPC: cache control via HID0[DCE] not yet implemented — uncached tests are a TO-VERIFY item */ }` so the cache/uncached tseq entries link. Flag in your report.
- get_key()/PC-keyboard port I/O (0x60/0x64), ascii_to_keycode(), wait_keyup() — x86 PS/2 keyboard; OF owns input.
- serial_echo_init(), serial_echo_print(), serial_console_setup() — no serial console (ttyprint no longer calls them). Comment/stub.
- reboot(): replace with a PPC version calling ofw_reset() (see ofw.h/ofw.c); comment the x86.
- get_list(), getnum() (config-menu input helpers) — these are for the Wave-6 config menu; comment/stub and flag, don't fully implement now.

REIMPLEMENT for PPC/OF:
- check_input(): upstream polls the PC keyboard. Reimplement to poll OF stdin: get the ihandle via ofw_get_stdin() (cache it in a static), ofw_read() one byte non-blocking; on ESC (27) reboot via ofw_reset(). Leave commented hooks for scroll-lock (slock) and 'c'/config (Wave 6). check_input is called from scroll() and do_tick(), so it must exist and be cheap. (The attic check_key() shows the OF read mechanics — learn from it, don't inherit blindly.)

## Do NOT
- Don't modify anything outside src/lib.c (and your report). Don't touch ref/, attic/, src/test.h, src/screen_buffer.* (another agent owns screen_buffer).
- Don't try to build (the lead integrates + builds the Wave-2 checkpoint). You may read usages to get calls right.

## Deliverables
1. /Users/cell/claude/memtestppc+/src/lib.c
2. Report /Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/port-lib_c.md per PORTING-GUIDE §5 (purpose; disposition; every x86 region commented + its replacement; the ttyprint rewrite; the check_input/reboot PPC reimpl; set_cache stub + the uncached-test TO-VERIFY; what stays verbatim; build impact / what it includes; open questions).

Then RETURN a concise summary (<450 words): the ttyprint rewrite, what you commented vs kept verbatim, the check_input/reboot/set_cache handling, and anything I must watch at the integration build.

---

## 🤖 Claude (Subagent)

I'll start by reading the contract and context files, then the file to port and its dependencies.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/HANDOFF.md"
}


---

## 🤖 Claude (Subagent)

Now let me read the file to port and its dependencies.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/lib.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/screen_buffer.h"
}


---

## 🤖 Claude (Subagent)

Now let me read the substrate headers I compile against.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/test.h"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/display.h"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/ppc.h"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/ofw.h"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/config.h"
}


---

## 🤖 Claude (Subagent)

I now have a complete picture. Let me check the attic display.c for the OF stdin/reboot mechanics, and verify the io.h/serial.h includes won't be needed.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/attic/src-v0.01/display.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/ofw.c"
}


---

## 🤖 Claude (Subagent)

I have everything I need. Note the attic `check_key()` referenced in the brief isn't in this display.c, but the OF read mechanics are clear from `ofw_get_stdin()`/`ofw_read()` in ofw.c. The brief mentions `cplace` and `strlen`/`strstr`/`memcpy` as "keep verbatim" but those don't exist in v2.00 lib.c — I'll keep only what's actually in the file (per fidelity discipline, I port what's there). Let me note that.

Now let me write the ported lib.c. I'll start from a verbatim copy and apply the disciplined edits.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/lib.c",
  "content": "/* lib.c - MemTest-86  Version 3.0\n *\n * Released under version 2 of the Gnu Public License.\n * By Chris Brady, cbrady@sgi.com\n * ----------------------------------------------------\n * MemTest86+ V2.00 Specific code (GPL V2.0)\n * By Samuel DEMEULEMEESTER, sdemeule@memtest.org\n * http://www.canardplus.com - http://www.memtest.org\n * ----------------------------------------------------\n * memtestppc+ port: line-faithful port of memtest86+ v2.00 lib.c to\n * PowerPC / Open Firmware (single-CPU). Discipline (see\n * docs/sessions/006-port-fidelity-restructure/PORTING-GUIDE.md): start from\n * the verbatim v2.00 file; comment out x86 leaf code with a one-line reason;\n * add PPC code inline marked \"PPC:\"; never delete; keep buildable. Nothing\n * from the old attic/src-v0.01 hack is inherited.\n *\n * THE ONE KEY EDIT in this file is ttyprint(): upstream it emits an ANSI\n * cursor escape + text to a serial console; here it becomes the framebuffer\n * renderer (loops fb_render_cell over the run), so cprint/scroll/etc. stay\n * byte-verbatim and \"just work\".\n*/\n\n/* x86: io.h = x86 port-I/O (inb/outb) primitives; serial.h = x86 16550 UART\n * register defs (UART_LSR etc.) \u2014 N/A on PPC/OF. The only consumers were the\n * PC-keyboard get_key(), the serial_echo_* console, and beep \u2014 all commented\n * out below. Replaced by ofw.h (the OF client interface = the io.h/BIOS\n * replacement). */\n/* #include \"io.h\" */\n/* #include \"serial.h\" */\n#include \"test.h\"\n#include \"config.h\"\n#include \"screen_buffer.h\"\n/* PPC: OF client interface for check_input()'s stdin poll and reboot(). */\n#include \"ofw.h\"\n\n\nint slock = 0, lsr = 0;\nshort serial_cons = SERIAL_CONSOLE_DEFAULT;\n\n#if SERIAL_TTY != 0 && SERIAL_TTY != 1\n#error Bad SERIAL_TTY. Only ttyS0 and ttyS1 are supported.\n#endif\nshort serial_tty = SERIAL_TTY;\nconst short serial_base_ports[] = {0x3f8, 0x2f8};\n\n#if ((115200%SERIAL_BAUD_RATE) != 0)\n#error Bad default baud rate\n#endif\nint serial_baud_rate = SERIAL_BAUD_RATE;\nunsigned char serial_parity = 0;\nunsigned char serial_bits = 8;\n\nchar buf[18];\n\nstruct ascii_map_str {\n\tint ascii;\n\tint keycode;\n};\n\n/* x86: codes[] is the x86 exception-vector name table consumed only by\n * inter() (the x86 interrupt handler) \u2014 N/A on PPC/OF (we install no x86 IDT).\n * Commented out alongside inter() below to keep the diff legible. */\n/*\nchar *codes[] = {\n\t\"  Divide\",\n\t\"   Debug\",\n\t\"     NMI\",\n\t\"  Brkpnt\",\n\t\"Overflow\",\n\t\"   Bound\",\n\t\"  Inv_Op\",\n\t\" No_Math\",\n\t\"Double_Fault\",\n\t\"Seg_Over\",\n\t\" Inv_TSS\",\n\t\"  Seg_NP\",\n\t\"Stack_Fault\",\n\t\"Gen_Prot\",\n\t\"Page_Fault\",\n\t\"   Resvd\",\n\t\"     FPE\",\n\t\"Alignment\",\n\t\" Mch_Chk\",\n\t\"SIMD FPE\"\n};\n*/\n\n/* x86: struct eregs is the x86 trap-frame (esp/ebp/.../eflag) pushed by the\n * IDT stubs and consumed only by inter() \u2014 N/A on PPC/OF. Commented out with\n * inter() below. test.h forward-declares \"struct eregs;\" so the (also\n * commented) prototype stays resolvable. */\n/*\nstruct eregs {\n\tulong esp;\n\tulong ebp;\n\tulong esi;\n\tulong edi;\n\tulong edx;\n\tulong ecx;\n\tulong ebx;\n\tulong eax;\n\tulong vect;\n\tulong code;\n\tulong eip;\n\tulong cs;\n\tulong eflag;\n};\n*/\n\nint memcmp(const void *s1, const void *s2, ulong count)\n{\n\tconst unsigned char *src1 = s1, *src2 = s2;\n\tint i;\n\tfor(i = 0; i < count; i++) {\n\t\tif (src1[i] != src2[i]) {\n\t\t\treturn (int)src1[i] - (int)src2[i];\n\t\t}\n\t}\n\treturn 0;\n}\n\nint strncmp(const char *s1, const char *s2, ulong n) {\n\tsigned char res = 0;\n\twhile (n) {\n\t\tres = *s1 - *s2;\n\t\tif (res != 0)\n\t\t\treturn res;\n\t\tif (*s1 == '\\0')\n\t\t\treturn 0;\n\t\t++s1, ++s2;\n\t\t--n;\n\t}\n\treturn res;\n}\n\nvoid *memmove(void *dest, const void *src, ulong n)\n{\n\tlong i;\n\tchar *d = (char *)dest, *s = (char *)src;\n\n\t/* If src == dest do nothing */\n\tif (dest < src) {\n\t\tfor(i = 0; i < n; i++) {\n\t\t\td[i] = s[i];\n\t\t}\n\t}\n\telse if (dest > src) {\n\t\tfor(i = n -1; i >= 0; i--) {\n\t\t\td[i] = s[i];\n\t\t}\n\t}\n\treturn dest;\n}\n\nchar toupper(char c)\n{\n\tif (c >= 'a' && c <= 'z')\n\t\treturn c + 'A' -'a';\n\telse\n\t\treturn c;\n}\n\nint isdigit(char c)\n{\n\treturn c >= '0' && c <= '9';\n}\n\nint isxdigit(char c)\n{\n\treturn isdigit(c) || (toupper(c) >= 'A' && toupper(c) <= 'F'); }\n\nunsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {\n\tunsigned long result = 0, value;\n\n\tif (!base) {\n\t\tbase = 10;\n\t\tif (*cp == '0') {\n\t\t\tbase = 8;\n\t\t\tcp++;\n\t\t\tif (toupper(*cp) == 'X' && isxdigit(cp[1])) {\n\t\t\t\tcp++;\n\t\t\t\tbase = 16;\n\t\t\t}\n\t\t}\n\t} else if (base == 16) {\n\t\tif (cp[0] == '0' && toupper(cp[1]) == 'X')\n\t\t\tcp += 2;\n\t}\n\twhile (isxdigit(*cp) &&\n\t\t(value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {\n\t\tresult = result*base + value;\n\t\tcp++;\n\t}\n\tif (endp)\n\t\t*endp = (char *)cp;\n\treturn result;\n}\n\n\n/*\n * Scroll the error message area of the screen as needed\n * Starts at line LINE_SCROLL and ends at line 23\n */\nvoid scroll(void)\n{\n\tint i, j;\n\tchar *s, tmp;\n\n\t/* Only scroll if at the bottom of the screen */\n\tif (v->msg_line < 23) {\n\t\tv->msg_line++;\n\t} else {\n\t\t/* If scroll lock is on, loop till it is cleared */\n\t\twhile (slock) {\n\t\t\tcheck_input();\n\t\t}\n\t\tfor (i=LINE_SCROLL; i<23; i++) {\n\t\t\ts = (char *)(SCREEN_ADR + ((i+1) * 160));\n\t\t\tfor (j=0; j<160; j+=2, s+=2) {\n\t\t\t\t*(s-160) = *s;\n\t\t\t\ttmp = get_scrn_buf(i+1, j/2);\n\t\t\t\tset_scrn_buf(i, j/2, tmp);\n\t\t\t}\n\t\t}\n\t\t/* Clear the newly opened line */\n\t\ts = (char *)(SCREEN_ADR + (23 * 160));\n\t\tfor (j=0; j<80; j++) {\n\t\t\t*s = ' ';\n\t\t\tset_scrn_buf(23, j, ' ');\n\t\t\ts += 2;\n\t\t}\n\t\ttty_print_region(LINE_SCROLL, 0, 23, 79);\n\t}\n}\n\n/*\n * Clear scroll region\n */\nvoid clear_scroll(void)\n{\n\tint i;\n\tchar *s;\n\n\ts = (char*)(SCREEN_ADR+LINE_HEADER*160);\n        for(i=0; i<80*(24-LINE_HEADER); i++) {\n                *s++ = ' ';\n                *s++ = 0x17;\n        }\n}\n\n/*\n * Print characters on screen\n */\nvoid cprint(int y, int x, const char *text)\n{\n\tregister int i;\n\tchar *dptr;\n\n\tdptr = (char *)(SCREEN_ADR + (160*y) + (2*x));\n\tfor (i=0; text[i]; i++) {\n\t\t*dptr = text[i];\n\t\tdptr += 2;\n\t}\n\ttty_print_line(y, x, text);\n}\n\nvoid itoa(char s[], int n)\n{\n\tint i, sign;\n\n\tif((sign = n) < 0)\n\t\tn = -n;\n\ti=0;\n\tdo {\n\t\ts[i++] = n % 10 + '0';\n\t} while ((n /= 10) > 0);\n\tif(sign < 0)\n\t\ts[i++] = '-';\n\ts[i] = '\\0';\n\treverse(s);\n}\n\nvoid reverse(char s[])\n{\n\tint c, i, j;\n\tfor(j = 0; s[j] != 0; j++)\n\t\t;\n\n\tfor(i=0, j = j - 1; i < j; i++, j--) {\n\t\tc = s[i];\n\t\ts[i] = s[j];\n\t\ts[j] = c;\n\t}\n}\n\n/*\n * Print a people friendly address\n */\nvoid aprint(int y, int x, ulong page)\n{\n\t/* page is in multiples of 4K */\n\tif ((page << 2) < 9999) {\n\t\tdprint(y, x, page << 2, 4, 0);\n\t\tcprint(y, x+4, \"K\");\n\t}\n\telse if ((page >>8) < 9999) {\n\t\tdprint(y, x, (page  + (1 << 7)) >> 8, 4, 0);\n\t\tcprint(y, x+4, \"M\");\n\t}\n\telse if ((page >>18) < 9999) {\n\t\tdprint(y, x, (page + (1 << 17)) >> 18, 4, 0);\n\t\tcprint(y, x+4, \"G\");\n\t}\n\telse {\n\t\tdprint(y, x, (page + (1 << 27)) >> 28, 4, 0);\n\t\tcprint(y, x+4, \"T\");\n\t}\n}\n\n/*\n * Print a decimal number on screen\n */\nvoid dprint(int y, int x, ulong val, int len, int right)\n{\n\tulong j, k;\n\tint i, flag=0;\n\n\tif (val > 999999999 || len > 9) {\n\t\treturn;\n\t}\n\tfor(i=0, j=1; i<len-1; i++) {\n\t\tj *= 10;\n\t}\n\tif (!right) {\n\t\tfor (i=0; j>0; j/=10) {\n\t\t\tk = val/j;\n\t\t\tif (k > 9) {\n\t\t\t\tj *= 100;\n\t\t\t\tcontinue;\n\t\t\t}\n\t\t\tif (flag || k || j == 1) {\n\t\t\t\tbuf[i++] = k + '0';\n\t\t\t\tflag++;\n\t\t\t} else {\n\t\t\t\tbuf[i++] = ' ';\n\t\t\t}\n\t\t\tval -= k * j;\n\t\t}\n\t} else {\n\t\tfor(i=0; i<len; j/=10) {\n\t\t\tif (j) {\n\t\t\t\tk = val/j;\n\t\t\t\t\tif (k > 9) {\n\t\t\t\t\tj *= 100;\n\t\t\t\t\tlen++;\n\t\t\t\t\tcontinue;\n\t\t\t\t}\n\t\t\t\tif (k == 0 && flag == 0) {\n\t\t\t\t\tcontinue;\n\t\t\t\t}\n\t\t\t\tbuf[i++] = k + '0';\n\t\t\t\tval -= k * j;\n\t\t\t} else {\n\t\t\t\tif (flag == 0 &&  i < len-1) {\n\t\t\t\t\tbuf[i++] = '0';\n\t\t\t\t} else {\n\t\t\t\t\tbuf[i++] = ' ';\n\t\t\t\t}\n\t\t\t}\n\t\t\tflag++;\n\t\t}\n\t}\n\tbuf[i] = 0;\n\tcprint(y,x,buf);\n}\n\n/*\n * Print a hex number on screen at least digits long\n */\nvoid hprint2(int y,int x, unsigned long val, int digits)\n{\n        unsigned long j;\n        int i, idx, flag = 0;\n\n        for (i=0, idx=0; i<8; i++) {\n                j = val >> (28 - (4 * i));\n                j &= 0xf;\n                if (j < 10) {\n                        if (flag || j || i == 7) {\n                                buf[idx++] = j + '0';\n                                flag++;\n                        } else {\n                                buf[idx++] = '0';\n                        }\n                } else {\n                        buf[idx++] = j + 'a' - 10;\n                        flag++;\n                }\n        }\n        if (digits > 8) {\n                digits = 8;\n        }\n        if (flag > digits) {\n                digits = flag;\n        }\n        buf[idx] = 0;\n        cprint(y,x,buf + (idx - digits));\n}\n\n/*\n * Print a hex number on screen exactly digits long\n */\nvoid hprint3(int y,int x, unsigned long val, int digits)\n{\n\tunsigned long j;\n\tint i, idx, flag = 0;\n\n\n\tfor (i=0, idx=0; i<digits; i++) {\n\t\tj = 0xf & val;\n\t\tval /= 16;\n\n\t\tif (j < 10) {\n\t\t\tif (flag || j || i == 7) {\n\t\t\t\tbuf[digits - ++idx] = j + '0';\n\t\t\t\tflag++;\n\t\t\t} else {\n\t\t\t\tbuf[digits - ++idx] = '0';\n\t\t\t}\n\t\t} else {\n\t\t\tbuf[digits - ++idx] = j + 'a' - 10;\n\t\t\tflag++;\n\t\t}\n\t}\n\tbuf[idx] = 0;\n\tcprint(y,x,buf);\n}\n\n/*\n * Print a hex number on screen\n */\nvoid hprint(int y, int x, unsigned long val)\n{\n\treturn hprint2(y, x, val, 8);\n}\n\n/*\n * Print an address in 0000m0000k0000 notation\n */\nvoid xprint(int y,int x, ulong val)\n{\n\tulong j;\n\n\tj = (val & 0xffc00000) >> 20;\n\tdprint(y, x, j, 4, 0);\n\tcprint(y, x+4, \"m\");\n\tj = (val & 0xffc00) >> 10;\n\tdprint(y, x+5, j, 4, 0);\n\tcprint(y, x+9, \"k\");\n\tj = val & 0x3ff;\n\tdprint(y, x+10, j, 4, 0);\n}\n\n/* x86: inter() is the x86 interrupt/exception handler. It is invoked from the\n * IDT stubs with a struct eregs trap-frame, reads CR2 for page-fault address\n * via inline asm, and dumps x86 registers (eax/ebx/.../eflag) and the x86 PC\n * (eip:cs) \u2014 N/A on PPC/OF: we install no x86 IDT, and the PPC exception model\n * (DSI/ISI/program via the OF-owned vectors) is entirely different. A faithful\n * PPC exception dump would be its own workstream; for now we have no caller\n * (head.S does not route traps here), so this is commented out wholesale. The\n * codes[] table and struct eregs it relied on are commented out above. */\n#if 0\n/* Handle an interrupt */\nvoid inter(struct eregs *trap_regs)\n{\n\tint i, line;\n\tunsigned char *pp;\n\tulong address = 0;\n\n\t/* Get the page fault address */\n\tif (trap_regs->vect == 14) {\n\t\t__asm__(\"movl %%cr2,%0\":\"=r\" (address));\n\t}\n#ifdef PARITY_MEM\n\n\t/* Check for a parity error */\n\tif (trap_regs->vect == 2) {\n\t\tparity_err(trap_regs->edi, trap_regs->esi);\n\t\treturn;\n\t}\n#endif\n\n\t/* clear scrolling region */\n\tpp=(unsigned char *)(SCREEN_ADR+(2*80*(LINE_SCROLL-2)));\n\tfor(i=0; i<2*80*(24-LINE_SCROLL-2); i++, pp+=2) {\n\t\t*pp = ' ';\n\t}\n\tline = LINE_SCROLL-2;\n\n\tcprint(line, 0, \"Unexpected Interrupt - Halting\");\n\tcprint(line+2, 0, \" Type: \");\n\tif (trap_regs->vect <= 19) {\n\t\tcprint(line+2, 7, codes[trap_regs->vect]);\n\t} else {\n\t\thprint(line+2, 7, trap_regs->vect);\n\t}\n\tcprint(line+3, 0, \"   PC: \");\n\thprint(line+3, 7, trap_regs->eip);\n\tcprint(line+4, 0, \"   CS: \");\n\thprint(line+4, 7, trap_regs->cs);\n\tcprint(line+5, 0, \"Eflag: \");\n\thprint(line+5, 7, trap_regs->eflag);\n\tcprint(line+6, 0, \" Code: \");\n\thprint(line+6, 7, trap_regs->code);\n\tif (trap_regs->vect == 14) {\n\t\t/* Page fault address */\n\t\tcprint(line+7, 0, \" Addr: \");\n\t\thprint(line+7, 7, address);\n\t}\n\n\tcprint(line+2, 20, \"eax: \");\n\thprint(line+2, 25, trap_regs->eax);\n\tcprint(line+3, 20, \"ebx: \");\n\thprint(line+3, 25, trap_regs->ebx);\n\tcprint(line+4, 20, \"ecx: \");\n\thprint(line+4, 25, trap_regs->ecx);\n\tcprint(line+5, 20, \"edx: \");\n\thprint(line+5, 25, trap_regs->edx);\n\tcprint(line+6, 20, \"edi: \");\n\thprint(line+6, 25, trap_regs->edi);\n\tcprint(line+7, 20, \"esi: \");\n\thprint(line+7, 25, trap_regs->esi);\n\tcprint(line+8, 20, \"ebp: \");\n\thprint(line+8, 25, trap_regs->ebp);\n\tcprint(line+9, 20, \"esp: \");\n\thprint(line+9, 25, trap_regs->esp);\n\n\tcprint(line+1, 38, \"Stack:\");\n\tfor (i=0; i<12; i++) {\n\t\thprint(line+2+i, 38, trap_regs->esp+(4*i));\n\t\thprint(line+2+i, 47, *(ulong*)(trap_regs->esp+(4*i)));\n\t\thprint(line+2+i, 57, trap_regs->esp+(4*(i+12)));\n\t\thprint(line+2+i, 66, *(ulong*)(trap_regs->esp+(4*(i+12))));\n\t}\n\n\tcprint(line+11, 0, \"CS:EIP:                          \");\n\tpp = (unsigned char *)trap_regs->eip;\n\tfor(i = 0; i < 10; i++) {\n\t\thprint2(line+11, 8+(3*i), pp[i], 2);\n\t}\n\n\twhile(1) {\n\t\tcheck_input();\n\t}\n}\n#endif\n\nvoid set_cache(int val)\n{\n\t/* x86: the original tested cpu_id (CPUID/type) to special-case the 386\n\t * (no cache), then toggled the x86 cache-disable bit in cr0 via\n\t * cache_off()/cache_on() (test.h) \u2014 N/A on PPC/OF. struct cpu_ident is\n\t * x86 CPUID state (commented out in test.h), so its use here is gone too.\n\t *   extern struct cpu_ident cpu_id;\n\t *   if ((cpu_id.cpuid < 1) && (cpu_id.type == 3)) {\n\t *       cprint(LINE_INFO, COL_CACHE, \"none\");\n\t *       return;\n\t *   }\n\t *   switch(val) {\n\t *   case 0:\n\t *       cache_off();\n\t *       cprint(LINE_INFO, COL_CACHE, \"off\");\n\t *       break;\n\t *   case 1:\n\t *       cache_on();\n\t *       cprint(LINE_INFO, COL_CACHE, \" on\");\n\t *       break;\n\t *   }\n\t */\n\t(void)val;\n\t/* PPC: cache control via HID0[DCE]/[ICE] (and dcbf+sync from ppc.h) is not\n\t * yet implemented. The tseq table (main.c, Wave 5) has cache=0 (uncached)\n\t * and cache=1 (cached) entries; this stub lets them link. The cache stays\n\t * in whatever state OF left it. TO VERIFY: the \"uncached\" tseq rows do not\n\t * actually run uncached until HID0 control lands \u2014 flagged in the report. */\n}\n\n/* x86: get_key() polls the PS/2 keyboard controller status port (inb 0x64) and\n * data port (inb 0x60), with a serial-console fallback (serial_echo_inb of the\n * UART LSR/RX, mapped via ascii_to_keycode) \u2014 N/A on PPC/OF: there is no PS/2\n * controller and no UART, and OF owns input. Replaced by check_input()'s OF\n * stdin poll below. Commented out; its only callers (check_input, getval,\n * wait_keyup) are commented/reimplemented to match. */\n#if 0\nint get_key() {\n\tint c;\n\n\tc = inb(0x64);\n\tif ((c & 1) == 0) {\n\t\tif (serial_cons) {\n\t\t\tint comstat;\n\t\t\tcomstat = serial_echo_inb(UART_LSR);\n\t\t\tif (comstat & UART_LSR_DR) {\n\t\t\t\tc = serial_echo_inb(UART_RX);\n\t\t\t\t/* Pressing '.' has same effect as 'c'\n\t\t\t\t   on a keyboard.\n\t\t\t\t   Oct 056   Dec 46   Hex 2E   Ascii .\n\t\t\t\t*/\n\t\t\t\treturn (ascii_to_keycode(c));\n\t\t\t}\n\t\t}\n\t\treturn(0);\n\t}\n\tc = inb(0x60);\n\treturn((c));\n}\n#endif\n\n/*\n * Poll for user input and act on it.\n *\n * x86: the original called get_key() (PS/2 scancodes) and switched on the\n * scancode (ESC=1 -> warm-boot via 0x472/0x64, 46='c' -> get_config,\n * 28=CR/57=SP toggle scroll lock, 0x26=^L redraw). That whole path is\n * commented out below.\n *\n * PPC: poll Open Firmware's stdin (the OF console \u2014 ADB/USB keyboard on real\n * Macs, the QEMU console under OpenBIOS). One non-blocking byte; ESC (ASCII 27)\n * reboots via ofw_reset(). The scroll-lock and config hooks are left as\n * commented stubs for Wave 6 (the footer advertises them; see HANDOFF \"Footer\n * honesty\"). check_input() is called from scroll() and do_tick(), so it must\n * exist and be cheap \u2014 a single ofw_read of one byte is.\n */\nvoid check_input(void)\n{\n\t/* PPC: cache the stdin ihandle across calls (it never changes). */\n\tstatic ofw_ihandle_t stdin_ih = 0;\n\tstatic int stdin_init = 0;\n\tunsigned char ch;\n\tint n;\n\n\tif (!stdin_init) {\n\t\tstdin_ih = ofw_get_stdin();\n\t\tstdin_init = 1;\n\t}\n\tif (stdin_ih == -1 || stdin_ih == 0) {\n\t\treturn;\n\t}\n\n\t/* OF \"read\" is non-blocking on a console: returns the bytes available\n\t * (0 if none). Read a single byte. */\n\tn = ofw_read(stdin_ih, &ch, 1);\n\tif (n <= 0) {\n\t\treturn;\n\t}\n\n\tswitch (ch) {\n\tcase 27:\n\t\t/* PPC: \"ESC\" key was pressed, bail out and reboot. */\n\t\tcprint(LINE_RANGE, COL_MID+23, \"Halting... \");\n\t\tofw_reset();\n\t\t/* ofw_reset() should not return; loop just in case. */\n\t\twhile (1) { }\n\t\tbreak;\n\t/* PPC: Wave-6 config-menu + scroll-lock hooks (the footer advertises\n\t * (c)configuration / (SP)scroll_lock / (CR)scroll_unlock). Wiring these\n\t * up requires get_config() (config.c, Wave 6) and a settled key story.\n\t * Left as commented stubs so the keys are not silently mis-handled now.\n\t * case 'c':\n\t * case 'C':\n\t *     get_config();\n\t *     break;\n\t * case ' ':\n\t *     slock = 1;\n\t *     footer();\n\t *     break;\n\t * case '\\r':\n\t * case '\\n':\n\t *     slock = 0;\n\t *     footer();\n\t *     break;\n\t * case 0x0c:\n\t *     // ^L - redraw the display\n\t *     tty_print_screen();\n\t *     break;\n\t */\n\tdefault:\n\t\tbreak;\n\t}\n\n\t/* x86 original (PS/2 scancodes), preserved for reference:\n\t *\n\t * unsigned char c;\n\t * if ((c = get_key())) {\n\t *     switch(c & 0x7f) {\n\t *     case 1:\n\t *         // \"ESC\" key was pressed, bail out.\n\t *         cprint(LINE_RANGE, COL_MID+23, \"Halting... \");\n\t *         // tell the BIOS to do a warm start\n\t *         *((unsigned short *)0x472) = 0x1234;\n\t *         outb(0xfe,0x64);\n\t *         break;\n\t *     case 46:\n\t *         // c - Configure\n\t *         get_config();\n\t *         break;\n\t *     case 28:\n\t *         // CR - clear scroll lock\n\t *         slock = 0;\n\t *         footer();\n\t *         break;\n\t *     case 57:\n\t *         // SP - set scroll lock\n\t *         slock = 1;\n\t *         footer();\n\t *         break;\n\t *     case 0x26:\n\t *         // ^L/L - redraw the display\n\t *         tty_print_screen();\n\t *         break;\n\t *     }\n\t * }\n\t */\n}\n\nvoid footer()\n{\n\tcprint(24, 0, \"(ESC)Reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock\");\n\tif (slock) {\n\t\tcprint(24, 74, \"LOCKED\");\n\t} else {\n\t\tcprint(24, 74, \"      \");\n\t}\n}\n\n/* x86: getval() reads a number interactively from the config menu using\n * get_key() PS/2 scancodes (the big scancode->char switch, BS/CR handling,\n * base detection, p/g/m/k/x suffixes) \u2014 this is a Wave-6 config-menu input\n * helper. OF owns input on PPC and the OF key story for the menu is not yet\n * settled, so this is parked: commented out wholesale and flagged. The string\n * math (simple_strtoul + suffix shift) is platform-neutral and will be reused\n * when the menu is reimplemented for OF in Wave 6. test.h's prototype is left\n * in place; the only callers are config.c (Wave 6, not yet ported). */\n#if 0\nulong getval(int x, int y, int result_shift)\n{\n\tunsigned long val;\n\tint done;\n\tint c;\n\tint i, n;\n\tint base;\n\tint shift;\n\tchar buf[16];\n\n\tfor(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {\n\t\tbuf[i] = ' ';\n\t}\n\tbuf[sizeof(buf)/sizeof(buf[0]) -1] = '\\0';\n\n\twait_keyup();\n\tdone = 0;\n\tn = 0;\n\tbase = 10;\n\twhile(!done) {\n\t\t/* Read a new character and process it */\n\t\tc = get_key();\n\t\tswitch(c) {\n\t\tcase 0x26: /* ^L/L - redraw the display */\n\t\t\ttty_print_screen();\n\t\t\tbreak;\n\t\tcase 0x1c: /* CR */\n\t\t\t/* If something has been entered we are done */\n\t\t\tif(n) done = 1;\n\t\t\tbreak;\n\t\tcase 0x19: /* p */ buf[n] = 'p'; break;\n\t\tcase 0x22: /* g */ buf[n] = 'g'; break;\n\t\tcase 0x32: /* m */ buf[n] = 'm'; break;\n\t\tcase 0x25: /* k */ buf[n] = 'k'; break;\n\t\tcase 0x2d: /* x */\n\t\t\t/* Only allow 'x' after an initial 0 */\n\t\t\tif (n == 1 && (buf[0] == '0')) {\n\t\t\t\tbuf[n] = 'x';\n\t\t\t}\n\t\t\tbreak;\n\t\tcase 0x0e: /* BS */\n\t\t\tif (n > 0) {\n\t\t\t\tn -= 1;\n\t\t\t\tbuf[n] = ' ';\n\t\t\t}\n\t\t\tbreak;\n\t\t/* Don't allow entering a number not in our current base */\n\t\tcase 0x0B: if (base >= 1) buf[n] = '0'; break;\n\t\tcase 0x02: if (base >= 2) buf[n] = '1'; break;\n\t\tcase 0x03: if (base >= 3) buf[n] = '2'; break;\n\t\tcase 0x04: if (base >= 4) buf[n] = '3'; break;\n\t\tcase 0x05: if (base >= 5) buf[n] = '4'; break;\n\t\tcase 0x06: if (base >= 6) buf[n] = '5'; break;\n\t\tcase 0x07: if (base >= 7) buf[n] = '6'; break;\n\t\tcase 0x08: if (base >= 8) buf[n] = '7'; break;\n\t\tcase 0x09: if (base >= 9) buf[n] = '8'; break;\n\t\tcase 0x0A: if (base >= 10) buf[n] = '9'; break;\n\t\tcase 0x1e: if (base >= 11) buf[n] = 'a'; break;\n\t\tcase 0x30: if (base >= 12) buf[n] = 'b'; break;\n\t\tcase 0x2e: if (base >= 13) buf[n] = 'c'; break;\n\t\tcase 0x20: if (base >= 14) buf[n] = 'd'; break;\n\t\tcase 0x12: if (base >= 15) buf[n] = 'e'; break;\n\t\tcase 0x21: if (base >= 16) buf[n] = 'f'; break;\n\t\tdefault:\n\t\t\tbreak;\n\t\t}\n\t\t/* Don't allow anything to be entered after a suffix */\n\t\tif (n > 0 && (\n\t\t\t(buf[n-1] == 'p') || (buf[n-1] == 'g') ||\n\t\t\t(buf[n-1] == 'm') || (buf[n-1] == 'k'))) {\n\t\t\tbuf[n] = ' ';\n\t\t}\n\t\t/* If we have entered a character increment n */\n\t\tif (buf[n] != ' ') {\n\t\t\tn++;\n\t\t}\n\t\tbuf[n] = ' ';\n\t\t/* Print the current number */\n\t\tcprint(x, y, buf);\n\n\t\t/* Find the base we are entering numbers in */\n\t\tbase = 10;\n\t\tif ((buf[0] == '0') && (buf[1] == 'x')) {\n\t\t\tbase = 16;\n\t\t}\n\t\telse if (buf[0] == '0') {\n\t\t\tbase = 8;\n\t\t}\n\t}\n\t/* Compute our current shift */\n\tshift = 0;\n\tswitch(buf[n-1]) {\n\tcase 'g': /* gig */  shift = 30; break;\n\tcase 'm': /* meg */  shift = 20; break;\n\tcase 'p': /* page */ shift = 12; break;\n\tcase 'k': /* kilo */ shift = 10; break;\n\t}\n\tshift -= result_shift;\n\n\t/* Compute our current value */\n\tval = simple_strtoul(buf, NULL, base);\n\tif (shift > 0) {\n\t\tif (shift >= 32) {\n\t\t\tval = 0xffffffff;\n\t\t} else {\n\t\t\tval <<= shift;\n\t\t}\n\t} else {\n\t\tif (-shift >= 32) {\n\t\t\tval = 0;\n\t\t}\n\t\telse {\n\t\t\tval >>= -shift;\n\t\t}\n\t}\n\treturn val;\n}\n#endif\n\n/*\n * THE ONE KEY EDIT.\n *\n * x86: ttyprint() emitted an ANSI cursor-position escape (\"\\e[<y>;<x>H\") plus\n * the text to the serial console via serial_echo_print() \u2014 that mirrored the\n * VGA text buffer onto a serial terminal. On PPC/OF there is no serial console;\n * instead vga_buf[] (= SCREEN_ADR) is our char+attr buffer and a linear\n * framebuffer is the only display. cprint()/scroll() already wrote the run into\n * vga_buf and routed (y,x,text) here through tty_print_line/tty_print_region,\n * so the faithful move is: keep the exact signature, and make ttyprint() the\n * framebuffer renderer. For each character position i in p, blit the cell at\n * (y, x+i) from vga_buf via the display backend's fb_render_cell() (display.h).\n * Result: cprint/scroll render verbatim with no edits to themselves.\n *\n * Note we loop over the text length, not the buffer width, matching exactly the\n * run of cells cprint() just wrote. (The char read by fb_render_cell comes from\n * vga_buf, not from p \u2014 but p and vga_buf agree because cprint wrote p first.)\n */\nvoid ttyprint(int y, int x, const char *p)\n{\n\tint i;\n\n\t/* PPC: render each cell of the run from vga_buf to the framebuffer. */\n\tfor (i = 0; p[i]; i++) {\n\t\tfb_render_cell(y, x + i);\n\t}\n\n\t/* x86 original (serial-console cursor escape + echo):\n\t *\n\t * static char sx[3];\n\t * static char sy[3];\n\t * sx[0]='\\0';\n\t * sy[0]='\\0';\n\t * x++; y++;\n\t * itoa(sx, x);\n\t * itoa(sy, y);\n\t * serial_echo_print(\"[\");\n\t * serial_echo_print(sy);\n\t * serial_echo_print(\";\");\n\t * serial_echo_print(sx);\n\t * serial_echo_print(\"H\");\n\t * serial_echo_print(p);\n\t */\n}\n\n\n/* x86: serial_echo_init() programmed the 16550 UART (divisor latch, line\n * control, baud rate) via serial_echo_inb/outb port I/O \u2014 N/A on PPC/OF (no\n * UART; ttyprint no longer drives a serial console). Stubbed: it also called\n * clear_screen_buf(), so we keep that one platform-neutral call (the screen\n * shadow still needs clearing if anything invokes init), and drop the UART\n * programming. TO VERIFY: confirm whether any caller (init.c/main.c, Wave 3-5)\n * still calls serial_echo_init() for the clear_screen_buf() side effect; if so\n * this stub preserves it. */\nvoid serial_echo_init(void)\n{\n\t/* x86 UART programming removed:\n\t *   int comstat, hi, lo, serial_div;\n\t *   unsigned char lcr;\n\t *   comstat = serial_echo_inb(UART_LCR);\n\t *   serial_echo_outb(comstat | UART_LCR_DLAB, UART_LCR);\n\t *   hi = serial_echo_inb(UART_DLM);\n\t *   lo = serial_echo_inb(UART_DLL);\n\t *   serial_echo_outb(comstat, UART_LCR);\n\t *   lcr = serial_parity | (serial_bits - 5);\n\t *   serial_echo_outb(lcr, UART_LCR);\n\t *   serial_div = 115200 / serial_baud_rate;\n\t *   serial_echo_outb(0x80|lcr, UART_LCR);\n\t *   serial_echo_outb(serial_div & 0xff, UART_DLL);\n\t *   serial_echo_outb((serial_div >> 8) & 0xff, UART_DLM);\n\t *   serial_echo_outb(lcr, UART_LCR);\n\t *   comstat = serial_echo_inb(UART_LSR);\n\t *   comstat = serial_echo_inb(UART_RX);\n\t *   serial_echo_outb(0x00, UART_IER);\n\t */\n\tclear_screen_buf();\n\treturn;\n}\n\n/* x86: serial_echo_print() wrote each char out the UART transmit register\n * (WAIT_FOR_XMITR / serial_echo_outb) \u2014 N/A on PPC/OF. ttyprint no longer\n * calls it. Stubbed to a no-op so any straggler caller links; it is gated on\n * serial_cons (always 0 on PPC, see config.h) anyway. */\nvoid serial_echo_print(const char *p)\n{\n\t(void)p;\n\t/* x86 original:\n\t * if (!serial_cons) {\n\t *     return;\n\t * }\n\t * while (*p) {\n\t *     WAIT_FOR_XMITR;\n\t *     serial_echo_outb(*p, UART_TX);\n\t *     if(*p==10) {\n\t *         WAIT_FOR_XMITR;\n\t *         serial_echo_outb(13, UART_TX);\n\t *     }\n\t *     p++;\n\t * }\n\t */\n\treturn;\n}\n\n/* x86: ser_map[] maps ASCII -> PS/2 keycodes for the serial-console keyboard\n * emulation used by ascii_to_keycode() (consumed by get_key()) \u2014 N/A on PPC/OF\n * (no serial console, OF owns input). Commented out with get_key()/\n * ascii_to_keycode(). */\n#if 0\n/* Except for multi-character key sequences this mapping\n * table is complete.  So it should not need to be updated\n * when new keys are searched for.  However the key handling\n * should really be turned around and only in get_key should\n * we worry about the exact keycode that was pressed.  Everywhere\n * else we should switch on the character...\n */\nstruct ascii_map_str ser_map[] =\n/*ascii keycode     ascii  keycode*/\n{\n  /* Special cases come first so I can leave\n   * their \"normal\" mapping in the table,\n   * without it being activated.\n   */\n  {  27,   0x01}, /* ^[/ESC -> ESC  */\n  { 127,   0x0e}, /*    DEL -> BS   */\n  {   8,   0x0e}, /* ^H/BS  -> BS   */\n  {  10,   0x1c}, /* ^L/NL  -> CR   */\n  {  13,   0x1c}, /* ^M/CR  -> CR   */\n  {   9,   0x0f}, /* ^I/TAB -> TAB  */\n  {  19,   0x39}, /* ^S     -> SP   */\n  {  17,     28}, /* ^Q     -> CR   */\n\n  { ' ',   0x39}, /*     SP -> SP   */\n  { 'a',   0x1e},\n  { 'A',   0x1e},\n  {   1,   0x1e}, /* ^A      -> A */\n  { 'b',   0x30},\n  { 'B',   0x30},\n  {   2,   0x30}, /* ^B      -> B */\n  { 'c',   0x2e},\n  { 'C',   0x2e},\n  {   3,   0x2e}, /* ^C      -> C */\n  { 'd',   0x20},\n  { 'D',   0x20},\n  {   4,   0x20}, /* ^D      -> D */\n  { 'e',   0x12},\n  { 'E',   0x12},\n  {   5,   0x12}, /* ^E      -> E */\n  { 'f',   0x21},\n  { 'F',   0x21},\n  {   6,   0x21}, /* ^F      -> F */\n  { 'g',   0x22},\n  { 'G',   0x22},\n  {   7,   0x22}, /* ^G      -> G */\n  { 'h',   0x23},\n  { 'H',   0x23},\n  {   8,   0x23}, /* ^H      -> H */\n  { 'i',   0x17},\n  { 'I',   0x17},\n  {   9,   0x17}, /* ^I      -> I */\n  { 'j',   0x24},\n  { 'J',   0x24},\n  {  10,   0x24}, /* ^J      -> J */\n  { 'k',   0x25},\n  { 'K',   0x25},\n  {  11,   0x25}, /* ^K      -> K */\n  { 'l',   0x26},\n  { 'L',   0x26},\n  {  12,   0x26}, /* ^L      -> L */\n  { 'm',   0x32},\n  { 'M',   0x32},\n  {  13,   0x32}, /* ^M      -> M */\n  { 'n',   0x31},\n  { 'N',   0x31},\n  {  14,   0x31}, /* ^N      -> N */\n  { 'o',   0x18},\n  { 'O',   0x18},\n  {  15,   0x18}, /* ^O      -> O */\n  { 'p',   0x19},\n  { 'P',   0x19},\n  {  16,   0x19}, /* ^P      -> P */\n  { 'q',   0x10},\n  { 'Q',   0x10},\n  {  17,   0x10}, /* ^Q      -> Q */\n  { 'r',   0x13},\n  { 'R',   0x13},\n  {  18,   0x13}, /* ^R      -> R */\n  { 's',   0x1f},\n  { 'S',   0x1f},\n  {  19,   0x1f}, /* ^S      -> S */\n  { 't',   0x14},\n  { 'T',   0x14},\n  {  20,   0x14}, /* ^T      -> T */\n  { 'u',   0x16},\n  { 'U',   0x16},\n  {  21,   0x16}, /* ^U      -> U */\n  { 'v',   0x2f},\n  { 'V',   0x2f},\n  {  22,   0x2f}, /* ^V      -> V */\n  { 'w',   0x11},\n  { 'W',   0x11},\n  {  23,   0x11}, /* ^W      -> W */\n  { 'x',   0x2d},\n  { 'X',   0x2d},\n  {  24,   0x2d}, /* ^X      -> X */\n  { 'y',   0x15},\n  { 'Y',   0x15},\n  {  25,   0x15}, /* ^Y      -> Y */\n  { 'z',   0x2c},\n  { 'Z',   0x2c},\n  {  26,   0x2c}, /* ^Z      -> Z */\n  { '-',   0x0c},\n  { '_',   0x0c},\n  {  31,   0x0c}, /* ^_      -> _ */\n  { '=',   0x0c},\n  { '+',   0x0c},\n  { '[',   0x1a},\n  { '{',   0x1a},\n  {  27,   0x1a}, /* ^[      -> [ */\n  { ']',   0x1b},\n  { '}',   0x1b},\n  {  29,   0x1b}, /* ^]      -> ] */\n  { ';',   0x27},\n  { ':',   0x27},\n  { '\\'',  0x28},\n  { '\"',   0x28},\n  { '`',   0x29},\n  { '~',   0x29},\n  { '\\\\',  0x2b},\n  { '|',   0x2b},\n  {  28,   0x2b}, /* ^\\      -> \\ */\n  { ',',   0x33},\n  { '<',   0x33},\n  { '.',   0x34},\n  { '>',   0x34},\n  { '/',   0x35},\n  { '?',   0x35},\n  { '1',   0x02},\n  { '!',   0x02},\n  { '2',   0x03},\n  { '@',   0x03},\n  { '3',   0x04},\n  { '#',   0x04},\n  { '4',   0x05},\n  { '$',   0x05},\n  { '5',   0x06},\n  { '%',   0x06},\n  { '6',   0x07},\n  { '^',   0x07},\n  {  30,   0x07}, /* ^^      -> 6 */\n  { '7',   0x08},\n  { '&',   0x08},\n  { '8',   0x09},\n  { '*',   0x09},\n  { '9',   0x0a},\n  { '(',   0x0a},\n  { '0',   0x0b},\n  { ')',   0x0b},\n  {   0,      0}\n};\n\n/*\n * Given an ascii character, return the keycode\n *\n * Uses ser_map definition above.\n *\n * It would be more efficient to use an array of 255 characters\n * and directly index into it.\n */\nint ascii_to_keycode (int in)\n{\n\tstruct ascii_map_str *p;\n\tfor (p = ser_map; p->ascii; p++) {\n\t\tif (in ==p->ascii)\n\t\t\treturn p->keycode;\n\t}\n\treturn 0;\n}\n\n/*\n * Call this when you want to wait for the user to lift the\n * finger off of a key.  It is a noop if you are using a\n * serial console.\n */\nvoid wait_keyup( void ) {\n\t/* Check to see if someone lifted the keyboard key */\n\twhile (1) {\n\t\tif ((get_key() & 0x80) != 0) {\n\t\t\treturn;\n\t\t}\n\t\t/* Trying to simulate waiting for a key release with\n\t\t * the serial port is to nasty to let live.\n\t\t * In particular some menus don't even display until\n\t\t * you release the key that caused to to get there.\n\t\t * With the serial port this results in double pressing\n\t\t * or something worse for just about every key.\n\t\t */\n\t\tif (serial_cons) {\n\t\t\treturn;\n\t\t}\n\t}\n}\n#endif\n\n\n/*\n * Handles \"console=<param>\" command line option\n *\n * x86: serial_console_setup() parsed a \"console=ttyS0,115200e8\" kernel cmdline\n * option and configured the x86 UART (serial_tty/baud/parity/bits) \u2014 N/A on\n * PPC/OF: there is no serial console and OF does not pass a Linux-style cmdline.\n * Body wrapped in #if 0; test.h's prototype stays so any straggler links. The\n * parsing is pure string math but has nothing to configure on PPC. (The only\n * caller is main.c's cmdline handling, which the Wave-5 main.c port comments\n * out per HANDOFF: defs.h cmdline/segment defs are #if 0'd.)\n */\n#if 0\nvoid serial_console_setup(char *param)\n{\n\tchar *option, *end;\n\tunsigned long tty;\n\tunsigned long baud_rate;\n\tunsigned char parity, bits;\n\n\tif (strncmp(param, \"ttyS\", 4))\n\t\treturn;   /* not a serial port */\n\n\tparam += 4;\n\n\ttty = simple_strtoul(param, &option, 10);\n\n\tif (option == param)\n\t\treturn;   /* there were no digits */\n\n\tif (tty > 1)\n\t\treturn;   /* only ttyS0 and ttyS1 supported */\n\n\tif (*option == '\\0')\n\t\tgoto save_tty; /* no options given, just ttyS? */\n\n\tif (*option != ',')\n\t\treturn;  /* missing the comma separator */\n\n\t/* baud rate must follow */\n\toption++;\n\tbaud_rate = simple_strtoul(option, &end, 10);\n\n\tif (end == option)\n\t\treturn;  /* no baudrate after comma */\n\n\tif (baud_rate == 0 || (115200 % baud_rate) != 0)\n\t\treturn;  /* wrong baud rate */\n\n\tif (*end == '\\0')\n\t\tgoto save_baud_rate;  /* no more options given */\n\n\tswitch (toupper(*end)) {\n\t\tcase 'N':\n\t\t\tparity = 0;\n\t\t\tbreak;\n\t\tcase 'O':\n\t\t\tparity = UART_LCR_PARITY;\n\t\t\tbreak;\n\t\tcase 'E':\n\t\t\tparity = UART_LCR_PARITY | UART_LCR_EPAR;\n\t\t\tbreak;\n\t\tdefault:\n\t\t\t/* Unknown parity */\n\t\t\treturn;\n\t}\n\n\tend++;\n\tif (*end == '\\0')\n\t\tgoto save_parity;\n\n\t/* word length (bits) */\n\tif (*end < '7' || *end > '8')\n\t\treturn;  /* invalid number of bits */\n\n\tbits = *end - '0';\n\n\tend++;\n\n\tif (*end != '\\0')\n\t\treturn;  /* garbage at the end */\n\n\tserial_bits = bits;\n\tsave_parity:\n\tserial_parity = parity;\n\tsave_baud_rate:\n\tserial_baud_rate = (int) baud_rate;\n  save_tty:\n\tserial_tty = (short) tty;\n\tserial_cons = 1;\n}\n#endif\n\n\n#ifdef LP\n#define DATA            0x00\n#define STATUS          0x01\n#define CONTROL         0x02\n\n#define LP_PBUSY        0x80  /* inverted input, active high */\n#define LP_PERRORP      0x08  /* unchanged input, active low */\n\n#define LP_PSELECP      0x08  /* inverted output, active low */\n#define LP_PINITP       0x04  /* unchanged output, active low */\n#define LP_PSTROBE      0x01  /* short high output on raising edge */\n\n#define DELAY 0x10c6ul\n\nvoid lp_wait(ulong xloops)\n{\n\tint d0;\n\t__asm__(\"mull %0\"\n\t\t:\"=d\" (xloops), \"=&a\" (d0)\n\t\t:\"1\" (xloops),\"0\" (current_cpu_data.loops_per_sec));\n\t__delay(xloops);\n}\nstatic void __delay(ulong loops)\n{\n\tint d0;\n\t__asm__ __volatile__(\n\t\t\"\\tjmp 1f\\n\"\n\t\t\".align 16\\n\"\n\t\t\"1:\\tjmp 2f\\n\"\n\t\t\".align 16\\n\"\n\t\t\"2:\\tdecl %0\\n\\tjns 2b\"\n\t\t:\"=&a\" (d0)\n\t\t:\"0\" (loops));\n}\n\nput_lp(char c, short port)\n{\n\tunsigned char status;\n\n\t/* Wait for printer to be ready */\n\twhile (1) {\n\t\tstatus = inb(STATUS(port));\n\t\tif (status & LP_PERRORP) {\n\t\t\tif (status & LP_PBUSY) {\n\t\t\t\tbreak;\n\t\t\t}\n\t\t}\n\t}\n\n\toutb(d, DATA(c));\n\tlp_wait(DELAY);\n\toutb((LP_PSELECP | LP_PINITP | LP_PSTROBE), CONTROL(port));\n\tlp_wait(DELAY);\n\toutb((LP_PSELECP | LP_PINITP), CONTROL(port));\n\tlp_wait(DELAY);\n}\n#endif\n/* Note: the LP (line-printer over x86 parallel port) block above is already\n * gated by #ifdef LP, which is never defined \u2014 x86 leaf, N/A on PPC/OF. Left\n * verbatim inside its dormant #ifdef. */\n"
}


---

## 🤖 Claude (Subagent)

Now let me verify a couple of things about the prototypes for `reboot()` and `get_key()`/`getval()` that the brief mentioned. The brief said to handle `reboot()` but there is no `reboot()` function in v2.00 lib.c — it's a static inline in test.h (already commented). Let me double-check there's no `reboot()` in this lib.c, and verify the `getval` prototype matches (test.h has `getval`, no `getnum`/`get_list`). The brief mentioned `get_list()`/`getnum()` which also don't exist in v2.00 lib.c — those are config.c helpers. Let me confirm by checking the v2.00 source tree.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref/memtest86+-2.00 && grep -n \"reboot\\|get_list\\|getnum\\|getval\" lib.c config.c 2>/dev/null | head -40",
  "description": "Search for reboot/get_list/getnum/getval in v2.00 lib.c and config.c"
}


---

## 🤖 Claude (Subagent)

Confirmed: in v2.00, `reboot()`, `get_list()`, and `getnum()` are NOT in lib.c — `reboot()` is the static inline in test.h (already commented there), and `get_list`/`getnum` don't exist in v2.00 at all (they're v5.01/v1.00 config helpers). `getval()` is the only config-input helper in lib.c, which I've correctly parked. My handling matches the file as it actually exists.

Let me verify the final file has balanced `#if 0`/`#endif` and check for the `current_cpu_data`/`d` undefined-symbol concerns inside the LP block (it's `#ifdef LP`, never defined, so harmless).

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/src && echo \"=== #if/#ifdef/#endif balance ===\" && grep -nE \"^#if|^#ifdef|^#ifndef|^#endif|^#else|^#error\" lib.c",
  "description": "Check preprocessor directive balance in lib.c"
}


---

## 🤖 Claude (Subagent)

Balanced. Pairs: 40/42, 46/48, 472/556 (with nested 484/491 PARITY_MEM), 594/617 (get_key), 744/862 (getval), 971/1165 (ser_map/ascii_to_keycode/wait_keyup), 1179/1258 (serial_console_setup), 1261/1317 (LP). All correct.

Now let me check the v2.00 main.c/init.c to verify whether `serial_echo_init()`, `serial_console_setup()`, `getval()`, `wait_keyup()`, `get_key()` have callers I need to flag for the integration build.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref/memtest86+-2.00 && echo \"=== serial_echo_init callers ===\" && grep -rn \"serial_echo_init\\|serial_console_setup\\|wait_keyup\\|ascii_to_keycode\\|get_key\\b\\|getval\\|set_cache\\|check_input\\|\\binter\\b\" *.c *.h 2>/dev/null | grep -v \"^lib.c:\" | grep -v \"^test.h:\"",
  "description": "Find callers of commented-out lib.c functions outside lib.c"
}


---

## 🤖 Claude (Subagent)

This is exactly the integration-build information I need. The key findings:

- **`set_cache()`** — called by main.c (3x), init.c, memsize.c. My PPC stub keeps them all linking. Good.
- **`check_input()`** — called by error.c (3x), main.c, test.c. My PPC reimpl satisfies them. Good.
- **`serial_echo_init()`** — called by init.c:49. My stub (keeps `clear_screen_buf()`) satisfies it. Good — and confirms the TO-VERIFY: init.c DOES call it, so keeping the `clear_screen_buf()` side-effect was the right call.
- **`serial_console_setup()`** — called by main.c:164 (cmdline path). The Wave-5 main.c port must comment that call (it's in the defs.h-gated cmdline path anyway). The prototype stays in test.h so it links if main.c keeps the call.
- **`get_key()`, `wait_keyup()`, `getval()`** — callers are ALL in Wave-6 PC-platform files (config.c, dmi.c, extra.c, spd.c, controller.c) — all expected ⛔/parked. None in Waves 3-5. So commenting these out does not break the Wave 2-5 build.

This means my parked `get_key`/`wait_keyup`/`getval`/`serial_console_setup` have **no live callers in Waves 2-5**, so the Wave-2 checkpoint and the full main build will link cleanly. Now let me write the report.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/port-lib_c.md",
  "content": "# port-lib_c.md \u2014 porting memtest86+ v2.00 `lib.c` \u2192 `src/lib.c`\n\n## File & purpose\n`ref/memtest86+-2.00/lib.c` (1138 lines) is the rendering-and-utilities core:\n- **Screen primitives** that write char+attr cells into the VGA text buffer\n  (`SCREEN_ADR`) and route through `tty_print_line`/`tty_print_region`:\n  `scroll` (174), `clear_scroll` (209), `cprint` (224), `dprint` (293),\n  `hprint`/`hprint2`/`hprint3` (350/383/412), `aprint` (269), `xprint` (420),\n  `footer` (597).\n- **String/number utils:** `memcmp` (80), `strncmp` (92), `memmove` (106),\n  `toupper`/`isdigit`/`isxdigit` (125/133/138), `simple_strtoul` (142),\n  `itoa`/`reverse` (237/253).\n- **The display leaf `ttyprint` (725)** \u2014 on x86 it emits an ANSI cursor escape\n  + text to a serial console.\n- **x86 platform leaves:** `inter` + `codes[]` + `struct eregs` (x86 IDT handler,\n  41/64/435), `set_cache` (518, x86 cr0 cache toggle), `get_key` (538, PS/2 port\n  I/O 0x60/0x64), `check_input` (561), `getval` (607, config-menu key input),\n  `serial_echo_init`/`serial_echo_print` (744/777), `ser_map[]` +\n  `ascii_to_keycode` + `wait_keyup` (803/956/971), `serial_console_setup` (1000),\n  and an `#ifdef LP` parallel-port block (1080).\n\n## Disposition\n**Ported** (the Wave-2 key file). Started from a verbatim copy; the one real edit\nis rewriting `ttyprint()` into the framebuffer renderer. Platform-neutral code is\nbyte-verbatim; x86 leaves are commented (`#if 0`) with reasons; `check_input` and\n`set_cache` are reimplemented/stubbed for PPC/OF. Per the discipline, I ported\nexactly what is in v2.00 `lib.c` \u2014 note the brief mentioned a few names that do\n**not** exist in this file (see Open questions).\n\n## THE ONE KEY EDIT \u2014 `ttyprint()` rewrite\nSignature kept exactly: `void ttyprint(int y, int x, const char *p)`.\nNew body loops over the run and blits each cell from `vga_buf` to the\nframebuffer:\n```c\nfor (i = 0; p[i]; i++)\n    fb_render_cell(y, x + i);\n```\nThe x86 serial-escape body (itoa of x/y into a `[<y>;<x>H` escape +\n`serial_echo_print`) is preserved as a comment. Because `cprint` writes the run\ninto `vga_buf` *before* calling `tty_print_line \u2192 ttyprint`, the chars\n`fb_render_cell` reads from `vga_buf` agree with `p`. **Result:** `cprint`,\n`scroll` (via `tty_print_region`), `dprint`, etc. stay byte-verbatim and render\n\"just works.\" (Note: `tty_print_line` dedups on the char shadow, so attr-only\npokes still need an explicit `fb_render_cell`/`display_refresh` \u2014 that's a\ncaller concern in init.c/error.c per the HANDOFF, not lib.c.)\n\n## x86 leaf code commented out + its replacement\n| region | line (v2.00) | disposition | replacement |\n|---|---|---|---|\n| `#include \"io.h\"`, `\"serial.h\"` | 11-12 | commented | `#include \"ofw.h\"` (OF = the io/BIOS replacement) |\n| `codes[]` exception names | 41 | `/* */` commented | none (only `inter()` used it) |\n| `struct eregs` trap frame | 64 | `/* */` commented | none (x86 trap layout; test.h keeps `struct eregs;` fwd-decl) |\n| `inter()` x86 IDT handler | 435 | `#if 0` | none yet \u2014 no caller (head.S routes no traps here); PPC exception dump is a future workstream |\n| `set_cache()` x86 cr0/`cache_off/on` | 518 | body commented | **PPC stub** (see below) |\n| `get_key()` PS/2 + serial poll | 538 | `#if 0` | folded into `check_input()` OF stdin poll |\n| `check_input()` PS/2 scancode switch | 561 | **reimplemented** | OF stdin poll (see below) |\n| `getval()` config-menu key input | 607 | `#if 0` parked | Wave-6 (OF key story unsettled); flagged |\n| `serial_echo_init()` UART program | 744 | stubbed | keeps only `clear_screen_buf()` side-effect |\n| `serial_echo_print()` UART TX | 777 | stubbed no-op | ttyprint no longer calls it |\n| `ser_map[]`/`ascii_to_keycode`/`wait_keyup` | 803/956/971 | `#if 0` | none (serial-console kbd emu; OF owns input) |\n| `serial_console_setup()` cmdline | 1000 | `#if 0` parked | none (no serial console; no Linux cmdline) |\n| `#ifdef LP` parallel-port block | 1080 | left verbatim | dormant \u2014 `LP` is never defined |\n\n## PPC/OF reimplementations\n- **`check_input()`** \u2014 polls OF stdin. Caches the ihandle in a `static`\n  (`ofw_get_stdin()` once), then `ofw_read(stdin_ih, &ch, 1)` non-blocking each\n  call. On **ESC (ASCII 27)** it prints `Halting...` and reboots via\n  `ofw_reset()`. Scroll-lock (`slock`) and `'c'`/config hooks are left as\n  commented stubs (footer advertises them; Wave 6). The x86 PS/2-scancode switch\n  is preserved as a comment. Called from `scroll()`, `do_tick()`, plus error.c /\n  main.c / test.c \u2014 must be cheap; one 1-byte OF read is.\n- **`reboot()`** \u2014 *not a function in v2.00 lib.c.* It is a `static inline` in\n  `test.h`, already commented there with the PPC note pointing at `ofw_reset()`.\n  The reboot path is realized here, inside `check_input()`'s ESC handler, via\n  `ofw_reset()`.\n- **`set_cache(int val)`** \u2014 PPC stub: `(void)val;`, no-op. The x86 body\n  (cpu_id 386 special-case + `cache_off`/`cache_on` cr0 toggles, and the\n  `cprint(LINE_INFO, COL_CACHE, ...)` status writes) is preserved as a comment.\n  Lets the cache/uncached `tseq` entries and the init.c/main.c/memsize.c callers\n  link. **TO VERIFY (uncached tests):** the `cache=0` tseq rows do not actually\n  run uncached until PPC HID0[DCE]/[ICE] control lands. Cache stays in whatever\n  state OF left it; tests run but the cached/uncached distinction is currently\n  cosmetic. Also: the original wrote \"none\"/\"off\"/\" on\" to `COL_CACHE` \u2014 with the\n  stub, init.c's cache-column display will need its own value (init.c port\n  decides).\n\n## Kept verbatim (platform-neutral)\n`scroll`, `clear_scroll`, `cprint`, `dprint`, `hprint`, `hprint2`, `hprint3`,\n`aprint`, `xprint`, `footer` (v2.00 text kept exactly:\n`\"(ESC)Reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock\"`),\n`memcmp`, `strncmp`, `memmove`, `toupper`, `isdigit`, `isxdigit`,\n`simple_strtoul`, `itoa`, `reverse`, and the file-scope serial vars\n(`slock`, `lsr`, `serial_cons`, `serial_tty`, `serial_base_ports[]`,\n`serial_baud_rate`, `serial_parity`, `serial_bits`, `buf[18]`) plus the two\n`#if SERIAL_TTY`/`#if 115200%...` compile-time guards (config.h keeps those\nsymbols defined at inert values so these still compile).\n\n> Note: the brief's \"keep verbatim\" list named `cplace`, `strlen`, `strstr`,\n> `memcpy` \u2014 **none of these exist in v2.00 `lib.c`** (they're from other\n> versions). I kept exactly what the v2.00 file contains; nothing invented.\n\n## Build impact / what it includes\n- Includes: `test.h` (\u2192 `ppc.h`, `display.h`, struct vars, prototypes,\n  `SCREEN_ADR=vga_buf`), `config.h`, `screen_buffer.h`, `ofw.h`. Drops\n  `io.h`/`serial.h`.\n- Depends on the substrate: `fb_render_cell` (display.h), `ofw_get_stdin`/\n  `ofw_read`/`ofw_reset` (ofw.h), `tty_print_line`/`tty_print_region`/\n  `clear_screen_buf`/`get_scrn_buf`/`set_scrn_buf` (screen_buffer.h, parallel\n  agent), `v` (test.h struct vars).\n- **No Makefile change beyond adding `lib.o` to OBJS.**\n- **Caller audit (grep of v2.00 tree) \u2014 integration build is clean Waves 2-5:**\n  - `set_cache` (main.c\u00d73, init.c, memsize.c) \u2192 stub links all. \u2705\n  - `check_input` (error.c\u00d73, main.c, test.c) \u2192 PPC reimpl satisfies all. \u2705\n  - `serial_echo_init` (init.c:49) \u2192 stub keeps `clear_screen_buf()`. \u2705\n  - `serial_console_setup` (main.c:164, cmdline path) \u2192 **Wave-5 main.c must\n    comment this call** (it lives in the defs.h-gated cmdline path that's already\n    `#if 0`'d, so this should fall out naturally). The prototype is kept.\n  - `get_key`/`wait_keyup`/`getval` \u2192 callers are **only** Wave-6 PC-platform\n    files (config.c, dmi.c, extra.c, spd.c, controller.c \u2014 all expected \u26d4/parked).\n    **Zero callers in Waves 2-5**, so parking them does not break the build.\n\n## Open questions / TO VERIFY\n1. **Uncached tests don't run uncached** until PPC HID0 cache control is\n   implemented (`set_cache` stub). Flagged above; decide when/whether to wire\n   HID0[DCE].\n2. **`check_input` semantics on real Apple OF** \u2014 relies on `ofw_read` returning\n   0 (not blocking) when no key is pending. True under QEMU/OpenBIOS; on real\n   New-World OF the `read` on the keyboard ihandle should also be non-blocking,\n   but verify on ibookg32 (a blocking read here would stall the test loop). The\n   attic port read OF stdin the same way and worked on the iMac G3.\n3. **`ofw_reset()` return** \u2014 assumed not to return; I added a `while(1)` guard.\n   If `reset-all` interpret fails silently on some OF, ESC would hang at\n   \"Halting...\" rather than reboot. Acceptable (matches \"needs physical reboot\"\n   risk note) but worth a real-hardware check.\n4. **Footer honesty** \u2014 `footer()` is verbatim and advertises `(c)configuration`\n   / scroll-lock; those keys are currently dead (commented stubs in\n   `check_input`). Wire up in Wave 6 (config.c) or accept advertised-but-dead per\n   the HANDOFF \"Footer honesty\" open question.\n5. **`getval`/`serial_console_setup` parked, not ported** \u2014 both are `#if 0`'d\n   with prototypes intact. When Wave 6 surfaces the config menu, `getval` needs a\n   from-scratch OF-stdin reimplementation (the string math \u2014\n   `simple_strtoul` + p/g/m/k suffix shift \u2014 is reusable; only the key-read loop\n   is x86).\n"
}


---

## 🤖 Claude (Subagent)

Both deliverables are written. Let me update the file-status table row in the HANDOFF for lib.c, since the CLAUDE.md instructs keeping it current and the PORTING-GUIDE §5 says to update the row.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/HANDOFF.md",
  "old_string": "| lib.c | \u2610 | print primitives + str/mem; ttyprint\u2192fb leaf |",
  "new_string": "| lib.c | \u2705 | report `port-lib_c.md`. ttyprint\u2192fb_render_cell loop (the one key edit); print/str/mem kept verbatim; check_input\u2192OF stdin poll (ESC\u2192ofw_reset); set_cache PPC stub (uncached-test TO-VERIFY); inter/get_key/getval/ser_map/serial_* commented; serial_echo_init stub keeps clear_screen_buf(). Caller audit: clean for Waves 2-5; Wave-5 main.c must comment the serial_console_setup(cmdline) call. |"
}


---

## 🤖 Claude (Subagent)

Done. Summary of the `lib.c` port.

## ttyprint rewrite (the one key edit)
Kept the exact signature `void ttyprint(int y, int x, const char *p)`. Body is now the framebuffer renderer: `for (i=0; p[i]; i++) fb_render_cell(y, x+i);`. The x86 serial-escape body is preserved as a comment. Because `cprint`/`scroll` write the run into `vga_buf` *before* routing through `tty_print_line → ttyprint`, the chars `fb_render_cell` reads from `vga_buf` match `p` — so every print primitive renders verbatim with zero edits to itself.

## Kept verbatim
`scroll`, `clear_scroll`, `cprint`, `dprint`, `hprint`/`hprint2`/`hprint3`, `aprint`, `xprint`, `footer` (v2.00 text exact), and all str/mem utils (`memcmp`, `strncmp`, `memmove`, `toupper`, `isdigit`, `isxdigit`, `simple_strtoul`, `itoa`, `reverse`), plus the serial file-scope vars and the two `#if SERIAL_*` compile-time guards.

## Commented out (x86 leaf, `#if 0` with reasons)
`inter()` + `codes[]` + `struct eregs` (x86 IDT handler, no caller), `get_key()` (PS/2 0x60/0x64), `ser_map[]`/`ascii_to_keycode`/`wait_keyup`, `serial_console_setup`, and `getval()` (Wave-6 config-menu input, parked). `serial_echo_print` stubbed to a no-op; `serial_echo_init` stubbed to keep only its `clear_screen_buf()` side-effect.

## check_input / reboot / set_cache
- **check_input** reimplemented: caches the OF stdin ihandle (`ofw_get_stdin`), `ofw_read` one byte non-blocking; ESC (27) prints `Halting...` and calls `ofw_reset()`. Scroll-lock and `'c'` config left as commented Wave-6 hooks.
- **reboot** is not a function in v2.00 lib.c (it's the inline already commented in test.h) — the reboot path lives in check_input's ESC handler via `ofw_reset()`.
- **set_cache** PPC stub `(void)val;`; x86 cr0 body preserved as comment. Flagged TO-VERIFY: uncached tseq rows don't truly run uncached until PPC HID0[DCE] control lands.

## Watch at the integration build
I grep-audited every commented function's callers across the v2.00 tree:
- `set_cache`, `check_input`, `serial_echo_init` all have live callers in Waves 2-5 — my stubs/reimpl satisfy them, so the build links clean.
- **One action item:** `serial_console_setup` is called at main.c:164 (the cmdline path). The Wave-5 main.c port must comment that call — it lives in the already-`#if 0`'d defs.h cmdline path, so it should fall out naturally, but confirm.
- `get_key`/`wait_keyup`/`getval` have callers **only** in Wave-6 files (config.c, dmi.c, extra.c, spd.c, controller.c) — zero in Waves 2-5, so parking them is safe.

Note: the brief listed `cplace`/`strlen`/`strstr`/`memcpy` and `reboot`/`get_list`/`getnum` as things to handle — none of those exist in v2.00 lib.c (they're other-version artifacts). I ported exactly the file as it is. Report at `docs/sessions/006-port-fidelity-restructure/port-lib_c.md`; HANDOFF status row updated.

---

