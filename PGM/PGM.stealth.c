# This is a shell archive.  Save it in a file, remove anything before
# this line, and then unpack it by entering "sh file".  Note, it may
# create directories; files and directories will be owned by you and
# have default permissions.
#
# This archive contains:
#
#       pgmstealth.c
#
echo x - pgmstealth.c
sed 's/^X//' >pgmstealth.c << 'END-of-pgmstealth.c'
X/*  -*- c -*-
X *
X * $RCSfile: pgmstealth.c,v $
X * ----------------------------------------------------------------------
X * PGMstealth.  Hide satanic messages (or any other useful information)
X * into the grayscale pixmaps.
X * ----------------------------------------------------------------------
X * Created      : Sun Feb 27 15:25:49 1994 tri
X * Last modified: Wed Mar  2 23:00:54 1994 tri
X * ----------------------------------------------------------------------
X * $Revision: 1.3 $
X * $State: Exp $
X * $Date: 1994/03/02 21:06:15 $
X * $Author: tri $
X * ----------------------------------------------------------------------
X * $Log: pgmstealth.c,v $
X * Revision 1.3  1994/03/02  21:06:15  tri
X * Minor fix.
X *
X * Revision 1.2  1994/03/02  21:01:20  tri
X * Bugfixes and some sanity check.
X *
X * Revision 1.1  1994/03/02  19:11:24  tri
X * Initial revision
X *
X * ----------------------------------------------------------------------
X * Copyright 1994, Timo Rinne <tri@cirion.fi> and Cirion oy.
X *
X * Address: Cirion oy, PO-BOX 250, 00121 HELSINKI, Finland
X *
X * Even though this code is copyrighted property of the author, it can
X * still be used for any purpose under following conditions:
X *
X *     1) This copyright notice is not removed.
X *     2) Source code follows any distribution of the software
X *        if possible.
X *     3) Copyright notice above is found in the documentation
X *        of the distributed software.
X *
X * Any express or implied warranties are disclaimed.  In no event
X * shall the author be liable for any damages caused (directly or
X * otherwise) by the use of this software.
X *
X * ----------------------------------------------------------------------
X *
X * TODO
X *
X * 1) Manual page.
X * 2) Support for GIF format (input/output).
X *
X * I do not have time to implement these changes,
X * but in case someone should do that, I'll be
X * glad to merge the changes into the `official'
X * sources.
X *
X * ----------------------------------------------------------------------
X *
X * USAGE:
X *             pgmstealth -c|d [-n] -b 1|2|4|8 pgmfile
X *
X * OPTIONS:
X *             -c      Merge (crypted) data from standard input into
X *                     the PGM pixmap.  Merged pixmap is printed to
X *                     stdout.
X *
X *             -d      Get (crypted) data hidden in PGM pixmap.
X *                     Data is written to stdout.
X *
X *             -n      Do not scale grayscales to fill whole scale
X *                     from 0 to 255. (Only with -c).
X *
X *             -b #    Use # bits from 8 for hidden message.
X *                     With 8 bits all you get is noise.
X *
X * ----------------------------------------------------------------------
X *
X * EXAMPLES:
X *
X *     # Hide crypted datafile secret.pgp into the lowest two bits
X *     # of the pixmap pussy.pgm
X *
X *     $ pgmstealth -c -b 2 pussy.pgm < secret.pgp > pussy2.pgm
X *
X *     # Get the hidden data back.  You have to be able to figure
X *     # out the length of the meaningful data, since random noise
X *     # is added to the end of the data.
X *
X *     $ pgmstealth -d -b 2 pussy2.pgm > secret.pgp
X *
X *     # Hide crypted datafile secret.pgp into the lowest two bits
X *     # of the 8-bit pixmap pussy.pgm.  Try to keep colors intact.
X *
X *     $ pgmstealth -c -b 2 -m pussy.pgm < secret.pgp > pussy2.pgm
X *
X *     # In case you have pbmplus (netpbm) package installed you
X *     # can hide information into the gif like this.
X *
X *     $ giftoppm < pussy.gif | ppmtopgm > pussy.pgm
X *     $ pgmstealth -c -b 2 pussy.pgm < secret.pgp | ppmtogif > pussy2.gif
X *
X * ----------------------------------------------------------------------
X */
X#define __PGMSTEALTH_C__ 1
X#ifndef lint
Xstatic char *RCS_id = "$Id: pgmstealth.c,v 1.3 1994/03/02 21:06:15 tri Exp $";
X#endif /* not lint */
X
X/* #define OUTPUT_P2 1 /* Uncomment this if you want PGMs out in ASCII only. */
X
X#include <stdio.h>
X#include <ctype.h>
X
X#define USAGE(n) \
X    do { fprintf(stderr, "USAGE: pgmstealth -c|d [-n] -b 1|2|4|8 pgmfile\n"); \
X        if(n) { exit(n); } } while(0)
X
X/*
X * Skip whitespace and #-commented linetails and
X * read positive integer.  Integer is returned.
X * Return value is -1 on EOF, -2 on error or -3
X * on non integer input garbage.
X */
Xint ReadInt(FILE *f)
X{
X    char buf[32];
X    int c, r, i;
X
X  restart:;
X    while((c = fgetc(f)) != EOF && isspace(c))
X       /*NOTHING*/;
X    if(c == EOF)
X       goto readint_abend;
X    if(c == '#') {
X       while((c = fgetc(f)) != EOF && (c != '\n'))
X           /*NOTHING*/;
X       if(c == EOF)
X           goto readint_abend;
X       else
X           goto restart;
X    }
X    for(i = 0; (i < 31 && c != EOF && isdigit(c)); (i++, c = fgetc(f))) {
X       buf[i] = c;
X       buf[i + 1] = 0;
X    }
X    if(i > 0 && i < 31) {
X       r = atoi(buf);
X       if(r >= 0)
X           return r;
X    }
X  readint_abend:;
X    if(ferror(f)) {
X       return -2;
X    } else if(feof(f)) {
X       return -1;
X    } else {
X       return -3;
X    }
X}
X/*
X * Read PGM pixmap in either ascii (P2) or binary (P5) format.
X */
Xunsigned short *ReadPGM(
X    FILE *pgm_f,
X    int *x,
X    int *y,
X    int *c,
X    int *max,
X    int *min)
X{
X    unsigned short *pgm;
X    char buf[4];
X    int i, pix, raw;
X
X    if(2 != fread((void *)buf, 1, 2, pgm_f)) {
X       return((unsigned short *)0);
X    }
X    if(buf[0] == 'P' && buf[1] == '2') {
X       raw = 0;
X    } else if(buf[0] == 'P' && buf[1] == '5') {
X       raw = 1;
X    } else {
X       fprintf(stderr, "Bad magic.\n");
X       return((unsigned short *)0);
X    }
X    if((*x = ReadInt(pgm_f)) < 1) {
X       return((unsigned short *)0);
X    }
X    if((*y = ReadInt(pgm_f)) < 1) {
X       return((unsigned short *)0);
X    }
X    if(((*c = ReadInt(pgm_f)) < 1) || (*c > 255)) {
X       return((unsigned short *)0);
X    }
X    if(!(pgm =
X        (unsigned short *)calloc(sizeof(unsigned short), (*x) * (*y) + 3)))
X       {
X           return((unsigned short *)0);
X       }
X    pgm[0] = *x;
X    pgm[1] = *y;
X    pgm[2] = *c;
X    if(raw) {
X       for((i = 0, *max = 0, *min = (*c)); i < ((*x) * (*y)); i++) {
X           if(((pix = fgetc(pgm_f)) == EOF) || (pix > (*c))) {
X               free(pgm);
X               fprintf(stderr, "PGM raw file truncated.\n");
X               return((unsigned short *)0);
X           } else {
X               pgm[i + 3] = pix;
X               *max = (pix > (*max)) ? pix : (*max);
X               *min = (pix < (*min)) ? pix : (*min);
X           }
X       }
X    } else {
X       for((i = 0, *max = 0, *min = (*c)); i < ((*x) * (*y)); i++) {
X           if(((pix = ReadInt(pgm_f)) < 0) || (pix > (*c))) {
X               free(pgm);
X               if(pix == -1)
X                   fprintf(stderr, "PGM ascii file truncated.\n");
X               else
X                   fprintf(stderr, "PGM ascii file contains garbage.\n");
X               return((unsigned short *)0);
X           } else {
X               pgm[i + 3] = pix;
X               *max = (pix > (*max)) ? pix : (*max);
X               *min = (pix < (*min)) ? pix : (*min);
X           }
X       }
X    }
X    return pgm;
X}
X
Xmain(int argc, char **argv)
X{
X    int x, y, c, npix, i, j, max, min, delta, bits, msgbsz;
X    FILE *pgm_f;
X    unsigned short *pgm, *msg, *msgbuf, mask;
X    extern char *optarg;
X    extern int optind;
X    int cry, dec, spread;
X
X    cry = dec = bits = 0;
X    spread = 1;
X    while((c = getopt(argc, argv, "nb:cd")) != -1)
X       switch(c) {
X       case 'n':
X           spread = 0;
X           break;
X       case 'c':
X           if(dec || cry) {
X               fprintf(stderr, "Only -c or -d is allowed.\n");
X               USAGE(1);
X           }
X           cry = 1;
X           break;
X       case 'd':
X           if(dec || cry) {
X               USAGE(1);
X           }
X           dec = 1;
X           break;
X       case 'b':
X           bits = atoi(optarg);
X           if((bits != 1) && (bits != 2) && (bits != 4) && (bits != 8)) {
X               fprintf(stderr, "Illegal bitcount %s.\n", optarg);
X               USAGE(1);
X           }
X           break;
X       default:
X           USAGE(1);
X       }
X    if((!(dec || cry)) || (!(bits))) {
X       fprintf(stderr, "Function ad bitcount has to be defined.\n");
X       USAGE(1);
X    }
X    argc = argc - (optind - 1);
X    argv = argv + (optind - 1);
X    if(argc < 2) {
X       USAGE(1);
X    } else {
X       if(!(pgm_f = fopen(argv[1], "r"))) {
X           fprintf(stderr, "Cannot open pgm file \"%s\".\n", argv[1]);
X           USAGE(1);
X       }
X       argc--;
X       argv++;
X    }
X    if(!(pgm = ReadPGM(pgm_f, &x, &y, &c, &max, &min))) {
X       exit(1);
X    }
X    srandom(getpid() ^ time((long *)0));
X    if(cry)
X       if(!(msg = (unsigned short *)calloc(sizeof(unsigned short), x * y))) {
X           fprintf(stderr, "Out of memory.\n");
X           exit(2);
X       }
X    switch(bits) {
X    case 1:
X       mask = ~((unsigned short)1);
X       msgbsz = ((x * y) / 8) - 1;
X       break;
X    case 2:
X       mask = ~((unsigned short)3);
X       msgbsz = ((x * y) / 4) - 1;
X       break;
X    case 4:
X       mask = ~((unsigned short)15);
X       msgbsz = ((x * y) / 2) - 1;
X       break;
X    case 8:
X       mask = ~((unsigned short)255);
X       msgbsz = (x * y) - 2;
X       break;
X    };
X    if(dec)
X       if(!(msgbuf =
X            (unsigned short *)calloc(sizeof(unsigned short), msgbsz)))
X           {
X               fprintf(stderr, "Out of memory.\n");
X               exit(2);
X           }
X    if(cry) {
X       /* crypt */
X       delta = max - min;
X       if(!delta) {
X           fprintf(stderr, "PGM picture contains only one color.\n");
X           exit(1);
X       }
X       /*
X        * We'll scale the grayscale values to fit in [0, 255].
X        * In the same loop the msg-bits are randomized.
X        */
X       if(spread) {
X           if(c != 255 || min != 0 || max != 255) {
X               for(i = 0; i < (x * y); i++) {
X                   j = pgm[i + 3] - min;
X                   j = (j * 255) / delta;
X                   if((j < 0) || j > 255) {
X                       fprintf(stderr, "Internal error.\n");
X                       exit(1);
X                   }
X                   pgm[i + 3] = ((unsigned short)j) & mask;
X                   msg[i] = (~mask) & ((unsigned short)random());
X               }
X           }
X       } else {
X           if(c != 255) {
X               fprintf(stderr, "PGM file should contain 255 grayscales if no sc
aling is allowed.\n");
X               USAGE(1);
X           } else {
X               for(i = 0; i < (x * y); i++) {
X                   pgm[i + 3] = pgm[i + 3] & mask;
X                   msg[i] = (~mask) & ((unsigned short)random());
X               }
X           }
X       }
X       /*
X        * Maximum gray-value is now 255 and all 8 bits are utilized.
X        */
X       pgm[2] = 255;
X       /*
X        * Now we'll read the message from stdin and encode it to the
X        * lower bits of the picture.
X        */
X       switch(bits) {
X       case 1:
X           for(i = 0; i <= msgbsz; i++) {
X               if(i == msgbsz) {
X                   fprintf(stderr, "Message too large.\n");
X                   exit(1);
X               }
X               if((j = fgetc(stdin)) != EOF) {
X                   msg[i * 8] =
X                       ((unsigned short)j) & ((unsigned short)1);
X                   msg[(i * 8) + 1] =
X                       (((unsigned short)j) >> 1) & ((unsigned short)1);
X                   msg[(i * 8) + 2] =
X                       (((unsigned short)j) >> 2) & ((unsigned short)1);
X                   msg[(i * 8) + 3] =
X                       (((unsigned short)j) >> 3) & ((unsigned short)1);
X                   msg[(i * 8) + 4] =
X                       (((unsigned short)j) >> 4) & ((unsigned short)1);
X                   msg[(i * 8) + 5] =
X                       (((unsigned short)j) >> 5) & ((unsigned short)1);
X                   msg[(i * 8) + 6] =
X                       (((unsigned short)j) >> 6) & ((unsigned short)1);
X                   msg[(i * 8) + 7] =
X                       (((unsigned short)j) >> 7) & ((unsigned short)1);
X               } else {
X                   goto out_of_here;
X               }
X           }
X           break;
X       case 2:
X           for(i = 0; i <= msgbsz; i++) {
X               if(i == msgbsz) {
X                   fprintf(stderr, "Message too large.\n");
X                   exit(1);
X               }
X               if((j = fgetc(stdin)) != EOF) {
X                   msg[i * 4] =
X                       ((unsigned short)j) & ((unsigned short)3);
X                   msg[(i * 4) + 1] =
X                       (((unsigned short)j) >> 2) & ((unsigned short)3);
X                   msg[(i * 4) + 2] =
X                       (((unsigned short)j) >> 4) & ((unsigned short)3);
X                   msg[(i * 4) + 3] =
X                       (((unsigned short)j) >> 6) & ((unsigned short)3);
X               } else {
X                   goto out_of_here;
X               }
X           }
X           break;
X       case 4:
X           for(i = 0; i <= msgbsz; i++) {
X               if(i == msgbsz) {
X                   fprintf(stderr, "Message too large.\n");
X                   exit(1);
X               }
X               if((j = fgetc(stdin)) != EOF) {
X                   msg[i * 2] =
X                       ((unsigned short)j) & ((unsigned short)15);
X                   msg[(i * 2) + 1] =
X                       (((unsigned short)j) >> 4) & ((unsigned short)15);
X               } else {
X                   goto out_of_here;
X               }
X           }
X           break;
X       case 8:
X           for(i = 0; i <= msgbsz; i++) {
X               if(i == msgbsz) {
X                   fprintf(stderr, "Message too large.\n");
X                   exit(1);
X               }
X               if((j = fgetc(stdin)) != EOF) {
X                   msg[i] =
X                       ((unsigned short)j);
X               } else {
X                   goto out_of_here;
X               }
X           }
X           break;
X       }
X      out_of_here:;
X       switch(bits) {
X       case 1:
X           msg[(x * y) - 2] &= (~((unsigned short)1));
X           msg[(x * y) - 1] &= (~((unsigned short)1));
X           break;
X       case 2:
X           msg[(x * y) - 2] &= (~((unsigned short)1));
X           msg[(x * y) - 1] |= ((unsigned short)1);
X           break;
X       case 4:
X           msg[(x * y) - 2] |= ((unsigned short)1);
X           msg[(x * y) - 1] &= (~((unsigned short)1));
X           break;
X       case 8:
X           msg[(x * y) - 2] |= ((unsigned short)1);
X           msg[(x * y) - 1] |= ((unsigned short)1);
X           break;
X       }
X#ifdef OUTPUT_P2
X       /*
X        * Print out the pgm. Format is PGM (P2-ASCII).
X        * This is not the very best, but who cares.
X        * Picture is anyway usually convefted to gifs.
X        */
X       printf("P2\n%u %u\n%u\n", pgm[0], pgm[1], pgm[2]);
X       for(i = 0; i < (x * y); i++) {
X           printf("%u\n", pgm[i + 3] | msg[i]);
X       }
X#else
X       /*
X        * Print out the pgm. Format is PGM (P5-BINARY).
X        * This is not the very best, but who cares.
X        * Picture is anyway usually convefted to gifs.
X        */
X       printf("P5\n%u %u\n%u\n", pgm[0], pgm[1], pgm[2]);
X       for(i = 0; i < (x * y); i++) {
X           fputc((pgm[i + 3] | msg[i]), stdout);
X       }
X#endif
X       exit(0);
X    } else {
X       /* decrypt */
X       int b = (((pgm[(x * y) + 1]) & 1) * 2) + ((pgm[(x * y) + 2]) & 1);
X       if(b == 0) b = 1;
X       else if(b == 1) b = 2;
X       else if(b == 2) b = 4;
X       else if(b == 3) b = 8;
X       if(b != bits) {
X           fprintf(stderr, "Wrong bitcount %d. (It may be %d.)\n", bits, b);
X           USAGE(1);
X       }
X       switch(bits) {
X       case 1:
X           for(i = 0; i < msgbsz; i++) {
X               msgbuf[i] =
X                   ((pgm[(8 * i) + 3]) & ((unsigned short)1)) |
X                   (((pgm[(8 * i) + 4]) & ((unsigned short)1)) << 1) |
X                   (((pgm[(8 * i) + 5]) & ((unsigned short)1)) << 2) |
X                   (((pgm[(8 * i) + 6]) & ((unsigned short)1)) << 3) |
X                   (((pgm[(8 * i) + 7]) & ((unsigned short)1)) << 4) |
X                   (((pgm[(8 * i) + 8]) & ((unsigned short)1)) << 5) |
X                   (((pgm[(8 * i) + 9]) & ((unsigned short)1)) << 6) |
X                   (((pgm[(8 * i) + 10]) & ((unsigned short)1)) << 7);
X           }
X           break;
X       case 2:
X           for(i = 0; i < msgbsz; i++) {
X               msgbuf[i] =
X                   ((pgm[(4 * i) + 3]) & ((unsigned short)3)) |
X                   (((pgm[(4 * i) + 4]) & ((unsigned short)3)) << 2) |
X                   (((pgm[(4 * i) + 5]) & ((unsigned short)3)) << 4) |
X                   (((pgm[(4 * i) + 6]) & ((unsigned short)3)) << 6);
X           }
X           break;
X       case 4:
X           for(i = 0; i < msgbsz; i++) {
X               msgbuf[i] =
X                   ((pgm[(2 * i) + 3]) & ((unsigned short)15)) |
X                   (((pgm[(2 * i) + 4]) & ((unsigned short)15)) << 4);
X           }
X           break;
X       case 8:
X           for(i = 0; i < msgbsz; i++) {
X               msgbuf[i] = pgm[i + 3];
X           }
X           break;
X       }
X       /* Dump out the data. */
X       for(i = 0; i < msgbsz; i++) {
X           fputc((int)msgbuf[i], stdout);
X       }
X       exit(0);
X    }
X}
X
X/* EOF (pstealth.c) */
END-of-pgmstealth.c
exit
