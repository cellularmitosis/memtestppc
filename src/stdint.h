#ifndef I386_STDINT_H
#define I386_STDINT_H

/* ----------------------------------------------------------------------------
 * memtestppc+ port note (session 006, v2.00 -> PowerPC/Open Firmware):
 *
 * Imported verbatim from the v2.00 i386 stdint.h. The original is freestanding
 * (no libc) and was written for the x86 ILP32 model: char=8, short=16, int=32,
 * long=32, long long=64, pointer=32.
 *
 * Our target is 32-bit big-endian PowerPC ELF built with powerpc-linux-gnu-gcc,
 * which is ALSO ILP32 with the identical width assignment. So every typedef
 * below is already correct for PPC32 as-is — no width changes are required:
 *
 *   uint8_t  = unsigned char       (8)   OK
 *   uint16_t = unsigned short      (16)  OK
 *   uint32_t = unsigned int        (32)  OK on PPC32 (int is 32, like x86)
 *   uint64_t = unsigned long long  (64)  OK
 *   intptr_t = int / uintptr_t = unsigned int (32)  OK (PPC32 pointers are 32-bit)
 *
 * Note these widths are byte-width definitions only — endianness does not enter
 * here. Big-endianness is a representation property, transparent to the C type
 * system. The kept "I386_STDINT_H" include guard is just a token; left as-is to
 * keep the diff against upstream minimal.
 * ------------------------------------------------------------------------- */

/* Exact integral types */
typedef unsigned char      uint8_t;
typedef signed char        int8_t;

typedef unsigned short     uint16_t;
typedef signed short       int16_t;

typedef unsigned int       uint32_t;
typedef signed int         int32_t;

typedef unsigned long long uint64_t;
typedef signed long long   int64_t;

/* Small types */
typedef unsigned char      uint_least8_t;
typedef signed char        int_least8_t;

typedef unsigned short     uint_least16_t;
typedef signed short       int_least16_t;

typedef unsigned int       uint_least32_t;
typedef signed int         int_least32_t;

typedef unsigned long long uint_least64_t;
typedef signed long long   int_least64_t;

/* Fast Types */
typedef unsigned char      uint_fast8_t;
typedef signed char        int_fast8_t;

typedef unsigned int       uint_fast16_t;
typedef signed int         int_fast16_t;

typedef unsigned int       uint_fast32_t;
typedef signed int         int_fast32_t;

typedef unsigned long long uint_fast64_t;
typedef signed long long   int_fast64_t;

/* Types for `void *' pointers.  */
/* PPC32: pointers are 32-bit, so int/unsigned int are the correct widths —
 * identical to the x86 ILP32 definitions, kept verbatim. */
typedef int                intptr_t;
typedef unsigned int       uintptr_t;

/* Largest integral types */
typedef long long int      intmax_t;
typedef unsigned long long uintmax_t;


#endif /* I386_STDINT_H */
