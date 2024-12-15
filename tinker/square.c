/* 
 * reference implementation of SQUARE
 *
 * Algorithm developed by Joan Daemen <joan.daemen@club.innet.be> and
 * Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 *
 * Version 2.1.1 (1997.02.07)
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*       
 * uses 32 bit words
 */

#include <stdio.h>
#include <stdlib.h>

#define R 8                  /* number of rounds       */
#define ROOT 0x1f5           /* for generating GF(2^8) */
#define ROTL(x, s) (((x) << (s)) | ((x) >> (32 - (s))))   

typedef unsigned char word8;  
typedef unsigned short word16; 
typedef unsigned long word32; 

#include "boxes1.dat"        /* substitution boxes */

word8 log[256], alog[256];
word8 offset[R];
                   
void init() {
   /* produce logtabel and alogtabel,
    * needed for multiplying in the field GF(2^m)
    */
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

word8 mul(word8 a, word8 b) {
   /* multiply two elements of GF(2^m)
    */
   if (a && b) return alog[(log[a] + log[b])%255];
   else return 0;
   }

void transform(word32 in[4], word32 out[4]) {
   /* apply theta to a roundkey
    */
   word8 A[4][4], B[4][4], i, j, t;

   for(i = 0; i < 4; i++)
      for(j = 0; j < 4; j++) A[i][j] = in[i] >> (24 - 8*j);


   /* B = A * G */      
   for(i = 0; i < 4; i++)
      for(j = 0; j < 4; j++) {
         B[i][j] = 0;
         for(t = 0; t < 4; t++) B[i][j] ^= mul(A[i][t],G[t][j]);
         }

   for(i = 0; i < 4; i++) {
      out[i] = 0;
      for(j = 0; j < 4; j++) out[i] ^= (B[i][j] << (24 - 8*j));     
      }   
  }

void generate_roundkeys(word32 key[4], word32 roundkeys_e[R+1][4],
                                      word32 roundkeys_d[R+1][4]) {
   word8 t, u;
   word32 tempkeys[R+1][4];

   /* generate the offset values
    */
   offset[0] = 1;
   for(t = 1; t < R; t++) offset[t] = mul(2,offset[t-1]); 

   /* apply the key evolution function 
    */
   
   for(u = 0; u < 4; u++) tempkeys[0][u] = key[u];
   
   for (t = 1; t < R+1; t++) {
      tempkeys[t][0] =
         tempkeys[(t-1)][0] ^ ROTL(tempkeys[(t-1)][3],8)
         ^ (offset[t-1] << 24);
      tempkeys[t][1] =
         tempkeys[(t-1)][1] ^ tempkeys[t][0];
      tempkeys[t][2] =
         tempkeys[(t-1)][2] ^ tempkeys[t][1];
      tempkeys[t][3] =
         tempkeys[(t-1)][3] ^ tempkeys[t][2];
      }  

   /* produce the round keys
    */
   for(t = 0; t < R; t++) transform(tempkeys[t], roundkeys_e[t]);
   for(u = 0; u < 4; u++) roundkeys_e[R][u] = tempkeys[R][u];  
   for(t = 0; t < R; t++)
      for(u = 0; u < 4; u++) roundkeys_d[t][u] = tempkeys[R-t][u];  
   for(u = 0; u < 4; u++) roundkeys_d[R][u] = roundkeys_e[0][u];  
   
   }

void round(word32 text[4], word32 T[256], word32 roundkey[4]) {
   word8 t;
   word32 temp[4];
  
  for(t = 0; t < 4; t++)
      temp[t] = T[(word8)(text[0] >> (24 - 8*t))] 
                ^ ((T[(word8)(text[1] >> (24 - 8*t))] >> 8) 
                    + (T[(word8)(text[1] >> (24 - 8*t))] << 24))
                ^ ((T[(word8)(text[2] >> (24 - 8*t))] >> 16) 
                    + (T[(word8)(text[2] >> (24 - 8*t))] << 16))
                ^ ((T[(word8)(text[3] >> (24 - 8*t))] >> 24) 
                    + (T[(word8)(text[3] >> (24 - 8*t))] << 8));
   for(t = 0; t < 4; t++) text[t] = temp[t] ^ roundkey[t];
   }             

void cryption(word32 text[4], word8 S[256], word32 T[256], word32 roundkeys[R+1][4]) {
   word8 r, t;
   word32 temp[4];
   
   /* initial key addition
    */
   for(t = 0; t < 4; t++) text[t] ^= roundkeys[0][t];
 
   /* R - 1 full rounds 
    */
   for(r = 0; r < R-1; r++) round(text, T, roundkeys[r+1]);    

   /* last round (diffusion becomes only transposition)
    */
   for(t = 0; t < 4; t++)
      temp[t] = (S[(word8)(text[0] >> (24 - 8*t))] << 24) 
              + (S[(word8)(text[1] >> (24 - 8*t))] << 16)
              + (S[(word8)(text[2] >> (24 - 8*t))] <<  8)
              +  S[(word8)(text[3] >> (24 - 8*t))]       ;
              
   for(t = 0; t < 4; t++) text[t] = temp[t] ^ roundkeys[R][t];       
   }      

void main() {
   word8 t, bytetext[16], bytekey[16];
   word32 key[4], text[4];
   word32 roundkeys_e[R+1][4], roundkeys_d[R+1][4];

   init();

   for(t = 0; t < 16; t++) bytekey[t] = t;
   for(t = 0; t < 4; t++) 
      key[t] = (bytekey[t*4] << 24)
              + (bytekey[t*4+1] << 16)
              + (bytekey[t*4+2] <<  8)
              + bytekey[t*4+3]; 
   generate_roundkeys(key, roundkeys_e, roundkeys_d);
   for(t = 0; t < 16; t++) bytetext[t] = t;
   for(t = 0; t < 4; t++) 
      text[t] = (bytetext[t*4] << 24)
              + (bytetext[t*4+1] << 16)
              + (bytetext[t*4+2] <<  8)
              + bytetext[t*4+3]; 
   for(t = 0; t < 4; t++) printf("%08lx ",text[t]);
   printf(" in\n");
   cryption(text, Se, Te, roundkeys_e);
   for(t = 0; t < 4; t++) printf("%08lx ",text[t]);
   printf(" encrypt\n");
   cryption(text, Sd, Td, roundkeys_d);   
   for(t = 0; t < 4; t++) printf("%08lx ",text[t]);
   printf(" uit\n\n");
   }
