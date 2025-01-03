From msunews!uwm.edu!hookup!news.mathworks.com!uunet!comp.vuw.ac.nz!waikato!auckland.ac.nz!news Tue Feb 28 20:09:28 1995
Path: msunews!uwm.edu!hookup!news.mathworks.com!uunet!comp.vuw.ac.nz!waikato!auckland.ac.nz!news
From: udee205@kcl.ac.uk (Bill Chambers)
Newsgroups: sci.crypt.research
Subject: A cryptographic pseudorandom number generator
Date: 28 Feb 1995 21:38:49 GMT
Organization: -
Lines: 416
Sender: crypt-submission@cs.aukuni.ac.nz (sci.crypt.research co-moderator)
Approved: crypt-submission@cs.aukuni.ac.nz
Message-ID: <3j0559$36q@net.auckland.ac.nz>
Reply-To: udee205@kcl.ac.uk (Bill Chambers)
NNTP-Posting-Host: cs26.cs.auckland.ac.nz
X-Newsreader: NN version 6.5.0 #3 (NOV)




/* Feb 28, 95:
DESCRIPTION: This is a 32-bit pseudorandom sequence generator, which is keyed
by a character string. It is designed for machines with 32-bit (unsigned) long
ints, and 8-bit (unsigned) chars.
 
USAGE: for normal use compile with RUN_TEST set to 0 and with REKEY set to 1.
Initialise by 'void set_yy32(char *key, int keylen)'. The key can be up to 1536
bytes long, but up to 256 is recommended. Then one can delete the key for
security, as the internal tables are now set; then repeatedly use 'unsigned
long int yy32(void)'. Finally for security call set_yy32(char *key, int keylen)
again with a dummy key, followed by yy32(); this will reset the tables.
 
APPLICATION: For situations where a key-stream can be used. Usually this is
XOR'd bit-by-bit with the plaintext. Stream-ciphers should be faster and
stronger than block ciphers, but are not always suitable. Moreover certain
precautions have to be taken, such as never re-using the same key. (This is a
problem shared with the one-time-pad system.) Also "random access" to the
key-stream is impractical, so that decryption has to be from the beginning of
the text.
 
PURPOSE AND BACKGROUND: This program combines two types of stream-cipher
algorithm, and hopefully keeps the advantages of each. The slower algorithm is
used to rekey the faster one every eight steps, to foil cryptanalytic methods
that require a large amount of output for each key-value. The fast algorithm
(in yy32) is a Jenkins generator, as proposed by Bob Jenkins, 765 San Antonio
#69, Palo Alto, CA 94303 (rjenkins@us.oracle.com). This is a table-modifier,
which should be cryptographically very strong, although there is not much
theory behind it. It has resemblances to 'alleged-RC4', and was designed so
that even when scaled down it gave good results in statistical tests. The
cascade-generator (in cascade8_yy) has a known period ((2**32-1)**8), just less
than 2**256, and it is also known that the frequency distribution of the
output-values over a period is maximally flat. (For the theory see 'Fast
Software Encryption', Ross Anderson (Ed.), Lecture Notes in Computer Science
809 (1994), 51-55 (W G Chambers). The generator is closely related to the
Gollmann clock-controlled cascades.) On the other hand it is about five times
as slow as the Jenkins generator, and the code for initialising its tables is
moderately lengthy. The rekeying system enables one to have a fairly fast
algorithm with an almost certain guarantee on a lower bound for the period.
 
STRUCTURE: There are three important procedures. The first one is a
key-expansion routine 'keyexpand_yy()', only used in the initial phase, to set
the lookup tables of the second procedure, 'cascade8_yy()', a
'cascade'-generator. This procedure is not only used to set up the tables of
the third procedure 'yy32()', a Jenkins generator, but it is also used in
effect to rekey it, feeding in one pseudo-random value for every eight outputs.
 
Externally available routines are void set_yy32(char *key, int keylen) and
unsigned long yy32(void), which returns a 32-bit string. Internal routines are
keyexpand_yy() and cascade8_yy().
 
FLAGS: There are three flags for controlling the compilation. If REKEY is set
non-zero, then the operation is as described. If REKEY is set to 0, then the
programme operates as a Jenkins generator, with the cascade-generator only used
for initialisation. This is for carrying out time-trials. If RUN_TEST is set
non-zero, then the programme runs a test of a million cycles. If RUN_TEST is
set to 0, then the programme acts as a subroutine for generating random
numbers, plus a key-setting subroutine. There is also a third flag, TEST_JBOX,
which determines whether the Jbox[] look-up table is checked for certain
mathematical properties needed to guarantee the period.
 
TABLES: The following tables are used:
A) unsigned char Sbox[256] in keyexpand_yy(); this is unset when the procedure
is decommissioned by the set-up part of cascade8_yy().
B) unsigned long Sbox[256], Jbox[256] in cascade8_yy(); these have to have
special properties to guarantee the period.
C) Mtab[256] and Optab[256] in yy32(); the first table is modified slightly on
each iteration, and the second is simply for buffering the output, and is
strictly not necessary. It could be used to hold the text to be processed.
 
RESEARCH QUESTION: In a rekeyed system, how should the resources, both of
time and memory, be shared between the driving and the driven generators?
 
 
WARNING!! For 32-bit machines only!! Also at the places marked 1s-COMP it
is assumed that if an unsigned long is decreased by the addition of another
unsigned long, then overflow has occurred. **/
 
 
 
