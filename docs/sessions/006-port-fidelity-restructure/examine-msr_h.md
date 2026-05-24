# examine: msr.h

Wave 6 classification. Upstream: `ref/memtest86+-2.00/msr.h` (124 lines).
Classify-only — NOT compiled into the PPC build; `src/` and the Makefile unchanged.

## File & purpose
`msr.h` is the x86 **Model-Specific-Register** access header (guard
`__ASM_MSR_H`). It is two things:

1. **Inline-asm accessors** for x86-only instructions (lines 10–34):
   - `rdmsr(msr,val1,val2)` → `rdmsr` instruction (10–13): read 64-bit MSR
     selected by `ecx` into `edx:eax`.
   - `wrmsr(msr,val1,val2)` → `wrmsr` (15–18): write `edx:eax` to the MSR in `ecx`.
   - `rdtsc(low,high)` / `rdtscl(low)` / `rdtscll(val)` → `rdtsc` (20–27): read the
     64-bit Time-Stamp Counter.
   - `write_tsc(v1,v2)` → `wrmsr(0x10,…)` (29): write the TSC via MSR 0x10.
   - `rdpmc(counter,low,high)` → `rdpmc` (31–34): read a performance-monitoring
     counter.
   All five use x86 register-constraint asm (`"=a"`,`"=d"`,`"c"`) — none of which
   exist on PowerPC.

2. **Symbolic MSR-number constants** (lines 36–122): Intel `MSR_IA32_*` (APIC base
   `0x1b`, microcode rev `0x8b`, thermal/throttle `0x19a–0x19c`, machine-check
   banks `0x400+`, perf counters), plus AMD `MSR_K6_*`/`MSR_K7_*`, Centaur/IDT,
   VIA Cyrix, and Transmeta MSR addresses. These are an x86 numbering space with no
   PowerPC analog.

## Disposition: ⛔ N/A on PPC/OF — not ported
`rdmsr`/`wrmsr`/`rdtsc`/`rdpmc` are x86 instructions; the constants name x86 MSRs.
PowerPC has no MSR address space — its equivalent control/timer state lives in
**SPRs** (Special-Purpose Registers) accessed by `mtspr`/`mfspr` (and the timebase
by `mftb`/`mftbu`), which is a completely different mechanism.

## What already-ported substrate replaces it
- **The timestamp need** (the only thing memtest86+ actually used MSRs for —
  reading `rdtsc` to time the CPU-speed gate and to drive the elapsed-time clock):
  replaced by the PPC **timebase** intrinsics `mftb()`/`mftbu()` in **`ppc.h`**
  (PORTING-GUIDE §4 — the PPC instruction intrinsics that replace x86 MSR/rdtsc).
- **CPU speed** specifically: the upstream `cpuspeed()` in `init.c` timed a
  PIT-gated `rdtsc` interval; that whole function is commented out under `#if 0`
  (`init.c:739–...`, the inline `rdtsc` at `init.c:750`/757 is dead), and clock
  frequency now comes from the OF CPU node's `clock-frequency` property (read in
  `cpu_type()`, per the note at `init.c:734–738`).
- No code needed APIC/thermal/machine-check/perf MSRs, so those constants have no
  replacement and need none.
- General platform-register / I/O facilities route through **`ofw.{c,h}`** per
  PORTING-GUIDE §4; SPR-level reads (timebase) through **`ppc.h`**.

## Live callers in src/: none
- `msr.h` is **not `#include`d anywhere** in `src/` (no copied-in `msr.h`).
- `grep` for `rdmsr`/`wrmsr`/`rdtsc`/`rdpmc`/`write_tsc` across `src/` finds only
  **dead `rdtsc` asm inside `#if 0`** (`init.c:750`, `init.c:757`); no live use of
  any MSR accessor or constant.

No build impact. No surprises — none of the x86 MSR machinery is reachable; the
timing role is fully covered by `ppc.h`'s timebase intrinsics.
