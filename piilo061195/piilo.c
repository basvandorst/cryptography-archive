/*----------------------------------------------------------------------

PIILO is a steganography program that hides files in grayscale PGM 
images. A secret key is used both for encrypting the file and for 
selecting the pixels whose least significant bits are used to hide 
the data. 

Author: Tuomas Aura <Tuomas.Aura@hut.fi> 

The algorithm is described in 'ftp://saturn.hut.fi/pub/aaura/ste1195.ps'.

You may use this program or parts of it for any purpose, but it is 
probably not a good idea. The code looks quite awful and may be byggy.

Half of the code is copied from the freeware program pgmstealth.c by 
Timo Rinne, 'ftp://ftp.funet.fi/pub/crypt/steganography/PGM.stealth.c.gz'.
The copyright message in the original source file was:
    Copyright 1994, Timo Rinne <tri@cirion.fi> and Cirion oy.

----------------------------------------------------------------------

INSTALLATION

You need the Crypto++ library, available for example from
'ftp://ftp.funet.fi/pub/crypt/cryptography/libs/crypto10.zip'.

piilo.c is written in C, but the library is in C++, so you need
a C++ compiler. Compile piilo.c and link it with cryptlib.o, misc.o,
shs.o and piilo.o of the Crypto++ library. There is a minimalistic 
Makefile.

----------------------------------------------------------------------

USAGE:
    piilo -c [-n] [-w] [-v] [-k key] [-f msgfile] pgmfile1 pgmfile2
    piilo -d [-v] [-k key] pgmfile

OPTIONS:
       -c          Encrypt and hide a file into the PGM image.  

       -d          Extract and decrypt a file hidden in PGM image.

       -s          Scale grayscales to fill whole scale
                   from 0 to 255. (Only with -c).

       -w          Paint white the pixels that would contain the 
                   data. No data is actually hidden.

       -v          Set verbose mode.

       -k key      Use the key 'key' for IDEA encryption and 
                   selecting pseudo random pixel to hide the data.
                   The key may be up to 16 characters long.

       -f msgfile  Read the data from file 'msgfile' instead of 
                   standard input. The same file name will be used 
                   when extracting the data.

----------------------------------------------------------------------

EXAMPLES:

   Encrypt and hide the file msg.txt:

   $ piilo -c -k 'very secret key!' -f msg.txt pic1.pgm pic2.pgm

   Extract the hidden data into file msg.txt:

   $ piilo -d -k 'very secret key!' pic2.pgm

   Encrypt and hide console input and send the new PGM file to the 
   standard output: 

   $ piilo -c -k 2jhw777x65ffffDQwert pic1.pgm

   In case you have pbmplus (netpbm) package installed you
   can hide information into the gif like this:

   $ giftoppm < pic1.gif | ppmtopgm > temp.pgm
   $ piilo -c -k qwerty -f msg.txt temp.pgm | ppmtogif > pic2.gif

   Extraction goes like this:

   $ giftoppm < pic1.gif | ppmtopgm > temp.pgm
   $ piilo -d -k qwerty temp.pgm

  ----------------------------------------------------------------------
*/


/* #define OUTPUT_P2 1 /* Uncomment this if you want PGMs out in ASCII only. */

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include "idea.h"
#include "shs.h"
#include <malloc.h>
#include <time.h>

#define PIILOVERSION 0
#define MAXFILENAMESIZE 100
#define FILENAMESTART 7
/* IDEA only uses 16 first characters of the key, hiding uses all 20 */
#define DEFAULTKEY "12345678901234567890"
#define KEYLENGTH 20
/* Max total length of key and printed x or y coordinate */
#define CONCATENATIONLENGTH 200
#define RUBISHBLOCK "1234567890123456"
#define IDEABLOCKLENGTH 16
/* Magic number for the hidden message */
#define PIILOMAGIC "Pi"
#define DIRECTORYSEPARATOR '/'

