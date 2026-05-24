# port-lib_c.md — porting memtest86+ v2.00 `lib.c` → `src/lib.c`

## File & purpose
`ref/memtest86+-2.00/lib.c` (1138 lines) is the rendering-and-utilities core:
- **Screen primitives** that write char+attr cells into the VGA text buffer
  (`SCREEN_ADR`) and route through `tty_print_line`/`tty_print_region`:
  `scroll` (174), `clear_scroll` (209), `cprint` (224), `dprint` (293),
  `hprint`/`hprint2`/`hprint3` (350/383/412), `aprint` (269), `xprint` (420),
  `footer` (597).
- **String/number utils:** `memcmp` (80), `strncmp` (92), `memmove` (106),
  `toupper`/`isdigit`/`isxdigit` (125/133/138), `simple_strtoul` (142),
  `itoa`/`reverse` (237/253).
- **The display leaf `ttyprint` (725)** — on x86 it emits an ANSI cursor escape
  + text to a serial console.
- **x86 platform leaves:** `inter` + `codes[]` + `struct eregs` (x86 IDT handler,
  41/64/435), `set_cache` (518, x86 cr0 cache toggle), `get_key` (538, PS/2 port
  I/O 0x60/0x64), `check_input` (561), `getval` (607, config-menu key input),
  `serial_echo_init`/`serial_echo_print` (744/777), `ser_map[]` +
  `ascii_to_keycode` + `wait_keyup` (803/956/971), `serial_console_setup` (1000),
  and an `#ifdef LP` parallel-port block (1080).

## Disposition
**Ported** (the Wave-2 key file). Started from a verbatim copy; the one real edit
is rewriting `ttyprint()` into the framebuffer renderer. Platform-neutral code is
byte-verbatim; x86 leaves are commented (`#if 0`) with reasons; `check_input` and
`set_cache` are reimplemented/stubbed for PPC/OF. Per the discipline, I ported
exactly what is in v2.00 `lib.c` — note the brief mentioned a few names that do
**not** exist in this file (see Open questions).

## THE ONE KEY EDIT — `ttyprint()` rewrite
Signature kept exactly: `void ttyprint(int y, int x, const char *p)`.
New body loops over the run and blits each cell from `vga_buf` to the
framebuffer:
```c
for (i = 0; p[i]; i++)
    fb_render_cell(y, x + i);
```
The x86 serial-escape body (itoa of x/y into a `[<y>;<x>H` escape +
`serial_echo_print`) is preserved as a comment. Because `cprint` writes the run
into `vga_buf` *before* calling `tty_print_line → ttyprint`, the chars
`fb_render_cell` reads from `vga_buf` agree with `p`. **Result:** `cprint`,
`scroll` (via `tty_print_region`), `dprint`, etc. stay byte-verbatim and render
"just works." (Note: `tty_print_line` dedups on the char shadow, so attr-only
pokes still need an explicit `fb_render_cell`/`display_refresh` — that's a
caller concern in init.c/error.c per the HANDOFF, not lib.c.)

## x86 leaf code commented out + its replacement
| region | line (v2.00) | disposition | replacement |
|---|---|---|---|
| `#include "io.h"`, `"serial.h"` | 11-12 | commented | `#include "ofw.h"` (OF = the io/BIOS replacement) |
| `codes[]` exception names | 41 | `/* */` commented | none (only `inter()` used it) |
| `struct eregs` trap frame | 64 | `/* */` commented | none (x86 trap layout; test.h keeps `struct eregs;` fwd-decl) |
| `inter()` x86 IDT handler | 435 | `#if 0` | none yet — no caller (head.S routes no traps here); PPC exception dump is a future workstream |
| `set_cache()` x86 cr0/`cache_off/on` | 518 | body commented | **PPC stub** (see below) |
| `get_key()` PS/2 + serial poll | 538 | `#if 0` | folded into `check_input()` OF stdin poll |
| `check_input()` PS/2 scancode switch | 561 | **reimplemented** | OF stdin poll (see below) |
| `getval()` config-menu key input | 607 | `#if 0` parked | Wave-6 (OF key story unsettled); flagged |
| `serial_echo_init()` UART program | 744 | stubbed | keeps only `clear_screen_buf()` side-effect |
| `serial_echo_print()` UART TX | 777 | stubbed no-op | ttyprint no longer calls it |
| `ser_map[]`/`ascii_to_keycode`/`wait_keyup` | 803/956/971 | `#if 0` | none (serial-console kbd emu; OF owns input) |
| `serial_console_setup()` cmdline | 1000 | `#if 0` parked | none (no serial console; no Linux cmdline) |
| `#ifdef LP` parallel-port block | 1080 | left verbatim | dormant — `LP` is never defined |

## PPC/OF reimplementations
- **`check_input()`** — polls OF stdin. Caches the ihandle in a `static`
  (`ofw_get_stdin()` once), then `ofw_read(stdin_ih, &ch, 1)` non-blocking each
  call. On **ESC (ASCII 27)** it prints `Halting...` and reboots via
  `ofw_reset()`. Scroll-lock (`slock`) and `'c'`/config hooks are left as
  commented stubs (footer advertises them; Wave 6). The x86 PS/2-scancode switch
  is preserved as a comment. Called from `scroll()`, `do_tick()`, plus error.c /
  main.c / test.c — must be cheap; one 1-byte OF read is.
