/*
 * Season7 -- Videocrypt smard card emulator for DOS.
 *
 * Copyright 1994 by Markus Kuhn
 *
 * Compiles e.g. with Borland C++ 2.0, small model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <conio.h>            /* for kbhit() & clrscr() */
#include <dos.h>              /* for int86() */

#include "decrypt.h"
#include "async.h"

#define VERSION "1.3"         /* last change: 1994-06-18 */

/*
 * information in answer to reset: (as defined in ISO 7816-3)
 * inverse convention (3f), protocol type T=0, F=372, D=1, I=50mA, P=5V, N=5.
 * historic characters (final 10 bytes): ???
 */
int reset_answer_len = 16;
char reset_answer[33] = {
  0x3f, 0xfa, 0x11, 0x25, 0x05, 0x00, 0x01, 0xb0,
  0x02, 0x3b, 0x36, 0x4d, 0x59, 0x02, 0x81, 0x80
};

/* we show this shortly after a reset on the screen */
unsigned char display[25] ="\x80"
  "SEASON7 V" VERSION
  "   --:--    ";

/* BSkyB channel id (based on byte 7 in 74 messages) for TV screen display */
char *channel_id[16] = {
  "    ????    ", " SKY MOVIES ", "MOVIECHANNEL", "MOVIES  GOLD",
  "    ????    ", "    ????    ", " SKY SPORTS ", "    ????    ",
  "  TV ASIA   ", "    ????    ", "    ????    ", "    ????    ",
  "MULTICHANNEL", "    ????    ", "    ????    ", "    ????    "
};

/* BSkyB channel name for text screen display */
char *channel_name[16] = {
  "???", "Sky Movies", "The Moviechannel", "Sky Movies Gold",
  "???", "???", "Sky Sports", "???",
  "TV Asia", "???", "???", "???",
  "Sky Multichannels", "???", "???", "???"
};

/* message directions */
#define IN   0   /* to card */
#define OUT  1   /* to decoder */


/*
 * These are some delay values (microseconds) used in the program.
 * Use command line option w to modify them.
 */
#define DELAYS 2
unsigned long delus[DELAYS] = {
  30000,      /* wait between reset and answer to reset */
  500,        /* wait after each outgoing byte */
};


/*
 * Reverse and invert all bits in each byte of a string.
 * E.g. 10010111b (0x97) becomes 00010110b (0x16).
 */
void inverse(char *data, int len) {
  int i, j;
  unsigned char c;

  for (i = 0; i < len; i++) {
    c = 0;
    for (j = 0; j < 8; j++)
      c |= ((data[i] & (1 << j)) != 0) << (7 - j);
    data[i] = ~c;
  }

  return;
}


/*
 * Uses a BIOS function in order to wait a specified number of
 * microseconds. This BIOS functions is only implemented in
 * IBM AT compatible BIOSes (i.e., not in old PC and XT systems)!
 * The timing resolution of this function isn't really down
 * to 1 us on most systems, but it's fine for our purposes.
 */
void wait_us(unsigned long microseconds)
{
  union REGS regs;

  if (microseconds == 0) return;
  regs.h.ah = 0x86;
  regs.x.cx = (int) (microseconds >> 16);
  regs.x.dx = (int) (microseconds & 0xffff);
  int86(0x15, &regs, &regs);
  return;
}


void activate_com(int com)
{
  int port;

  switch (com) {
  case 1: port = COM1; break;
  case 2: port = COM2; break;
  case 3: port = COM3; break;
  case 4: port = COM4; break;
  default:
    printf("Port COM%d not available!\n", com);
    exit(1);
  }
  if (AsyncInit(port)) {
    printf("Can't initialize port COM%d!\n", com);
    exit(1);
  }
  AsyncHand(DTR | RTS);
  AsyncSet(9600, BITS_8 | STOP_2 | ODD_PARITY);

  return;
}


void deactivate_com(void)
{
  AsyncStop();
}


