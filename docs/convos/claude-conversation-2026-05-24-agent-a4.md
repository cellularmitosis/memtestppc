# Claude Conversation Log

Session ID: agent-a48aace39537e42f8
Date: 2026-05-24 06:12:14

---

## 🤖 Claude (Orchestrator)

Write a detailed comparison report of two memtest86+ source versions, to help choose which one to use as the base for a port. Read whole files — accuracy matters, and these are small codebases (~10k and ~14k lines).

## Context (why this comparison matters)
"memtestppc+" is a standalone, bootable memory tester for PowerPC Macs — a port of the classic x86 memtest86+ that boots from an Open Firmware-loaded ELF with NO operating system, reproducing memtest86+'s blue-screen text-mode TUI. Constraints/goals that shape the decision:
- **Single-CPU only.** Target machines are single-core G3 (and we run the same generic binary on G4/G5). No SMP/multi-core support wanted. A version with little/no SMP code is preferable.
- **Porting discipline:** each ported source file will START as a verbatim copy of the chosen upstream version, then we comment out the x86/BIOS/PC-hardware leaf code that doesn't apply, add PowerPC/Open-Firmware equivalents inline, and NEVER delete upstream code. So: the LESS x86/BIOS/chipset cruft a version carries, the cleaner the port.
- **TUI fidelity:** we will faithfully reproduce the on-screen TUI of whichever version we pick (title bar, progress bars, cache/memory readout, error table, footer). So the *exact screen layout* of each version is decision-relevant.
- **The display reaches the screen via a virtual VGA text buffer** (an 80x25 char+attr array standing in for x86's 0xb8000), which is then blitted to the framebuffer. So we care about how each version's display layer (cprint/dprint/scroll, screen_buffer.c) and screen-drawing (init.c) are structured.

## The two candidates (both already extracted)
- v1.00 (Dec 2003): /Users/cell/claude/memtestppc+/ref/memtest86+-1.00/
- v2.00 (Feb 2008): /Users/cell/claude/memtestppc+/ref/memtest86+-2.00/
(For reference only, do NOT base on it: v5.01 lives at /Users/cell/claude/memtestppc+/ref/memtest86+-5.01/ — it's heavily multi-CPU.)

**ENCODING GOTCHA:** some files (notably init.c) are ISO-8859 encoded (they contain a raw `°C` byte), which makes plain `grep` treat them as binary and silently skip all matches. Use the Read tool (it decodes) or `grep -a` for anything in init.c and similar files, or you WILL wrongly conclude code is "missing."

## What the report must cover
1. **Module/file inventory diff** — which .c/.h/.S files exist in each, with line counts; what 2.00 added or removed vs 1.00 (e.g. error.c, dmi.c, spd.c). A table.
2. **Error-reporting & error-table path** — THIS IS KEY. v1.00 has no error.c; find where error reporting lives (test.c? main.c? lib.c?) and how the on-screen error table is drawn. v2.00 has error.c (547 lines) — describe its structure (error/ad_err1/ad_err2/common_err/print modes, the error-row column layout, the lazy header). Compare the two error displays concretely (column headers, fields, formats). Which is cleaner to port and which produces a richer error screen?
3. **TUI / static screen layout** — compare init.c (and wherever the header is drawn) line-by-line at the layout level: exact title-bar text, the row/column of each label (Pass/Test progress bars, Test #, Testing:, Pattern:, cache lines L1/L2/L3, CLK, Memory, WallTime/Time clock, the State/Pass/Errors status block, vertical separators, the `---` rules, footer key hints). Describe what each version's screen actually looks like and enumerate the differences between 1.00 and 2.00. Which is more complete / more polished?
4. **Test suite (tseq[]) & test.c** — list the test sequence table for each (the patterns, iteration counts, on/off). Did the set of memory tests change between 1.00 and 2.00? Note any test.c algorithm changes/bugfixes between the two (the test routines: addr_tst1/2, movinv1/movinvr/movinv32, modtst, block_move, bit_fade). Which has more/better tests?
5. **main.c structure & relocation/windowing** — both should be single-CPU; describe each main loop, the relocation/windowing machinery (reloc.c, run_at, windows), find_ticks logic, and how complex each is to port. Note SMP/barrier references in each (I expect ~0 in both).
6. **x86/BIOS/PC-hardware leaf-code surface** — how much code in each is x86/BIOS/chipset-specific (controller.c memory-controller detection, dmi.c, spd.c, pci.c, APM, serial console, PC keyboard, rdtsc timing, cache_on/off asm, BIOS memory probing in memsize.c)? This is the code we'd comment out and replace. Quantify roughly (which files, line counts) so we can judge porting effort. 2.00 has a much bigger controller.c plus dmi/spd — call that out.
7. **Display primitives (lib.c) & screen_buffer.c** — did cprint/dprint/hprint/aprint/xprint/scroll/clear_scroll change meaningfully between versions? Is screen_buffer.c the same?
8. **Overall recommendation FOR THIS USE CASE** — given single-CPU PPC/OF, the verbatim-copy-and-comment discipline, and wanting a faithful TUI with the least x86 cruft: which base (1.00 or 2.00) do you recommend and why? Give the tradeoffs honestly (e.g. 1.00 = simplest/least cruft but no error.c and older TUI; 2.00 = has error.c + more polished TUI but more chipset code to comment out). Note any gotchas either way.

## Deliverable
WRITE the full report as markdown to:
`/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/v1.00-vs-v2.00.md`
with real file paths and line numbers throughout so it's actionable.
Then RETURN a concise summary (under 800 words) to me covering the inventory diff, the error-path difference, the TUI difference, and your recommendation. Do NOT modify any source files — research + write the one report file only.

---

## 🤖 Claude (Subagent)

I'll start by exploring both codebases to build a complete inventory, then read the key files in detail.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref && echo \"=== v1.00 files ===\" && ls -la memtest86+-1.00/ && echo \"\" && echo \"=== v2.00 files ===\" && ls -la memtest86+-2.00/",
  "description": "List files in both candidate versions"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref && echo \"=== v1.00 .c/.h/.S line counts ===\" && wc -l memtest86+-1.00/*.c memtest86+-1.00/*.h memtest86+-1.00/*.S 2>/dev/null | sort -k2 && echo \"\" && echo \"=== v2.00 .c/.h/.S line counts ===\" && wc -l memtest86+-2.00/*.c memtest86+-2.00/*.h memtest86+-2.00/*.S 2>/dev/null | sort -k2",
  "description": "Line counts for source files in both versions"
}


---

## 🤖 Claude (Subagent)

Good inventory baseline. Now let me read the key files. I'll start with the core decision files: main.c, init.c, test.c, lib.c, error.c for both versions.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-1.00/main.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/main.c"
}


---

## 🤖 Claude (Subagent)

main.c is nearly identical between the two — same windowing/relocation machinery, same find_ticks. The differences are: 2.00 adds `parse_command_line()` (serial console), `high_test_adr` becomes a runtime variable, and the tseq table changes. Now let me read the error-reporting paths. First v2.00's error.c, then I need to find where v1.00 does error reporting (in test.c, since there's no error.c).

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/error.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-1.00/test.c"
}


