# Examine report — `bootsect.S` + `setup.S` (Wave 6)

## File & purpose

Both files are the **x86 16-bit real-mode boot/loader chain** that gets
memtest86+ off a floppy/disk and into 32-bit protected mode on a PC. On
PowerPC there is no real-mode boot sector at all — Open Firmware `load`s our
ELF and `src/head.S` is the entry stub — so this whole chain is replaced
wholesale.

### `bootsect.S` (375 lines)
Linus Torvalds' original Linux boot sector (1991/92), adapted by Chris Brady
(1-Jan-96) as memtest's loader (header comment, lines 1–15). It is the classic
512-byte BIOS-loaded sector:

- `.code16` / `.section ".bootsect"` — 16-bit real mode (lines 21–22).
- `_main` (lines 28–39): BIOS loads it at `0x7c00`; it copies itself
  (`rep movsw`, 256 words) to `0x90000` (`BOOTSEG`→`INITSEG`) and far-jumps
  there — the self-move described in the header.
- `go` (lines 41–97): sets up `ds=es=ss=cs=INITSEG`, a real-mode stack
  (`%sp`), patches the BIOS floppy parameter table to allow 18-sector reads,
  and resets the FDC via **`int $0x13`** (line 97).
- `load_setup` / `ok_load_setup` (lines 102–147): reads the setup sectors and
  the "system" (the memtest image at `TSTLOAD`=`0x10000`) off the disk with
  **BIOS `int $0x13`** disk reads, probing sectors/track (18/15/9 guess,
  lines 127–143).
- Helpers all use **BIOS `int $0x10`** (teletype/video) for "Loading..."
  messages, plus `read_it`/`read_track`/`print_all`/`print_hex`/`print_nl`
  (lines 186–341) and `kill_motor` which does a raw port-out to the floppy
  controller (`outb %al, %dx` with `%dx=0x3f2`, lines 349–355).
- After everything is loaded it far-jumps to `SETUPSEG:0` (line 174) → enters
  `setup.S`.
- The tail is the boot-sector signature layout: `setup_sects` at offset 497,
  `syssize` at 500, `root_dev` at 508, and `boot_flag` = `0xAA55` (lines
  364–374) — the BIOS bootable-sector magic.

### `setup.S` (115 lines)
The second-stage real-mode setup (header lines 1–7), also `.code16`
(`.section ".setup"`, lines 12–13):

- `start` (lines 15–58): masks interrupts (`cli`), disables NMI via port
  `0x70` (lines 20–22), loads a null IDT and a real GDT (`lidt`/`lgdt`, lines
  29–30), **enables the A20 gate** through the 8042 keyboard controller
  (`empty_8042`, command-write `0xD1`→port `0x64`, `0xDF`→port `0x60`, lines
  33–41), then flips the **PE bit with `lmsw $0x0001`** to enter protected
  mode (lines 47–48) and far-jumps to the memtest entry at
  `KERNEL_CS:(TSTLOAD<<4)` (line 58).
- `empty_8042` / `delay` (lines 67–85): 8042 status-port polling via
  `inb $0x64`/`inb $0x60`, with an I/O `jmp $+2` delay.
- `gdt` / `idt_48` / `gdt_48` (lines 87–108): the flat 128 MB code/data GDT
  entries and the IDT/GDT descriptors that `setup.S` installs.

## Disposition — ⛔ **N/A** (not imported into `src/`)

Every line is PC/BIOS/real-mode by construction:

1. **BIOS services** (`int $0x13` disk, `int $0x10` video) do not exist on
   PowerPC / Open Firmware. There is no BIOS.
2. **x86 protected-mode bring-up** (A20 gate, 8042 keyboard controller, GDT/IDT,
   `lmsw`/PE bit, segment registers) is meaningless on PPC — flat 32-bit
   address space, MMU already on under OF (Key invariant #2).
3. **The boot model is fundamentally different.** memtest86+ is loaded off a
   disk by a BIOS-executed 512-byte sector that then pulls in `setup.S` and the
   image and switches CPU mode. On memtestppc+, **Open Firmware loads the ELF**
   (claims + maps it at the `linker.ld` address `0x00400000`), then jumps to our
   ELF entry — no boot sector, no setup stage, no mode switch.

There is no "comment the leaf, keep the skeleton" path here: the skeleton *is*
the x86 boot protocol.

## What replaces it
- **`src/head.S`** — the PPC entry stub. OF passes the client-interface pointer
  in **r5** (Key invariant #3); head.S sets up the stack in BSS and calls
  `main()`. This is the single, tiny replacement for the entire
  `bootsect.S` + `setup.S` chain.
- **Open Firmware `load`** does what `bootsect.S`'s `int $0x13` reads did —
  brings the image into RAM at the link address and maps it (Key invariant #2).
- **The boot wrapper** lives in `cd/ofboot.b` (CHRP boot script) + the ISO
  layout, not an x86 boot sector.
- No A20/GDT/PE-bit equivalent is needed; PPC has no such mode gating.

## Confirmed: no live callers in `src/`
`grep -rn -i -e 'bootsect' -e 'setup\.S' src/` finds only **descriptive
comments**, never code:
- `src/defs.h:11` — notes these constants once fed
  "the real-mode assembler (bootsect.S/setup.S/head.S)".
- `src/memsize.c:43,51` — notes "their x86 producers (setup.S / linuxbios.c)
  are not ported" and that no `mem_info` symbol (which `setup.S` would have
  populated) exists; `memsize.c` uses OF instead.

These are exactly the conscious-classification breadcrumbs we want; nothing
includes, assembles, or jumps to either file.

## Build impact
None. Neither file is assembled into the PPC build, listed in the Makefile, or
copied into `src/`. They are pure x86 real-mode assembly and would not assemble
under `powerpc-linux-gnu-gcc`.

## Open questions / TO VERIFY
None. Clean N/A: this is the PC boot/real-mode→protected-mode bring-up, fully
subsumed by `head.S` + Open Firmware loading.
