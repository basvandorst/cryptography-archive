/*****************************************************************************
 * testvec.c                                                                 *
 *             DES ASIC Simlator, Test Vector Generation Program             *
 *                                                                           *
 *    Written 1998 by Cryptography Research (http://www.cryptography.com)    *
 *       and Paul Kocher for the Electronic Frontier Foundation (EFF).       *
 *       Placed in the public domain by Cryptography Research and EFF.       *
 *  THIS IS UNSUPPORTED FREE SOFTWARE. USE AND DISTRIBUTE AT YOUR OWN RISK.  *
 *                                                                           *
 *  IMPORTANT: U.S. LAW MAY REGULATE THE USE AND/OR EXPORT OF THIS PROGRAM.  *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *   IMPLEMENTATION NOTES:                                                   *
 *                                                                           *
 *   This program automatically determines the configuration of a search     *
 *   array.  Additional diagnostic code should be added to detect common     *
 *   chip failures (once these are known).                                   *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *   REVISION HISTORY:                                                       *
 *                                                                           *
 *   Version 1.0:  Initial release by Cryptography Research to EFF.          *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include "sim.h"

int USE_RAW_IO = 0;
FILE *FILE_TOCHIP, *FILE_FROMCHIP;        /* TOCHIP can be input *or* output */
int  CREATING_VECTOR;                      /* reading vs writing TOCHIP file */
unsigned char HARDWIRED_CHIP_ID  = 0x3A;

int ALLACTIVE_IN = 1;                               /* gets toggled randomly */
int BOARD_EN_IN  = 1;                       /* input value for run_set/check */
int ADRSEL1_IN   = 1;


void GetUserInfo(unsigned char plaintextVector[32],
        unsigned char plaintextXorMask[8],
        unsigned char ciphertext0[8], unsigned char ciphertext[8],
        unsigned char *plaintextByteMask, int *useCBC, int *extraXor,
        int *randomVector, unsigned char startKey[7], long *totalClocks);
void LoadState(unsigned char plaintextVector[32],
        unsigned char plaintextXorMask[8],
        unsigned char ciphertext0[8], unsigned char ciphertext1[8],
        unsigned char plaintextByteMask, int useCBC, int extraXor,
        unsigned char startKey[7]);
void RunSimulator_SetRegister(int addr, int data);
unsigned char RunSimulator_CheckRegister(int addr);
void RunSimulator_DummyIO(void);
static void EXIT_ERR(char *s) { fprintf(stderr, s); exit(1); }

void desDecrypt(unsigned char m[8], unsigned char c[8], unsigned char k[7]);
void increment32(unsigned char *num);
void decrement32(unsigned char *num);
int hex2bin(char *hex, unsigned char *bin);
void printHexString(char *tag, unsigned char *data, int len);
void OpenFiles(char *toChipFilename, char *fromChipFilename, int useRaw);
void printKeyInfo(FILE *outDev, char *preamble, int searchUnit);
long getClockCounter(void);
void proceedNormal(long totalClocks);
void proceedRandom(void);


/*
 *
 *
 *   THESE FUNCTIONS CREATE AND MANAGE THE TEST VECTORS.
 *
 *
 *
 */


