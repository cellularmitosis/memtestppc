# Port report — `config.h` (+ `stdint.h`, `defs.h`)

Wave 1 headers. Owner of `config.h`, `stdint.h`, `defs.h` only (a separate
agent owns `test.h`). Discipline per
[`PORTING-GUIDE.md`](PORTING-GUIDE.md): import verbatim, comment x86 leaf with a
one-line reason, never delete, keep buildable.

## Files & purpose
- `config.h` — compile-time feature flags (parity, serial console, beep, APM,
  USB workaround, screen-buffer debug).
- `stdint.h` — freestanding fixed-width integer typedefs (no libc on target).
- `defs.h` — x86 boot/relocation constants shared between C and the real-mode
  assembler (segment addresses, GDT selectors, low test address).

## Disposition
- `config.h` — **ported.** Verbatim import; each flag consciously kept or
  disabled with a reason. Created `src/config.h`.
- `stdint.h` — **ported (verbatim, validated).** The x86 ILP32 typedefs are
  already correct for 32-bit big-endian PowerPC (also ILP32). No width changes
  needed; added a port note documenting why. Created `src/stdint.h`.
- `defs.h` — **imported + body disabled.** Every constant is x86 real-mode
  boot / GDT / relocation specific and has no PPC/OF meaning. Body wrapped in
  one `#if 0 … #endif` (preserves the original lines and their inline comments
  byte-for-byte). Created `src/defs.h`.

---

## config.h — per-flag keep/disable decisions

Usage was grepped across the whole v2.00 tree (`grep -a`, ISO-8859 safe) to
make each call evidence-based.

| Flag | Decision | Reason (and where used upstream) |
|---|---|---|
| `PARITY_MEM` | **DISABLE** (commented `#define`) | x86 ISA/PCI memory-parity NMI reporting via `parity_err()` — `lib.c:445`, `error.c:374`. No x86 parity NMI bus on PPC/OF. |
| `SERIAL_CONSOLE_DEFAULT 0` | **KEEP, value 0** | Default state of the (now nonexistent) serial console. Kept at 0 so `lib.c:19 serial_cons` never goes hot; our ttyprint drives the framebuffer, not a UART. |
| `SERIAL_TTY 0` | **KEEP DEFINED** | x86 UART selector (0x3f8/0x2f8) — N/A, but kept **defined** because `lib.c:21` has a compile-time guard `#if SERIAL_TTY != 0 && SERIAL_TTY != 1 #error`. Removing it would break that `#if`. Serial leaf code itself is commented out in the lib.c port. |
| `SERIAL_BAUD_RATE 9600` | **KEEP DEFINED** | x86 UART divisor base — N/A, but kept **defined** because `lib.c:27` guards `#if ((115200%SERIAL_BAUD_RATE) != 0) #error`. Removing it breaks that `#if`. |
| `BEEP_MODE 0` | **KEEP DEFINED, value 0** | Drives `beep()` which programs the PC speaker (PIT) — `test.c:1405`, `error.c:72-73`. No PC speaker on PPC. Kept **defined** at 0 because `init.c:113` does `beepmode = BEEP_MODE;` (needs the symbol to compile); 0 keeps the beep path dead. |
| `SCRN_DEBUG` | **KEEP (commented, as upstream)** | Platform-neutral SCREEN_BUFFER consistency check (`screen_buffer.c:28/114/127`). Upstream ships it disabled; left exactly so. Not x86-specific — could be enabled later for debugging. |
| `APM_OFF` | **DISABLE** (commented `#define`) | Gates the real-mode APM-disconnect BIOS call in `head.S:888`. x86 APM has no PPC analog; our head.S is PPC asm and never sees it. |
| `USB_WAR` | **DISABLE** (commented `#define`) | Works around BIOS USB legacy-keyboard SMM emulation corrupting low memory — `error.c:36`, `test.c:1087/1156/1223`. OF owns ADB/USB input; no SMM legacy emulation on PPC. |

### Flags named in the task brief but ABSENT in v2.00
- `BEEP_END_NO_ERROR` and `CONSERVATIVE_SMP` are **v5.01 additions**; they do
  not exist in v2.00's `config.h` (confirmed by tree-wide grep). Nothing to
  disable. Documented in the header so the absence is intentional, not an
  oversight.

### Key subtlety (the reason this header isn't just `#undef` everything)
`SERIAL_TTY`, `SERIAL_BAUD_RATE`, and `BEEP_MODE` must stay **defined** even
though their features are dead, because they feed **compile-time** `#if`/`#error`
guards and an initializer (`beepmode = BEEP_MODE`). `#undef`-ing them would turn
those guards into hard preprocessor errors. We instead leave them defined with
inert values (0 / 9600) and rely on the *consumer* files (lib.c serial block,
test.c/error.c beep) to comment out the actual leaf code. Verified by
preprocessing the exact lib.c guards against `src/config.h` — they compile.

---

## stdint.h — PPC32 width review