#define USAGE(n) \
do { fprintf(stderr, "USAGE: %s -c [-s] [-w] [-v] [-k key] [-f msgfile] pgmfile1 pgmfile2\n       %s -d [-v] [-k key] pgmfile\n",exe,exe); \
       if(n) { exit(n); } } while(0)

     /* Select a pixel in the picture using a secret key algorithm */
     int select_pixel (int i, int x, int y, char *key)
{
  unsigned int L, R;
  char concatenation[CONCATENATIONLENGTH];
  unsigned int hash;
  unsigned char shs_code[KEYLENGTH+1];
  SHS *aSHS;
  int k;

  L = i / x;       
  R = i % x;

  for (k=0; k<CONCATENATIONLENGTH; k++) concatenation[k] = 0;
  strncpy(concatenation,key,KEYLENGTH); 
  sprintf(&concatenation[KEYLENGTH],"%d",R);
  aSHS = new SHS();
  aSHS->Update((unsigned char *)concatenation,CONCATENATIONLENGTH);
  aSHS->Final(shs_code);
  delete aSHS;
  hash = * (unsigned int *) shs_code;
  L = ( L + hash ) % y;

  for (k=0; k<CONCATENATIONLENGTH; k++) concatenation[k] = 0;
  strncpy(concatenation,key,KEYLENGTH); 
  sprintf(&concatenation[KEYLENGTH],"%d",L);
  aSHS = new SHS();
  aSHS->Update((unsigned char *)concatenation,CONCATENATIONLENGTH);
  aSHS->Final(shs_code);
  delete aSHS;
  hash = * (unsigned int *) shs_code;
  R = ( R + hash ) % x;


  for (k=0; k<CONCATENATIONLENGTH; k++) concatenation[k] = 0;
  strncpy(concatenation,key,KEYLENGTH); 
  sprintf(&concatenation[KEYLENGTH],"%d",R);
  aSHS = new SHS();
  aSHS->Update((unsigned char *)concatenation,CONCATENATIONLENGTH);
  aSHS->Final(shs_code);
  delete aSHS;
  hash = * (unsigned int *) shs_code;
  L = ( L + hash ) % y;
  
  return L*x+R;
} 




/*
 * Skip whitespace and #-commented linetails and
 * read positive integer.  Integer is returned.
 * Return value is -1 on EOF, -2 on error or -3
 * on non integer input garbage.
 */
int ReadInt(FILE *f)
{
  char buf[32];
  int c, r, i;

 restart:;
  while((c = fgetc(f)) != EOF && isspace(c))
    /*NOTHING*/;
  if(c == EOF)
    goto readint_abend;
  if(c == '#') {
    while((c = fgetc(f)) != EOF && (c != '\n'))
      /*NOTHING*/;
    if(c == EOF)
      goto readint_abend;
    else
      goto restart;
  }
  for(i = 0; (i < 31 && c != EOF && isdigit(c)); (i++, c = fgetc(f))) {
    buf[i] = c;
    buf[i + 1] = 0;
  }
  if(i > 0 && i < 31) {
    r = atoi(buf);
    if(r >= 0)
      return r;
  }
 readint_abend:;
  if(ferror(f)) {
    return -2;
  } else if(feof(f)) {
    return -1;
  } else {
    return -3;
  }
}
/*
 * Read PGM pixmap in either ascii (P2) or binary (P5) format.
 */
