# Session 004: Real Hardware Testing on iBook G3

## What happened

First successful run of memtestppc+ on real PowerPC hardware — an iBook G3 (PowerBook4,1), 500 MHz PowerPC 750, 256MB RAM, Mac OS X 10.4 Tiger, booted via Open Firmware from a second HFS+ partition.

### Bugs fixed for real hardware

1. **Stack placement crash (code=900 at 0x400018)** — `head.S` hardcoded the stack at `0x003F0000` (below our 4MB load point). Real Apple OF has the MMU on and hadn't mapped that address. Fix: allocate a 16KB stack buffer in BSS (which OF mapped when loading the ELF) and point SP there.

2. **Memory test crash (code=900 at 0x4024e4)** — Test routines wrote directly to physical addresses (e.g., `0x800000+`), but with OF's MMU active those addresses aren't mapped. Fix: call `ofw_claim()` for each memory region before testing it. Initial attempt to claim the entire range at once only got ~120MB; switched to claiming in 1MB chunks, which yields ~244MB (256MB minus OF/framebuffer/code regions).

3. **8-bit framebuffer palette (cyan instead of blue/black)** — Apple OF's `color!` method takes arguments in `( index b g r -- )` order (BGR, not RGB), and the index goes on the Forth stack *first* (bottom), not last. Also, `set-colors` bulk method didn't work reliably. Fix: use `color!` only, with corrected argument order `args[2]=index, args[3]=b, args[4]=g, args[5]=r`.

4. **Progress bars stuck at 100%** — `find_ticks_for_test()` tick estimates from memtest86+ assumed ~1 tick per memory traversal, but `do_tick()` is called once per `SPINSZ` (16MB) chunk. With 244MB of testable memory, actual tick count was ~15x the estimate. Fix: scale all tick estimates by `selected_pages / (SPINSZ/1024)`.

5. **ESC key behavior** — Changed from `ofw_exit()` (drops to OF prompt) to `ofw_reset()` via OF `interpret "reset-all"`, which reboots the machine. Falls back to `ofw_exit()` if interpret fails.

6. **Error display** — Fixed column alignment to match header (Address/Expected/Actual/Xor/Tst/Pass). Set error rows to white-on-red background (`0x47` attribute), matching memtest86+ v5.01 convention. (Note: this was not visually verified on hardware yet — the fake error injection was removed before this build.)

### Boot method: partition boot via Open Firmware

ibookg32 has two HFS+ partitions:
- Partition 3 (`disk0s3`, "Untitled 1", 20GB) — Tiger boot volume
- Partition 5 (`disk0s5`, "Untitled 2", 36GB) — test partition

The ELF is copied to the second partition via SCP. To boot:
1. Reboot, hold **Cmd+Option+O+F** for OF prompt
2. Type: `boot hd:5,memtestppc.elf`
3. Tiger boots normally on next reboot (NVRAM `boot-device` is untouched)

### What was NOT fixed

- The `color!` argument order discovery (BGR, index-first) is empirical. It may differ on other Mac OF implementations. The 32-bit framebuffer path (used by QEMU) doesn't use the palette at all, so this only matters for real 8-bit hardware.

## Files changed (relative to session 003)

- `src/head.S` — Stack in BSS instead of hardcoded address; 16KB `_stack_bottom`/`_stack_top` in `.bss` section.
- `src/main.c` — 1MB-chunked `ofw_claim()` in `discover_memory()`; tick estimates scaled by chunk count in `find_ticks_for_test()`; error display with aligned columns and `0x47` attribute; ESC triggers `ofw_reset()`.
- `src/display.c` — `fb_setup_palette()` rewritten: BGR argument order for `color!`, index as first stack arg; screen fill after palette setup; footer says "(ESC)reboot".
- `src/ofw.c` — Added `ofw_reset()` using `interpret "reset-all"`.
- `src/ofw.h` — Added `ofw_reset()` declaration.

## Build & test environment

- **Cross-compiler**: opti7050 (`ssh opti7050`), Ubuntu, `powerpc-linux-gnu-gcc` 12.2.0
- **Real hardware**: ibookg32 (`ssh ibookg32`), iBook G3 PowerBook4,1, 500MHz PPC 750, 256MB, Mac OS X 10.4.11, 8-bit framebuffer (ATI Rage Mobility)
- **QEMU**: opti7050, `qemu-system-ppc` 7.0.0, mac99, 32-bit framebuffer
- **opti7050 cannot SCP directly to ibookg32** (needs legacy SSH ciphers). Route files through the local Mac: `scp opti7050:file /tmp/ && scp /tmp/file ibookg32:dest`

### Build and deploy commands
```bash
# Sync source to build machine:
rsync -av --exclude '.git' --exclude 'ref/' --exclude '*.o' --exclude '*.elf' \
    --exclude '*.bin' --exclude '*.iso' --exclude '*.png' --exclude '*.ppm' \
    --exclude 'docs/' --exclude 'tmp/' \
    /Users/cell/claude/memtestppc+/ opti7050:/home/cell/memtestppc+/

# Build on opti7050:
ssh opti7050 'cd /home/cell/memtestppc+ && make clean && make memtestppc.iso'

# Deploy ELF to ibookg32 (via local hop):
scp opti7050:/home/cell/memtestppc+/memtestppc.elf /tmp/memtestppc.elf
scp /tmp/memtestppc.elf ibookg32:"/Volumes/Untitled 2/memtestppc.elf"
```

## Current state

The TUI boots and runs correctly on real iBook G3 hardware with proper colors (blue background, green title bar, black border, white text). 244MB of 256MB is tested. Progress bars track accurately. ESC reboots. Zero real errors detected.

Bootable ISO (`memtestppc.iso`, 416KB) built on opti7050 and copied to `/tmp/memtestppc.iso` locally.

Error display with white-on-red rows has NOT been visually verified yet (fake error injection was removed).

## What to do next

1. **Pivot to true port** — Restructure the codebase to preserve more of the original memtest86+ v5.01 code structure. Keep function skeletons the same; replace x86-specific leaf code with OF/PPC implementations. The goal is to make the diff against upstream as small as possible, making it easier to pull in upstream fixes and for people familiar with memtest86+ to navigate the code. Discuss details at session start.

2. **Verify error display** — Re-inject fake errors briefly to confirm the white-on-red treatment looks correct on the iBook's 8-bit display.

3. **Test CD boot on real hardware** — Burn `memtestppc.iso` on opti7050, test on ibookg32's optical drive (if it has one — it's an iBook, so yes it has a slot-loading CD/DVD drive).

4. **Memory map refinement** — Currently skips first 8MB unconditionally. Could probe OF's `/memory/available` property to skip only what's actually in use, reclaiming more testable memory.

5. **Test on other machines** — imacg3 (when back online), other PPC Macs with different GPU (32-bit framebuffer path), G4/G5 machines.

6. **Known issues**:
   - `movinv32` warning: `k` may be used uninitialized (`src/test.c:324`)
   - Bit fade test `sleep()` is stubbed out (test 11)
   - `color!` BGR order is empirical — may need per-machine detection or a different approach for non-ATI GPUs