/*
 * Send a string of bytes to serial port and wait for each single echo.
 * (More efficient send methods are too fast for decoder, it seems to
 * need more than 2 stop bits, so we wait between individual bytes.)
 */
int send(char *data, int len)
{
  int i;
  char c;

  AsyncClear();
  for (i = 0; i < len; i++) {
    c = data[i];
    inverse(&c, 1);
    AsyncOut(c);
    while (AsyncInStat() == 0)
      if ((AsyncStat() & DCD) || kbhit()) return 1;
    AsyncIn();
    wait_us(delus[1]);
  }

  return 0;
}


/*
 * Get all bytes available from serial port FIFO and return how many
 * bytes were available.
 */
int receive(char *data, int max)
{
  int i = 0;

  while (AsyncInStat() > 0 && i < max)
    data[i++] = AsyncIn();
  inverse(data, i);

  return i;
}


/*
 * Get n bytes from serial port FIFO. Don't wait more than timeout
 * microseconds and return how many bytes were available.
 */
int receive_timeout(char *data, int n, long timeout)
{
  int i = 0;

  while (i < n && timeout > 0)
    if (AsyncInStat() > 0)
      data[i++] = AsyncIn();
    else {
      wait_us(15000);
      timeout -= 15000;
    }
  inverse(data, i);

  return i;
}


/*
 * In debugging mode, print the contents of all packets.
 */
void log_packet(unsigned char *header, unsigned char *data,
                int direction)
{
  int i, j;
  static time_t last_time = 0;
  time_t now;
  char tmp[40];

  /* print time from time to time */
  time(&now);
  if (difftime(now, last_time) >= 10) {
    strftime(tmp, 40, "%Y-%m-%d %H:%M:%SZ", gmtime(&now));
    printf("%s\n", tmp);
    last_time = now;
  }

  if (header[1] == 0x60)    /* illegal command code 0x60: reset */
    printf("RESET:   ");
  else
    printf("%s %02x: ", (direction == IN) ? "dcdr" : "card", header[1]);
  for (i = 0; i < header[4]; i += 16) {
    if (i > 0) printf("         ");
    for (j = 0; j < 16; j++)
      if (i + j < header[4])
        printf("%02x ", data[i + j]);
      else
        printf("   ");
    printf("  ");
    for (j = 0; j < 16 && i + j < header[4]; j++)
      putchar((data[i + j] > 31 && data[i + j] < 127) ? data[i + j] : '.');
    putchar('\n');
  }

  return;
}



/*
 * Parse an answer to reset packet.
 *
 * Return value:  >0  correct answer to reset received, length returned
 *                -1  no correct answer to reset received
 */
int parse_reset(void)
{
  int y, p, i, tck_expected = 0;

  reset_answer_len = 0;

  /* Skip reset impulse noise characters and read TS */
  reset_answer[0] = 0;
  for (i = 0; i < 5 && reset_answer[0] != 0x3f; i++)
    if (receive_timeout(reset_answer, 1, 500000L) == 0) return -1;
  if (reset_answer[0] != 0x3f) return -1;

  /* Read T0 */
  if (receive_timeout(reset_answer + 1, 1, 500000L) == 0) return -1;

  /* Read TA_i, TB_i, TC_i, TD_i */
  p = 2;
  y = reset_answer[1] >> 4;
  while (y && p < 33 - 5 - (reset_answer[1] & 15)) {
    for (i = 0; i < 4; i++) {
      if ((y >> i) & 1) {
        if (receive_timeout(reset_answer + p, 1, 500000L) != 1) return -1;
        if (i == 3) {
          tck_expected |= (reset_answer[p] & 15) != 0;
          y = reset_answer[p] >> 4;
        }
        p++;
      } else
        if (i == 3) y = 0;
    }
  }  /* while () */

  /* historic characters */
  if (receive_timeout(reset_answer + p, reset_answer[1] & 15, 500000L) !=
      (reset_answer[1] & 15))
    return -1;
  p += (reset_answer[1] & 15);

  /* TCK */
  if (tck_expected) {
    if (receive_timeout(reset_answer + p, 1, 500000L) != 1) return -1;
    p++;
    y = 0;
    for (i = 1; i < p; i++)
      y ^= reset_answer[i];
    if (y != 0) return -1;
  }

  /* everything was fine */
  reset_answer_len = p;

  return p;
}