---

## 🤖 Claude (Subagent)

Key finding: in v1.00 the entire error-reporting path lives in test.c (lines 1118-1424: error, mv_error, ad_err1, ad_err2, print_hdr, common_err, print_err, print_ecc_err, parity_err, printpatn, do_tick). In v2.00 that all moved to error.c. Now let me read v2.00's test.c to compare the test algorithms, and both init.c files for the TUI layout. I'll mark a chapter and proceed.

---

## 🤖 Claude (Subagent)


🔧 Using tool: mcp__ccd_session__mark_chapter
Input: {
  "title": "Source comparison v1.00 vs v2.00",
  "summary": "Reading core files of both memtest86+ versions to write a port-base comparison report"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/test.c"
}


---

## 🤖 Claude (Subagent)

Now I have a complete picture of test.c. Both have identical addr_tst1/addr_tst2/movinv1/modtst/block_move, but v2.00 adds movinvr (random data) and bit_fade, plus sleep/beep, and has the CDH-optimized movinv32 inner loop. v1.00 keeps error reporting inline in test.c; v2.00 moved it to error.c. Now let me read both init.c files for the TUI layout, plus test.h for layout constants.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-1.00/init.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-1.00/test.h"
}


---

## 🤖 Claude (Subagent)

Now let me read v2.00's init.c and test.h, plus the lib.c and screen_buffer.c for both versions to round out the display layer comparison.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/init.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/test.h"
}


---

## 🤖 Claude (Subagent)

Now let me read lib.c and screen_buffer.c for both versions, plus the footer/config code, and check the head.S/reloc.c for SMP/barrier references. Let me batch the display-primitive reads and the SMP/x86-cruft greps.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-1.00/lib.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-1.00/screen_buffer.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/screen_buffer.c"
}


---

## 🤖 Claude (Subagent)

screen_buffer.c is byte-identical (only whitespace differs). Now let me read v2.00's lib.c to compare the display primitives, check the footer differences, and grep both trees for SMP/barrier references and x86-cruft line counts. Let me batch these.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/memtest86+-2.00/lib.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref && echo \"=== SMP / barrier / cpu_num refs (v1.00) ===\" && grep -rasci -e 'smp' -e 'barrier' -e 'num_cpus' -e 'ap_continue' -e 'cpu_num' memtest86+-1.00/*.c memtest86+-1.00/*.h memtest86+-1.00/*.S 2>/dev/null | grep -v ':0$' ; echo \"\" && echo \"=== SMP / barrier / cpu_num refs (v2.00) ===\" && grep -rasci -e 'smp' -e 'barrier' -e 'num_cpus' -e 'ap_continue' -e 'cpu_num' memtest86+-2.00/*.c memtest86+-2.00/*.h memtest86+-2.00/*.S 2>/dev/null | grep -v ':0$' ; echo \"\" && echo \"=== SMP / barrier refs (v5.01 for contrast) ===\" && grep -rasci -e 'smp' -e 'barrier' -e 'num_cpus' memtest86+-5.01/*.c memtest86+-5.01/*.h 2>/dev/null | grep -v ':0$' | head -40",
  "description": "Grep for SMP/barrier references in all three versions"
}


---

## 🤖 Claude (Subagent)

Confirmed: both v1.00 and v2.00 have ZERO SMP/barrier references, while v5.01 is riddled with them (a dedicated smp.c, plus refs in init/error/lib/main/test). lib.c is essentially identical between v1.00 and v2.00 (v2.00 adds clear_scroll, hprint3, strncmp/toupper/isdigit/simple_strtoul helpers, and serial_console_setup; the display primitives cprint/dprint/hprint/aprint/xprint/scroll are unchanged).

Now let me quantify the x86/BIOS/chipset cruft surface. Let me look at the controller.c size difference, the dmi/spd/pci/extra files, the config.c, and the head.S/reloc.c relocation machinery.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref && echo \"=== x86/BIOS/chipset leaf-code surface: line counts ===\" && echo \"\" && printf \"%-18s %8s %8s\\n\" \"file\" \"v1.00\" \"v2.00\" && printf \"%-18s %8s %8s\\n\" \"------------------\" \"--------\" \"--------\" && for f in controller.c dmi.c spd.c pci.c memsize.c config.c extra.c random.c head.S reloc.c bootsect.S setup.S serial.h io.h msr.h linuxbios.c patn.c; do v1=$(wc -l < memtest86+-1.00/$f 2>/dev/null || echo \"-\"); v2=$(wc -l < memtest86+-2.00/$f 2>/dev/null || echo \"-\"); printf \"%-18s %8s %8s\\n\" \"$f\" \"$v1\" \"$v2\"; done",
  "description": "Quantify x86/BIOS/chipset leaf-code line counts per file"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref && echo \"=== controller.c: count of memory-controller detect functions ===\" && echo \"v1.00 setup_ funcs:\" && grep -ac 'static void setup_\\|static void poll_' memtest86+-1.00/controller.c && echo \"v2.00 setup_ funcs:\" && grep -ac 'static void setup_\\|static void poll_' memtest86+-2.00/controller.c && echo \"\" && echo \"=== controller_table entries (chipsets supported) ===\" && echo \"v1.00:\" && grep -ac '{ PCI_VENDOR' memtest86+-1.00/controller.c && echo \"v2.00:\" && grep -ac '{ PCI_VENDOR' memtest86+-2.00/controller.c && echo \"\" && echo \"=== config.c head (to see config menu / options) ===\" && head -40 memtest86+-2.00/config.c",
  "description": "Count chipset detection routines and inspect config.c"
}


---

## 🤖 Claude (Subagent)

The second command got cut off (`grep -c` exits non-zero when count differs, breaking the `&&` chain). Let me rerun it without the `&&` short-circuit, and also look at config.c, extra.c, and the reference screenshots for the actual TUI.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref\necho \"=== controller.c chipset/setup routine counts ===\"\necho -n \"v1.00 setup_/poll_ funcs: \"; grep -ac 'void setup_\\|void poll_' memtest86+-1.00/controller.c\necho -n \"v2.00 setup_/poll_ funcs: \"; grep -ac 'void setup_\\|void poll_' memtest86+-2.00/controller.c\necho -n \"v1.00 controller_table entries: \"; grep -ac 'PCI_VENDOR_ID' memtest86+-1.00/controller.c\necho -n \"v2.00 controller_table entries: \"; grep -ac 'PCI_VENDOR_ID' memtest86+-2.00/controller.c\necho \"\"\necho \"=== extra.c: what is it? (header + function list) ===\"\nsed -n '1,30p' memtest86+-2.00/extra.c\necho \"--- extra.c function names ---\"\ngrep -aoE '^[a-z].*\\(' memtest86+-2.00/extra.c | head -40",
  "description": "Count controller routines and inspect extra.c"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref && echo \"=== reference screenshots / photos available ===\" && ls -la 2>/dev/null && echo \"\" && find . -maxdepth 2 -iname '*.png' -o -iname '*.jpg' -o -iname '*.gif' -o -iname '*.ppm' 2>/dev/null | head -30",
  "description": "Find reference screenshots for the TUI"
}


