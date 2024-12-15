/***********************************************************************/
/* (c) Copyright 1989, 1990, Alan Finlay.      Tangle, Version 3.4 .	*/
/* This program may be freely distributed but may not be sold or	 */
/* marketed in any form without the permission and written consent of	  */
/* the author.	I retain all copyrights to this program, in either the	  */
/* original or modified forms, and no violation, deletion, or change of   */
/* the copyright notice is allowed.  I will accept no responsibility for  */
/* any loss, damage, or compromised data caused directly or indirectly by */
/* this program.  It is released on an "as is" basis with no warranty	 */
/* as to its being fit or suitable for encrypting any form of data.	*/
/***********************************************************************/
#include <stdio.h>

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <stdlib.h>
#include <dos.h>
#include <sys\stat.h>
#endif

#define ENCRYPT 0	 /* Choose tangle (1) or untangle (0) */
#define DEBUG 0	 /* Show page after each shuffle if non zero */
#define TRUE -1
#define FALSE 0
#define LIMIT 100	 /* Maximum block size is LIMIT*LIMIT */
#define SECURE 10	 /* The number of block transformations */
#define MINB 450	 /* Minimum block size - insecure if too small */

typedef unsigned char line[LIMIT];

char copyright[40] = "(c) copyright 1989,1990, Alan Finlay";

unpat(page,wide,high) /* Simple substitution to eliminate simple patterns */
   line page[LIMIT];  /* [width,height] */
   int wide,high;
   {
   int i,j,k;
   k = 0;
   for (i=0;i<wide;i++) for (j=0;j<high;j++) {
      k = (k+7)%256;
      page[i][j] = page[i][j] ^ k;
      }
   }

#if ENCRYPT
shuffle(page1,code,wide,high)
   line page1[LIMIT];  /* [width,height] */
   line code;
   int wide,high;
   {
   int i,j,k,key,shift;
   line *mix1,*mix2;
   line *oldline,page2[LIMIT];	/* [height,width] */
   for (k=0;k<SECURE;k++) {
#if DEBUG
      show(page1,wide,high);
#endif
      /* Shift columns */
      for (i=0;i<wide;i++) {
	 oldline = page1+i;
	 key = (int) code[i];
	 for (j=0;j<high;j++) page2[j][i] =(*oldline)[(j+key)%high];
	 }
      /* Mixup */
	 for (j=1;j<high;j+=2) {
	 mix2 = page2+j;
	 mix1 = mix2-1;
	 for (i=0;i<wide;i++) (*mix2)[i] = ((*mix2)[i]^(*mix1)[i])+1;
	       /* Assume overflow ignored so 255+1==0 */
	 }
      /* Shift rows */
      for (j=0;j<high;j++) {
	 oldline = page2+j;
	 shift = (j%(wide-1))+1;
	 for (i=0;i<wide;i++) page1[i][j] = (*oldline)[(i+shift)%wide];
	 }
      /* Eliminate any pattern (after first iteration only) */
      if (k==0) unpat(page1,wide,high);
      }
   }

#else
unshuffle(page1,code,wide,high)
   line page1[LIMIT];  /* [width,height] */
   line code;
   int wide,high;
   {
   int i,j,k,key,shift;
   line *mix1,*mix2;
   line *newline,page2[LIMIT];	/* [height,width] */
   for (k=0;k<SECURE;k++) {
#if DEBUG
      show(page1,wide,high);
#endif
      /* Eliminate any pattern (before last iteration only) */
      if (k==SECURE-1) unpat(page1,wide,high);
      /* Shift rows back */
      for (j=0;j<high;j++) {
	 newline = page2+j;
	 shift = wide-(j%(wide-1))-1;
	 for (i=0;i<wide;i++) (*newline)[i] = page1[(i+shift)%wide][j];
	 }
      /* Reverse mixup */
      for (j=1;j<high;j+=2) {
	 mix2 = page2+j;
	 mix1 = mix2-1;
	 for (i=0;i<wide;i++) (*mix2)[i] = ((*mix2)[i]-1)^(*mix1)[i];
		/*  Assume underflow is ignored so 0-1==255 */
	 }
      /* Shift columns back */
      for (i=0;i<wide;i++) {
	 newline = page1+i;
	 key = (int) code[i];
	 for (j=0;j<high;j++) (*newline)[(j+key)%high] = page2[j][i];
	 }
      }
   }
#endif

show(page,wide,high)
   line page[LIMIT];
   int wide,high;
   {
   int i,j;
   puts("\n");
   for (j=0;j<high;j++) {
      putc('\n',stdout);
      for (i=0;i<wide;i++) {
	 if (page[i][j]<30) putc('*',stdout);
	 else putc(page[i][j],stdout);
	 }
      }
   }

