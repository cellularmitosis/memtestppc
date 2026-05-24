# Claude Conversation Log

Session ID: 8d831cb7-8ae5-48af-a88c-e8a94b840a9b
Date: 2026-05-24 04:54:52

---

## 👤 User

read docs/sessions/005-release-tweaks/HANDOFF.md

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/005-release-tweaks/HANDOFF.md"
}


---

## 🤖 Claude

I've read the session 005 handoff. Here's where things stand:

**Session 005 accomplished** (all done):
- 3 TUI tweaks (progress-bar offsets, two-line memory readout, slimmed footer) + an error-row red-fill fix — all QEMU-verified.
- First physical CD-R burned. The initial xorrisofs disc gave a **blinking folder** on the real iBook G3, but a reworked **genisoimage HFS-hybrid recipe** (DDM + APM + Apple_HFS, blessed `tbxi`, `<COMPATIBLE>` block incl. `MacRISC2`) **boots on real Apple OF** ✅ — the headline win.
- Committed; release artifacts copied to `artifacts/`. Versions kept at **0.01** (no bump, user's call).

**Open / next-session items:**
1. **Session 006: the true memtest86+ v5.01 port restructuring** — the deferred big workstream, and the project's central discipline per CLAUDE.md invariant #1.
2. Carried-over loose ends: `movinv32` uninitialized-`k` warning (`src/test.c:324`), bit-fade `sleep()` stub (test 11), `color!` BGR palette order is empirical, and the word "Testing" now appears twice on screen (left readout + right address-range field) — easy rename if it bugs you.

What would you like to work on? If it's the session 006 port restructuring, I'd start a new session dir and read `docs/PLAN.md` + the upstream `ref/memtest86+-5.01/` tree first.

---

## 👤 User

1

---

## 🤖 Claude

You want to start session 006: the true memtest86+ v5.01 port restructuring. Let me get up to speed on the workflow, the brief, and the current vs. upstream code.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/README.md"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/PLAN.md"
}


---

## 🤖 Claude

Let me see the current source tree and the upstream reference layout.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== src/ ===\" && ls -la src/ && echo && echo \"=== ref/ ===\" && ls -la ref/ && echo && echo \"=== ref/memtest86+-5.01/ ===\" && ls -la ref/memtest86+-5.01/ 2>/dev/null"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== line counts: src/ ===\" && wc -l src/* && echo && echo \"=== line counts: upstream key files ===\" && wc -l ref/memtest86+-5.01/main.c ref/memtest86+-5.01/test.c ref/memtest86+-5.01/error.c ref/memtest86+-5.01/display.* 2>/dev/null; ls ref/memtest86+-5.01/*.c ref/memtest86+-5.01/*.h 2>/dev/null"
}


---

## 👤 User

[Request interrupted by user]

---

## 👤 User

sorry to interrupt, just to clarify, step 1 is to research the task thoroughly and create a detailed plan.md (which will live in your session dir for now).  please continue

---

## 🤖 Claude

Got it — step 1 is research + a detailed plan in the session dir. Let me set up the session directory and start reading the actual code.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "mkdir -p docs/sessions/006-port-fidelity-restructure && ls docs/sessions/"
}


---

## 🤖 Claude

Now let me read the current port files. They're the smaller, more important set for understanding the gap.

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
  "file_path": "/Users/cell/claude/memtestppc+/src/display.c"
}


---

## 🤖 Claude

Now the remaining port files and the key upstream sources for the display/error paths.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/memtest.h"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/display.h"
}


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
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/screen_buffer.h"
}


---

## 🤖 Claude

Now the upstream structural definitions and the port's test/ofw layers.

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
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/config.h"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/screen_buffer.c"
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/src/ofw.c"
}


---

## 🤖 Claude

I have a clear picture of the display/error layers now. Let me pin down the upstream `main.c` and `lib.c` structure (targeted reads) and, in parallel, delegate a test.c structural comparison.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd ref/memtest86+-5.01 && echo \"=== lib.c: function defs ===\" && grep -n '^[a-zA-Z].*(' lib.c | grep -vE 'if|while|for|return' | head -80"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd ref/memtest86+-5.01 && echo \"=== main.c: function defs + key markers ===\" && grep -n -E '^(void|int|static|struct|[a-z].*\\bmain\\b)' main.c | head -60 && echo && echo \"=== tseq table in main.c ===\" && grep -n 'tseq\\[\\]' main.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== main.c: function defs ===\" && grep -nE '^(void|int|static|unsigned|struct tseq|[a-z_]+ +main)' main.c | head -70 && echo && echo \"=== tseq table ===\" && grep -n 'struct tseq tseq' main.c"
}