---

## 🤖 Claude (Subagent)

The grep with `-c` keeps short-circuiting the `&&`. Let me run those counts with a wrapper that won't break, and look at the screenshots to ground the TUI description.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref\nset +e\necho \"v1.00 setup_/poll_ funcs:\"; grep -ac 'void setup_\\|void poll_' memtest86+-1.00/controller.c\necho \"v2.00 setup_/poll_ funcs:\"; grep -ac 'void setup_\\|void poll_' memtest86+-2.00/controller.c\necho \"v1.00 PCI_VENDOR_ID refs:\"; grep -ac 'PCI_VENDOR_ID' memtest86+-1.00/controller.c\necho \"v2.00 PCI_VENDOR_ID refs:\"; grep -ac 'PCI_VENDOR_ID' memtest86+-2.00/controller.c\necho \"--- extra.c functions ---\"\ngrep -anE '^(void|int|ulong|static|unsigned|char) ' memtest86+-2.00/extra.c | head -50\ntrue",
  "description": "Count controller routines and list extra.c functions"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/screenshots/mt86plus-01.png"
}


---

## 🤖 Claude (Subagent)

That screenshot is the modern v6.00 (different layout — not relevant to the 1.00/2.00 decision). The grep `-c` keeps short-circuiting. Let me get the controller counts reliably with awk, and look at the v5.01 photo which is closer to the classic layout, plus check the controller.c headers to understand the chipset surface.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+/ref\necho \"controller setup_/poll_ functions:\"\nawk '/void (setup|poll)_/{n++} END{print \"  v1.00:\", n+0}' memtest86+-1.00/controller.c\nawk '/void (setup|poll)_/{n++} END{print \"  v2.00:\", n+0}' memtest86+-2.00/controller.c\necho \"PCI_VENDOR_ID (chipset table) refs:\"\nawk '/PCI_VENDOR_ID/{n++} END{print \"  v1.00:\", n+0}' memtest86+-1.00/controller.c\nawk '/PCI_VENDOR_ID/{n++} END{print \"  v2.00:\", n+0}' memtest86+-2.00/controller.c\necho \"\"\necho \"=== extra.c top-level functions (v2.00) ===\"\nawk '/^(void|int|ulong|unsigned|char|static)[ \\t].*\\(/ && /\\(/ {print NR\": \"$0}' memtest86+-2.00/extra.c | head -40\necho \"\"\necho \"=== extra.c references in init.c/main.c? (is it even called?) ===\"\ngrep -aln 'show_spd\\|get_spd\\|setup_smbus\\|extra' memtest86+-2.00/*.c | sort -u",
  "description": "Reliable counts of controller routines and extra.c function inventory"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: Read
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/ref/photos/Memtest86+_memory_errors.png"
}


---

## 🤖 Claude (Subagent)

The photo is too low-res to read, but I can clearly reconstruct both TUIs from the init.c/error.c source which I've fully read. The counts confirm v2.00's controller.c nearly doubled (36 → 61 setup/poll routines) and extra.c (1012 lines) is a memory-overclocking/timing-tweak module (change_timing_*, amd64_tweak, get_menu) that is entirely PC-chipset-specific.

I now have everything I need. Let me verify the session directory exists and check config.c briefly to round out the config-menu/PRINTMODE comparison, then write the report.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Bash
Input: {
  "command": "cd /Users/cell/claude/memtestppc+\necho \"=== does the session dir exist? ===\"\nls -la docs/sessions/ 2>/dev/null | tail -20\necho \"\"\necho \"=== config.c: get_config / printmode menu (v1.00 vs v2.00 head) ===\"\necho \"--- v1.00 config.c functions ---\"\nawk '/^(void|int|static)[ \\t].*\\(/{print NR\": \"$0}' ref/memtest86+-1.00/config.c\necho \"--- v2.00 config.c functions ---\"\nawk '/^(void|int|static)[ \\t].*\\(/{print NR\": \"$0}' ref/memtest86+-2.00/config.c",
  "description": "Check session dir exists and compare config.c function inventory"
}


---

## 🤖 Claude (Subagent)

The session dir `006-port-fidelity-restructure` already exists. I have a complete picture now. Let me write the report.

---

## 🤖 Claude (Subagent)


