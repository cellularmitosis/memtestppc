# Claude Conversation Log

Session ID: 00aaed09-c20f-4096-8117-080f0241fe0d
Date: 2026-05-24 00:21:03

---

## 👤 User

to get up to speed, read docs/sessions/003-graphical-boot/HANDOFF.md and docs/sessions/004-real-hardware/HANDOFF.md

---

## 🤖 Claude

I'll read both handoff files to get up to speed.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/003-graphical-boot/HANDOFF.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/004-real-hardware/HANDOFF.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/projects/-Users-cell-claude-memtestppc-/memory/project-ibookg32-testing.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/projects/-Users-cell-claude-memtestppc-/memory/feedback-port-fidelity.md"
}


---

## 🤖 Claude

I'm up to speed on both sessions. Let me check the current repo state and the reference source.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "git log --oneline -10 && echo \"---STATUS---\" && git status && echo \"---TREE---\" && ls -la",
  "description": "Check git state and top-level layout"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"---SRC---\" && ls -la src/ 2>/dev/null && echo \"---REF---\" && ls -la ref/ 2>/dev/null && echo \"---DOCS---\" && ls -la docs/sessions/ 2>/dev/null",
  "description": "List src, ref, and docs/sessions"
}


---

## 🤖 Claude

Let me examine the reference source structure and the current ported files.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "ls -la ref/memtest86+-5.01/ | head -80",
  "description": "List reference memtest86+ 5.01 source"
}


---

## 🤖 Claude

I'm fully up to speed. The headline next-step from session 004 is the **pivot to a true port** of memtest86+ v5.01 — and that handoff explicitly says to "discuss details at session start." Let me look at the current ported code vs. the reference before I propose a restructuring plan.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/main.c"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/memtest.h"
}


---

## 🤖 Claude

Now let me compare against the reference to see what diverged. Let me read the reference's main.c, test.c, error.c, and lib.c (which has the display primitives).

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/test.h"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/test.c"
}


---

## 🤖 Claude

The `test.c` is already a faithful port (just dropped the `cpu` param and x86 asm). Now let me see where `main.c`, error reporting, and the display primitives diverge from upstream — the handoff flagged these as the rolled-our-own parts.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/main.c"
}


---

## 🤖 Claude

The reference `main.c` is heavy with x86 SMP, relocation, and windowing machinery that the port deliberately simplified. The flagged divergence areas are display and error reporting. Let me read the reference `error.c` and `lib.c` (display primitives) to scope what a faithful port of those would look like.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/error.c"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/display.c"
}


---

## 👤 User

[Request interrupted by user]

---

## 👤 User

who, sorry, I actually another task in mind before we start the next session.

somehow claude started at session 003.

I have a full transcript of everything which led up to the creation of the 003 HANDOFF.md at docs/convos/claude-conversation-2026-05-23-c9a7979e.md

can you read that and try to figure out why we ended up with 003 as the first session?

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "ls -la docs/convos/ 2>/dev/null && echo \"---\" && wc -l docs/convos/claude-conversation-2026-05-23-c9a7979e.md 2>/dev/null",
  "description": "Check the convos dir and transcript size"
}


---

## 🤖 Claude

The transcript is large (7600 lines). Let me search it for where session numbering gets decided rather than read it all linearly.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd docs/convos && grep -n -i -E \"session ?(00)?[0-9]|001|002|003|HANDOFF|sessions/\" claude-conversation-2026-05-23-c9a7979e.md | head -80",
  "description": "Find session-numbering references in transcript"
}


---

## 🤖 Claude

It's a JSON transcript and source code is polluting the matches. Let me narrow to the actual session-numbering decision.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd docs/convos && grep -n -i -E \"003-graphical|002-|001-|sessions/00|session 00|next session|session number|number this|call this session|which session|start.{0,15}session\" claude-conversation-2026-05-23-c9a7979e.md | head -60",
  "description": "Find specific session-numbering text"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -n -i -E \"003-graphical|002-|001-|sessions/00|session 00|next session|session number|call this session|which session\" claude-conversation-2026-05-23-c9a7979e.md | head -60",
  "description": "Find specific session-numbering text"
}


---

## 🤖 Claude

Found the moment: at line 7199 the user asked for a handoff, and the assistant immediately did `mkdir .../003-graphical-boot` and wrote that "this session picked up from session 002." So the assistant chose "003" itself. Now I need to find *where that number came from* — was there a real 001/002, or did the assistant infer it? Let me look at how this session started.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "head -50 claude-conversation-2026-05-23-c9a7979e.md",
  "description": "Read transcript header/start"
}


---

## 🤖 Claude

This is the smoking gun — this transcript **starts the entire project** ("ok, new project"). It's the first session, yet it ends by creating a `003-graphical-boot` handoff. Let me confirm there's no real 001/002 by checking every `mkdir` and session reference across the transcript.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== all mkdir commands ===\" && grep -n -i \"mkdir\" claude-conversation-2026-05-23-c9a7979e.md; echo \"=== 'session 001' / 'session 002' / 'session 1' / 'session 2' mentions ===\" && grep -n -i -E \"session 0?0?[12]\\b|first session|second session|three session|3 session\" claude-conversation-2026-05-23-c9a7979e.md | head -40",
  "description": "Find all mkdir and early-session references"
}


---

## 👤 User

[Request interrupted by user]

---

## 👤 User

can you read the convo md in full?  part of this task will be to see if we need to capture anything else out of it (i.e. content for a 001 and 002 session) or just rename 003 and 004 to 001 and 002

---

## 🤖 Claude

Reading the full transcript now to see what work happened in this one mega-session — whether it naturally divides into earlier "sessions" worth capturing, or whether 003/004 should just be renumbered.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/convos/claude-conversation-2026-05-23-c9a7979e.md",
  "limit": 1100
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/convos/claude-conversation-2026-05-23-c9a7979e.md",
  "limit": 750
}


---

## 🤖 Claude

Let me find all the chapter markers — they'll reveal the natural phase boundaries within this mega-session.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -n -A3 \"mark_chapter\" claude-conversation-2026-05-23-c9a7979e.md | grep -E \"title|summary\" ",
  "description": "Extract all chapter titles and summaries"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/docs/convos && grep -n -A4 \"mark_chapter\" claude-conversation-2026-05-23-c9a7979e.md | grep -E '\"title\"|\"summary\"'",
  "description": "Extract all chapter titles and summaries"
}


---

## 🤖 Claude

The structure is now clear. This one session (c9a7979e) has six phases: Research/planning → Core scaffold → Build environment pivot → Bootable CD creation → First graphical boot → (003 handoff). It went from "ok, new project" all the way to first graphical boot, yet labeled its handoff "003." Let me read the full transcript to assess what early-phase content exists that the 003 handoff doesn't capture. Continuing.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/convos/claude-conversation-2026-05-23-c9a7979e.md",
  "offset": 750,
  "limit": 750
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/convos/claude-conversation-2026-05-23-c9a7979e.md",
  "offset": 750,
  "limit": 400
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/convos/claude-conversation-2026-05-23-c9a7979e.md",
  "offset": 750,
  "limit": 200
}


---

## 🤖 Claude

This file is mostly tool-call JSON with embedded source. Rather than read all 7600 lines mechanically, let me extract the human-readable narrative — user messages and Claude's prose (skipping tool-call blocks) — which is where decisions and reasoning live.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "awk '\nBEGIN{RS=\"\\n---\\n\"}\n{\n  block=$0\n  # skip pure tool-call blocks\n  if (block ~ /\ud83d\udd27 Using tool/) next\n  if (block ~ /\ud83d\udee0\ufe0f Tool Result/) next\n  # trim leading/trailing whitespace\n  gsub(/^[ \\n]+|[ \\n]+$/,\"\",block)\n  if (block==\"\") next\n  print block\n  print \"\\n========================================\\n\"\n}' claude-conversation-2026-05-23-c9a7979e.md | head -250",
  "description": "Extract prose narrative, skipping tool blocks (part 1)"
}


---

## 🤖 Claude

The extraction works. Let me continue through the rest of the narrative, skipping the embedded skill dump.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "awk '\nBEGIN{RS=\"\\n---\\n\"}\n{\n  block=$0\n  if (block ~ /\ud83d\udd27 Using tool/) next\n  if (block ~ /\ud83d\udee0\ufe0f Tool Result/) next\n  # skip the imacg3-dev skill dump block\n  if (block ~ /Base directory for this skill/) next\n  gsub(/^[ \\n]+|[ \\n]+$/,\"\",block)\n  if (block==\"\") next\n  print block\n  print \"\\n===\\n\"\n}' claude-conversation-2026-05-23-c9a7979e.md > /tmp/prose.txt\nwc -l /tmp/prose.txt\necho \"=== SHOWING LINES 1-200 of prose (after skill) ===\"\nsed -n '120,330p' /tmp/prose.txt",
  "description": "Save prose extraction and view middle section"
}


---

## 🤖 Claude

There's an embedded context-compaction summary (the original session itself ran out of context once). Good — that confirms how long this session was. Let me continue through the prose.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "sed -n '330,560p' /tmp/prose.txt",
  "description": "View prose lines 330-560"
}


---

## 🤖 Claude

The compaction summary is rich. Let me continue through the remaining prose — the build completion, CD creation, and graphical boot phases.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "sed -n '560,820p' /tmp/prose.txt",
  "description": "View prose lines 560-820"
}


---

## 🤖 Claude

Excellent detail on the CD boot breakthrough. Let me read the final portion through to the handoff.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "sed -n '820,1100p' /tmp/prose.txt",
  "description": "View prose lines 820-1100"
}


---

## 🤖 Claude

Two compactions occurred within this single session. Let me read the final stretch covering the graphical boot success through the handoff.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "sed -n '1100,1400p' /tmp/prose.txt",
  "description": "View prose lines 1100-1400"
}


---

## 🤖 Claude

Let me read the final stretch to see the session closeout.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "sed -n '1400,1700p' /tmp/prose.txt",
  "description": "View final prose section"
}


---

## 🤖 Claude

Let me read the final closeout — the handoff creation moment and any final exchange.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "sed -n '1700,2136p' /tmp/prose.txt",
  "description": "View final prose to end"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== context compactions inside this one session ===\"; grep -c \"This session is being continued from a previous conversation\" claude-conversation-2026-05-23-c9a7979e.md; echo \"=== identify the other transcripts ===\"; for f in claude-conversation-2026-05-23-f076ac30.md claude-conversation-2026-05-23-agent-a1.md claude-conversation-2026-05-23-agent-af.md; do echo \"--- $f ---\"; sed -n '1,12p' \"$f\" | grep -iE \"Session ID|Date|^# |new project|real hardware|ibook|handoff\" | head -4; done",
  "description": "Count compactions and identify sibling transcripts"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "for f in f076ac30 agent-a1 agent-af; do echo \"=== $f ===\"; head -15 \"claude-conversation-2026-05-23-$f.md\"; echo; done",
  "description": "Inspect sibling transcript headers"
}


---

## 🤖 Claude

I've now read the full transcript and cross-checked the sibling transcripts and git history. Here's what happened.

## Why we ended up with "003" as the first session

**There was only ever one prior session before this numbering got created — not three.** The transcript `claude-conversation-2026-05-23-c9a7979e.md` is a single Claude Code session that started the project from absolute scratch ("ok, new project") and ran all the way to the first graphical boot. The tell is at the very top — it opens with your project brief and me reading `ref/` for the first time.

The reason it *looks* like session 3: **that one session hit context-compaction three times internally.** I can see three separate `"This session is being continued from a previous conversation that ran out of context"` summary blocks embedded in it. From my vantage point at the end of that session, I had been "continued from a previous conversation" several times, so I incorrectly modeled the work as a *chain of separate sessions* and mapped the phases onto invented numbers:

- phantom "session 001" ≈ inception + scaffold + build-toolchain saga
- phantom "session 002" ≈ CD-boot breakthrough + nographic run
- "session 003" ≈ the graphical-boot/TUI-polish stretch I still had in clear context

When you asked for a handoff, I only wrote one — for the slice I remembered well — and labeled it `003-graphical-boot`, with prose asserting it "picked up from session 002." There is no 001 or 002 anywhere; git confirms it (`initial import, project already in-progress` → `session 003 end`).

**So the real session ledger is:**
| transcript | what it is | got labeled |
|---|---|---|
| `c9a7979e` (19:51) | project inception → first graphical boot (3 internal compactions) | **003** |
| `f076ac30` (22:52) | starts by reading the 003 handoff; real-hardware iBook G3 work | **004** |
| `agent-af`, `agent-a1` | subagents (CD-boot research; iBook 8-bit CLUT research) | — |

Two real working sessions, numbered 003 and 004. The numbering is inflated by exactly 2.

## Do we need to capture 001/002 content, or just rename?

There **is** genuinely valuable early-phase content in `c9a7979e` that *no* handoff captures — the 003 doc assumes a working "session 002" state without ever documenting how it got there:

- **Architecture decisions**: the VGA-text-buffer→framebuffer abstraction (the thing that let us reuse the original TUI code), hybrid OF-client approach, font sourced from the Linux kernel's `font_8x16.c`.
- **Build-toolchain saga**: your ask to build natively on ibookg32 → discovery that Tiger gcc emits Mach-O not ELF → decision to use the `powerpc-linux-gnu` cross-compiler; the `-Wa,-mregnames` fix, `__udivdi3`/libgcc, `-N`.
- **The CD-boot breakthrough** (the most load-bearing knowledge in the whole project): CIF pointer arrives in **r5**, not r3; QEMU `-kernel` triggers ISI exceptions because OpenBIOS runs with the MMU on and never maps the kernel — so you *must* boot from a CHRP-scripted HFS+ hybrid ISO.

None of that is in a session doc. It's recoverable from the code + git + the transcript itself (now preserved under `docs/convos/`), but a transcript is 7,600 lines; a handoff is digestible.

My recommendation is **not a pure rename**, because that throws away the chance to capture the above. But I'd also avoid fabricating two separate phantom docs for work that genuinely happened in one session. Cleanest options:

1. **Renumber to reflect reality (recommended):** `003-graphical-boot` → `001`, `004-real-hardware` → `002`; fix the internal cross-references ("session 002/003" mentions); and fold the inception/build/CD-boot findings into the new `001` handoff as a "How we got here" background section. Honest about session count, captures the knowledge.
2. **Backfill phantoms:** write fresh `001-inception-and-scaffold` and `002-cd-boot` from the transcript, leave `003`/`004` as-is. Cross-references stay valid untouched, but it enshrines a 4-session history that didn't really happen.
3. **Just rename** `003→001`, `004→002`, fix cross-refs, capture nothing extra. Fastest; relies on the transcript for the early findings.

Which do you want? I lean option 1. (Whichever you pick, note the next session's `f076ac30` work — the port-fidelity pivot — is still the open thread once this cleanup is done.)

---

## 👤 User

yes, I'd like to capture everything, I'd lean towards 2. 

actually, I just realized I forgot to set up a CLAUDE.md for this project.  I realize now that my ~/.claude/CLAUDE.md is how the prior claude session even knew to follow a sessions/NNN-slug/ pattern.  I've added a bit more content there, but we'll create a project-level CLAUDE.md  etc after you finish with this retroactive patch-up.

---

## 🤖 Claude

Got it — option 2: backfill `001` and `002` from the transcript, leave `003`/`004` untouched so their cross-references stay valid. I have all the technical detail from the full read. Let me reconstruct the two handoffs to match the established style, with a provenance note since they're written after the fact.

The natural split (matching both the chapter markers and the three internal compaction boundaries):
- **001** — inception, architecture, build-toolchain saga, full scaffold, first successful compile
- **002** — the CD-boot breakthrough (r5, MMU/ISI, CHRP ISO), first boot from CD in nographic mode

This makes 003's opening line ("picked up from session 002 where the CD boot worked in `-nographic` mode but failed in QEMU VNC/graphical mode") land correctly.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/docs/sessions && mkdir -p 001-inception-and-scaffold 002-cd-boot && ls -la",
  "description": "Create the two backfilled session directories"
}


---

## 🤖 Claude

Now the 001 handoff:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/001-inception-and-scaffold/HANDOFF.md",
  "content": "# Session 001: Inception & Scaffold\n\n> **Note:** This handoff was reconstructed retroactively (during session 005) from\n> the full transcript at\n> [docs/convos/claude-conversation-2026-05-23-c9a7979e.md](../../convos/claude-conversation-2026-05-23-c9a7979e.md).\n> Sessions 001, 002, and 003 were originally a *single* Claude Code session that\n> hit context-compaction three times; at the end it wrote only one handoff and\n> labelled it \"003\". This document and [002-cd-boot](../002-cd-boot/HANDOFF.md)\n> backfill the earlier phases of that session so the numbering reflects the work.\n\n## What happened\n\nThe project was created from scratch. This phase covers understanding the goal,\nchoosing an architecture, settling the build toolchain, writing the complete\nsource scaffold, and getting it to compile to a PowerPC ELF for the first time.\n\n### The goal (from the user's brief)\n\nBuild **memtestppc+** \u2014 a standalone, bootable memory tester for PowerPC Macs\nthat faithfully reproduces the classic **memtest86+ v5.01** blue-screen TUI. It\nboots from CD-ROM straight into testing with **no operating system**, so the\nmaximum fraction of RAM is under test. Start at version **0.01** and iterate to\n**1.00**. Use the **IBM VGA 8x16 font** (codepage 437) for pixel-perfect\nauthenticity. **Leverage as much of the original memtest86+ TUI code as\npossible** \u2014 ideally replacing its BIOS/VGA leaf calls with our own rendering\nroutines rather than rewriting the display layer. Reference material (full\nmemtest86+ v5.01 source, screenshots, photos) is in `ref/`.\n\n### Architecture decided: hybrid Open Firmware client\n\n- Open Firmware (IEEE 1275) loads our ELF from the CD.\n- We use the OF **client interface** (`finddevice`, `getprop`, `open`, `read`,\n  `claim`, `exit`) to discover memory layout, framebuffer, and timebase.\n- We render the TUI to the OF framebuffer using an **embedded 8x16 VGA font**.\n- **Key trick that preserves the original code:** we keep a virtual VGA text\n  buffer `vga_buf[80*25*2]` (char+attribute pairs, exactly like x86 text mode)\n  and `#define SCREEN_ADR ((unsigned long)vga_buf)`. The original memtest86+\n  `cprint`/`dprint`/`hprint`/`aprint`/`scroll` functions then work almost\n  verbatim \u2014 they write to the buffer, and we render changed cells to the\n  framebuffer with the font.\n\nx86 \u2192 PowerPC substitutions:\n- `CPUID` \u2192 **PVR** (Processor Version Register) for CPU identification.\n- `RDTSC` \u2192 **Timebase Register** (`mftb`/`mftbu`); frequency from OF device tree.\n- E820 BIOS memory map \u2192 OF **`/memory`** node `reg` property.\n- VGA text mode at `0xB8000` \u2192 OF framebuffer + our font renderer.\n- **No SMP** \u2014 the target G3 iMac is single-core, so all the SMP/relocation/\n  windowing machinery in the original `main.c` is dropped.\n\n### Build-toolchain saga (this ate real time \u2014 read before re-litigating it)\n\n1. The local Mac has **no** PPC cross-compiler and no Docker.\n2. **opti9020** (Debian 13) has `gcc-powerpc-linux-gnu` (14.2.0),\n   `qemu-system-ppc` (10.0.8), and `genisoimage`. Installed the cross-compiler\n   there first.\n3. **User interrupted** to ask that we build with the **native system gcc on\n   ibookg32** instead, because that's what a fellow Tiger enthusiast with an\n   Xcode 2.5 install would have.\n4. **Blocker discovered:** Tiger's gcc emits **Mach-O**, but Open Firmware needs\n   **ELF** (or XCOFF). Tiger has no `objcopy`/ELF linker to bridge this.\n5. User's response: *\"do whatever you think is best. Most users will just\n   download the CD image artifact anyway, rather than attempting to build from\n   source.\"* \u2192 We went with the cross-compiler. **Building from source on a real\n   Tiger Mac is not a supported path; the deliverable is the CD image.**\n6. opti9020 then went unreachable mid-session, so we pivoted to **opti7050**\n   (Ubuntu 22.10), installed `powerpc-linux-gnu-gcc` **12.2.0**, and that became\n   the build host for the rest of the project.\n\n### Font sourcing\n\nThe IBM VGA 8x16 OTB from int10h.org needed parsing, so instead we took the\nidentical bitmap from the **Linux kernel's `lib/fonts/font_8x16.c`** (GPLv2-\ncompatible), extracted the raw bytes, and generated\n`src/font_vga.h` \u2014 `static const unsigned char font_vga_8x16[4096]` (256 chars \u00d7\n16 rows \u00d7 8 px, MSB = leftmost pixel). Verified the 'A' glyph by eye.\n\n### Source scaffold written\n\nAll of `src/` was created this phase: `head.S`, `ofw.c`/`ofw.h`,\n`display.c`/`display.h`, `font_vga.h`, `memtest.h`, `test.c`, `main.c`,\n`linker.ld`, plus the `Makefile`.\n\n- `test.c` is a close port of the original `test.c`: same function skeletons\n  (`addr_tst1`, `addr_tst2`, `movinv1`, `movinv32`, `movinvr`, `modtst`,\n  `block_move`, `bit_fade_fill`/`chk`), with x86 inline asm replaced by portable\n  C and the `cpu` parameter dropped (single-CPU). Marsaglia MWC RNG.\n- `linker.ld`: ELF32 big-endian PowerPC, entry `_start`, **load address\n  0x00400000 (4MB)** \u2014 leaving the low memory for OF.\n\n### Build fixes to reach first successful compile\n\n- **head.S immediate overflow:** `addi r1,r1,-0x10000` exceeds the 16-bit signed\n  immediate. Switched the stack setup to `lis r1, 0x003F` (stack at 0x003F0000).\n- **`@ha`/`@l` relocations with named registers fail** unless the assembler gets\n  `-Wa,-mregnames`. Without it, `lis r3, foo@ha` treats `r3` as a *symbol*, not a\n  register. (Numeric `lis 3, foo@ha` also works.) Added `-Wa,-mregnames` to\n  CFLAGS.\n- **CIF pointer storage:** a plain C global `ofw_cif` couldn't be referenced\n  PC-relatively from `head.S`. Renamed to `_ofw_cif_store`, defined in head.S's\n  `.data`, declared `extern` in C.\n- **C89 cleanups:** moved mid-block declarations to block tops; added\n  `#include \"memtest.h\"` to display.c; added `rand_seed`/`rand_next` prototypes;\n  forward-declared `check_key()`.\n- **`__udivdi3` undefined at link:** 64-bit division in `-nostdlib` freestanding\n  code needs libgcc. Added\n  `LIBGCC = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)` and link it.\n\n## Files created\n\n- `src/head.S` \u2014 PPC entry; saves OF CIF pointer; sets stack (0x003F0000);\n  clears BSS; calls `main()`.\n- `src/ofw.c` / `src/ofw.h` \u2014 OF client interface wrappers.\n- `src/display.c` / `src/display.h` \u2014 virtual VGA text buffer + framebuffer font\n  renderer; the ported memtest86+ print functions.\n- `src/font_vga.h` \u2014 IBM VGA 8x16 font (4096 bytes).\n- `src/memtest.h` \u2014 core defs/structs/layout constants, PPC inline-asm helpers.\n- `src/test.c` \u2014 the memory test algorithms, ported to portable C.\n- `src/main.c` \u2014 entry, PVR-based CPU id, OF memory discovery, `draw_screen()`\n  TUI layout, the test scheduling loop.\n- `src/linker.ld` \u2014 ELF32-PPC, load 0x00400000.\n- `Makefile` \u2014 cross-compile, `-Wa,-mregnames`, libgcc link.\n\n## Build & test environment\n\n- **Cross-compiler / build host:** opti7050 (`ssh opti7050`), Ubuntu 22.10,\n  `powerpc-linux-gnu-gcc` 12.2.0. (opti9020 was the first choice but went\n  offline; it has a newer toolchain if ever needed again.)\n- **CFLAGS:** `-Wall -O1 -ffreestanding -fno-builtin -nostdlib -mcpu=750\n  -mbig-endian -fno-pic -fno-pie -mno-toc -fno-stack-protector -Wa,-mregnames -I src`\n- Native Tiger builds on ibookg32 are **not** supported (Mach-O vs ELF).\n\n## Current state\n\nCompiles cleanly to a **~26KB ELF32 MSB PowerPC** binary. **It does not boot\nyet.** QEMU's `-kernel` path jumps to 0x01000000 and immediately takes an ISI\n(Instruction Storage Interrupt) \u2014 OpenBIOS runs with the MMU on and never maps\nthe kernel's address. Resolving that is the entire subject of session 002.\n\n## What to do next\n\n1. **Get it to actually boot** \u2014 figure out why `-kernel` ISI-faults and find a\n   boot path that respects OF's memory mapping. (\u2192 session 002 solves this via\n   CD boot.)\n2. Confirm which register OF passes the client-interface pointer in.\n3. Once booting, exercise the OF framebuffer / memory / timebase discovery.\n"
}


