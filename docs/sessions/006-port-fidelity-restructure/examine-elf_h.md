# Examine report — `elf.h` (Wave 6)

## File & purpose
`ref/memtest86+-2.00/elf.h` (589 lines) is a self-contained **ELF format
header** — the type/constant definitions for parsing ELF objects. From reading
it:

- **Enumerated constants** for ELF header fields: `e_type` (`ET_REL`/`ET_EXEC`/
  `ET_DYN`…, lines 7–11), a long `e_machine` table including both `EM_386`=3
  (line 17) and `EM_PPC`=20 (line 30), program-header `p_type`/`p_flags`
  (lines 95–106), `e_ident` magic (`ELFMAG0..3`, `ELFCLASS32/64`,
  `ELFDATA2LSB/MSB`, lines 111–139).
- **C structs** (guarded by `#ifndef ASSEMBLY`, line 143, which pulls in
  `"stdint.h"`, line 145): `Elf32_*`/`Elf64_*` typedefs (lines 151–167), the
  ELF/program/dynamic/symbol/relocation structures `Elf32_Ehdr`, `Elf32_Phdr`,
  `Elf32_Dyn`, `Elf32_Sym`, `Elf32_Rel`, `Elf32_Rela` (and Elf64 twins).
- **Dynamic-linking + section constants**: the full `DT_*` table (lines
  256–358), `SHN_*`/`SHT_*`/`SHF_*` section values (lines 396–463), and the
  `ELF32_R_SYM`/`ELF32_R_TYPE`/`ELF32_R_INFO` r_info accessors (lines 524–530).
- **The i386 relocation types** `R_386_NONE`, `R_386_32`, `R_386_PC32`,
  `R_386_COPY`, `R_386_GLOB_DAT`, `R_386_JMP_SLOT`, `R_386_RELATIVE`, … through
  `R_386_NUM`=38 (lines 533–585). **No `R_PPC_*` types are defined** — this
  header only knows i386 relocations.

It is a passive header: no functions, no x86 inline asm. Its purpose in
upstream is to give the self-relocator the type names and the **`R_386_*`**
constants it switches on.

## Disposition — 🅿 examined → **parked / N-A** (not imported into `src/`)

Reasons, from reading the file in full and tracing its one consumer:

1. **Its only project consumer is `reloc.c`, which is already parked.** The
   parked `examine-reloc_c.md` records that `reloc.c` is x86-ABI self-relocation
   machinery the PPC port deliberately does not use, and notes: "`elf.h` (its
   only project-header dependency …) is likewise not needed unless a future
   consumer appears — none does in Waves 0-5." That holds through Wave 6 as
   well — see the grep below.

2. **The relocation half is x86-only.** The `R_386_*` block (lines 533–585) and
   the `Elf32_Rel`/`ELF32_R_TYPE` accessors are exactly what `reloc.c`'s
   `elf_machine_rel()` switches on. There are no `R_PPC_*` definitions here, so
   even a hypothetical PPC relocator could not use this header as-is. But we
   don't need one: the PPC image is **non-PIC** and OF loads it at the link
   address (Key invariant #2/#4), so there are **no runtime relocations to
   parse** at all.

3. **Our ELF is produced, not parsed.** memtestppc+ emits an ELF via the
   cross-linker (`src/linker.ld`, `OUTPUT_FORMAT("elf32-powerpc")`) and Open
   Firmware's `load` interprets it. Nothing in our runtime reads ELF headers,
   program headers, or relocation tables — so the `Elf32_Ehdr`/`Elf32_Phdr`/
   `Elf32_Dyn`/`R_386_*` definitions have no live use.

## What replaces it
Nothing needs to. ELF *consumption* is done by Open Firmware (the loader);
ELF *production* is done by the cross-linker via `src/linker.ld`. No in-image
ELF parser exists, so no `elf.h` equivalent is required.

## Confirmed: no live callers in `src/`
`grep -rn -i -e 'elf\.h' -e 'Elf32' -e 'Elf64' -e 'R_386' src/` returns only
one line — `src/linker.ld:7`'s `OUTPUT_FORMAT("elf32-powerpc")` — which is the
linker-script directive, **not** an include of or reference to this header.
No `.c`/`.h` in `src/` `#include`s `elf.h` or uses any `Elf32_*`/`R_386_*`
symbol.

## Build impact
None. `elf.h` is **not** copied into `src/`, **not** included by any ported
file, and **not** in the Makefile. It stays in `ref/` as upstream reference for
the parked `reloc.c`.

## Open questions / TO VERIFY
None. Clean N-A: a passive ELF/i386-reloc header whose sole consumer
(`reloc.c`) is itself parked; the PPC port neither parses ELF at runtime nor
self-relocates.
