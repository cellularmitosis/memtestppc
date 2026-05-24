# examine-dmi_c.md — dmi.c (+ dmi.h)

Wave 6 classification. Read-only: NOT copied into `src/`, NOT compiled/linked.

## File & purpose

`dmi.c` (331 lines) parses the **SMBIOS / DMI tables** that a PC BIOS leaves in
low memory, to report per-DIMM-slot info and to map a failing address back to a
named memory slot. (Header `dmi.c` © Joachim Deguara/AMD 2006, SMBIOS v2.5.)

- **`open_dmi()`** (L130) scans physical memory from `DMI_SEARCH_START` for the
  `"_SM_"` SMBIOS entry-point anchor (L139-147), validates the checksum
  (L155-160) and version (L163), then walks the structure table at
  `eps->tableaddress` (L169-185) collecting **type 17** structures (Memory Device
  → `mem_devs[]`) and **type 20** structures (Memory Device Mapped Address →
  `md_maps[]`). `init_dmi()` (L189) wraps it.
- **`print_dmi_info()`** (L197) renders the "DMI Memory Device Info" popup pages:
  per slot it prints Location (`get_tstruct_string` L114), Size(MB), Speed(MHz),
  Type (`memory_types[]` L92), Form factor (`form_factors[]` L85), and the
  physical address range(s) the slot maps to (L253-271).
- **`add_dmi_err(ulong adr)`** (L281) — given a failing address, finds the
  matching `md_map` range and flags the owning memory device in
  `dmi_err_cnts[]`. **`print_dmi_err()`** (L307) prints "Bad Memory Devices: <slot
  locator list>" on `v->msg_line`.
- The structs (`dmi_eps` L21, `mem_dev` L44, `md_map` L64, `pma` L75) are packed
  overlays of the SMBIOS binary table layout. `dmi.h` declares `add_dmi_err`,
  `print_dmi_err`, `print_dmi_info`.

## Disposition: ⛔ N/A on PPC/OF (do not port)

The entire module depends on an x86 BIOS having published an SMBIOS table at a
fixed low-memory location. PowerPC Macs have **no SMBIOS/DMI** — the analogous
data lives in the **Open Firmware device tree** (`/memory`, DIMM nodes), reached
via `ofw_getprop`, with a completely different format. Confirmed not parked into
`src/`.

## What x86 mechanism makes it N/A

- **Fixed PC physical-address scan** — `dmi_search_start = (char *)DMI_SEARCH_START`
  and the `for(... dmi += 16)` anchor hunt (L136-147) read raw low physical memory
  (the BIOS F-segment region). With OF's MMU on, dereferencing those PC addresses
  faults (CLAUDE.md #4); and there's no `_SM_` anchor there on a Mac anyway.
- **SMBIOS binary format** — the packed structs and `get_tstruct_string()`
  string-table walk (L114-127) decode an Intel/PC firmware data structure that
  PPC firmware does not produce.
- **`eps->tableaddress` deref** (L169) — another absolute PC physical address.

No port I/O / PCI / MSR here (it's pure memory parsing), but the *source* of the
data is x86-firmware-specific, so it's still ⛔.

## Features surfaced or shelved

All **shelved**:

- **"DMI Memory Device Info" popup** — per-slot Location / Size / Speed / Type /
  Form factor / mapped address ranges (`print_dmi_info` L197-278). This is the
  richest hardware-inventory screen in memtest86+; on PPC the equivalent would be
  a *new* OF-device-tree reader, not a port of this code.
- **"Bad Memory Devices: <slot>" line** — `print_dmi_err()` L307, which would
  translate a failing address into a human-readable DIMM-slot name. Its feeder
  `add_dmi_err()` is already commented out at the error.c call site (see below),
  so on PPC a failing address is reported by raw address only, with no slot name.

Conscious call: slot-level attribution is a genuinely useful feature a future PPC
wave might want — but it must come from OF (`/memory` reg/DIMM nodes), so this
file gives us only the *idea*, not reusable code.

## Live callers in the ported `src/` tree — confirmed none

- `error.c:170` — `/* add_dmi_err((ulong)adr); */` commented in the error path,
  with rationale at L166-169.
- `error.c:46-47` — the `extern int dmi_err_cnts[]` / `extern short dmi_initialized`
  externs are commented out (L44-45 note they come from dmi.c, N/A).
- `print_dmi_info()` / `print_dmi_err()` have no caller in `src/` at all (the
  footer/popup key that would invoke the DMI screen is not wired up). grep for
  `print_dmi`/`add_dmi_err`/`init_dmi` in `src/` finds only the commented hooks.

Zero live references. Nothing to stub.

## Open questions / TO VERIFY

None. The module is unambiguously SMBIOS/BIOS-dependent. The only judgment is the
deferral of the per-slot inventory + bad-slot naming, a conscious deferral to a
possible OF-device-tree-based PPC feature later.

## Build impact

None. Not in `src/`, not in the Makefile, no live callers. Pure reference.

> Aside (not load-bearing): dmi.c defines its own `int strlen(char*)` at L107.
> Were this ever ported it would collide with lib.c's `strlen` — another reason
> the OF replacement would be a fresh module, not a copy.