---

## 🤖 Claude

Working dir persisted into `ref/`. Let me read the canonical upstream sections directly with absolute paths.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/main.c",
  "offset": 50,
  "limit": 130
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/main.c",
  "offset": 370,
  "limit": 160
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/main.c",
  "offset": 530,
  "limit": 295
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/main.c",
  "offset": 1043,
  "limit": 160
}


---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/lib.c",
  "offset": 174,
  "limit": 300
}


---

## 🤖 Claude

I have the main/lib/error picture. Let me grab the upstream screen-header drawing (in `init.c`) and, in parallel, delegate a focused `test.c` structural comparison.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== init.c: cprint/dprint header-drawing calls ===\" && grep -nE 'cprint\\(|dprint\\(|aprint\\(|hprint\\(|^void |^static ' init.c | head -80"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "pwd && echo \"---\" && grep -nc 'cprint' init.c && echo \"--- lines with cprint ---\" && grep -n 'cprint(' init.c | head -60"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rn 'void init(void)\\|^void init\\|init(void)' *.c | head; echo \"--- who draws Pass/Test/WallTime labels? ---\"; grep -rln 'Pass\\|WallTime\\|L1 Cache\\|Memtest86' *.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rn 'init()' *.c | grep -v 'pci_init\\|dmi\\|_cpus\\|echo' | head; echo \"=== files defining init ===\"; grep -rln '^void init' *.c; echo \"=== the screen header (WallTime / RAM / labels) lives in: ===\"; grep -rln 'WallTime\\|Pass:\\|Time:' *.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rn 'Memtest86\\|Pass   %\\|Test   %\\|WallTime\\|RsvdMem\\|init (void)\\|init(void)' *.c | head -30"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rn 'void init' config.c extra.c 2>/dev/null; echo \"=== grep init definition w/ word boundary ===\"; grep -rnE '^\\s*(void|int)\\s+init\\s*\\(' *.c; echo \"=== where is the green title/labels drawn? search 'cprint(0,' ===\"; grep -rn 'cprint(0,\\|cprint(LINE_TITLE' *.c | head"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -nE '^[a-zA-Z].*\\(' config.c | grep -viE 'if|while|for|return|else' | head -40"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -nE '^[a-zA-Z].*\\(' init.c | grep -viE 'if|while|for|return|else|sizeof' | head -40"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -nE 'init|cprint|\"L1|\"L2|Cache|Pass|Test #|WallTime|Temp' init.c | head -50"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "head -20 init.c; echo \"=== grep all .c for the screen labels ===\"; grep -rn 'Memtest86\\|Pass  \\|Test  \\|WallTime\\|Cache' *.c | head"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rnE '\\binit\\s*\\(\\s*(void)?\\s*\\)' *.c | grep -v 'btrace\\|init_dmi\\|pci_init\\|initialise\\|serial\\|=\\|;' | head; echo \"=== cprint count per file ===\"; for f in main.c init.c extra.c config.c; do echo -n \"$f: \"; grep -c 'cprint' $f; done"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -n 'cprint\\|dprint\\|aprint\\|^void init\\|^init' main.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rn 'v5.01\\|V5.01\\|5.01\\|MT_VERSION\\|TITLE\\|footer\\|\"Memtest' *.c *.h 2>/dev/null | head; echo \"=== init() definition location (search all) ===\"; grep -rn 'void init(void)\\|^void init(\\| init(void)$' *.c *.h; echo \"=== does extra.c hold the header? cprint in extra.c ===\"; grep -c cprint extra.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rn '^init\\|^void init\\|^\tinit\\|init (void)\\|init ()' *.c; echo \"=== brute: which file has the most distinctive header label? grep for 'Temperature' 'CLK' 'Pass:' across tree ===\"; grep -rIn 'Temperature\\|emperatur\\|CLK\\| RAM:\\|Pass: \\|State:' *.c | head; echo \"=== read footer + neighbors ===\"; sed -n '655,700p' lib.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -cE 'cprint ?\\(' init.c; echo \"--- with optional space ---\"; grep -nE 'cprint ?\\(|dprint ?\\(|void init' init.c | head -40"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -niE 'print|cpu_type|cache|dprint|aprint|\"' init.c | head -40"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "wc -l init.c; echo \"--- count double-quotes ---\"; grep -c '\"' init.c; echo \"--- last 5 funcs by brace style ---\"; grep -nE 'void |^\\{|cpu_type|l1_cache|l2_cache' init.c | head"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rIn 'Cache\\|WallTime\\|Pass  \\|Test  \\|emtest86\\|Pattern\\|State:' . 2>/dev/null | grep -v 'Binary' | head -30"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -rnE '^(static )?(void|int) +init *\\(' *.c; echo \"=== maybe in extra.c \u2014 list its funcs ===\"; grep -nE '^[A-Za-z].*\\{?$' extra.c | grep -iE 'init|void|int ' | head; echo \"=== grep whole tree for 'init(void)' as def ===\"; grep -rn 'init(void)' *.c"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"extra.c lines:\"; wc -l extra.c; echo \"=== extra.c voids ===\"; grep -n 'void\\|^init\\|init(' extra.c | head; echo; echo \"=== GREP ENTIRE TREE for init definition (loose) ===\"; grep -rn 'init' *.c | grep -E '\\)[ ]*$|\\)[ ]*\\{' | grep -iE '^\\w+\\.c:[0-9]+:(static |void |int )?init' | head"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "echo \"=== cprint across EVERY .c (with counts) ===\"; for f in *.c; do n=$(grep -c 'cprint' \"$f\" 2>/dev/null); [ \"$n\" -gt 0 ] && echo \"$f: $n\"; done; echo; echo \"=== where is init() defined? check controller.c cpuid.c ===\"; grep -n 'init(' controller.c cpuid.c | grep -v 'pci_init\\|_init(' | head"
}


