# Port report — `random.c` (Wave 2, display/string core)

## File & purpose
`ref/memtest86+-2.00/random.c` (32 lines) is the pseudo-random number
generator used by the random-pattern / random-sequence memory tests. It
implements a **concatenation of two 16-bit multiply-with-carry (MWC)
generators** (per the header comment, lines 1-4): one stream `x(n) =
a*x(n-1)+carry mod 2^16` with `a=18000`, a second `y(n) = b*y(n-1)+carry
mod 2^16` with `b=30903`; the 16-bit value and its carry are packed into one
32-bit integer each, and `rand()` returns `(SEED_X<<16) + (SEED_Y&65535)`
(lines 16-24). It exposes:
- `unsigned int rand(void)` (line 6 / 16) — returns a 32-bit value.
- `void rand_seed(unsigned int, unsigned int)` (line 7 / 27) — sets the two
  seed words; a 0 argument leaves the default seed for that stream untouched
  (lines 29-30: `if (seed1) SEED_X = seed1;`).
- `#define rand_float ((double)rand() / 4294967296.0)` (line 10) — unused in
  v2.00 (no references to `rand_float` anywhere in the tree).

State lives in two file-static seeds, `SEED_X = 521288629` /
`SEED_Y = 362436069` (lines 12-13).

## Disposition
**Ported — verbatim.** This is pure portable C: there is no x86 leaf code in
the file at all (no `rdtsc`, no `asm`, no port I/O, no MSR). The generator's
output is platform-independent and the random tests must reproduce the same
sequence, so the body was copied byte-for-byte. Only a port header comment was
prepended; the executable code is unchanged.

## x86 leaf code commented out
**None — there is none in this file.** The x86-specific bits associated with
the RNG live at the **call sites in `test.c`** (and `main.c`), which are ported
in a later wave (Wave 4 / Wave 5), not here:
- `test.c:266` — `asm __volatile__ ("rdtsc":"=a"(seed1),"=d"(seed2));` seeds
  from the timestamp counter when `v->rdtsc` is set. (PPC replacement: `mftb`
  / `mftbu` from `ppc.h`, decided in the test.c wave.)
- `test.c:302-314` and `test.c:347-357` — hand-tuned x86 asm loops that inline
  `call rand` to fill / verify memory (upstream left the equivalent plain-C
  `for (...) *p = rand();` commented just above each; per discipline §3, those
  C versions get re-enabled and the asm commented when test.c is ported).
- The default-seed path (`seed1 = 521288629 + v->pass; seed2 = 362436069 -
  v->pass;`, test.c:268-269) is portable C and stays.

So all RNG-related platform work is deferred to the call-site waves; `random.c`
itself needs no edits.

## PPC/OF additions
None to the code. Added only a port-header comment block documenting fidelity,
the deferred seeding, and the prototype/include reconciliation below.

## Signature / include reconciliation (important)
Upstream `random.c` **does not `#include "test.h"`**; it declares its own local
prototypes at the top:
```
unsigned int rand( void );
void  rand_seed( unsigned int, unsigned int );
```
`test.h` (already ported, lines 131-132) declares the same functions with
slightly different spellings:
```
void rand_seed(int seed1, int seed2);
ulong rand();          /* ulong = unsigned long */
```
On the target (ILP32, big-endian PPC) `unsigned int`, `int`, and
`unsigned long` are all 32-bit, so these are **ABI-identical** — `test.c` /
`main.c` call through `test.h`'s prototypes and link against the verbatim
`random.o` with no mismatch. I deliberately **kept `random.c` standalone (did
NOT add `#include "test.h"`)**, matching upstream, because including test.h
would introduce a redeclaration conflict between `unsigned int rand(void)` and
`ulong rand()` (return-type mismatch → compile error) and between the
`unsigned int` vs `int` `rand_seed` params. The task brief also forbids
touching `test.h`. random.c uses no types or macros from test.h, so it has no
need to include it. This is intentional and low-risk; see "Open questions".

## Features surfaced or shelved
- `rand_float` is defined but unused upstream — kept verbatim (harmless;
  `4294967296.0` is `2^32`). No floating-point is actually emitted because
  nothing references it.

## Open questions / TO VERIFY
- **Cross-compiler warning (TO VERIFY at the build checkpoint):** with
  `random.c` standalone, `powerpc-linux-gnu-gcc` may emit a benign
  conflicting-types note only if test.h ever gets transitively pulled into the
  same TU as random.c — it won't, since random.c includes nothing. Expect a
  clean `-fsyntax-only`. If the lead prefers a single source of truth for the
  prototypes, the cleaner long-term fix is to reconcile `test.h`'s
  `ulong rand()` / `rand_seed(int,int)` to `unsigned int` — but that edits
  test.h (out of scope here) and would be a one-line change in a later pass.
- **Sequence fidelity:** the algorithm is unchanged, so the generated
  sequence matches upstream exactly for identical seeds; the only way it could
  differ on PPC is integer width, and all operands are 32-bit
  (`unsigned int`), so wrap-around at `mod 2^32` is identical. No endian
  dependence (no byte-level reinterpretation). Considered verified by reading.

## Build impact
- New object: add `random.o` to the Makefile `OBJS` (upstream had it; the
  port Makefile grows per wave — lead to add when wiring Wave 2).
- No new dependencies: `random.c` includes nothing.
- Call sites (`test.c`, `main.c`) are ported in later waves and already see
  the `rand` / `rand_seed` prototypes via `test.h`.

## Status table row (for HANDOFF.md)
`| random.c | ✅ | report `port-random_c.md`. Pure C, imported VERBATIM (no x86 leaf in-file; rdtsc seed + asm fill loops are call-site code in test.c, deferred). Standalone — declares own prototypes, does NOT include test.h (avoids ulong-vs-uint redeclaration clash; ABI-identical on ILP32 PPC). |`