/******************* MAIN ***********************/
 
#define REKEY 1 /** if set to 0, program runs as a pure Jenkins generator **/
#define RUN_TEST 1 /** 0 if you want a random-number generator **/
 
#define JBOX_TEST 1 /** 0 if you don't want to test the J-boxes **/
#define UINT32 unsigned long int
 
#include <stdio.h>
#include <stdlib.h>
 
#if RUN_TEST
main() {
#include <string.h>
#include <time.h>
 
UINT32 val, yy32();
long int j, ta;
char key[256];
int keylen, teston, ptr;
void set_yy32();
 
static unsigned long test[] =
#if REKEY
{0xf11eef42, 0xe66d2c0e, 0x3cd0ce84, 0x13fa419f, 0x952f6a73, 0x67618a2a,
0x79020459, 0xc2a5abe4, 0x7584d1c9, 0x6a096097, 0x8df4feb0, 0xf9845e1d,
0xb913e735, 0x4ee1638c, 0xf8bd6898};
#else
{0x8ac50488, 0xe287225b, 0xa5336781, 0x30c18c59, 0x6a408ec3, 0x15e7b6ad,
0xfe6a88a7, 0xe9d2d50d, 0x3f772ed7, 0xa870f689, 0xd0669fa0, 0x6c4da4e6,
0x25be0b0e, 0xcdabd69, 0x47533b2};
#endif
 
printf("key? (0 for test): ");
scanf("%255s", key);
if(strcmp(key, "0") == 0) strcpy(key, "qwertyuiop");
 
keylen = strlen(key);
printf("key = %s, keylen = %d\n", key, keylen);
teston = (strcmp(key, "qwertyuiop") == 0);
if(teston) printf("testing op\n");
 
ptr = 0;
ta = clock();
set_yy32(key, keylen);
printf("delta-t = %d\n", clock() - ta);
ta = clock();
for(j=0; j<1000000; j++)
  {val = yy32();
   if(((j+1) & (0200000 - 1)) == 0)
     {printf("%10lx", val);
      if(teston && (test[ptr++] != val))
         printf("  DISAGREEMENT WITH TEST RESULTS\n");
       else
         printf("\n");
      }
   }
printf("delta-t = %d\n", clock() - ta);
}
#endif /** RUN-TEST **/
 
/*********** GLOBAL VARIABLES ***********/
static int Keyexpand_READY = 0, Cascade8_READY = 0, Yy32_READY = 0;
static char *Key = NULL;
static int  Keylen = 0;
static int Op_ptr = -1; /** declare yy32() has empty op-buffer table **/
 
/************* KEYEXPAND_YY **************************/
 