---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "grep -n 'init' extra.c | head; echo \"=== sample of extra.c cprint strings ===\"; grep -n 'cprint' extra.c | head -20"
}


---

## 🤖 Claude

I've mapped the display/error/main divergences thoroughly. Two things remain: a structural `test.c` comparison, and pinning down how upstream draws its static header (it's oddly absent from the obvious files). Let me delegate both to an agent while I begin drafting, then reset my own shell to the project root.

---

## 🤖 Claude


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+ && pwd && ls ref/screenshots/ ref/photos/ 2>/dev/null"
}


---

## 🤖 Claude


🔧 Using tool: Agent
Input: {
  "description": "Compare port test.c vs upstream",
  "subagent_type": "general-purpose",
  "prompt": "I'm doing a \"port-fidelity\" audit of a PowerPC port of memtest86+ v5.01. The porting discipline is: preserve upstream's code structure (function skeletons, variable names, layout constants) and replace ONLY the x86-specific leaf code with PowerPC/Open-Firmware equivalents. I need to know exactly where the port's code diverges *structurally* from upstream so I can plan a restructure. Read whole files, not excerpts \u2014 accuracy matters.\n\nTwo separate questions. Report on both, concisely (aim for under 600 words total), with specific line numbers.\n\n=== QUESTION 1: test.c structural comparison ===\nCompare these two files in full:\n- PORT:     /Users/cell/claude/memtestppc+/src/test.c        (728 lines)\n- UPSTREAM: /Users/cell/claude/memtestppc+/ref/memtest86+-5.01/test.c  (1568 lines)\n\nThese contain the actual memory-test routines: addr_tst1, addr_tst2, movinv1, movinvr, movinv32, modtst, block_move, bit_fade_fill, bit_fade_chk, and helpers. For EACH test routine, tell me:\n- Does the port preserve upstream's structure/variable-names/loop-shape, or was it rewritten?\n- Upstream's signatures take a trailing `int cpu` arg and use SMP windowing (cpu_ord, get_cpu_range, segment splitting across CPUs). The port is single-CPU. How did the port handle the per-CPU segment-splitting and the `cpu` parameter \u2014 dropped cleanly, or restructured in a way that diverges from upstream's loop skeleton?\n- Note any test routine where the port's loop/iteration structure differs from upstream in a way that affects fidelity (not just the cpu arg).\n- Note routines that are essentially faithful (good) vs ones that were substantially rewritten (candidates for re-aligning).\n\n=== QUESTION 2: where does upstream draw its static screen header? ===\nIn the upstream tree /Users/cell/claude/memtestppc+/ref/memtest86+-5.01/, I cannot find the source of the static TUI header \u2014 the green \"Memtest86+ v5.01\" title bar, the \"Pass\"/\"Test\" progress-bar labels, the L1/L2 cache lines, \"WallTime\", the vertical separators, the column header row. Grepping the .c files for strings like \"Cache\", \"Pass\", \"WallTime\", \"Memtest86\" finds NOTHING (init.c has zero string literals and zero cprint calls). Yet `init()` is declared in test.h and called from main.c:425, and the running program clearly shows these labels.\n\nFigure out the truth: Where and how is the static header actually drawn? Candidates to investigate: is the header a pre-built byte template copied into SCREEN_ADR (0xb8000)? Is `init()` defined in an unexpected file (check controller.c, cpuid.c, every .c)? Is it assembled in head.S/bootsect.S/setup.S? Is there a binary blob? Run whatever greps/reads you need. Tell me definitively: which file+function draws each part of the static header, and by what mechanism. This matters because the port has a hand-written draw_screen() in main.c that I need to map back to its upstream origin.\n\nDo NOT modify any files. Research only."
}


