CC = powerpc-linux-gnu-gcc
LD = powerpc-linux-gnu-ld
CFLAGS = -Wall -O1 -ffreestanding -fno-builtin -nostdlib -mcpu=750 -mbig-endian -fno-pic -fno-pie -mno-toc -Wa,-mregnames
LDFLAGS = -T linker.ld -nostdlib -N

all: test.elf

test.elf: head.o main.o
	$(LD) $(LDFLAGS) -o $@ head.o main.o

head.o: head.S
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o test.elf

.PHONY: all clean
