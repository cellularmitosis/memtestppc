# Port report — `screen_buffer.c` / `screen_buffer.h` (Wave 2)

## File & purpose
Upstream `ref/memtest86+-2.00/screen_buffer.{c,h}` (by Jani Averbach, 2001) is
the screen-output **change-detection layer** that sits between the print
primitives (`cprint` etc. in lib.c) and the actual output leaf (`ttyprint`). It
maintains a **char-only shadow** of the 80×25 text screen and only re-emits the
cells that actually changed.

- `screen_buf[Y_SIZE][X_SIZE]` (`screen_buffer.c:26`) — `static char[25][81]`;
  the extra column holds a per-row `'\0'` terminator (`X_SIZE = SCREEN_X+1`,
  line 24) so a row can be handed to `ttyprint` as a C string.
- `get_scrn_buf` / `set_scrn_buf` (lines 40–56) — accessors used by callers that
  want to read/poke the shadow directly.
- `clear_screen_buf` (lines 58–70) — fills the shadow with spaces and
  re-terminates each row.
- `tty_print_region` (lines 72–91) — for each row in `[top,bottom) × [left,right)`,
  temporarily NUL-terminates at `pi_right`, calls `ttyprint(y, pi_left, &row[pi_left])`,
  then restores the saved byte. (Used by `scroll`/`tty_print_screen`.)
- `tty_print_line` (lines 93–109) — the dedup core: scans from `x` to find the
  first cell whose new char differs from the shadow, returns early if the whole
  string already matches, otherwise calls `ttyprint(y, x, text)` for the changed
  tail and writes the new chars into the shadow.
- `tty_print_screen` (lines 112–122) — full repaint via `tty_print_region(0,0,25,80)`
  (plus a `SCRN_DEBUG`-only padding pass).
- `print_error` (lines 124–134) — `SCRN_DEBUG` assert sink: prints at (0,35) and
  spins forever.

`SCRN_DEBUG` is **off** (config.h ships `/* #define SCRN_DEBUG */` commented), so
`CHECK_BOUNDS` expands to nothing and `padding`/the debug `print_error` path are
compiled out — identical to upstream's default build.

## Disposition
**Ported — VERBATIM.** Imported as `src/screen_buffer.{c,h}`. Code bodies are
**byte-for-byte identical** to upstream v2.00 (verified with `diff` from the
`#include "test.h"` / `#ifndef` guard onward — `BODY IDENTICAL`). The only
addition is the standard memtestppc+ port-header comment block at the top of each
file, per PORTING-GUIDE §1. No code lines were added, removed, or commented out.

## x86 leaf code commented out
**None.** This file is platform-neutral plumbing — pure C array manipulation and
a single forward call to `ttyprint`. There is nothing x86-specific to comment
(as expected and called out in the task brief).

## PPC/OF additions
**None in this file.** The rendering swap happens entirely in the `ttyprint`
*leaf* (lib.c) and the `display.c` backend, exactly as PORTING-GUIDE §3 / HANDOFF
decision #5 prescribe. Because `screen_buffer.c` routes all output through
`ttyprint(y,x,text)`, and lib.c's `ttyprint` now loops `fb_render_cell(y,x)` over
the run, this file renders to the framebuffer with **zero local change**.

## Dependencies / how includes resolve
- `screen_buffer.h` `#include "config.h"` → `src/config.h` (Wave 1, present). It
  uses `SCRN_DEBUG` from there. Resolves.
- `screen_buffer.c` `#include "test.h"` (→ `src/test.h`, Wave 1) and
  `"screen_buffer.h"`. `test.h:140` declares `void ttyprint(int y, int x, const char *s);`
  so the `ttyprint` call type-checks; the definition lands in lib.c. Resolves.
- `config.h` already references this file (a comment at `config.h:60` documents
  that `SCRN_DEBUG` gates `screen_buffer.c`). Consistent.

## Features surfaced or shelved
Nothing user-facing here. `SCRN_DEBUG` kept off (upstream default).

## Important behavioral interaction — char-only dedup (NOTED, not "fixed")
`tty_print_line` (and thus the whole `cprint → tty_print_line → ttyprint` path)
dedups on the **char shadow only** (`screen_buffer.c:97`: `if (*text != screen_buf[y][x]) break;`).
**Attribute-only changes do not flow through this path** — if a caller pokes a new
attribute byte into `vga_buf` while leaving the char identical, `tty_print_line`
sees no char difference, returns early, and `ttyprint`/`fb_render_cell` is never
invoked, so the framebuffer is not repainted.

This **matches upstream exactly** (on x86 it didn't matter — VGA text hardware
re-paints from the attr byte automatically; our shadow buffer carries no attrs and
the framebuffer is not self-repainting). Per HANDOFF "Substrate contract", this is
**handled elsewhere, by design**: init.c (green title bar / reverse footer) and
error.c (`0x47` red error rows) poke attrs directly into `vga_buf` and must call
`fb_render_cell(y,x)` (or `display_refresh()`) themselves after doing so. I made
**no change here** to address it — `screen_buffer.c` is correct as-is and faithful
to upstream; the responsibility correctly lives with the attr-poking call sites.

## Open questions / TO VERIFY
- None for this file. (Cross-compiler not available on this box, so no local
  `-fsyntax-only`; the lead builds at the Wave-2 checkpoint. The bodies are
  verbatim v2.00 and all includes resolve against Wave-1 headers, so no
  compile surprises are expected.)

## Build impact
- New objects: `src/screen_buffer.o`. Add `screen_buffer` to the Makefile `OBJS`
  for Wave 2.
- New call sites: none introduced here. `clear_screen_buf`, `tty_print_line`,
  `tty_print_region`, `tty_print_screen`, `get/set_scrn_buf` are consumed by
  lib.c (`cprint`/`scroll`/`clear_screen`) and later init.c/error.c — those waves
  already expect them.
- Depends on: `config.h`, `test.h` (Wave 1, present), and `ttyprint` from lib.c
  (Wave 2, in parallel). No Makefile change beyond the OBJS entry.

## HANDOFF file-status table — proposed row update
`| screen_buffer.c/.h | ✅ | report port-screen_buffer.md. Imported VERBATIM (bodies byte-identical to v2.00); zero x86 code, no changes. Routes all output through ttyprint→fb_render_cell. Char-only dedup means attr-only changes don't auto-render — handled by init.c/error.c per Substrate contract. |`
