# Port report — `test.c` (Wave 4)

**File & purpose.** `ref/memtest86+-2.00/test.c` (1424 lines) holds the actual
memory-test routines: `addr_tst1`, `addr_tst2`, `movinvr`, `movinv1`,
`movinv32`, `modtst`, `block_move`, `bit_fade`, plus the helpers `sleep()` and
`beep()` and the static `roundup()`. Each test routine carries a hand-tuned x86
`asm __volatile__` inner loop, with the equivalent plain-C loop usually left
commented just above it.

**Disposition.** Ported. Outer structure (segment loops `for(j=0;j<segs;j++)`,
the `do { ... pe += SPINSZ; ... } while(!done)` SPINSZ chunking with overflow/
underflow guards, forward/reverse passes, `do_tick()`/`BAILR` placement) kept
**verbatim**. For every routine the x86 asm inner loop is commented out (with a
one-line reason) and the plain-C equivalent enabled.

## Per-routine: asm commented → C enabled

| routine | C source | faithful or rewritten |
|---|---|---|
| `roundup` (static inline) | pure C upstream | unchanged |
| `addr_tst1` | pure C upstream (no asm) | unchanged — verbatim |
| `addr_tst2` | upstream-commented C enabled (both passes) | faithful |
| `movinvr` | upstream-commented C enabled (fill + 2 inv passes) | faithful |
| `movinv1` | upstream-commented C enabled (fill, fwd, rev) | faithful |
| `movinv32` | upstream-commented C enabled (fill, fwd, back-up-step, rev) | faithful |
| `modtst` | upstream-commented C enabled (3 loops) | faithful |
| `block_move` | **rewritten C** (no upstream C left) — init, move, check | rewritten, carry-faithful |
| `bit_fade` | pure C upstream (no asm) | unchanged — verbatim |
| `sleep` | rewritten (rdtsc→Time Base) | rewritten |
| `beep` | commented out wholesale | N/A on PPC |

Details:

- **`addr_tst1`** — entirely plain C upstream (walking-ones over address bits).
  No asm to swap; imported verbatim.
- **`addr_tst2`** — two asm blocks (store-own-address, then check-own-address).
  Enabled upstream's commented C: `for(;p<pe;p++) *p=(ulong)p;` and the check
  loop calling `ad_err2((ulong)p, bad)`. `bad` is **not** a function local in
  upstream (the asm declared it implicitly), so I wrapped the check loop in a
  block with a local `ulong bad;`. **Note:** kept the upstream-verbatim
  `ad_err2((ulong)p, bad)` call which passes an integer where the prototype
  wants `ulong *` — an int→pointer warning, matching upstream. (TO VERIFY: lead
  may want a `(ulong*)` cast to silence it; left verbatim for fidelity.)
- **`movinvr`** — fill loop `*p = rand();` and the check-and-complement loop.
  The asm folded the `i`-pass complement into an xor-with-`num` (num=0 or
  0xffffffff); the enabled C uses the original `if(i) num = ~num;` form, which
  is the documented algorithm and bit-identical. Block-scoped `ulong bad;`.
