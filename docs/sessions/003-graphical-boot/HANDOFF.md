# Session 003: Graphical Boot & TUI Polish

## What happened

This session picked up from session 002 where the CD boot worked in `-nographic` mode but failed in QEMU VNC/graphical mode with "No valid state has been set by load or init-program".

### Root cause of VNC boot failure

The VNC boot failure was **not reproducible** with the current code and 256MB RAM. Auto-boot from the CHRP boot script works correctly in VNC mode. The previous failures were likely from earlier testing configurations or transient QEMU state. Confirmed by:
1. Interactive OF prompt: `load cd:,\memtestppc.elf` + `go` works perfectly in VNC mode
2. Auto-boot (no `auto-boot?=false`): boots directly into the TUI from CD

**However**, auto-boot fails with <128MB RAM in QEMU. With 32MB and 64MB, OpenBIOS's `load` command fails (likely an OpenBIOS memory allocation issue). This is a QEMU/OpenBIOS-specific problem — real Apple OF on real Macs won't have this limitation.

### TUI rendering — first successful graphical output

The memtest86+ blue-screen TUI renders correctly in QEMU's ATI Rage 128 Pro framebuffer:
- 80x25 VGA text mode emulated via framebuffer + IBM VGA 8x16 font
- 32-bit color depth (QEMU mac99 provides 32bpp framebuffer)
- Blue background, white text, green title bar, reverse-video footer
- All display functions working: cprint, dprint, hprint, aprint, scroll

### Bugs fixed

1. **Debug text on framebuffer** — `ofw_puts("display initialized\r\n")` was called *after* `display_init()`, painting text via OF's console on top of our cleared framebuffer. Removed all non-essential `ofw_puts` calls (kept only the failure message in `main()` for serial diagnostics).

2. **`dprint()` right-justify zero-padding** — The `right=1` mode (used for time display) had broken loop logic that produced garbage for zero-padded numbers. Rewrote the right-justify branch to correctly zero-pad.

3. **Time display column misalignment** — Initial label `"| Time:   0:00:00"` (3 spaces after colon) placed the time value one column too far right. `update_time()` writes hours at col 67 (3 chars), minutes at col 71 (2 chars), seconds at col 74 (2 chars) — total cols 67-75. But the initial label put the last "0" at col 76, which was never overwritten, making seconds display as e.g. "020" instead of "20". Fixed by removing one space: `"| Time:  0:00:00"`.

4. **Title layout** — `"   Memtestppc  v0.01     "` with blinking "+" at col 14 produced "Memtestppc +v0.01" (space before +, no space before v). Fixed to `"   Memtestppc+ v0.01      "` with "+" at col 13, producing "Memtestppc+ v0.01".

5. **Debug messages in `ofw_get_fb_info()`** — Removed `ofw_puts("screen device found")` and `ofw_puts("screen alias not found")` that painted on screen before display init (harmless since display_init clears the framebuffer, but unnecessary noise on serial).

### Features added

1. **Pass-level progress bar** (line 1) — Added `find_ticks_for_test()` that calculates expected tick count per test pattern (matching original memtest86+ v5.01 logic). `calc_pass_ticks()` sums across all enabled tests. `do_tick()` now updates both test progress (line 2) and pass progress (line 1).