void main(int argc, char **argv) {
  unsigned char startKey[7], plaintextVector[32];
  unsigned char plaintextXorMask[8];
  unsigned char ciphertext0[8];
  unsigned char ciphertext1[8];
  unsigned char plaintextByteMask;
  int useCBC, extraXor, randomVector;
  long totalClocks;
  char buffer[512];

  if (argc != 3 && argc != 4) {
    fprintf(stderr,"Command line: TO_CHIP.OUT FROM_CHIP.OUT [RAW]\n");
    fprintf(stderr,"  TO_CHIP.OUT      File for data going to chip\n");
    fprintf(stderr,"         (If this file exists, it will be simulated.\n");
    fprintf(stderr,"         Otherwise, a new file will be created.)\n");
    fprintf(stderr,"  FROM_CHIP.OUT    File for chip's output\n");
    fprintf(stderr,"  RAW              Gives unix CRLFs & no header.\n");
    exit(1);
  }

  /*
   * Open files and set CREATING_VECTOR to:
   *     0=reading TOCHIP file,
   *     1=create TOCHIP from user input,
   *     2=create random vector
   */
  OpenFiles(argv[1], argv[2], (argc == 4) ? 1 : 0);

  if (CREATING_VECTOR == 0) {
    fprintf(stderr, "Using input vector from file.\n");
    while (1) {
      if (fgets(buffer, 500, FILE_TOCHIP) == NULL)
        break;
      if (strlen(buffer) < 10)
        break;
      RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    }
  } else {
    GetUserInfo(plaintextVector, plaintextXorMask, ciphertext0, ciphertext1,
            &plaintextByteMask, &useCBC, &extraXor, &randomVector, startKey,
            &totalClocks);
    if (randomVector == 0) {
      fprintf(stderr, "Seed=random (time-based)\n");
      srand((unsigned) time(NULL));
      HARDWIRED_CHIP_ID = (unsigned char)(rand() & 255);
    } else if (randomVector == 1) {
      fprintf(stderr, "Using user params.\n");
    } else {
      fprintf(stderr, "Seed=%d\n", randomVector);
      srand(randomVector);
      HARDWIRED_CHIP_ID = (unsigned char)(rand() & 255);
    }

    /* Reset chip and set the chip ID */
    sprintf(buffer, "01011111 00 %02X 00\n", HARDWIRED_CHIP_ID);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO); fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO); fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO); fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO); fputs(buffer, FILE_TOCHIP);
    sprintf(buffer, "11011111 %02X %02X 00\n", HARDWIRED_CHIP_ID,
            HARDWIRED_CHIP_ID);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    buffer[2] = '1';
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    buffer[2] = '0';
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);

    if (randomVector == 1) {
      LoadState(plaintextVector, plaintextXorMask, ciphertext0, ciphertext1,
              plaintextByteMask, useCBC, extraXor, startKey);
      proceedNormal(totalClocks);
    } else {
      proceedRandom();
    }
  }

  /* Clean up a bit (doesn't really matter -- this is test code :-) */
  fclose(FILE_FROMCHIP);
  fclose(FILE_TOCHIP);
}


void proceedNormal(long totalClocks) {
  long numClocks = getClockCounter();
  unsigned char goodKey[8];
  int i,j,r;

  while (++numClocks < totalClocks) {
    r = RunSimulator_CheckRegister(REG_SEARCHINFO);
    if (r & 4) {
      fprintf(stderr, "------- Idle --------\n");
      RunSimulator_DummyIO();
      continue;
    }
    for (i = 0; i < 24; i++) {
      /* If we're going to see a stall, give some settling time */
      if ((peekState(REG_SEARCH_STATUS(i)) & 1) == 0) {          /* stalled? */
        RunSimulator_DummyIO();                          /* wait before read */
        RunSimulator_DummyIO();
        RunSimulator_DummyIO();
      }
      r = RunSimulator_CheckRegister(REG_SEARCH_STATUS(i));
      if ((r & 1) == 0) {                                        /* stalled? */
        goodKey[6] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+0);
        goodKey[5] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+1);
        goodKey[4] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+2);
        goodKey[3] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+3);
        goodKey[2] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+4);
        goodKey[1] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+5);
        goodKey[0] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+6);

        fprintf(stderr, "ALERT: Full match in unit %d; extracted k = ", i);
        printf("ALERT: Full match in unit %d; extracted k = ", i);
        for (j = 0; j < 7; j++) {
          fprintf(stderr, "%02X", goodKey[j]);
          printf("%02X", goodKey[j]);
        }
        fprintf(stderr, "\n");
        printf("\n");
        RunSimulator_DummyIO();                             /* Settling time */
        RunSimulator_DummyIO();
        RunSimulator_SetRegister(REG_SEARCH_STATUS(i), 1);        /* restart */
      }
    }
  }
}


