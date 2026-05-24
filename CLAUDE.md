# Claude agent instructions for this project

## What this project is

**memtestppc+** is a standalone, bootable memory tester for PowerPC Macs that
reproduces the classic **memtest86+ v5.01** blue-screen TUI. It boots straight
into testing with **no operating system**, so the maximum fraction of RAM is
under test. It runs from an Open Firmware-loaded ELF (today: a QEMU `-cdrom` ISO,
and a partition-booted ELF on a real iBook G3). Versions run 0.01 → 1.00; "done"
is a faithful, real-hardware-verified port that a PowerPC Mac owner can boot and
trust.

Project brief: [`docs/PLAN.md`](docs/PLAN.md).

## Repo layout

```
README.md          — user-facing front door: what it is, how to run, status.
CLAUDE.md          — this file.
Makefile           — cross-compile + ISO-build + QEMU targets.
docs/
  PLAN.md          — project brief: mission, non-goals, strategy, roadmap.
  sessions/        — one dir per session, globally numbered. Read README.md first.
    NNN-<slug>/    — each contains a single HANDOFF.md.
  convos/          — raw conversation transcripts (managed externally; ignore).
src/               — the port itself: head.S, ofw.{c,h}, display.{c,h},
                     font_vga.h, memtest.h, test.c, main.c, linker.ld.
cd/                — CHRP bootinfo.txt for the bootable ISO.
ref/               — memtest86+ v5.01 source + reference screenshots/photos.
scripts/           — helpers.
test_minimal/      — throwaway OF-boot-protocol probes (from the session-002
                     r5/CD-boot bring-up). Safe to delete eventually.
```

## Sessions workflow

**Read [`docs/sessions/README.md`](docs/sessions/README.md) before starting
work — that's the workflow you must follow.** The short version: substantive work
lives in `docs/sessions/NNN-<slug>/`, and **each session writes one `HANDOFF.md`**
that serves as its running log, findings, and primer for the next session.

To get up to speed at the start of a session: this `CLAUDE.md` (loaded
automatically), the project `README.md`, and **the single most recent
`HANDOFF.md`**. Each HANDOFF stands on its own, so you don't normally need older
ones.

## Fleet hosts

- **uranium** — this Mac, the main dev box. Edit code, drive QEMU locally, and
  route files between the Linux boxes and the PPC hardware.
- **opti7050** (`ssh opti7050`) — primary build + QEMU host. Ubuntu 22.10,
  `powerpc-linux-gnu-gcc` 12.2.0, `qemu-system-ppc` 7.0.0 (mac99), `xorrisofs`.
  ISOs are built here.
- **opti9020** (`ssh opti9020`) — alternate Linux build host, newer toolchain.
  Occasionally offline; opti7050 is the default.
- **ibookg32** (`ssh ibookg32`) — primary **real hardware** test target. iBook G3
  (PowerBook4,1), 500 MHz PPC 750, 256 MB, Tiger 10.4.11, 8-bit ATI framebuffer.
  Boots the ELF from a second HFS+ partition via OF (`boot hd:5,memtestppc.elf`).
  **opti7050 can't SCP to it directly** (legacy ciphers) — route through uranium:
  `scp opti7050:file /tmp/ && scp /tmp/file ibookg32:dest`.
- **imacg3** (`ssh imacg3`) — secondary real hardware target (iMac G3, Rage 128).
  Frequently offline.

## Risk tolerance by host

- **uranium** — low. It's the daily driver; standard installs fine, no random
  binaries off the net.
- **opti7050 / opti9020** — medium. Reinstallable Linux scratch boxes; build and
  experiment freely, clean up after yourself.
- **PPC hardware (ibookg32, imacg3)** — high. Reinstallable test machines; the
  bar is "will this teach us something?" If one goes unresponsive it's usually
  sitting at the OF prompt or mid-test — it needs a physical reboot.

## Build & test

