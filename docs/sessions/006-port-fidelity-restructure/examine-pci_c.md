# examine-pci_c.md — `pci.c` (160) + `pci.h` (95)

Wave-6 classification. Read in full; cited by line. Classify-only — **not**
compiled into the PPC build, `src/` untouched.

## File & purpose

`pci.c` is the x86 PCI **configuration-space** access layer: raw read/write of a
device's 256-byte config space, plus probing for which legacy access mechanism
the chipset supports.

- `pci_conf_read` (28–58) / `pci_conf_write` (60–90): read/write 1/2/4 bytes at
  `(bus, dev, fn, reg)`. Two mechanisms, selected by `pci_conf_type`:
  - **Config Type 1** (37–44, 69–76): write the 32-bit address word
    `0x80000000|bus<<16|dev<<11|fn<<8|(reg&~3)` (macro 23–24) to I/O port
    **`0xCF8`** via `outl`, then read/write data through port **`0xCFC`**
    (`inb`/`inw`/`inl` / `outb`/`outw`/`outl`).
  - **Config Type 2** (45–55, 77–87): legacy pre-PCI-2.0 scheme using ports
    `0xCF8`/`0xCFA`/`0xCFB` and the `0xC000|dev<<8|reg` window (macro 26).
- `pci_sanity_check` (92–109): reads the class register of bus/dev/fn 0/0/0 and
  confirms it is `PCI_CLASS_BRIDGE_HOST` (0x0600) — i.e. there's a host bridge.
- `pci_check_direct` (111–150): decides `pci_conf_type`. Special-cases AMD fam-15
  (`cpu_id.vend_id[0]=='A' && type==15`, 116) to Type 1; otherwise probes Type 1
  then Type 2 by poking the ports and re-reading (122–141), restoring saved
  port state. Sets `PCI_CONF_TYPE_NONE` on total failure (146).
- `pci_init` (152–160): just calls `pci_check_direct`.

`pci.h` is pure declarations + the standard PCI config-space register-offset and
class constants (`PCI_VENDOR_ID`=0x00 … `PCI_CLASS_BRIDGE_HOST`=0x0600, lines
15–94). No code.

## Disposition: ⛔ N/A — parked, not ported

Every access path is **x86 port I/O** (`inb/inl/outb/outl` on the fixed I/O ports
`0xCF8`/`0xCFA`/`0xCFB`/`0xCFC`, lines 38–54, 70–86, 122–141). PowerPC has no
port-I/O address space, and per PORTING-GUIDE §4, **Open Firmware owns the PCI
tree** — we never issue raw config cycles. PPC PCI config access would instead go
through OF (`call-method config-l@`/`config-l!` on a PCI node, or the
mac-io–specific CONFIG_ADDR/CONFIG_DATA MMIO windows), which is a completely
different mechanism — not a leaf swap. So this is a parked reference file, kept
verbatim under `ref/`, not brought into `src/`.

## x86 mechanism cited
- Port I/O: `outl(...,0xCF8)` + `inb/inw/inl(0xCFC...)` (38–42); Type-2 ports
  `0xCF8/0xCFA/0xCFB` (46–54, 122–141). All via `io.h`.
- x86 CPU dispatch: `cpu_id.vend_id`/`cpu_id.type` (19, 116).

## Live callers in ported `src/` — NONE
`grep -rn pci_ src/` → only `init.c:225-228`, a commented-out `/* pci_init(); */`
with the reason already written. No live reference. Upstream callers (all
themselves parked/N-A): `init.c` `pci_init`; `controller.c`, `extra.c`, `spd.c`
use `pci_conf_read/write`.

## Features worth surfacing later
None as written (it's plumbing). *If* a future version wants to show the
host-bridge / memory-controller chipset name on the info screen, the PPC route is
OF property reads on the PCI node (`getprop "name"`/`"device_id"` etc.) via
`ofw.c`, **not** this file. Low priority; chipset line currently stays "Unknown".

## Open questions / TO VERIFY
- None about disposition. If chipset-ID display is ever desired: TO VERIFY the
  exact OF method names (`config-l@`?) on Apple New World OF vs. OpenBIOS.

## Build impact
None. Not added to `src/` or the Makefile; no new/changed call sites.
