# Port report — `config.c` (Wave 6)

## File & purpose
`ref/memtest86+-2.00/config.c` (603 lines) is the **runtime config menu** + the
popup-window primitives + `adj_mem()`. `get_config()` is the interactive
(c)configuration overlay the footer advertises; it offers Test Selection,
Address Range, Memory Sizing, Error Report Mode, DMI Info, ECC Mode, Restart,
Refresh Screen, and SPD Data. The popups (`popup`/`popclear`/`popdown`/`pop2up`/
`pop2down`/`pop2clear`) save/restore screen regions; `clear_screen()` blanks the
screen; `adj_mem()` recomputes `v->selected_pages` from the test's address-range
limits against `v->pmap[]`.

## Disposition — **ported (adapted)**, per HANDOFF **Decision #6**.
Surface the platform-neutral options, comment the PC-only ones. The popups,
`clear_screen`, and `adj_mem` are pure neutral C (they write into `vga_buf` =
`SCREEN_ADR` and call `tty_print_region`, which the PPC framebuffer backend
renders) and are imported verbatim. `get_config()` keeps its structure with the
PC-only branches commented.

### Surfaced (neutral, kept)
- **(1) Test Selection** — Default / Skip / Select Test [0-9] / Select Bit Fade.
  Pure `v->testsel`/`find_ticks()` logic.
- **(2) Address Range** — Set Lower/Upper Limit / Test All Memory. Uses
  `getval()` + `adj_mem()` + `find_ticks()`; all neutral.
- **(4) Error Report Mode** — Error Summary / Individual Errors / BadRAM Patterns
  / Error Counts Only. (The DMI Device Name + Beep options within it are
  commented — see below.)
- **(7) Restart**, **(8) Refresh Screen** (`tty_print_screen`), **(0) Continue**.

### Commented (PC-only, ⛔ N/A — Decision #6)
- **(3) Memory Sizing** — selects the BIOS E820/E801 map vs a destructive probe
  (`e820_nr`/`memsz_mode`/`restart`). PPC has one source of truth (OF `/memory`,
  see `memsize.c`). Whole `case` `#if 0`'d; the `extern e820_nr/memsz_mode`
  commented (also: `memsize.c` defines `memsz_mode` as `short`, not config.c's
  `char` — a latent type mismatch the comment-out avoids).
- **(5) Show DMI Memory Info** — `print_dmi_info()` → `dmi.c` (SMBIOS, ⛔).
- **(6) ECC Mode** — `set_ecc_polling()` → `controller.c` (chipset ECC, ⛔).
- **(9) Display SPD Data** — `show_spd()` → `spd.c` (SMBus SPD, ⛔).
- Within Error Report Mode: **(5) DMI Device Name** (`PRINTMODE_DMI`, needs
  SMBIOS slot names) and **(6) Beep on Error** (PC speaker) — menu lines + sub-
  cases commented.
- `#include "controller.h"` / `#include "dmi.h"` commented (modules not built).

## The interactive-input substrate (the real enabler)
The menu reads keys via `get_key()`/`getval()`/`wait_keyup()`, which were x86
PS/2 (`#if 0`'d in `lib.c`). To make the surfaced menu actually usable on PPC, I
wired them to the **OF stdin console** in `lib.c` (not config.c):
- **`get_key()`** — rewritten: read one byte from the OF stdin ihandle
  (non-blocking), map ASCII→PC keycode via `ascii_to_keycode()`/`ser_map[]`
  (both re-enabled — they are pure data + a lookup, exactly what turns OF's ASCII
  console into the keycodes the menu switches on), return 0 when no key. The x86
  PS/2 original is preserved `#if 0`. `get_config()`/`getval()` busy-poll, so a 0
  just means "spin".
- **`getval()`** — enabled verbatim (keycode→char, base detection, p/g/m/k/x
  suffix math — all neutral).
- **`wait_keyup()`** — rewritten to a **no-op**: the OF console (like the x86
  serial console upstream already short-circuited) delivers key-DOWN bytes only;
  the x86 version busy-waited for the 0x80 key-release scancode, which would
  loop forever on OF. x86 original preserved `#if 0`.
- **`check_input()`** — wired the footer keys: `'c'/'C'` → `get_config()`,
  `' '`/CR → scroll-lock toggle + `footer()`, `^L` → `tty_print_screen()`. (ESC
  → reboot was already there.) This makes the **(c)configuration footer key
  real** — closes the HANDOFF "footer honesty" open question.

## adj_mem() — replaces the Wave-5 stub
`adj_mem()` (verbatim) recomputes `v->selected_pages` by intersecting each
`v->pmap[]` segment with `[v->plim_lower, v->plim_upper]`. It **replaces the
main.c no-op stub**. With the default full range it yields `selected_pages ==
test_pages` (identical to the stub), and now an Address-Range selection actually
narrows the tested span.

## Build impact
- `src/config.o` added to `OBJS` + build rule. `src/patn.o` too (see
  `port-patn_c.md`). `adj_mem`/`insertaddress` stubs removed from `main.c`.
- Compiles + links clean (config.o/patn.o; only the known pre-existing `test.c`
  warnings remain). 13 objects now.
- Live callers resolved: `adj_mem` (memsize.c), `get_config` (check_input),
  `get_key`/`getval`/`wait_keyup` (config.c), `ascii_to_keycode`/`ser_map`
  (get_key).

## Open questions / TO VERIFY
1. **Interactive menu navigation is only lightly verified.** Build + the
   no-input test loop (regression: tests still cycle, `Errors: 0`) are confirmed
   in QEMU. Whether the popup renders and keys navigate was probed via QEMU
   `sendkey` — see HANDOFF progress log for the result. Full keyboard-driven
   navigation (esp. `getval()` numeric entry for Address Range) is best verified
   on **real hardware** (ADB/USB keyboard via OF), where the OF console input is
   the real thing rather than OpenBIOS emulation.
2. **OF stdin read semantics.** `get_key()` assumes `ofw_read` returns 0 when no
   byte is pending (non-blocking) and 1 byte otherwise — true for the OF console
   used by `check_input`. If a particular OF returns >1 or blocks, the menu
   busy-poll still works but cadence differs. Confirm on hardware.
3. **`ser_map` ambiguity:** a few ASCII codes map to multiple keycodes
   (`ascii_to_keycode` returns the first match; special cases are ordered first
   in `ser_map`). Digits/Enter/Space/Esc — all the menu needs — resolve
   correctly. Verbatim upstream behavior.
