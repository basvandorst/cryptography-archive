#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "des.h"
#include "misc.h"

#define VERBOSE

void GetUserInfo(unsigned char plaintextVector[32], 
        unsigned char plaintextXorMask[8], 
        unsigned char ciphertext0[8], unsigned char ciphertext[8],
        unsigned char *plaintextByteMask, int *useCBC, int *extraXor,
        int *quickStart, unsigned char startKey[7], long *numClocks);
void increment32(unsigned char *v);
void decrement32(unsigned char *v);
void desDecrypt(unsigned char m[8], unsigned char c[8], unsigned char k[7]);
void printHexString(char *tag, unsigned char *data, int len);
static void EXIT_ERR(char *s) { fprintf(stderr, s); exit(1); }
int hex2bin(char *hex, unsigned char *bin);


void GetUserInfo(unsigned char plaintextVector[32], 
        unsigned char plaintextXorMask[8], 
        unsigned char ciphertext0[8], unsigned char ciphertext1[8],
        unsigned char *plaintextByteMask, int *useCBC, int *extraXor,
        int *quickStart, unsigned char startKey[7], long *numClocks) {
  char buffer[1024];
  unsigned char tmp[512];
  int i;

#ifdef VERBOSE
  printf("Enter plaintextVector values: ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i <= 0 || i >= 256)
    EXIT_ERR("Must have at least 1 plaintextVector entry and at most 255.\n");
  memset(plaintextVector, 0, 32);
  while (i--)
    plaintextVector[tmp[i]/8] |= (128 >> (tmp[i] % 8));

#ifdef VERBOSE
  printf("    Enter plaintext xor mask: ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 8)
    EXIT_ERR("Must have 8 plaintext xor mask bytes.");
  memcpy(plaintextXorMask, tmp, 8);

#ifdef VERBOSE
  printf("          Enter ciphertext 0: ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 8)
    EXIT_ERR("Must have 8 bytes in ciphertext 0.");
  memcpy(ciphertext0, tmp, 8);

#ifdef VERBOSE
  printf("          Enter ciphertext 1: ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 8)
    EXIT_ERR("Must have 8 bytes in ciphertext 1.");
  memcpy(ciphertext1, tmp, 8);

#ifdef VERBOSE
  printf("   Enter plaintext byte mask: ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1)
    EXIT_ERR("Plaintext byte mask is 1 byte long.");
  *plaintextByteMask = tmp[0];

#ifdef VERBOSE
  printf("       Enter useCBC (0 or 1): ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1 || tmp[0] > 1)
    EXIT_ERR("Must enter 0 or 1 for useCBC.");
  *useCBC = tmp[0];

#ifdef VERBOSE
  printf("     Enter extraXor (0 or 1): ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1 || tmp[0] > 1)
    EXIT_ERR("Must enter 0 or 1 for extraXor.");
  *extraXor = tmp[0];

#ifdef VERBOSE
  printf("   Enter quickStart (0 or 1): ");
#endif
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1 || tmp[0] > 1)
    EXIT_ERR("Must enter 0 or 1 for quickStart\n");
  *quickStart = tmp[0];


#ifdef VERBOSE
  printf("          Enter starting key: ");
#endif
  gets(buffer);
  if (hex2bin(buffer, tmp) != 7)
    EXIT_ERR("Must enter 7 hex bytes as the key.\n");
  memcpy(startKey, tmp, 7);

#ifdef VERBOSE
  printf("      Enter number of clocks: ");
#endif
  gets(buffer);
  sscanf(buffer, "%ld", numClocks);
  if (*numClocks < 1 || *numClocks > 1000000000L)
    EXIT_ERR("Must have between 1 and 1 billion clocks.\n");

#ifdef VERBOSE
  printHexString("\n  PtxtVector = ", plaintextVector, 32);
  printHexString(" PtxtXorMask = ", plaintextXorMask, 8);
  printHexString("Ciphertext 0 = ", ciphertext0, 8);
  printHexString("Ciphertext 1 = ", ciphertext1, 8);
  printHexString("PtxtByteMask = ", plaintextByteMask, 1);
  printf(        "      useCBC = %d\n", *useCBC);
  printf(        "    extraXor = %d\n", *extraXor);
  printf(        "  quickStart = %d\n", *quickStart);
  printHexString("Starting key = ", startKey, 7);
  printf(        "Total clocks = %ld\n\n", *numClocks);
#endif
}


void increment32(unsigned char *v) {
  if ((++(v[3])) == 0)
    if ((++(v[2])) == 0)
      if ((++(v[1])) == 0)
        ++v[0];
}

void decrement32(unsigned char *v) {
  if (((v[3])--) == 0)
    if (((v[2])--) == 0)
      if (((v[1])--) == 0)
        v[0]--;
}

void desDecrypt(unsigned char m[8], unsigned char c[8], unsigned char k[7]) {
  bool key[56], message[64];
  int i;

//  printf("DES_DECRYPT(k="); for (i=0; i<7;i++) printf("%02X",k[i]);  //!!!
//  printf(", c="); for (i=0; i<8;i++) printf("%02X",c[i]);            //!!!

  for (i = 0; i < 56; i++)
    key[55-i] = ((k[i/8] << (i & 7)) & 128) ? 1 : 0;
  for (i = 0; i < 64; i++)
    message[63-i] = ((c[i/8] << (i & 7)) & 128) ? 1 : 0;
  DecryptDES(key, message, message, 0);
  for (i = 0; i < 8; i++)
    m[i] = 0;
  for (i = 0; i < 64; i++)
    if (message[63-i])
      m[i/8] |= 128 >> (i%8);

//  printf(")="); for (i=0; i<8;i++) printf("%02X",m[i]); printf("\n"); //!!!

}


int unhex(char c) {
  if (c >= '0' && c <= '9')
    return (c - '0');
  if (c >= 'a' && c <= 'f')
    return (c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return (c - 'A' + 10);
  return (-1);
}


int hex2bin(char *hex, unsigned char *bin) {
  int i = 0;
  int j = 0;

  /* Trim string if comments present */
  if (strchr(hex, '#') != NULL)
    *strchr(hex, '#') = 0;
  if (strchr(hex, '*') != NULL)
    *strchr(hex, '*') = 0;
  if (strchr(hex, '\'') != NULL)
    *strchr(hex, '\'') = 0;

  for (i = 0; i < strlen(hex); i++) {
    if (hex[i] >= '0' && unhex(hex[i]) < 0)
      EXIT_ERR("Bad hex digit encountered.\n");
  }

  for (i = 0; i < strlen(hex); i++) {
    if (hex[i] < '0')
      continue;
    if (hex[i] >= '0' && hex[i+1] >= '0') {
      bin[j++] = unhex(hex[i])*16+unhex(hex[i+1]);
      i++;    // skip one
      continue;
    }
    if (hex[i] >= '0') {
      bin[j++] = unhex(hex[i]);
    }
  }
  return (j);
}


void printHexString(char *tag, unsigned char *data, int len) {
  int i;

  printf("%s", tag);
  for (i = 0; i < len; i++)
    printf("%02X", data[i]);
  printf("\n");
}