---

## 🤖 Claude

Now the 002 handoff:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/002-cd-boot/HANDOFF.md",
  "content": "# Session 002: CD Boot Breakthrough\n\n> **Note:** This handoff was reconstructed retroactively (during session 005) from\n> the full transcript at\n> [docs/convos/claude-conversation-2026-05-23-c9a7979e.md](../../convos/claude-conversation-2026-05-23-c9a7979e.md).\n> Sessions 001\u2013003 were originally a single Claude Code session that hit\n> context-compaction three times. See [001-inception-and-scaffold](../001-inception-and-scaffold/HANDOFF.md)\n> for the prior phase.\n\n## What happened\n\nPicked up from [session 001](../001-inception-and-scaffold/HANDOFF.md) with a\ncompiling ELF that would not boot. This phase is about **how PowerPC Macs / Open\nFirmware actually boot a bare-metal ELF**, and it produced the single most\nload-bearing piece of knowledge in the project: **you must boot from a CD image\nvia a CHRP boot script \u2014 not via QEMU `-kernel`.**\n\n### Root cause: why `-kernel` fails (do not use it)\n\nQEMU's `-kernel` always loaded our binary at 0x01000000 (regardless of link\naddress) and then took an **ISI (Instruction Storage Interrupt)** plus an\n**HV_EMU** exception the instant it jumped there. An execution trace confirmed\nthe NIP never reached our code.\n\n**Why:** OpenBIOS hands control to the kernel with the **MMU translation on**\n(MSR IR/DR = 1), but the `-kernel` load path doesn't establish a mapping for the\nload address. So the very first instruction fetch faults. Tried lower load\naddresses, `OMAGIC`, `mac99` vs `g3beige`, the generic loader, and `-prom-env`\nclaim tricks \u2014 **all** dead ends. `-kernel` is fundamentally the wrong path.\n\n### The fix: boot from a CD image\n\nOF's `load` command **from a device** does the right thing \u2014 it reads the ELF\nprogram headers, **claims and maps** the memory, then `go` runs it. This is also\nexactly what real hardware needs, so it's not a QEMU workaround.\n\nCD structure built:\n- **HFS+ hybrid ISO** via `xorrisofs`.\n- A **CHRP boot script** at `/ppc/chrp/bootinfo.txt`, blessed as the `tbxi` boot\n  file, containing:\n  ```\n  <CHRP-BOOT>\n  <BOOT-SCRIPT>\n  load cd:,\\memtestppc.elf\n  go\n  </BOOT-SCRIPT>\n  </CHRP-BOOT>\n  ```\n- The project ELF (linked at 0x00400000) sits at the CD root.\n\n(A subagent \u2014 see\n[docs/convos/claude-conversation-2026-05-23-agent-af.md](../../convos/claude-conversation-2026-05-23-agent-af.md)\n\u2014 researched the OF CD-boot format in parallel.)\n\n### Breakthrough: the CIF pointer is in r5\n\nUsing a minimal test program booted from CD, we confirmed empirically that Open\nFirmware passes the **client-interface function pointer in r5** (not r3, as some\ndocs suggest). The test printed **\"r5 works!\"** once booted via the CD path.\n`head.S` already saved r5, so no change was needed there \u2014 but this nailed down a\npreviously uncertain assumption.\n\n### First real boot\n\nWith the CD path, the **full memtestppc+ binary boots and runs in QEMU\n`-nographic` mode**: OF calls succeed, debug output appears on serial, and the\ntest loop starts. `display_init()` correctly **fails gracefully** in nographic\nmode (there's no framebuffer hardware) and falls back rather than hanging.\n\n### Debug plumbing added\n\n- `ofw_write()` / `ofw_puts()` in `ofw.c` for serial/OF-console debug output.\n- A `test_minimal/` tree of tiny programs used to bisect the OF boot protocol\n  (the r5 discovery came from here). Can be deleted once the project is stable.\n- QEMU monitor interaction via a unix socket + `socat` for `screendump` and\n  `sendkey` (the workflow used heavily in later sessions).\n\n## Files changed (relative to session 001)\n\n- `cd/bootinfo.txt` \u2014 **new** CHRP boot script.\n- `Makefile` \u2014 **new** `memtestppc.iso` target (`xorrisofs` with HFS+ and the\n  CHRP `tbxi` blessing); QEMU targets switched from `-kernel` to\n  `-cdrom memtestppc.iso -boot d`; added `-N` to LDFLAGS to minimize ELF\n  alignment padding.\n- `src/ofw.c` / `src/ofw.h` \u2014 added `ofw_write()` / `ofw_puts()`.\n- `test_minimal/` \u2014 **new**, throwaway OF-boot-protocol test programs.\n\n## Build & test environment\n\n- **Build host:** opti7050 (`ssh opti7050`), `powerpc-linux-gnu-gcc` 12.2.0.\n- **QEMU:** `qemu-system-ppc` (mac99 machine), booting the ISO from CD.\n- **Monitor interaction:** unix socket + `socat` for `screendump`; convert PPM \u2192\n  PNG to inspect.\n\n```bash\n# Build + ISO on opti7050:\nssh opti7050 'cd /home/cell/memtestppc+ && make clean && make memtestppc.iso'\n\n# Boot from CD (NOT -kernel):\nqemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d -nographic\n```\n\n## Current state\n\nmemtestppc+ **boots from CD and runs in QEMU `-nographic` mode**. OF client calls\nwork, the CIF-in-r5 question is settled, and the test loop executes. **No\ngraphical output has been verified yet** \u2014 in nographic mode `display_init()`\nfails by design, and an early attempt to boot with a QEMU VNC/graphical display\nappeared to fail with *\"No valid state has been set by load or init-program\"*.\n\nThat graphical-boot question is exactly where [session 003](../003-graphical-boot/HANDOFF.md)\npicks up (and finds it was non-reproducible at 256MB \u2014 really a `<128MB`\nOpenBIOS-in-QEMU quirk, not a real bug).\n\n## What to do next\n\n1. **Verify graphical boot** in QEMU with a real display and confirm the\n   framebuffer rendering + VGA font actually paint the blue TUI. (\u2192 session 003.)\n2. Debug OF framebuffer property discovery (address/width/height/depth/linebytes).\n3. Clean up the `ofw_puts` debug output once rendering is confirmed.\n4. Eventually delete `test_minimal/`.\n"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "for d in */; do echo \"$d -> $(ls \"$d\")\"; done",
  "description": "Confirm final sessions layout"
}


---

## 🤖 Claude

Done. The backfill is complete — `docs/sessions/` now reads as a coherent 1→4 chronology.

**What I created (both reconstructed from the transcript, with a provenance note at the top of each so future readers know they were written after the fact):**

- [001-inception-and-scaffold/HANDOFF.md](docs/sessions/001-inception-and-scaffold/HANDOFF.md) — the project brief, the hybrid-OF-client architecture (incl. the virtual-VGA-text-buffer trick that lets us reuse the original print functions), the x86→PPC substitutions, the **build-toolchain saga** (ibookg32-native → Mach-O-vs-ELF blocker → cross-compiler on opti7050; opti9020 went offline), font sourcing, the full scaffold, and the compile fixes (`-Wa,-mregnames`, libgcc/`__udivdi3`). Ends at: compiles to a 26KB ELF, doesn't boot yet.
- [002-cd-boot/HANDOFF.md](docs/sessions/002-cd-boot/HANDOFF.md) — the **`-kernel` ISI/MMU dead-end**, the pivot to **CD boot via a CHRP boot script + HFS+ hybrid ISO**, the **CIF-pointer-is-in-r5** discovery, and first successful boot in `-nographic` mode. Ends exactly where 003 says it inherited from: "CD boot worked in `-nographic` mode but [graphical boot was untested/appeared to fail]."

**Cross-references are intact:** 003 and 004 were left untouched, and their "relative to session 002/003" framing now points at real documents. I also linked the new docs to the preserved transcripts under `docs/convos/` (including the `agent-af` CD-boot research subagent).

**Root-cause recap for the record:** the "003" label came from a single session compacting three times — each compaction read as a "previous conversation," so I modeled one session as three and only wrote a handoff for the last slice. The real ledger is two working sessions (`c9a7979e`→003, `f076ac30`→004); the numbering was inflated by 2. The backfill resolves it by making the phantom phases real.

Ready when you are to set up the project-level CLAUDE.md — and note the actual open work thread is still the **port-fidelity pivot** from session 004 (restructure toward a closer memtest86+ v5.01 port).

---

## 👤 User

let's also make it clear that we haven't yet created a physical CD yet, that testing was all done using QEMU

---

## 🤖 Claude

Good call — "CD boot" in those docs could easily be misread as a burned disc. The ISO has only ever been booted as a QEMU `-cdrom` image; no physical CD has been created, and the one real-hardware session (004) used *partition* boot, not CD. Let me make that explicit in the 002 handoff where the ambiguity lives.

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/002-cd-boot/HANDOFF.md",
  "old_string": "Picked up from [session 001](../001-inception-and-scaffold/HANDOFF.md) with a\ncompiling ELF that would not boot. This phase is about **how PowerPC Macs / Open\nFirmware actually boot a bare-metal ELF**, and it produced the single most\nload-bearing piece of knowledge in the project: **you must boot from a CD image\nvia a CHRP boot script \u2014 not via QEMU `-kernel`.**",
  "new_string": "Picked up from [session 001](../001-inception-and-scaffold/HANDOFF.md) with a\ncompiling ELF that would not boot. This phase is about **how PowerPC Macs / Open\nFirmware actually boot a bare-metal ELF**, and it produced the single most\nload-bearing piece of knowledge in the project: **you must boot from a CD image\nvia a CHRP boot script \u2014 not via QEMU `-kernel`.**\n\n> **No physical CD has ever been burned.** Everything here (\"CD boot\", \"from the\n> CD\", etc.) means an `.iso` handed to QEMU as `-cdrom`. We have never written\n> the image to a real disc or booted a real optical drive. Burning and testing a\n> physical CD on real hardware is still an open to-do (the one real-hardware\n> session, 004, used **partition boot** \u2014 an ELF on an HFS+ partition booted via\n> OF `boot hd:5,...` \u2014 not a CD)."
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/002-cd-boot/HANDOFF.md",
  "old_string": "memtestppc+ **boots from CD and runs in QEMU `-nographic` mode**. OF client calls\nwork, the CIF-in-r5 question is settled, and the test loop executes.",
  "new_string": "memtestppc+ **boots from the CD image in QEMU (`-cdrom`, `-nographic` mode)** and\nruns. OF client calls work, the CIF-in-r5 question is settled, and the test loop\nexecutes. (Again: this is an ISO in QEMU \u2014 no physical disc exists.)"
}


---

## 🤖 Claude

Done. The 002 handoff now states it twice, unambiguously:

- A callout right under "What happened": **no physical CD has ever been burned** — "CD boot" everywhere means an `.iso` handed to QEMU as `-cdrom`. It also notes that the real-hardware session (004) used partition boot, not a disc.
- The "Current state" line now reads "boots from the CD image in QEMU (`-cdrom`, `-nographic`)" with a parenthetical reminder that no physical disc exists.

The other docs were already consistent with this: 003 lists "burn the ISO to CD" as future work, and 004 explicitly documents partition boot (`boot hd:5,...`) with "test CD boot on real hardware / burn `memtestppc.iso`" still in its to-do list — so burning + testing a physical disc remains a tracked open item across the chronology.

Ready to move on to the project-level CLAUDE.md whenever you are.

---

## 👤 User

great, take a look at a few sibling projects, at the files ../<project>/CLAUDE.md, ../<project>/docs/sessions/README.md and maybe a few ../<project>/README.md or ../<project>/BRIEF.md

lumo and janet chibicc would be good ones to look at, I made an attempt at standardizing them to similar workflows recently

---

## 🤖 Claude

Let me find the sibling projects and look at the convention files in the ones you mentioned.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude && ls -d */ | head -60",
  "description": "List sibling project directories"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude && for p in lumo-darwin8-ppc janet-darwin8-ppc chibicc-book; do echo \"=== $p ===\"; ls -la \"$p\" 2>/dev/null | grep -iE \"CLAUDE|README|BRIEF|\\.md\" ; echo \"--- docs/sessions ---\"; ls \"$p/docs/sessions/\" 2>/dev/null | head; echo; done",
  "description": "Inspect convention files in the three sibling projects"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude && for p in lumo-darwin8-ppc janet-darwin8-ppc chibicc-book; do echo \"=== $p/docs/sessions/README.md ===\"; ls -la \"$p/docs/sessions/README.md\" 2>/dev/null || echo \"(none)\"; echo \"=== $p/docs/ (other) ===\"; ls \"$p/docs/\" 2>/dev/null; echo; done",
  "description": "Check for sessions/README.md and docs layout"
}


---

## 🤖 Claude

Let me read the CLAUDE.md and sessions/README.md from the two primary examples (janet and lumo), plus chibicc for comparison.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/janet-darwin8-ppc/CLAUDE.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/janet-darwin8-ppc/docs/sessions/README.md"
}


---

## 🤖 Claude

Now the lumo files, plus the chibicc CLAUDE.md, to see what's common (the standardized template) vs. project-specific.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/lumo-darwin8-ppc/CLAUDE.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/lumo-darwin8-ppc/docs/sessions/README.md"
}


---

## 🤖 Claude

Let me glance at chibicc to triangulate, then I'll synthesize the common template.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/chibicc-book/CLAUDE.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/chibicc-book/CLAUDE-README.md"
}


---

## 🤖 Claude

I've studied all three. They share a clear, deliberate template (janet is the most complete; lumo leaner; chibicc adapts it for a writing project). Here's the standard and how I'd adapt it for memtestppc+.

## The common template

**`CLAUDE.md`** — agent instructions, in this order:
1. **What this project is** — one-paragraph mission + links to plan/roadmap/deferred docs
2. **Repo layout** — annotated ASCII tree
3. **Sessions workflow** — defers to `docs/sessions/README.md` ("read this before starting")
4. **Fleet hosts** — each machine and its role (dev / build / test)
5. **Risk tolerance by host** — low on the main Mac, high on reinstallable PPC fleet
6. **Document everything** — capture liberally in the session dir
7. **Unsupervised mode** — what to do when you're unreachable
8. **Key invariants** — the project-specific must-not-break rules
9. Project-specific sections (release distribution, linking strategy, voice/style, etc.)

**`docs/sessions/README.md`** — the workflow itself:
- Per-session-dir files: `README.md` (narrative; first para = arrival, last = exit), **`HANDOFF.md` (every session)**, optional `findings.md` / `commits.md` / `build-logs/`
- Naming: `NNN-<short-slug>/`, globally monotonic from `001`
- Start-of-session checklist + End-of-session ritual

A user-facing **`README.md`** and a plan doc (`docs/plan.md` for janet/lumo, `book-plan.md` for chibicc) round it out; janet also has `roadmap.md` + `deferred.md`.

## How memtestppc+ differs / what I'd fill in

- **Session docs**: ours are **HANDOFF-only** (001–004 have no per-session `README.md`). That actually matches your global `~/.claude/CLAUDE.md`, which is HANDOFF-centric. The sibling standard is README+HANDOFF.
- **No `docs/plan.md`, `roadmap.md`, or user-facing `README.md` yet** — the siblings all have these.
- I already have the project-specific content from the transcript: mission (faithful v5.01 TUI, boots from CD *image*, no OS, → 1.00), fleet (uranium=dev/QEMU, opti7050=build+ISO, opti9020=alt build, ibookg32 & imacg3=real-hw test), and invariants (cross-compile only / no native Tiger build, CD-image boot via CHRP not `-kernel`, CIF in r5, **port-fidelity** per your stored feedback, no physical CD burned yet).

## Decisions before I draft

1. **Session-doc shape:** keep **HANDOFF-only** (matches current 001–004 + your global file), or adopt the sibling **README + HANDOFF** split going forward? I lean HANDOFF-only for consistency with what exists, with `findings.md` available when a session warrants it.
2. **Which docs to create now:** just (a) `CLAUDE.md` + `docs/sessions/README.md`? Or also (b) a `docs/plan.md` (mission/non-goals/roadmap-to-1.00, incl. the port-fidelity pivot) and (c) a user-facing `README.md`? I'd suggest a + b now, and defer the public README until the port settles.
3. **Roadmap/deferred:** fold forward-work into `docs/plan.md`, or split out `roadmap.md` + `deferred.md` like janet? For a project this size I'd keep it in one `plan.md`.

My recommendation: HANDOFF-only; create `CLAUDE.md`, `docs/sessions/README.md`, and `docs/plan.md` now; single `plan.md` rather than separate roadmap/deferred. Want me to proceed on that basis, or adjust?

---

## 👤 User

yeah, handoff-only as both running log, findings and handoff-for-next session is probably the best pattern emerging here.  "read the last few handoffs to get caught up to speed" is an convenient directive.

actually, since you have all of this meta context, let's take a slight diversion and create a skill for setting up the workflow for a brand new project, then we'll apply your suggestions to this project in particular

---

## 🤖 Claude

This is a perfect fit for the skill-creator skill — let me invoke it to build this properly.

---

## 🤖 Claude


🔧 Using tool: Skill
Input: {
  "skill": "anthropic-skills:skill-creator",
  "args": "Create a new skill that scaffolds a session-based project workflow for a brand-new project: a CLAUDE.md (agent instructions), docs/sessions/README.md (workflow), and docs/plan.md, following the HANDOFF-only convention where each session's HANDOFF.md serves as running log + findings + handoff-for-next-session, and \"read the last few HANDOFFs to get up to speed\" is the catch-up directive. Modeled on sibling projects janet-darwin8-ppc, lumo-darwin8-ppc, chibicc-book at /Users/cell/claude/."
}


