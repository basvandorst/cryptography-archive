

#include <stdio.h>
#include <stdlib.h>

#define ROOT 0x1f5U

#define ROTR(x, s) (((x) >> (s)) | ((x) << (32 - (s))))

typedef unsigned char  byte;
typedef unsigned short word16;
typedef unsigned long  word32;

static byte exptab[256], logtab[256];

static byte mul (byte a, byte b)
	/* multiply two elements of GF(2^m) */
{
	return (a && b) ? exptab[(logtab[a] + logtab[b])%255] : 0;
} /* mul */


static void init (void)
/* produce logtab and exptab,
 * needed for multiplying in the field GF(2^m)
 */
{
	word16 i, j;

	exptab[0] = 1;
	for (i = 1; i < 256; i++) { 
		j = exptab[i-1] << 1;
		if (j & 0x100U) {
			j ^= ROOT;
		}
		exptab[i] = (byte) j;
	}
	logtab[0] = 0;
	for (i = 1; i < 255; i++) {
		logtab[exptab[i]] = (byte) i;
	}
} /* init */


int main (void)
{
   FILE *out;
   byte ibox[256], g[9];
   byte in, u, t, pivot, tmp;
   byte box[256], G[4][4], iG[4][4], A[4][8];
   byte trans[9] = { 0xd6, 0x7b, 0x3d, 0x1f, 
                     0x0f, 0x05, 0x03, 0x01,
                     0xb1};
   word16 i, j, k;
   
word32 T[256];

   init();
   /* the substitution box based on F^{-1}(x)
    * + affine transform of the output
    */
   box[0] = 0;
   box[1] = 1;
   for(i = 2; i < 256; i++) 
      box[i] = exptab[255 - logtab[i]];
    
   for(i = 0; i < 256; i++) {
      in = box[i];
      box[i] = 0;
      for(t = 0; t < 8; t++) {
         u = in & trans[t];
         box[i] ^= ((1 & (u ^ (u >> 1) ^ (u >> 2) ^ (u >> 3) 
                  ^ (u >> 4) ^ (u >> 5) ^ (u >> 6) ^ (u >> 7)))
                   << (7 - t));
         }
      box[i] ^= trans[8];
      }

   /* diffusion box G
    * created by make_g.c
    */
   g[3] = 0x01U;
   g[2] = 0xf5U;
   g[1] = 0x26U;
   g[0] = 0x31U;
    
   for(i = 0; i < 4; i++) 
      for(j = 0; j < 4; j++) 
         G[i][j] = g[(4 + j - i) % 4];
   
   for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) A[i][j] = G[i][j];
      for(j = 4; j < 8; j++) A[i][j] = 0;
      A[i][i+4] = 1;
      }
   for(i = 0; i < 4; i++) {
      pivot = A[i][i];
      if (pivot == 0) {
         t = i + 1;
 
        while ((A[t][i] == 0) && (t < 4)) t++;
         if (t == 4) fprintf(stderr,"noninvertible matrix G\n");
         else {
            for(j = 0; j < 8; j++) {
               tmp = A[i][j];
               A[i][j] = A[t][j];
               A[t][j] = tmp;
               }
            pivot = A[i][i];
            }
         }
      for(j = 0; j < 8; j++) 
         if (A[i][j])
            A[i][j] = exptab[(255 + logtab[A[i][j]] - logtab[pivot])%255];
      for(t = 0; t < 4; t++)
         if (i != t)
            {
            for(j = i+1; j < 8; j++)
               A[t][j] ^= mul(A[i][j],A[t][i]);
            A[t][i] = 0;
            }
      }
   for(i = 0; i < 4; i++)
      for(j = 0; j < 4; j++) iG[i][j] = A[i][j+4];
   
   
   /* output
    */
   out = fopen ("square.tab", "w");
   for(i = 0; i < 256; i++) ibox[box[i]] = (byte)i;
   
   /*
   fprintf(out,"byte Se[256] = {\n");
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) fprintf(out,"%3d, ",box[i*16+j]);
      fprintf(out,"\n");
      }
   fprintf(out,"};\n\n");
   fprintf(out,"byte Sd[256] = {\n");
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) fprintf(out,"%3d, ",ibox[i*16+j]);
      fprintf(out,"\n");
      }
   fprintf(out,"};\n\n");
   */

	fprintf(out,"static byte G[4][4] = {\n");
	for(i = 0; i < 4; i++) {
		fprintf (out,"\t0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU,\n",
			G[i][0], G[i][1], G[i][2], G[i][3]);
	}
	fprintf(out,"};\n\n");

	fprintf(out,"static byte iG[4][4] = {\n");
	for(i = 0; i < 4; i++) {
		fprintf (out,"\t0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU,\n",
			iG[i][0], iG[i][1], iG[i][2], iG[i][3]);
	}
	fprintf(out,"};\n\n");

	fprintf(out,"static word32 Te0[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = (box[k]) ?
				((word32) mul (box[4*t+k], G[0][0]) << 24) |
				((word32) mul (box[4*t+k], G[0][1]) << 16) |
				((word32) mul (box[4*t+k], G[0][2]) <<  8) |
				((word32) mul (box[4*t+k], G[0][3])) : 0UL;
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static word32 Te1[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = ROTR (T[4*t+k], 8);
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static word32 Te2[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = ROTR (T[4*t+k], 8);
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static word32 Te3[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = ROTR (T[4*t+k], 8);
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static word32 Td0[256] = {\n");
	for(t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for(k = 0; k < 4; k++) {
			T[4*t+k] = ibox[k] ?
				((word32) mul (ibox[4*t+k], iG[0][0]) << 24) |
				((word32) mul (ibox[4*t+k], iG[0][1]) << 16) |
				((word32) mul (ibox[4*t+k], iG[0][2]) <<  8) |
				((word32) mul (ibox[4*t+k], iG[0][3])) : 0UL;
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
			/*if (ibox[k]) {
				fprintf (out, "0x%02x%02x%02x%02xUL, ",
					mul (ibox[4*t+k], iG[0][0]),
					mul (ibox[4*t+k], iG[0][1]),
					mul (ibox[4*t+k], iG[0][2]),
					mul (ibox[4*t+k], iG[0][3]));
			} else {
				fprintf(out,"0x00000000UL, ");
			}*/
		}
		fprintf(out,"\n");   
	}
	fprintf(out,"};\n\n");

	fprintf(out,"static word32 Td1[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = ROTR (T[4*t+k], 8);
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static word32 Td2[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = ROTR (T[4*t+k], 8);
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static word32 Td3[256] = {\n");
	for (t = 0; t < 64; t++) {
		fprintf(out,"\t");   
		for (k = 0; k < 4; k++) {
			T[4*t+k] = ROTR (T[4*t+k], 8);
			fprintf (out, "0x%08lxUL, ", T[4*t+k]);
		}
		fprintf (out,"\n");   
	}
	fprintf (out,"};\n\n");
   
	fprintf(out,"static byte logtab[256] = {\n");
	for(i = 0; i < 256; i += 8) {
		fprintf(out,"\t0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU,\n",
			logtab[i], logtab[i+1], logtab[i+2], logtab[i+3], logtab[i+4], logtab[i+5], logtab[i+6], logtab[i+7]);
	}
	fprintf(out,"};\n\n");
/*
	fprintf(out,"static byte exptab[256] = {\n");
	for(i = 0; i < 256; i += 8) {
		fprintf(out,"\t0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU,\n",
			exptab[i], exptab[i+1], exptab[i+2], exptab[i+3], exptab[i+4], exptab[i+5], exptab[i+6], exptab[i+7]);
	}
	fprintf(out,"};\n\n");
*/
	fprintf(out,"static byte exptab[512] = {\n");
	for(i = 0; i < 256; i += 8) {
		fprintf(out,"\t0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU,\n",
			exptab[i  ], exptab[i+1], exptab[i+2], exptab[i+3], exptab[i+4], exptab[i+5], exptab[i+6], exptab[i+7]);
	}
	for(i = 0; i < 256; i += 8) {
		fprintf(out,"\t0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU, 0x%02xU,\n",
			exptab[i+1], exptab[i+2], exptab[i+3], exptab[i+4], exptab[i+5], exptab[i+6], exptab[i+7], exptab[i+8]);
	}
	fprintf(out,"};\n\n");
	fclose(out);
	return 0;
}