2. **Spinner animation** — `do_tick()` cycles through `|/-\` on the State line (LINE_CPU, col 7).

3. **`find_ticks_for_test()`** — Per-test tick estimator replacing the old hardcoded `c_iter * 2 + 2` formula. Now `test_ticks` is set from this function in the main loop.

## Files changed (relative to session 002)

- `src/main.c` — All bug fixes and features above. Added `pass_ticks`, `total_ticks`, `find_ticks_for_test()`, `calc_pass_ticks()`. Updated `do_tick()`, `draw_screen()`, `main()`.
- `src/display.c` — Fixed `dprint()` right-justify mode.
- `src/ofw.c` — Removed debug `ofw_puts` calls from `ofw_get_fb_info()`.

## Build & test environment

- **Cross-compiler**: opti7050 (`ssh opti7050`), Ubuntu 22.10, `powerpc-linux-gnu-gcc` 12.2.0
- **QEMU testing**: opti7050, `qemu-system-ppc` 7.0.0, mac99 machine, VNC display `:99`
- **opti9020** is also back online (user mentioned it this session, not used yet)
- **imacg3** is currently offline (ssh times out)
- **QEMU monitor interaction**: Use unix socket at `/tmp/qemu-mon.sock` with `socat` to send `screendump` commands. Keyboard input via `sendkey` through the monitor (helper script at `/tmp/qemu_type.sh` on opti7050).

### Build commands
```bash
# On opti7050:
cd /home/cell/memtestppc+
make clean && make memtestppc.iso

# Test in QEMU (128MB+ required for auto-boot):
qemu-system-ppc -M mac99 -m 256 \
    -cdrom memtestppc.iso -boot d \
    -display vnc=:99 \
    -monitor unix:/tmp/qemu-mon.sock,server,nowait &

# Screenshot via monitor:
echo "screendump /tmp/ss.ppm" | socat - UNIX-CONNECT:/tmp/qemu-mon.sock
convert /tmp/ss.ppm /tmp/ss.png
```

### Syncing source to build machine
```bash
rsync -av --exclude '.git' --exclude 'ref/' --exclude '*.o' --exclude '*.elf' \
    --exclude '*.bin' --exclude '*.iso' --exclude '*.png' --exclude '*.ppm' \
    /Users/cell/claude/memtestppc+/ opti7050:/home/cell/memtestppc+/
```

## Current state

The TUI boots and runs in QEMU VNC mode. Screenshot at boot shows:
```
   Memtestppc+ v0.01      | PowerPC 7400 (G4)
CLK:  900 MHz              | Pass   2####...
L1 Cache:  32K             | Test 100#################################
L2 Cache: 256K             | Test #01 [Address test, own address Sequential]
Memory :  248M             | Testing: 8192K -  256M
                           | Pattern:   address        | Time:  0:00:02
------------------------------------------------------------------------------
State: | Running...

Pass:       0                                               Errors:      0
------------------------------------------------------------------------------

 Address       Expected    Actual      Xor        Tst  Pass
------------------------------------------------------------------------------




(ESC)exit  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock
```

## What to do next

1. **Real hardware testing** — When imacg3 comes back online, burn the ISO to CD or use OF's `boot cd:` command. The iMac G3 has a real Rage 128 GPU (8-bit indexed color), real G3 CPU (PVR 0x0008), and real Apple OF — will test all the fallback paths we haven't exercised in QEMU (8-bit palette setup, G3 CPU detection, real timebase frequency).

2. **Test progression verification** — Tests are extremely slow in QEMU (emulated timebase barely advances, memory writes are word-by-word in emulation). On real hardware they'll run at full speed. Could try reducing test memory range for QEMU-only testing.

3. **TUI fidelity improvements** — Compare against real memtest86+ v5.01 screenshots (the `ref/screenshots/` image is v6.00b1, not v5.01). Minor layout items:
   - Pass/test `%` suffix not displayed (just the number)
   - The `v->tptr`/`v->pptr` progress bar tracking from the original isn't used (we recalculate from scratch each tick — works but slightly different visual behavior)
   - No "of XXXM" display on the Testing line

4. **Bit fade test `sleep()`** — Currently stubbed out (test 11). Needs a timebase-based delay loop.

5. **`movinv32` warning** — `src/test.c:324: 'k' may be used uninitialized` — pre-existing, should initialize `k = 0` at declaration.

6. **Memory map refinement** — Currently skips first 8MB unconditionally. Could be smarter about avoiding only OF + framebuffer + our code regions.

7. **ELF size** — Currently ~52KB. Could strip further with `-s` linker flag if needed for CD boot compatibility on older OF implementations.