---

## 👤 User

Base directory for this skill: /Users/cell/Library/Application Support/Claude/local-agent-mode-sessions/skills-plugin/486ac48c-99cd-41fa-91d0-504c726cb8ef/702722d1-1d63-4f31-b885-e24cf5ba03d4/skills/skill-creator

# Skill Creator

A skill for creating new skills and iteratively improving them.

At a high level, the process of creating a skill goes like this:

- Decide what you want the skill to do and roughly how it should do it
- Write a draft of the skill
- Create a few test prompts and run claude-with-access-to-the-skill on them
- Help the user evaluate the results both qualitatively and quantitatively
  - While the runs happen in the background, draft some quantitative evals if there aren't any (if there are some, you can either use as is or modify if you feel something needs to change about them). Then explain them to the user (or if they already existed, explain the ones that already exist)
  - Use the `eval-viewer/generate_review.py` script to show the user the results for them to look at, and also let them look at the quantitative metrics
- Rewrite the skill based on feedback from the user's evaluation of the results (and also if there are any glaring flaws that become apparent from the quantitative benchmarks)
- Repeat until you're satisfied
- Expand the test set and try again at larger scale

Your job when using this skill is to figure out where the user is in this process and then jump in and help them progress through these stages. So for instance, maybe they're like "I want to make a skill for X". You can help narrow down what they mean, write a draft, write the test cases, figure out how they want to evaluate, run all the prompts, and repeat.

On the other hand, maybe they already have a draft of the skill. In this case you can go straight to the eval/iterate part of the loop.

Of course, you should always be flexible and if the user is like "I don't need to run a bunch of evaluations, just vibe with me", you can do that instead.

Then after the skill is done (but again, the order is flexible), you can also run the skill description improver, which we have a whole separate script for, to optimize the triggering of the skill.

Cool? Cool.

## Communicating with the user

The skill creator is liable to be used by people across a wide range of familiarity with coding jargon. If you haven't heard (and how could you, it's only very recently that it started), there's a trend now where the power of Claude is inspiring plumbers to open up their terminals, parents and grandparents to google "how to install npm". On the other hand, the bulk of users are probably fairly computer-literate.

So please pay attention to context cues to understand how to phrase your communication! In the default case, just to give you some idea:

- "evaluation" and "benchmark" are borderline, but OK
- for "JSON" and "assertion" you want to see serious cues from the user that they know what those things are before using them without explaining them

It's OK to briefly explain terms if you're in doubt, and feel free to clarify terms with a short definition if you're unsure if the user will get it.

---

## Creating a skill

### Capture Intent

Start by understanding the user's intent. The current conversation might already contain a workflow the user wants to capture (e.g., they say "turn this into a skill"). If so, extract answers from the conversation history first — the tools used, the sequence of steps, corrections the user made, input/output formats observed. The user may need to fill the gaps, and should confirm before proceeding to the next step.

1. What should this skill enable Claude to do?
2. When should this skill trigger? (what user phrases/contexts)
3. What's the expected output format?
4. Should we set up test cases to verify the skill works? Skills with objectively verifiable outputs (file transforms, data extraction, code generation, fixed workflow steps) benefit from test cases. Skills with subjective outputs (writing style, art) often don't need them. Suggest the appropriate default based on the skill type, but let the user decide.

### Interview and Research

Proactively ask questions about edge cases, input/output formats, example files, success criteria, and dependencies. Wait to write test prompts until you've got this part ironed out.

Check available MCPs - if useful for research (searching docs, finding similar skills, looking up best practices), research in parallel via subagents if available, otherwise inline. Come prepared with context to reduce burden on the user.

### Write the SKILL.md

Based on the user interview, fill in these components:

- **name**: Skill identifier
- **description**: When to trigger, what it does. This is the primary triggering mechanism - include both what the skill does AND specific contexts for when to use it. All "when to use" info goes here, not in the body. Note: currently Claude has a tendency to "undertrigger" skills -- to not use them when they'd be useful. To combat this, please make the skill descriptions a little bit "pushy". So for instance, instead of "How to build a simple fast dashboard to display internal Anthropic data.", you might write "How to build a simple fast dashboard to display internal Anthropic data. Make sure to use this skill whenever the user mentions dashboards, data visualization, internal metrics, or wants to display any kind of company data, even if they don't explicitly ask for a 'dashboard.'"
- **compatibility**: Required tools, dependencies (optional, rarely needed)
- **the rest of the skill :)**

### Skill Writing Guide

#### Anatomy of a Skill

```
skill-name/
├── SKILL.md (required)
│   ├── YAML frontmatter (name, description required)
│   └── Markdown instructions
└── Bundled Resources (optional)
    ├── scripts/    - Executable code for deterministic/repetitive tasks
    ├── references/ - Docs loaded into context as needed
    └── assets/     - Files used in output (templates, icons, fonts)
```

#### Progressive Disclosure

Skills use a three-level loading system:
1. **Metadata** (name + description) - Always in context (~100 words)
2. **SKILL.md body** - In context whenever skill triggers (<500 lines ideal)
3. **Bundled resources** - As needed (unlimited, scripts can execute without loading)

These word counts are approximate and you can feel free to go longer if needed.

**Key patterns:**
- Keep SKILL.md under 500 lines; if you're approaching this limit, add an additional layer of hierarchy along with clear pointers about where the model using the skill should go next to follow up.
- Reference files clearly from SKILL.md with guidance on when to read them
- For large reference files (>300 lines), include a table of contents

**Domain organization**: When a skill supports multiple domains/frameworks, organize by variant:
```
cloud-deploy/
├── SKILL.md (workflow + selection)
└── references/
    ├── aws.md
    ├── gcp.md
    └── azure.md
```
Claude reads only the relevant reference file.

#### Principle of Lack of Surprise

This goes without saying, but skills must not contain malware, exploit code, or any content that could compromise system security. A skill's contents should not surprise the user in their intent if described. Don't go along with requests to create misleading skills or skills designed to facilitate unauthorized access, data exfiltration, or other malicious activities. Things like a "roleplay as an XYZ" are OK though.

#### Writing Patterns

Prefer using the imperative form in instructions.

**Defining output formats** - You can do it like this:
```markdown
## Report structure
ALWAYS use this exact template:
# [Title]
## Executive summary
## Key findings
## Recommendations
```

**Examples pattern** - It's useful to include examples. You can format them like this (but if "Input" and "Output" are in the examples you might want to deviate a little):
```markdown
## Commit message format
**Example 1:**
Input: Added user authentication with JWT tokens
Output: feat(auth): implement JWT-based authentication
```

### Writing Style

Try to explain to the model why things are important in lieu of heavy-handed musty MUSTs. Use theory of mind and try to make the skill general and not super-narrow to specific examples. Start by writing a draft and then look at it with fresh eyes and improve it.

### Test Cases

After writing the skill draft, come up with 2-3 realistic test prompts — the kind of thing a real user would actually say. Share them with the user: [you don't have to use this exact language] "Here are a few test cases I'd like to try. Do these look right, or do you want to add more?" Then run them.

Save test cases to `evals/evals.json`. Don't write assertions yet — just the prompts. You'll draft assertions in the next step while the runs are in progress.

```json
{
  "skill_name": "example-skill",
  "evals": [
    {
      "id": 1,
      "prompt": "User's task prompt",
      "expected_output": "Description of expected result",
      "files": []
    }
  ]
}
```

See `references/schemas.md` for the full schema (including the `assertions` field, which you'll add later).

## Running and evaluating test cases

This section is one continuous sequence — don't stop partway through. Do NOT use `/skill-test` or any other testing skill.

Put results in `<skill-name>-workspace/` as a sibling to the skill directory. Within the workspace, organize results by iteration (`iteration-1/`, `iteration-2/`, etc.) and within that, each test case gets a directory (`eval-0/`, `eval-1/`, etc.). Don't create all of this upfront — just create directories as you go.

### Step 1: Spawn all runs (with-skill AND baseline) in the same turn

For each test case, spawn two subagents in the same turn — one with the skill, one without. This is important: don't spawn the with-skill runs first and then come back for baselines later. Launch everything at once so it all finishes around the same time.

**With-skill run:**

```
Execute this task:
- Skill path: <path-to-skill>
- Task: <eval prompt>
- Input files: <eval files if any, or "none">
- Save outputs to: <workspace>/iteration-<N>/eval-<ID>/with_skill/outputs/
- Outputs to save: <what the user cares about — e.g., "the .docx file", "the final CSV">
```

**Baseline run** (same prompt, but the baseline depends on context):
- **Creating a new skill**: no skill at all. Same prompt, no skill path, save to `without_skill/outputs/`.
- **Improving an existing skill**: the old version. Before editing, snapshot the skill (`cp -r <skill-path> <workspace>/skill-snapshot/`), then point the baseline subagent at the snapshot. Save to `old_skill/outputs/`.

Write an `eval_metadata.json` for each test case (assertions can be empty for now). Give each eval a descriptive name based on what it's testing — not just "eval-0". Use this name for the directory too. If this iteration uses new or modified eval prompts, create these files for each new eval directory — don't assume they carry over from previous iterations.

```json
{
  "eval_id": 0,
  "eval_name": "descriptive-name-here",
  "prompt": "The user's task prompt",
  "assertions": []
}
```

### Step 2: While runs are in progress, draft assertions

Don't just wait for the runs to finish — you can use this time productively. Draft quantitative assertions for each test case and explain them to the user. If assertions already exist in `evals/evals.json`, review them and explain what they check.

Good assertions are objectively verifiable and have descriptive names — they should read clearly in the benchmark viewer so someone glancing at the results immediately understands what each one checks. Subjective skills (writing style, design quality) are better evaluated qualitatively — don't force assertions onto things that need human judgment.

Update the `eval_metadata.json` files and `evals/evals.json` with the assertions once drafted. Also explain to the user what they'll see in the viewer — both the qualitative outputs and the quantitative benchmark.

### Step 3: As runs complete, capture timing data

When each subagent task completes, you receive a notification containing `total_tokens` and `duration_ms`. Save this data immediately to `timing.json` in the run directory:

```json
{
  "total_tokens": 84852,
  "duration_ms": 23332,
  "total_duration_seconds": 23.3
}
```

This is the only opportunity to capture this data — it comes through the task notification and isn't persisted elsewhere. Process each notification as it arrives rather than trying to batch them.

### Step 4: Grade, aggregate, and launch the viewer

Once all runs are done:

1. **Grade each run** — spawn a grader subagent (or grade inline) that reads `agents/grader.md` and evaluates each assertion against the outputs. Save results to `grading.json` in each run directory. The grading.json expectations array must use the fields `text`, `passed`, and `evidence` (not `name`/`met`/`details` or other variants) — the viewer depends on these exact field names. For assertions that can be checked programmatically, write and run a script rather than eyeballing it — scripts are faster, more reliable, and can be reused across iterations.

2. **Aggregate into benchmark** — run the aggregation script from the skill-creator directory:
   ```bash
   python -m scripts.aggregate_benchmark <workspace>/iteration-N --skill-name <name>
   ```
   This produces `benchmark.json` and `benchmark.md` with pass_rate, time, and tokens for each configuration, with mean ± stddev and the delta. If generating benchmark.json manually, see `references/schemas.md` for the exact schema the viewer expects.
Put each with_skill version before its baseline counterpart.

3. **Do an analyst pass** — read the benchmark data and surface patterns the aggregate stats might hide. See `agents/analyzer.md` (the "Analyzing Benchmark Results" section) for what to look for — things like assertions that always pass regardless of skill (non-discriminating), high-variance evals (possibly flaky), and time/token tradeoffs.

4. **Launch the viewer** with both qualitative outputs and quantitative data:
   ```bash
   nohup python <skill-creator-path>/eval-viewer/generate_review.py \
     <workspace>/iteration-N \
     --skill-name "my-skill" \
     --benchmark <workspace>/iteration-N/benchmark.json \
     > /dev/null 2>&1 &
   VIEWER_PID=$!
   ```
   For iteration 2+, also pass `--previous-workspace <workspace>/iteration-<N-1>`.

   **Cowork / headless environments:** If `webbrowser.open()` is not available or the environment has no display, use `--static <output_path>` to write a standalone HTML file instead of starting a server. Feedback will be downloaded as a `feedback.json` file when the user clicks "Submit All Reviews". After download, copy `feedback.json` into the workspace directory for the next iteration to pick up.

Note: please use generate_review.py to create the viewer; there's no need to write custom HTML.

5. **Tell the user** something like: "I've opened the results in your browser. There are two tabs — 'Outputs' lets you click through each test case and leave feedback, 'Benchmark' shows the quantitative comparison. When you're done, come back here and let me know."

### What the user sees in the viewer

The "Outputs" tab shows one test case at a time:
- **Prompt**: the task that was given
- **Output**: the files the skill produced, rendered inline where possible
- **Previous Output** (iteration 2+): collapsed section showing last iteration's output
- **Formal Grades** (if grading was run): collapsed section showing assertion pass/fail
- **Feedback**: a textbox that auto-saves as they type
- **Previous Feedback** (iteration 2+): their comments from last time, shown below the textbox

The "Benchmark" tab shows the stats summary: pass rates, timing, and token usage for each configuration, with per-eval breakdowns and analyst observations.

Navigation is via prev/next buttons or arrow keys. When done, they click "Submit All Reviews" which saves all feedback to `feedback.json`.

### Step 5: Read the feedback

When the user tells you they're done, read `feedback.json`:

```json
{
  "reviews": [
    {"run_id": "eval-0-with_skill", "feedback": "the chart is missing axis labels", "timestamp": "..."},
    {"run_id": "eval-1-with_skill", "feedback": "", "timestamp": "..."},
    {"run_id": "eval-2-with_skill", "feedback": "perfect, love this", "timestamp": "..."}
  ],
  "status": "complete"
}
```

Empty feedback means the user thought it was fine. Focus your improvements on the test cases where the user had specific complaints.

Kill the viewer server when you're done with it:

```bash
kill $VIEWER_PID 2>/dev/null
```

---

## Improving the skill

This is the heart of the loop. You've run the test cases, the user has reviewed the results, and now you need to make the skill better based on their feedback.

### How to think about improvements

1. **Generalize from the feedback.** The big picture thing that's happening here is that we're trying to create skills that can be used a million times (maybe literally, maybe even more who knows) across many different prompts. Here you and the user are iterating on only a few examples over and over again because it helps move faster. The user knows these examples in and out and it's quick for them to assess new outputs. But if the skill you and the user are codeveloping works only for those examples, it's useless. Rather than put in fiddly overfitty changes, or oppressively constrictive MUSTs, if there's some stubborn issue, you might try branching out and using different metaphors, or recommending different patterns of working. It's relatively cheap to try and maybe you'll land on something great.

2. **Keep the prompt lean.** Remove things that aren't pulling their weight. Make sure to read the transcripts, not just the final outputs — if it looks like the skill is making the model waste a bunch of time doing things that are unproductive, you can try getting rid of the parts of the skill that are making it do that and seeing what happens.

3. **Explain the why.** Try hard to explain the **why** behind everything you're asking the model to do. Today's LLMs are *smart*. They have good theory of mind and when given a good harness can go beyond rote instructions and really make things happen. Even if the feedback from the user is terse or frustrated, try to actually understand the task and why the user is writing what they wrote, and what they actually wrote, and then transmit this understanding into the instructions. If you find yourself writing ALWAYS or NEVER in all caps, or using super rigid structures, that's a yellow flag — if possible, reframe and explain the reasoning so that the model understands why the thing you're asking for is important. That's a more humane, powerful, and effective approach.

4. **Look for repeated work across test cases.** Read the transcripts from the test runs and notice if the subagents all independently wrote similar helper scripts or took the same multi-step approach to something. If all 3 test cases resulted in the subagent writing a `create_docx.py` or a `build_chart.py`, that's a strong signal the skill should bundle that script. Write it once, put it in `scripts/`, and tell the skill to use it. This saves every future invocation from reinventing the wheel.

This task is pretty important (we are trying to create billions a year in economic value here!) and your thinking time is not the blocker; take your time and really mull things over. I'd suggest writing a draft revision and then looking at it anew and making improvements. Really do your best to get into the head of the user and understand what they want and need.

### The iteration loop

After improving the skill:

1. Apply your improvements to the skill
2. Rerun all test cases into a new `iteration-<N+1>/` directory, including baseline runs. If you're creating a new skill, the baseline is always `without_skill` (no skill) — that stays the same across iterations. If you're improving an existing skill, use your judgment on what makes sense as the baseline: the original version the user came in with, or the previous iteration.
3. Launch the reviewer with `--previous-workspace` pointing at the previous iteration
4. Wait for the user to review and tell you they're done
5. Read the new feedback, improve again, repeat

Keep going until:
- The user says they're happy
- The feedback is all empty (everything looks good)
- You're not making meaningful progress

---

## Advanced: Blind comparison

For situations where you want a more rigorous comparison between two versions of a skill (e.g., the user asks "is the new version actually better?"), there's a blind comparison system. Read `agents/comparator.md` and `agents/analyzer.md` for the details. The basic idea is: give two outputs to an independent agent without telling it which is which, and let it judge quality. Then analyze why the winner won.

This is optional, requires subagents, and most users won't need it. The human review loop is usually sufficient.

---

## Description Optimization

The description field in SKILL.md frontmatter is the primary mechanism that determines whether Claude invokes a skill. After creating or improving a skill, offer to optimize the description for better triggering accuracy.

### Step 1: Generate trigger eval queries

Create 20 eval queries — a mix of should-trigger and should-not-trigger. Save as JSON:

```json
[
  {"query": "the user prompt", "should_trigger": true},
  {"query": "another prompt", "should_trigger": false}
]
```

The queries must be realistic and something a Claude Code or Claude.ai user would actually type. Not abstract requests, but requests that are concrete and specific and have a good amount of detail. For instance, file paths, personal context about the user's job or situation, column names and values, company names, URLs. A little bit of backstory. Some might be in lowercase or contain abbreviations or typos or casual speech. Use a mix of different lengths, and focus on edge cases rather than making them clear-cut (the user will get a chance to sign off on them).

Bad: `"Format this data"`, `"Extract text from PDF"`, `"Create a chart"`

Good: `"ok so my boss just sent me this xlsx file (its in my downloads, called something like 'Q4 sales final FINAL v2.xlsx') and she wants me to add a column that shows the profit margin as a percentage. The revenue is in column C and costs are in column D i think"`

For the **should-trigger** queries (8-10), think about coverage. You want different phrasings of the same intent — some formal, some casual. Include cases where the user doesn't explicitly name the skill or file type but clearly needs it. Throw in some uncommon use cases and cases where this skill competes with another but should win.

For the **should-not-trigger** queries (8-10), the most valuable ones are the near-misses — queries that share keywords or concepts with the skill but actually need something different. Think adjacent domains, ambiguous phrasing where a naive keyword match would trigger but shouldn't, and cases where the query touches on something the skill does but in a context where another tool is more appropriate.

The key thing to avoid: don't make should-not-trigger queries obviously irrelevant. "Write a fibonacci function" as a negative test for a PDF skill is too easy — it doesn't test anything. The negative cases should be genuinely tricky.

### Step 2: Review with user

Present the eval set to the user for review using the HTML template:

1. Read the template from `assets/eval_review.html`
2. Replace the placeholders:
   - `__EVAL_DATA_PLACEHOLDER__` → the JSON array of eval items (no quotes around it — it's a JS variable assignment)
   - `__SKILL_NAME_PLACEHOLDER__` → the skill's name
   - `__SKILL_DESCRIPTION_PLACEHOLDER__` → the skill's current description
3. Write to a temp file (e.g., `/tmp/eval_review_<skill-name>.html`) and open it: `open /tmp/eval_review_<skill-name>.html`
4. The user can edit queries, toggle should-trigger, add/remove entries, then click "Export Eval Set"
5. The file downloads to `~/Downloads/eval_set.json` — check the Downloads folder for the most recent version in case there are multiple (e.g., `eval_set (1).json`)

This step matters — bad eval queries lead to bad descriptions.

### Step 3: Run the optimization loop

Tell the user: "This will take some time — I'll run the optimization loop in the background and check on it periodically."

Save the eval set to the workspace, then run in the background:

```bash
python -m scripts.run_loop \
  --eval-set <path-to-trigger-eval.json> \
  --skill-path <path-to-skill> \
  --model <model-id-powering-this-session> \
  --max-iterations 5 \
  --verbose
```

Use the model ID from your system prompt (the one powering the current session) so the triggering test matches what the user actually experiences.

While it runs, periodically tail the output to give the user updates on which iteration it's on and what the scores look like.

This handles the full optimization loop automatically. It splits the eval set into 60% train and 40% held-out test, evaluates the current description (running each query 3 times to get a reliable trigger rate), then calls Claude to propose improvements based on what failed. It re-evaluates each new description on both train and test, iterating up to 5 times. When it's done, it opens an HTML report in the browser showing the results per iteration and returns JSON with `best_description` — selected by test score rather than train score to avoid overfitting.

### How skill triggering works

Understanding the triggering mechanism helps design better eval queries. Skills appear in Claude's `available_skills` list with their name + description, and Claude decides whether to consult a skill based on that description. The important thing to know is that Claude only consults skills for tasks it can't easily handle on its own — simple, one-step queries like "read this PDF" may not trigger a skill even if the description matches perfectly, because Claude can handle them directly with basic tools. Complex, multi-step, or specialized queries reliably trigger skills when the description matches.

This means your eval queries should be substantive enough that Claude would actually benefit from consulting a skill. Simple queries like "read file X" are poor test cases — they won't trigger skills regardless of description quality.

### Step 4: Apply the result

Take `best_description` from the JSON output and update the skill's SKILL.md frontmatter. Show the user before/after and report the scores.

---

### Package and Present (only if `present_files` tool is available)

Check whether you have access to the `present_files` tool. If you don't, skip this step. If you do, package the skill and present the .skill file to the user:

```bash
python -m scripts.package_skill <path/to/skill-folder>
```

After packaging, direct the user to the resulting `.skill` file path so they can install it.

---

## Claude.ai-specific instructions

In Claude.ai, the core workflow is the same (draft → test → review → improve → repeat), but because Claude.ai doesn't have subagents, some mechanics change. Here's what to adapt:

**Running test cases**: No subagents means no parallel execution. For each test case, read the skill's SKILL.md, then follow its instructions to accomplish the test prompt yourself. Do them one at a time. This is less rigorous than independent subagents (you wrote the skill and you're also running it, so you have full context), but it's a useful sanity check — and the human review step compensates. Skip the baseline runs — just use the skill to complete the task as requested.

**Reviewing results**: If you can't open a browser (e.g., Claude.ai's VM has no display, or you're on a remote server), skip the browser reviewer entirely. Instead, present results directly in the conversation. For each test case, show the prompt and the output. If the output is a file the user needs to see (like a .docx or .xlsx), save it to the filesystem and tell them where it is so they can download and inspect it. Ask for feedback inline: "How does this look? Anything you'd change?"

**Benchmarking**: Skip the quantitative benchmarking — it relies on baseline comparisons which aren't meaningful without subagents. Focus on qualitative feedback from the user.

**The iteration loop**: Same as before — improve the skill, rerun the test cases, ask for feedback — just without the browser reviewer in the middle. You can still organize results into iteration directories on the filesystem if you have one.

**Description optimization**: This section requires the `claude` CLI tool (specifically `claude -p`) which is only available in Claude Code. Skip it if you're on Claude.ai.

**Blind comparison**: Requires subagents. Skip it.

**Packaging**: The `package_skill.py` script works anywhere with Python and a filesystem. On Claude.ai, you can run it and the user can download the resulting `.skill` file.

**Updating an existing skill**: The user might be asking you to update an existing skill, not create a new one. In this case:
- **Preserve the original name.** Note the skill's directory name and `name` frontmatter field -- use them unchanged. E.g., if the installed skill is `research-helper`, output `research-helper.skill` (not `research-helper-v2`).
- **Copy to a writeable location before editing.** The installed skill path may be read-only. Copy to `/tmp/skill-name/`, edit there, and package from the copy.
- **If packaging manually, stage in `/tmp/` first**, then copy to the output directory -- direct writes may fail due to permissions.

---

## Cowork-Specific Instructions

If you're in Cowork, the main things to know are:

- You have subagents, so the main workflow (spawn test cases in parallel, run baselines, grade, etc.) all works. (However, if you run into severe problems with timeouts, it's OK to run the test prompts in series rather than parallel.)
- You don't have a browser or display, so when generating the eval viewer, use `--static <output_path>` to write a standalone HTML file instead of starting a server. Then proffer a link that the user can click to open the HTML in their browser.
- For whatever reason, the Cowork setup seems to disincline Claude from generating the eval viewer after running the tests, so just to reiterate: whether you're in Cowork or in Claude Code, after running tests, you should always generate the eval viewer for the human to look at examples before revising the skill yourself and trying to make corrections, using `generate_review.py` (not writing your own boutique html code). Sorry in advance but I'm gonna go all caps here: GENERATE THE EVAL VIEWER *BEFORE* evaluating inputs yourself. You want to get them in front of the human ASAP!
- Feedback works differently: since there's no running server, the viewer's "Submit All Reviews" button will download `feedback.json` as a file. You can then read it from there (you may have to request access first).
- Packaging works — `package_skill.py` just needs Python and a filesystem.
- Description optimization (`run_loop.py` / `run_eval.py`) should work in Cowork just fine since it uses `claude -p` via subprocess, not a browser, but please save it until you've fully finished making the skill and the user agrees it's in good shape.
- **Updating an existing skill**: The user might be asking you to update an existing skill, not create a new one. Follow the update guidance in the claude.ai section above.

---

## Reference files

The agents/ directory contains instructions for specialized subagents. Read them when you need to spawn the relevant subagent.

- `agents/grader.md` — How to evaluate assertions against outputs
- `agents/comparator.md` — How to do blind A/B comparison between two outputs
- `agents/analyzer.md` — How to analyze why one version beat another

The references/ directory has additional documentation:
- `references/schemas.md` — JSON structures for evals.json, grading.json, etc.

---

Repeating one more time the core loop here for emphasis:

- Figure out what the skill is about
- Draft or edit the skill
- Run claude-with-access-to-the-skill on test prompts
- With the user, evaluate the outputs:
  - Create benchmark.json and run `eval-viewer/generate_review.py` to help the user review them
  - Run quantitative evals
- Repeat until you and the user are satisfied
- Package the final skill and return it to the user.

Please add steps to your TodoList, if you have such a thing, to make sure you don't forget. If you're in Cowork, please specifically put "Create evals JSON and run `eval-viewer/generate_review.py` so human can review test cases" in your TodoList to make sure it happens.

Good luck!


ARGUMENTS: Create a new skill that scaffolds a session-based project workflow for a brand-new project: a CLAUDE.md (agent instructions), docs/sessions/README.md (workflow), and docs/plan.md, following the HANDOFF-only convention where each session's HANDOFF.md serves as running log + findings + handoff-for-next-session, and "read the last few HANDOFFs to get up to speed" is the catch-up directive. Modeled on sibling projects janet-darwin8-ppc, lumo-darwin8-ppc, chibicc-book at /Users/cell/claude/.

---

## 🤖 Claude

I have all the context I need from studying janet/lumo/chibicc plus this whole session. Let me set up tracking and check where skills live.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "ls -la /Users/cell/.claude/skills/ 2>/dev/null | head -30",
  "description": "List existing user skills"
}


---

## 🤖 Claude

Skills live inside projects and are symlinked into `~/.claude/skills/`. This one is cross-project (meta), so I'll build it under `~/.claude/skills/` directly and flag the symlink option to you at the end. Let me scaffold it.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "mkdir -p \"/Users/cell/.claude/skills/project-workflow-setup/assets\" && echo created",
  "description": "Create skill directory structure"
}


---

## 🤖 Claude

Now the SKILL.md:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "content": "---\nname: project-workflow-setup\ndescription: >-\n  Scaffolds the session-based documentation workflow for a project \u2014 a CLAUDE.md\n  (agent instructions), docs/sessions/README.md (the workflow spec), and\n  docs/plan.md (the project brief), following the HANDOFF-only convention where\n  each session writes one docs/sessions/NNN-slug/HANDOFF.md that doubles as\n  running log, findings, and primer for the next session. Use this whenever the\n  user wants to set up project conventions, bootstrap a new (or newly-adopted)\n  repo, \"add a CLAUDE.md\", establish a sessions/handoff workflow, start tracking\n  work in numbered session directories, or standardize an existing project to\n  match the sibling-project workflow \u2014 even if they don't name the specific files.\n  Also use when retrofitting these conventions onto a project that already has\n  some work in it.\n---\n\n# Project workflow setup\n\nThis skill establishes a lightweight, durable workflow for a project so that any\nfuture session (a fresh Claude, or the user weeks later) can pick up the thread\nwithout re-explaining. The whole system rests on one habit: **every working\nsession writes a single `HANDOFF.md`** in its own numbered directory, and the\nnext session starts by **reading the last few HANDOFFs**.\n\nIt produces three files, tailored to the specific project:\n\n- `CLAUDE.md` \u2014 agent instructions: what the project is, how to work in it, what\n  must not break.\n- `docs/sessions/README.md` \u2014 the workflow spec: session-dir conventions, the\n  start-of-session and end-of-session rituals.\n- `docs/plan.md` \u2014 the project brief: mission, non-goals, strategy, roadmap.\n\nTemplates for all three live in `assets/`. They are starting points, **not**\nfill-in-the-blank forms \u2014 read them, then adapt.\n\n## The convention in one breath\n\n- Substantive work lives in `docs/sessions/NNN-<short-slug>/`, globally numbered\n  from `001`, monotonically increasing for the life of the project.\n- Each session dir contains **one** `HANDOFF.md`. It is the running log *and* the\n  findings *and* the handoff to the next session \u2014 not three separate files.\n- A session starts by reading the last 1\u20133 HANDOFFs. It ends by writing this\n  session's HANDOFF.\n- `CLAUDE.md` points at `docs/sessions/README.md` for the workflow and at\n  `docs/plan.md` for the mission.\n\n## Why HANDOFF-only (the reasoning, so you can defend it)\n\nThe sibling projects this is borrowed from originally split each session into a\n`README.md` (narrative of what happened) plus a `HANDOFF.md` (primer for next\ntime). In practice that split is overhead: the two documents say nearly the same\nthing, and the narrative rarely gets read on its own. Collapsing to a single\n`HANDOFF.md` per session keeps the value (a future reader can reconstruct the\nthinking) without the bookkeeping. \"Read the last few HANDOFFs to get up to\nspeed\" then becomes a clean, reliable catch-up directive \u2014 the HANDOFFs *are* the\nproject's memory.\n\nSo: don't recreate the README/HANDOFF split. One file per session. If a session\nproduces a heavy artifact worth keeping verbatim (a long build log, a captured\ntranscript), drop it in the session dir alongside the HANDOFF and reference it \u2014\nbut the HANDOFF stays the single narrative spine.\n\n## Step 1 \u2014 Interview before you scaffold\n\nThe templates are worthless filled with generic boilerplate. The value is in the\nproject-specific content, so gather it first. Pull what you can from the\nconversation and the repo (read an existing README, `git log`, the directory\ntree) before asking, then ask the user only for what's genuinely missing:\n\n- **Mission** \u2014 one paragraph: what is this, why does it exist, what does \"done\"\n  look like? (\u2192 `docs/plan.md`, and a shorter form in `CLAUDE.md`.)\n- **Non-goals** \u2014 what this project deliberately is *not* doing. Saves future\n  sessions from scope creep.\n- **Hosts / fleet** \u2014 what machines are involved and their roles (dev box, build\n  host, test target). Include how to reach them (ssh aliases) and any per-host\n  quirks. Skip this section entirely for a single-machine project.\n- **Build & test commands** \u2014 the canonical way to build and to verify. Future\n  sessions should not have to rediscover these.\n- **Risk tolerance** \u2014 are some machines reinstallable/experimental (high\n  tolerance) while others are the daily driver (low)? This tells a future agent\n  where it can take chances.\n- **Key invariants** \u2014 the must-not-break rules specific to this project (e.g.\n  \"patches/ is canonical\", \"never commit external/\", \"cross-compile only\"). These\n  are the most valuable and most project-specific part of `CLAUDE.md`.\n- **Roadmap** \u2014 the prioritized forward work, even if rough (M1/M2/M3 or a punch\n  list). (\u2192 `docs/plan.md`.)\n\nIf the project already has working sessions but no convention docs (a\nretrofit), also skim the newest sessions/handoffs so the scaffold reflects how\nwork is *actually* being done, not an idealized version.\n\n## Step 2 \u2014 Scaffold the files\n\n1. `mkdir -p docs/sessions` (and `docs/convos` if the user keeps raw transcripts).\n2. Copy `assets/sessions-README.md.template` \u2192 `docs/sessions/README.md` and fill\n   it in. This one is the most reusable \u2014 mostly the same across projects.\n3. Copy `assets/CLAUDE.md.template` \u2192 `CLAUDE.md` and fill it in. Drop sections\n   that don't apply (e.g. Fleet hosts for a solo-machine project); add\n   project-specific sections (release process, linking strategy, house style \u2014\n   whatever this project needs). The sibling projects all grew bespoke sections;\n   that's expected and good.\n4. Copy `assets/plan.md.template` \u2192 `docs/plan.md` and fill it in.\n\nPlaceholders are written as `<ANGLE_BRACKET_CAPS>`. Instructions meant for *you*\n(the filler), not the final reader, are in `<!-- HTML comments -->` \u2014 delete them\nonce the section is written. Don't leave a placeholder unfilled; if a section has\nno real content, cut the section.\n\n## Step 3 \u2014 Surface and confirm\n\nShow the user what you created (brief \u2014 the file paths and a one-line summary of\neach). Offer to seed the first session dir (`docs/sessions/001-<slug>/HANDOFF.md`)\nif this is a brand-new project and there's already work to record, or note that\nthe next real working session will create it.\n\n## What a HANDOFF.md should contain\n\nThe `docs/sessions/README.md` template documents this in full for the project,\nbut the shape is:\n\n- `# Session NNN: <topic>` title.\n- **What happened** \u2014 the running log: what was done, in roughly chronological\n  order, including decisions made, dead-ends hit, and gotchas discovered. The\n  \"findings\" live here \u2014 don't make a separate findings file.\n- **Current state** \u2014 status on exit. What works, what's half-done, where the\n  artifacts are now.\n- **What to do next** \u2014 a priority-ordered punch list for the next session.\n\nKeep it honest and specific (file paths, command lines, the *why* behind\nchoices). A good HANDOFF is one a stranger could act on cold.\n"
}


