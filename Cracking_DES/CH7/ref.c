#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "des.h"
#include "misc.h"

#define CLOCKS_PER_DES 18

int plaintextMatch(unsigned char plaintextVector[32], unsigned char m[8],
        unsigned char plaintextByteMask, int ciphertext, unsigned char key[7]);
void checkKey(unsigned char key[7], unsigned char plaintextVector[32], 
        unsigned char plaintextXorMask[8], 
        unsigned char ciphertext0[8], unsigned char ciphertext1[8],
        unsigned char plaintextByteMask, int useCBC, int extraXor);

void main(void) {
  unsigned char startKey[7], plaintextVector[32];
  unsigned char plaintextXorMask[8];
  unsigned char ciphertext0[8];
  unsigned char ciphertext1[8];
  unsigned char plaintextByteMask;
  int useCBC, extraXor, quickStart;
  int i,j;
  long numClocks;
  unsigned char key[7];

  GetUserInfo(plaintextVector, plaintextXorMask, ciphertext0, ciphertext1, 
          &plaintextByteMask, &useCBC, &extraXor, &quickStart, startKey,
          &numClocks);

  for (i = 0; i < numClocks; i += CLOCKS_PER_DES) {
    for (j = 0; j < 24; j++) {
      memcpy(key, startKey, 8);
      key[0] += j;
      checkKey(key, plaintextVector, plaintextXorMask, ciphertext0, 
          ciphertext1, plaintextByteMask, useCBC, extraXor);
    }
    increment32(startKey+3);
  }
}


void checkKey(unsigned char key[7], unsigned char plaintextVector[32], 
        unsigned char plaintextXorMask[8], 
        unsigned char ciphertext0[8], unsigned char ciphertext1[8],
        unsigned char plaintextByteMask, int useCBC, int extraXor) {
  unsigned char m[8];
  int i;

  desDecrypt(m, ciphertext0, key);
  printf("DES_decrypt(K="); for (i = 0; i < 7; i++) printf("%02X", key[i]); 
  printf(", C0="); for (i = 0; i < 8; i++) printf("%02X", ciphertext0[i]);
  printf(") -> "); for (i = 0; i < 8; i++) printf("%02X", m[i]); printf("\n");
  if (extraXor) {
    m[0] ^= m[4];
    m[1] ^= m[5];
    m[2] ^= m[6];
    m[3] ^= m[7];
  }
  for (i = 0; i < 8; i++)
    m[i] ^= plaintextXorMask[i];

  if (plaintextMatch(plaintextVector, m, plaintextByteMask, 0, key)) {
    desDecrypt(m, ciphertext1, key);
    printf("DES_decrypt(K="); for (i = 0; i < 7; i++) printf("%02X", key[i]); 
    printf(", C1="); for (i = 0; i < 8; i++) printf("%02X", ciphertext1[i]);
    printf(") -> "); for (i = 0; i < 8; i++) printf("%02X", m[i]); printf("\n");
    if (extraXor) {
      m[0] ^= m[4];
      m[1] ^= m[5];
      m[2] ^= m[6];
      m[3] ^= m[7];
    }
    if (useCBC) {
      for (i = 0; i < 8; i++)
        m[i] ^= ciphertext0[i];
    }
    if (plaintextMatch(plaintextVector, m, plaintextByteMask, 1, key)) {
      printf("------- VALID MATCH--------\n");
      fprintf(stderr, "Match found at key =");
      for (i = 0; i < 7; i++)
        fprintf(stderr, "%02X", key[i]);
      fprintf(stderr, "\n");
    }
  }
}


int plaintextMatch(unsigned char plaintextVector[32], unsigned char m[8],
       unsigned char plaintextByteMask, int ciphertext, unsigned char key[7]) {
  int i;

  for (i = 0; i < 8; i++) {
    if ((plaintextByteMask & (128>>i)) == 1)
      continue;   /* this byte is skipped */
    if (plaintextVector[m[i]/8] & (128 >> (m[i]%8)))
      continue;
    return (0);           /* no match */
  }

  printf("Match of C%d with key ", ciphertext);
  for (i = 0; i < 7; i++)
    printf("%02X", key[i]);
  printf(" = ");
  for (i = 0; i < 8; i++)
    printf("%02X", m[i]);
  printf("\n");

  fprintf(stderr, "Match of C%d with key ", ciphertext);
  for (i = 0; i < 7; i++)
    fprintf(stderr, "%02X", key[i]);
  fprintf(stderr, " = ");
  for (i = 0; i < 8; i++)
    fprintf(stderr, "%02X", m[i]);
  fprintf(stderr, "\n");

  return (1);
}


