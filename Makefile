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

OBJS = src/head.o src/main.o src/display.o src/test.o src/ofw.o

all: memtestppc.elf memtestppc.bin

memtestppc.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBGCC)

memtestppc.bin: memtestppc.elf
	$(OBJCOPY) -O binary $< $@

src/head.o: src/head.S
	$(CC) $(CFLAGS) -c -o $@ $<

src/main.o: src/main.c src/memtest.h src/display.h src/ofw.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/display.o: src/display.c src/display.h src/font_vga.h src/ofw.h src/memtest.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/test.o: src/test.c src/memtest.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/ofw.o: src/ofw.c src/ofw.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o memtestppc.elf memtestppc.bin memtestppc.iso
	rm -rf cd_root

# Build bootable CD image
memtestppc.iso: memtestppc.elf cd/bootinfo.txt
	mkdir -p cd_root/ppc/chrp
	cp memtestppc.elf cd_root/
	cp cd/bootinfo.txt cd_root/ppc/chrp/
	xorrisofs -R -hfsplus \
		-hfsplus-file-creator-type chrp tbxi /ppc/chrp/bootinfo.txt \
		-hfs-bless-by p /ppc/chrp \
		-o $@ cd_root/
	rm -rf cd_root

# Test in QEMU (mac99 with graphical display, boots from CD)
qemu: memtestppc.iso
	qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d

# Test in QEMU (nographic, boots from CD)
qemu-nographic: memtestppc.iso
	qemu-system-ppc -M mac99 -m 256 -cdrom memtestppc.iso -boot d -nographic

.PHONY: all clean qemu qemu-nographic
