# memtestppc+

A standalone, bootable RAM tester for PowerPC Macs — a faithful port of the
classic [memtest86+](https://www.memtest.org/) v5.01 blue-screen TUI to Open
Firmware. It boots with **no operating system**, so the maximum fraction of
system memory is under test. Same IBM VGA 8x16 font, same green title bar, same
blinking `+`.

## Status

**Early, pre-release (v0.01).** Boots and runs end-to-end in QEMU (mac99) and on
real iBook G3 hardware: the TUI renders correctly, memory is discovered and
tested, progress bars and timer work. Not yet packaged for download, and a
*physical* CD hasn't been burned yet (testing so far is a QEMU `-cdrom` image and
partition boot on a real iBook). No releases yet.

| Surface | Status | Notes |
|---|---|---|
| Boot from CD image in QEMU (mac99) | ✅ Working | Via a CHRP boot script; needs ≥128 MB RAM under OpenBIOS. |
| Run on real iBook G3 | ✅ Working | Booted from an HFS+ partition via Open Firmware. |
| TUI rendering (VGA font, colors, layout) | ✅ Working | Embedded IBM VGA 8x16 font rendered to the OF framebuffer. |
| 32-bit framebuffer path | ✅ Working | QEMU's mac99 framebuffer. |
| 8-bit framebuffer + palette | ✅ Working | Real iBook G3 (ATI Rage Mobility); Apple OF `color!` palette setup. |
| CPU identification (PVR → G3/G4/G5) | ✅ Working | Plus cache sizes from the OF device tree. |
| Memory discovery + test | ✅ Working | Claims RAM from OF in 1 MB chunks (~244 MB of 256 on the iBook). |
| Error display (white-on-red) | 🟡 Partial | Full-row red verified in QEMU; not yet seen on real 8-bit hardware. |
| Bit-fade test (test 11) | 🟡 Partial | Runs, but the timed fade window is stubbed out (no `sleep()` yet). |
| Physical CD boot | ❌ Not working | Burned CD-R gives a blinking folder on real Apple OF; the CHRP boot script only works under QEMU/OpenBIOS. Needs a Mac-blessed boot setup. |

## Building & running

Requires a `powerpc-linux-gnu` cross toolchain, `qemu-system-ppc`, and
`xorrisofs` (all on a Linux host; Tiger's own gcc can't produce the ELF that
Open Firmware needs).

```sh
make memtestppc.iso

# Run in QEMU (mac99; 128 MB+ is required for OpenBIOS auto-boot):
qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d
```

On a real PowerPC Mac, boot the ISO from Open Firmware (`boot cd:,\memtestppc.elf`)
or copy `memtestppc.elf` to an HFS+ partition and `boot hd:N,memtestppc.elf`.

## Layout

```
src/        — the port: head.S, ofw.{c,h}, display.{c,h}, font_vga.h,
              memtest.h, test.c, main.c, linker.ld.
cd/         — CHRP bootinfo.txt for the bootable ISO.
ref/        — memtest86+ v5.01 reference source + screenshots.
docs/       — project plan and per-session notes (docs/sessions/).
Makefile    — cross-compile, ISO build, QEMU targets.
```

## License

GPL v2, inherited from memtest86+ (Chris Brady and Samuel Demeulemeester). The
PowerPC / Open Firmware port code is GPL v2 to match.