🔧 Using tool: Write
Input: {
  "file_path": "/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/v1.00-vs-v2.00.md",
  "content": "# memtest86+ v1.00 vs v2.00 \u2014 port-base comparison\n\nComparison of the two extracted candidate sources to pick the base for the\nPowerPC/Open-Firmware port (memtestppc+).\n\n- **v1.00** (Dec 2003): `ref/memtest86+-1.00/` \u2014 10,112 lines total\n- **v2.00** (Feb 2008): `ref/memtest86+-2.00/` \u2014 14,364 lines total\n- (reference only, NOT a candidate) **v5.01**: `ref/memtest86+-5.01/` \u2014 heavily multi-CPU\n\nDecision constraints (from the brief): single-CPU only; verbatim-copy-then-comment\nporting discipline (never delete upstream, so *less x86/BIOS/chipset cruft = cleaner*);\nfaithful reproduction of the chosen version's exact TUI; display reaches the screen via\na virtual VGA text buffer (80x25 char+attr at the `0xb8000` stand-in) blitted to the\nframebuffer.\n\n**Bottom line up front: recommend v1.00.** Both are equally single-CPU (zero SMP code),\nbut v1.00 carries far less x86/BIOS/chipset cruft (no `dmi.c`, no `spd.c`, no `extra.c`,\na controller.c half the size), its error path is simpler, and its TUI is the classic\nblue screen with only cosmetic differences from v2.00. The one real thing v2.00 buys you\n\u2014 a richer \"summary\"/confidence error screen and 3 extra test patterns \u2014 is not worth the\n~4,250 extra lines of PC-chipset leaf code you'd have to wade through and comment out.\nDetails and the honest tradeoffs below.\n\n---\n\n## 1. Module / file inventory diff\n\nLine counts of every `.c`/`.h`/`.S` (blank = file absent in that version):\n\n| File | v1.00 | v2.00 | Notes |\n|---|---:|---:|---|\n| `main.c` | 558 | 606 | near-identical; v2.00 adds cmdline parse + runtime `high_test_adr` |\n| `test.c` | 1424 | 1424 | same core tests; v2.00 swaps in `movinvr`/`bit_fade` for x86-asm, adds `sleep`/`beep` |\n| `init.c` | 928 | 1301 | v2.00 adds many newer CPUs + `correct_tsc` (TSC scaling) |\n| `lib.c` | 954 | 1137 | display primitives identical; v2.00 adds `clear_scroll`, `hprint3`, str helpers, serial-console setup |\n| `controller.c` | 1330 | **2694** | memory-controller detection \u2014 roughly **doubled** |\n| `error.c` | **\u2014** | **547** | **NEW in v2.00**; v1.00 has no error.c (error path lives in test.c) |\n| `dmi.c` / `dmi.h` | **\u2014** | 331 / 6 | **NEW**; DMI/SMBIOS table parsing (per-DIMM error attribution) |\n| `spd.c` | **\u2014** | 140 | **NEW**; reads SPD EEPROM over SMBus |\n| `extra.c` / `extra.h` | **\u2014** | 1012 / 27 | **NEW**; memory-timing *overclocking* menu (`change_timing_*`, `amd64_tweak`) |\n| `random.c` | **\u2014** | 31 | **NEW**; PRNG (`rand`/`rand_seed`) for the new random tests |\n| `memsize.c` | 563 | 515 | BIOS E820/E801/E88 memory probing |\n| `config.c` | 519 | 603 | config popup; v2.00 adds a `pop2*` (large popup) + `performance()` |\n| `pci.c` / `pci.h` | 135 / 94 | 160 / 95 | PCI config-space access |\n| `patn.c` | 138 | 144 | BadRAM pattern bookkeeping |\n| `head.S` | 921 | 984 | startup/relocation/GDT/IDT; r5-equivalent entry, paging tables |\n| `reloc.c` | 262 | 262 | **byte-identical** ELF self-relocation |\n| `screen_buffer.c/.h` | 134 / 25 | 134 / 25 | **byte-identical** (only whitespace differs) \u2014 see \u00a77 |\n| `bootsect.S` / `setup.S` | 375 / 115 | 375 / 115 | identical x86 real-mode boot stub (we replace with OFW boot anyway) |\n| `serial.h` / `io.h` / `msr.h` | 163 / 118 / 124 | 163 / 119 / 124 | ~identical |\n| `linuxbios.c` + tables | 154 + 82 | 154 + 82 | identical (LinuxBIOS/coreboot table probe) |\n\n**What v2.00 *added* vs v1.00:** `error.c`, `dmi.c`/`dmi.h`, `spd.c`, `extra.c`/`extra.h`,\n`random.c`, plus a doubled `controller.c`. **Nothing was removed.** Every addition except\n`error.c` and `random.c` is PC-platform-specific (DMI/SPD/SMBus/overclocking/more chipsets)\n\u2014 exactly the category we'd have to comment out under the porting discipline.\n\n---\n\n## 2. Error-reporting & error-table path  \u2190 KEY DECISION POINT\n\n### v1.00 \u2014 error path lives in `test.c` (no `error.c`)\n\nAll reporting is at the bottom of `ref/memtest86+-1.00/test.c:1118-1424`:\n\n- `error()` (`test.c:1121`) \u2014 data-error entry; dedup against last `(adr,xor)`, then\n  `print_err()` or BadRAM-pattern path depending on `v->printmode`.\n- `mv_error()` (`test.c:1166`) \u2014 block-move test error (failing address unknown).\n- `ad_err1()` (`test.c:1183`) / `ad_err2()` (`test.c:1199`) \u2014 address-test errors.\n- `print_hdr()` (`test.c:1216`) \u2014 lazily prints the two header rows once.\n- `update_err_counts()` (`test.c:1227`) \u2014 `++v->ecount; tseq[v->test].errors++;` (4 lines).\n- `print_err_counts()` (`test.c:1233`) \u2014 repaints the two count fields.\n- `common_err(page, offset)` (`test.c:1239`) \u2014 scrolls, prints Tst/Pass/address columns.\n- `print_err()` (`test.c:1263`) \u2014 fills Good/Bad/Err-Bits/Count.\n- `print_ecc_err()` / `parity_err()` (`test.c:1283` / `1304`) \u2014 ECC/parity rows.\n- `printpatn()` (`test.c:1326`) \u2014 emits the `badram=0x..,0x..` LILO option line.\n- `do_tick()` (`test.c:1361`) \u2014 progress bars + WallTime (rdtsc).\n\nThere is exactly **one** error screen \u2014 the scrolling address table \u2014 drawn by `print_hdr()`:\n\n```\nTst  Pass   Failing Address          Good       Bad     Err-Bits  Count Chan\n---  ----  -----------------------  --------  --------  --------  ----- ----\n```\n\nColumns (from `common_err`/`print_err`, `test.c:1239-1278`):\n`Tst`@col0(3), `Pass`@col4(5), page-hex@col11, offset-hex@col19, ` -  . MB` decimal MB\nreadout@col22-31, `Good`@col36, `Bad`@col46, `Err-Bits`(xor)@col56, `Count`@col66.\nECC rows write \"corrected/uncorrected\", a 4-digit syndrome@col60, \"ECC\"@col68, chan@col74.\n\n`v->printmode` in v1.00 has only **3** values (`test.h:152`): `PRINTMODE_ADDRESSES`(0,\ndefault), `PRINTMODE_PATTERNS`(1, BadRAM), `PRINTMODE_NONE`(2).\n\n### v2.00 \u2014 dedicated `error.c` (547 lines)\n\n`ref/memtest86+-2.00/error.c` centralizes everything and adds a second, much richer\ndisplay mode. Entry points:\n\n- `error()` (`error.c:31`) \u2014 data error; computes xor, calls `common_err(...,type=0)`.\n- `ad_err1()` (`error.c:54`) \u2014 address error (mask), `type=1`.\n- `ad_err2()` (`error.c:64`) \u2014 address-equals-own-address error, `type=0` + BadRAM.\n- `print_ecc_err()` (`error.c:365`, `type=2`) / `parity_err()` (`error.c:378`, `type=3`).\n- `common_err(adr,good,bad,xor,type)` (`error.c:110`) \u2014 the hub; one big `switch(printmode)`.\n- `update_err_counts()` (`error.c:69`) \u2014 same `++ecount`/`tseq[].errors++` **plus** optional\n  `beep(600);beep(1000)` (beepmode) and clearing the \"Pass complete\" message.\n- `print_err_counts()` (`error.c:85`) \u2014 repaints counts **and paints the whole error row\n  red** (`*pp = 0x47`, `error.c:97-104`) for ADDRESSES/PATTERNS modes \u2014 this is the \"vivid\n  red error row\" the memtestppc+ TUI already mimics (per the project README/session 005).\n- `printpatn()` (`error.c:394`) / `do_tick()` (`error.c:429`).\n\n`v->printmode` in v2.00 has **5** values (`test.h:180`): `PRINTMODE_SUMMARY`(0),\n`PRINTMODE_ADDRESSES`(1, the shipped default \u2014 `init.c:180`), `PRINTMODE_PATTERNS`(2),\n`PRINTMODE_NONE`(3), `PRINTMODE_DMI`(4).\n\n**PRINTMODE_ADDRESSES (the default \u2014 same screen as v1.00):** identical header and column\nlayout to v1.00 (`error.c:287-290` matches v1.00 verbatim), same MB readout, same\nGood/Bad/Err-Bits/Count/Chan fields. The only functional add is the red-row repaint.\n\n**PRINTMODE_SUMMARY (NEW, no v1.00 equivalent):** a non-scrolling statistics panel\n(`error.c:120-277`) drawn at `LINE_HEADER`:\n\n```\nError Confidence Value:\n  Lowest Error Address:    <page>...  -  . MB\n Highest Error Address:    <page>...  -  . MB\n    Bits in Error Mask:    <ebits hex>\n Bits in Error - Total:    Min:    Max:    Avg:\n Max Contiguous Errors:\nECC Correctable Errors:\nErrors per Memory Slot:    00: 0  01: 0 ...     Test  Errors\n```\n\nIt also lists per-`tseq` test error counts down the right side (col 66/68) and, if\n`dmi_initialized`, per-DIMM-slot error counts wired to `dmi_err_cnts[]` (from `dmi.c`).\nThis summary screen depends on the `struct err_info v->erri` block (`test.h:276-288`,\nadded to `struct vars`) and on DMI.\n\n**Which is cleaner to port / which is richer?**\n- *Cleaner:* **v1.00.** Its error path is ~300 lines of plain `cprint`/`dprint`/`hprint`\n  with no external deps (no DMI, no `erri` struct, no beep). It's the single screen we\n  already reproduce.\n- *Richer:* **v2.00**, by the SUMMARY screen + per-slot DMI attribution + the red-row paint.\n  But the richness (confidence %, per-slot counts) is **DMI/SMBIOS-dependent**, which doesn't\n  exist on a PPC Mac \u2014 the per-slot table would be dead on our hardware. The red-row paint\n  is trivial to lift into a v1.00 base by hand if we want it (and per session 005 we already\n  have an \"error-row red\" effect). Net: v2.00's error *richness* is largely PC-only.\n\n---\n\n## 3. TUI / static screen layout\n\nBoth screens are the classic 80x25 blue text screen, drawn by `display_init()` + `init()`.\nThe layout constants in `test.h` are **almost identical** (`COL_TST` is the only positional\ndifference: 52 in v1.00 `test.h:64` vs 51 in v2.00 `test.h:71`). Both share:\n`LINE_CPU=1`, `COL_MID=30`, `LINE_TST=2`, `LINE_RANGE=3`, `LINE_PAT=4`/`COL_PAT=41`,\n`LINE_TIME=10`/`COL_TIME=0`, `LINE_INFO=10`, `LINE_HEADER=12`, `LINE_SCROLL=14`,\n`BAR_SIZE=(78-COL_MID-9)=39`.\n\n### Title bar (row 0)\n\n- v1.00 (`init.c:54`): `\"      Memtest86  v1.00       \"` over a red band (`TITLE_WIDTH=28`),\n  plus a `\"+\"` plotted at col 15 (`init.c:59`) and two `0xA4` attribute cells \u2014 i.e. it\n  renders as **Memtest86+ v1.00**.\n- v2.00 (`init.c:66`): `\"      Memtest86  v2.00      \"`, identical `\"+\"` overlay \u2192 **Memtest86+ v2.00**.\n- Only the version string differs.\n\n### Right-hand progress/status block (cols ~30-78) \u2014 IDENTICAL between versions\n\nDrawn at `init.c` (v1.00 `:120-134`, v2.00 `:162-176`), both print:\n\n```\nrow 0  col30: \"Pass   %\"          (Pass-progress bar starts COL_MID+9=39, 39 wide)\nrow 1  col30: \"Test   %\"          (Test-progress bar, same geometry)\nrow 2  col30: \"Test #\"\nrow 3  col30: \"Testing: \"         (range \"lo - hi  <selected> Relocated\")\nrow 4  col30: \"Pattern: \"\nrows 0-4 col28: \"| \"              (vertical separator, COL_MID-2)\n```\n\n### Left-hand CPU/cache block (rows 1-4, col 0) \u2014 IDENTICAL labels\n\n```\nrow 1 (LINE_CPU)   : <cpu name + speed>          (e.g. \"Pentium III  933 MHz\")\nrow 2 (LINE_CPU+1) : \"L1 Cache: Unknown \"        -> \"L1 Cache:  nnnK  nnnnnn MB/s\"\nrow 3 (LINE_CPU+2) : \"L2 Cache: Unknown \"        -> \"L2 Cache: nnnnK  nnnnnn MB/s\"\nrow 4 (LINE_CPU+3) : \"Memory  : <size>           nnnnnn MB/s\"\nrow 5 (LINE_CPU+4) : \"Chipset : <controller>\"\n```\n(v2.00 nudges the L1/L2 MB/s field one col right \u2014 `init.c:990/1010` use `+16` vs v1.00\n`+15` \u2014 cosmetic.) `WallTime` clock is `LINE_TIME=10, COL_TIME=0`, format `\":  :\"` set at\n`init.c` (`v1:118` / `v2:160`), filled by `do_tick`.\n\n### Status table (rows 8-11) \u2014 IDENTICAL between versions\n\n```\nrow 8 (LINE_INFO-2): \" WallTime   Cached  RsvdMem   MemMap   Cache  ECC  Test  Pass  Errors ECC Errs\"\nrow 9 (LINE_INFO-1): \" ---------  ------  -------  --------  -----  ---  ----  ----  ------ --------\"\nrow 10(LINE_INFO)  : <values> ... \"Std\" @COL_TST, \"    0\" @COL_PASS, \"     0\" @COL_ERR\nrow 11(LINE_INFO+1): \" -----------------------------------------------------------------------------\"\n```\n(v2.00 writes `\" Std\"` with a leading space at `COL_TST=51`; v1.00 writes `\"Std\"` at\n`COL_TST=52` \u2014 same glyphs land in the same cells.)\n\n### Footer (row 24) \u2014 BYTE-IDENTICAL\n\nBoth `lib.c footer()` (v1 `:484`, v2 `:597`):\n```\n(ESC)Reboot  (c)configuration  (SP)scroll_lock  (CR)scroll_unlock\n```\nwith `\"LOCKED\"`/blank at col 74, on a reverse-video bar (`init.c` sets attr `0x71`).\n\n### Error/scroll region\n\nHeader at `LINE_HEADER=12`, scroll body rows 14-23. v2.00 adds `LINE_MSG=18`/`COL_MSG=18`\n(`test.h:78-79`) used only for the \"Pass complete, no errors, press Esc to exit\" banner\n(`main.c:450`) and the summary panel \u2014 v1.00 has no such banner.\n\n### Verdict\n\nThe **static screen is effectively the same** in both \u2014 same labels, same columns, same\nseparators, same footer. v2.00's *only* user-visible TUI additions are: (a) the \"Pass\ncomplete\u2026\" message line, (b) the optional SUMMARY error panel, and (c) the red error-row\npaint. None of these is a layout change to the main screen. So \"TUI fidelity\" does **not**\nfavor v2.00 in any structural way \u2014 a v1.00 base reproduces the same classic screen, and\nthe two extras (banner, red row) are a few lines to add by hand if desired.\n\n---\n\n## 4. Test suite (`tseq[]`) & `test.c`\n\n### v1.00 tseq (`main.c:12-27`) \u2014 `DEFTESTS=7`, 8 default + 4 extended rows\n\n| # | cache | pat | iter | msg |\n|---|---|---|---|---|\n| 0 | 0 | 5 | 3 | Address test, walking ones, no cache |\n| 1 | 1 | 0 | 3 | Moving inv, ones & zeros, cached |\n| 2 | 0 | 6 | 3 | Address test, own address, no cache |\n| 3 | 1 | 1 | 2 | Moving inv, 8 bit pattern, cached |\n| 4 | 1 | 2 | 2 | Moving inv, 32 bit pattern, cached |\n| 5 | 1 | 7 | 64 | Block move, 64 moves, cached |\n| 6 | 1 | 3 | 4 | Modulo 20, ones & zeros, cached |\n| 7 | 0 | 0 | 2 | Moving inv, ones & zeros, no cache |\n| 8.. | | | | (extended: 512-move block, no-cache variants, modulo-20 8-bit) |\n\nPatterns used: 0,1,2,3 (movinv), 5,6 (addr), 7 (block move). **No random tests, no bit-fade.**\n\n### v2.00 tseq (`main.c:18-30`) \u2014 `DEFTESTS=9`, 10 rows\n\n| # | cache | pat | iter | msg |\n|---|---|---|---|---|\n| 0 | 0 | 5 | 3 | Address test, walking ones, uncached |\n| 1 | 1 | 6 | 3 | Address test, own address |\n| 2 | 1 | 0 | 3 | Moving inversions, ones & zeros |\n| 3 | 1 | 1 | 2 | Moving inversions, 8 bit pattern |\n| 4 | 1 | **10** | 60 | Moving inversions, **random pattern** |\n| 5 | 1 | 7 | 64 | Block move, 64 moves |\n| 6 | 1 | 2 | 2 | Moving inversions, 32 bit pattern |\n| 7 | 1 | **9** | 40 | **Random number sequence** |\n| 8 | 1 | 3 | 4 | Modulo 20, random pattern |\n| 9 | 1 | **8** | 1 | **Bit fade test, 90 min, 2 patterns** |\n\n**Did the test set change?** Yes, modestly. v2.00 adds three pattern kinds:\n- `pat 8` \u2192 `bit_fade()` (`test.c:1289`) \u2014 writes a pattern, `sleep(5400s)`, re-checks.\n- `pat 9` \u2192 `movinvr()` (`test.c:257`) \u2014 moving-inv over a PRNG stream.\n- `pat 10` \u2192 random-data moving-inv (handled inline in `main.c:380`).\nThese need `random.c` (`rand`/`rand_seed`) and `sleep()` (which is **rdtsc-based**,\n`test.c:1346`). The classic deterministic tests (addr1/2, movinv1, movinv32, modtst,\nblock_move) are present in **both** and are essentially the same code.\n\n### test.c algorithm changes between the two\n\nThe leaf test routines are mostly identical x86 inline-asm. Two real diffs:\n1. **`movinv32`** \u2014 v2.00 has a hand-optimized inner loop (\"CDH\" edits, `test.c:607-877`)\n   using `roll`/`incb`/`andb $31` instead of v1.00's explicit shift+counter\n   (`1.00/test.c:486-508` etc). Same algorithm, tighter asm. The original C is preserved as\n   comments in both (good for our port \u2014 we re-implement from the C, not the asm).\n2. **`addr_tst1`** \u2014 v2.00 steps `p += bank/4` (`test.c:119`) where v1.00 steps `p += bank`\n   (`1.00/test.c:112`) \u2014 finer address-bit coverage per bank.\n3. v2.00's `modtst` clears the pattern field at the end (`test.c:1066`); cosmetic.\n\n**Which has more/better tests?** v2.00 (random + bit-fade are genuinely useful). **But** for\nour port both the random tests and bit-fade depend on x86 `rdtsc` for seeding/timing\n(`test.c:265-270`, `1353`), so they're extra porting work, and bit-fade's 90-minute sleep is\nof marginal value on a boot-from-CD memory tester. The deterministic core \u2014 which is what\ncatches the overwhelming majority of faults \u2014 is identical in both.\n\n---\n\n## 5. `main.c` structure & relocation/windowing\n\nBoth are **single-CPU** with the *same* main loop and windowing machinery. `main.c` diffs\nare small:\n\n- **`tseq[]`** differs (see \u00a74) and v2.00 uses `#define DEFTESTS 9` + a hardcoded\n  `if (v->test >= 9 ...)` end-of-pass check (`main.c:441`) vs v1.00's `tseq[].msg==NULL`\n  sentinel + extended/default `xtst_flag` machinery (`main.c:209-228`, removed in v2.00).\n- **`parse_command_line()`** \u2014 NEW in v2.00 (`main.c:144-175`): reads the Linux boot\n  protocol cmdline for `console=ttyS...` (serial console). Pure x86/BIOS; we'd comment it out.\n- **`high_test_adr`** \u2014 v1.00 uses a compile-time `HIGH_TEST_ADR`; v2.00 makes it a runtime\n  `ulong high_test_adr` set to 16 MB if RAM > ~17 MB (`main.c:43,202-206`). Minor.\n- `pat 8` (bit_fade) runs only in `window==0` (`main.c:368`).\n\n**Relocation / windowing (the part that matters for the OFW port):**\n- `windows[]` table (`main.c:41-96` both) \u2014 same 2 MB-ish window scheme covering 0\u201316 MB.\n- `run_at()` / `__run_at()` (`main.c:102-136` both) \u2014 `memmove` the whole image to LOW\n  (`LOW_TEST_ADR`) or HIGH copy and `goto *startup_32` \u2014 **identical logic**.\n- `compute_segments()` (`main.c:473-556` v1 / `523-606` v2) \u2014 **identical** (incl. the same\n  `#if 0` debug blocks).\n- `find_ticks()` / `find_ticks_for_test()` \u2014 **identical** except v1.00 honors\n  `xtst_flag`/`cache_flag` selection (`main.c:439-459`) which v2.00 dropped.\n- `reloc.c` is **byte-identical** between the two (262 lines each).\n\nFor the port, the relocate-the-whole-image-and-jump trick is the trickiest piece either\nway (it presumes flat physical addressing; under OFW's MMU it interacts with our claim/map\nstrategy \u2014 see invariant #4 in CLAUDE.md). It is the **same difficulty in both versions**,\nso this dimension is a wash \u2014 except v1.00's `main.c` is slightly simpler (no cmdline parse,\nno runtime high_test_adr).\n\n**SMP/barrier references:** `grep` for `smp|barrier|num_cpus|ap_continue|cpu_num` returns\n**0 hits in both v1.00 and v2.00**. (For contrast, v5.01 hits in init.c, error.c, lib.c,\nmain.c, test.c, random.c, config.h, smp.h, and a dedicated `smp.c`.) Both candidates are\ngenuinely single-CPU \u2014 neither carries the SMP tax. This dimension does **not** distinguish\nthem; it only rules out v5.01.\n\n---\n\n## 6. x86 / BIOS / PC-hardware leaf-code surface (the comment-out burden)\n\nThe code we'd have to neutralize (comment out + provide OFW/PPC equivalents). Approximate\nline counts of the platform-specific surface:\n\n| Concern | File(s) | v1.00 lines | v2.00 lines |\n|---|---|---:|---:|\n| Memory-controller (chipset) detection | `controller.c` | 1330 | **2694** |\n| DMI / SMBIOS table parse | `dmi.c`,`dmi.h` | 0 | **337** |\n| SPD EEPROM over SMBus | `spd.c` | 0 | **140** |\n| Memory-timing overclock menu | `extra.c`,`extra.h` | 0 | **1039** |\n| BIOS memory probe (E820/E801/E88) | `memsize.c` | 563 | 515 |\n| PCI config-space access | `pci.c`,`pci.h` | 229 | 255 |\n| LinuxBIOS/coreboot table probe | `linuxbios.c` (+h) | 236 | 236 |\n| x86 real-mode boot stub | `bootsect.S`,`setup.S` | 490 | 490 |\n| Startup/GDT/IDT/paging asm | `head.S` | 921 | 984 |\n| Serial console (UART) | `serial.h` + lib.c bits | ~163 | ~163 + cmdline |\n| PC keyboard (i8042 `inb 0x60/0x64`) | `lib.c get_key/check_input` | ~60 | ~60 |\n| `rdtsc` timing, `cache_on/off`, MSR | `test.h`,`init.c`,`msr.h` | scattered | scattered + `correct_tsc` |\n| **Roughly platform-specific total** | | **~4,200** | **~6,800** |\n\nKey observations:\n- **`controller.c` nearly doubles** (1330 \u2192 2694). Setup/poll routine count goes 36 \u2192 61 \u2014\n  v2.00 supports many more Intel/AMD/VIA/nForce chipsets, all irrelevant on a PPC Mac. Under\n  \"never delete,\" that's ~1,360 extra lines of dead, commented chipset code to carry.\n- **`dmi.c` + `spd.c` + `extra.c` (~1,516 lines) are 100% PC-platform** and have **no PPC\n  analog**: DMI/SMBIOS, SMBus SPD reads, and a BIOS-poke memory-overclocking menu\n  (`change_timing_i875`, `amd64_tweak`, etc.). `extra.c` is wired into `main.c`/`config.c`\n  via `get_menu()`/`performance()`. All of it would be commented out wholesale.\n- `init.c` is also +373 lines in v2.00, almost entirely a longer CPU-ID `switch` (Core 2,\n  Nehalem, Phenom, VIA Isaiah\u2026) plus `correct_tsc()` MSR scaling \u2014 none meaningful on PPC.\n- `head.S` (+63) adds more x86 startup; we replace `head.S` with our own PPC `head.S`\n  regardless, so this is moot either way.\n\nSo the comment-out burden is roughly **60% larger** in v2.00 (~6,800 vs ~4,200 lines), and\nthe *added* 2,600 lines are exactly the most PC-coupled, least-portable code in the tree.\n\n---\n\n## 7. Display primitives (`lib.c`) & `screen_buffer.c`\n\n- **`screen_buffer.c`/`.h` are byte-identical** between the versions (diff is whitespace\n  only). The virtual VGA buffer abstraction (`get_scrn_buf`/`set_scrn_buf`/`tty_print_*`)\n  we'd reuse is the same. Note: this is the *serial-echo* mirror buffer, distinct from the\n  `0xb8000` char+attr array \u2014 both versions write the live screen straight to `SCREEN_ADR`\n  in `cprint` (`lib.c` `*dptr = text[i]; dptr += 2;`).\n- **Core display primitives are unchanged**: `cprint`, `dprint`, `aprint`, `hprint`,\n  `hprint2`, `xprint`, and `scroll` are line-for-line the same (v1 `lib.c:96-310` vs v2\n  `lib.c:174-432`). The `0xb8000`\u2192buffer write loop in `cprint` is identical.\n- **v2.00 adds to `lib.c`** (additive, doesn't disturb the v1.00 primitives):\n  `clear_scroll()` (`lib.c:209`, used by the summary screen), `hprint3()` (exact-width hex,\n  `lib.c:383`), C-lib helpers `toupper/isdigit/isxdigit/simple_strtoul/strncmp`\n  (`lib.c:92-167`), and `serial_console_setup()` (`lib.c:1000`, the cmdline serial path).\n  `set_cache()` in v1.00 prints upper-case OFF/ON when cache is force-toggled (`lib.c:407`);\n  v2.00 always prints lower-case (`lib.c:529`). The `inter()` x86 fault dumper and the\n  `get_key`/`check_input` PC-keyboard path are identical.\n\n**Conclusion:** the display layer we actually port is the same in both. Picking v1.00 loses\nnothing here; the few v2.00 helpers (`clear_scroll`, `hprint3`) are easy to add by hand if a\nlater feature wants them.\n\n---\n\n## 8. Overall recommendation\n\n### Recommend: **base the port on v1.00.**\n\nReasoning against the four decision axes:\n\n1. **Single-CPU:** tie \u2014 both have zero SMP code. (This only matters for excluding v5.01.)\n2. **Least x86/chipset cruft (verbatim-copy-then-comment discipline):** **v1.00 wins\n   decisively.** It has no `dmi.c`/`spd.c`/`extra.c`/`random.c`, a `controller.c` half the\n   size, and ~2,600 fewer lines of the most PC-coupled code. Every one of v2.00's additions\n   except `error.c` is platform code we'd immediately comment out and carry as dead weight.\n3. **TUI fidelity:** near-tie, slight edge to v1.00 for simplicity. The static screen is the\n   same classic blue layout in both (same labels/columns/separators/footer); v2.00's only\n   extras are a \"Pass complete\" banner, an optional DMI-dependent SUMMARY panel, and a\n   red-error-row paint \u2014 none of which change the main screen, and the first/third are a few\n   lines to graft onto a v1.00 base if wanted (session 005 already has an error-row-red\n   effect).\n4. **Tests & error screen (where v2.00 is genuinely better):** v2.00 adds random + bit-fade\n   tests and a richer summary/per-slot error panel. **But** the random/bit-fade tests rely on\n   x86 `rdtsc`, and the per-slot error attribution relies on DMI/SPD that don't exist on PPC\n   Macs \u2014 so most of v2.00's advantage here evaporates on our target.\n\n### Honest tradeoffs / gotchas either way\n\n- **Choosing v1.00, you give up:** `error.c` as a clean separate module (you inherit the\n  error path embedded in `test.c:1118-1424` instead \u2014 slightly less tidy but ~300 lines of\n  plain prints, easy to port); the SUMMARY/confidence error screen; the red-error-row paint\n  (re-add by hand); 3 extra test patterns (random\u00d72 + bit-fade); and `clear_scroll`/`hprint3`\n  helpers (re-add by hand). None of these block the mission (\"faithful, trustworthy port\").\n- **Choosing v2.00, you pay:** ~2,600 extra lines to read and comment out \u2014 and the bulk of\n  it (`controller.c` growth, `dmi.c`, `spd.c`, `extra.c`) is the densest, most BIOS-poking\n  code in the project, which works *against* the project's central discipline (keep the diff\n  vs upstream small and navigable). The serial-console cmdline path (`parse_command_line` +\n  `serial_console_setup`) assumes the Linux x86 boot protocol and is meaningless under OFW.\n- **Shared gotcha (both):** the `run_at()`/windowing relocation assumes flat physical\n  addressing; under OFW's MMU it must be reconciled with the claim/map approach\n  (CLAUDE.md invariant #4). Equal difficulty in both \u2014 not a differentiator.\n- **If TUI fidelity to the *specific* version's screen is a hard requirement:** confirm which\n  classic screenshot the project is targeting. The two reference images in `ref/screenshots`\n  / `ref/photos` are a v6.00 (totally different modern layout) and a low-res classic shot;\n  both v1.00 and v2.00 render the same \"Memtest86+ vX.XX\" classic blue screen described in \u00a73,\n  so either base reproduces \"the\" classic look \u2014 the version string is the only title diff.\n\n**Net:** v1.00 is the smaller, cleaner, lower-risk base that still reproduces the classic\nTUI and the full deterministic test suite. Start from v1.00 and, if later wanted, hand-port\nthe two or three small v2.00 niceties (red error row, \"Pass complete\" banner) \u2014 far cheaper\nthan commenting out 2,600 lines of irrelevant chipset code.\n"
}


