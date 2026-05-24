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

## First physical CD burned (session 005)

Committed the source state as `134ea1a` ("session 005: pre-release TUI tweaks +
error-row red fix"), then burned the first physical disc — versions stay at
**0.01** per user (no bump), both "Testing" labels kept.

- Media: blank CD-R in opti7050's `/dev/sr0` (not erasable, ~700MB blank).
- **Gotcha:** the ISO is only 208 sectors (416KB), below the CD ~300-sector
  minimum track size. Padded a burn copy to 512 sectors first:
  `cp memtestppc.iso /tmp/burn.iso && truncate -s 1048576 /tmp/burn.iso`
  (trailing zeros are outside the ISO9660/HFS+ filesystem → harmless; the
  filesystem still declares 208 sectors).
- Burn: `sudo wodim dev=/dev/sr0 -v -dao -eject speed=8 /tmp/burn.iso` — wrote
  512 sectors, fixated in ~9s, no errors, disc ejected. (`cell` is in the
  `cdrom` group and has passwordless sudo on opti7050.)

This satisfies CLAUDE.md invariant #6's open to-do (no physical CD had ever been
burned).

### Physical CD does NOT boot on real Apple OF (blinking folder)

Booted the burned disc on ibookg32 → **blinking-folder icon**, i.e. Apple Open
Firmware found nothing bootable. So our CHRP `bootinfo.txt` boot script, which
OpenBIOS (QEMU) honors for `-cdrom` boot, is **not** honored by real Apple OF.
This is a genuine OpenBIOS-vs-Apple-OF divergence, not a bad burn. Physical CD
boot on real Macs needs a different approach (likely an Apple "blessed" tbxi /
BootX-style setup, or a real CHRP boot partition) — scoped as future work, NOT
solved this session. The burn mechanics themselves are fine (image was written
and fixated cleanly).

### Fell back to partition boot (proven path from session 004)

iBook was up in Tiger and reachable, so deployed the fresh session-005 ELF to the
second HFS+ partition, overwriting the older 004 ELF:
`scp opti7050:.../memtestppc.elf /tmp/ && scp /tmp/memtestppc.elf
ibookg32:"/Volumes/Untitled 2/memtestppc.elf"` (opti7050 can't SCP to the iBook
directly — legacy ciphers — so route through uranium). Verified md5 match on the
iBook: `839fe0115107844ebfe67f81279987ae`. Boot from OF with
`boot hd:5,memtestppc.elf`. Awaiting the user's on-hardware result of the
session-005 tweaks (progress bars, two-line memory, footer) and especially the
full-width-red error rows on the real 8-bit display.

## CD-boot rework for real Apple OF (genisoimage recipe)

The first burn's blinking folder sent us to look at prior art in
`../rogerppc/attic/subprojects/` (user pointer). Key references:
`osx-livecd/notes/disc-formats.md` (what makes a PPC disc bootable) and
`brick-recovery/scripts/remaster.sh` (a **proven** New World bootable-CD recipe —
booted finnix recovery CDs on real PowerBooks).

**Diagnosis — three reasons the xorrisofs disc failed on Apple OF:**
1. `xorrisofs -hfsplus -hfs-bless-by p` produces an HFS+ hybrid whose bless Apple
   OF didn't recognize. The proven recipe is genisoimage's *classic* HFS hybrid:
   `genisoimage -hfs -part -hfs-bless <dir> -map <hfs.map>` → DDM + APM +
   Apple_HFS slice + a blessed folder, which is what New World OF actually scans.
2. **No `<COMPATIBLE>` block.** Apple OF matches the CHRP boot script against the
   machine's `compatible` property. finnix's `ofboot.b` lists
   `MacRISC MacRISC3 MacRISC4`; the iBook G3 (PowerBook4,1) is **MacRISC2**, so we
   list `MacRISC MacRISC2 MacRISC3 MacRISC4`.
3. The boot file must be HFS type **`tbxi`** in the blessed folder (set via
   `hfs.map`: `.b → 'tbxi'`), so OF's hold-C / `\\:tbxi` lookup finds it.

**What changed in the repo:**
- New `cd/ofboot.b` — CHRP boot script with `<COMPATIBLE>` + a `load cd:,\memtestppc.elf` / `go` body.
- New `cd/hfs.map` — assigns `.b → tbxi`.
- `Makefile` `memtestppc.iso` target rewritten from xorrisofs to
  `genisoimage -hfs -part -map cd/hfs.map -hfs-bless cd_root/boot ...`, layout
  `/memtestppc.elf` + blessed `/boot/ofboot.b`.
- `cd/bootinfo.txt` (old xorrisofs CHRP script) is now unused — left in place.

**Boot-script verb — OpenBIOS vs Apple OF (a real divergence):**
- First try used finnix's exact `boot cd:,\\memtestppc.elf` (doubled backslash,
  `boot` verb). QEMU/OpenBIOS found the tbxi (`Trying cd:,\\:tbxi...` — **the
  bless works!**) but then `No valid state has been set by load or init-program`
  — OpenBIOS wants `load`+`go`, not `boot`, and dislikes the `\\`.
- Switched the script body to `load cd:,\memtestppc.elf` + `go` (single
  backslash). **QEMU now boots the TUI fully** — see
  [`qemu-genisoimage-boot.png`](qemu-genisoimage-boot.png). This is the
  OpenBIOS-proven form; whether Apple OF prefers `load`+`go` vs `boot` with `\\`
  is the open question the hardware test answers. If the auto-script fails on
  Apple OF, the disc is still readable (DDM+APM+HFS confirmed) so we can drive it
  manually from the OF prompt without re-burning.

**Second burn:** ISO is 591 sectors (no padding needed this time). md5
`cd0c72c892fe659cdb2e8599e2d84239`. Burned with
`sudo wodim dev=/dev/sr0 -v -dao -eject speed=8 memtestppc.iso` — clean, fixated,
ejected. Awaiting iBook test.

## Hardware-test procedure (next — task #8, user-driven)

The disc is in opti7050; ibookg32 is a separate machine, so the user must
physically move the disc to the iBook's slot-loading drive. CD boot has only ever
been done in QEMU/OpenBIOS — **real Apple OF CD boot is unverified**, so this is
an experiment. Best-guess boot path, in order:
1. Insert disc, restart, **hold `C`** at the chime (Apple's boot-from-CD key).
   Our disc is HFS+ with a CHRP bless (type `tbxi`, creator `chrp` on
   `/ppc/chrp/bootinfo.txt`, blessed dir `/ppc/chrp`), so Apple OF may pick it up.
2. If `C` doesn't boot it: **Cmd-Opt-O-F** to the OF prompt, then
   `boot cd:,\\:tbxi` (boot the blessed tbxi file). Fallbacks:
   `boot cd:,\ppc\chrp\bootinfo.txt` or check the device alias with `devalias`.
3. Optional pre-check before walking it over: re-insert in opti7050 and verify
   the burn read-back: `sudo dd if=/dev/sr0 bs=2048 count=208 2>/dev/null | \
   cmp - <(head -c 425984 /home/cell/memtestppc+/memtestppc.iso)` → no output = good.

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