---

## 🤖 Claude

Now the three templates:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/sessions-README.md.template",
  "content": "<!--\n  TEMPLATE: docs/sessions/README.md\n  This file is the most reusable of the three \u2014 it's nearly identical across\n  projects. Adjust the start-of-session and end-of-session steps to reference\n  this project's actual plan/roadmap docs and any first-session setup script.\n  Delete these HTML comments when done.\n-->\n# Sessions\n\nSubstantive work on this project happens in **sessions**. One Claude\nconversation produces one session, recorded as a single `HANDOFF.md` in its own\nglobally-numbered directory:\n\n```\ndocs/sessions/NNN-<short-slug>/HANDOFF.md\n```\n\n## Why a single HANDOFF per session\n\nThe `HANDOFF.md` is the session's **running log, its findings, and its primer\nfor the next session, all in one file**. We deliberately do not split narrative\nand handoff into separate documents \u2014 the overhead isn't worth it, and one file\nis what actually gets read. The HANDOFFs, read in sequence, *are* this project's\nmemory. So the catch-up directive is simply:\n\n> **Read the last few HANDOFFs to get up to speed.**\n\n## What a HANDOFF.md contains\n\n- `# Session NNN: <topic>` \u2014 title.\n- **What happened** \u2014 the running log. What was done, in roughly chronological\n  order, with enough detail that a future session can reconstruct the thinking:\n  decisions and their rationale, dead-ends hit (so they aren't re-tried), and\n  gotchas discovered. Findings live here \u2014 there is no separate findings file.\n- **Current state** \u2014 status on exit: what works, what's half-finished, and\n  where the canonical artifacts live now if anything moved.\n- **What to do next** \u2014 a priority-ordered punch list for the next session, with\n  a clear \"I'd start with #1\" recommendation.\n\nWrite it so a stranger could act on it cold: real file paths, real command\nlines, and the *why* behind choices, not just the *what*.\n\nIf a session produces a bulky artifact worth keeping verbatim (a long build log,\na captured transcript), drop it in the session dir next to the HANDOFF and link\nto it \u2014 but the HANDOFF stays the single narrative spine.\n\n## Naming\n\n`NNN-<short-slug>/`\n\n`NNN` is a global, monotonically-increasing session number across the whole life\nof the project, starting at `001`. The slug should hint at the substantive\ncontent (`001-bring-up`, `004-cd-boot`) so the sessions are skimmable from a\nplain `ls`. Numbers never reset and never reflect anything but order \u2014 one\nsession is one directory.\n\n## Start-of-session checklist\n\n1. **Read the last 1\u20133 `HANDOFF.md` files** under `docs/sessions/` \u2014 newest\n   first. This is the primary catch-up. The most recent one supersedes anything\n   ambiguous in older ones.\n2. Glance at [`../plan.md`](../plan.md) for mission and current priorities.\n3. <!-- Add any first-session / fresh-clone setup here, e.g. \"run scripts/fetch-upstream.sh if external/ is empty\". Delete if N/A. -->\n4. Confirm the baseline builds/passes before changing anything load-bearing.\n\n## End-of-session ritual\n\n1. Commit the actual work.\n2. **Write this session's `HANDOFF.md`** \u2014 every session, not just the eventful\n   ones. The only valid reason to skip is that the project has genuinely run out\n   of roadmap; even then, a HANDOFF saying exactly that beats silence.\n3. Update [`../plan.md`](../plan.md) if priorities or scope shifted.\n4. <!-- If the project has a user-facing README with a status table / releases section, add: \"Update the project README for anything user-visible that changed.\" Delete if N/A. -->\n5. **Print the next session's HANDOFF path** in your closing summary as a\n   clickable relative link, so the user can paste it into the next session's\n   opening prompt.\n6. Commit the session notes.\n"
}


---

## 🤖 Claude

The skill is now registered. Now the CLAUDE.md and plan.md templates:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/CLAUDE.md.template",
  "content": "<!--\n  TEMPLATE: CLAUDE.md (project root)\n  Fill every <PLACEHOLDER> with real content; CUT any section that doesn't apply\n  to this project rather than leaving it generic. Add bespoke sections at the end\n  for whatever this project specifically needs (release process, linking/packaging\n  strategy, house style, cross-project coordination, etc.) \u2014 that's expected.\n  Delete these HTML comments when done.\n-->\n# Claude agent instructions for this project\n\n## What this project is\n\n<ONE OR TWO PARAGRAPHS: what this project is, why it exists, and what \"done\"\nlooks like. Keep it tight \u2014 the full version lives in docs/plan.md.>\n\nProject brief: [`docs/plan.md`](docs/plan.md).\n\n## Repo layout\n\n```\n<ANNOTATED TREE \u2014 list the dirs/files that matter and one-line what each is for.\nExample:\ndocs/\n  plan.md          \u2014 project brief: goals, non-goals, strategy, roadmap.\n  sessions/        \u2014 one dir per session, globally numbered.\n    README.md      \u2014 sessions workflow. Read this first.\n    NNN-<slug>/    \u2014 session dirs, each with a HANDOFF.md.\n  convos/          \u2014 raw conversation transcripts (optional).\nsrc/               \u2014 <...>\nscripts/           \u2014 <...>\n>\n```\n\n## Sessions workflow\n\n**Read [`docs/sessions/README.md`](docs/sessions/README.md) before starting\nwork \u2014 that's the workflow you must follow.** The short version: substantive work\nlives in `docs/sessions/NNN-<slug>/`, and **each session writes one\n`HANDOFF.md`** that serves as its running log, findings, and primer for the next\nsession.\n\nTo get up to speed at the start of a session, **read the last few HANDOFFs**\n(newest first). They are this project's memory.\n\n<!-- KEEP THE FOLLOWING SECTIONS ONLY IF THEY APPLY. Multi-machine projects want\n     Fleet hosts + Risk tolerance; a solo-laptop project should cut both. -->\n\n## Fleet hosts\n\n<LIST EACH MACHINE AND ITS ROLE. Include the ssh alias and any per-host quirk.\nExample:\n- **uranium** \u2014 main dev Mac (this machine). Edit code, drive builds, publish.\n- **<build-host>** \u2014 build host (`ssh <alias>`). <toolchain / quirks>.\n- **<test-host>** \u2014 test target (`ssh <alias>`). <what it verifies>.\n>\n\n## Risk tolerance by host\n\n<Where can a future agent take chances, and where must it be careful?\nExample:\n- **<dev machine>** \u2014 low. Standard installs OK; no random binaries off the net.\n- **<reinstallable test machines>** \u2014 high. Experiment freely; the bar is \"will\n  this teach us something?\" not \"is this provably safe?\"\n>\n\n## Build & test\n\n<The canonical build command and the canonical verify/test command, so future\nsessions don't have to rediscover them. Include where the build runs if it's a\nremote host, and how artifacts come back.>\n\n## Document everything\n\nDefault to capturing liberally in the active session's `HANDOFF.md`. Decisions,\ndead ends, ambiguity, gotchas-in-the-moment \u2014 all worth writing down. Err on the\nside of MORE capture, not less. The goal is that a future session can pick up the\nthread without re-explaining.\n\n<!-- KEEP if the user sometimes runs the agent unattended. Otherwise cut. -->\n## Unsupervised mode\n\nWhen the user says \"work unsupervised\" (or similar), they're unreachable and\ncannot answer questions. Don't stop to ask \u2014 unblock yourself: make reasonable\nassumptions, run experiments, search for prior art, try the obvious fixes. Long\nruntimes are fine. Only block for genuinely irreversible or unreasonable actions.\n**Log every judgment call** in the active session's HANDOFF \u2014 that log is what\nthe user reviews on return.\n\n## Key invariants\n\n<THE MOST IMPORTANT, MOST PROJECT-SPECIFIC SECTION. The must-not-break rules a\nfuture agent could plausibly violate without realizing. Number them. Examples\nfrom sibling projects:\n1. \"patches/ is canonical; external/<upstream>/ is a gitignored working copy.\"\n2. \"Cross-compile only \u2014 never attempt a native build on the target.\"\n3. \"Releases go to three places: GitHub, leopard.sh, and the README table.\"\nWrite the ones that are true for THIS project, and say WHY each matters.>\n"
}