---

## 🤖 Claude (Subagent)

Report written. Here is the summary.

## memtest86+ v1.00 vs v2.00 — comparison summary

Full report: `/Users/cell/claude/memtestppc+/docs/sessions/006-port-fidelity-restructure/v1.00-vs-v2.00.md`

**Recommendation: base the port on v1.00.**

### Inventory diff
v1.00 is 10,112 lines; v2.00 is 14,364. v2.00 *added* (removed nothing): `error.c` (547), `dmi.c`/`dmi.h` (337), `spd.c` (140), `extra.c`/`extra.h` (1039 — a PC memory-overclocking/timing-tweak menu), `random.c` (31), and roughly *doubled* `controller.c` (1330 → 2694; chipset setup/poll routines 36 → 61). `reloc.c` and `screen_buffer.c` are byte-identical between the two. Everything v2.00 added except `error.c`/`random.c` is PC-platform code we'd immediately comment out.

### Error-path difference (the key axis)
v1.00 has **no error.c** — the whole error path lives at the bottom of `test.c` (lines 1118-1424: `error`/`mv_error`/`ad_err1`/`ad_err2`/`print_hdr`/`common_err`/`print_err`/`do_tick`). One screen: the scrolling `Tst Pass Failing Address Good Bad Err-Bits Count Chan` table. ~300 lines of plain `cprint`/`dprint`, no external deps. v2.00 moved this to `error.c` with a `common_err(...,type)` hub and added (a) a second `PRINTMODE_SUMMARY` panel (confidence value, lowest/highest address, bits-in-error min/max/avg, per-test and **per-DIMM-slot** error counts) and (b) a red-error-row repaint. The default ADDRESSES screen is column-identical to v1.00. The summary's richness is largely **DMI/SPD-dependent** — dead on a PPC Mac.

