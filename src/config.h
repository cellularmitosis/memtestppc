/* config.h - MemTest-86  Version 3.0
 *
 * Compile time configuration options
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 */

/* ----------------------------------------------------------------------------
 * memtestppc+ port note (session 006, v2.00 -> PowerPC/Open Firmware):
 *
 * This is a single-CPU PowerPC/OF memory tester. There is no serial console
 * (our ttyprint drives the linear framebuffer, not a UART), no PC speaker,
 * no x86 APM, no BIOS USB-keyboard emulation, and no x86 memory parity bus.
 * Per the porting discipline we IMPORT this file verbatim and then, for each
 * flag, KEEP it (platform-neutral) or DISABLE it (x86/PC-specific) with a
 * one-line reason. Nothing is deleted; the original intent stays visible.
 *
 * The two flags the task brief mentions but that DO NOT exist in v2.00 --
 * BEEP_END_NO_ERROR and CONSERVATIVE_SMP -- are v5.01 additions and are
 * absent here; there is nothing to disable for them.
 * ------------------------------------------------------------------------- */

/* PARITY_MEM - Enables support for reporting memory parity errors */
/*	Experimental, normally enabled */
/* x86: NMI-driven ISA/PCI parity-error reporting (parity_err(), lib.c:445,
 * error.c:374) — N/A on PPC/OF (no x86 parity NMI bus). DISABLED. */
/* #define PARITY_MEM */

/* SERIAL_CONSOLE_DEFAULT -  The default state of the serial console. */
/*	This is normally off since it slows down testing.  Change to a 1 */
/*	to enable. */
/* PPC: kept defined and forced to 0 (off). On PPC/OF there is no serial
 * console — ttyprint renders to the framebuffer, not a UART — so leaving the
 * default at 0 means lib.c's serial_cons never goes hot. KEEP (value 0). */
#define SERIAL_CONSOLE_DEFAULT 0

/* SERIAL_TTY - The default serial port to use. 0=ttyS0, 1=ttyS1 */
/* x86: selects an x86 UART (0x3f8/0x2f8) — N/A on PPC/OF (no serial console).
 * KEPT DEFINED so lib.c's compile-time guard "#if SERIAL_TTY != 0 && != 1"
 * (lib.c:21) still compiles; the serial code it feeds is commented out in the
 * lib.c port. Value left at 0. */
#define SERIAL_TTY 0

/* SERIAL_BAUD_RATE - Baud rate for the serial console */
/* x86: UART divisor base — N/A on PPC/OF. KEPT DEFINED so lib.c's compile-time
 * guard "#if ((115200%SERIAL_BAUD_RATE) != 0)" (lib.c:27) still compiles.
 * Value left at 9600. */
#define SERIAL_BAUD_RATE 9600

/* BEEP_MODE - Beep on error. Default off, Change to 1 to enable */
/* x86: drives beep() which programs the PC speaker via PIT port 0x43/0x61
 * (test.c:1405, error.c:72-73) — N/A on PPC/OF (no PC speaker). KEPT DEFINED
 * and forced to 0 (off) so init.c's "beepmode = BEEP_MODE" (init.c:113)
 * compiles and the beep path stays dead. */
#define BEEP_MODE 0

/* SCRN_DEBUG - extra check for SCREEN_BUFFER
 */
/* Platform-neutral SCREEN_BUFFER consistency check (screen_buffer.c). Left
 * disabled exactly as upstream ships it. KEEP (commented, as upstream). */
/* #define SCRN_DEBUG */

/* APM - Turns off APM at boot time to avoid blanking the screen */
/*	Normally enabled */
/* x86: gates head.S code that issues the real-mode APM "disconnect" BIOS call
 * (head.S:888) — N/A on PPC/OF (no APM; our head.S is PPC asm). DISABLED. */
/* #define APM_OFF */

/* USB_WAR - Enables a workaround for errors caused by BIOS USB keyboard */
/*	and mouse support*/
/*	Normally enabled */
/* x86: works around BIOS USB legacy-keyboard emulation corrupting low memory
 * (error.c:36, test.c:1087/1156/1223) — N/A on PPC/OF (OF owns ADB/USB input,
 * no SMM legacy emulation). DISABLED. */
/* #define USB_WAR */