---

## 🤖 Claude

The agent cracked it: `init.c` is ISO-8859 encoded (a `°C` byte), so plain `grep` treated it as binary and skipped all 111 `cprint` calls. The static header lives in `init()` + `display_init()` there. Let me read that section directly to cite it accurately.

---

## 🤖 Claude


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-5.01/init.c",
  "offset": 137,
  "limit": 100
}


---

## 🤖 Claude

Mystery solved and confirmed. The port's `draw_screen()` (main.c:388) maps cleanly onto upstream `init()`+`display_init()` (init.c:137/186). I now have a complete structural picture. Let me write the detailed plan to the session dir.

---

## 🤖 Claude


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/plan.md",
  "content": "# Session 006 \u2014 plan: port-fidelity restructure (M1)\n\n**Status:** research complete, plan drafted, *not yet started on code*. This file\nis the working plan; it will fold into `HANDOFF.md` at closeout.\n\n## Goal\n\nBring `src/` structurally close to upstream **memtest86+ v5.01** so the diff\nagainst upstream is small and legible. Per CLAUDE.md invariant #1 / PLAN.md M1:\npreserve upstream's **function skeletons, variable names, file boundaries, and\nlayout constants**; replace only x86-specific *leaf* code with PPC/OF\nequivalents; prefer commented-out original over deletion where the replacement is\nnon-obvious.\n\nThis is a *restructure*, not a rewrite: behavior should stay green (QEMU + iBook)\nat every stage. The payoff is that upstream bugfixes become pullable and someone\nwho knows memtest86+ can navigate our tree.\n\n## How this plan was researched\n\nRead in full: the port's `src/{main,display,test,ofw}.c`, `src/{memtest,display,ofw}.h`;\nupstream `error.c`, `screen_buffer.{c,h}`, `test.h`, `config.h`; and the canonical\nsections of upstream `main.c` (tseq, `next_test`, `set_defaults`, `test_start`,\n`test_setup`, `do_test`, `find_chunks`, `find_ticks_for_pass/_test`,\n`compute_segments`), `lib.c` (the print primitives + `scroll`/`clear_scroll`),\nand `init.c` (`display_init`+`init`). A subagent did a line-level `test.c`\ncomparison and located the static-header draw.\n\n**Gotcha worth recording:** upstream `init.c` is ISO-8859 encoded (it has a raw\n`\u00b0C` byte). Plain `grep` treats it as binary and **silently skips all matches** \u2014\nthat is why the header strings looked \"missing.\" Always `grep -a` the upstream\ntree, and use the Read tool (which decodes) rather than `grep`/`sed` when in\ndoubt.\n\n## Where the static header actually lives (was unclear at session start)\n\nThe green title bar, `Pass/Test %` labels, cache lines, `| Time:` clock, vertical\nseparators, and `---` rules are drawn by upstream **`init()` + `display_init()`\nin `init.c`** (`display_init` at init.c:137, `init` at init.c:186; called from\nmain.c:425). The port's hand-written `draw_screen()` (src/main.c:388) is the\ndirect analog. The error-row column header is *not* part of this static draw \u2014\nupstream draws it lazily on the first error inside `common_err`\n(error.c:313-320).\n\n---\n\n## Current-state structural map (port \u21c6 upstream)\n\n| Concern | Port location | Upstream home | Verdict |\n|---|---|---|---|\n| Print primitives `cprint/cplace/dprint/hprint/hprint2/aprint/xprint/scroll/clear_scroll/footer` | `display.c` (mixed with FB backend) | `lib.c` (+ `footer` in lib.c) | Faithful logic, **wrong file**. `dprint` right-justify branch was simplified \u2014 re-align to upstream exact. Missing `hprint3`. |\n| Framebuffer + font rendering (`fb_*`, `display_render_cell`, palette, `display_init/refresh`) | `display.c` | *(none \u2014 this is the PPC leaf replacing VGA text HW + 0xb8000 autorender)* | Correct as a backend; should be its own file, the \"io\" of this port. |\n| `vga_buf[]` = the 80\u00d725 char/attr buffer | `display.c`, `#define SCREEN_ADR vga_buf` | `SCREEN_ADR 0xb8000` (test.h) | **Faithful key move.** Keep. |\n| Error path `error/ad_err1/ad_err2` | `main.c` (custom, simplified) | `error.c` (`common_err` + `PRINTMODE_*`) | **Biggest divergence.** Custom row layout & header; no `common_err`, no `err_info` accumulation. |\n| `do_tick` | `main.c` (local statics, full bar redraw) | `error.c` (uses `v->tptr/v->pptr`, incremental bars) | Diverges; belongs in `error.c`. |\n| Static header draw | `main.c draw_screen()` | `init.c display_init()+init()` | Analog exists; line/col mostly match (see below). |\n| CPU detect | `main.c identify_cpu()` | `init.c init()` (calls `get_cpuid` in cpuid.c) | PPC leaf; lives with `init`. |\n| Memory discovery / claim | `main.c discover_memory()` | `init.c`/`mem_size()` (memsize.c) | PPC/OF leaf; reasonable. |\n| Main loop | `main.c main()` | `main.c test_start()` | Port is a clean single-CPU collapse, but uses standalone globals instead of `v->` fields and a custom tick calc. |\n| `find_ticks_for_test`/`calc_pass_ticks` | `main.c` (hardcoded switch) | `main.c find_ticks_for_pass/_test/find_chunks` | Rewritten; re-derive from upstream. |\n| `setup_segments` | `main.c` | `compute_segments` | Single-window collapse of `compute_segments`. |\n| RNG `rand_seed/rand_next` | `test.c` | `random.c` | **Wrong file** \u2014 move to a `random.c` analog. |\n| Test routines | `test.c` | `test.c` | Mostly faithful (asm\u2192commented-C). 3 routines need re-aligning (below). |\n| `struct vars`, layout `#define`s, `struct tseq/mmap/pmap/err_info` | `memtest.h` | `test.h` | Port dropped `v->` fields it still needs; `tseq.cpu_sel` values changed. |\n| OF client interface | `ofw.{c,h}` | *(none \u2014 replaces BIOS/`io.h`)* | PPC leaf, fine. |\n\n### Files to add (mirror upstream), files to keep\n\n- **add `error.c`** \u2014 `error/ad_err1/ad_err2/common_err/update_err_counts/print_err_counts/do_tick`.\n- **add `init.c`** \u2014 `display_init()` + `init()` (header draw, CPU detect, memory\n  discovery). Pulls `draw_screen/identify_cpu/discover_memory` out of `main.c`.\n- **add `lib.c`** \u2014 the print primitives + `strlen/memset/memcpy/memmove`.\n- **add `random.c`** \u2014 `rand_seed/rand_next` (the MWC generator).\n- **rename `display.c` \u2192 framebuffer backend** (`display.c` is fine as a name; it\n  becomes purely the FB/font/palette leaf + `vga_buf` + `display_render_cell`).\n- **`main.c`** shrinks to: globals, `tseq[]`, `next_test`, `set_defaults`,\n  `test_setup`, the main loop (named `test_start` or kept as `main`), `do_test`,\n  `find_ticks_*`, `compute_segments`, `main()` bootstrap.\n- **keep** `head.S`, `ofw.{c,h}`, `font_vga.h`, `linker.ld`, `memtest.h`(\u2192 align to `test.h`).\n- **out of scope / do not port:** `config.{c,h}` menu, `screen_buffer.c` (serial\n  mirror \u2014 we have no serial console), `smp.*`, `reloc.c`, `controller.c`,\n  `dmi.c`, `spd.c`, `patn.c` (badram), `extra.c`. These are deliberately dropped\n  (PLAN.md non-goals). Note them as commented stubs only where a call site would\n  otherwise dangle.\n\n---\n\n## Detailed divergence inventory\n\n### A. Error path (highest value)\nPort `error()` (main.c:177) invents a layout: header `\" Address Expected Actual\nXor Tst Pass\"` (drawn statically in `draw_screen`, main.c:440) and a row of\n`hprint(adr)@1, good@15, bad@27, xor@39, dprint test@51, pass@55`.\n\nUpstream `common_err` `PRINTMODE_ADDRESSES` (the default printmode, error.c:307)\ndraws, lazily on first error:\n- header (error.c:315): `\"Tst  Pass   Failing Address          Good       Bad     Err-Bits  Count CPU\"`\n- row: `test+1@0, pass@4, page@11, offset@19, \" -      . MB\"@22, mb@25,\n  frac@31, good@36, bad@46, xor@56, ecount@66, cpu@74`.\n- de-dups consecutive identical errors (`adr==eadr && xor==exor`).\n- `print_err_counts()` (error.c:114) updates the count at `(LINE_INFO,72)` and\n  paints the row red by writing attr `0x47` across cols 1..76.\n\n**Plan:** create `error.c` with `common_err` + the `PRINTMODE_ADDRESSES` branch\nverbatim (drop `PRINTMODE_SUMMARY/PATTERNS/NONE`, or keep `SUMMARY` later). cpu\ncol = 0. The session-005 \"render each red cell\" loop is the faithful PPC version\nof `print_err_counts`'s 0x47 fill (our shadow buffer needs explicit render) \u2014\nmove it into `print_err_counts`. Drop the custom static error header from\n`draw_screen`. This makes the error display match the v5.01 screenshot\n(`ref/photos/Memtest86+_memory_errors.png`).\n\n### B. do_tick\nAdopt upstream `error.c:461` shape: incremental bars via `v->tptr`/`v->pptr`\n(only draw new `#`s), `dprint(2,COL_MID+4,pct,3,0)` etc., spinner via\n`spin_idx[]`/`cplace`. Replace the x86 `rdtsc` elapsed-time block (error.c:585)\nwith our timebase `update_time()` leaf. Move `do_tick` into `error.c`.\n\n### C. Print primitives \u2192 lib.c\nMove `cprint/cplace/dprint/hprint/hprint2/aprint/xprint/scroll/clear_scroll/footer`\n+ `strlen/memset/memcpy/memmove` into `lib.c`. Re-align `dprint`'s `right` branch\nto upstream's exact loop (lib.c:344-367) \u2014 the port simplified it. Add `hprint3`\n(used by some upstream paths). `cprint` keeps calling the FB backend\n(`display_render_cell`) where upstream calls `tty_print_line`; that's the leaf\nswap. `scroll/clear_scroll` already match upstream + our render step.\n\n### D. struct vars / globals \u2192 use v->\nPort uses standalone globals `v_msg_line, test_ticks, nticks` and never uses the\n`v->msg_line/tptr/pptr/pass_ticks/total_ticks` fields that already exist in the\nstruct. Upstream uses `v->`. Re-align: drop `v_msg_line` for `v->msg_line`, use\n`v->tptr/v->pptr/v->pass_ticks/v->total_ticks`. Add back `set_defaults()`\n(main.c:118) to initialize `v->erri.*` and the rest \u2014 the port inlined a partial\nversion into `main()`. Restore `err_info` fields used by the error path.\n\n### E. tseq table\nPort set every `cpu_sel` to `1` (memtest.h tseq). Upstream uses `-1/32/1` to\nencode \"round-robin all CPUs / N CPUs / single CPU\". For a single-CPU port the\n*values* are inert, but fidelity says keep upstream's literals and let the\nsingle-CPU collapse in `next_test`/the loop treat them as \"run once\". Keep\nupstream's `{1,-1,..}` etc. and document that `cpu_sel` is vestigial here.\n\n### F. find_ticks\nRe-derive `find_ticks_for_pass/_test` + `find_chunks` from upstream\n(main.c:1043-1187) instead of the port's hardcoded `switch`. With single CPU,\n`act_cpus==1`, so the `/act_cpus` and `*act_cpus` terms simplify out \u2014 keep them\nwritten (as `/1`) per the \"comment-out, don't delete\" rule, or reduce with a\none-line note. Reconcile `SPINSZ`: port uses `0x400000`, upstream `0x4000000`;\nthis feeds the tick math, so pick deliberately and document.\n\n### G. test.c re-alignment (from the subagent audit)\nMostly faithful (every asm block \u2192 upstream's commented-out C). Three to fix:\n1. **addr_tst1** \u2014 port dropped upstream's inner `for(i=0;i<50;i++)` repeat on the\n   second bank (up test.c:140). Restore.\n2. **movinvr** \u2014 most diverged: port rolls its own seed (`mftb()`+fixed XOR) and\n   embeds the RNG. Upstream seeds from pass/tsc and uses `random.c`. Re-align:\n   move RNG to `random.c`, restore the seed scheme (tsc\u2192timebase leaf).\n3. **block_move** \u2014 port uses plain `<<|>>` rotate + `memcpy`; upstream uses\n   rotate-*through-carry* (`rcll`) + `rep movsl`. Verify the carry-rotate\n   semantics match before declaring faithful; keep memcpy as the leaf but match\n   the pattern rotate.\nAlso: `movinv32` reuses loop var `i` where upstream used `n` (harmless naming\ndrift \u2014 optional).\n\n### H. draw_screen vs init()/display_init() \u2014 known intentional diffs\nThe port already matches upstream's right-column labels and `| Time:` exactly.\nDifferences to *decide on* (some are session-005 user choices):\n- Title `\"Memtestppc+ v0.01\"` vs `\"Memtest86  5.01\"` \u2014 **keep** (project identity).\n- Two-line `Memory:`/`Testing:` readout (session 005) vs upstream's single\n  `Memory:` + L3 line \u2014 user-approved divergence; keep but note \"Testing\" appears\n  twice (flagged in 005).\n- Lines 7-9: port `State:/Running` + `Pass/Errors` vs upstream\n  `Core#/State/Cores...Pass...Errors`. Single-CPU collapse; keep simplified but\n  align column positions where free.\n\n---\n\n## Staged execution plan (each stage ends green in QEMU)\n\nBuild/verify loop per stage = rsync \u2192 `make clean && make memtestppc.iso` on\nopti7050 \u2192 QEMU mac99 256MB VNC screenshot (see CLAUDE.md \u2192 Build & test).\nCommit after each green stage so regressions bisect cleanly.\n\n1. **Mechanical file splits, zero behavior change.** Extract print primitives +\n   mem* into `lib.c`; extract RNG into `random.c`; move `draw_screen/identify_cpu/\n   discover_memory` into `init.c` as `init()`/`display_init()`; leave `display.c`\n   as the FB backend. Wire up `Makefile`. Verify identical screen. *(Low risk;\n   pure relocation. Do first to de-risk later diffs.)*\n2. **struct vars / globals re-align (D, E).** Switch to `v->` fields, add\n   `set_defaults()`, restore `err_info`, restore upstream `tseq` literals. Verify\n   progress bars + pass counter still advance. *(Medium.)*\n3. **find_ticks re-derive (F).** Replace hardcoded switch with upstream\n   `find_ticks_for_pass/_test/find_chunks`. Verify bar percentages look sane over\n   a full pass. *(Medium \u2014 easy to get tick totals wrong; compare bar fill rate\n   before/after.)*\n4. **error.c restructure (A, B).** New `error.c` with `common_err`\n   (`PRINTMODE_ADDRESSES`), `print_err_counts` (incl. the red-fill), `do_tick`.\n   Drop the custom static error header. **Verify with a temporary fake-error\n   injection** (re-use the session-005 technique; remove before commit) and\n   screenshot against `ref/photos/Memtest86+_memory_errors.png`. *(Highest risk +\n   highest value; do after the plumbing above is stable.)*\n5. **test.c re-alignment (G).** Fix addr_tst1 50\u00d7 loop, movinvr seed/RNG,\n   block_move rotate. Verify a clean full pass still reports 0 errors and timings\n   are reasonable. *(Medium.)*\n6. **main.c skeleton tidy.** Rename/reshape `main()` toward `test_start()` shape;\n   reconcile `setup_segments` \u21c6 `compute_segments` naming; comment-out (not\n   delete) the SMP/reloc/window scaffolding with a one-line \"single-CPU port:\n   N/A\" so the skeleton still reads like upstream.\n7. **Hardware verify on ibookg32.** Deploy ELF via the uranium hop (partition\n   boot `boot hd:5,memtestppc.elf`); confirm header, bars, and \u2014 critically \u2014 the\n   restored error rows on the real 8-bit display. Re-burn CD only if the user\n   wants a new release artifact.\n\nStages 1-3 are safe plumbing; 4 is the headline. If time is short, 1+4 alone\ndeliver most of the fidelity win.\n\n## Decisions needed from the user (prose, since these shape scope)\n\n1. **How far on fidelity vs. the deliberate simplifications?** Do you want the\n   commented-out SMP/reloc/window scaffolding *kept* in `main.c` (max\n   skeleton-fidelity, more visual noise) or *removed* (cleaner, smaller file but a\n   bigger structural diff from upstream)? Invariant #1 leans toward keeping it\n   commented; confirm.\n2. **Error display:** adopt upstream's full `PRINTMODE_ADDRESSES` row\n   (page/offset/MB + good/bad/err-bits/count/cpu) even though it's denser than the\n   port's current simple row? (I recommend yes \u2014 it's the whole point of M1 and\n   matches the reference photo.)\n3. **Keep the session-005 TUI choices** (the `Memtestppc+ v0.01` title and the\n   two-line Memory/Testing readout) through the restructure, or revert toward\n   upstream's exact `Memtest86 5.01` + single `Memory:`+`L3` layout? I assume\n   keep; confirm.\n4. **`SPINSZ`** \u2014 keep the port's `0x400000` or restore upstream `0x4000000`?\n   Affects chunking + tick math; I lean toward matching upstream and re-tuning\n   only if a pass feels too long on real hardware.\n\n## Out of scope for this session\nConfig menu, serial console, SMP/relocation, DMI/SPD chipset probing, badram\npattern output, the carried-over `sleep()` stub for the bit-fade test (test 11),\nand the empirical 8-bit BGR palette order (tracked in memory + PLAN.md). The\n`movinv32` uninitialized-`k` warning can be fixed opportunistically during the\ntest.c stage.\n"
}