void proceedRandom(void) {
  unsigned char readout[256];
  unsigned char goodKey[7];
  int i,j;
  unsigned char plaintextVector[32];
  char buffer[256];

  /* chip has already been set and the chip ID has been loaded */

  /* Create plaintext vector with 181 bits set */
  memset(plaintextVector, 0, sizeof(plaintextVector));
  i = 0;
  while (i < 181) {
    j = rand() & 255;
    if ((plaintextVector[j/8] & (1 << (j % 8))) == 0) {
      plaintextVector[j/8] |= (1 << (j % 8));
      i++;
    }
  }

  /* Load state */
  for (i = 0; i < 32; i++)
    RunSimulator_SetRegister(REG_PTXT_VECTOR + i, plaintextVector[i]);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(REG_PTXT_XOR_MASK + i, rand() & 255);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(REG_CIPHERTEXT0 + i, rand() & 255);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(REG_CIPHERTEXT1 + i, rand() & 255);
  RunSimulator_SetRegister(REG_PTXT_BYTE_MASK, 1 << (rand() & 7));
  i = (rand() % 3) + (rand() & 16);  /* 0/1/2 for CBC & extraXor. 16=activOn */
    fprintf(stderr, "Using mode %d with ActiveOn=%d.\n", (i&3), i/16);
    RunSimulator_SetRegister(REG_SEARCHINFO, i);
  for (i = 0; i < 24; i++) {                              /* for each engine */
    for (j = 0; j < 7; j++)                          /* set random start key */
      RunSimulator_SetRegister(REG_SEARCH_KEY(i)+j, rand() & 255);
    RunSimulator_SetRegister(REG_SEARCH_STATUS(i), 1);
  }

  /* Read out all registers (real and not) except for ptxt vector */
  for (i = 255; i >= 32; i--)
    readout[i] = RunSimulator_CheckRegister(i);
  /* Change the key in any stopped units */
  for (i = 0; i < 24; i++) {
    if ((readout[REG_SEARCH_STATUS(i)] & 1) == 0)                /* stalled? */
      RunSimulator_SetRegister(REG_SEARCH_KEY(i),
              readout[REG_SEARCH_KEY(i)] ^ 0x08);                 /* fix key */
  }
  /* Read out ptxt vector */
  for (i = 31; i >= 0; i--)
    readout[i] = RunSimulator_CheckRegister(i);
  /* scan stopped units */
  for (i = 0; i < 24; i++) {
    if ((readout[REG_SEARCH_STATUS(i)] & 1) == 0) {              /* stalled? */
      goodKey[6] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+0);
      goodKey[5] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+1);
      goodKey[4] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+2);
      goodKey[3] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+3);
      goodKey[2] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+4);
      goodKey[1] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+5);
      goodKey[0] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+6);
      if (rand() % 8)
        RunSimulator_SetRegister(REG_SEARCH_STATUS(i), 1);        /* restart */
      fprintf(stderr, "****** Full match in unit %d; extracted k = ", i);
      for (j = 0; j < 7; j++) {
        fprintf(stderr, "%02X", goodKey[j]);
        printf("%02X", goodKey[j]);
      }
      fprintf(stderr, "\n");
    }
  }

  /* pick a different chip, read/write some registers, and reset chip id */
  do { i = rand() & 255; } while (i == HARDWIRED_CHIP_ID);
  sprintf(buffer, "11011111 %02X %02X 00\n", i, HARDWIRED_CHIP_ID);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);
  buffer[2] = '1';
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);
  buffer[2] = '0';
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(rand() & 255, rand() & 255);
  for (i = 0; i < 8; i++)
    RunSimulator_CheckRegister(rand() & 255);
  sprintf(buffer, "11011111 %02X %02X 00\n", HARDWIRED_CHIP_ID,
          HARDWIRED_CHIP_ID);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);
  buffer[2] = '1';
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);
  buffer[2] = '0';
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  /* Test board enable and ADRSEL1 */
  BOARD_EN_IN = 0;
  ADRSEL1_IN = 0;
  for (i = 0; i < 4; i++)
    RunSimulator_SetRegister(rand() & 255, rand() & 255);
  for (i = 0; i < 4; i++)
    RunSimulator_CheckRegister(rand() & 255);
  BOARD_EN_IN = 0;
  ADRSEL1_IN = 1;
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(rand() & 255, rand() & 255);
  for (i = 0; i < 8; i++)
    RunSimulator_CheckRegister(rand() & 255);
  BOARD_EN_IN = 1;
  ADRSEL1_IN = 0;
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(rand() & 255, rand() & 255);
  for (i = 0; i < 8; i++)
    RunSimulator_CheckRegister(rand() & 255);
  BOARD_EN_IN = 1;
  ADRSEL1_IN = 1;

  /* Make a final pass reading all the registers */
  for (i = 255; i >= 0; i--)
    readout[i] = RunSimulator_CheckRegister(i);
  /* scan stopped units */
  for (i = 0; i < 24; i++) {
    if ((readout[REG_SEARCH_STATUS(i)] & 1) == 0) {              /* stalled? */
      goodKey[6] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+0);
      goodKey[5] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+1);
      goodKey[4] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+2);
      goodKey[3] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+3);
      goodKey[2] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+4);
      goodKey[1] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+5);
      goodKey[0] = RunSimulator_CheckRegister(REG_SEARCH_KEY(i)+6);
      RunSimulator_SetRegister(REG_SEARCH_STATUS(i), 1);          /* restart */
      fprintf(stderr, "****** Full match in unit %d; extracted k = ", i);
      for (j = 0; j < 7; j++) {
        fprintf(stderr, "%02X", goodKey[j]);
        printf("%02X", goodKey[j]);
      }
      fprintf(stderr, "\n");
    }
  }
}