Target: 32-bit big-endian PowerPC ELF via `powerpc-linux-gnu-gcc`, the **ILP32**
model — identical width assignment to the x86 source: `char`=8, `short`=16,
`int`=32, `long`=32, `long long`=64, pointer=32.

Conclusion: **no width changes required.** Each upstream typedef is already
correct for PPC32:

| Type | Upstream def | PPC32 width | Correct? |
|---|---|---|---|
| `uint8_t` | `unsigned char` | 8 | yes |
| `uint16_t` | `unsigned short` | 16 | yes |
| `uint32_t` | `unsigned int` | 32 (int is 32 on PPC32) | yes |
| `uint64_t` | `unsigned long long` | 64 | yes |
| `intptr_t`/`uintptr_t` | `int`/`unsigned int` | 32 (PPC32 pointers are 32-bit) | yes |

Notes:
- Endianness does not enter `stdint.h` — these are byte-width definitions only;
  big-endian representation is transparent to the C type system.
- The `I386_STDINT_H` include guard is a cosmetic token; left as-is to keep the
  diff against upstream minimal (renaming it would touch nothing functional).
- Upstream uses `int`/`unsigned int` (not `long`) for the 32-bit types; on PPC32
  `int` and `long` are both 32-bit so this is fine, and `int` is the faithful
  choice.

---

## defs.h — kept vs disabled

Every symbol is x86-specific; all are disabled (one `#if 0` region):

- `SETUPSECS` — x86 real-mode setup-sector count for the PC boot image.
- `LOW_TEST_ADR` — address the x86 image relocates itself down to (< 640K).
- `BOOTSEG` / `INITSEG` / `SETUPSEG` / `TSTLOAD` — x86 real-mode segment paragraphs.
- `KERNEL_CS` / `KERNEL_DS` / `REAL_CS` / `REAL_DS` — x86 GDT selectors.

PPC/OF replacement: none in this header. The platform model differs — OF's
`load` claims+maps the ELF (MMU on), `head.S` (PPC) sets up the stack in BSS,
`linker.ld` fixes the load address (0x00400000), and the port tests RAM in
place rather than relocating. Those facilities live in the substrate, not here.

**Nested-comment hazard avoided:** the upstream lines carry inline `/*…*/`
comments. Wrapping each in `/*…*/` would have closed the comment early and left
stray `*/` tokens (a compile error). Using `#if 0 … #endif` (explicitly blessed
by the guide for dead regions) preserves the lines verbatim and compiles.

---

## Cross-file dependency note (for the main.c porter)
`defs.h` symbols `LOW_TEST_ADR` and `INITSEG` are referenced **only** by
`main.c`'s x86 self-relocation / boot-protocol command-line machinery
(`__run_at`/`run_at`, `MK_PTR(INITSEG,…)` — `main.c:99,132,140,155,195,200,221,
422,456,477`). Those are exactly the regions the main.c port already plans to
comment out (HANDOFF: "comment reloc/window/cmdline"). With `defs.h`'s body
disabled, the main.c port MUST comment those references in the same pass or the
build will fail to resolve `LOW_TEST_ADR`/`INITSEG`. This is consistent — the
defs and their only call sites are retired together.

The x86 `setup.S`, `bootsect.S`, and x86 `head.S` also `#include "defs.h"`, but
they are replaced wholesale by our PPC substrate (`head.S`) and never built, so
their dependence on these defs is moot.

---

## Open questions / TO VERIFY
- **lib.c serial guards (low risk):** I kept `SERIAL_TTY`/`SERIAL_BAUD_RATE`
  defined specifically so lib.c's `#if` guards compile. Verified by
  preprocessing those exact guards against `src/config.h`. If the lib.c porter
  instead deletes the guard lines along with the serial block, these macros
  become unused but harmless — no action needed either way.
- **`SCRN_DEBUG`:** left disabled as upstream. If we want a screen-buffer
  consistency check during PPC bring-up it can be enabled; it is platform-neutral.

## Build impact
- New files: `src/config.h`, `src/stdint.h`, `src/defs.h`. No Makefile change
  (headers, not compiled units).
- Consumers in our active tree: `config.h` ← lib.c, screen_buffer.h, init.c,
  memsize.c, error.c, test.c. `stdint.h` ← reloc.c, elf.h (and dmi.c if ever
  ported). `defs.h` ← init.c, memsize.c, main.c (the latter only in
  to-be-commented x86 paths — see cross-file note).
- Validated on the host compiler (`cc -fsyntax-only`, compiler-agnostic): all
  three headers preprocess cleanly; `PARITY_MEM`/`APM_OFF`/`USB_WAR`/`SCRN_DEBUG`
  confirmed undefined; `defs.h` body symbols confirmed disabled; lib.c-style
  `#if SERIAL_TTY`/`#if 115200%SERIAL_BAUD_RATE` guards compile; `uint32_t`==4,
  `uint64_t`==8. The authoritative cross-compile happens at the wave checkpoint
  on opti7050.
