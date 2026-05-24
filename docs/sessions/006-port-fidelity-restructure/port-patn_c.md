# Port report — `patn.c` (Wave 6)

## File & purpose
`ref/memtest86+-2.00/patn.c` (144 lines) builds the **BadRAM pattern table** —
the `badram=0x..,0x..` address/mask pairs shown by `PRINTMODE_PATTERNS`. It is
pure array/bit bookkeeping over `struct vars *v`: `combine()`, `addresses()`,
`combicost()`, `cheapindex()`, `relocateidx()`, `relocateiffree()`, and
`insertaddress()` (the entry point `error.c`/`common_err()` calls when in
PATTERNS mode). `printpatn()` — which dumps the table — is **not** here; it lives
in `error.c` (already ported, `src/error.c`).

## Disposition — **ported VERBATIM** (neutral).
`patn.c` is platform-neutral C: no asm, no port I/O, no MSR/PCI/BIOS/serial, and
it includes only `test.h`. There is nothing x86-specific to comment. Copied to
`src/patn.c` unchanged except a port-note header.

## The one platform assumption (verified safe)
`DEFAULT_MASK = (~0L) << 2` and `addresses()` looping `i = 32` assume a **32-bit
longword**. Our target is `elf32-powerpc` (ILP32, `ulong` is 32-bit), so this is
identical to the x86 build — correct, not a bug. (On an LP64 build it would be
wrong, but we never build LP64.)

## What this replaces
`insertaddress()` here **replaces the Wave-5 no-op stub** that was in
`src/main.c` (`int insertaddress(ulong){ return 0; }`), which existed only so
`error.c`'s PATTERNS branch would link. With the real `patn.c`, the BadRAM
pattern accumulation is functional.

## Features surfaced / shelved
- **Surfaced:** the BadRAM `PRINTMODE_PATTERNS` feature is now real — failing
  addresses accumulate into `v->patn[]` and `printpatn()` (error.c) emits the
  `badram=` boot-option string.
- **Caveat:** the dump is only *visible* once the user selects PATTERNS mode in
  the config menu (Error Report Mode → BadRAM Patterns). `init.c` defaults to
  `PRINTMODE_ADDRESSES`; the selector is in the now-ported `config.c`. So on a
  default run the code is correct but dormant.

## Build impact
- Added `src/patn.c` to the Makefile `OBJS` + a build rule (`src/patn.o:
  src/patn.c src/test.h`).
- Removed the `insertaddress()` stub from `src/main.c` (no duplicate symbol —
  verified by a clean link).
- No call-site changes: `error.c` (`common_err`/`printpatn`) already references
  `insertaddress`/the pattern table. Compiles + links clean (only the known
  pre-existing `test.c` warnings remain).

## Open questions / TO VERIFY
- The accumulation logic is exercised only when real errors occur in PATTERNS
  mode — not hit on QEMU's good RAM. Logic is verbatim-upstream and pure C, so
  confidence is high, but a real-error run (or fault injection) in PATTERNS mode
  would exercise it end-to-end. Low priority.