---

## 🤖 Claude

Research done, detailed plan written to [docs/sessions/006-port-fidelity-restructure/plan.md](docs/sessions/006-port-fidelity-restructure/plan.md).

**Headline findings:**

- **The port's display *primitives* are already faithful** (`cprint/dprint/hprint/scroll/...` match `lib.c`; `vga_buf` correctly stands in for `SCREEN_ADR`). They're just in the wrong file (`display.c`) and `dprint`'s right-justify branch was simplified.
- **The error path is the biggest divergence.** The port invented a simplified error row + static header in `main.c`; upstream draws a richer row (`page/offset/MB · good · bad · err-bits · count · cpu`) lazily via `common_err` in a dedicated `error.c`, with `PRINTMODE_ADDRESSES`. This is the core M1 win.
- **`do_tick`, `find_ticks`, the main loop, and `struct vars` usage all drifted** — the port uses standalone globals instead of the `v->msg_line/tptr/pptr` fields that already exist, and rewrote the tick math as a hardcoded switch.
- **`test.c` is mostly faithful** (every x86 asm block became upstream's commented-out C); only 3 routines need re-aligning (`addr_tst1` dropped a 50× loop, `movinvr` rolls its own RNG/seed, `block_move` uses plain rotate vs rotate-through-carry).
- **Gotcha logged:** upstream `init.c` is ISO-8859 encoded, so plain `grep` silently skips it as binary — that's why the static header looked "missing." It's drawn by `init()`+`display_init()`. Use `grep -a` / the Read tool on the upstream tree.

The plan proposes a target file layout mirroring upstream (`add error.c, init.c, lib.c, random.c`; `display.c` becomes the framebuffer backend) and a **7-stage execution order** where each stage stays green in QEMU, with the error-path restructure (stage 4) as the headline.

**Four decisions I need before implementing** (my leanings noted):

1. **Skeleton fidelity:** keep upstream's SMP/reloc/window scaffolding *commented out* in `main.c` (max fidelity, more noise) or *remove* it (cleaner, bigger diff)? Invariant #1 leans toward keeping commented — confirm.
2. **Error display:** adopt upstream's full dense `PRINTMODE_ADDRESSES` row? I recommend yes (it's the point of M1 and matches `ref/photos/Memtest86+_memory_errors.png`).
3. **Keep session-005 TUI choices** (`Memtestppc+ v0.01` title, two-line Memory/Testing readout) through the restructure, or revert toward upstream's exact layout? I assume keep.
4. **`SPINSZ`:** keep the port's `0x400000` or restore upstream's `0x4000000`? Affects chunking + tick math; I lean toward matching upstream.

---

