# examine-patn_c.md — Wave 6 classification of `patn.c`

## File & purpose
`ref/memtest86+-2.00/patn.c` (144 lines) — the **BadRAM pattern** support. It
maintains a small table (`v->patn[BADRAM_MAXPATNS]`, max 10 adr/mask pairs;
`test.h:212,368-369`) and folds each newly-failing address into it, keeping the
masks as selective as possible. The table is what `PRINTMODE_PATTERNS` dumps as a
Linux `badram=0x..,0x..` boot option.

Functions (all in this file):
- `combine()` (39-46) — merge two adr/mask pairs via the `COMBINE_MASK` macro (35).
- `addresses()` (50-60) — count addresses a mask covers; loops `i=32`.
- `combicost()` (65-70), `cheapindex()` (75-86) — cost heuristic over `v->patn[]`.
- `relocateidx()` (91-99), `relocateiffree()` (105-120) — buddy-style coalescing.
- `insertaddress(ulong adr)` (125-144) — public entry; returns 1 if table changed.

It is **pure pattern math + array bookkeeping over `struct vars *v`**. It `#include`s
only `test.h` (line 16). `printpatn()` is **NOT** in this file — it lives in
`error.c` (`ref` 394; already ported live at `src/error.c:478-508`). The task brief
was slightly off on that point.

## Disposition — **PORT AS NEUTRAL** (surface the feature)
No x86 facilities whatsoever: no asm, no port I/O, no MSR/PCI/BIOS/serial, no
`io.h`. Just C arithmetic, comparisons, recursion, and writes to the shared
`v->patn[]`/`v->numpatn` struct. It is byte-for-byte portable.

### 32-bit assumption (verified, not a blocker)
The code is hardwired to 32-bit longwords: `DEFAULT_MASK ((~0L) << 2)` (22) and
`addresses()`'s `i=32` (52). Target is `elf32-powerpc` (`src/linker.ld:7`) with
`typedef unsigned long ulong` (`src/test.h:59`) = 32-bit, so the math matches the
x86 build exactly — no fix needed. (On a 64-bit build this would undercount, but
that is not our target.)

## x86 leaf code commented out
**None.** Nothing in the file is platform-specific, so a verbatim copy compiles
and runs correctly. The `/* extern struct vars *v; */` already-commented line (25)
stays as-is (`v` comes from `test.h`).

## PPC/OF additions
**None required.** Port is a verbatim copy of the upstream file.

## Features surfaced or shelved
- Surfacing `patn.c` makes the real `insertaddress()` available, so the
  `PRINTMODE_PATTERNS` branch in `src/error.c:404-422` (`insertaddress` →
  `printpatn`) becomes fully functional instead of a no-op.
- **Caveat — feature is reachable only if `printmode` can be set to
  PRINTMODE_PATTERNS.** Today `src/init.c:323` hardcodes
  `v->printmode = PRINTMODE_ADDRESSES`; the selector (`get_printmode()`, the `c`
  config menu) is in the **parked `config.c`** (Decision #6 / Task #6). So porting
  `patn.c` removes the stub and makes the *code path* correct and live, but the
  PATTERNS dump won't be user-visible until config.c's print-mode selection is
  surfaced. Worth doing anyway: it deletes a stub, restores upstream fidelity, and
  the table-update logic is then exercised the moment PRINTMODE_PATTERNS is set.

## Open questions / TO VERIFY
None. The file is self-contained, all symbols it touches (`v`, `struct pair`,
`BADRAM_MAXPATNS`, `ulong`) are defined in `src/test.h`, and `insertaddress` is
already declared there (`src/test.h:132`).

## Build impact / integration steps for the lead
1. Copy `ref/memtest86+-2.00/patn.c` verbatim to `src/patn.c` (only edit: confirm
   the include resolves to `test.h` — it does).
2. **Delete the Wave-5 stub** `int insertaddress(ulong addr){ (void)addr; return 0; }`
   and its comment block at `src/main.c:203-213` (the real one in `patn.c` replaces it).
3. **Add `src/patn.o` to `OBJS`** in `Makefile:22` and a build rule, e.g.
   `src/patn.o: src/patn.c src/test.h`.
4. No other call sites change. `printpatn()` (`src/error.c:478`) and the
   `PRINTMODE_PATTERNS` branch (`src/error.c:404-422`) already call into it;
   `v->numpatn=0` is initialized at `src/init.c:324`.
5. Watch for a duplicate-symbol link error: confirm no other surviving definition
   of `insertaddress` remains once the main.c stub is removed (grep shows the stub
   at `src/main.c:209` is the only one).