/** this is a byte-oriented key-expander using an invertible s-box
with three cornered swapping, similar to alleged-RC4 **/
 
static UINT32 keyexpand_yy(burn_flag)
int burn_flag;
{
UINT32 ans;
static unsigned char Sbox[256], Xx, Yy, Count;
unsigned char c, a, b;
int cc, i, keyptr, keylenp;
 
/** Note that when this routine is called for the first time, it initialises
itself before proceeding with the main business **/
 
if(burn_flag != 0)  /** BURN tables **/
  {Xx = Yy = 0;
   for(i=0; i<256; i++)
      Sbox[i] = 0;
   Keyexpand_READY = 0;
   Key = NULL; Keylen = 0;
   return 0L;}
 
if(!Keyexpand_READY)
  {if((Key == NULL) || (Keylen <= 0))
     {fprintf(stderr, " No Key set\n"); exit(1);}
   keylenp = Keylen + (01 ^ (Keylen & 01)); /** make it odd! **/
   keyptr=0;
 
   Xx = 1; Yy = 2;
   for(i=0; i<256; i++)
      Sbox[i] = i;
 
   for(cc=0; cc<6; cc++) /** do it six times for a real stir **/
      for(i=0; i<256; i++)
        {Xx ^= (keyptr < Keylen? Key[keyptr]: Keylen & 0377);
         if(++keyptr >= keylenp) keyptr = 0;
         Yy = Sbox[((Yy^(i+cc)) + Sbox[Xx]) & 0377];
         Xx = Sbox[Xx ^ Sbox[Yy]];
         c = Sbox[i]; /* gives a perm even if (Xx==i) || (Yy==i) || (Xx==Yy) */
         Sbox[i] = Sbox[Xx];
         Sbox[Xx] = Sbox[Yy];
         Sbox[Yy] = c;
         }
   Count = 0;
   Keyexpand_READY=1;
   }
 
for(ans=0, cc=0; cc<4; cc++)
  {Yy = Sbox[((Yy^Count) + Sbox[Xx]) & 0377];
   Xx = Sbox[Xx ^ Sbox[Yy]];
   c = Sbox[Count];
   a = Sbox[Count] = Sbox[Xx];
   b = Sbox[Xx] = Sbox[Yy];
   Sbox[Yy] = c;
   Count = (Count + 1) & 0377;
   ans = (ans << 8) ^ Sbox[((c^a) + b) & 0377];
   }
return ans;
}
 
/*********** CASCADE8_YY *****************/
 
/** See 'Fast Software Encryption', Ross Anderson (Ed.),
LNCS 809 (1994), 51-55 (W G Chambers) and 127-134 (David Wheeler) **/
 
/* Routine is FIRST called by set_yy32() to set up tables and then
decommission keyexpand_yy() */
 