main (argc,argv)
   int argc;
   char *argv[];
{
   FILE *infile,*outfile;
   int wide,high,i,j,k;    /* Block width and height, loop counters */
   int blkn = 1;	   /* Block counter */
   int clen;	    /* Password code length */
   long chksum;     /* Password checksum */
   int ch = 0;
   int invers;		   /* Version of input file for decrypt */
   int vers = 3;	   /* Version of this program */
   line page[LIMIT],code;
#if ENCRYPT
   int chrcnt;	     /* Character counter */
   long fsize;	     /* Input file size */
   int blocksize,nblocks;
#ifdef UNIX
   long t;  /* For system time */
#else
   struct time t;  /* For system time */
#endif
   struct stat st; /* For input file stats */
   /* Randomise the rand() function */
#ifdef UNIX
   t = time(0);
   srand(t%30000); /* random seed <30000 */
#else
   gettime(&t);
   srand(t.ti_min*400+t.ti_sec*100+t.ti_hund); /* random seed <30000 */
#endif
   /* Check the input arguments */
   if (argc!=3) {puts("\nUsage is: tangle src dst\n"); exit(1);}
#else
   int blkcnt;
   /* Check the input arguments */
   if (argc!=3) {puts("\nUsage is: untangle src dst\n"); exit(1);}
#endif
   if ((infile = fopen(argv[1],"rb")) == NULL) {
      printf("\n%s",argv[1]); perror(" "); exit(1);}
   if ((outfile = fopen(argv[2],"wb")) == NULL) {
      printf("\n%s",argv[2]); perror(" "); exit(1);}
#if ENCRYPT
   /* Get input file size */
   if (stat(argv[1],&st)!=0) {perror(" "); exit(1);}
   fsize = st.st_size;
   printf("The input file size is %ld\n",fsize);
   /* Choose block size accordingly */
   if (fsize<(LIMIT*LIMIT)) blocksize = (int) fsize;
   else {
      nblocks = (int) (fsize/(LIMIT*LIMIT)+1);
      blocksize = (int) (fsize/nblocks+1);
      }
   if (fsize<MINB) blocksize = MINB;	/* Minimum block size enforced */
   wide = 0; while (wide*wide<blocksize) wide++;  /* Approx square root */
   wide = wide+10; if (wide>LIMIT) wide = LIMIT;
   high = blocksize/wide+1; if (high>LIMIT) high = LIMIT;
   while (1) {
      blocksize = wide*high;
      if (fsize<(long) blocksize) break;
      else {
	 /* Multiple blocks, check for last block too small */
	 if (((fsize-1)%blocksize)>(blocksize*3/4)) break;
	 /* (fsize-1) is used above so perfect fit is accepted! */
	 high--; wide--; /* Try a smaller block */
	 }
      if (wide<50) break;
      }
   printf("The width and height are (%d,%d)\n",wide,high);
   printf("The last block is %ld bytes\n",((fsize-1)%blocksize)+1);
   fprintf(outfile,"%d,%d,%d,",vers,wide,high);
#else
   fscanf(infile,"%d,%d,%d,",&invers,&wide,&high);
   if (invers!=vers) {
      printf("This is version %d of the encryption program.\n",vers);
      printf("The input file is for program version %d or invalid.\n",invers);
      exit(1);
      }
#endif
   /* Get password */
   while(1) {
      puts("\nPlease enter your password");
      fgets(code,LIMIT,stdin);
      clen = strlen(code);
      if (clen>9) break;
      puts("Insecure password, try a longer one.");
      puts("For security do not use a name or word in any dictionary.");
      puts("For example use something like \"Dazed and Konfuzed\"");
      }
   for (i=0;i<25;i++) puts(" ");   /* Clear the screen */
   if (clen>wide) puts("Warning: tail of password ignored");
   /* Extend password to possible limit, not null terminated */
   for (i=clen;i<LIMIT;i++) code[i] = code[i%clen] ^ '\152';
   /* Generate a checksum for the characters */
   for (chksum=0,i=0;i<clen;i++) chksum += (int) code[i]*i;
   printf("The password checksum is %ld.  Please wait ...\n",chksum % 1000);
   do { /* tangle or untangle a block */
#if ENCRYPT
      chrcnt = 0;
#else
      if (fscanf(infile,"%d,",&blkcnt)==EOF) goto NOBLOCK;
#endif
      for (j=0;j<high;j++) {
	 for (i=0;i<wide;i++) {
	    if ((ch = getc(infile)) != EOF) {
	       page[i][j] = ch;
#if ENCRYPT
	       chrcnt++;}
	    else if (i==0 && j==0) goto NOBLOCK; /* EOF at start of block! */
	    /* Pad the last block with existing junk */
	    else page[i][j] = page[rand()%wide][rand()%high];
#else
	       ;}
	    else {puts("Error: unexpected end of file"); goto NOBLOCK;}
#endif
	    }
	 }
#if ENCRYPT
      fprintf(outfile,"%d,",chrcnt);
      shuffle(page,code,wide,high);
      for (j=0;j<high;j++) for (i=0;i<wide;i++) putc(page[i][j],outfile);
#else
      unshuffle(page,code,wide,high);
      for (j=0;j<high;j++) for (i=0;i<wide;i++)
	 if ((j*wide+i)<blkcnt) putc(page[i][j],outfile);
#endif
      printf("Finished block number %d\n",blkn++);
      }
   while (ch != EOF);
NOBLOCK:		  /* Jump here to avoid writing an empty block */
   for (i=0;i<LIMIT;i++) code[i] = ' ';   /* Rubout the password before exit */
   fclose(infile);
   fclose(outfile);
}