---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/plan.md.template",
  "content": "<!--\n  TEMPLATE: docs/plan.md (the project brief)\n  This is the durable \"why and what\" of the project \u2014 the thing a HANDOFF points\n  back to for the big picture. Keep it current as scope shifts; it's not a\n  write-once document. For a small project, the roadmap and deferred items can\n  live here as sections (don't split into separate roadmap.md / deferred.md\n  unless the project gets big enough to warrant it). Delete these HTML comments.\n-->\n# <PROJECT NAME> \u2014 project plan\n\n## Mission\n\n<What this project is and why it exists. The fuller version of the CLAUDE.md\n\"What this project is\" paragraph. What does success / \"1.0\" / \"done\" look like?>\n\n## Non-goals\n\n<What this project is deliberately NOT doing. This is as valuable as the goals \u2014\nit stops future sessions from scope-creeping. Be specific.>\n\n## Strategy / approach\n\n<The shape of how the work gets done. Key technical decisions and their\nrationale. The constraints that drive the design (target platform, compatibility\nrequirements, upstream-tracking discipline, etc.).>\n\n## Roadmap\n\n<Prioritized forward work. Milestones or a punch list \u2014 rough is fine, but\nordered. Example:\n- **M1** \u2014 <first shippable milestone>.\n- **M2** \u2014 <next>.\n- **M3** \u2014 <later>.\n>\n\n## Open questions / deferred\n\n<Things parked for later, unresolved questions, follow-ups. The stuff that would\notherwise get lost between sessions. Keep this pruned as items get resolved or\npromoted into the roadmap.>\n"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup && find . -type f | sort && echo \"---\" && wc -l SKILL.md assets/*",
  "description": "Verify skill file structure"
}


---

## 🤖 Claude

The skill is drafted. Here's what it contains:

**`~/.claude/skills/project-workflow-setup/`**
- **SKILL.md** — the workflow: a pushy description for triggering, the one-breath convention summary, the *why* behind HANDOFF-only (so a future agent can defend it rather than re-splitting into README+HANDOFF), a 3-step process (interview → scaffold → confirm), and the HANDOFF.md shape.
- **assets/sessions-README.md.template** — the workflow spec (most reusable; nearly identical across projects).
- **assets/CLAUDE.md.template** — agent instructions, with cut-if-N/A sections (Fleet hosts, Risk tolerance, Unsupervised mode) and a strong emphasis on the project-specific **Key invariants**.
- **assets/plan.md.template** — mission / non-goals / strategy / roadmap / deferred, in one file.

Two deliberate design choices baked in, both from this conversation:
- **HANDOFF-only** is stated as the convention *and* justified — the skill explains that the README+HANDOFF split is overhead that doesn't get read, so future agents won't "helpfully" reintroduce it.
- **"Read the last few HANDOFFs to get up to speed"** is the canonical catch-up directive, surfaced in both the SKILL and the sessions-README template.
- Interview-first: the templates explicitly are "starting points, not fill-in-the-blank forms," with `<PLACEHOLDER>` values and `<!-- filler instructions -->` that get deleted.

**On testing it:** the skill-creator flow normally runs a subagent eval matrix, but since you've already lined up the perfect live test — applying it to memtestppc+ — I'd suggest we **dogfood it here** as iteration 1: I invoke the skill on this project, you review the generated `CLAUDE.md` / `docs/sessions/README.md` / `docs/plan.md`, and we refine the skill from what the real output looks like. That's more signal than synthetic evals for a scaffolding skill, and it gets your actual project files done in the same pass.

Want me to proceed that way — run the skill against memtestppc+ now? (If you'd rather I set up the formal benchmark with separate test projects first, I can do that instead.)

---

## 👤 User

> "Read the last few HANDOFFs to get up to speed" is the canonical
ah, maybe let's relax that a bit, I just mentioned it offhand, but for a typical workflow, reading several sessions would use a lot of tokens.  canonical should probably be "./CLAUDE.md (automatic), ./README.md and most recent HANDOFF.md"

---

## 🤖 Claude

Good catch — reading several HANDOFFs per session would be expensive, and it undercuts the discipline of writing a self-sufficient handoff. Let me relax it across the skill: catch-up = `CLAUDE.md` (auto-loaded) + `README.md` + the single most recent `HANDOFF.md`.

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "It produces three files, tailored to the specific project:\n\n- `CLAUDE.md` \u2014 agent instructions: what the project is, how to work in it, what\n  must not break.",
  "new_string": "The whole system rests on one habit: **every working session writes a single\n`HANDOFF.md`** in its own numbered directory, and the next session gets up to\nspeed cheaply \u2014 from `CLAUDE.md` (loaded automatically), the project `README.md`,\nand **the single most recent `HANDOFF.md`**.\n\nIt produces three files, tailored to the specific project:\n\n- `CLAUDE.md` \u2014 agent instructions: what the project is, how to work in it, what\n  must not break."
}


---

## 🤖 Claude

I created a duplicate sentence. Let me read the current top of the file and fix it.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "limit": 40
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "This skill establishes a lightweight, durable workflow for a project so that any\nfuture session (a fresh Claude, or the user weeks later) can pick up the thread\nwithout re-explaining. The whole system rests on one habit: **every working\nsession writes a single `HANDOFF.md`** in its own numbered directory, and the\nnext session starts by **reading the last few HANDOFFs**.\n\nThe whole system rests on one habit: **every working\nsession writes a single `HANDOFF.md`** in its own numbered directory, and the\nnext session gets up to speed cheaply \u2014 from `CLAUDE.md` (loaded automatically),\nthe project `README.md`, and **the single most recent `HANDOFF.md`**.",
  "new_string": "This skill establishes a lightweight, durable workflow for a project so that any\nfuture session (a fresh Claude, or the user weeks later) can pick up the thread\nwithout re-explaining. The whole system rests on one habit: **every working\nsession writes a single `HANDOFF.md`** in its own numbered directory, and the\nnext session gets up to speed cheaply \u2014 from `CLAUDE.md` (loaded automatically),\nthe project `README.md`, and **the single most recent `HANDOFF.md`**."
}


---

## 🤖 Claude

The duplicate paragraph also dropped some words. Let me fix both with the exact current text.

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "The whole system rests on one habit: **every working\nsession writes a single `HANDOFF.md`** in its own numbered directory, and the\nnext session starts by **reading the last few HANDOFFs**.\n\nThe whole system rests on one habit: **every working\n`HANDOFF.md`** in its own numbered directory, and the next session gets up to\nspeed cheaply \u2014 from `CLAUDE.md` (loaded automatically), the project `README.md`,\nand **the single most recent `HANDOFF.md`**.",
  "new_string": "The whole system rests on one habit: **every working session writes a single\n`HANDOFF.md`** in its own numbered directory, and the next session gets up to\nspeed cheaply \u2014 from `CLAUDE.md` (loaded automatically), the project `README.md`,\nand **the single most recent `HANDOFF.md`**."
}


---

## 🤖 Claude

The multi-line matches keep failing on whitespace/dash ambiguity. Let me re-read the full file and rewrite it cleanly.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md"
}


---

## 🤖 Claude

The file now shows the correct paragraph at lines 25–28 but the duplicate at 21–23 remains, plus two other spots still say "last few HANDOFFs." I'll rewrite the file cleanly with all three fixes.

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "content": "---\nname: project-workflow-setup\ndescription: >-\n  Scaffolds the session-based documentation workflow for a project \u2014 a CLAUDE.md\n  (agent instructions), docs/sessions/README.md (the workflow spec), and\n  docs/plan.md (the project brief), following the HANDOFF-only convention where\n  each session writes one docs/sessions/NNN-slug/HANDOFF.md that doubles as\n  running log, findings, and primer for the next session. Use this whenever the\n  user wants to set up project conventions, bootstrap a new (or newly-adopted)\n  repo, \"add a CLAUDE.md\", establish a sessions/handoff workflow, start tracking\n  work in numbered session directories, or standardize an existing project to\n  match the sibling-project workflow \u2014 even if they don't name the specific files.\n  Also use when retrofitting these conventions onto a project that already has\n  some work in it.\n---\n\n# Project workflow setup\n\nThis skill establishes a lightweight, durable workflow for a project so that any\nfuture session (a fresh Claude, or the user weeks later) can pick up the thread\nwithout re-explaining.\n\nThe whole system rests on one habit: **every working session writes a single\n`HANDOFF.md`** in its own numbered directory, and the next session gets up to\nspeed cheaply \u2014 from `CLAUDE.md` (loaded automatically), the project `README.md`,\nand **the single most recent `HANDOFF.md`**.\n\nIt produces three files, tailored to the specific project:\n\n- `CLAUDE.md` \u2014 agent instructions: what the project is, how to work in it, what\n  must not break.\n- `docs/sessions/README.md` \u2014 the workflow spec: session-dir conventions, the\n  start-of-session and end-of-session rituals.\n- `docs/plan.md` \u2014 the project brief: mission, non-goals, strategy, roadmap.\n\nTemplates for all three live in `assets/`. They are starting points, **not**\nfill-in-the-blank forms \u2014 read them, then adapt.\n\n## The convention in one breath\n\n- Substantive work lives in `docs/sessions/NNN-<short-slug>/`, globally numbered\n  from `001`, monotonically increasing for the life of the project.\n- Each session dir contains **one** `HANDOFF.md`. It is the running log *and* the\n  findings *and* the handoff to the next session \u2014 not three separate files.\n- A session gets up to speed from `CLAUDE.md` (auto-loaded), the project\n  `README.md`, and **the single most recent `HANDOFF.md`** \u2014 not a stack of old\n  ones. It ends by writing this session's HANDOFF.\n- `CLAUDE.md` points at `docs/sessions/README.md` for the workflow and at\n  `docs/plan.md` for the mission.\n\n## Why HANDOFF-only (the reasoning, so you can defend it)\n\nThe sibling projects this is borrowed from originally split each session into a\n`README.md` (narrative of what happened) plus a `HANDOFF.md` (primer for next\ntime). In practice that split is overhead: the two documents say nearly the same\nthing, and the narrative rarely gets read on its own. Collapsing to a single\n`HANDOFF.md` per session keeps the value (a future reader can reconstruct the\nthinking) without the bookkeeping.\n\nCatch-up is deliberately cheap: a new session reads `CLAUDE.md` (auto-loaded),\nthe project `README.md`, and **only the most recent `HANDOFF.md`** \u2014 not the\nwhole stack, which would burn tokens fast on a long-lived project. The corollary\nis a writing discipline: **each HANDOFF must stand on its own** as a primer. If\nsomething from three sessions ago still matters, carry it forward \u2014 restate it,\nor point to the older session dir \u2014 rather than assuming the next reader has seen\nit.\n\nSo: don't recreate the README/HANDOFF split. One file per session. If a session\nproduces a heavy artifact worth keeping verbatim (a long build log, a captured\ntranscript), drop it in the session dir alongside the HANDOFF and reference it \u2014\nbut the HANDOFF stays the single narrative spine.\n\n## Step 1 \u2014 Interview before you scaffold\n\nThe templates are worthless filled with generic boilerplate. The value is in the\nproject-specific content, so gather it first. Pull what you can from the\nconversation and the repo (read an existing README, `git log`, the directory\ntree) before asking, then ask the user only for what's genuinely missing:\n\n- **Mission** \u2014 one paragraph: what is this, why does it exist, what does \"done\"\n  look like? (\u2192 `docs/plan.md`, and a shorter form in `CLAUDE.md`.)\n- **Non-goals** \u2014 what this project deliberately is *not* doing. Saves future\n  sessions from scope creep.\n- **Hosts / fleet** \u2014 what machines are involved and their roles (dev box, build\n  host, test target). Include how to reach them (ssh aliases) and any per-host\n  quirks. Skip this section entirely for a single-machine project.\n- **Build & test commands** \u2014 the canonical way to build and to verify. Future\n  sessions should not have to rediscover these.\n- **Risk tolerance** \u2014 are some machines reinstallable/experimental (high\n  tolerance) while others are the daily driver (low)? This tells a future agent\n  where it can take chances.\n- **Key invariants** \u2014 the must-not-break rules specific to this project (e.g.\n  \"patches/ is canonical\", \"never commit external/\", \"cross-compile only\"). These\n  are the most valuable and most project-specific part of `CLAUDE.md`.\n- **Roadmap** \u2014 the prioritized forward work, even if rough (M1/M2/M3 or a punch\n  list). (\u2192 `docs/plan.md`.)\n\nIf the project already has working sessions but no convention docs (a\nretrofit), also skim the newest sessions/handoffs so the scaffold reflects how\nwork is *actually* being done, not an idealized version.\n\n## Step 2 \u2014 Scaffold the files\n\n1. `mkdir -p docs/sessions` (and `docs/convos` if the user keeps raw transcripts).\n2. Copy `assets/sessions-README.md.template` \u2192 `docs/sessions/README.md` and fill\n   it in. This one is the most reusable \u2014 mostly the same across projects.\n3. Copy `assets/CLAUDE.md.template` \u2192 `CLAUDE.md` and fill it in. Drop sections\n   that don't apply (e.g. Fleet hosts for a solo-machine project); add\n   project-specific sections (release process, linking strategy, house style \u2014\n   whatever this project needs). The sibling projects all grew bespoke sections;\n   that's expected and good.\n4. Copy `assets/plan.md.template` \u2192 `docs/plan.md` and fill it in.\n\nPlaceholders are written as `<ANGLE_BRACKET_CAPS>`. Instructions meant for *you*\n(the filler), not the final reader, are in `<!-- HTML comments -->` \u2014 delete them\nonce the section is written. Don't leave a placeholder unfilled; if a section has\nno real content, cut the section.\n\n## Step 3 \u2014 Surface and confirm\n\nShow the user what you created (brief \u2014 the file paths and a one-line summary of\neach). Offer to seed the first session dir (`docs/sessions/001-<slug>/HANDOFF.md`)\nif this is a brand-new project and there's already work to record, or note that\nthe next real working session will create it.\n\n## What a HANDOFF.md should contain\n\nThe `docs/sessions/README.md` template documents this in full for the project,\nbut the shape is:\n\n- `# Session NNN: <topic>` title.\n- **What happened** \u2014 the running log: what was done, in roughly chronological\n  order, including decisions made, dead-ends hit, and gotchas discovered. The\n  \"findings\" live here \u2014 don't make a separate findings file.\n- **Current state** \u2014 status on exit. What works, what's half-done, where the\n  artifacts are now.\n- **What to do next** \u2014 a priority-ordered punch list for the next session.\n\nKeep it honest and specific (file paths, command lines, the *why* behind\nchoices). Because the next session reads only the most recent HANDOFF by default,\na good one stands on its own \u2014 it carries forward any still-relevant state rather\nthan assuming earlier HANDOFFs were read.\n"
}


---

## 🤖 Claude

Now fix the same directive in the sessions-README template (the "Why a single HANDOFF" blurb and the start-of-session checklist):

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/sessions-README.md.template",
  "old_string": "The `HANDOFF.md` is the session's **running log, its findings, and its primer\nfor the next session, all in one file**. We deliberately do not split narrative\nand handoff into separate documents \u2014 the overhead isn't worth it, and one file\nis what actually gets read. The HANDOFFs, read in sequence, *are* this project's\nmemory. So the catch-up directive is simply:\n\n> **Read the last few HANDOFFs to get up to speed.**",
  "new_string": "The `HANDOFF.md` is the session's **running log, its findings, and its primer\nfor the next session, all in one file**. We deliberately do not split narrative\nand handoff into separate documents \u2014 the overhead isn't worth it, and one file\nis what actually gets read. So catch-up at the start of a session is deliberately\ncheap:\n\n> **`CLAUDE.md` (loaded automatically) + the project `README.md` + the single\n> most recent `HANDOFF.md`.**\n\nWe don't read a stack of old HANDOFFs \u2014 that would burn tokens fast once the\nproject has run for a while. The trade-off is a writing discipline: **each\nHANDOFF must stand on its own.** Carry forward any still-relevant state from\nearlier sessions into the current one (restate it, or link the older session\ndir) rather than assuming the next reader has seen it."
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/sessions-README.md.template",
  "old_string": "1. **Read the last 1\u20133 `HANDOFF.md` files** under `docs/sessions/` \u2014 newest\n   first. This is the primary catch-up. The most recent one supersedes anything\n   ambiguous in older ones.",
  "new_string": "1. **Read `CLAUDE.md` (loaded automatically), the project `README.md`, and the\n   most recent `docs/sessions/NNN-*/HANDOFF.md`.** That's the standard catch-up \u2014\n   the latest HANDOFF is written to stand on its own. Only dig into older\n   HANDOFFs if the latest one points you at something specific."
}


---

## 🤖 Claude

And the CLAUDE.md template's "get up to speed" line:

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/CLAUDE.md.template",
  "old_string": "To get up to speed at the start of a session, **read the last few HANDOFFs**\n(newest first). They are this project's memory.",
  "new_string": "To get up to speed at the start of a session: this `CLAUDE.md` (loaded\nautomatically), the project `README.md`, and **the single most recent\n`HANDOFF.md`**. Each HANDOFF is written to stand on its own, so you don't\nnormally need to read older ones."
}


---

## 🤖 Claude

Let me verify no stale "last few/1–3 HANDOFFs" phrasing remains anywhere in the skill.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup && grep -rni \"last few\\|1\u20133\\|1-3 hand\\|last 1\\|read.*the last\" . || echo \"CLEAN \u2014 no stale catch-up phrasing\"",
  "description": "Check for stale catch-up phrasing"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup && grep -rni \"last few HANDOFF\\|HANDOFFs to get\\|1\u20133\\|last 1\" . ; echo \"exit=$? (1 = no matches = clean)\"",
  "description": "Re-check for stale phrasing with exit status"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup && (grep -rni \"last few HANDOFF\\|HANDOFFs to get\\|1\u20133\\|last 1\" . && echo \"FOUND STALE\") || echo \"CLEAN \u2014 no stale catch-up phrasing remains\"",
  "description": "Final clean-phrasing check"
}


---

## 🤖 Claude

Fixed and verified. The catch-up directive is now consistent across all four files (SKILL.md + the three templates):

> **`CLAUDE.md` (auto-loaded) + the project `README.md` + the single most recent `HANDOFF.md`.**

I also baked in the corollary that makes the cheap catch-up actually work: **each HANDOFF must stand on its own** — carry forward still-relevant state from earlier sessions rather than assuming a stack of old handoffs gets read. That reasoning is now in both the SKILL's "Why HANDOFF-only" section and the sessions-README template, so a future agent understands *why* it's one-most-recent and won't drift back to "read everything." (And I cleaned up a duplicated intro paragraph that crept in during editing.)

The skill is in good shape. Ready to dogfood it on memtestppc+ whenever you'd like — that run will be the real test of whether the templates produce good output, and it gets your actual `CLAUDE.md` / `docs/sessions/README.md` / `docs/plan.md` written in the same pass. Want me to go ahead and invoke it on this project?

---

## 👤 User

this is taking good shape.  a bit of feedback:

> (and docs/convos if the user keeps raw transcripts).
you can strike this -- docs/convos is managed by me externally to each project (I just cd ~/claude/convos and type "make").  these are part of every project, but the claude sessions don't need to know anything about them / maintain them in any way

> plan.md
we might as well make this PLAN.md to be consistent (don't worry about retrofitting the sibling projects, I'll do that later)

I think we should also include some bits about what makes for a good project-level README.

some README's to read (we have plenty of context window left, so read these in full) (not all of these are necessarily examples of "what to do"):

* ghc-darwin8-ppc
* golang-darwin8-ppc
* ionpower-node
* llvm-7-darwin-ppc
* lumo-darwin8-ppc
* rogerppc