static UINT32 cascade8_yy()
{
#define BPW32 32 /** assumed to be number of bits in a word **/
#define NREGS 8 /* the length of the cascade */
#define NREGSM1 (NREGS-1)
#define L2SBS 8 /** log of sbox size **/
#define L2JBS 8 /** log of jump-box size **/
#define JBS (01L << L2JBS)
#define MASKJB (JBS -1) /* for masking bits to select jump */
 
long int j, nbits, k, count, pp, ptr;
UINT32 keyexpand_yy(), old_r, r, op, inval, p_sum[5], t0,t1, temp, sum;
static UINT32
   RegB, Reg[NREGSM1], /* counters mod 2^32-1 */
   StepB, Jbox[JBS], /* jump table */
   Sbox[01L << L2SBS]; /* specially organised s-boxes */
 
static UINT32 Pf[5] = {3, 5, 0x11, 0x101, 0x10001}; /* prime factors of 2^32-1 */
/* 2^32-1 divided by its five prime factors in turn */
static UINT32 Nn[5] = {0x55555555, 0x33333333, 0xf0f0f0f, 0xff00ff, 0xffff};
 
/** Note that when this routine is called for the first time, it initialises
itself with several calls to keyexpand_yy(), which is finally decomissioned.
The output is a dummy value **/
 
if(Cascade8_READY) /** Here is the part executed every time except the FIRST **/
  {old_r = RegB;
   if((r = old_r + StepB) < old_r) r++; /* 1s-COMP */
   op = RegB = r;
   op = (op >> L2SBS) ^ Sbox[op & ((01L<<L2SBS)-1)];
 
   for(count=NREGSM1; count--; ) /* in effect count drops from NREGSM1-1 to 0 */
     {old_r = Reg[count];
      if((r = old_r + Jbox[op & MASKJB]) < old_r) r++; /* 1s-COMP */
      op += (Reg[count] = r);
      op = (op >> L2SBS) ^ Sbox[op & ((01L<<L2SBS)-1)];
      }
   return op;
   }
                /** INITIALISE: set registers to non-zero values **/
if((RegB = keyexpand_yy(0)) == 0)
   RegB = 1;
for(j=NREGSM1; j--; ) /* in effect j drops from NREGSM1-1 to 0 */
   if((Reg[j] = keyexpand_yy(0)) == 0)
      Reg[j] = (NREGSM1 + 1) - j;
 
        /** set up Jbox, last two entries done later. Each entry is rel prime
        to 2^32 -1, with the sum of all entries zero mod 2^32 - 1. **/
for(ptr=0; ptr<JBS; ptr++)
  {inval = keyexpand_yy(0);
   for(old_r=0, pp=0; pp<5; pp++)
     {nbits = 01 << pp;
      temp = (1 + (inval & ((01L << nbits) - 1)));
      inval >>= nbits;
      if((r = old_r + temp*Nn[pp]) < old_r) r++; old_r = r; /* 1s-COMP */
      }
   Jbox[ptr] = old_r;
   }
StepB = Jbox[JBS-1]; /** last entry transferred to StepB **/
        /** prepare last two entries so that sum is multiple of 2**32-1 **/
for(sum=0, ptr=0; ptr<JBS-1; ptr++)
  {if((r = sum + Jbox[ptr]) < sum) r++; sum = r;} /* 1s-COMP */
t0 = Jbox[JBS-2]; t1 = 0xffffffff - sum; /** negative mod 2**32-1 **/
for(pp=0; pp<5; pp++)
   if(t1%Pf[pp] == 0)
     {if((r = t0 - Nn[pp]) > t0) r--; t0 = r; /* 1s-COMP, drop t0 */
      if((r = t1 + Nn[pp]) < t1) r++; t1 = r; /* 1s-COMP, raise t1 */
      if((t0%Pf[pp]) == 0) /* do it again */
        {if((r = t0 - Nn[pp]) > t0) r--; t0 = r;  /* 1s-COMP, drop t0 */
         if((r = t1 + Nn[pp]) < t1) r++; t1 = r;} /* 1s-COMP, raise t1 */
      }
Jbox[JBS-2] = t0; Jbox[JBS-1] = t1;
 
#if JBOX_TEST   /** test your Jbox **/
for(pp=0; pp<5; pp++)
   p_sum[pp] = 0;
for(ptr=0; ptr<JBS; ptr++)
  {for(pp=0; pp<5; pp++)
     {p_sum[pp] += (op = Jbox[ptr] % Pf[pp]);
      if(op == 0)
        {fprintf(stderr, "CRASH on Jbox setup!!\n"); exit(1);}
      if(((ptr+1) & 0377) == 0)
         p_sum[pp] %= Pf[pp]; /* overflow prevention */
      }
   }
for(pp=0; pp<5; pp++)
   if((p_sum[pp] % Pf[pp]) != 0)
     {fprintf(stderr, "sum-test CRASH on Jbox setup!!\n"); exit(1);};
for(pp=0; pp<5; pp++)
   if((StepB % Pf[pp]) == 0)
     {fprintf(stderr, "CRASH on StepB setup!!\n"); exit(1);}
#endif /** JBOX_TEST **/
                /** set up special s-boxes **/
/** the top L2SBS bits are a perm of the address, and the rest are random. Thus
when later the op is processed by
   op = (op >> L2SBS) ^ Sbox[op & ((01L<<L2SBS)-1)]
we have a reversible mapping and a r-circ shift. The r-shift MUST be logical,
with zero-fill on the left, so 'op' must be unsigned long.
See Lecture Notes in Computer Science 809 (1994),127-134 (David Wheeler). **/
 
for(k=(01L<<L2SBS)-1; k>=0; k--) /* set up the permutation */
   Sbox[k] = k;
for(k=(01L<<L2SBS)-1; k>=0; k--)
  {j = keyexpand_yy(0)%(k+1);
   r = Sbox[k];
   Sbox[k] = Sbox[j];
   Sbox[j] = r;
   }
for(k=(01L<<L2SBS)-1; k>=0; k--)
   Sbox[k] = (Sbox[k] << (BPW32-L2SBS))
      ^ (keyexpand_yy(0) & ((01L<<(BPW32-L2SBS))-1));
 
keyexpand_yy(-1); /** force BURN of keyexpand's tables **/
Cascade8_READY = 1;
return 0L;
}
 
