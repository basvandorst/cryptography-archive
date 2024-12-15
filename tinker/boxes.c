#include <stdio.h>
#include <stdlib.h>


#define ROOT 0x1f5

typedef unsigned char word8;
typedef unsigned short word16;
typedef unsigned long word32;

word8 alog[256], log[256];

word8 mul(word8 a, word8 b)
/* multiply two elements of GF(2^m)
 */
{
   if (a && b) return alog[(log[a] + log[b])%255];
   else return 0;
}

void init()
/* produce logtabel and alogtabel,
 * needed for multiplying in the field GF(2^m)
 */
{
   word16 i, j;
   alog[0] = 1;
   for(i = 1; i < 256; i++) { 
      j = alog[i-1] << 1;
      if (j & 0x100) j ^= ROOT;
      alog[i] = j;
      }
   log[0] = 0;
   for(i = 1; i < 255; i++)
      log[alog[i]] = i;
}


void main()
{
   FILE *out;
   word8 ibox[256], g[9];
   word8 in, u, t, pivot, tmp;
   word8 box[256], G[4][4], iG[4][4], A[4][8];
   word8 trans[9] = { 0xd6, 0x7b, 0x3d, 0x1f, 
                     0x0f, 0x05, 0x03, 0x01,
                     0xb1};
   word16 i, j, k;

   init();
   /* the substitution box based on F^{-1}(x)
    * + affine transform of the output
    */
   box[0] = 0;
   box[1] = 1;
   for(i = 2; i < 256; i++) 
      box[i] = alog[255 - log[i]];
    
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
   g[3] = 3;
   g[2] = 1;
   g[1] = 1;
   g[0] = 2;
    
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
            A[i][j] = alog[(255 + log[A[i][j]] - log[pivot])%255];
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
   out = fopen("boxes1.dat","w");
   for(i = 0; i < 256; i++) ibox[box[i]] = i;
   
   fprintf(out,"word8 Se[256] = {\n");
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) fprintf(out,"%3d, ",box[i*16+j]);
      fprintf(out,"\n");
      }
   fprintf(out,"};\n\n");
   fprintf(out,"word8 Sd[256] = {\n");
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) fprintf(out,"%3d, ",ibox[i*16+j]);
      fprintf(out,"\n");
      }
   fprintf(out,"};\n\n");
   fprintf(out,"word8 G[4][4] = {\n");
   for(i = 0; i < 4; i++) {
      for(k = 0; k < 4; k++) fprintf(out,"0x%02x, ",G[i][k]);
      fprintf(out,"\n");
      }
   fprintf(out,"};\n\n");
   fprintf(out,"word8 iG[4][4] = {\n");
   for(i = 0; i < 4; i++) {
      for(k = 0; k < 4; k++) fprintf(out,"0x%02x, ",iG[i][k]);
      fprintf(out,"\n");
      }
   fprintf(out,"};\n\n");

   fprintf(out,"word32 Te[256] = {\n");
   for(t = 0; t < 64; t++) {
      for(k = 0; k < 4; k++) {
         fprintf(out,"0x");
         if (box[k]) 
            for(i = 0; i < 4; i++) 
               fprintf(out,"%02x", mul(box[4*t+k],G[0][i]));
         else fprintf(out,"00000000");
         fprintf(out,", ");
         }
      fprintf(out,"\n");   
      }
   fprintf(out,"};\n\n");
   fprintf(out,"word32 Td[256] = {\n");
   for(t = 0; t < 64; t++) {
      for(k = 0; k < 4; k++) {
         fprintf(out,"0x");
         if (ibox[k]) 
            for(i = 0; i < 4; i++) 
               fprintf(out,"%02x", mul(ibox[4*t+k],iG[0][i]));
         else fprintf(out,"00000000");
         fprintf(out,", ");
         }
      fprintf(out,"\n");   
      }
   fprintf(out,"};\n\n");
   fclose(out);
   }