main(int argc, char **argv)
{
  unsigned char header[5];
  int hc = 0, received = -1, length, dir;
  unsigned char buffer[65];
  unsigned char msg[32];
  unsigned char seed[8];
  unsigned char *data;
  char proc_byte;    /* procedure byte in passive mode */
  int sw2_next = 0;  /* second procedure byte SW2 will follow */
  int sw2_ok = 0;    /* second procedure byte SW2 has been received */
  int sync_lost = 1;
  time_t now;
  char *current_name = "???";
  int station = 0;
  int com = 2;
  int debug = 0, quit = 0, message = 1;
  int economy = 0;
  int passive = 0;   /* passive==1: Season7 listens only to card traffic */
  int suspended = 0;
  int i, j, k, c;
#define LOGMAX 400
  time_t logtime[LOGMAX], start;
  unsigned char logmsg[LOGMAX][32];
  int loglength = 0, lognext = 0, check;
  char *logfn = "VCLOG";
  char satellite[80], transponder[80], program[80];
  char tmp[129], fn[16];
  FILE *f;
  FILE *vcl_out = NULL, *vcl_in = NULL;
  char *vcl_out_fn = NULL, *vcl_in_fn = NULL;
  unsigned long vcl_count = 0, l;
  unsigned vcl_current = 0, u;
  long *vcl_msg_hash;
  char (*vcl_seed)[8];

#ifndef __MSC
  clrscr();
#endif
  fprintf(stderr, "\nSeason7 -- Version " VERSION
                  " -- Videocrypt smart card emulator\n\n");
  fprintf(stderr,
    "Important: This software must not be used in order to view channels\n"
    "           for which a regular subscription would be available in\n"
    "           your country, unless you have already such a subscription\n"
    "           and want to use this software only for educational\n"
    "           purposes.\n\n");

  tzset();  /* get local time zone from environment variable TZ */
  time(&start);

  for (i = 1; i < argc; i++) {
    if (isdigit(argv[i][0]))
      com = atoi(argv[i]);
    else
      for (j = 0; j < 999 && argv[i][j]; j++)
        switch(argv[i][j]) {
        case 'd':
        case 'D':
          /* switch on screen log mode */
          debug = 1;
          break;
        case 'p':
        case 'P':
          /* activate passive mode: Season7 is now only listening */
          passive = 1;
          break;
        case 'm':
        case 'M':
          /* suppress TV display messages */
          message = 0;
          break;
        case 'e':
        case 'E':
          /* economy mode: avoid time consuming tasks like printf()s
           * in order to allow stable operation on very slow systems */
          economy = 1;
          break;
        case 'o':
        case 'O':
          /* produce a VCL (videocrypt card log) file from the data
           * recorded in passive mode.                              */
          if (vcl_out_fn || vcl_in_fn) break;
          fprintf(stderr, "Which satellite do you receive (Astra): ");
          gets(satellite);
          if (strlen(satellite) == 0) strcpy(satellite, "Astra");
          fprintf(stderr, "Which transponder on '%s' do you record (08): ",
                  satellite);
          gets(transponder);
          if (strlen(transponder) == 0) strcpy(transponder, "08");
          fprintf(stderr, "What are you recording: ");
          gets(program);
          fprintf(stderr, "\n");
          satellite[31] = 0;
          transponder[7] = 0;
          program[63] = 0;
          for (k = strlen(satellite); k < 32; k++) satellite[k] = 0;
          for (k = strlen(transponder); k < 8; k++) transponder[k] = 0;
          for (k = strlen(program); k < 64; k++) program[k] = 0;
          if (argv[i][j+1] == '\0') {
            start += 60 * 5; /* round up the hour by 5 minutes */
            strftime(fn, 9, "vc%m%d%H", gmtime(&start));
            start -= 60 * 5;
            sprintf(fn + 8, ".%.1s%.2s", satellite, transponder);
            vcl_out_fn = fn;
          } else
            vcl_out_fn = argv[i] + j + 1;
          j = 1000;
          passive = 1;
          break;
        case 'i':
        case 'I':
          /* read in a VCL file and use it in order to answer the
           * hash function queries of the decoder.                */
          if (vcl_in_fn || vcl_out_fn) break;
          if (argv[i][j+1] == '\0') {
            fprintf(stderr, "Specify filename after option i "
                    "(e.g. 'season7 ivc061817.a08')!\n");
            exit(1);
          } else
            vcl_in_fn = argv[i] + j + 1;
          j = 1000;
          break;
        case 'w':
        case 'W':
          /*
           * modify delay table, e.g. option wb200 waits 200 us after each
           * byte because this sets delus['b' - 'a'] = 200.
           */
          k = tolower(argv[i][j+1]) - 'a';
          if (k < 0 || k >= DELAYS || !isdigit(argv[i][j + 2])) {
            printf("Only wait options between wa<ticks> and "
                   "w%c<ticks> possible.\n", 'a' + DELAYS - 1);
            exit(1);
          }
          j += 2;
          delus[k] = atol(argv[i] + j);
          while (isdigit(argv[i][j + 1])) j++;
          break;
        default:
          break;
        }
  }

  fprintf(stderr, "Press q for exit.\n\n");
  fprintf(stderr, "Using serial port COM%d, byte delay %lu \xe6s.\n",
          com, delus[1]);
  strftime(tmp, 80, "%Y-%m-%d %H:%M:%S", gmtime(&start));
  fprintf(stderr, "World time (UTC): %s\n", tmp);
  if (passive)
    fprintf(stderr, "Passive mode: only listening to data traffic.\n");

  if (vcl_out_fn) {
    /* open VCL output file */
    vcl_out = fopen(vcl_out_fn, "wb");
    if (vcl_out == NULL) {
      fprintf(stderr, "Sorry, can't open VCL file '%s", vcl_out_fn);
      perror("'");
      exit(1);
    }
    fprintf(stderr, "Writing VCL file '%s'.\n", vcl_out_fn);
    /* Write VCL header (128 bytes) */
    fprintf(vcl_out, "VCL1");  /* Magic code, version 1 */
    putc(0, vcl_out);          /* space for number of seeds (bigendian) */
    putc(0, vcl_out);
    putc(0, vcl_out);
    putc(0, vcl_out);
    strftime(tmp, 80, "%Y%m%dT%H%M%SZ", gmtime(&start));
    fwrite(tmp, 1, 16, vcl_out);
    fwrite(satellite, 1, 32, vcl_out);
    fwrite(transponder, 1, 8, vcl_out);
    fwrite(program, 1, 64, vcl_out);
  }

  if (vcl_in_fn) {
    /* open VCL input file */
    vcl_in = fopen(vcl_in_fn, "rb");
    if (vcl_in == NULL) {
      fprintf(stderr, "Sorry, can't open VCL file '%s", vcl_in_fn);
      perror("'");
      exit(1);
    }
    fprintf(stderr, "Reading VCL file '%s'.\n", vcl_in_fn);
    fread(tmp, 1, 128, vcl_in);
    if (feof(vcl_in)) {
      fprintf(stderr, "Unexpected end of VCL input file!\n");
      exit(1);
    }
    if (strncmp(tmp, "VCL1", 4)) {
      fprintf(stderr, "Sorry, '%s' is not a VCL file!\n", vcl_in_fn);
      exit(1);
    }
    tmp[55] = tmp[63] = tmp[127] = 0;
    printf("\n%s, transponder %s, %.4s-%.2s-%.2s %.2s:%.2s:%.2s%.1s\n%s\n\n",
           tmp + 24, tmp + 56, tmp + 8, tmp + 12, tmp + 14,
           tmp + 17, tmp + 19, tmp + 21, tmp + 23, tmp + 64);
    if (tmp[4] || tmp[5]) {
      fprintf(stderr, "Sorry, VCL file too long for this implementation.\n");
      exit(1);
    }
    vcl_count = ((unsigned long)(unsigned char) tmp[6] << 8) |
                 (unsigned long)(unsigned char) tmp[7];
#if (sizeof(long) != 4)
    fprintf(stderr, "Porting error!\n");
    exit(1);
#endif
    vcl_msg_hash = (long *) malloc((size_t) (vcl_count * 4L));
    vcl_seed = (char (*)[8]) malloc((size_t) (vcl_count * 8L));
    if (!vcl_msg_hash || !vcl_seed) {
      fprintf(stderr, "Not enough memory for %lu bytes (%lu entries).\n",
              vcl_count * 12, vcl_count);
      exit(1);
    }
    for (l = 0; l < vcl_count; l++)
      if (fread(vcl_msg_hash + (unsigned) l, 1, 4, vcl_in) != 4 ||
          fread(vcl_seed + (unsigned) l, 1, 8, vcl_in) != 8) {
        fprintf(stderr, "Unexpected end of VCL input file!\n");
        exit(1);
      }
  }

  activate_com(com);
  AsyncStat();   /* clear D_DCD */

  if (economy)
    fprintf(stderr, "\nOperating ...");
  else
    fprintf(stderr, "\nWaiting for reset ...");

  while (!quit) {
    /* keyboard commands */
    if (kbhit()) {
      c = getch();
      switch (c) {
      case 0:
        /* ignore function keys */
        getch();
        break;
      case 'q':
      case 'Q':
      case 'x':
      case 'X':
      case 27:
        /* abort program */
        quit = 1;
        continue;
      case 'd':
      case 'D':
        /* switch on/off screen log */
        debug = !debug;
        printf("\rDebug mode %s.\t\t\t\t\t\n", debug ? "on" : "off");
        break;
      case 'l':
      case 'L':
        /* write last crypto messages (command 0x74) to a file */
        if (economy) break;
        i = lognext - loglength;
        while (i < 0) i += LOGMAX;
        printf("\rWriting last %d messages to logfile '%s'...\t\t\t\n",
               loglength, logfn);
        f = fopen(logfn, "w");
        if (f == NULL)
          perror("Sorry, can't open logfile");
        else {
          strftime(tmp, 80, "%Y-%m-%d", gmtime(&logtime[i]));
          fprintf(f, "%s\n", tmp);
          for (j = 0; j < loglength; j++) {
            strftime(tmp, 80, "%H:%M:%SZ", gmtime(&logtime[i]));
            fprintf(f, "%s ", tmp);
            for (k = 0; k < 32; k++)
              fprintf(f, "%02x", logmsg[i][k]);
            fprintf(f, "\n");
            if (++i >= LOGMAX) i = 0;
          }
          fclose(f);
        }
        break;
      case '+':
        /* increase byte delay by 0.5 ms */
        if (delus[1] < 2000) delus[1] += 500;
        printf("\rByte delay set to %lu \xe6s.\t\t\t\t\t\n", delus[1]);
        break;
      case '-':
        /* decrease byte delay by 0.5 ms */
        if (delus[1] >= 500) delus[1] -= 500;
        printf("\rByte delay set to %lu \xe6s.\t\t\t\t\t\n", delus[1]);
        break;
      case 's':
      case 'S':
        if (!vcl_out || suspended) break;
        printf("\rVCL file recording suspended.\t\t\t\t\t\n");
        suspended = 1;
        break;
      case 'c':
      case 'C':
        if (!vcl_out || !suspended) break;
        printf("\rVCL file recording continues.\t\t\t\t\t\n");
        suspended = 0;
        break;
      default:
        break;
      }
    }
    if (AsyncStat() & (DCD | D_DCD)) {
      /*
       *  we have a reset and the decoder now expects an
       *  answer-to-reset packet from the card as described in
       *  ISO 7816.
       */
      if (!passive || (AsyncStat() & DCD))
        AsyncClear();
      sync_lost = 0;
      if (!economy) fprintf(stderr, "\r\t\t\t\t\t\r");
      if (!economy && !debug)
        printf("RESET");
      while ((AsyncStat() & DCD) && !kbhit());
      if (kbhit()) continue;
      if (passive) {
        if (parse_reset() < 0) sync_lost = 1;
      } else {
        wait_us(delus[0]);
        if (send(reset_answer, reset_answer_len)) continue;
        if (!economy && message) {
          now = time(NULL);
          strftime((char *) display + 13, 13, "   %H:%M    ",
                   localtime(&now));
          display[0] = '\xd8';
        }
      }
      if (debug && !sync_lost) {
        header[1] = 0x60; /* special value indicates log_packet a reset */
        header[4] = reset_answer_len;
        log_packet(header, (unsigned char *) reset_answer, 0);
      }
      station = 0;
      hc = 0;
      received = -1;
      sw2_next = 0;
      AsyncStat();  /* clear D_DCD */
    }

    if (passive && sync_lost) {
      /*
       * In passive mode, we can't just wait for a reset if
       * anything went wrong. So we try to resynchronize here
       * by waiting for characteristic byte sequences like
       * 90 00, 53 7x, or 53 8x. Let's hope this makes the passive mode a
       * little bit more robust.
       */
       if (receive((char *) buffer + sync_lost - 1, 1)) {
         if (sync_lost == 1 && (buffer[0] == 0x90 || buffer[0] == 0x53)) {
           sync_lost++;
         } else {
           if (sync_lost == 2) {
             if (buffer[0] == 0x90 && buffer[1] == 0x00) {
               /* same state as after a reset */
               hc = 0;
               received = -1;
               sw2_next = 0;
               sync_lost = 0;
               if (!economy) fprintf(stderr, "\r\t\t\t\t\t\r");
             } else if (buffer[0] == 0x53 &&
                        ((buffer[1] & 0xf0) == 0x70 ||
                         (buffer[1] & 0xf0) == 0x80)) {
               header[0] = buffer[0];
               header[1] = buffer[1];
               hc = 2;
               received = -1;
               sw2_next = 0;
               sync_lost = 0;
               if (!economy) fprintf(stderr, "\r\t\t\t\t\t\r");
             } else {
               /* resynchronization attempt failed, because of 2nd byte */
               sync_lost = 1;
             }
           }  /* if (sync_lost == 2) */
         }
       } /* if (receive()) */
    } else if (hc < 5) {
      /* waiting for more header bytes */
      hc += receive((char *) header + hc, 5 - hc);
    } else {
      if (received < 0) {
        /* new 5 byte command header has arrived */
        if (header[0] != 0x53) {
          if (!passive) {
            send("\x6e\x00", 2);  /* unsupported class */
            hc = 0;
          } else sync_lost = 1;
          printf("\rUnsupported class code %02x!\t\t\n", header[0]);
        } else {
          data = buffer;
          memset(buffer, 0, 64); /* default answer: 00 00 00 ... */
          switch (header[1]) { /* the different instruction types */
            case 0x70: length =  6; dir = OUT; break;
            case 0x72: length = 16; dir = IN;  break;
            case 0x74: length = 32; dir = IN;  data = msg; break;
            case 0x76: length =  1; dir = IN;  break;
            case 0x78: length =  8; dir = OUT; data = seed; break;
            case 0x7a: length = 25; dir = OUT; data = display; break;
            case 0x7c: length = 16; dir = OUT; break;
            case 0x7e: length = 64; dir = OUT; break;
            case 0x80: length =  1; dir = IN;  break;
            case 0x82: length = 64; dir = OUT; break;
            default:
              if (!passive) {
                send("\x6d\x00", 2);  /* unsupported instruction */
                hc = 0;
              } else sync_lost = 1;
              printf("\rUnsupported instruction code %02x!\t\t\n", header[1]);
          }
          if (header[2] != 0x00 || header[3] != 0x00) {
            if (!passive) {
              send("\x6b\x00", 2);  /* incorrect reference */
              hc = 0;
            } else sync_lost = 1;
            printf("\rUnexpected P1 P2 values: %02x %02x!\t\t\t\n",
                   header[2], header[3]);
          }
          if (length != header[4]) {
            if (!passive) {
              send("\x67\x00", 2);  /* incorrect length */
              hc = 0;
            } else sync_lost = 1;
            printf("\rIncorrect length %d in instruction %02x!\t\t\n",
                   header[4], header[1]);
          }
          if (hc == 5)
            if (!passive) {
              /* procedure byte: no Vpp, send all data in one piece */
              if (send((char *) header + 1, 1)) continue;
              if (dir == IN)
                received = 0;
              else {
                /* here comes the answer */
                if (send((char *) data, length)) continue;
                if (send("\x90\x00", 2)) continue;  /* ack: everything ok */
                if (debug) log_packet(header, data, OUT);
                hc = 0;
                received = -1;
              }
            } else {
              if (receive(&proc_byte, 1)) {
                if (sw2_next) {
                  hc = 0;
                  received = -1;
                  sw2_next = 0;
                } else if (((proc_byte & 0xf0) == 0x60 ||
                            (proc_byte & 0xf0) == 0x90) && proc_byte != 0x60)
                  sw2_next = 1;
                else if (((proc_byte ^ header[1]) & 0xfe) == 0xfe) {
                  printf("\rSingle byte handshaking not supported!\t\t\n");
                  sync_lost = 1;
                }
                else if ((proc_byte & 0xfe) != header[1]) {
                  printf("\rUnexpected procedure byte %02x, INS = %02x!\t\n",
                         (unsigned char) proc_byte, header[1]);
                  sync_lost = 1;
                } else {
                  received = 0;
                  sw2_ok = 0;
                }
              }
            }
        }
      } else if (received < header[4]) {
        /* waiting for more data to receive */
        received += receive((char *) data + received, header[4] - received);
      } else if (!passive || sw2_ok) {
        /* here, all data has been received */
        if (!passive)
          if (send("\x90\x00", 2)) continue;       /* ack: everything ok */
        if (debug) log_packet(header, data, dir);
        /* reaction on received data */
        switch (header[1]) {
          case 0x72:
            if (debug) break;
            printf("\nMessage from previous card:\n");
            for (i = 0; i < 16; i++)
              printf("%02x ", buffer[i]);
            printf("  ");
            for (i = 0; i < 16; i++)
              putchar((buffer[i] > 31 && buffer[i] < 127) ? buffer[i] : '.');
            putchar('\n');
            break;
          case 0x74:
            if (data[0] & 8) {
              if (vcl_in) {
                /* lookup random number seed in VCL file */
                u = vcl_current + 1;
                if (u >= vcl_count) u = 0;
                while (*(long *)(msg + 28) != vcl_msg_hash[u] &&
                       u != vcl_current)
                  if (++u >= vcl_count) u = 0;
                if (u != vcl_current) {
                  vcl_current = u;
                  memcpy(seed, vcl_seed + u, 8);
                  display[0] = 0x80;
                  if (!debug && !economy)
                    printf("\r\t\t\t\t\t\rMatched position %u.", u);
                } else {
                  memset(seed, 0, 8);
                  if (!debug && !economy)
                    printf("\r\t\t\t\t\t\rNo match currently in VCL file.");
                  if (!economy && message) {
                    display[0] = '\xd8';
                    memcpy(display + 13, "NO MATCH :-(", 12);
                  }
                }
              } else {
                /* calculate random number seed from crypto message */
                check = decode(msg, seed);
                if (!economy) {
                  /* keep history of last LOGMAX crypto messages */
                  time(&logtime[lognext]);
                  memcpy(logmsg[lognext++], buffer, 32);
                  if (loglength < LOGMAX) loglength++;
                  if (lognext >= LOGMAX) lognext = 0;
                  /* manage on screen display */
                  if (station == 0) {
                      if ((check >> 2) == 1) {
                        /* it's a Sky channel */
                        if (data[6] != 0xd1) {
                          station = 1;
                          strcpy((char *) display + 13,
                                 channel_id[data[6] & 0x0f]);
                        }
                      } else if ((check >> 2) == 2) {
                        station = 1;
                        strcpy((char *) display + 13, "ADULTCHANNEL");
                      } else if ((check >> 3) == 3) {
                        station = 1;
                        strcpy((char *) display + 13, "    JSTV    ");
                      } else {
                        station = 1;
                        if ((data[0] & 0xe0) != 0xc0)
                          strcpy((char *) display + 13, "NO KEY FITS!");
                      }
                  } else if (station == 1) {
                    /* now remove on-screen display */
                    display[0] = 0x80;
                    station = 2;
                  }
                  /* PC display message */
                  if ((check >> 2) == 1) {
                    /* it's a Sky channel */
                    if (data[6] != 0xd1)
                      current_name = channel_name[data[6] & 0x0f];
                  } else if ((check >> 2) == 2)
                    current_name = "Adult Channel";
                  else if ((check >> 2) == 3)
                    current_name = "JSTV";
                  else if ((data[0] & 0xe0) != 0xc0)
                    current_name = "???, no key fits";
                  if (!debug)
                    printf("\r\t\t\t\t\t\rDecrypting %s (%02x %02x %02x)",
                           current_name, data[0], data[1], data[6]);
                }
                if (check & 2)
                  printf("\nChecksum wrong !!!\n");
              }
            }
            break;
          case 0x76:
            printf("\rAuthorize button pressed.\t\t\t\t\n");
            break;
          case 0x78:
            if (vcl_out && !suspended) {
              fwrite(msg + 28, 1, 4, vcl_out); /* final 8 bytes of 74h cmd */
              seed[7] &= 0x0f;   /* only 60-bit are needed by decoder */
              fwrite(seed, 1, 8, vcl_out);
              vcl_count++;
            }
            break;
          default: break;
        }
        hc = 0;
        received = -1;
      } else {
        /* wait in passive mode for SW1 and SW2 */
        if (receive(&proc_byte, 1)) {
          if (sw2_next) {
            sw2_ok = 1;
            sw2_next = 0;
          } else if (((proc_byte & 0xf0) == 0x60 ||
               (proc_byte & 0xf0) == 0x90) && proc_byte != 0x60)
            sw2_next = 1;
          else {
            printf("\rUnexpected SW1 byte %02x, INS = %02x!\t\n",
                   (unsigned char) proc_byte, header[1]);
            sync_lost = 1;
          }
        }
      }  /* if (!passive || sw2_ok) else */
    }  /* if (hc < 5) else */

  }  /* while (!quit) */

  deactivate_com();
  printf("\n");

  if (vcl_out) {
    /* close VCL file */
    buffer[0] = 0;
    /* fill up file to a multiple of 1024, so that there are
     * no undeleted bytes in final DOS disk clusters. */
    for (l = 128 + vcl_count * 12; (l & 0x3ff) != 0; l++)
      fwrite(buffer, 1, 1, vcl_out);
    /* write number of seeds into header */
    if (fseek(vcl_out, 4, SEEK_SET)) {
      perror("Sorry, can't fseek() in VCL file");
      exit(1);
    }
    putc((vcl_count >> 24) & 0xff, vcl_out);
    putc((vcl_count >> 16) & 0xff, vcl_out);
    putc((vcl_count >>  8) & 0xff, vcl_out);
    putc(vcl_count & 0xff, vcl_out);
    if (fclose(vcl_out)) {
      perror("Sorry, can't close VCL file!");
      exit(1);
    }
  }

  return 0;
}
