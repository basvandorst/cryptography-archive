/*
 * This program makes an .asm text file of all the lookup tables
 * needed by DES. Several tables are manipulated to give better speed.
 * Output is to stdout. Use with redirection (maketabl >table.asm).
 */

#include <stdio.h>

#define ps(n)       ((unsigned char)(0x80 >> (n-1)))
#define b(n,r)      ((n>r||n<r-7)?0:ps(n-(r-8)))
#define p(n)        b(n, 8),b(n,16),b(n,24),b(n,32),\
                    b(n,40),b(n,48),b(n,56),b(n,64)
#define q(n) p((n)+4)

void make_table(char*,int);
void nybble(char *table);
long swapbyte(long input);

/*
 * The (in)famous S-boxes 
 */
static char si[8][64] = {
        /* S1 */
        14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
         0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
         4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
        15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

        /* S2 */
        15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
         3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
         0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
        13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

        /* S3 */
        10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
        13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
        13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
         1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

        /* S4 */
         7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
        13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
        10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
         3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

        /* S5 */
         2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
        14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
         4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
        11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

        /* S6 */
        12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
        10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
         9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
         4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

        /* S7 */
         4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
        13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
         1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
         6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

        /* S8 */
        13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
         1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
         7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
         2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

/*
 * 32-bit permutation function P used on the output of the S-boxes 
 */
static char p32i[] = {  
        16,  7, 20, 21,
        29, 12, 28, 17,
         1, 15, 23, 26,
         5, 18, 31, 10,
         2,  8, 24, 14,
        32, 27,  3,  9,
        19, 13, 30,  6,
        22, 11,  4, 25
};

/*
 * initial permutation IP 
 */
static char ip[] = {
        58, 50, 42, 34, 26, 18, 10,  2,
        60, 52, 44, 36, 28, 20, 12,  4,
        62, 54, 46, 38, 30, 22, 14,  6,
        64, 56, 48, 40, 32, 24, 16,  8,
        57, 49, 41, 33, 25, 17,  9,  1,
        59, 51, 43, 35, 27, 19, 11,  3,
        61, 53, 45, 37, 29, 21, 13,  5,
        63, 55, 47, 39, 31, 23, 15,  7
};

/*
 * final permutation IP^-1
 */
static char fp[] = {
        40,  8, 48, 16, 56, 24, 64, 32,
        39,  7, 47, 15, 55, 23, 63, 31,
        38,  6, 46, 14, 54, 22, 62, 30,
        37,  5, 45, 13, 53, 21, 61, 29,
        36,  4, 44, 12, 52, 20, 60, 28,
        35,  3, 43, 11, 51, 19, 59, 27,
        34,  2, 42, 10, 50, 18, 58, 26,
        33,  1, 41,  9, 49, 17, 57, 25
};


/*
 * Permuted Choice 1 for key scheduling. Note that the output is two blocks
 * of 28 bits, each left-justified in a 32-bit word.
 */
int PC1tbl[] = {
    57,49,41,33,25,17, 9,
     1,58,50,42,34,26,18,
    10, 2,59,51,43,35,27,
    19,11, 3,60,52,44,36,
    0,0,0,0,

    63,55,47,39,31,23,15,
     7,62,54,46,38,30,22,
    14, 6,61,53,45,37,29,
    21,13, 5,28,20,12, 4,
    0,0,0,0
};
/* 
 * Permuted Choice 2 for key scheduling.
 */
int PC2tbl[] = {
    14,17,11,24, 1, 5, 3,28,
    15, 6,21,10,23,19,12, 4,
    26, 8,16, 7,27,20,13, 2,

    41,52,31,37,47,55,30,40,
    51,45,33,48,44,49,39,56,
    34,53,46,42,50,36,29,32
};


int main()
 {
   char pbox[32];
   int p,i,s,j,rowcol;
   long val;

   printf("public sp_table\n");
   printf("public p1\n");
   printf("public p2\n");
   printf("public Pmask\n");
   printf("public PC1tbl\n");
   printf("public PC2tbl\n");
   printf("ifdef TRIPLE_DES\n");
   printf("public _en_keytbl\n");
   printf("public _de_keytbl\n");
   printf("else\n");
   printf("public _keytbl\n");
   printf("endif\n");

   printf("\nd32seg segment para public 'DATA' ;align for cache\n\n");

   /*
    * Make Initial Permutation and Final Permutation tables.
    * Use inverse-tables as input: inverse tables are faster at runtime.
    * (fp[] is used to make p1; ip[] is used to make p2)
    */
   printf("p1 label dword\n");
   make_table(fp,64);
   printf("\n\np2 label dword\n");
   make_table(ip,64);

   /*
    * Make permutation setting-bits.
    */
   printf("\n\nPmask label dword\n");
   for(i=1;i<=64;++i)
    {
      printf("   db %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh\n",p(i));
     }

   /* 
    * Make Permuted Choice tables for key scheduling
    */
   printf("\n\nPC1tbl label dword\n");
   for(i=0;i<64;++i)
    printf("   db %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh\n",p(PC1tbl[i]));
   printf("\n\nPC2tbl label dword\n");
   for(i=0;i<24;++i)
    printf("   db %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh\n",p(PC2tbl[i]));
   for(i=0;i<24;++i)
    printf("   db %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh, %02Xh\n",q(PC2tbl[24+i]));

   /*
    * Make sptbl, combining the S- and P- boxes.
    * First make an inverse-permuted P-box, then look up its value in the
    * right S-box. Then swap the bytes of the output, so it doesn't have
    * to be done at runtime.
    */

   for(p=0;p<32;++p)            /* first make inverse-perm P-box */
      for(i=0;i<32;++i)
         if(p32i[i]-1 == p)
          {
            pbox[p]=i;
            break;
           }
   printf("\n\nsp_table label dword\n");
   for(s=0;s<8;++s)
    {
      printf(" ;   sp[%d]\n",s);
      for(i=0;i<64;++i)         /* next look up S-box value */
       {
         val=0;
         rowcol=(i&32)|((i&1)?16:0)|((i>>1)&0xf);
         for(j=0;j<4;++j)
          {
            if(si[s][rowcol]&(8>>j))
             val |= 1L <<(31-pbox[4*s+j]);
           }
         printf("   dd %08lXh\n",swapbyte(val));
        }
     }
   printf("\nifdef TRIPLE_DES\n");
   printf("_en_keytbl label byte       ;encryption key space\n");
   printf("   db 16*8 dup (?)\n");

   printf("_de_keytbl label byte       ;decryption key space\n");
   printf("   db 16*8 dup (?)\n");
   printf("\nelse\n");
   printf("\n_keytbl label byte\n");
   printf("   db 16*8 dup (?)\n");
   printf("\nendif\n");

   printf("\nd32seg ends\n\n");
   printf("DGROUP group d32seg\n");
   printf("end\n");
   return 0;
  }

void make_table(char*table,int tablesize)
 {
   int offset;
   for(offset=0; offset<tablesize; offset+=8)
    {
      nybble(table+offset+4);   /* do low bits */
      nybble(table+offset);     /* do high bits */
      printf("\n");
     }
  }

void nybble(char *table)
 {
   int i,j;
   char val;
   long high,low;

   for(i=0;i<16;++i)
    {
      low=0;high=0;
      for(j=0;j<4; ++j)
       if((1<<j)&i)
        {
          val=table[3-j];
          if(val>32)
           low |= swapbyte(1L<<(64-val));
          else
           high |= swapbyte(1L<<(32-val));
         }
      printf("   dd %08lXh, %08lXh\n",high,low);
     }
  }

/* This function reverses the bytes in a long int. By doing this
   here we eliminate the need to BSWAP at runtime.
*/
long swapbyte(long input)
 {
   unsigned char *cp,c;
   int i;
   (long*)cp= &input;
   c=*(cp+3);
   *(cp+3)=*cp;
   *cp=c;

   c=*(cp+2);
   *(cp+2)=*(cp+1);
   *(cp+1)=c;
   return(input);
  }