void GetUserInfo(unsigned char plaintextVector[32],
        unsigned char plaintextXorMask[8],
        unsigned char ciphertext0[8], unsigned char ciphertext1[8],
        unsigned char *plaintextByteMask, int *useCBC, int *extraXor,
        int *randomVector, unsigned char startKey[7], long *totalClocks) {
  char buffer[1024];
  unsigned char tmp[512];
  int i;

  printf("Enter plaintextVector values: ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i <= 0 || i >= 256)
    EXIT_ERR("Must have at least 1 plaintextVector entry and at most 255.\n");
  memset(plaintextVector, 0, 32);
  while (i--)
    plaintextVector[tmp[i]/8] |= (1 << (tmp[i] % 8));

  printf("    Enter plaintext xor mask: ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 8)
    EXIT_ERR("Must have 8 plaintext xor mask bytes.");
  memcpy(plaintextXorMask, tmp, 8);

  printf("          Enter ciphertext 0: ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 8)
    EXIT_ERR("Must have 8 bytes in ciphertext 0.");
  memcpy(ciphertext0, tmp, 8);

  printf("          Enter ciphertext 1: ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 8)
    EXIT_ERR("Must have 8 bytes in ciphertext 1.");
  memcpy(ciphertext1, tmp, 8);

  printf("   Enter plaintext byte mask: ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1)
    EXIT_ERR("Plaintext byte mask is 1 byte long.");
  *plaintextByteMask = tmp[0];

  printf("       Enter useCBC (0 or 1): ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1 || tmp[0] > 1)
    EXIT_ERR("Must enter 0 or 1 for useCBC.");
  *useCBC = tmp[0];

  printf("     Enter extraXor (0 or 1): ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1 || tmp[0] > 1)
    EXIT_ERR("Must enter 0 or 1 for extraXor.");
  *extraXor = tmp[0];

  printf(" Enter randomVector (0=randomize, 1=user input, >1=seed): ");
  gets(buffer);
  i = hex2bin(buffer, tmp);
  if (i != 1)
    EXIT_ERR("Must enter 0=randomize 1=use input, >1=value for prng seed).");
  *randomVector = tmp[0];

  printf("          Enter starting key: ");
  gets(buffer);
  if (hex2bin(buffer, tmp) != 7)
    EXIT_ERR("Must enter 7 hex bytes as the key.\n");
  memcpy(startKey, tmp, 7);

  printf("      Enter number of clocks: ");
  gets(buffer);
  sscanf(buffer, "%ld", totalClocks);
  if (*totalClocks < 1 || *totalClocks > 1000000000L)
    EXIT_ERR("Must have between 1 and 1 billion clocks.\n");

  printHexString("\n  PtxtVector = ", plaintextVector, 32);
  printHexString(" PtxtXorMask = ", plaintextXorMask, 8);
  printHexString("Ciphertext 0 = ", ciphertext0, 8);
  printHexString("Ciphertext 1 = ", ciphertext1, 8);
  printHexString("PtxtByteMask = ", plaintextByteMask, 1);
  printf(        "      useCBC = %d\n", *useCBC);
  printf(        "    extraXor = %d\n", *extraXor);
  printf(        "randomVector = %x\n", *randomVector);
  printHexString("Starting key = ", startKey, 7);
  printf(        "Total clocks = %ld\n\n", *totalClocks);
}


void LoadState(unsigned char plaintextVector[32],
        unsigned char plaintextXorMask[8],
        unsigned char ciphertext0[8], unsigned char ciphertext1[8],
        unsigned char plaintextByteMask, int useCBC, int extraXor,
        unsigned char startKey[7]) {
  int i;

  for (i = 0; i < 32; i++)
    RunSimulator_SetRegister(REG_PTXT_VECTOR + i, plaintextVector[i]);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(REG_PTXT_XOR_MASK + i, plaintextXorMask[7-i]);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(REG_CIPHERTEXT0 + i, ciphertext0[7-i]);
  for (i = 0; i < 8; i++)
    RunSimulator_SetRegister(REG_CIPHERTEXT1 + i, ciphertext1[7-i]);
  RunSimulator_SetRegister(REG_PTXT_BYTE_MASK, plaintextByteMask);
  RunSimulator_SetRegister(REG_SEARCHINFO, (useCBC?1:0) |
          (extraXor?2:0) | 16);                       /* enable board active */
  for (i = 0; i < 24; i++) {                              /* for each engine */
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+0, startKey[6]);
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+1, startKey[5]);
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+2, startKey[4]);
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+3, startKey[3]);
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+4, startKey[2]);
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+5, startKey[1]);
    RunSimulator_SetRegister(REG_SEARCH_KEY(i)+6, (startKey[0] + i) & 255);
    RunSimulator_SetRegister(REG_SEARCH_STATUS(i), 1);
  }
}



void RunSimulator_SetRegister(int addr, int data) {
  char buffer[256];

  /* RESET,BOARD_EN,ALE,ADRSEL1,WRB,RDB,ADRSEL2,ALLACT_IN,ADDR,CHIP_ID,DATA */
  sprintf(buffer, "1%d0%d110%d %02x %02x %02x\n", BOARD_EN_IN, ADRSEL1_IN,
          ALLACTIVE_IN, addr, HARDWIRED_CHIP_ID, data);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  sprintf(buffer, "1%d0%d010%d %02x %02x %02x\n", BOARD_EN_IN, ADRSEL1_IN,
          ALLACTIVE_IN, addr, HARDWIRED_CHIP_ID, data);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  sprintf(buffer, "1%d0%d110%d %02x %02x %02x\n", BOARD_EN_IN, ADRSEL1_IN,
          ALLACTIVE_IN, addr, HARDWIRED_CHIP_ID, data);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  if ((rand() & 31) == 0)
    ALLACTIVE_IN = 1-ALLACTIVE_IN;
}


void RunSimulator_DummyIO(void) {
  char buffer[256];
  int i,b,addr,chip;

  if ((rand() & 3) > 0) {
    addr = rand() & 255;
    chip = (rand() & 7) ? HARDWIRED_CHIP_ID : (rand() & 255);
    b = (rand() & 7) ? 1 : 0;
    /*RESET,BOARD_EN,ALE,ADRSEL1,WRB,RDB,ADRSEL2,ALLACT_IN,ADDR,CHIP_ID,DATA*/
    sprintf(buffer, "1%d01110%d %02x %02x 00\n", b, ALLACTIVE_IN, addr, chip);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    sprintf(buffer, "1%d01100%d %02x %02x 00\n", b, ALLACTIVE_IN, addr, chip);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    sprintf(buffer, "1%d01111%d %02x %02x 00\n", b, ALLACTIVE_IN, addr, chip);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
    RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
    fputs(buffer, FILE_TOCHIP);
  } else {
    sprintf(buffer, "1101111%d FF %02x FF\n", ALLACTIVE_IN, HARDWIRED_CHIP_ID);
    for (i = rand() & 7; i > 0; i--) {
      RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
      fputs(buffer, FILE_TOCHIP);
    }
  }
}



unsigned char RunSimulator_CheckRegister(int addr) {
  unsigned char rval;
  char buffer[256];

  /* RESET,BOARD_EN,ALE,ADRSEL1,WRB,RDB,ADRSEL2,ALLACT_IN,ADDR,CHIP_ID,DATA */
  sprintf(buffer, "1%d0%d110%d %02x %02x 00\n", BOARD_EN_IN, ADRSEL1_IN,
          ALLACTIVE_IN, addr, HARDWIRED_CHIP_ID /*no data*/);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  sprintf(buffer, "1%d0%d100%d %02x %02x 00\n", BOARD_EN_IN, ADRSEL1_IN,
          ALLACTIVE_IN, addr, HARDWIRED_CHIP_ID /*no data*/);
  rval=(unsigned char)RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  sprintf(buffer, "1%d0%d111%d %02x %02x 00\n", BOARD_EN_IN, ADRSEL1_IN,
          ALLACTIVE_IN, addr, HARDWIRED_CHIP_ID /*no data*/);
  RunChip(buffer, FILE_FROMCHIP, USE_RAW_IO);
  fputs(buffer, FILE_TOCHIP);

  return (rval);
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

  for (i = 0; i < (int)strlen(hex); i++) {
    if (hex[i] >= '0' && unhex(hex[i]) < 0)
      EXIT_ERR("Bad hex digit encountered.\n");
  }

  for (i = 0; i < (int)strlen(hex); i++) {
    if (hex[i] < '0')
      continue;
    if (hex[i] >= '0' && hex[i+1] >= '0') {
      bin[j++] = (unsigned char)(unhex(hex[i])*16+unhex(hex[i+1]));
      i++;                                                       /* skip one */
      continue;
    }
    if (hex[i] >= '0') {
      bin[j++] = (unsigned char)(unhex(hex[i]));
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


void OpenFiles(char *toChipFilename, char *fromChipFilename, int useRaw) {
  FILE_TOCHIP = fopen(toChipFilename, useRaw ? "rb" : "r");
  if (FILE_TOCHIP != NULL) {
    CREATING_VECTOR = 0;
  } else {
    FILE_TOCHIP = fopen(toChipFilename, useRaw ? "wb" : "w");
    if (FILE_TOCHIP == NULL) {
      fprintf(stderr, "Can't open \"s\" for toChip file\n", toChipFilename);
      exit(1);
    }
    CREATING_VECTOR = 1;
  }

  FILE_FROMCHIP = fopen(fromChipFilename, useRaw ? "wb" : "w");
  if (FILE_FROMCHIP == NULL) {
    fprintf(stderr, "Can't open \"s\" for fromChip file\n", fromChipFilename);
    exit(1);
  }

  USE_RAW_IO = useRaw;
#if 0               /* Activate this to add column descriptors in the output */
  if (!useRaw){
    fprintf(FILE_TOCHIP, "RESET\n");
    fprintf(FILE_TOCHIP, "|BOARD_EN\n");
    fprintf(FILE_TOCHIP, "||ALE\n");
    fprintf(FILE_TOCHIP, "|||ADRSEL1\n");
    fprintf(FILE_TOCHIP, "||||WRB\n");
    fprintf(FILE_TOCHIP, "|||||RDB\n");
    fprintf(FILE_TOCHIP, "||||||ADRSEL2\n");
    fprintf(FILE_TOCHIP, "|||||||ALLACTIVE_IN\n");
    fprintf(FILE_TOCHIP, "|||||||| ADDR\n");
    fprintf(FILE_TOCHIP, "|||||||| /\\ CHIP_ID\n");
    fprintf(FILE_TOCHIP, "|||||||| || /\\ DATA\n");
    fprintf(FILE_TOCHIP, "|||||||| || || /\\ ALLACTIVE_OUT\n");
    fprintf(FILE_FROMCHIP, "DATA\n");
    fprintf(FILE_FROMCHIP, "/\\ ALLACTIVE_OUT\n");
    fprintf(FILE_FROMCHIP, "|| | /-- IsActive [0..23] --\\\n");
  }
#endif
  fprintf(FILE_FROMCHIP, "KEY            DES_OUT          MATCH & SELECT1:\n");
}

