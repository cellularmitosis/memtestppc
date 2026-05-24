# Session 002: CD Boot Breakthrough

> **Note:** This handoff was reconstructed retroactively (during session 005) from
> the full transcript at
> [docs/convos/claude-conversation-2026-05-23-c9a7979e.md](../../convos/claude-conversation-2026-05-23-c9a7979e.md).
> Sessions 001–003 were originally a single Claude Code session that hit
> context-compaction three times. See [001-inception-and-scaffold](../001-inception-and-scaffold/HANDOFF.md)
> for the prior phase.

## What happened

Picked up from [session 001](../001-inception-and-scaffold/HANDOFF.md) with a
compiling ELF that would not boot. This phase is about **how PowerPC Macs / Open
Firmware actually boot a bare-metal ELF**, and it produced the single most
load-bearing piece of knowledge in the project: **you must boot from a CD image
via a CHRP boot script — not via QEMU `-kernel`.**

> **No physical CD has ever been burned.** Everything here ("CD boot", "from the
> CD", etc.) means an `.iso` handed to QEMU as `-cdrom`. We have never written
> the image to a real disc or booted a real optical drive. Burning and testing a
> physical CD on real hardware is still an open to-do (the one real-hardware
> session, 004, used **partition boot** — an ELF on an HFS+ partition booted via
> OF `boot hd:5,...` — not a CD).

### Root cause: why `-kernel` fails (do not use it)

QEMU's `-kernel` always loaded our binary at 0x01000000 (regardless of link
address) and then took an **ISI (Instruction Storage Interrupt)** plus an
**HV_EMU** exception the instant it jumped there. An execution trace confirmed
the NIP never reached our code.

**Why:** OpenBIOS hands control to the kernel with the **MMU translation on**
(MSR IR/DR = 1), but the `-kernel` load path doesn't establish a mapping for the
load address. So the very first instruction fetch faults. Tried lower load
addresses, `OMAGIC`, `mac99` vs `g3beige`, the generic loader, and `-prom-env`
claim tricks — **all** dead ends. `-kernel` is fundamentally the wrong path.

### The fix: boot from a CD image

OF's `load` command **from a device** does the right thing — it reads the ELF
program headers, **claims and maps** the memory, then `go` runs it. This is also
exactly what real hardware needs, so it's not a QEMU workaround.

CD structure built:
- **HFS+ hybrid ISO** via `xorrisofs`.
- A **CHRP boot script** at `/ppc/chrp/bootinfo.txt`, blessed as the `tbxi` boot
  file, containing:
  ```
  <CHRP-BOOT>
  <BOOT-SCRIPT>
  load cd:,\memtestppc.elf
  go
  </BOOT-SCRIPT>
  </CHRP-BOOT>
  ```
- The project ELF (linked at 0x00400000) sits at the CD root.

(A subagent — see
[docs/convos/claude-conversation-2026-05-23-agent-af.md](../../convos/claude-conversation-2026-05-23-agent-af.md)
— researched the OF CD-boot format in parallel.)

### Breakthrough: the CIF pointer is in r5

Using a minimal test program booted from CD, we confirmed empirically that Open
Firmware passes the **client-interface function pointer in r5** (not r3, as some
docs suggest). The test printed **"r5 works!"** once booted via the CD path.
`head.S` already saved r5, so no change was needed there — but this nailed down a
previously uncertain assumption.

### First real boot

With the CD path, the **full memtestppc+ binary boots and runs in QEMU
`-nographic` mode**: OF calls succeed, debug output appears on serial, and the
test loop starts. `display_init()` correctly **fails gracefully** in nographic
mode (there's no framebuffer hardware) and falls back rather than hanging.

### Debug plumbing added

- `ofw_write()` / `ofw_puts()` in `ofw.c` for serial/OF-console debug output.
- A `test_minimal/` tree of tiny programs used to bisect the OF boot protocol
  (the r5 discovery came from here). Can be deleted once the project is stable.
- QEMU monitor interaction via a unix socket + `socat` for `screendump` and
  `sendkey` (the workflow used heavily in later sessions).

## Files changed (relative to session 001)

- `cd/bootinfo.txt` — **new** CHRP boot script.
- `Makefile` — **new** `memtestppc.iso` target (`xorrisofs` with HFS+ and the
  CHRP `tbxi` blessing); QEMU targets switched from `-kernel` to
  `-cdrom memtestppc.iso -boot d`; added `-N` to LDFLAGS to minimize ELF
  alignment padding.
- `src/ofw.c` / `src/ofw.h` — added `ofw_write()` / `ofw_puts()`.
- `test_minimal/` — **new**, throwaway OF-boot-protocol test programs.

## Build & test environment

- **Build host:** opti7050 (`ssh opti7050`), `powerpc-linux-gnu-gcc` 12.2.0.
- **QEMU:** `qemu-system-ppc` (mac99 machine), booting the ISO from CD.
- **Monitor interaction:** unix socket + `socat` for `screendump`; convert PPM →
  PNG to inspect.

```bash
# Build + ISO on opti7050:
ssh opti7050 'cd /home/cell/memtestppc+ && make clean && make memtestppc.iso'

# Boot from CD (NOT -kernel):
qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d -nographic
```

## Current state

memtestppc+ **boots from the CD image in QEMU (`-cdrom`, `-nographic` mode)** and
runs. OF client calls work, the CIF-in-r5 question is settled, and the test loop
executes. (Again: this is an ISO in QEMU — no physical disc exists.) **No
graphical output has been verified yet** — in nographic mode `display_init()`
fails by design, and an early attempt to boot with a QEMU VNC/graphical display
appeared to fail with *"No valid state has been set by load or init-program"*.

That graphical-boot question is exactly where [session 003](../003-graphical-boot/HANDOFF.md)
picks up (and finds it was non-reproducible at 256MB — really a `<128MB`
OpenBIOS-in-QEMU quirk, not a real bug).

## What to do next

1. **Verify graphical boot** in QEMU with a real display and confirm the
   framebuffer rendering + VGA font actually paint the blue TUI. (→ session 003.)
2. Debug OF framebuffer property discovery (address/width/height/depth/linebytes).
3. Clean up the `ofw_puts` debug output once rendering is confirmed.
4. Eventually delete `test_minimal/`.