bits which I like in these READMEs (these won't necessarily make sense for every project):

* first thing: logo and what is this thing?
* "try it out!" / quick start: simplest possible install + hello world
* detailed status table and ✅ / 🟡 / ❌
   * several of these projects have a separate Status and Implementation Status, not really sure how I like that split.  If split, Status should probably aim for no more than say, 3 sentences.
* table of releases as a basic changelog
* demos
* build-from-source instructions (break out to BUILDING.md if more than half a page)
* project layout
* license

"why" feels a bit weak in these README's, feels unneccesary.  I made it because it would be awesome if it existed, no need to explain.

"sibling projects" meh, I'll focus on that when these are bit more mature, this section feels premature

---

## 👤 User

[Request interrupted by user]

---

## 👤 User

this is taking good shape.  a bit of feedback:

> (and docs/convos if the user keeps raw transcripts).
you can strike this -- docs/convos is managed by me externally to each project (I just cd ~/claude/convos and type "make").  these are part of every project, but the claude sessions don't need to know anything about them / maintain them in any way

> plan.md
we might as well make this PLAN.md to be consistent (don't worry about retrofitting the sibling projects, I'll do that later)

I think we should also include some bits about what makes for a good project-level README.

some README's to read (we have plenty of context window left, so read these in full) (not all of these are necessarily examples of "what to do"):

* ghc-darwin8-ppc
* golang-darwin8-ppc
* ionpower-node
* llvm-7-darwin-ppc
* lumo-darwin8-ppc
* rogerppc
* tcc-darwin8-ppc


bits which I like in these READMEs (these won't necessarily make sense for every project):

* first thing: logo and what is this thing?
* "try it out!" / quick start: simplest possible install + hello world
* detailed status table and ✅ / 🟡 / ❌
   * several of these projects have a separate Status and Implementation Status, not really sure how I like that split.  If split, Status should probably aim for no more than say, 3 sentences.
* table of releases as a basic changelog
* demos
* build-from-source instructions (break out to BUILDING.md if more than half a page)
* project layout
* license

"why" feels a bit weak in these README's, feels unneccesary.  I made it because it would be awesome if it existed, no need to explain.

"sibling projects" meh, I'll focus on that when these are bit more mature, this section feels premature

ghc's CLAUDE.md has good sections on '## Release workflow' and '## README format'

llvm-7-darwin-ppc/CLAUDE.md also has good '## Release workflow' '## README format' '## Sessions workflow' etc.  '## Repo layout' is also useful to have in CLAUDE.md so a new session can orient itself.

read the above, give me your thoughts, let's reach a shared understanding before you proceed

---

## 🤖 Claude

Let me read all the requested files in full before forming an opinion.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/ghc-darwin8-ppc/README.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/ghc-darwin8-ppc/CLAUDE.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/llvm-7-darwin-ppc/README.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/llvm-7-darwin-ppc/CLAUDE.md"
}


---

## 🤖 Claude

Now the remaining READMEs: golang, ionpower-node, lumo, rogerppc, tcc.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/golang-darwin8-ppc/README.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/ionpower-node/README.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/lumo-darwin8-ppc/README.md"
}


---

## 🤖 Claude

The golang README is huge (36K tokens — itself a signal). Let me read its top structure, plus rogerppc and tcc in full.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/golang-darwin8-ppc/README.md",
  "limit": 130
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/tcc-darwin8-ppc/README.md"
}


---

## 🤖 Claude

Finally, rogerppc:

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/rogerppc/README.md"
}


---

## 🤖 Claude

I've read all of them in full (golang only partially — but its sheer size, 36K tokens, is itself the lesson). Here's my synthesis and a concrete proposal. No edits yet — let's align first.

## What the good READMEs share (the skeleton)

`tcc`, `ionpower-node`, `llvm-7`, `lumo` are the cleanest. The recurring top-to-bottom shape:

1. **Logo + one-liner** — `![](docs/media/…)`, H1, then 1–2 sentences of "what is this." Sometimes a tagline (`_vibe-coded with Claude_`).
2. **Try it out! / Quick start** — copy-pasteable *prebuilt* install + hello-world. The best ones (ionpower, llvm, lumo) let a stranger paste 4 lines and see it run. This is the highest-value section.
3. **Status** — short prose: current state + latest-release callout.
4. **Implementation status** — the ✅/🟡/❌ tables, one per area. The heart of a maturing project.
5. **Demos** — table of runnable showcases.
6. **Releases** — `Tag | Date | Headline` changelog table.
7. **Build from source** — short, or a pointer to `BUILDING.md` (ionpower breaks out `BUILDING.md` *and* `TESTING.md`).
8. **Layout** — annotated tree.
9. **License.**

## The anti-pattern to actively warn against

`golang`'s README (and `ghc`'s to a lesser degree) shows what happens without discipline: the **Status section metastasizes into a prose changelog** — "Pass 130… Pass 131… Pass 139…" narrated inline, plus Notes-column cells that run for paragraphs (ghc's GHCi-REPL row is ~40 lines in one table cell). Ironically, ghc/llvm's *own* `## README format` rule says "each Notes cell is one or two sentences; anything longer goes in the linked session." So the rule is right but easy to let rot. The skill should encode the rule **and** name the failure mode: the releases table + linked sessions are where blow-by-blow detail lives; the status tables stay scannable.

## On your specific questions

- **Status vs Implementation status split:** my read — keep it *combined* for small/new projects (one `## Status`: a few sentences + the table). Only split once the table is large: then `## Status` = ≤3 sentences (current state + latest release) and `## Implementation status` = the tables. I'll encode exactly that threshold.
- **Drop "Why":** agree for the *existence-justification* kind (rogerppc's "Why PowerPC", generic "why this exists"). But note two of them — ionpower's "Why" (reusing TenFourFox's JIT) and golang's "Why Go 1.15?" — are really *load-bearing design-decision rationale*, not justification. Those are worth keeping, just better framed as a specific "Why \<this choice\>?" heading or folded into `PLAN.md`. So: drop generic "Why"; keep decision-rationale where it's real.
- **Drop "Sibling projects":** agree, premature. Leave it out of the template.

## Division of labor (so the three docs don't overlap badly)

- **README.md** = user-facing: what / quickstart / status table / releases / demos / build / layout / license.
- **CLAUDE.md** = agent-facing: how to work here, repo layout, sessions workflow, **README format** rules, **release workflow**, invariants.
- **PLAN.md** = the brief: mission / non-goals / strategy / roadmap.
- `## Repo layout` lives in **both** README (user "Layout") and CLAUDE.md (agent orientation) — you explicitly wanted it in CLAUDE.md, and the slight overlap is fine since they serve different readers.

## Concrete changes I'd make to the skill

1. **Strike** the `docs/convos` parenthetical in SKILL.md Step 2 (it's externally managed; sessions never touch it).
2. **Rename** `plan.md` → `PLAN.md` throughout SKILL.md, and the asset `plan.md.template` → `PLAN.md.template`.
3. **New asset `README.md.template`** — the skeleton above, with the ✅/🟡/❌ legend, releases-table changelog, BUILDING.md break-out note; **no** "Why" or "Sibling projects" sections.
4. **CLAUDE.md.template** gains two optional sections borrowed from ghc/llvm: `## README format` (the emoji legend + "keep Notes scannable" rule) and `## Release workflow` (demo + README-status-update + releases-row + tag, all in the version commit) — marked "include if the project ships releases / maintains a status README."
5. **SKILL.md** gains a short "README" subsection: the skeleton, the anti-pattern warning, the combined-vs-split rule, and an interview note (logo? simplest quickstart path? does it ship releases?). README starts minimal for a brand-new project and grows the table/releases/demos sections as things ship.

Does that match your thinking — particularly (a) keeping decision-rationale "Why" but killing justification-"Why", and (b) the combined-status-default / split-when-large rule? If so I'll apply all five.

---

## 👤 User

sounds great.

yes, good point about golang, the section structure is good, but too many rows in each, and there are now walls of text which should be short paragraphs.  definitely metastasized.

proceed.

---

## 🤖 Claude

Approved — applying all five. Let me track this and start with the file rename.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup/assets && mv plan.md.template PLAN.md.template && ls",
  "description": "Rename plan template to PLAN"
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/PLAN.md.template",
  "old_string": "<!--\n  TEMPLATE: docs/plan.md (the project brief)",
  "new_string": "<!--\n  TEMPLATE: docs/PLAN.md (the project brief)"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/PLAN.md.template"
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/PLAN.md.template",
  "old_string": "  TEMPLATE: docs/plan.md (the project brief)\n  This is the durable \"why and what\" of the project \u2014 the thing a HANDOFF points\n  back to for the big picture. Keep it current as scope shifts; it's not a\n  write-once document. For a small project, the roadmap and deferred items can\n  live here as sections (don't split into separate roadmap.md / deferred.md\n  unless the project gets big enough to warrant it). Delete these HTML comments.",
  "new_string": "  TEMPLATE: docs/PLAN.md (the project brief)\n  This is the durable \"why and what\" of the project \u2014 the thing a HANDOFF points\n  back to for the big picture. Keep it current as scope shifts; it's not a\n  write-once document. For a small project, the roadmap and deferred items can\n  live here as sections (don't split into separate ROADMAP.md / DEFERRED.md\n  unless the project gets big enough to warrant it). This is also the right home\n  for load-bearing decision rationale (\"why this language version\", \"why this\n  upstream pin\") that doesn't belong in the user-facing README. Delete these\n  HTML comments."
}


---

## 🤖 Claude

Now the new README.md.template asset:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/README.md.template",
  "content": "<!--\n  TEMPLATE: README.md (project root, user-facing)\n  This is the front door for a human who lands on the repo. Different audience\n  from CLAUDE.md (that's for the agent). Include only the sections that have\n  real content YET \u2014 a brand-new project might be just the logo + one-liner +\n  a \"Status: early, pre-release\" line. Add Quick start / status tables / Releases\n  / Demos as those things actually exist. Grow it as the project ships.\n\n  Two sections the sibling projects tend to include that we deliberately OMIT:\n  - A generic \"Why\" / \"Why this exists\" section. Don't justify the project's\n    existence; if it's worth building, it's worth building. (Exception: a\n    load-bearing *decision* rationale \u2014 \"Why version X\", \"Why this upstream\n    pin\" \u2014 is worth keeping, but put it under a specific heading or in PLAN.md,\n    not a vague \"Why\".)\n  - \"Sibling/related projects\" \u2014 premature until the project is mature.\n\n  Delete these HTML comments when done.\n-->\n<!-- Logo is optional but nice. Drop art at docs/media/ and reference it here. -->\n![](docs/media/<logo>.png)\n\n# <project-name>\n\n<One or two sentences: what this is, for whom, on what platform. The single most\nimportant lines in the file \u2014 a stranger should know in five seconds whether this\nis the thing they want. A short italic tagline on its own line can go here too.>\n\n## Try it out!\n\n<!-- The highest-value section. Give the SIMPLEST path: install a prebuilt\n     artifact and run a hello-world / demo. Copy-pasteable, end-to-end. If there's\n     nothing to install yet, omit this section until there is. -->\n```\n<the fewest commands that get a stranger from nothing to a working demo>\n```\n\n## Status\n\n<!-- Combined-vs-split rule: for a small or new project, ONE Status section \u2014\n     a few sentences of current state + latest-release pointer, then the table(s)\n     below. Only split out a separate \"## Implementation status\" once the tables\n     get large; if you split, keep THIS Status to ~3 sentences (current state +\n     latest release) and move the tables there. -->\n<A few sentences: where the project is right now, and a pointer to the latest\nrelease. Resist the urge to narrate every milestone here \u2014 that's what the\nReleases table and the linked session HANDOFFs are for.>\n\n<!-- Status tables: one per area. Status column legend, in priority order:\n     \u2705 Working   \u2014 lands a green test or a verified end-to-end run.\n     \ud83d\udfe1 Partial   \u2014 works for the common case but has a known gap / needs a\n                    workaround / one variant only. Always add a one-line\n                    \"what's missing\" in Notes.\n     \u274c Missing   \u2014 known not to work, with a pointer to where the fix is scoped.\n     KEEP EACH NOTES CELL TO ONE OR TWO SENTENCES. Anything longer goes in a\n     linked session HANDOFF or PLAN.md. This is the #1 way these tables rot:\n     cells grow into paragraphs and the table stops being scannable. Don't. -->\n\n| Feature | Status | Notes |\n|---|---|---|\n| <feature> | \u2705 Working | <one line> |\n| <feature> | \ud83d\udfe1 Partial | <one line: what's missing> |\n| <feature> | \u274c Missing | <one line: pointer to where it's scoped> |\n\n## Demos\n\n<!-- Optional. A table of runnable showcase pieces, one row each. Omit until you\n     have demos. -->\n| Demo | What it shows |\n|---|---|\n| [`demos/<name>`](demos/<name>) | <one line> |\n\n## Releases\n\n<!-- This table IS the changelog \u2014 keep new detail here (one headline per row)\n     rather than letting the Status section above grow into a prose changelog. -->\n| Tag | Date | Headline |\n|---|---|---|\n| [<vX.Y.Z>](<release-url>) | <YYYY-MM-DD> | <one-line headline of what it unlocked> |\n\n## Build from source\n\n<!-- Keep this to about half a page. If it's longer, move it to BUILDING.md and\n     leave a one-line pointer here. Most users want the prebuilt artifact above,\n     not a source build. -->\n<short build steps, or: \"See [BUILDING.md](BUILDING.md).\">\n\n## Layout\n\n```\n<annotated tree of the dirs/files that matter to someone reading the repo. This\nis the user-facing cousin of CLAUDE.md's \"Repo layout\" \u2014 it can be terser.>\n```\n\n## License\n\n<License + one line on how it relates to any upstream the project builds on.>\n"
}


---

## 🤖 Claude

Now update the CLAUDE.md.template — fix the PLAN.md reference, update the layout, and add the two optional sections. Let me read it first.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/CLAUDE.md.template"
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/CLAUDE.md.template",
  "old_string": "<ONE OR TWO PARAGRAPHS: what this project is, why it exists, and what \"done\"\nlooks like. Keep it tight \u2014 the full version lives in docs/plan.md.>\n\nProject brief: [`docs/plan.md`](docs/plan.md).",
  "new_string": "<ONE OR TWO PARAGRAPHS: what this project is, why it exists, and what \"done\"\nlooks like. Keep it tight \u2014 the full version lives in docs/PLAN.md.>\n\nProject brief: [`docs/PLAN.md`](docs/PLAN.md)."
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/CLAUDE.md.template",
  "old_string": "<ANNOTATED TREE \u2014 list the dirs/files that matter and one-line what each is for.\nExample:\ndocs/\n  plan.md          \u2014 project brief: goals, non-goals, strategy, roadmap.\n  sessions/        \u2014 one dir per session, globally numbered.\n    README.md      \u2014 sessions workflow. Read this first.\n    NNN-<slug>/    \u2014 session dirs, each with a HANDOFF.md.\n  convos/          \u2014 raw conversation transcripts (optional).\nsrc/               \u2014 <...>\nscripts/           \u2014 <...>\n>",
  "new_string": "<ANNOTATED TREE \u2014 list the dirs/files that matter and one-line what each is for.\nExample:\nREADME.md          \u2014 user-facing front door: what it is, quick start, status.\nPLAN.md            \u2014 (or docs/PLAN.md) project brief: goals, non-goals, roadmap.\ndocs/\n  PLAN.md          \u2014 project brief: goals, non-goals, strategy, roadmap.\n  sessions/        \u2014 one dir per session, globally numbered.\n    README.md      \u2014 sessions workflow. Read this first.\n    NNN-<slug>/    \u2014 session dirs, each with a HANDOFF.md.\nsrc/               \u2014 <...>\nscripts/           \u2014 <...>\n>"
}


---

## 🤖 Claude

Now insert the two optional sections (README format + Release workflow) borrowed from ghc/llvm:

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/CLAUDE.md.template",
  "old_string": "<The canonical build command and the canonical verify/test command, so future\nsessions don't have to rediscover them. Include where the build runs if it's a\nremote host, and how artifacts come back.>\n\n## Document everything",
  "new_string": "<The canonical build command and the canonical verify/test command, so future\nsessions don't have to rediscover them. Include where the build runs if it's a\nremote host, and how artifacts come back.>\n\n<!-- KEEP the next two sections only if the project maintains a status-table\n     README and/or ships releases. A book, a one-off script, or a project with\n     nothing to ship yet should cut them. -->\n\n## README format\n\nThe user-facing `README.md` carries a status table (one per area for a big\nproject). Status column legend, in priority order:\n\n- \u2705 Working \u2014 lands a green test or a verified end-to-end run.\n- \ud83d\udfe1 Partial \u2014 works for the common case but has a known gap, needs a workaround,\n  or covers one variant only. Always followed by a one-line \"what's missing\" in\n  Notes.\n- \u274c Missing \u2014 known not to work, with a pointer to where the fix is scoped.\n\n**Keep each Notes cell to one or two sentences, and keep the Status prose short.**\nThis is the single most common way these READMEs rot: the status section grows\ninto a prose changelog (\"then we did X, then Y, then Z\u2026\") and the Notes cells\nswell into paragraphs until the table stops being scannable. Blow-by-blow detail\nbelongs in the linked session HANDOFF or the Releases table \u2014 not here.\n\n## Release workflow\n\n<!-- Adjust to this project's actual release shape. -->\nEvery release ships, in addition to the artifact itself:\n\n1. A **README update** so the front page reflects what shipped \u2014 flip the\n   relevant status rows (\u274c \u2192 \ud83d\udfe1 / \u2705), and add a row to the Releases table.\n2. <A demo / changelog entry / whatever else this project attaches to a release.>\n\nThe release isn't done until those have landed \u2014 artifact + session HANDOFF +\nREADME update + tag, all in the commit that bears the version number.\n\n## Document everything"
}


---

## 🤖 Claude

Now the SKILL.md edits. Let me read the current version to make precise changes.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "offset": 17,
  "limit": 120
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "It produces three files, tailored to the specific project:\n\n- `CLAUDE.md` \u2014 agent instructions: what the project is, how to work in it, what\n  must not break.\n- `docs/sessions/README.md` \u2014 the workflow spec: session-dir conventions, the\n  start-of-session and end-of-session rituals.\n- `docs/plan.md` \u2014 the project brief: mission, non-goals, strategy, roadmap.\n\nTemplates for all three live in `assets/`. They are starting points, **not**\nfill-in-the-blank forms \u2014 read them, then adapt.",
  "new_string": "It produces these files, tailored to the specific project:\n\n- `CLAUDE.md` \u2014 agent instructions: what the project is, how to work in it, what\n  must not break.\n- `docs/sessions/README.md` \u2014 the workflow spec: session-dir conventions, the\n  start-of-session and end-of-session rituals.\n- `docs/PLAN.md` \u2014 the project brief: mission, non-goals, strategy, roadmap.\n- `README.md` \u2014 the user-facing front door: what it is, quick start, status. For\n  a brand-new project this can start as just a logo + one-liner + \"early,\n  pre-release\" status, and grow as things ship. See the README section below.\n\nTemplates for all of these live in `assets/`. They are starting points, **not**\nfill-in-the-blank forms \u2014 read them, then adapt."
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "- `CLAUDE.md` points at `docs/sessions/README.md` for the workflow and at\n  `docs/plan.md` for the mission.",
  "new_string": "- `CLAUDE.md` points at `docs/sessions/README.md` for the workflow and at\n  `docs/PLAN.md` for the mission."
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "- **Mission** \u2014 one paragraph: what is this, why does it exist, what does \"done\"\n  look like? (\u2192 `docs/plan.md`, and a shorter form in `CLAUDE.md`.)",
  "new_string": "- **Mission** \u2014 one paragraph: what is this, why does it exist, what does \"done\"\n  look like? (\u2192 `docs/PLAN.md`, and a shorter form in `CLAUDE.md` + `README.md`.)"
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "- **Roadmap** \u2014 the prioritized forward work, even if rough (M1/M2/M3 or a punch\n  list). (\u2192 `docs/plan.md`.)\n\nIf the project already has working sessions but no convention docs (a",
  "new_string": "- **Roadmap** \u2014 the prioritized forward work, even if rough (M1/M2/M3 or a punch\n  list). (\u2192 `docs/PLAN.md`.)\n- **README essentials** \u2014 is there a logo/art? What's the *simplest* path for a\n  stranger to try it (the quick-start)? Does the project ship releases yet? These\n  shape how much of the `README.md` to fill in now vs. defer. (\u2192 `README.md`.)\n\nIf the project already has working sessions but no convention docs (a"
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "1. `mkdir -p docs/sessions` (and `docs/convos` if the user keeps raw transcripts).\n2. Copy `assets/sessions-README.md.template` \u2192 `docs/sessions/README.md` and fill\n   it in. This one is the most reusable \u2014 mostly the same across projects.\n3. Copy `assets/CLAUDE.md.template` \u2192 `CLAUDE.md` and fill it in. Drop sections\n   that don't apply (e.g. Fleet hosts for a solo-machine project); add\n   project-specific sections (release process, linking strategy, house style \u2014\n   whatever this project needs). The sibling projects all grew bespoke sections;\n   that's expected and good.\n4. Copy `assets/plan.md.template` \u2192 `docs/plan.md` and fill it in.",
  "new_string": "1. `mkdir -p docs/sessions`.\n2. Copy `assets/sessions-README.md.template` \u2192 `docs/sessions/README.md` and fill\n   it in. This one is the most reusable \u2014 mostly the same across projects.\n3. Copy `assets/CLAUDE.md.template` \u2192 `CLAUDE.md` and fill it in. Drop sections\n   that don't apply (e.g. Fleet hosts for a solo-machine project, or README\n   format / Release workflow for a project with nothing to ship yet); add\n   project-specific sections (release process, linking strategy, house style \u2014\n   whatever this project needs). The sibling projects all grew bespoke sections;\n   that's expected and good.\n4. Copy `assets/PLAN.md.template` \u2192 `docs/PLAN.md` and fill it in.\n5. Copy `assets/README.md.template` \u2192 `README.md` and fill it in \u2014 including only\n   the sections that have real content yet (see the README section below)."
}


---

## 🤖 Claude

