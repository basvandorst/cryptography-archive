/*****************************************************************************
 * sim.c                                                                     *
 *                  Software Simulator for DES keysearch ASIC                *
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
 *   REVISION HISTORY:                                                       *
 *                                                                           *
 *   Version 1.0:  Initial version.                                          *
 *   Version 1.1:  Initial release by Cryptography Research to EFF.          *
 *                 (Fixed byte/bit ordering notation to match VHDL.)         *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "des.h"
#include "sim.h"

#define DEBUG

long getClockCounter(void);
int peekState(int addr);
int RunChip(char *input, FILE *outfile, int useRaw);

static void EXIT_ERR(char *s) { fprintf(stderr, s); exit(1); }
static void parseInput(char *input, int *reset, int *boardEn, int *ale,
        int *adrsel1, int *web, int *rdb, int *adrsel2, int *allactIn,
        int *addr, int *chipId, int *data);
static int unhex(char c);
static void RunClock(void);
static void desDecrypt(unsigned char m[8], unsigned char c[8],
        unsigned char k[7]);
static void increment32(unsigned char *num);
static void decrement32(unsigned char *num);
static void printKeyInfo(FILE *outDev, char *preamble, int searchUnit);

static unsigned char ALLACTIVE_IN = 1;             /* not held between calls */
unsigned char ALLACTIVE_OUT      = 0;

unsigned char STATE[256];
unsigned char SELECTED_CHIP;
         long CLOCK_COUNTER = 1;
          int DES_POSITION;
unsigned char WORKING_CTXT[24*8];          /* last DES input                 */
unsigned char WORKING_PTXT[24*8];          /* last DES out (for ptxt check)  */
unsigned char RAW_DES_OUT[24*8];           /* raw DES outputs                */
int           WORKING_KDELTA[24];          /* key delta (-1, 0, or +1)       */
unsigned char WORKING_LAST_SELECTOR[24];   /* last ciphertext selector       */
unsigned char WORKING_NEXT_SELECTOR[24];   /* next ciphertext selector       */
          int STARTUP_DELAY[24];           /* startup delay                  */
unsigned char THIS_KEY[24*7];              /* current DES key                */
unsigned char NEXT_KEY[24*7];              /* next DES key                   */
         int  PENDING_UPDATE_ADDR1 = -1, PENDING_UPDATE_DATA1 = -1;
         int  PENDING_UPDATE_ADDR2 = -1, PENDING_UPDATE_DATA2 = -1;
         int  PENDING_UPDATE_ADDR3 = -1, PENDING_UPDATE_DATA3 = -1;
unsigned char MATCH[24];



static void resetChip(void) {
  memset(STATE, 0, sizeof(STATE));                                  /* RESET */
  SELECTED_CHIP = 0;
  DES_POSITION = 13;
  memset(WORKING_CTXT, 0, sizeof(WORKING_CTXT));
  memset(WORKING_PTXT, 0, sizeof(WORKING_PTXT));
  memset(RAW_DES_OUT, 0, sizeof(RAW_DES_OUT));
  memset(WORKING_KDELTA, 0, sizeof(WORKING_KDELTA));
  memset(WORKING_LAST_SELECTOR, 1, sizeof(WORKING_LAST_SELECTOR));
  memset(WORKING_NEXT_SELECTOR, 1, sizeof(WORKING_NEXT_SELECTOR));
  memset(STARTUP_DELAY, 0, sizeof(STARTUP_DELAY));
  memset(THIS_KEY, 0, sizeof(THIS_KEY));
  memset(NEXT_KEY, 0, sizeof(NEXT_KEY));
  PENDING_UPDATE_ADDR1 = -1;
  PENDING_UPDATE_ADDR2 = -1;
  PENDING_UPDATE_ADDR3 = -1;
  memset(MATCH, 0, sizeof(MATCH));
}


long getClockCounter(void) {
  return (CLOCK_COUNTER);
}


int peekState(int addr) {
  return (STATE[addr]);
}