- **`movinv1`** — `rep stosl` fill → `for(;p<pe;p++) *p=p1;`; forward
  check-p1/write-p2 and reverse check-p2/write-p1 (`do{...}while(p-- > pe);`,
  matching the asm's trailing `subl $4`). Block-scoped `ulong bad;` in each.
- **`movinv32`** — re-enabled `ulong p3 = sval << 31;` (upstream commented it
  out because the "CDH" asm rewrite folded the shift into roll/ror; the plain-C
  reverse pass needs it). Enabled the documented original-C forms: forward uses
  `pat<<1 | sval` with `k` wrap at 32; the "back up one step" of k/pat
  (`pat=lb; if(k=(k-1)&31) pat=(pat<<k)| ((sval<<k)-1); k++`); reverse uses
  `pat>>1 | p3` with `--k` reset to `hb`. **Decision:** I chose the
  documented-original-C pattern math over a literal transcription of the
  "CDH" `roll/ror $1` asm (which advances the pattern as a plain 1-bit rotate
  with `sval` dropped). The two diverge in detail; the original C is what the
  algorithm comment describes and is the faithful target per the porting
  contract ("invert: comment the asm, enable the C"). FLAGGED below as TO
  VERIFY.
- **`modtst`** — three asm loops: every-MOD_SZ-th fill of p1 (`addl $80` =
  20 longs = MOD_SZ), fill-all-but-every-nth with p2 (the `cmpl $19`/wrap is
  `if(++k > MOD_SZ-1) k=0`), every-MOD_SZ-th check of p1. All enabled from
  upstream-commented C.

## block_move — rotate-through-carry handling

Upstream `block_move` had **no** commented-C fallback — only asm. I wrote
faithful C for all three phases:

1. **Init (pattern write).** The asm writes a fixed 64-byte block (pattern in 12
   longs, complement in 4) then advances the pattern with **`rcll $1, %%eax`** —
   a 32-bit **rotate-through-carry** (a 33-bit rotate: bit31→CF, old CF→bit0).
   Crucially, the following `decl %%ecx` and `leal` do **not** modify CF, so the
   carry **persists across iterations** — this is a genuine 33-bit rotate, *not*
   a plain `rol`/`<<`. I reproduced it exactly with an explicit carry
   accumulator:
   ```c
   newcarry = (pat >> 31) & 1;
   pat = ((pat << 1) | carry) & 0xffffffff;
   carry = newcarry;
   ```
   starting `pat=1, carry=0`. (The old attic hack used `(pat<<1)|(pat>>31)` — a
   plain rotate — which is **wrong**; I did not inherit it.) The 12/4 long
   layout (offsets 16,20,40,44,56,60 get the complement) is transcribed
   directly from the asm `movl` offsets.
2. **Move (data shuffle).** Three sequential `rep movsl` copies. Reproduced as
   three `memmove`s: `pp <- p` (len longs), `p+32 <- pp` (len-8 longs), then
   `p <- pp+(len-8)` (8 longs). The third copy's source is where the second
   `rep movsl` left `esi` (pp advanced by len-8 longs), transcribed faithfully.
3. **Check.** Adjacent-word compare (`cmpl 4(%edi),%ecx`), stepping 8 bytes;
   on mismatch `error(addr, first, second)` matching the asm's push order
   (edi=addr, ecx=first/good, 4(edi)=second/bad).

## RNG / seed timebase change

`movinvr` seeds from a hardware time source when `v->rdtsc` is set. The x86
`asm("rdtsc":"=a"(seed1),"=d"(seed2))` is commented; replaced with
`seed1=(int)mftb(); seed2=(int)mftbu();` (PPC Time Base low/high — the exact
analog of rdtsc's eax/edx). The non-rdtsc default path
(`521288629 + v->pass` / `362436069 - v->pass`) is kept **verbatim**.
`rand_seed()`/`rand()` are the already-ported `random.c` (called unchanged).

## bit_fade sleep (timebase + 90-min flag)

`bit_fade` itself is pure C upstream and imported verbatim, **including the
`sleep(STIME, 0)` call with `STIME=5400` (90 min) and `test_ticks += STIME*2`.**
The real work is in `sleep()`:

- x86 read the start/now TSC via `rdtsc`, 64-bit-subtracted via `subl/sbbl`, and
  converted to ms/s using `v->clks_msec` (TSC cycles per ms).
- **PPC replacement:** read the 64-bit Time Base via `mftbu`/`mftb` (re-reading
  the high half to guard a low-half rollover), open-code the 64-bit
  borrow-aware subtract, and convert ticks→ms/s using
  `ofw_get_timebase_freq()` (ticks/sec; ticks/ms = freq/1000) instead of
  `clks_msec`. The two-term high·(2^32/div) + low/div structure mirrors the x86
  math. Screen update, `check_input()`, and `BAILR` paths are kept verbatim.

> **⚠ FLAG for the user:** `bit_fade` therefore performs the **full ~90-minute
> dwell** (two patterns × 5400 s = ~3 h total for the test) — faithful to
> v2.00, NOT silently skipped. The user may want to shorten `STIME` later. The
> dwell is real; do not assume the test "hangs."

## calculate_chunk / single-CPU inlining

None needed. v2.00's test routines are already cpu-free (no trailing `int cpu`,
no `calculate_chunk()`/`get_cpu_range()`); the whole segment
(`start=v->map[j].start`, `end=v->map[j].end`) is already the unit of work.
No multi-CPU split to inline or comment.

## PPC/OF additions (summary)

- `movinvr`: `rdtsc` → `mftb()`/`mftbu()` seed.
- `sleep`: `rdtsc`+`clks_msec` → `mftbu`/`mftb` Time Base + `ofw_get_timebase_freq()`.
- `block_move`: carry-faithful C for the `rcll`-rotate init, `memmove` for the
  `rep movsl` shuffle, C for the adjacent-word check.
- All other routines: enabled the upstream-commented plain-C inner loops.
- No cache ops were present between passes in v2.00 `test.c` (the x86 cache
  toggling lives in init/lib, not here), so no `dcbf/sync` was needed in this
  file. (Caching note: `set_cache` is already a PPC stub in lib.c.)

## x86 leaf commented out

- `#include <sys/io.h>` and `#include "dmi.h"` — only `sys/io.h` was used (by
  `beep`); both commented.
- Every test routine's inner `asm __volatile__` block (listed above).
- `movinvr` rdtsc seed asm; `sleep` rdtsc + subl/sbbl + clks_msec math.
- `beep()` — entire function commented (PC-speaker port I/O, N/A; only caller
  is main.c's `v->beepmode` error path, left inert).

## Open questions / TO VERIFY

1. **`movinv32` pattern math (asm vs documented-C):** the "CDH" asm advances the
   pattern via plain `roll/ror $1` (sval folded out), while the enabled
   documented-C uses `pat<<1|sval` / `pat>>1|p3`. These produce **different walk
   sequences** when `sval != 0`. I chose the documented-C per the contract.
   Verify against a green run that the moving-32-bit-pattern test passes; if the
   intent was the asm's plain-rotate sequence, swap to that. (For the standard
   tseq invocations `sval` is typically 0/1 — at sval=0 the two coincide except
   for the `lb`/`hb` reload edges.)
2. **`addr_tst2` `ad_err2((ulong)p, bad)`** passes an int to a `ulong*` param —
   int→pointer warning, kept verbatim from upstream. Lead may add a `(ulong*)`
   cast.
3. **`ofw_get_timebase_freq()` returning 0** would make `sleep` divide-by-zero;
   I guard with `if (tb_X == 0) tb_X = 1`. Confirm OF always provides
   `timebase-frequency` (memsize/init use it; substrate has it).
4. **`block_move` move-phase `len-8` vs tiny segments:** if a SPINSZ*4 chunk is
   smaller than 8 longs (`len < 8`), `(len-8)*4` underflows. Upstream had the
   same exposure; in practice chunks are huge (SPINSZ*4 = 128 MB). Noted, not
   changed.

## Build impact

`src/test.c` is self-contained C (no new headers beyond `test.h`+`config.h`).
Add it to the Makefile `OBJS` in Wave 4. **Globals/functions the Wave-4
checkpoint must provide** (all `extern`-declared at the top of test.c, verbatim
from upstream):

- **From error.c (parallel Wave-4 agent):** `error()`, `ad_err1()`, `ad_err2()`,
  `do_tick()`, plus the declared-but-unused-here `update_err_counts()`,
  `print_err_counts()`, `poll_errors()`.
- **From main.c (Wave 5) / checkpoint stub:** `int segs`, `int bail`,
  `volatile ulong *p`, `ulong p1, p2`, `int test_ticks, nticks`,
  `struct tseq tseq[]`, and `struct vars * const v`.
- **From lib.c (Wave 2, done):** `cprint`, `hprint`, `dprint`, `check_input`,
  `memmove`.
- **From random.c (Wave 2, done):** `rand`, `rand_seed`.
- **From ppc.h (substrate):** `mftb`, `mftbu`.
- **From ofw.{c,h} (substrate):** `ofw_get_timebase_freq`.
- `BAILR`, `SPINSZ`, `MOD_SZ` from test.h; `USB_WAR` (undefined → the
  `#ifdef USB_WAR` low-memory guards stay compiled out, correct for PPC).