Now add the README guidance section. Let me read the end of the file.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "offset": 128,
  "limit": 30
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "Keep it honest and specific (file paths, command lines, the *why* behind\nchoices). Because the next session reads only the most recent HANDOFF by default,\na good one stands on its own \u2014 it carries forward any still-relevant state rather\nthan assuming earlier HANDOFFs were read.",
  "new_string": "Keep it honest and specific (file paths, command lines, the *why* behind\nchoices). Because the next session reads only the most recent HANDOFF by default,\na good one stands on its own \u2014 it carries forward any still-relevant state rather\nthan assuming earlier HANDOFFs were read.\n\n## The README\n\n`README.md` is the user-facing front door \u2014 a different audience from `CLAUDE.md`\n(that's for the agent). The `assets/README.md.template` carries the full menu;\nthe proven skeleton, top to bottom:\n\n1. **Logo + one-liner** \u2014 art (optional) then 1\u20132 sentences of what this is, for\n   whom, on what platform. The most-read lines in the repo.\n2. **Try it out! / Quick start** \u2014 the *simplest* path: install a prebuilt\n   artifact, run a hello-world or demo, copy-pasteable end-to-end. Omit until\n   there's something to install.\n3. **Status** \u2014 current state in a few sentences + a pointer to the latest\n   release, then the status table(s).\n4. **Demos**, **Releases** (a `Tag | Date | Headline` changelog table), **Build\n   from source** (half a page max, else break out to `BUILDING.md`), **Layout**,\n   **License**.\n\nThree rules worth internalizing:\n\n- **Start minimal, grow as you ship.** A brand-new project's README is honestly\n  just the logo + one-liner + \"Status: early, pre-release.\" Add Quick start, the\n  status table, Releases, and Demos only once those things exist. Don't scaffold\n  empty sections.\n- **Status default is combined; split only when large.** For a small project use\n  one `## Status` (a few sentences + the table). Only when the tables grow big do\n  you split into `## Status` (\u22643 sentences: current state + latest release) and a\n  separate `## Implementation status` (the tables).\n- **Guard against the metastasis failure mode.** The single most common way these\n  READMEs rot: the Status section grows into a prose changelog (\"then X, then Y,\n  then Z\u2026\") and the table Notes cells swell into paragraphs until nothing is\n  scannable. The status structure stays good; the *rows multiply and the prose\n  walls grow*. Keep Notes cells to one or two sentences; push blow-by-blow detail\n  into the Releases table or the linked session HANDOFF. The maintenance rules\n  live in `CLAUDE.md`'s \"README format\" / \"Release workflow\" sections so they're\n  in front of every future session that touches the README.\n\nDeliberately omitted from the template (don't add them back by reflex): a generic\n**\"Why this exists\"** section (don't justify the project \u2014 though a *specific*\ndecision rationale like \"Why version X\" is worth keeping, in `PLAN.md` or under\nits own heading), and a **\"Sibling/related projects\"** section (premature until\nthe project is mature)."
}


---

## 🤖 Claude

All five changes are in. Let me verify consistency — no lingering lowercase `plan.md` or `convos` references, and the asset set is right.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup && echo \"=== files ===\" && find . -type f | sort && echo && echo \"=== stray 'plan.md' (lowercase) refs ===\" && (grep -rn \"plan\\.md\" . | grep -v \"PLAN.md\" || echo \"none\") && echo && echo \"=== stray 'convos' refs ===\" && (grep -rn \"convos\" . || echo \"none\") && echo && echo \"=== README/PLAN/release mentions present ===\" && grep -rln \"README.md.template\\|PLAN.md.template\\|README format\\|Release workflow\" .",
  "description": "Verify consistency of the skill files"
}


---

## 🤖 Claude

Three stray lowercase refs to fix.

---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": false,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/SKILL.md",
  "old_string": "  docs/plan.md (the project brief), following the HANDOFF-only convention where",
  "new_string": "  docs/PLAN.md (the project brief), following the HANDOFF-only convention where"
}


---

## 🤖 Claude


🔧 Using tool: Edit
Input: {
  "replace_all": true,
  "file_path": "/Users/cell/.claude/skills/project-workflow-setup/assets/sessions-README.md.template",
  "old_string": "[`../plan.md`](../plan.md)",
  "new_string": "[`../PLAN.md`](../PLAN.md)"
}


---

## 🤖 Claude

Let me confirm everything is clean now.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/.claude/skills/project-workflow-setup && (grep -rn \"plan\\.md\" . | grep -v \"PLAN.md\" && echo \"STRAY FOUND\") || echo \"CLEAN: no lowercase plan.md refs\" ",
  "description": "Final lowercase plan.md check"
}


---

## 🤖 Claude

All five changes are applied and verified clean. The `project-workflow-setup` skill now stands at:

- **SKILL.md** — produces four files (now including `README.md`); `docs/convos` reference struck; `plan.md` → `PLAN.md` throughout; a new **README section** covering the skeleton, the start-minimal/grow-as-you-ship rule, the combined-status-default / split-when-large rule, and an explicit warning about the **metastasis failure mode** (status prose becoming a changelog wall, Notes cells swelling to paragraphs — the golang lesson). Interview now asks about logo/quickstart/releases.
- **assets/README.md.template** — new: logo + one-liner → Try it out! → Status (✅/🟡/❌ legend, scannable-rows rule) → Demos → Releases changelog table → Build (½-page-then-BUILDING.md) → Layout → License. Deliberately omits generic "Why" and "Sibling projects," with a comment explaining why.
- **assets/CLAUDE.md.template** — `PLAN.md` reference fixed; layout shows `README.md` + `PLAN.md` (and no longer lists externally-managed `convos/`); gained optional **`## README format`** and **`## Release workflow`** sections (cut-if-N/A), carrying the maintenance rules in front of every future session.
- **assets/PLAN.md.template** — renamed from `plan.md.template`; now also flagged as the home for load-bearing decision rationale ("why version X") that doesn't belong in the user-facing README.
- **assets/sessions-README.md.template** — `PLAN.md` links updated.

The skill description re-registered cleanly (visible in the skills list with the `docs/PLAN.md` wording).

That completes the skill work. Whenever you're ready, the natural next step is to **dogfood it on memtestppc+** — invoke `project-workflow-setup` here to generate this project's `CLAUDE.md` / `docs/sessions/README.md` / `docs/PLAN.md` / `README.md`, which both produces your actual project docs and serves as the skill's first real test.

---

## 👤 User

dogfood me baby!  use the project-workflow-setup skill

---

## 🤖 Claude


🔧 Using tool: Skill
Input: {
  "skill": "project-workflow-setup"
}


---

## 👤 User

Base directory for this skill: /Users/cell/.claude/skills/project-workflow-setup

# Project workflow setup

This skill establishes a lightweight, durable workflow for a project so that any
future session (a fresh Claude, or the user weeks later) can pick up the thread
without re-explaining.

The whole system rests on one habit: **every working session writes a single
`HANDOFF.md`** in its own numbered directory, and the next session gets up to
speed cheaply — from `CLAUDE.md` (loaded automatically), the project `README.md`,
and **the single most recent `HANDOFF.md`**.

It produces these files, tailored to the specific project:

- `CLAUDE.md` — agent instructions: what the project is, how to work in it, what
  must not break.
- `docs/sessions/README.md` — the workflow spec: session-dir conventions, the
  start-of-session and end-of-session rituals.
- `docs/PLAN.md` — the project brief: mission, non-goals, strategy, roadmap.
- `README.md` — the user-facing front door: what it is, quick start, status. For
  a brand-new project this can start as just a logo + one-liner + "early,
  pre-release" status, and grow as things ship. See the README section below.

Templates for all of these live in `assets/`. They are starting points, **not**
fill-in-the-blank forms — read them, then adapt.

## The convention in one breath

- Substantive work lives in `docs/sessions/NNN-<short-slug>/`, globally numbered
  from `001`, monotonically increasing for the life of the project.
- Each session dir contains **one** `HANDOFF.md`. It is the running log *and* the
  findings *and* the handoff to the next session — not three separate files.
- A session gets up to speed from `CLAUDE.md` (auto-loaded), the project
  `README.md`, and **the single most recent `HANDOFF.md`** — not a stack of old
  ones. It ends by writing this session's HANDOFF.
- `CLAUDE.md` points at `docs/sessions/README.md` for the workflow and at
  `docs/PLAN.md` for the mission.

## Why HANDOFF-only (the reasoning, so you can defend it)

The sibling projects this is borrowed from originally split each session into a
`README.md` (narrative of what happened) plus a `HANDOFF.md` (primer for next
time). In practice that split is overhead: the two documents say nearly the same
thing, and the narrative rarely gets read on its own. Collapsing to a single
`HANDOFF.md` per session keeps the value (a future reader can reconstruct the
thinking) without the bookkeeping.

Catch-up is deliberately cheap: a new session reads `CLAUDE.md` (auto-loaded),
the project `README.md`, and **only the most recent `HANDOFF.md`** — not the
whole stack, which would burn tokens fast on a long-lived project. The corollary
is a writing discipline: **each HANDOFF must stand on its own** as a primer. If
something from three sessions ago still matters, carry it forward — restate it,
or point to the older session dir — rather than assuming the next reader has seen
it.

So: don't recreate the README/HANDOFF split. One file per session. If a session
produces a heavy artifact worth keeping verbatim (a long build log, a captured
transcript), drop it in the session dir alongside the HANDOFF and reference it —
but the HANDOFF stays the single narrative spine.

## Step 1 — Interview before you scaffold

The templates are worthless filled with generic boilerplate. The value is in the
project-specific content, so gather it first. Pull what you can from the
conversation and the repo (read an existing README, `git log`, the directory
tree) before asking, then ask the user only for what's genuinely missing:

- **Mission** — one paragraph: what is this, why does it exist, what does "done"
  look like? (→ `docs/PLAN.md`, and a shorter form in `CLAUDE.md` + `README.md`.)
- **Non-goals** — what this project deliberately is *not* doing. Saves future
  sessions from scope creep.
- **Hosts / fleet** — what machines are involved and their roles (dev box, build
  host, test target). Include how to reach them (ssh aliases) and any per-host
  quirks. Skip this section entirely for a single-machine project.
- **Build & test commands** — the canonical way to build and to verify. Future
  sessions should not have to rediscover these.
- **Risk tolerance** — are some machines reinstallable/experimental (high
  tolerance) while others are the daily driver (low)? This tells a future agent
  where it can take chances.
- **Key invariants** — the must-not-break rules specific to this project (e.g.
  "patches/ is canonical", "never commit external/", "cross-compile only"). These
  are the most valuable and most project-specific part of `CLAUDE.md`.
- **Roadmap** — the prioritized forward work, even if rough (M1/M2/M3 or a punch
  list). (→ `docs/PLAN.md`.)
- **README essentials** — is there a logo/art? What's the *simplest* path for a
  stranger to try it (the quick-start)? Does the project ship releases yet? These
  shape how much of the `README.md` to fill in now vs. defer. (→ `README.md`.)

If the project already has working sessions but no convention docs (a
retrofit), also skim the newest sessions/handoffs so the scaffold reflects how
work is *actually* being done, not an idealized version.

## Step 2 — Scaffold the files

1. `mkdir -p docs/sessions`.
2. Copy `assets/sessions-README.md.template` → `docs/sessions/README.md` and fill
   it in. This one is the most reusable — mostly the same across projects.
3. Copy `assets/CLAUDE.md.template` → `CLAUDE.md` and fill it in. Drop sections
   that don't apply (e.g. Fleet hosts for a solo-machine project, or README
   format / Release workflow for a project with nothing to ship yet); add
   project-specific sections (release process, linking strategy, house style —
   whatever this project needs). The sibling projects all grew bespoke sections;
   that's expected and good.
4. Copy `assets/PLAN.md.template` → `docs/PLAN.md` and fill it in.
5. Copy `assets/README.md.template` → `README.md` and fill it in — including only
   the sections that have real content yet (see the README section below).

Placeholders are written as `<ANGLE_BRACKET_CAPS>`. Instructions meant for *you*
(the filler), not the final reader, are in `<!-- HTML comments -->` — delete them
once the section is written. Don't leave a placeholder unfilled; if a section has
no real content, cut the section.

## Step 3 — Surface and confirm

Show the user what you created (brief — the file paths and a one-line summary of
each). Offer to seed the first session dir (`docs/sessions/001-<slug>/HANDOFF.md`)
if this is a brand-new project and there's already work to record, or note that
the next real working session will create it.

## What a HANDOFF.md should contain

The `docs/sessions/README.md` template documents this in full for the project,
but the shape is:

- `# Session NNN: <topic>` title.
- **What happened** — the running log: what was done, in roughly chronological
  order, including decisions made, dead-ends hit, and gotchas discovered. The
  "findings" live here — don't make a separate findings file.
- **Current state** — status on exit. What works, what's half-done, where the
  artifacts are now.
- **What to do next** — a priority-ordered punch list for the next session.

Keep it honest and specific (file paths, command lines, the *why* behind
choices). Because the next session reads only the most recent HANDOFF by default,
a good one stands on its own — it carries forward any still-relevant state rather
than assuming earlier HANDOFFs were read.

## The README

`README.md` is the user-facing front door — a different audience from `CLAUDE.md`
(that's for the agent). The `assets/README.md.template` carries the full menu;
the proven skeleton, top to bottom:

1. **Logo + one-liner** — art (optional) then 1–2 sentences of what this is, for
   whom, on what platform. The most-read lines in the repo.
2. **Try it out! / Quick start** — the *simplest* path: install a prebuilt
   artifact, run a hello-world or demo, copy-pasteable end-to-end. Omit until
   there's something to install.
3. **Status** — current state in a few sentences + a pointer to the latest
   release, then the status table(s).
4. **Demos**, **Releases** (a `Tag | Date | Headline` changelog table), **Build
   from source** (half a page max, else break out to `BUILDING.md`), **Layout**,
   **License**.

Three rules worth internalizing:

- **Start minimal, grow as you ship.** A brand-new project's README is honestly
  just the logo + one-liner + "Status: early, pre-release." Add Quick start, the
  status table, Releases, and Demos only once those things exist. Don't scaffold
  empty sections.
- **Status default is combined; split only when large.** For a small project use
  one `## Status` (a few sentences + the table). Only when the tables grow big do
  you split into `## Status` (≤3 sentences: current state + latest release) and a
  separate `## Implementation status` (the tables).
- **Guard against the metastasis failure mode.** The single most common way these
  READMEs rot: the Status section grows into a prose changelog ("then X, then Y,
  then Z…") and the table Notes cells swell into paragraphs until nothing is
  scannable. The status structure stays good; the *rows multiply and the prose
  walls grow*. Keep Notes cells to one or two sentences; push blow-by-blow detail
  into the Releases table or the linked session HANDOFF. The maintenance rules
  live in `CLAUDE.md`'s "README format" / "Release workflow" sections so they're
  in front of every future session that touches the README.

Deliberately omitted from the template (don't add them back by reflex): a generic
**"Why this exists"** section (don't justify the project — though a *specific*
decision rationale like "Why version X" is worth keeping, in `PLAN.md` or under
its own heading), and a **"Sibling/related projects"** section (premature until
the project is mature).


---

## 🤖 Claude

Let me do quick recon to fill the last gaps (git remote, license, existing files) before scaffolding — I have the rest from this session.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== git remote ===\" && (git remote -v || echo \"no remote\") && echo && echo \"=== existing top-level docs/config ===\" && ls -la CLAUDE.md README.md LICENSE docs/PLAN.md docs/sessions/README.md 2>&1 | grep -v \"No such\" || true && echo && echo \"=== what exists ===\" && ls -1 && echo \"=== docs/ ===\" && ls -1 docs && echo \"=== ref/ (logo candidates?) ===\" && ls -1 ref/screenshots ref/photos 2>/dev/null && echo \"=== version + license header ===\" && grep -n \"VERSION\\|GPL\" src/main.c | head",
  "description": "Recon: remote, existing docs, license, version"
}


---

## 🤖 Claude

Recon confirms: no remote (local-only repo), no existing `CLAUDE.md`/`README.md`/`LICENSE`/`PLAN.md`/`sessions/README.md`, no project logo, version 0.01, GPL v2. I have everything else from this session — no need to interview you. Scaffolding the four files now, tailored to memtestppc+.

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/README.md",
  "content": "# Sessions\n\nSubstantive work on memtestppc+ happens in **sessions**. One Claude conversation\nproduces one session, recorded as a single `HANDOFF.md` in its own\nglobally-numbered directory:\n\n```\ndocs/sessions/NNN-<short-slug>/HANDOFF.md\n```\n\n## Why a single HANDOFF per session\n\nThe `HANDOFF.md` is the session's **running log, its findings, and its primer for\nthe next session, all in one file**. We deliberately do not split narrative and\nhandoff into separate documents \u2014 the overhead isn't worth it, and one file is\nwhat actually gets read. So catch-up at the start of a session is deliberately\ncheap:\n\n> **`CLAUDE.md` (loaded automatically) + the project `README.md` + the most\n> recent `HANDOFF.md`.**\n\nWe don't read a stack of old HANDOFFs \u2014 that would burn tokens fast. The\ntrade-off is a writing discipline: **each HANDOFF must stand on its own.** Carry\nforward any still-relevant state from earlier sessions into the current one\n(restate it, or link the older session dir) rather than assuming the next reader\nhas seen it.\n\n## What a HANDOFF.md contains\n\n- `# Session NNN: <topic>` \u2014 title.\n- **What happened** \u2014 the running log. What was done, in roughly chronological\n  order, with enough detail that a future session can reconstruct the thinking:\n  decisions and their rationale, dead-ends hit (so they aren't re-tried), and\n  gotchas discovered. Findings live here \u2014 there is no separate findings file.\n- **Current state** \u2014 status on exit: what works, what's half-finished, and\n  where the canonical artifacts live now if anything moved.\n- **What to do next** \u2014 a priority-ordered punch list for the next session, with\n  a clear \"I'd start with #1\" recommendation.\n\nWrite it so a stranger could act on it cold: real file paths, real command lines\n(the rsync/build/QEMU invocations), and the *why* behind choices. The existing\n001\u2013004 handoffs are the model.\n\n## Naming\n\n`NNN-<short-slug>/`\n\n`NNN` is a global, monotonically-increasing session number across the whole life\nof the project, starting at `001`. The slug hints at the substantive content\n(`001-inception-and-scaffold`, `004-real-hardware`) so sessions are skimmable\nfrom a plain `ls`.\n\nHistory note: sessions `001-inception-and-scaffold` and `002-cd-boot` were\nwritten *retroactively* (during session 005) from the transcript in\n`docs/convos/`, because the original first working session compacted three times\nand only emitted one handoff (labelled `003`). Their top-of-file notes say so.\n`003` and `004` were written live.\n\n## Start-of-session checklist\n\n1. **Read `CLAUDE.md` (loaded automatically), this project's `README.md`, and the\n   most recent `docs/sessions/NNN-*/HANDOFF.md`.** That's the standard catch-up \u2014\n   the latest HANDOFF is written to stand on its own. Only dig into older\n   HANDOFFs if the latest one points you at something specific.\n2. Glance at [`../PLAN.md`](../PLAN.md) for mission and current priorities.\n3. Confirm the baseline builds before changing anything load-bearing: rsync to\n   the build host and `make memtestppc.iso` (see `CLAUDE.md` \u2192 Build & test).\n\n## End-of-session ritual\n\n1. Commit the actual work.\n2. **Write this session's `HANDOFF.md`** \u2014 every session, not just the eventful\n   ones. The only valid reason to skip is that the project has genuinely run out\n   of roadmap; even then, a HANDOFF saying exactly that beats silence.\n3. Update [`../PLAN.md`](../PLAN.md) if priorities or scope shifted.\n4. Update the project `README.md` for anything user-visible that changed \u2014 flip a\n   status-table row, note a newly-working surface. (See `CLAUDE.md` \u2192 README\n   format.)\n5. **Print the next session's HANDOFF path** in your closing summary as a\n   clickable relative link, so the user can paste it into the next session's\n   opening prompt.\n6. Commit the session notes.\n"
}


---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/CLAUDE.md",
  "content": "# Claude agent instructions for this project\n\n## What this project is\n\n**memtestppc+** is a standalone, bootable memory tester for PowerPC Macs that\nreproduces the classic **memtest86+ v5.01** blue-screen TUI. It boots straight\ninto testing with **no operating system**, so the maximum fraction of RAM is\nunder test. It runs from an Open Firmware-loaded ELF (today: a QEMU `-cdrom` ISO,\nand a partition-booted ELF on a real iBook G3). Versions run 0.01 \u2192 1.00; \"done\"\nis a faithful, real-hardware-verified port that a PowerPC Mac owner can boot and\ntrust.\n\nProject brief: [`docs/PLAN.md`](docs/PLAN.md).\n\n## Repo layout\n\n```\nREADME.md          \u2014 user-facing front door: what it is, how to run, status.\nCLAUDE.md          \u2014 this file.\nMakefile           \u2014 cross-compile + ISO-build + QEMU targets.\ndocs/\n  PLAN.md          \u2014 project brief: mission, non-goals, strategy, roadmap.\n  sessions/        \u2014 one dir per session, globally numbered. Read README.md first.\n    NNN-<slug>/    \u2014 each contains a single HANDOFF.md.\n  convos/          \u2014 raw conversation transcripts (managed externally; ignore).\nsrc/               \u2014 the port itself: head.S, ofw.{c,h}, display.{c,h},\n                     font_vga.h, memtest.h, test.c, main.c, linker.ld.\ncd/                \u2014 CHRP bootinfo.txt for the bootable ISO.\nref/               \u2014 memtest86+ v5.01 source + reference screenshots/photos.\nscripts/           \u2014 helpers.\ntest_minimal/      \u2014 throwaway OF-boot-protocol probes (from the session-002\n                     r5/CD-boot bring-up). Safe to delete eventually.\n```\n\n## Sessions workflow\n\n**Read [`docs/sessions/README.md`](docs/sessions/README.md) before starting\nwork \u2014 that's the workflow you must follow.** The short version: substantive work\nlives in `docs/sessions/NNN-<slug>/`, and **each session writes one `HANDOFF.md`**\nthat serves as its running log, findings, and primer for the next session.\n\nTo get up to speed at the start of a session: this `CLAUDE.md` (loaded\nautomatically), the project `README.md`, and **the single most recent\n`HANDOFF.md`**. Each HANDOFF stands on its own, so you don't normally need older\nones.\n\n## Fleet hosts\n\n- **uranium** \u2014 this Mac, the main dev box. Edit code, drive QEMU locally, and\n  route files between the Linux boxes and the PPC hardware.\n- **opti7050** (`ssh opti7050`) \u2014 primary build + QEMU host. Ubuntu 22.10,\n  `powerpc-linux-gnu-gcc` 12.2.0, `qemu-system-ppc` 7.0.0 (mac99), `xorrisofs`.\n  ISOs are built here.\n- **opti9020** (`ssh opti9020`) \u2014 alternate Linux build host, newer toolchain.\n  Occasionally offline; opti7050 is the default.\n- **ibookg32** (`ssh ibookg32`) \u2014 primary **real hardware** test target. iBook G3\n  (PowerBook4,1), 500 MHz PPC 750, 256 MB, Tiger 10.4.11, 8-bit ATI framebuffer.\n  Boots the ELF from a second HFS+ partition via OF (`boot hd:5,memtestppc.elf`).\n  **opti7050 can't SCP to it directly** (legacy ciphers) \u2014 route through uranium:\n  `scp opti7050:file /tmp/ && scp /tmp/file ibookg32:dest`.\n- **imacg3** (`ssh imacg3`) \u2014 secondary real hardware target (iMac G3, Rage 128).\n  Frequently offline.\n\n## Risk tolerance by host\n\n- **uranium** \u2014 low. It's the daily driver; standard installs fine, no random\n  binaries off the net.\n- **opti7050 / opti9020** \u2014 medium. Reinstallable Linux scratch boxes; build and\n  experiment freely, clean up after yourself.\n- **PPC hardware (ibookg32, imacg3)** \u2014 high. Reinstallable test machines; the\n  bar is \"will this teach us something?\" If one goes unresponsive it's usually\n  sitting at the OF prompt or mid-test \u2014 it needs a physical reboot.\n\n## Build & test\n\n```sh\n# 1. Sync source to the build host (excludes git/ref/artifacts/docs):\nrsync -av --exclude '.git' --exclude 'ref/' --exclude '*.o' --exclude '*.elf' \\\n    --exclude '*.bin' --exclude '*.iso' --exclude 'docs/' --exclude 'testing/' \\\n    /Users/cell/claude/memtestppc+/ opti7050:/home/cell/memtestppc+/\n\n# 2. Build the bootable ISO:\nssh opti7050 'cd /home/cell/memtestppc+ && make clean && make memtestppc.iso'\n\n# 3. Test in QEMU (128MB+ required for OpenBIOS auto-boot; use 256):\nqemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d \\\n    -display vnc=:99 -monitor unix:/tmp/qemu-mon.sock,server,nowait &\n#    Screenshot: echo \"screendump /tmp/ss.ppm\" | socat - UNIX-CONNECT:/tmp/qemu-mon.sock ; convert /tmp/ss.ppm ss.png\n\n# 4. Deploy to real hardware (via the uranium hop):\nscp opti7050:/home/cell/memtestppc+/memtestppc.elf /tmp/memtestppc.elf\nscp /tmp/memtestppc.elf ibookg32:\"/Volumes/Untitled 2/memtestppc.elf\"\n```\n\n## README format\n\n`README.md` carries a status table. Status column legend, in priority order:\n\n- \u2705 Working \u2014 verified by a green run (in QEMU or, better, on real hardware).\n- \ud83d\udfe1 Partial \u2014 works with a caveat / unverified-on-hardware / one path only.\n  Always add a one-line \"what's missing\" in Notes.\n- \u274c Missing \u2014 known not to work, with a pointer to where it's scoped.\n\n**Keep each Notes cell to one or two sentences, and keep the Status prose short.**\nThe common failure mode is the status section growing into a prose changelog and\nthe Notes cells swelling into paragraphs until nothing is scannable \u2014 push\nblow-by-blow detail into the relevant session HANDOFF instead.\n\n## Document everything\n\nDefault to capturing liberally in the active session's `HANDOFF.md`. Decisions,\ndead ends, OF/hardware gotchas, ambiguity in the moment \u2014 all worth writing down.\nErr on the side of MORE capture. The goal is that a future session can pick up the\nthread without re-explaining.\n\n## Unsupervised mode\n\nWhen the user says \"work unsupervised\" (or similar), they're unreachable. Don't\nstop to ask \u2014 make reasonable assumptions, run experiments (QEMU is cheap; real\nhardware less so), try the obvious fixes. Long runtimes are fine. Only block for\ngenuinely irreversible actions. **Log every judgment call** in the active\nsession's HANDOFF \u2014 that's what the user reviews on return.\n\n## Key invariants\n\n1. **Port fidelity to memtest86+ v5.01.** Preserve the original code structure \u2014\n   function skeletons, variable names, layout constants \u2014 and replace only the\n   x86-specific *leaf* code with OF/PPC implementations. Prefer commented-out\n   original over deletion where the replacement is non-obvious. *Why:* makes the\n   diff against upstream small, so upstream fixes are pullable, people who know\n   memtest86+ can navigate, and correctness is verifiable against the original.\n   This is the project's central discipline and the current active workstream.\n\n2. **Boot from a CD *image* via the CHRP boot script \u2014 never QEMU `-kernel`.**\n   *Why:* real Apple OF (and OpenBIOS) run with the MMU on; `-kernel` jumps to an\n   address OF never mapped \u2192 instant ISI fault. OF's `load` from a device claims\n   and maps memory correctly, and matches how real hardware boots.\n\n3. **OF passes the client-interface pointer in `r5`** (not r3). `head.S` relies\n   on this.\n\n4. **With OF's MMU on, claim memory before testing it.** `discover_memory()`\n   `ofw_claim`s in 1 MB chunks; the stack lives in BSS (which OF maps when it\n   loads the ELF), not a hardcoded low address. Writing to unclaimed/unmapped\n   physical addresses faults.\n\n5. **Cross-compile only.** Tiger's gcc emits Mach-O; OF needs ELF, and Tiger has\n   no ELF linker. Build with `powerpc-linux-gnu-gcc` on Linux. The deliverable is\n   the CD image \u2014 build-from-source on a real Tiger Mac is *not* a supported path.\n\n6. **No physical CD has been burned yet.** \"CD boot\" everywhere means a QEMU\n   `-cdrom` ISO; the only real-hardware testing so far is partition boot on\n   ibookg32. Burning + booting a physical disc is still an open to-do.\n"
}


