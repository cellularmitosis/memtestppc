# Examine report — `reloc.c` (Wave 5)

## File & purpose
`ref/memtest86+-2.00/reloc.c` (263 lines) is memtest86+'s **x86 ELF
self-relocation runtime** — a stripped-down glibc `_dl_start`. It is the code
that lets the image run as a position-independent ELF that relocates itself to
an arbitrary load address at startup. It defines:

- `_dl_start(void)` — the entry the bootstrap calls. It reads the image's own
  `_DYNAMIC` section, builds a `struct link_map`, and applies the dynamic
  relocations against the running image so GOT-based data/function access works
  after a move.
- `elf_machine_rel()` — applies one `Elf32_Rel`, switching on the **x86**
  relocation types `R_386_RELATIVE`, `R_386_NONE`, `R_386_COPY`,
  `R_386_GLOB_DAT`, `R_386_JMP_SLOT`, `R_386_32`, `R_386_PC32`.
- `elf_machine_dynamic()` / `elf_machine_load_address()` — read the GOT and the
  load address via inline asm pinned to **`%ebx`** (`register Elf32_Addr *got
  asm ("%ebx")`, `leal _start@GOTOFF(%%ebx)`).
- `elf_get_dynamic_info()` / `elf_dynamic_do_rel()` — walk the `.dynamic` table
  and iterate the REL section.

It is invoked from `head.S` right before the test loop: `leal
_dl_start@GOTOFF(%ebx); call *%eax; call do_test` (ref head.S:448-450).

## Disposition — 🅿 examined → **parked / N-A** (not imported into `src/`).

Reasons, from reading the file in full:

1. **It is x86 by construction, not just by leaf.** Every relocation type is
   `R_386_*`; the GOT/load-address accessors are hand-written `%ebx` inline asm;
   the whole thing assumes the i386 PIC/GOT calling convention. There is no
   "comment the leaf, keep the skeleton" path — the skeleton *is* the x86 ELF
   ABI. A PPC equivalent would be a from-scratch `R_PPC_*` relocator, which we
   do not need (see #2).

2. **The PPC port does not self-relocate — Key invariant #4.** On x86 the image
   is PIC and `_dl_start` fixes it up so `run_at()` can `memmove` it to a new
   address and keep running (the relocation scheme that also forms the test
   loop). On PPC/Open Firmware:
   - OF's `load` claims and maps the ELF at the **link address**
     (`linker.ld` loads at `0x00400000`); the MMU is already on.
   - `src/head.S` sets up the stack in BSS and **calls `main()` directly** —
     there is no `_dl_start` call, no GOT bootstrap, no PIC requirement.
   - We build **non-PIC** (`-fno-pic -fno-pie -mno-toc` in the Makefile), so
     there are no dynamic relocations to apply at runtime; absolute addressing
     resolves at link time.
   - The Wave-5 `main.c` replaced the `run_at()`/`__run_at()` self-relocation
     with an in-place `for(;;) do_test();` loop (those x86 reloc paths are
     preserved `#if 0` in `main.c` for fidelity). So nothing calls into a
     relocator.

3. **Nothing references `_dl_start` on PPC.** Our `head.S` has no
   `_dl_start@GOTOFF; call` sequence (that was the x86 head.S). With no caller
   and no PIC build, importing `reloc.c` would add dead code that, moreover,
   **would not even compile** under `powerpc-linux-gnu-gcc` (the `%ebx`/`leal`
   asm and `R_386_*` constants are x86-only).

## What replaces it
Nothing needs to. The functions of `reloc.c` are subsumed by the platform:
- **Loading at the right address:** Open Firmware `load` + `linker.ld`.
- **"Relocate to test the RAM we run from":** N/A — we claim RAM in place
  (`memsize.c` / `ofw_claim`) and simply do not test the ~8 MB low region where
  our image, OF, and the framebuffer live (`LOW_TEST_ADR` skip). This is the
  conscious trade recorded in the project brief: a small fixed slice is not
  under test, in exchange for never relocating.

## Build impact
None. `reloc.c` is **not** added to the Makefile `OBJS` and **not** copied into
`src/`. `elf.h` (its only project-header dependency, still ☐ in the file-status
table) is likewise not needed unless a future consumer appears — none does in
Waves 0-5.

## Open questions / TO VERIFY
- None. This is a clean N-A: the file is x86-ABI machinery for a self-relocation
  scheme the PPC port deliberately does not use.