unsigned short *ReadPGM(
		       FILE *pgm_f,
		       char *exe,
		       int *x,
		       int *y,
		       int *c,
		       int *max,
		       int *min)
{
  unsigned short *pgm;
  char buf[4];
  int i, pix, raw;

  if(2 != fread((void *)buf, 1, 2, pgm_f)) {
    return((unsigned short *)0);
  }
  if(buf[0] == 'P' && buf[1] == '2') {
    raw = 0;
  } else if(buf[0] == 'P' && buf[1] == '5') {
    raw = 1;
  } else {
    fprintf(stderr, "%s: Bad magic.\n",exe);
    return((unsigned short *)0);
  }
  if((*x = ReadInt(pgm_f)) < 1) {
    return((unsigned short *)0);
  }
  if((*y = ReadInt(pgm_f)) < 1) {
    return((unsigned short *)0);
  }
  if(((*c = ReadInt(pgm_f)) < 1) || (*c > 255)) {
    return((unsigned short *)0);
  }
  if(!(pgm =
       (unsigned short *)calloc(sizeof(unsigned short), (*x) * (*y) + 3)))
    {
      return((unsigned short *)0);
    }
  pgm[0] = *x;
  pgm[1] = *y;
  pgm[2] = *c;
  if(raw) {
    for((i = 0, *max = 0, *min = (*c)); i < ((*x) * (*y)); i++) {
      if(((pix = fgetc(pgm_f)) == EOF) || (pix > (*c))) {
	free(pgm);
	fprintf(stderr, "%s: PGM raw file truncated.\n",exe);
	return((unsigned short *)0);
      } else {
	pgm[i + 3] = pix;
	*max = (pix > (*max)) ? pix : (*max);
	*min = (pix < (*min)) ? pix : (*min);
      }
    }
  } else {
    for((i = 0, *max = 0, *min = (*c)); i < ((*x) * (*y)); i++) {
      if(((pix = ReadInt(pgm_f)) < 0) || (pix > (*c))) {
	free(pgm);
	if(pix == -1)
	  fprintf(stderr, "%s: PGM ascii file truncated.\n",exe);
	else
	  fprintf(stderr, "%s: PGM ascii file contains garbage.\n",exe);
	return((unsigned short *)0);
      } else {
	pgm[i + 3] = pix;
	*max = (pix > (*max)) ? pix : (*max);
	*min = (pix < (*min)) ? pix : (*min);
      }
    }
  }
  return pgm;
}