---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/PLAN.md",
  "content": "# memtestppc+ \u2014 project plan\n\n## Mission\n\nA standalone, bootable memory tester for PowerPC Macs that faithfully reproduces\nthe classic **memtest86+ v5.01** blue-screen TUI. Because it runs with **no\noperating system**, the maximum fraction of system RAM is under test \u2014 the whole\npoint of memtest86+, which has no PowerPC equivalent (Apple Hardware Test and\nOpen Firmware's built-in test don't compare).\n\nIt boots straight into testing from an Open Firmware-loaded ELF. The aesthetic\ntarget is pixel-faithful: the IBM VGA 8x16 font (codepage 437), the green title\nbar with the blinking `+`, white-on-blue text, white-on-red errors. Versions run\n**0.01 \u2192 1.00**; \"done\" (1.00) is a faithful, real-hardware-verified tester a\nPowerPC Mac owner can boot from CD and trust.\n\n## Non-goals\n\n- **No SMP / multi-core.** Target machines are single-CPU G3s. The original's\n  SMP, relocation, and windowing machinery is deliberately dropped \u2014 that's why\n  the port's `main.c` is far simpler than upstream's.\n- **No build-from-source on Tiger.** Tiger's gcc emits Mach-O; OF needs ELF.\n  We cross-compile, and ship the CD image as the artifact. Most users download\n  the ISO; they don't build it.\n- **Not x86.** This is a PowerPC/Open Firmware port, not a general refactor of\n  memtest86+.\n\n## Strategy / approach\n\n- **Reuse the original TUI code via a virtual VGA text buffer.** We keep an\n  80\u00d725 char+attribute buffer (exactly the x86 text-mode format) and render\n  changed cells to the OF framebuffer with an embedded 8x16 font. This lets the\n  original `cprint`/`dprint`/`hprint`/`scroll`/etc. work almost verbatim \u2014 the\n  key move that makes a *faithful port* rather than a look-alike feasible.\n- **Open Firmware client interface** for everything hardware: memory discovery\n  and claiming, framebuffer, timebase, keyboard, reboot.\n- **Cross-compile on Linux** (`powerpc-linux-gnu-gcc`), **iterate in QEMU**\n  (mac99, fast), **verify on real hardware** (iBook G3 \u2014 the source of truth for\n  OF MMU behaviour, the 8-bit palette, real CPU detection, real timebase).\n- **Port-fidelity discipline** (see CLAUDE.md \u2192 Key invariants): keep upstream's\n  structure; replace only x86 leaf code. Minimize the diff against v5.01.\n\nThe detailed bring-up findings (the r5 CIF discovery, the `-kernel` MMU/ISI\ndead-end, the CHRP/HFS+ ISO recipe, the 8-bit `color!` BGR ordering, the\nofw_claim-in-1MB-chunks approach) live in the session HANDOFFs `001`\u2013`004`.\n\n## Roadmap\n\n- **M1 (active): port-fidelity restructure.** Bring the codebase closer to\n  upstream v5.01's structure \u2014 especially the display and error-reporting paths,\n  which are currently more rolled-our-own than they should be. Goal: a small,\n  legible diff against upstream.\n- **M2: hardware-truth pass.** Visually verify the white-on-red error display on\n  the iBook's 8-bit screen (re-inject a fake error briefly). Burn and boot a\n  *physical* CD on real hardware (only QEMU ISO + iBook partition boot tested so\n  far). Refine the memory map (probe OF `/memory/available` instead of skipping\n  the first 8 MB unconditionally).\n- **M3: breadth + release.** Test on imacg3 and G4/G5 (exercises the 32-bit\n  framebuffer path and other CPU PVRs). Stand up a release/packaging flow (no\n  GitHub remote or releases exist yet). Drive to **1.00**.\n\n## Open questions / deferred\n\n- `movinv32` warning: `k` may be used uninitialized (`src/test.c`) \u2014 initialize\n  at declaration.\n- Bit-fade test (test 11) `sleep()` is stubbed \u2014 needs a timebase-based delay.\n- The 8-bit `color!` BGR / index-first argument order is empirical (from the\n  iBook's ATI Rage Mobility). May differ on other Macs' OF; might need per-machine\n  detection or a different approach for non-ATI GPUs.\n- ELF size / strip: could `-s` further if older OF implementations are picky about\n  CD-loaded ELF size.\n"
}


---

## 🤖 Claude

Now the user-facing README — generic build instructions (the public face), not the fleet-specific workflow:

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/README.md",
  "content": "# memtestppc+\n\nA standalone, bootable RAM tester for PowerPC Macs \u2014 a faithful port of the\nclassic [memtest86+](https://www.memtest.org/) v5.01 blue-screen TUI to Open\nFirmware. It boots with **no operating system**, so the maximum fraction of\nsystem memory is under test. Same IBM VGA 8x16 font, same green title bar, same\nblinking `+`.\n\n## Status\n\n**Early, pre-release (v0.01).** Boots and runs end-to-end in QEMU (mac99) and on\nreal iBook G3 hardware: the TUI renders correctly, memory is discovered and\ntested, progress bars and timer work. Not yet packaged for download, and a\n*physical* CD hasn't been burned yet (testing so far is a QEMU `-cdrom` image and\npartition boot on a real iBook). No releases yet.\n\n| Surface | Status | Notes |\n|---|---|---|\n| Boot from CD image in QEMU (mac99) | \u2705 Working | Via a CHRP boot script; needs \u2265128 MB RAM under OpenBIOS. |\n| Run on real iBook G3 | \u2705 Working | Booted from an HFS+ partition via Open Firmware. |\n| TUI rendering (VGA font, colors, layout) | \u2705 Working | Embedded IBM VGA 8x16 font rendered to the OF framebuffer. |\n| 32-bit framebuffer path | \u2705 Working | QEMU's mac99 framebuffer. |\n| 8-bit framebuffer + palette | \u2705 Working | Real iBook G3 (ATI Rage Mobility); Apple OF `color!` palette setup. |\n| CPU identification (PVR \u2192 G3/G4/G5) | \u2705 Working | Plus cache sizes from the OF device tree. |\n| Memory discovery + test | \u2705 Working | Claims RAM from OF in 1 MB chunks (~244 MB of 256 on the iBook). |\n| Error display (white-on-red) | \ud83d\udfe1 Partial | Implemented; not yet visually verified on real hardware. |\n| Bit-fade test (test 11) | \ud83d\udfe1 Partial | Runs, but the timed fade window is stubbed out (no `sleep()` yet). |\n| Physical CD boot | \u274c Not yet | The ISO has only ever been booted in QEMU. |\n\n## Building & running\n\nRequires a `powerpc-linux-gnu` cross toolchain, `qemu-system-ppc`, and\n`xorrisofs` (all on a Linux host; Tiger's own gcc can't produce the ELF that\nOpen Firmware needs).\n\n```sh\nmake memtestppc.iso\n\n# Run in QEMU (mac99; 128 MB+ is required for OpenBIOS auto-boot):\nqemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d\n```\n\nOn a real PowerPC Mac, boot the ISO from Open Firmware (`boot cd:,\\memtestppc.elf`)\nor copy `memtestppc.elf` to an HFS+ partition and `boot hd:N,memtestppc.elf`.\n\n## Layout\n\n```\nsrc/        \u2014 the port: head.S, ofw.{c,h}, display.{c,h}, font_vga.h,\n              memtest.h, test.c, main.c, linker.ld.\ncd/         \u2014 CHRP bootinfo.txt for the bootable ISO.\nref/        \u2014 memtest86+ v5.01 reference source + screenshots.\ndocs/       \u2014 project plan and per-session notes (docs/sessions/).\nMakefile    \u2014 cross-compile, ISO build, QEMU targets.\n```\n\n## License\n\nGPL v2, inherited from memtest86+ (Chris Brady and Samuel Demeulemeester). The\nPowerPC / Open Firmware port code is GPL v2 to match.\n"
}


---

## 🤖 Claude

All four files written. Let me verify no template placeholders or HTML comments leaked through.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== created files ===\" && ls -la CLAUDE.md README.md docs/PLAN.md docs/sessions/README.md && echo && echo \"=== leftover placeholders / template comments? ===\" && (grep -rn \"<!--\\|<PLACEHOLDER\\|<ANGLE\\|<PROJECT NAME\\|<slug>\\|<feature>\\|TEMPLATE:\" CLAUDE.md README.md docs/PLAN.md docs/sessions/README.md && echo \"LEFTOVERS FOUND\") || echo \"CLEAN \u2014 no template artifacts\"",
  "description": "Verify scaffolded files and check for template leftovers"
}


---

## 🤖 Claude

Done — the skill produced all four files cleanly (the only `<slug>` matches are the literal `NNN-<slug>/` naming notation, not unfilled placeholders).

**Scaffolded for memtestppc+:**
- [CLAUDE.md](CLAUDE.md) — what it is, repo layout, sessions workflow, the 5-host fleet (uranium / opti7050 / opti9020 / ibookg32 / imacg3) with the SCP-routing quirk, risk tolerance, the real build/test/deploy commands, README-format rules, and 6 numbered **Key invariants** (port fidelity #1, CD-not-`-kernel`, r5 CIF, claim-before-test, cross-compile-only, no physical CD yet).
- [docs/PLAN.md](docs/PLAN.md) — mission, non-goals (no SMP / no Tiger-native build / not x86), the VGA-buffer-reuse strategy, and an M1→M3 roadmap (port-fidelity → hardware-truth → breadth+release).
- [docs/sessions/README.md](docs/sessions/README.md) — HANDOFF-only workflow, the cheap catch-up directive, naming (with the note that 001/002 were backfilled), start/end-session rituals (including print-the-next-HANDOFF-path, matching your global preference).
- [README.md](README.md) — front door: one-liner, combined Status section with a ✅/🟡/❌ table, generic build+run, layout, GPL v2.

**Division of labor held up:** README build instructions are generic/public (`powerpc-linux-gnu` + qemu + xorrisofs); CLAUDE.md carries the fleet-specific rsync→opti7050→QEMU→ibookg32 workflow. Different audiences, no awkward duplication.

**Dogfood verdict — the skill worked well.** The "omit sections without real content yet" rule did real work: I dropped "Try it out!" (no prebuilt artifact → used "Building & running" instead), dropped Demos/Releases (none yet), and cut the CLAUDE.md "Release workflow" section (no release process). One minor refinement I'd consider: the CLAUDE.md template ties **README format** *and* **Release workflow** to "ships releases," but README format is actually useful the moment you have a status table (as here, pre-release). I made that call correctly anyway, so it's optional — want me to loosen that wording in the skill?

Two follow-ups:
1. Sessions 001–004 exist; **this** session (the renumber cleanup + skill build + dogfood) is logically **005** and will get its `docs/sessions/005-<slug>/HANDOFF.md` at closeout — not created yet.
2. I haven't committed anything. Want me to commit the backfilled handoffs + these scaffolded docs (and note the skill lives in `~/.claude/skills/`, outside this repo)?

---

## 👤 User

yes, commit everything.  I think we can skip creating a session for this administration work and I'll start session 005 using the 004 handoff after clearing the context, after you commit

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "git status && echo \"=== LOG ===\" && git log --oneline -5 && echo \"=== is docs/convos tracked or ignored? ===\" && git check-ignore docs/convos 2>/dev/null && echo \"(convos is gitignored)\" || echo \"(convos NOT ignored)\" && echo \"=== .gitignore ===\" && cat .gitignore 2>/dev/null",
  "description": "Inspect repo state before committing"
}


---

## 🤖 Claude

This session's own transcript (`docs/convos/...00aaed09.md`) showed up untracked, but you manage `docs/convos/` externally — so I'll stage only the docs we authored, by name, and leave the transcript for your `make` process. Committing now.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "git add CLAUDE.md README.md docs/PLAN.md docs/sessions/README.md docs/sessions/001-inception-and-scaffold/HANDOFF.md docs/sessions/002-cd-boot/HANDOFF.md && git commit -m \"$(cat <<'EOF'\nadd project conventions + backfill sessions 001-002\n\nSet up the session-based workflow docs for the project:\n- CLAUDE.md (agent instructions, fleet, key invariants)\n- README.md (user-facing front door + status table)\n- docs/PLAN.md (mission, non-goals, roadmap)\n- docs/sessions/README.md (HANDOFF-only workflow spec)\n\nPlus backfilled HANDOFFs for sessions 001 (inception/scaffold) and\n002 (cd-boot), reconstructed from the transcript: the original first\nworking session compacted three times and only emitted one handoff,\nlabelled \"003\", leaving phantom predecessors. Numbering is now\ncontiguous 001-004.\n\nCo-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>\nEOF\n)\"",
  "description": "Stage authored docs and commit"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "git status && echo \"=== last 3 ===\" && git log --oneline -3",
  "description": "Verify commit landed and working tree state"
}


---

## 🤖 Claude

Committed as `c0268af`. Working tree is clean except this session's transcript (`docs/convos/...00aaed09.md`), which I left untracked for your external `make` process.

**What landed in the commit (6 files, +640 lines):**
- `CLAUDE.md`, `README.md`, `docs/PLAN.md`, `docs/sessions/README.md` — the scaffolded workflow + project docs
- `docs/sessions/001-inception-and-scaffold/HANDOFF.md`, `docs/sessions/002-cd-boot/HANDOFF.md` — the backfilled handoffs

Session numbering is now contiguous **001 → 004**, and the "why 003 came first" mystery is resolved and documented.

**Not committed (by design):**
- The `project-workflow-setup` skill lives at `~/.claude/skills/project-workflow-setup/` — outside this repo, and `.gitignore` excludes `.claude/` anyway. It's saved and registered, just not part of memtestppc+'s history.
- This session's transcript — yours to manage.

You're set to clear context and start **session 005** from [docs/sessions/004-real-hardware/HANDOFF.md](docs/sessions/004-real-hardware/HANDOFF.md) — whose headline next-step is the **port-fidelity restructure** (M1 in the new `docs/PLAN.md`). Per your global convention, here's the handoff you'll paste in: [docs/sessions/004-real-hardware/HANDOFF.md](docs/sessions/004-real-hardware/HANDOFF.md).

---

