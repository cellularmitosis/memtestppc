# Examine report — x86 linker scripts + `mt86+_loader` (Wave 6)

Covers the three upstream GNU linker scripts (`memtest.lds`,
`memtest.bin.lds`, `memtest_shared.lds`) and the DOS loader pair
(`mt86+_loader`, `mt86+_loader.asm`).

## Files & purpose

### `memtest.lds` (12 lines)
Trivial i386 link script: `OUTPUT_FORMAT("elf32-i386")`, `OUTPUT_ARCH(i386)`,
`ENTRY(_start)` (lines 1–4). Places `.data` starting at `. = 0x10000` with
`_start` at that origin (lines 5–11). This is the `TSTLOAD`=`0x10000` load
address the boot chain (`bootsect.S`/`setup.S`) targets.

### `memtest.bin.lds` (15 lines)
`OUTPUT_FORMAT("binary")`, `OUTPUT_ARCH("i386")`, `ENTRY(_main)` (lines 1–4).
Lays out the **flat raw boot image** from origin 0: `.bootsect` then `.setup`
then `.memtest` (lines 5–13), and computes `_syssize = (_end - _start + 15) >> 4`
(line 14) — the paragraph count `bootsect.S` reads from offset 500
(`syssize:` `.word _syssize`, bootsect.S:368–369). This is what stitches the
boot sector, setup, and the memtest body into the single disk image.

### `memtest_shared.lds` (52 lines)
`OUTPUT_FORMAT("elf32-i386")`, `OUTPUT_ARCH(i386)`, `ENTRY(startup_32)`
(lines 1–4). The **PIC/shared link layout** for memtest's self-relocation: it
emits `.dynsym`/`.dynstr`/`.hash`/`.dynamic` (lines 18–21), all the i386 REL
sections `.rel.text`/`.rel.rodata`/`.rel.data`/`.rel.got`/`.rel.plt`
(lines 23–27), a `.got`/`.got.plt` (lines 35–39), and 256-byte-aligned `_end`
(lines 41–50). This is precisely the dynamic-relocation machinery that
**`reloc.c`** consumes at runtime (see the parked `examine-reloc_c.md`): the
script produces the `.dynamic`/REL/GOT that `_dl_start` walks.

### `mt86+_loader.asm` (231 lines) + `mt86+_loader` (binary)
A **DOS `.EXE` loader**, "by Eric Auer 2003" (header lines 1–13). It is NASM
source that hand-builds an MZ header (`db "MZ"`, lines 26–44) and a real-mode
stub that: prints via `int 10h`, **checks the CPU is ≥386** by toggling EFLAGS
bits (lines 58–76), checks the system is **still in real mode** via
`smsw`/PE-bit test (lines 84–95, else error), then **patches** the embedded
memtest image — it reads the setup-sector count at boot-sector offset `1f1h`
(=497, line 132), recomputes the linear address of `head.S`, and rewrites the
hardcoded protected-mode entry jump (offset `239h`) and the GDT linear offset
(lines 131–172) before `jmp far` into `setup.S` (line 200). It exists so a
`memtest.bin`-style image can be concatenated behind this stub
(`cat memteste.bin memtest.bin > memtest.exe`, lines 8–13) and run from DOS.

`mt86+_loader` (no extension) is **the assembled binary** of that source —
`file` reports `MS-DOS executable`, the bytes begin with `4d5a` (`"MZ"`) and
contain the literal strings `"...386 CPU to use Memtest86+"`,
`"...already is in protected mode"`, etc. from the `.asm`. As instructed, not
dumped beyond confirming it is the DOS `.EXE` artifact of `mt86+_loader.asm`.

## Disposition — ⛔ **N/A** for all five (not imported into `src/`)

- **The three `.lds`** are x86 link layouts (`elf32-i386` / `binary` /
  `OUTPUT_ARCH(i386)`). They are replaced by **`src/linker.ld`**, which is the
  PPC analogue: `OUTPUT_FORMAT("elf32-powerpc")`, `OUTPUT_ARCH(powerpc)`,
  `ENTRY(_start)`, single load at `. = 0x00400000` (Open Firmware claims/maps
  it). Brief comparison:
  - vs `memtest.lds`: same idea (one origin, `_start` there) but PPC ELF at
    `0x00400000` rather than i386 ELF at `0x10000`.
  - vs `memtest.bin.lds`: no flat boot image is built — there is no
    `.bootsect`/`.setup` to concatenate and no `_syssize`; OF loads a normal
    ELF. N/A.
  - vs `memtest_shared.lds`: **most strongly N/A** — we build **non-PIC**
    (`-fno-pic -fno-pie -mno-toc`), so there is no `.dynamic`/`.rel.*`/`.got`
    self-relocation layout. `src/linker.ld` instead `/DISCARD/`s `.comment`
    and `.note.*` and keeps plain `.text`/`.data`/`.bss`. This is the link-time
    counterpart to the parked `reloc.c`.
- **`mt86+_loader` / `mt86+_loader.asm`** are a 16-bit **DOS real-mode** loader
  (MZ `.EXE`, `int 10h`/`int 21h`, 386 detection, real-mode check, x86
  GDT/jump patching). None of this exists on PPC/OF: there is no DOS, no MZ
  format, no real mode, no x86 GDT to patch. Open Firmware `load` + `head.S`
  do the loading/entry. Pure x86; would not assemble or run on PPC.

## Confirmed: no live callers in `src/`
`grep -rn -i -e 'memtest\.lds' -e 'memtest\.bin\.lds' -e 'memtest_shared'
-e 'mt86'` over `src/` returns **nothing**. The build uses `src/linker.ld`
only; nothing references the x86 scripts or the DOS loader.

## Build impact
None. The PPC build links with `src/linker.ld` (already present and in use).
The three x86 `.lds` and the loader pair are not in the Makefile and not copied
into `src/`.

## Open questions / TO VERIFY
None. All five are x86/DOS build- and load-time machinery, cleanly replaced by
`src/linker.ld` + Open Firmware + `src/head.S`. `memtest_shared.lds`
specifically pairs with the already-parked `reloc.c`.
