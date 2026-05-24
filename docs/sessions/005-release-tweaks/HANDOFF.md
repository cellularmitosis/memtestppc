# Session 005: Pre-release TUI tweaks + first real CD release

## Plan for this session

User's stated arc: make a few small TUI tweaks to the *existing* `src/` tree,
verify in QEMU, then cut the **first real release CD** (build ISO → burn →
test on real hardware). The true memtest86+ port restructuring is explicitly
deferred to **session 006**. So this session stays inside the current
(non-fidelity) codebase.

## Tweaks (all done + QEMU-verified)

Screenshot: [`qemu-tweaks-verified.png`](qemu-tweaks-verified.png) — QEMU mac99,
256MB, mid-run (Test #07 Block move, Pass 17%, 0 errors).

### 1. Progress-bar layout — `Test 10% ###` instead of `Test   21####`

Our `do_tick()` printed the percent at `COL_MID+5` (which clobbered the static
`%` glyph at `COL_MID+7`) and drew bars starting at `COL_MID+8`. Upstream
(`ref/memtest86+-5.01/error.c:502-526`) prints pct at `COL_MID+4` (width 3) and
bars at `COL_MID+9`, leaving `COL_MID+7`=`%` and `COL_MID+8`=space. We were off
by one on both. Fix in `src/main.c` `do_tick()`:
- test bar: `dprint(2, COL_MID+4, ...)`, bars `cprint(2, COL_MID+9+i, ...)`
- pass bar: `dprint(1, COL_MID+4, ...)`, bars `cprint(1, COL_MID+9+i, ...)`
- the two "clear progress bar" `cprint`s in the main loop moved `COL_MID+8` →
  `COL_MID+9` to match.

The static labels (`"Pass   %"` / `"Test   %"`) already put `%` at `COL_MID+7`,
so they were left unchanged.

### 2. Split memory readout into two left-column lines

Before: one line, `Memory  :` showed the *tested* amount (`v->test_pages`),
which was misleading. Now:
```
Memory  : 256M   <- total installed RAM (total_mem_mb, converted MB->pages)
Testing : 236M   <- amount we actually claimed/test (v->test_pages)
```
In `src/main.c` `draw_screen()`: added `cprint(5, 0, "Testing :         ")` and
replaced the single `aprint(4,10,test_pages)` with
`aprint(4, 9, total_mem_mb << 8)` (installed) + `aprint(5, 9, v->test_pages)`
(tested). The vertical-separator loop already covers rows 0–5, so row 5 needed
no extra separator. `total_mem_mb` is set by `discover_memory()`, which runs
before `draw_screen()`.

Note: the *right* side still has the upstream `Testing: <start> - <end>`
address-range field on line 4 (col 30) — e.g. `Testing: 8192K - 252M`. That is
a different field (address range, not amount) and was left as-is, but it does
mean the word "Testing" now appears twice on screen. Flag for the user; easy to
rename the left one (e.g. "Tested :") if they object.

### 3. Footer reduced to `<ESC> reboot`

`src/display.c` `footer()` was the full memtest86+ string
(`(ESC)reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock`) — but we
have no config menu / scroll-lock, so the extra hints were misleading. Now just
`<ESC> reboot`. Row 24 keeps its full-width reverse-video attribute (set in
`draw_screen`); the trailing cells stay reverse-video blanks.

## Files changed (relative to session 004)
- `src/main.c` — `do_tick()` bar/pct offsets; `draw_screen()` two memory lines.
- `src/display.c` — `footer()` text.

## Build & verify done
- rsync → opti7050, `make clean && make memtestppc.iso` — OK (only the known
  pre-existing `movinv32` `k`-may-be-uninitialized warning at `src/test.c:324`).
- QEMU `-M mac99 -m 256 -cdrom memtestppc.iso -boot d`, VNC :99, monitor
  `screendump` → all three tweaks confirmed rendering correctly. ISO lives at
  `opti7050:/home/cell/memtestppc+/memtestppc.iso`.

## Fake-error display check (found a fidelity bug)

User asked to re-inject fake errors (removed before the 004 build) and screenshot
the white-on-red error rows. Temporary injection added in `main.c` just before
the test loop (6 `error()` calls, marked "TEMPORARY (session 005) ... Remove
before the release build"). Screenshot: [`qemu-fake-errors.png`](qemu-fake-errors.png).

**Finding (now fixed):** the white-on-red text worked, but the red was
**segmented** — only behind the printed text in each column, with blue gaps
between columns ([`qemu-fake-errors.png`](qemu-fake-errors.png)). Upstream
(`ref/memtest86+-5.01/error.c:126-135`) paints the row red by writing attr `0x47`
to cols 1–76 of the memory-mapped VGA buffer, so the whole row turns red
instantly. Our `vga_buf` is a *shadow* buffer: `error()` set the `0x47` attr
across the row but only re-rendered the cells it wrote text into, leaving the gap
cells blue.

**Fix applied:** in `src/main.c` `error()`, after `scroll()`, loop cols 1..76
setting attr `0x47` *and* calling `display_render_cell()` on each, then print the
fields. Now the row is solid red edge-to-edge, matching memtest86+. Verified in
QEMU: [`qemu-fake-errors-fixed.png`](qemu-fake-errors-fixed.png).

**Injection removed:** the temporary 6-error block is gone from `main.c`. Clean
release ISO rebuilt (`make clean && make memtestppc.iso`) — only the known
pre-existing warnings (`movinv32` `k`, linker RWX/GNU-stack). Clean build
re-verified in QEMU showing `Errors: 0`, no red rows:
[`qemu-release-clean.png`](qemu-release-clean.png).

## Files changed (relative to session 004) — updated
- `src/main.c` — `do_tick()` bar/pct offsets; `draw_screen()` two memory lines;
  `error()` full-row red re-render.
- `src/display.c` — `footer()` text.

## Current state

All four TUI changes landed and QEMU-verified (3 tweaks + error-row red fix);
fake-error injection removed; clean release ISO builds. NOT yet done: commit,
real-hardware re-test on ibookg32 (incl. the now-correct error rows on the real
8-bit display), version bump decision, and the release-CD burn. Changes are
display-layer only — no test logic touched.

ISO: `opti7050:/home/cell/memtestppc+/memtestppc.iso` (clean, no fake errors).

## What to do next

1. **Decide the release version.** Title bar + `VERSION` still say `0.01`. A
   "first real release" probably wants a bump (roadmap is 0.01→1.00). Ask the
   user; change `VERSION` in `src/main.c:14` and the title string in
   `draw_screen()` (`"   Memtestppc+ v0.01      "`) together.
2. **Commit** the session-005 tweaks.
3. **Burn the ISO + boot it on real hardware.** This is the headline goal — no
   physical CD has ever been burned (see CLAUDE.md invariant #6). opti7050 has
   the ISO; needs a burner + blank media. ibookg32 has a slot-loading optical
   drive. Also worth re-confirming the tweaks on ibookg32's real 8-bit display
   (esp. the green title bar / blue bg still look right).
4. **Then** (session 006) pivot to the true memtest86+ v5.01 port restructuring.

## Open items carried from session 004 (still true)
- `movinv32` uninitialized-`k` warning (`src/test.c:324`).
- Bit-fade test `sleep()` stubbed (test 11).
- Error display (white-on-red rows) never visually verified on hardware.
- `color!` BGR palette order is empirical (only affects 8-bit real hardware).
