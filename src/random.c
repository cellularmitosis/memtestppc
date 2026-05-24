/* random.c - MemTest-86+ V2.00 random number generator
 *
 * ----------------------------------------------------
 * memtestppc+ port: line-faithful port of memtest86+ v2.00 random.c to
 * PowerPC / Open Firmware (single-CPU). This file is PURE C in upstream —
 * no x86 leaf code (no rdtsc, no asm). The pseudo-random sequence is
 * platform-independent and must reproduce byte-for-byte, so this file is
 * imported VERBATIM. See docs/sessions/006-port-fidelity-restructure/
 * PORTING-GUIDE.md and port-random_c.md. Nothing from the old
 * attic/src-v0.01 hack is inherited.
 *
 * NB: the rdtsc-vs-default seeding and the hand-tuned asm `rand` fill loops
 * live at the CALL SITES in test.c (and main.c), NOT here — those are ported
 * in a later wave. This file only owns the generator itself.
 *
 * NB: upstream random.c does NOT #include "test.h"; it declares its own
 * `rand`/`rand_seed` prototypes below. Keeping that arrangement avoids a
 * redeclaration clash with test.h's slightly-different (but ABI-identical on
 * ILP32 BE PPC) prototypes (`ulong rand()` / `void rand_seed(int,int)`).
 * See the report for the full reconciliation.
 */

/* concatenation of following two 16-bit multiply with carry generators */
/* x(n)=a*x(n-1)+carry mod 2^16 and y(n)=b*y(n-1)+carry mod 2^16, */
/* number and carry packed within the same 32 bit integer.        */
/******************************************************************/

unsigned int rand( void );           /* returns a random 32-bit integer */
void  rand_seed( unsigned int, unsigned int );      /* seed the generator */

/* return a random float >= 0 and < 1 */
#define rand_float          ((double)rand() / 4294967296.0)

static unsigned int SEED_X = 521288629;
static unsigned int SEED_Y = 362436069;


unsigned int rand ()
   {
   static unsigned int a = 18000, b = 30903;

   SEED_X = a*(SEED_X&65535) + (SEED_X>>16);
   SEED_Y = b*(SEED_Y&65535) + (SEED_Y>>16);

   return ((SEED_X<<16) + (SEED_Y&65535));
   }


void rand_seed( unsigned int seed1, unsigned int seed2 )
   {
   if (seed1) SEED_X = seed1;   /* use default seeds if parameter is 0 */
   if (seed2) SEED_Y = seed2;
   }