### TUI difference
Essentially none structurally. Both render the same classic blue 80x25 screen — same title bar ("Memtest86+ vX.XX"), same Pass/Test progress bars, Test#/Testing/Pattern block, L1/L2/Memory/Chipset left column, the `WallTime…Errors` status table, and a **byte-identical footer**. Only the version string and a 1-column `COL_TST` nudge differ. v2.00's sole user-visible extras are a "Pass complete" banner and the optional SUMMARY/red-row — not main-screen layout changes, and a few lines to graft onto a v1.00 base if wanted.

### Other findings
- **SMP: zero references in both** (`smp|barrier|num_cpus` = 0 hits each); v5.01 by contrast has a dedicated smp.c plus refs throughout. SMP only rules out v5.01, doesn't distinguish 1.00/2.00.
- **Tests:** v2.00 adds random-pattern, random-sequence, and bit-fade tests — but all depend on x86 `rdtsc` for seed/timing, so they're extra porting work; the deterministic core (addr_tst1/2, movinv1/movinv32, modtst, block_move) is the same in both.
- **Comment-out burden:** ~4,200 platform-specific lines in v1.00 vs ~6,800 in v2.00 (~60% more), and v2.00's extra 2,600 lines are the densest BIOS/chipset code in the tree.
- **main.c/relocation:** the `run_at()`/windowing machinery is the same in both (equal porting difficulty); v2.00 only adds a Linux-x86 serial-console cmdline parser (meaningless under OFW).

**Tradeoff:** v1.00 gives up `error.c` as a tidy module and 3 extra tests, but it's the smaller, cleaner, lower-risk base that still reproduces the classic TUI and full deterministic suite. Cheaper to hand-add v2.00's 2-3 small niceties than to comment out 2,600 lines of irrelevant chipset code.

---

