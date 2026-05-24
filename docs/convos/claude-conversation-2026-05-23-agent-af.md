# Claude Conversation Log

Session ID: agent-af32416074af49b43
Date: 2026-05-23 21:15:51

---

## 🤖 Claude (Orchestrator)

Research how to create a bootable CD-ROM image for Open Firmware PowerPC Macs that boots an ELF binary. I need to understand:

1. What format does the CD need to be? (HFS? ISO 9660? hybrid?)
2. Where does the bootable ELF binary need to be placed on the CD?
3. What tools create such an image on Linux/macOS?
4. How does OF find the boot file on the CD?
5. Are there any special requirements for the ELF binary (e.g., CHRP script, tbxi blessing)?

Context: I'm building a standalone memory tester (like memtest86+) for PPC Macs. The binary is ELF32-powerpc, about 32KB. It needs to boot directly from CD into my code (no OS). The target machines range from iMac G3 (Old World) to Power Mac G5 (New World). QEMU g3beige and mac99 are the test environments.

I'm specifically interested in the QEMU/OpenBIOS path as well as real hardware. Report what you find in under 300 words, with concrete command examples.

---

## 🤖 Claude (Subagent)


🔧 Using tool: mcp__ccd_session__mark_chapter
Input: {
  "title": "Bootable PPC CD-ROM research",
  "summary": "Researching how to create a bootable CD-ROM for Open Firmware PowerPC Macs with an ELF binary"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: ToolSearch
Input: {
  "query": "select:WebSearch",
  "max_results": 1
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "bootable CD-ROM PowerPC Open Firmware ELF binary HFS ISO 9660 mkisofs grub"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "grub-mkrescue powerpc bootable CD HFS hybrid ISO tbxi blessing Open Firmware"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "QEMU OpenBIOS PowerPC boot CD-ROM ELF image mac99 g3beige"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "mkisofs genisoimage -hfs -boot-hfs-file HFS bootable PowerPC CD yaboot"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "xorriso -hfsplus PowerPC boot CD \"chrp-boot-part\" OR \"hfs-bless\" OR \"-map\" tbxi"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "FreeBSD generate-hfs.sh powerpc boot1.chrp CHRP script bootable CD Open Firmware"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "OpenBIOS QEMU PowerPC boot cdrom ELF kernel directly \"-kernel\" bare metal"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "site:github.com freebsd-src release powerpc mkisoimages.sh"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "Open Firmware PowerPC \"boot cd\" \"\\\\:tbxi\" CHRP bootinfo.txt script ELF load"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "xorriso mkisofs options \"-hfs-bless\" \"-hfs-bless-by\" chrp \"-apple\" PowerPC boot CD create"
}


---

## 🤖 Claude (Subagent)

Now let me look at the FreeBSD mkisoimages.sh script and the CHRP bootinfo.txt format for concrete details.

---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "\"bootinfo.txt\" CHRP-BOOT boot-script \"load\" \"go\" Forth Open Firmware PowerPC example"
}


---

## 🤖 Claude (Subagent)


🔧 Using tool: WebSearch
Input: {
  "query": "QEMU ppc \"-kernel\" ELF \"-M g3beige\" OR \"-M mac99\" bare metal boot directly OpenBIOS bypass"
}


---

## 🤖 Claude (Subagent)

Here is what I found:

---

## Bootable PPC CD-ROM for Open Firmware: Summary

**1. Format: HFS/ISO 9660 hybrid**

The CD must contain an Apple Partition Map with an HFS (or HFS+) partition alongside the ISO 9660 filesystem. Open Firmware on New World Macs looks for the HFS partition and its blessed boot file. Pure ISO 9660 alone will not boot on real Apple hardware -- OF needs the Apple Partition Map and HFS blessing.

**2. Boot file placement and blessing**

Your ELF binary is wrapped by (or referenced from) a **CHRP boot script** -- a small XML file traditionally named `bootinfo.txt` or `ofboot.b`, placed in a directory like `/ppc/chrp/`. The script gets HFS type `tbxi` and its containing directory gets "blessed" via `hattrib -b`. The CHRP script contains a Forth `boot-script` that loads your actual ELF binary. Example bootinfo.txt:

```xml
<CHRP-BOOT>
<BOOT-SCRIPT>boot cd:,\mykernel.elf</BOOT-SCRIPT>
</CHRP-BOOT>
```

OF finds the boot file by looking for the `tbxi`-typed file in the blessed directory of the HFS partition (the `\:tbxi` path shorthand).

**3. Tools**

- **`xorriso`/`xorrisofs`** (preferred, modern): supports `-hfsplus`, `-apm-block-size`, `-hfs-bless-by`, and `-hfsplus-file-creator-type chrp tbxi` to create the hybrid image.
- **`genisoimage`/`mkisofs`** (older): with `-apple` or `-hfs` flags plus `hfsutils` (`hmount`, `hattrib -b`, `hattrib -c chrp -t tbxi`).
- **FreeBSD's `mkisoimages.sh`** is an excellent reference implementation.

Concrete example with xorrisofs:

```sh
mkdir -p cd_root/ppc/chrp
cp memtest.elf cd_root/ppc/chrp/
cp bootinfo.txt cd_root/ppc/chrp/
xorrisofs -R -hfsplus -apm-block-size 2048 \
  -hfsplus-file-creator-type chrp tbxi cd_root/ppc/chrp/bootinfo.txt \
  -hfs-bless-by p /ppc/chrp \
  -o boot.iso cd_root/
```

**4. QEMU shortcut**

For testing, bypass the CD entirely:

```sh
qemu-system-ppc -M mac99 -kernel memtest.elf
qemu-system-ppc -M g3beige -kernel memtest.elf
```

The `-kernel` flag loads your ELF directly, skipping OpenBIOS boot device search. For CD boot testing in QEMU, create the hybrid ISO as above and pass it with `-cdrom boot.iso`.

**5. ELF binary requirements**

The ELF binary itself needs no special wrapper -- OF/OpenBIOS can load a standard ELF32-powerpc executable directly. The CHRP script is the "special requirement": it is the blessed `tbxi` file that OF executes, and its Forth boot-script tells OF where to `load` and `go` with your ELF. On Old World Macs (iMac G3 rev A/B), the situation is more complex -- they use a BootROM-based boot process rather than CHRP, and CD booting is generally not supported without a Mac OS System Folder. Your primary targets should be New World machines (iMac G3 slot-loading and later) and QEMU.

---

