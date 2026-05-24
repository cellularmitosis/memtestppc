# examine: linuxbios.c (+ linuxbios_tables.h)

Wave 6 classification. Upstream: `ref/memtest86+-2.00/linuxbios.c` (154 lines),
`ref/memtest86+-2.00/linuxbios_tables.h` (82 lines). Classify-only — NOT compiled
into the PPC build, `src/` and the Makefile are unchanged.

## File & purpose
`linuxbios.c` parses **coreboot / LinuxBIOS** firmware tables to recover the
system memory map on x86 machines whose firmware is coreboot rather than a
classic PC BIOS.

- `ip_compute_csum()` (lines 4–54) — IP-style 16-bit ones-complement checksum used
  to validate the table.
- `for_each_lbrec` macro (56–61) + `count_lb_records()` (64–73) — iterate the
  variable-length `lb_record` entries.
- `__find_lb_table()` (75–96) — scan a physical address range in 16-byte steps for
  the **`"LBIO"`** signature (line 82), validating header size, header checksum,
  table checksum, and record count.
- `find_lb_table()` (98–111) — probe the two canonical x86 physical locations:
  **`0x00000`–`0x1000`** and **`0xf0000`–`0x100000`** (lines 104, 108) — the low
  IVT/BDA area and the legacy BIOS ROM shadow window.
- `query_linuxbios()` (113–153) — find the table, locate the `LB_TAG_MEMORY`
  record (tag `0x0001`, `linuxbios_tables.h:58`), and copy each `lb_memory_range`
  (`start`/`size`/`type`, mapping `LB_MEM_RAM` → `E820_RAM` else `E820_RESERVED`,
  line 146) into the global `mem_info.e820[]` array (lines 136–151).

`linuxbios_tables.h` is the wire-format header for those tables: `lb_header`
(`signature[4]="LBIO"`, checksums, counts — lines 35–43), `lb_record`
(tag+size, 51–54), `lb_memory_range`/`lb_memory` (60–73), `lb_hwrpb` (76–80), and
the tag/type constants (`LB_TAG_MEMORY`, `LB_MEM_RAM`, …).

## Disposition: ⛔ N/A on PPC/OF — not ported
A PowerPC Mac's firmware is **always Open Firmware**, never coreboot/LinuxBIOS.
The entire mechanism is moot for three independent reasons:
1. There is no LinuxBIOS table to find — the `"LBIO"` signature (`linuxbios.c:82`)
   will never appear.
2. The probe addresses `0x00000` and `0xf0000` (lines 104/108) are x86 physical
   conventions (IVT/BDA, BIOS ROM shadow). On a Mac under OF's MMU these are
   either unmapped (→ fault) or meaningless.
3. The output sink — the global `mem_info`/`e820[]` E820 map — does not exist on
   PPC. There is no E820 producer at all.

## What already-ported substrate replaces it
**`src/memsize.c`** (read in full). On x86, `mem_size()` dispatches via
`memsize_bios()` → `memsize_linuxbios()` (the LinuxBIOS path) which turns the
LinuxBIOS-populated `e820[]` into `v->pmap[]`. In the port:
- `memsize_bios()` is commented out (`memsize.c:137–147`) and its LinuxBIOS branch
  (`memsize_linuxbios()`, line 144) with it.
- `memsize_linuxbios()` itself is **preserved verbatim but commented out**
  (`memsize.c:286–311`), with the explicit note that LinuxBIOS does not exist on a
  PPC Mac and is replaced by `memsize_ofw()`.
- The replacement is **`memsize_ofw()`** (`memsize.c:176–259`): it
  `ofw_finddevice("/memory")`, reads the `"reg"` property via `ofw_getprop`, and
  `ofw_claim`s RAM in 1 MB chunks into `v->pmap[]` — exactly the
  `addr>>12` page-unit shape `memsize_linuxbios()` produced (note at
  `memsize.c:288–290`). OF's device tree is the authoritative memory source, so
  all three x86 sizing modes collapse to one OF pass.
- `init.c:202–216` keeps the upstream `query_linuxbios()/query_pcbios()` firmware
  probe **commented out wholesale**, with the firmware externs neutralized
  (`memsize.c:44–46`: `firmware = FIRMWARE_UNKNOWN`, `e820_nr = 0`).

## Live callers in src/: none
- `test.h:130` still declares `int query_linuxbios(void);` (prototype carried for
  fidelity — harmless, no definition linked).
- The only call site, `init.c:209`, is inside the commented-out firmware-probe
  block (`init.c:206–216`).
- `query_linuxbios()` is **not defined anywhere in `src/`** (`linuxbios.c` was
  never copied in), so nothing references the `lb_*` structs or `LBIO` scan.

No build impact. No surprises — fully and cleanly superseded by OF discovery.