int RunChip(char *input, FILE *outfile, int useRaw) {
  int reset,boardEn,ale,adrsel1,web,rdb,adrsel2,allactiveIn,addr,chipId,data;
  int dataOut;
  int i,j;

  parseInput(input, &reset, &boardEn, &ale, &adrsel1, &web, &rdb, &adrsel2,
          &allactiveIn, &addr, &chipId, &data);
  ALLACTIVE_IN = (unsigned char)allactiveIn;

  dataOut = data;                                                 /* default */
  if (reset == 0) {                                                /* reset? */
    resetChip();
    RunClock();
  } else if (boardEn == 0) {                              /* board disabled? */
    RunClock();
  } else if (ale == 1) {                                /* select chip/board */
    RunClock();
    if (adrsel1 == 1)
      SELECTED_CHIP = (unsigned char)addr;
    else
      { /* board select done off-chip */ }
  } else if (chipId != SELECTED_CHIP) {                  /* chipId not ours? */
    RunClock();
  } else if (web == 0) {                                /* writing register? */
    RunClock();
    if (addr >= REG_SEARCH_KEY(0)) {
      PENDING_UPDATE_ADDR2 = addr;                                    /* key */
      PENDING_UPDATE_DATA2 = data;
      if (((addr & 7) == 7) && (data & 1) && ((STATE[addr] & 1) == 0)) {
        if (CLOCK_COUNTER < 750)
          STARTUP_DELAY[(addr - 0x47) / 8] = 21;                  /* adjust? */
        else {
          STARTUP_DELAY[(addr - 0x47) / 8] = 2*CLOCKS_PER_DES - DES_POSITION;
          if (DES_POSITION >= 15)
            STARTUP_DELAY[(addr - 0x47) / 8] += CLOCKS_PER_DES;
#if 0                            /* uncomment for debugging message on halts */
          fprintf(stderr,"Startup with DES_POSITION=%d in unit %d, delay=%d\n",
                DES_POSITION, (addr-0x47)/8, STARTUP_DELAY[(addr - 0x47) / 8]);
#endif
        }
      }
    } else {
      PENDING_UPDATE_ADDR2 = addr;                              /* other reg */
      PENDING_UPDATE_DATA2 = data;
    }
  } else if (rdb == 0) {                                  /* read a register */
    dataOut = STATE[addr];
    RunClock();
  } else {
    RunClock();
  }

  if (CLOCK_COUNTER >= 2) {
    if (useRaw) {
      fprintf(outfile, "%02X %d\n", dataOut, ALLACTIVE_OUT);
    } else {
      fprintf(outfile, " (Addr: %02X)  (Exp: 00) (Get: %02X) at Cycle:%ld\n",
              addr, dataOut, CLOCK_COUNTER);
      for (i = 0; i < 24; i++) {
        for (j = 6; j >= 0; j--)
          fprintf(outfile, "%02X", STATE[REG_SEARCH_KEY(i)+j]);
        fprintf(outfile, " ");
        if (CLOCK_COUNTER < 22)
          fprintf(outfile, "0000000000000000");
        else if (CLOCK_COUNTER <= 37)
          fprintf(outfile, "094CCE83D677160F");
        else {
          for (j = 7; j >= 0; j--)
            fprintf(outfile, "%02X", RAW_DES_OUT[8*i+j]);
        }
#if 0                      /* uncomment to print information about the MATCH */
        {
          static int latch[24]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0};
          if (DES_POSITION==10) latch[i] = MATCH[i];
          fprintf(outfile, " %d", latch[i]);
        }
#endif
#if 0                  /* uncomment to print information about NEXT_SELECTOR */
        {
          static int latch[24]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                1,1,1,1,1,1,1,1,1,1};
          if (DES_POSITION==15) latch[i] = WORKING_NEXT_SELECTOR[i];
          fprintf(outfile, "%d", latch[i]);
        }
#endif
        fprintf(outfile, " : Unit%d\n", i);
      }
      fprintf(outfile, "\n");
    }
  }
  CLOCK_COUNTER++;
  return (dataOut);
}