```sh
# 1. Sync source to the build host (excludes git/ref/artifacts/docs):
rsync -av --exclude '.git' --exclude 'ref/' --exclude '*.o' --exclude '*.elf' \
    --exclude '*.bin' --exclude '*.iso' --exclude 'docs/' --exclude 'testing/' \
    /Users/cell/claude/memtestppc+/ opti7050:/home/cell/memtestppc+/

# 2. Build the bootable ISO:
ssh opti7050 'cd /home/cell/memtestppc+ && make clean && make memtestppc.iso'

# 3. Test in QEMU (128MB+ required for OpenBIOS auto-boot; use 256):
qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d \
    -display vnc=:99 -monitor unix:/tmp/qemu-mon.sock,server,nowait &
#    Screenshot: echo "screendump /tmp/ss.ppm" | socat - UNIX-CONNECT:/tmp/qemu-mon.sock ; convert /tmp/ss.ppm ss.png

# 4. Deploy to real hardware (via the uranium hop):
scp opti7050:/home/cell/memtestppc+/memtestppc.elf /tmp/memtestppc.elf
scp /tmp/memtestppc.elf ibookg32:"/Volumes/Untitled 2/memtestppc.elf"
```

## README format

`README.md` carries a status table. Status column legend, in priority order:

- ✅ Working — verified by a green run (in QEMU or, better, on real hardware).
- 🟡 Partial — works with a caveat / unverified-on-hardware / one path only.
  Always add a one-line "what's missing" in Notes.
- ❌ Missing — known not to work, with a pointer to where it's scoped.

**Keep each Notes cell to one or two sentences, and keep the Status prose short.**
The common failure mode is the status section growing into a prose changelog and
the Notes cells swelling into paragraphs until nothing is scannable — push
blow-by-blow detail into the relevant session HANDOFF instead.

## Document everything

Default to capturing liberally in the active session's `HANDOFF.md`. Decisions,
dead ends, OF/hardware gotchas, ambiguity in the moment — all worth writing down.
Err on the side of MORE capture. The goal is that a future session can pick up the
thread without re-explaining.

## Unsupervised mode

When the user says "work unsupervised" (or similar), they're unreachable. Don't
stop to ask — make reasonable assumptions, run experiments (QEMU is cheap; real
hardware less so), try the obvious fixes. Long runtimes are fine. Only block for
genuinely irreversible actions. **Log every judgment call** in the active
session's HANDOFF — that's what the user reviews on return.

## Key invariants

1. **Port fidelity to memtest86+ v5.01.** Preserve the original code structure —
   function skeletons, variable names, layout constants — and replace only the
   x86-specific *leaf* code with OF/PPC implementations. Prefer commented-out
   original over deletion where the replacement is non-obvious. *Why:* makes the
   diff against upstream small, so upstream fixes are pullable, people who know
   memtest86+ can navigate, and correctness is verifiable against the original.
   This is the project's central discipline and the current active workstream.

2. **Boot from a CD *image* via the CHRP boot script — never QEMU `-kernel`.**
   *Why:* real Apple OF (and OpenBIOS) run with the MMU on; `-kernel` jumps to an
   address OF never mapped → instant ISI fault. OF's `load` from a device claims
   and maps memory correctly, and matches how real hardware boots.

3. **OF passes the client-interface pointer in `r5`** (not r3). `head.S` relies
   on this.

4. **With OF's MMU on, claim memory before testing it.** `discover_memory()`
   `ofw_claim`s in 1 MB chunks; the stack lives in BSS (which OF maps when it
   loads the ELF), not a hardcoded low address. Writing to unclaimed/unmapped
   physical addresses faults.

5. **Cross-compile only.** Tiger's gcc emits Mach-O; OF needs ELF, and Tiger has
   no ELF linker. Build with `powerpc-linux-gnu-gcc` on Linux. The deliverable is
   the CD image — build-from-source on a real Tiger Mac is *not* a supported path.

6. **No physical CD has been burned yet.** "CD boot" everywhere means a QEMU
   `-cdrom` ISO; the only real-hardware testing so far is partition boot on
   ibookg32. Burning + booting a physical disc is still an open to-do.