- **`reboot()`** — *not a function in v2.00 lib.c.* It is a `static inline` in
  `test.h`, already commented there with the PPC note pointing at `ofw_reset()`.
  The reboot path is realized here, inside `check_input()`'s ESC handler, via
  `ofw_reset()`.
- **`set_cache(int val)`** — PPC stub: `(void)val;`, no-op. The x86 body
  (cpu_id 386 special-case + `cache_off`/`cache_on` cr0 toggles, and the
  `cprint(LINE_INFO, COL_CACHE, ...)` status writes) is preserved as a comment.
  Lets the cache/uncached `tseq` entries and the init.c/main.c/memsize.c callers
  link. **TO VERIFY (uncached tests):** the `cache=0` tseq rows do not actually
  run uncached until PPC HID0[DCE]/[ICE] control lands. Cache stays in whatever
  state OF left it; tests run but the cached/uncached distinction is currently
  cosmetic. Also: the original wrote "none"/"off"/" on" to `COL_CACHE` — with the
  stub, init.c's cache-column display will need its own value (init.c port
  decides).

## Kept verbatim (platform-neutral)
`scroll`, `clear_scroll`, `cprint`, `dprint`, `hprint`, `hprint2`, `hprint3`,
`aprint`, `xprint`, `footer` (v2.00 text kept exactly:
`"(ESC)Reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock"`),
`memcmp`, `strncmp`, `memmove`, `toupper`, `isdigit`, `isxdigit`,
`simple_strtoul`, `itoa`, `reverse`, and the file-scope serial vars
(`slock`, `lsr`, `serial_cons`, `serial_tty`, `serial_base_ports[]`,
`serial_baud_rate`, `serial_parity`, `serial_bits`, `buf[18]`) plus the two
`#if SERIAL_TTY`/`#if 115200%...` compile-time guards (config.h keeps those
symbols defined at inert values so these still compile).

> Note: the brief's "keep verbatim" list named `cplace`, `strlen`, `strstr`,
> `memcpy` — **none of these exist in v2.00 `lib.c`** (they're from other
> versions). I kept exactly what the v2.00 file contains; nothing invented.

## Build impact / what it includes
- Includes: `test.h` (→ `ppc.h`, `display.h`, struct vars, prototypes,
  `SCREEN_ADR=vga_buf`), `config.h`, `screen_buffer.h`, `ofw.h`. Drops
  `io.h`/`serial.h`.
- Depends on the substrate: `fb_render_cell` (display.h), `ofw_get_stdin`/
  `ofw_read`/`ofw_reset` (ofw.h), `tty_print_line`/`tty_print_region`/
  `clear_screen_buf`/`get_scrn_buf`/`set_scrn_buf` (screen_buffer.h, parallel
  agent), `v` (test.h struct vars).
- **No Makefile change beyond adding `lib.o` to OBJS.**
- **Caller audit (grep of v2.00 tree) — integration build is clean Waves 2-5:**
  - `set_cache` (main.c×3, init.c, memsize.c) → stub links all. ✅
  - `check_input` (error.c×3, main.c, test.c) → PPC reimpl satisfies all. ✅
  - `serial_echo_init` (init.c:49) → stub keeps `clear_screen_buf()`. ✅
  - `serial_console_setup` (main.c:164, cmdline path) → **Wave-5 main.c must
    comment this call** (it lives in the defs.h-gated cmdline path that's already
    `#if 0`'d, so this should fall out naturally). The prototype is kept.
  - `get_key`/`wait_keyup`/`getval` → callers are **only** Wave-6 PC-platform
    files (config.c, dmi.c, extra.c, spd.c, controller.c — all expected ⛔/parked).
    **Zero callers in Waves 2-5**, so parking them does not break the build.

## Open questions / TO VERIFY
1. **Uncached tests don't run uncached** until PPC HID0 cache control is
   implemented (`set_cache` stub). Flagged above; decide when/whether to wire
   HID0[DCE].
2. **`check_input` semantics on real Apple OF** — relies on `ofw_read` returning
   0 (not blocking) when no key is pending. True under QEMU/OpenBIOS; on real
   New-World OF the `read` on the keyboard ihandle should also be non-blocking,
   but verify on ibookg32 (a blocking read here would stall the test loop). The
   attic port read OF stdin the same way and worked on the iMac G3.
3. **`ofw_reset()` return** — assumed not to return; I added a `while(1)` guard.
   If `reset-all` interpret fails silently on some OF, ESC would hang at
   "Halting..." rather than reboot. Acceptable (matches "needs physical reboot"
   risk note) but worth a real-hardware check.
4. **Footer honesty** — `footer()` is verbatim and advertises `(c)configuration`
   / scroll-lock; those keys are currently dead (commented stubs in
   `check_input`). Wire up in Wave 6 (config.c) or accept advertised-but-dead per
   the HANDOFF "Footer honesty" open question.
5. **`getval`/`serial_console_setup` parked, not ported** — both are `#if 0`'d
   with prototypes intact. When Wave 6 surfaces the config menu, `getval` needs a
   from-scratch OF-stdin reimplementation (the string math —
   `simple_strtoul` + p/g/m/k suffix shift — is reusable; only the key-read loop
   is x86).