static void parseInput(char *input, int *reset, int *boardEn, int *ale,
        int *adrsel1, int *web, int *rdb, int *adrsel2, int *allactIn,
        int *addr, int *chipId, int *data) {
  int i;

  if (strlen(input) < 17 || input[8]!=' ' || input[11]!=' ' || input[14]!=' ')
     EXIT_ERR("Bad input.\n");
  for (i = 0; i < 8; i++) {
    if (input[i] != '0' && input[i] != '1')
      EXIT_ERR("Bad input (first 8 digits must be binary.)\n");
  }
  if (unhex(input[9]) < 0 || unhex(input[10]) < 0 ||
        unhex(input[12]) < 0 || unhex(input[13]) < 0 ||
        unhex(input[15]) < 0 || unhex(input[16]) < 0) {
    EXIT_ERR("Bad input (addr, chipId, data must be hex)");
  }

  *reset    = input[0]-'0';
  *boardEn  = input[1]-'0';
  *ale      = input[2]-'0';
  *adrsel1  = input[3]-'0';
  *web      = input[4]-'0';
  *rdb      = input[5]-'0';
  *adrsel2  = input[6]-'0';
  *allactIn = input[7]-'0';
  *addr     = 16*unhex(input[9]) + unhex(input[10]);
  *chipId   = 16*unhex(input[12]) + unhex(input[13]);
  *data     = 16*unhex(input[15]) + unhex(input[16]);
}