/************* SET_YY32 *****************/
 
void set_yy32(key, keylen)
char *key;
int keylen;
{
UINT32 cascade8_yy();
 
Key = key; Keylen = keylen; /** put as global variables, and unset all flags **/
Keyexpand_READY = Cascade8_READY = Yy32_READY = 0;
Op_ptr = -1;
 
cascade8_yy(); /* dummy FIRST call to set tables and decomission keyexpand_yy */
}
 
/************* YY32 *********************/
 
/** use modified PRNG suggested by Bob Jenkins, 765 San Antonio
#69, Palo Alto, CA 94303 (rjenkins@us.oracle.com), but with odd changes **/
 
UINT32 yy32()
{
#define BPW32 32 /** assumed to be number of bits in a word **/
#define L2MTS 8 /** log of mtab size **/
#define MTS (01 << L2MTS) /** type 'int' **/
#define MASKMT (MTS -1) /* for masking adress of mtab[] */
 
static UINT32 Optab[MTS], Mtab[MTS], A, B;
register UINT32 inject, x, y, a, b;
register int ptr;
UINT32 cascade8_yy();
 
/** Note that when this routine is called for the first time, it initialises
itself with several calls to cascade8_yy(), which is also called once in
every eight iterations to provide 'rekeying' **/
 
if(Op_ptr >= 0) /** set by set_yy32() **/
   return Optab[Op_ptr--];
 
if(!Yy32_READY)
  {B = cascade8_yy();
   A = cascade8_yy();
   for(ptr=MTS; ptr--; ) /* ptr effectively drops from MTS-1 to 0 */
      Mtab[ptr] = cascade8_yy();
   Yy32_READY = 1;
   }
 
#if REKEY
a = A; b = B; /** take out of static storage **/
for(ptr=MTS; ; )
  {if((ptr-- & 07) == 0) /** decrement ptr here! **/
     {if(ptr >= 0)  /** strange loop-exit to cut down on 'if's **/
         inject = cascade8_yy();
       else
         break;
      }
   a = ((a << 19) ^ (a >> (BPW32-19))) + Mtab[ptr ^ (MTS/2)];
   x = Mtab[ptr];
   y = (a+b) ^ Mtab[x & MASKMT];
   Mtab[ptr] = inject ^ y;
   Optab[ptr] = b = x + Mtab[y >> (BPW32 - L2MTS)];
   }
A = a; B = b;
 
#else /** NOT REKEY **/
a = A; b = B; /** take out of static storage **/
for(ptr=MTS-1; ptr>=0; ptr--)
  {a = ((a << 19) ^ (a >> (BPW32-19))) + Mtab[ptr ^ (MTS/2)];
   x = Mtab[ptr];
   y = (a+b) ^ Mtab[x & MASKMT];
   Mtab[ptr] = y;
   Optab[ptr] = b = x + Mtab[y >> (BPW32 - L2MTS)];
   }
A = a; B = b;
#endif /** REKEY **/
 
Op_ptr = MTS-1;
return Optab[Op_ptr--];
}


