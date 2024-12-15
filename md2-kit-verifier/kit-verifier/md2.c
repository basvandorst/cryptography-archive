/* RSA-MD2 Message Digest algorithm in C  */
/*  by Ronald L. Rivest 10/1/88  */

#include <stdio.h>

/**********************************************************************/
/* Message digest routines:                                           */
/* To form the message digest for a message M                         */
/*    (1) Initialize a context buffer md using MDINIT                 */
/*    (2) Call MDUPDATE on md and each character of M in turn         */
/*    (3) Call MDFINAL on md                                          */
/* The message digest is now in md->D[0...15]                         */
/**********************************************************************/
/* An MDCTX structure is a context buffer for a message digest        */
/*  computation; it holds the current "state" of a message digest     */
/*  computation                                                       */
struct MDCTX
{
   unsigned char  D[48];   /* buffer for forming digest in */
                           /* At the end, D[0...15] form the message */
                           /*  digest */
   unsigned char  C[16];   /* checksum register */
   unsigned char  i;       /* number of bytes handled, modulo 16 */
   unsigned char  L;       /* last checksum char saved */
};
/* The table S given below is a permutation of 0...255 constructed    */
/*  from the digits of pi.  It is a ``random'' nonlinear byte         */
/*  substitution operation.                                           */
int S[256] = {
        41, 46, 67,201,162,216,124,  1, 61, 54, 84,161,236,240,  6, 19,
        98,167,  5,243,192,199,115,140,152,147, 43,217,188, 76,130,202,
        30,155, 87, 60,253,212,224, 22,103, 66,111, 24,138, 23,229, 18,
       190, 78,196,214,218,158,222, 73,160,251,245,142,187, 47,238,122,
       169,104,121,145, 21,178,  7, 63,148,194, 16,137, 11, 34, 95, 33,
       128,127, 93,154, 90,144, 50, 39, 53, 62,204,231,191,247,151,  3,
       255, 25, 48,179, 72,165,181,209,215, 94,146, 42,172, 86,170,198,
        79,184, 56,210,150,164,125,182,118,252,107,226,156,116,  4,241,
        69,157,112, 89,100,113,135, 32,134, 91,207,101,230, 45,168,  2,
        27, 96, 37,173,174,176,185,246, 28, 70, 97,105, 52, 64,126, 15,
        85, 71,163, 35,221, 81,175, 58,195, 92,249,206,186,197,234, 38,
        44, 83, 13,110,133, 40,132,  9,211,223,205,244, 65,129, 77, 82,
       106,220, 55,200,108,193,171,250, 36,225,123,  8, 12,189,177, 74,
       120,136,149,139,227, 99,232,109,233,203,213,254, 59,  0, 29, 57,
       242,239,183, 14,102, 88,208,228,166,119,114,248,235,117, 75, 10,
        49, 68, 80,180,143,237, 31, 26,219,153,141, 51,159, 17,131, 20,
};
/*The routine MDINIT initializes the message digest context buffer md.*/
/* All fields are set to zero.                                        */
void MDINIT(md)
  struct MDCTX *md;
  { int i;
    for (i=0;i<16;i++) md->D[i] = md->C[i] = 0;
    md->i = 0;
    md->L = 0;
  }
/* The routine MDUPDATE updates the message digest context buffer to  */
/*  account for the presence of the character c in the message whose  */
/*  digest is being computed.  This routine will be called for each   */
/*   message byte in turn.                                            */
void MDUPDATE(md,c)
  struct MDCTX *md;
  unsigned char c;
  { register unsigned char i,j,t,*p;
    /**** Put i in a local register for efficiency ****/
       i = md->i;
    /**** Add new character to buffer ****/
       md->D[16+i] = c;
       md->D[32+i] = c ^ md->D[i];
    /**** Update checksum register C and value L ****/
       md->L = (md->C[i] ^= S[0xFF & (c ^ md->L)]);
    /**** Increment md->i by one modulo 16 ****/
       i = md->i = (i + 1) & 15;
    /**** Transform D if i=0 ****/
       if (i == 0)
         { t = 0;
           for (j=0;j<18;j++)
             {/*The following is a more efficient version of the loop:*/
               /*  for (i=0;i<48;i++) t = md->D[i] = md->D[i] ^ S[t]; */
               p = md->D;
               for (i=0;i<8;i++)
                 { t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                   t = (*p++ ^= S[t]);
                 }
               /* End of more efficient loop implementation */
               t = t + j;
             }
         }
  }
/* The routine MDFINAL terminates the message digest computation and  */
/* ends with the desired message digest being in md->D[0...15].       */
void MDFINAL(md)
  struct MDCTX *md;
  { int i,padlen;
    /* pad out to multiple of 16 */
       padlen  = 16 - (md->i);
       for (i=0;i<padlen;i++) MDUPDATE(md,(unsigned char)padlen);
    /* extend with checksum */
    /* Note that although md->C is modified by MDUPDATE, character    */
    /* md->C[i] is modified after it has been passed to MDUPDATE, so  */
    /* the net effect is the same as if md->C were not being modified.*/
    for (i=0;i<16;i++) MDUPDATE(md,md->C[i]);
  }

/**********************************************************************/
/* End of message digest implementation                               */
/**********************************************************************/
