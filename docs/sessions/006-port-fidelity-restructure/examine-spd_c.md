# examine-spd_c.md — spd.c

Wave 6 classification. Read-only: NOT copied into `src/`, NOT compiled/linked.

## File & purpose

`spd.c` (140 lines, Reto Sonderegger 2004) reads the **SPD EEPROM** on each DIMM
over the chipset's **SMBus** and dumps the raw 256 bytes in a popup. SPD =
Serial Presence Detect, the small I²C EEPROM on every DIMM holding its
timing/size/type parameters.

- **`find_smb_controller()`** (L85) brute-force scans PCI bus 0 (dev 0-31, fn 0-7)
  via `pci_conf_read`, matching vendor/device against the `smbcontrollers[]` table
  (L73-83) — all Intel ICH4-ICH9 SMBus controllers (`{0x8086,0x2930,"Intel
  ICH9",...}` etc.).
- **`ich5_get_smb()`** (L26) reads the SMBus base I/O address from PCI config
  offset 0x20 (L30-31).
- **`ich5_smb_read_byte()`** (L34) drives the ICH SMBus host controller through
  port I/O — `__outb`/`__inb` to `SMBHSTSTS/CNT/CMD/ADD/DAT` (offsets off
  `smbusbase`, L13-17): reset, set command/address, kick a byte-read transaction
  (0x48), poll status, return the data byte. Timeout uses `rdtsc`/`v->clks_msec`
  (L44-49).
- **`ich5_read_spd()`** (L54) reads all 256 SPD bytes from DIMM address `0x50+n`.
- **`show_spd()`** (L108) is the popup: find controller → for each of 16 slots
  read SPD and `hprint2` the 256 bytes in a hex grid (L125-137).

## Disposition: ⛔ N/A on PPC/OF (do not port)

Doubly x86-specific: it both finds the SMBus controller over PCI config space and
drives it through x86 port I/O, for Intel-ICH-family controllers that don't exist
on a Mac. On PPC, DIMM SPD (when exposed at all) is an I²C device in the OF device
tree; reading it would be a `ofw_*` / I²C-driver job, format-compatible at the SPD
byte level but with an entirely different transport. Confirmed not parked into
`src/`.

## What x86 mechanism makes it N/A

- **Port I/O `__inb`/`__outb`** (L38-51) — the ICH SMBus host registers are x86
  I/O-port mapped. PPC has no `in`/`out` instruction; I/O is memory-mapped and the
  bus address comes from OF. (io.h, the source of `__inb`/`__outb`, is itself a
  Wave-6 ⛔ file.)
- **PCI configuration cycles `pci_conf_read`** (L30, L91, L95) — used to find the
  controller and its SMBus base. Same reason as controller.c: PPC PCI config is
  firmware/host-bridge-specific and OF already enumerates the bus.
- **Intel ICH SMBus controller IDs + register layout** — `smbcontrollers[]`
  (L73-83) and the `SMBHST*` offsets (L13-17) are Intel-chipset-specific; a Mac's
  SMBus/I²C lives on Apple UniNorth/KeyLargo, not an ICH.
- **`rdtsc` / `v->clks_msec`** timeout (L44-49) — x86 timestamp counter; PPC uses
  the timebase (`mftb`, ppc.h).

## Features surfaced or shelved

**Shelved:** the **"SPD Data: Slot N"** hex-dump popup (`show_spd` L108). This is
a power-user diagnostic (raw EEPROM bytes), not part of the main test TUI. memtest86+
exposes it via a footer/menu key; on PPC we surface nothing here. A future PPC wave
could re-implement it as an OF/I²C SPD reader — the *display* (`hprint2` grid) and
the SPD byte semantics would carry over, only the read transport changes.

## Live callers in the ported `src/` tree — confirmed none

- `show_spd()` is declared in `src/test.h:198` (`void show_spd(void);`) but is
  **never called** anywhere in `src/` (grep finds only the prototype; no footer
  key invokes it). The internal helpers (`find_smb_controller`, `ich5_*`) are file-
  local and unreferenced.

Zero live references. The dangling prototype in test.h is harmless (no definition
linked because spd.c isn't compiled); if the lead wants tidiness it could be
commented, but it causes no build issue.

## Open questions / TO VERIFY

None. Unambiguously x86-SMBus/port-I/O specific.

## Build impact

None. Not in `src/`, not in the Makefile, no live callers (only an unused
prototype in test.h). Pure reference.