main(int argc, char **argv)
{
  int x, y, c, max, min, delta;
  FILE *pgm_f, *new_pgm_f, *msg_f;
  char *exe, *msg_f_name, *save_f_name;
  unsigned short *pgm;
  extern char *optarg;
  extern int optind;
  int cry, dec, spread, white, verbose, key_len, name_len;
  long i, j, k, msgbsz, pixelno, msg_len;
  unsigned int byte, bit;
  unsigned char ch;
  char secret_key[KEYLENGTH+1] = {0};
  SHS aSHS;
  IDEA *aIDEADe;
  IDEA *aIDEAEn;
  unsigned char shs_code[KEYLENGTH+1] = {0};
  char *msg;
  long msg_byte;
  long point;

  exe = argv[0];
  cry = dec = 0;
  spread = 0;
  white = 0;
  verbose = 0;
  strcpy(secret_key,DEFAULTKEY);
  msg_f_name = "";
  while((c = getopt(argc, argv, "cdswvk:f:")) != -1)
    switch(c) {
    case 'c':
      if(dec) {
	fprintf(stderr, "%s: Only -c or -d is allowed.\n",exe);
	USAGE(1);
      }
      cry = 1;
      break;
    case 'd':
      if(cry) {
	fprintf(stderr, "%s: Only -c or -d is allowed.\n",exe);
	USAGE(1);
      }
      dec = 1;
      break;
    case 's':
      spread = 1;
      break;
    case 'w':
      white = 1;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'k':
      key_len = strlen(optarg);
      strncpy(secret_key,optarg,KEYLENGTH);
      break;
    case 'f':
      msg_f_name = optarg;
      break;
    default:
      USAGE(1);
    }

  if((!(dec || cry))) {
    fprintf(stderr, "%s: Function has to be defined.\n",exe);
    USAGE(1);
  }
 
  if(white) {
    if (cry) 
      fprintf(stderr, "%s: Only painting the selected pixels white.\n",exe);
    else
      fprintf(stderr, "%s: Warning: option -w has no effect.\n",exe);
  }
  if(dec && strcmp("",msg_f_name)) {
    fprintf(stderr, "%s: Warning: option -f has no effect.\n",exe);
  }

  if (verbose) {
    fprintf (stderr, "%s: Secret key \"%s\", ",exe,secret_key);
    for (i=0; i<KEYLENGTH; i++) fprintf(stderr, "%d ",secret_key[i]);
    fprintf (stderr, "\n");
  if(strcmp(secret_key,DEFAULTKEY) == 0) 
    fprintf(stderr, "%s: No secret key given. Using default key.\n",exe);
  else
    if(verbose && dec && key_len != KEYLENGTH) {
      fprintf(stderr, 
	      "%s: Key length %d. The key may be up to %d characters long.\n",
	      exe,key_len,KEYLENGTH);
    }
  }
  argc = argc - (optind - 1);
  argv = argv + (optind - 1);
  if(cry && (argc < 2 || argc > 3)) {
    fprintf(stderr, "%s: Wrong argument count.\n",exe);    
    USAGE(1);
  }
  if(dec && argc != 2) {
    fprintf(stderr, "%s: Wrong argument count.\n",exe);    
    USAGE(1);
  }

  if(!(pgm_f = fopen(argv[1], "r"))) {
    fprintf(stderr, "%s: Cannot open pgm file \"%s\".\n",exe,argv[1]);
    exit(1);
  }
  if(!(pgm = ReadPGM(pgm_f, exe, &x, &y, &c, &max, &min))) {
    fprintf(stderr, "%s: Error reading pgm file \"%s\".\n",exe,argv[1]);
    exit(1);
  }
  msgbsz = ((x * y) / 8) - 1;    
  if (verbose) fprintf(stderr,"%s: Image size: x=%d, y=%d.\n",exe,x,y);
  
  if(!(msg = (char *)calloc(sizeof(char),x*y/8+MAXFILENAMESIZE+200))) {
    fprintf(stderr, "%s: Cannot allocate memory.");   
    exit(1);
  }

  if(cry) {
    /* crypt */

    if (strlen(msg_f_name) == 0)
      msg_f = stdin;
    else {
      if(!(msg_f = fopen(msg_f_name, "r"))) {
	fprintf(stderr, "%s: Cannot open message file \"%s\".\n",
		exe,msg_f_name);
	USAGE(1);
      }
    } 

    delta = max - min;
    if(!delta) {
      fprintf(stderr, "%s: PGM picture contains only one color.\n",exe);
      exit(1);
    }
    /*
     * We'll scale the grayscale values to fit in [0, 255].
     * In the same loop the msg-bits are randomized.
     */
    if(spread) {
      if(c != 255 || min != 0 || max != 255) {
	for(i = 0; i < (x * y); i++) {
	  j = pgm[i + 3] - min;
	  j = (j * 255) / delta;
	  if((j < 0) || j > 255) {
	    fprintf(stderr, "%s: Internal error.\n",exe);
	    exit(1);
	  }
	  pgm[i + 3] = ((unsigned char)j);
	}
      }
    } else {
      if(c != 255) {
	fprintf(stderr, "%s: PGM file should contain 255 grayscales if no scaling is allowed.\n",exe);
	exit(1);
      } 
    }
    /*
     * Maximum gray-value is now 255 and all 8 bits are utilized.
     */
    pgm[2] = 255;
  }

  
  /* SHS, discarding least significant bits */
  for (i=3; i<x*y; i++) {
    byte = pgm[i] & 254;
    ch = (unsigned char) byte;
    aSHS.Update(&ch,1);
    aSHS.Final(shs_code);
  }
  /* Same secret key for different images should select different pixels. */
  for (i=0; i<KEYLENGTH; i++)
    secret_key[i] ^= shs_code[i];
  if (verbose) {
    fprintf(stderr,"%s: Image SHS: ",exe);
    for (i=0; i<KEYLENGTH; i++)
      fprintf(stderr,"%d ",shs_code[i]);
    fprintf(stderr,"\n");
    fprintf(stderr,"%s: Hiding/extraction key: ",exe);
    for (i=0; i<KEYLENGTH; i++)
      fprintf(stderr,"%d ", secret_key[i]);
    fprintf(stderr,"\n");
  }


  if (cry) {
    /* Message header:
       Bytes                     Use
       2                         magic number "1s"
       1                         0
       4                         message length 
       strlen(msg_f_name) + 1    message file name + 0
    */
    save_f_name = msg_f_name;
    name_len = strlen(msg_f_name);
    for(i=name_len; i >= 0; i--) 
      if (msg_f_name[i] == DIRECTORYSEPARATOR) {
	save_f_name = &msg_f_name[i+1];
	break;
      }
    if(name_len > MAXFILENAMESIZE) {
      fprintf(stderr, 
	      "%s: Filename too long.\n",exe);
    }
    strcpy(msg,PIILOMAGIC);
    msg[2] = PIILOVERSION;
    strcpy(&msg[FILENAMESTART],save_f_name);
    if (verbose) {
      fprintf(stderr,"%s: Message file path: \"%s\"\n",exe,msg_f_name);
      fprintf(stderr,"%s: Hiding file name: \"%s\"\n",exe,&msg[FILENAMESTART]);
    }
    i = FILENAMESTART + name_len + 1;

    /* Read the message file */
    while((msg_byte = fgetc(msg_f)) != EOF && i < msgbsz) { 
      msg[i] = msg_byte; 
      i++;
    }
    if(i == msgbsz) {
      fprintf(stderr, "%s: Message too large. This image can take up to %d bytes including header info. Only a small fraction of that can go undetected.\n"
	      ,exe,msgbsz);
      exit(1);
    }
    
    msg_len = i;
    for (j=3; j<7; j++) {
      *((unsigned char *) &msg[j]) = msg_len % 256;
      msg_len /= 256;
    }
    if (verbose) 
      fprintf(stderr,"%s: User message length %d bytes, with header info %d bytes.\n",
	      exe,i-(FILENAMESTART + name_len + 1),i);
    msg_len = (i % IDEABLOCKLENGTH ? 
	       i + IDEABLOCKLENGTH - (i % IDEABLOCKLENGTH) : i);
    fprintf(stderr, "%s: Encrypted message is %d bytes. Using %.1f %% of the pixels.\n"
	    ,exe,msg_len,(float)msg_len/(float)msgbsz*100);

    if (verbose) {
      fprintf(stderr,"%s: First unencrypted block: ",exe);
      for (k=0; k<IDEABLOCKLENGTH; k++)
	fprintf(stderr,"%d ", msg[k]);
      fprintf(stderr,"\n");
    }
    /* Encrypt message */
    aIDEAEn = new IDEAEncryption((unsigned char *) secret_key);
    for (i=0; i<msg_len; i+=IDEABLOCKLENGTH)
      aIDEAEn->ProcessBlock((unsigned char *) &msg[i]);

    if (verbose) {
      fprintf(stderr,"%s: First encrypted block: ",exe);
      for (k=0; k<IDEABLOCKLENGTH; k++)
	fprintf(stderr,"%d ", msg[k]);
      fprintf(stderr,"\n");
    }

    i = 0;
    while(i < msg_len) {
      byte = msg[i];
      for (j=0; j<8; j++) {
	pixelno = select_pixel(8*i+j,x,y,secret_key);
	bit = (byte >> j) & ((unsigned char)1);
	if (!white) {
	  pgm[pixelno + 3] &= 254;
	  pgm[pixelno + 3] |= bit;
	} else
	  pgm[pixelno + 3] |= 255;
      }
      i++;
    }

    if(argc == 3) {
      if(!(new_pgm_f = fopen(argv[2], "w"))) {
	fprintf(stderr, "%s: Cannot create new pgm file \"%s\".\n",exe,argv[2]);
	USAGE(1);
      }
    } else 
      new_pgm_f = stdout;

#ifdef OUTPUT_P2
    /*
      Print out the pgm. Format is PGM (P2-ASCII).
      This is not the very best, but who cares.
      Picture is anyway usually convefted to gifs.
    */
    fprintf(new_pgm_f,"P2\n%u %u\n%u\n", pgm[0], pgm[1], pgm[2]);
    for(i = 0; i < (x * y); i++) {
      fprintf(new_pgm_f,"%u\n", pgm[i + 3]);
    }
#else
    /*
      Print out the pgm. Format is PGM (P5-BINARY).
    */
    fprintf(new_pgm_f,"P5\n%u %u\n%u\n", pgm[0], pgm[1], pgm[2]);
    for(i = 0; i < (x * y); i++) {
      fputc((pgm[i + 3]), new_pgm_f);
    }
#endif
    exit(0);


  } else {
    
    
    /* decrypt */
    
    /* Extract header info */
    i = 0;
    while(i < (FILENAMESTART+1+MAXFILENAMESIZE+IDEABLOCKLENGTH)) {
      byte = 0;
      for (k=0; k<8; k++) {
	pixelno = select_pixel(8*i+k,x,y,secret_key);
	byte |= ( pgm[pixelno + 3] & 1 ) << k;
      }
      msg[i] = byte;
      i++;
    }

    if (verbose) {
      fprintf(stderr,"%s: First undecrypted block: ",exe);
      for (k=0; k<IDEABLOCKLENGTH; k++)
	fprintf(stderr,"%d ", msg[k]);
      fprintf(stderr,"\n");
    }

    /* Decrypt header info */
    aIDEADe = new IDEADecryption((unsigned char *) secret_key);
    for (j=0; j<(FILENAMESTART+1+MAXFILENAMESIZE+IDEABLOCKLENGTH); 
	 j+=IDEABLOCKLENGTH)
      aIDEADe->ProcessBlock((unsigned char *) &msg[j]);

    if (verbose) {
      fprintf(stderr,"%s: First decrypted block: ",exe);
      for (k=0; k<IDEABLOCKLENGTH; k++)
	fprintf(stderr,"%d ", msg[k]);
      fprintf(stderr,"\n");
    }

    /* Process header info */
    if (verbose) fprintf(stderr,"%s: Magic number found: \"%c%c\"\n",
			 exe,msg[0],msg[1]);
    if (strncmp(msg,PIILOMAGIC,2) != 0) {
      fprintf(stderr, "%s: No hidden message or wrong key!\n",exe);
//      exit(0);
    }
    msg_len = 0;
    for (k=6; k>=3; k--) {
      msg_len *= 256;
      msg_len = msg_len + *((unsigned char *) &msg[k]);
    }
    if (verbose) 
      fprintf(stderr,"%s: Message length with header info is %d bytes. (%d,%d,%d,%d)\n",
	      exe,msg_len,msg[3],msg[4],msg[5],msg[6]);
    name_len = strlen(&msg[FILENAMESTART]);
    if (verbose && name_len != 0) 

      fprintf(stderr,"%s: Message file name: \"%s\"\n",
			 exe,&msg[FILENAMESTART]);
    if (verbose && name_len == 0) 
      fprintf(stderr,"%s: No message file name. Using standard output.\n");

    /* Extract rest of the message */
    while(i < FILENAMESTART+1+MAXFILENAMESIZE+msg_len+IDEABLOCKLENGTH) {
      byte = 0;
      for (k=0; k<8; k++) {
	pixelno = select_pixel(8*i+k,x,y,secret_key);
	byte |= ( pgm[pixelno + 3] & 1 ) << k;
      }
      msg[i] = byte;
      i++;
    }
    /* Decrypt rest of the message */
    aIDEADe = new IDEADecryption((unsigned char *) secret_key);
    for (; j<x*y/FILENAMESTART+1+MAXFILENAMESIZE+50; j+=IDEABLOCKLENGTH)
      aIDEADe->ProcessBlock((unsigned char *) &msg[j]);

    if (name_len == 0)
      msg_f = stdout;
    else {
      if(!(msg_f = fopen(&msg[FILENAMESTART], "w"))) {
	fprintf(stderr, "%s: Cannot create message file \"%s\".\n",
		exe,msg[FILENAMESTART]);
	exit(1);
      }
    }
    
    /* Save message */
    for (i = FILENAMESTART+1+name_len; i < msg_len; i++)
      fputc((int)msg[i], msg_f);
    
  }
}

/* EOF (piilo.c) */




