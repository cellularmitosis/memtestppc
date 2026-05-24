# memtestppc+ Makefile
#
# Build with a powerpc cross-compiler (e.g. powerpc-linux-gnu-gcc).
# The default CROSS_COMPILE works with Debian's gcc-powerpc-linux-gnu package.

CROSS_COMPILE ?= powerpc-linux-gnu-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -Wall -O1 -ffreestanding -fno-builtin -nostdlib \
         -mcpu=750 -mbig-endian -fno-pic -fno-pie -mno-toc \
         -fno-stack-protector -Wa,-mregnames -I src

LIBGCC = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
LDFLAGS = -T src/linker.ld -nostdlib -N

# Wave 0 (substrate) object set. Grows as upstream v2.00 files are ported:
# Wave 1 test.h/config.h; Wave 2 screen_buffer/lib/random; Wave 3 init/memsize;
# Wave 4 error/test; Wave 5 main (replaces the smoke-test stub). See
# docs/sessions/006-port-fidelity-restructure/HANDOFF.md.
OBJS = src/head.o src/ofw.o src/display.o src/lib.o src/screen_buffer.o src/random.o src/init.o src/memsize.o src/error.o src/test.o src/main.o

all: memtestppc.elf memtestppc.bin

memtestppc.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBGCC)

memtestppc.bin: memtestppc.elf
	$(OBJCOPY) -O binary $< $@

src/head.o: src/head.S
	$(CC) $(CFLAGS) -c -o $@ $<

src/ofw.o: src/ofw.c src/ofw.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/display.o: src/display.c src/display.h src/font_vga.h src/ofw.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/lib.o: src/lib.c src/test.h src/display.h src/ofw.h src/ppc.h src/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/screen_buffer.o: src/screen_buffer.c src/screen_buffer.h src/test.h src/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/random.o: src/random.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/init.o: src/init.c src/test.h src/display.h src/ofw.h src/ppc.h src/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/memsize.o: src/memsize.c src/test.h src/ofw.h src/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/error.o: src/error.c src/test.h src/display.h src/ofw.h src/ppc.h src/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/test.o: src/test.c src/test.h src/ppc.h src/ofw.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Throwaway smoke-test stub; replaced by the ported v2.00 main.c in Wave 5.
src/main.o: src/main.c src/display.h src/ofw.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o memtestppc.elf memtestppc.bin memtestppc.iso
	rm -rf cd_root

# Build bootable CD image.
#
# Uses genisoimage's classic-HFS hybrid (-hfs -part -hfs-bless), the recipe
# that boots on real New World Apple Open Firmware (the prior xorrisofs
# -hfsplus image booted under QEMU/OpenBIOS but gave a blinking folder on a
# real iBook G3 — see docs/sessions/005). Layout:
#   /memtestppc.elf      the program, loaded by the boot script
#   /boot/ofboot.b       CHRP boot script, HFS type 'tbxi' (via hfs.map),
#                        in the blessed folder so OF finds it on hold-C.
memtestppc.iso: memtestppc.elf cd/ofboot.b cd/hfs.map
	mkdir -p cd_root/boot
	cp memtestppc.elf cd_root/
	cp cd/ofboot.b cd_root/boot/
	cp cd/hfs.map cd_root/boot/
	genisoimage -hide-rr-moved -hfs -part \
		-map cd/hfs.map \
		-no-desktop \
		-hfs-volid memtestppc \
		-hfs-bless cd_root/boot \
		-pad -l -r -J -v -V MEMTESTPPC \
		-o $@ cd_root/
	rm -rf cd_root

# Test in QEMU (mac99 with graphical display, boots from CD)
qemu: memtestppc.iso
	qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d

# Test in QEMU (nographic, boots from CD)
qemu-nographic: memtestppc.iso
	qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d -nographic

.PHONY: all clean qemu qemu-nographic