/* Decodes a hex char or returns -1 if bad. */
static int unhex(char c) {
  if (c >= '0' && c <= '9')
    return (c - '0');
  if (c >= 'a' && c <= 'f')
    return (c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return (c - 'A' + 10);
  return (-1);
}



/*
 *   Run the system for one clock cycle and update the state.
 */
void RunClock(void) {
  int i,j,k,b;
  unsigned char key[7], m[8], c[8];

  for (i = 0; i < 24; i++) {
    if (STARTUP_DELAY[i] > 0) {
      STARTUP_DELAY[i]--;
      if (STARTUP_DELAY[i] == 0)
        STARTUP_DELAY[i] = -1;               /* prevent stop if 1st C0=match */
    }
  }

  /* DES CLOCK 5: Plaintext vector result from last DES is ready. */
  if (DES_POSITION == 5) {
    for (i = 0; i < 24; i++) {               /* i = search engine            */
      k = 0;                                 /* k = result of byte lookups   */
      for (j = 0; j < 8; j++) {              /* j = byte idx                 */
        b = WORKING_PTXT[8*i+j];             /* b = byte value               */
        if (STATE[b/8] & (1 << (b%8)))       /* check plaintext vector       */
          k = (k >> 1) | 128;                /*   match = load 1 in k msb    */
        else                                 /*                              */
          k = (k >> 1) | 0;                  /*   no match = load 0 into k   */
      }
      k |= STATE[REG_PTXT_BYTE_MASK];        /* set bits where bytemask=1    */
      MATCH[i] = (unsigned char)((k == 255) ? 1 : 0);

      if ((STATE[REG_SEARCH_STATUS(i)] & 1) == 0 || STARTUP_DELAY[i] > 0) {
        /* If search not active, key delta = 0 and do C0 next */
        WORKING_KDELTA[i] = 0;
        WORKING_NEXT_SELECTOR[i] = 1;
      } else if (k != 0xFF || (STATE[REG_SEARCH_STATUS(i)] & 2) ||
              STARTUP_DELAY[i] < 0) {
        /* If no match or CURRENTLY doing C1 or first DES result,
         *     key delta = 1 and do C0 next.
         */
        WORKING_KDELTA[i] = 1;
        WORKING_NEXT_SELECTOR[i] = 0;
        if (k==0xFF)
          printKeyInfo(stderr, "ALERT: Skip match while doing C1 ", i);
        if (k == 0xFF && STARTUP_DELAY[i] < 0)
          printKeyInfo(stderr, "ALERT:    (C1 above is startup phantom.)", i);
      } else if (WORKING_LAST_SELECTOR[i] == 0) {
        /* If doing C0 and got a match from C0, back up and do C1 */
        WORKING_KDELTA[i] = -1;
        WORKING_NEXT_SELECTOR[i] = 1;
        printKeyInfo(stderr, "ALERT: Match C0; will backup for C1 ", i);
      } else {
        /* If doing C0 and got a match from C1, halt */
        STATE[REG_SEARCH_STATUS(i)] &= (255-1);
        WORKING_KDELTA[i] = 0;
        WORKING_NEXT_SELECTOR[i] = 1;
        printKeyInfo(stderr, "ALERT: Matched C1; halting ", i);
      }
      if (STARTUP_DELAY[i] < 0)
        STARTUP_DELAY[i]++;
    }
  }

  if (DES_POSITION == 15) {
    for (i = 0; i < 24; i++) {
      memcpy(THIS_KEY+i*7, NEXT_KEY+i*7, 7);
      memcpy(NEXT_KEY+i*7, STATE+REG_SEARCH_KEY(i), 7);
    }
  }

  /* END OF DES CYCLE: Extract results */
  if (DES_POSITION == CLOCKS_PER_DES-1) {
    for (i = 0; i < 24; i++) {

      /* Do the DES decryption */
      for (j = 0; j < 7; j++)
        key[j] = THIS_KEY[i*7+(6-j)];
      for (j = 0; j < 8; j++)
        c[j] = WORKING_CTXT[8*i+7-j];
      desDecrypt(m, c, key);
      for (j = 0; j < 8; j++) {
        WORKING_PTXT[8*i+7-j] = m[j];
        RAW_DES_OUT[8*i+7-j] = m[j];
      }

      if (STATE[REG_SEARCHINFO] & 2) {                     /* if extraXOR    */
        WORKING_PTXT[8*i+4] ^= WORKING_PTXT[8*i+0];        /*    L = L xor R */
        WORKING_PTXT[8*i+5] ^= WORKING_PTXT[8*i+1];        /*      "         */
        WORKING_PTXT[8*i+6] ^= WORKING_PTXT[8*i+2];        /*      "         */
        WORKING_PTXT[8*i+7] ^= WORKING_PTXT[8*i+3];        /*      "         */
      }
      if ((STATE[REG_SEARCH_STATUS(i)] & 2) == 0) {         /* if c0,        */
        WORKING_PTXT[8*i+0] ^= STATE[REG_PTXT_XOR_MASK+0];  /* do ptxtXorMsk */
        WORKING_PTXT[8*i+1] ^= STATE[REG_PTXT_XOR_MASK+1];  /*      "        */
        WORKING_PTXT[8*i+2] ^= STATE[REG_PTXT_XOR_MASK+2];  /*      "        */
        WORKING_PTXT[8*i+3] ^= STATE[REG_PTXT_XOR_MASK+3];  /*      "        */
        WORKING_PTXT[8*i+4] ^= STATE[REG_PTXT_XOR_MASK+4];  /*      "        */
        WORKING_PTXT[8*i+5] ^= STATE[REG_PTXT_XOR_MASK+5];  /*      "        */
        WORKING_PTXT[8*i+6] ^= STATE[REG_PTXT_XOR_MASK+6];  /*      "        */
        WORKING_PTXT[8*i+7] ^= STATE[REG_PTXT_XOR_MASK+7];
      } else {                                              /* if c1         */
        if (STATE[REG_SEARCHINFO] & 1) {                    /* if useCBC     */
          WORKING_PTXT[8*i+0] ^= STATE[REG_CIPHERTEXT0+0];  /* xor with c0   */
          WORKING_PTXT[8*i+1] ^= STATE[REG_CIPHERTEXT0+1];  /*      "        */
          WORKING_PTXT[8*i+2] ^= STATE[REG_CIPHERTEXT0+2];  /*      "        */
          WORKING_PTXT[8*i+3] ^= STATE[REG_CIPHERTEXT0+3];  /*      "        */
          WORKING_PTXT[8*i+4] ^= STATE[REG_CIPHERTEXT0+4];  /*      "        */
          WORKING_PTXT[8*i+5] ^= STATE[REG_CIPHERTEXT0+5];  /*      "        */
          WORKING_PTXT[8*i+6] ^= STATE[REG_CIPHERTEXT0+6];  /*      "        */
          WORKING_PTXT[8*i+7] ^= STATE[REG_CIPHERTEXT0+7];  /*      "        */
        }
      }

      /* Update ciphertext selector (state & last) */
      WORKING_LAST_SELECTOR[i] = (STATE[0x47+8*i] & 2) ? 1 : 0;
      STATE[0x47+8*i] &= 0xFD;                      /* select ciphertext 0   */
      if (WORKING_NEXT_SELECTOR[i])                 /* ... unless we want c1 */
        STATE[0x47+8*i] |= 2;                       /* ... then select c1    */
    }
  }

  /* LAST DES CLOCK: Load in the updated key */
  if (DES_POSITION == 14) {
    for (i = 0; i < 24; i++) {
      if (WORKING_KDELTA[i] == 1) {                    /*   if key delta = 1 */
        increment32(STATE+REG_SEARCH_KEY(i));
      }
      if (WORKING_KDELTA[i] == -1) {                  /*   if key delta = -1 */
        decrement32(STATE+REG_SEARCH_KEY(i));
      }
    }
  }

  /* DES CLOCK 0: Latch in new working keys and working ciphertexts */
  if (DES_POSITION == 0) {
    for (i = 0; i < 24; i++) {                          /* i = search engine */
      /* pick between ctxt 0 and ctxt 1? */
      if ((STATE[REG_SEARCH_STATUS(i)] & 2) == 0 && STARTUP_DELAY[i] == 0)
        memcpy(WORKING_CTXT+8*i, STATE+REG_CIPHERTEXT0, 8);       /* copy c0 */
      else
        memcpy(WORKING_CTXT+8*i, STATE+REG_CIPHERTEXT1, 8);       /* copy c1 */
    }
  }

  /* Update ChipAllActive, board all active */
  j = 1;
  for (i = 0; i < 24; i++)
    j &= STATE[0x47+i*8];
  j = (j & 1) ? 1 : 0;
  STATE[REG_SEARCHINFO] &= (255-4);        /* set ChipAllActive              */
  STATE[REG_SEARCHINFO] |= (4*j);          /*    "                           */
  if ((STATE[REG_SEARCHINFO] & 16) == 0)   /* If board all active enable = 0 */
    ALLACTIVE_OUT = ALLACTIVE_IN;
  else
    ALLACTIVE_OUT = ALLACTIVE_IN & j;
  STATE[REG_SEARCHINFO] &= (255-8);                  /* set board all active */
  STATE[REG_SEARCHINFO] |= (8*ALLACTIVE_OUT);        /* set board all active */

  /* Do any pending updates and update DES cycle position */
  if (PENDING_UPDATE_ADDR1 >= 0)
    STATE[PENDING_UPDATE_ADDR1] = PENDING_UPDATE_DATA1;
  PENDING_UPDATE_ADDR1 = PENDING_UPDATE_ADDR2;
  PENDING_UPDATE_DATA1 = PENDING_UPDATE_DATA2;
  PENDING_UPDATE_ADDR2 = PENDING_UPDATE_ADDR3;
  PENDING_UPDATE_DATA2 = PENDING_UPDATE_DATA3;
  PENDING_UPDATE_ADDR3 = -1;
  DES_POSITION = (DES_POSITION + 1) % CLOCKS_PER_DES;
}


static void desDecrypt(unsigned char m[8], unsigned char c[8],
        unsigned char k[7]) {
  bool key[56], message[64];
  int i;

#ifdef DEBUG
  printf("DES_DECRYPT(k="); for (i=0; i<7;i++) printf("%02X",k[i]);
  printf(", c="); for (i=0; i<8;i++) printf("%02X",c[i]);
#endif

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

#ifdef DEBUG
  printf(")=");
  for (i=0; i<8;i++)
    printf("%02X",m[i]);
  printf(", clk=%ld\n",CLOCK_COUNTER);
#endif

}




static void printKeyInfo(FILE *outDev, char *preamble, int searchUnit) {
  fprintf(outDev, preamble);
  fprintf(outDev, "(K=%02X%02X%02X%02X%02X%02X%02X, clk=%ld, searchUnit=%d)\n",
                STATE[0x40+8*searchUnit+6],STATE[0x40+8*searchUnit+5],
                STATE[0x40+8*searchUnit+4],STATE[0x40+8*searchUnit+3],
                STATE[0x40+8*searchUnit+2],STATE[0x40+8*searchUnit+1],
                STATE[0x40+8*searchUnit+0], CLOCK_COUNTER, searchUnit);

  printf(preamble);
  printf("(K=%02X%02X%02X%02X%02X%02X%02X, clk=%ld, searchUnit=%d)\n",
                STATE[0x40+8*searchUnit+6],STATE[0x40+8*searchUnit+5],
                STATE[0x40+8*searchUnit+4],STATE[0x40+8*searchUnit+3],
                STATE[0x40+8*searchUnit+2],STATE[0x40+8*searchUnit+1],
                STATE[0x40+8*searchUnit+0], CLOCK_COUNTER, searchUnit);

}


static void increment32(unsigned char *num) {
  if ((++(num[0])) == 0)
    if ((++(num[1])) == 0)
      if ((++(num[2])) == 0)
        ++(num[3]);
}


static void decrement32(unsigned char *num) {
  if (((num[0])--) == 0)
    if (((num[1])--) == 0)
      if (((num[2])--) == 0)
        (num[3])--;
}


