/**************************************************************************\
* WNINJECT(tm):  Program to inject or extract LSB's out of a PCX image. It *
*                is reccomended that the image be 24 bits for the purpose  *
*                of having it used with WNSTORM.                           *
*                                                                          *
*                             V9405130050                                  *
*                                                                          *
* Copyright (C) 1994 by Ray (Arsen) Arachelian, All Rights Reserved, GTDR  *
*                                                                          *
*                                                                          *
*    WARNING: This source code is the sole property of the author and is   *
*    to be treated as such.  The use of this source code is subject to     *
*    certain limitations stated in the documentation.  Use of this source  *
*    code is subject to the conditions stated therein. Using this source   *
*    code signifies acceptance to the terms listed in the documentation.   *
*                                                                          *
*    The ideas/algorithms of this software MAY not be patented by other    *
*    parties.                                                              *
*                                                                          *
* Ray (Arsen) Arachelian is the author and sole owner of WNINJECT(tm) and  *
* WNSTORM(tm) and maintains exclusive proprietary interests and exclusive  *
* copyright for these forementioned programs.                              *
*                                                                          *
* WNINJECT and WNSTORM are trademarks of the author                        *
*                                                                          *
* WARANTY: There ain't none.  Due to the nature of this software, the      *
*          author disclaims all warranties whether express or implied. The *
*          software distributed in this package is distributed "AS IS" and *
*          all consequences arising from its use are the solely the user's *
*          responsability.  Neither the author, nor his associates may be  *
*          held liable for any damage/consequnces arising from the use of  *
*          this software.  Some states do not allow the exclusion of       *
*          warranties, so your rights may vary.                            *
*                                                                          *
*          Don't bother suing me.   I'm not rich.                          *
*                                                                          *
*                                                                          *
* LICENSE:                                                                 *
*                                                                          *
* This is published proprietary source code of the author, and may be      *
* freely distributed and copied as long as this and any other copyright,   *
* trademark, and ownership notices are not altered in any way shape or form*
*                                                                          *
* This software is hereby deemed freeware and may not be sold.  A small    *
* duplication charge (less than $5.00 US) may be made as compensation for  *
* the specific distribution of this software. This software may be included*
* on any CD-ROM collection of shareware and or freeware.   Please feel free*
* to give out copies of this software to your friends.                     *
*                                                                          *
* You may not distribute modified copies of this software without the      *
* written consent of the author.                                           *
*                                                                          *
* Usage of this software indicates an agreement to the above license.      *
*                                                                          *
* Should any statement in this license be deemed invalid, only that        *
* statement shall be revoked, leaving the rest of this license valid.      *
*                                                                          *
* This license terminates when the user destroys all copies of this        *
* software from all media on which it resides.                             *
*                                                                          *
\**************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>  /* for getch()  */
#include <ctype.h>


/* Used by bit shifting i/o routine */

int inbits=0;
int fromwns=0;
int shreg=0;


int mask=1;               /* Selected by number of colors */
int highbit=1;            /* Selected by number of colors */



FILE *original;
FILE *wnfile;
FILE *newfile;
long original_end; /* Size of the original PCX file.  Doctored to work */
		   /* correctly with the pallete information.          */
long actualfilesize; /* Actual end of original PCX file                */


const char *kind="PCX";


struct pcxheader
{
       char manufacturer;
       char version;
       char encoding;
       char bitsperpixel;
       int  windowx1;
       int  windowy1;
       int  windowx2;
       int  windowy2;
       int  xdpi;
       int  ydpi;
       char colormap[48];
       char reserved;

       char numcolorplanes;

       int  bytesperline;
       int  palletteinfo;
       int  xscrsize;
       int  yscrsize;
       char filler[54];
} pcxheader;




int XSIZE, YSIZE;
long LineBytes;      /* Total memory required to hold one PCX scanline */
long PCXTotalBytes;  /* Total memory required to hold whole PCX image  */



void  pcxinfo(void);
void  helpscreen(void);
void  extract(void);
void  inject(void);
void  copyfile(void);
int   rleflag(char);
void  scanrot(void);
void  sethighbit(int);
void  bitput(char);
void  bitmix(int);
void  thermometer(void);









/***********************************************************************\
*                                                                       *
* Display a rotating cursor to inform user that program is busy.        *
*                                                                       *
* This should be replaced with a rotating watch cursor/hourglass mouse  *
* cursor.                                                               *
*                                                                       *
\***********************************************************************/

void scanrot(void)
{

 char scan[]="/-\\|";
 static int rotpos;

 rotpos++;
 rotpos=(rotpos & 3);

 printf("%c%c",scan[rotpos],8);

}



/*-------------------------------------------------------------------*\
| FUNCTION: thermometer                                               |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Display the infile thermometer bar                         |
|                                                                     |
| PARAMS:  none                                                       |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
| NOTE:  This function may need to be re-written for different o/s's. |
|        For unix systems running on non-80x86's, you may wish to     |
|        change the filled character to a pound symbol, and the       |
|        empty character to perhaps a minus symbol, or an underline.  |
|                                                                     |
|        Then again on unix systems, you may want to remove this      |
|        function entirely if you plan to pipe its output.            |
|                                                                     |
|        On GUI based o/s's (Mac, Windows, Next, etc.) you may wish   |
|        use o/s calls to actually draw a thermometer window, and     |
|        animate it.                                                  |
|                                                                     |
\*-------------------------------------------------------------------*/

void thermometer(void)
{
 unsigned long a,i;
 unsigned long percent;

 char filled='Û';
 char empty ='±';


/*-----For non DOS systems, swap these blocks.----------------------
 char filled='#';
 char empty ='_';
 ------------------------------------------------------------------*/

 if (original_end<=0)
	return;

 printf("\r");


 percent=(100*ftell(original)/original_end);

 a=(percent/5);

 for (i=0; i<=20; i++)
  if (i<=a)
	 putchar(filled);
  else
	 putchar(empty);


 printf(" %3d%% ",percent);

 scanrot();
}





/**********************************************************************\
*                                                                      *
* Set the highbit and mask variables for bitextract routines           *
*                                                                      *
*                                                                      *
*                                                                      *
\**********************************************************************/

void sethighbit(int b)
{

 if (b>4)
 {
  puts("Sorry, but I cannot support more than four least significant");
  puts("bits as the PCX format uses the upper two bits as part of its");
  puts("RLE data compression method.   Selecting more than four lsb's");
  puts("has been found to radically alter the picture and would give");
  puts("away the fact that it containes steganographical data.");
  puts("");

  fcloseall();
  exit(1);

 }


 if (pcxheader.bitsperpixel < b)
  {
   puts("I'm sorry, but the header for this picture has less bits/pixel");
   puts("than the number of bits you've requested to extract/inject, even");
   puts("though the number of color planes may be higher.  Please use a");
   puts("smaller lsb value.");

   fcloseall();

   exit(1);

  }


 switch (b)
 {
  case 1:  mask=  1; highbit=  1; break;
  case 2:  mask=  3; highbit=  2; break;
  case 3:  mask=  7; highbit=  4; break;
  case 4:  mask= 15; highbit=  8; break;
  case 5:  mask= 31; highbit= 16; break;
  case 6:  mask= 63; highbit= 32; break;
  case 7:  mask=127; highbit= 64; break;
  case 8:  mask=255; highbit=128; break;

  default:
  {
   puts("You've chosen an invalid least_significant_bit value");
   puts("Valid values are 1-8, however they depend on the image");
   puts("processed.");

  }
 }

 printf("BIT INFO:  Number of LSB's:%d      LSB-BitMask:%3d      LSB HighBit:%3d\n",b,mask,highbit);


}


/**********************************************************************\
*                                                                      *
* Display a help screen if the user enters invalid arguements          *
*                                                                      *
*                                                                      *
\**********************************************************************/


void helpscreen(void)
{
 puts("Usage:");
 puts("");
 puts("To get info on a PCX file:");
 puts("WNINJECT P ORIGINAL.PCX");
 puts("         |   |");
 puts("         |   +-----------------------> PCX file name");
 puts("         +---------------------------> Print PCX info Command");
 puts("");
 puts("To extract the lowest significant bits of a PCX image:");
 puts("WNINJECT E ORIGINAL.PCX LSB.DAT {bits}");
 puts("         |  |           |         |");
 puts("         |  |           |         +--> Number of bits to inject");
 puts("         |  |           |");
 puts("         |  |           +------------> Extracted bits output file)");
 puts("         |  +------------------------> Original PCX file to hide data into");
 puts("         +---------------------------> Extract bits command");
 puts("");
 puts("");
 printf("[MORE]\r");
 getch();
 puts("       \n");


 puts("To inject an encrypted document in a PCX image:");
 puts("WNINJECT I ORIGINAL.PCX CRYPT.DAT STEGO.PCX {bits}");
 puts("         |  |           |         |          |");
 puts("         |  |           |         |          +-> number of bits to extract");
 puts("         |  |           |         |");
 puts("         |  |           |         +--> Output File with encrypted data");
 puts("         |  |           +------------> Encrypted data file from WNSTORM **");
 puts("         |  +------------------------> Original PCX file to hide data into");
 puts("         +---------------------------> Inject bits command");
 puts(" ** Must be the encrypted result of WNSTORM using an WNINJECT extracted PCX");
 puts("    file as the random number file for this to work! See documentation.");

 puts("");
 printf("[MORE]\r");
 getch();
 puts("       \n");

 puts("The WNINJECT program is written as a companion to the WNSTORM encryption");
 puts("software.  It's purpose is to aid you in hiding encrypted messages in an");
 puts("image file.  This version only supports PCX files.");
 puts("");
 puts("The hiding of secret messages in another medium (a picture, a movie, a sound,");
 puts("etc. is known as STEGANOGRAPHY, and its use is to help you send a secret");
 puts("message when it is dangerous/illegal to send a clearly encrypted messages.");

 exit(1);
}






/****************************************************************************\
* main():  parse arguements, open files, and dispatch functions that do work *
\****************************************************************************/


int main(int argc, char *argv[])
{
int bits;

 char cmd;

 puts("===========================================================================");
 puts("                                  WNINJECT(tm)");
 puts("---------------------------------------------------------------------------");
 puts("    A PCX BIT INJECTCTOR/EXTRACTOR FOR THE WNSTORM(tm) ENCRYPTION SYSTEM   ");
 puts("             * THIS VERSION MAY NOT BE DISTRIBUTED WITHOUT WNSTORM *");
 puts("---------------------------------------------------------------------------");
 puts("                   Copyright(C) 1994 by Ray (Arsen) Arachelian");
 puts("                          All Rights Reserved, GTDR.");
 puts("===========================================================================");
 puts("");


 if (argc<3) helpscreen();


 cmd=argv[1][0];

 if (cmd=='/' || cmd == '-') cmd=argv[1][1];

 cmd=toupper(cmd);
 switch(cmd)
  {
   case 'I':

       if (argc<5 || argc>6) helpscreen();

       if (argc==6) bits=atoi(argv[5]);
	  else bits=1;


       if (  (original=fopen(argv[2],"rb") )==0)
	  {
	   perror("Couldn't open PCX file");
	   fcloseall();
	   exit(1);
	  };


       if (  (wnfile  =fopen(argv[3],"rb") )==0)
	  {
	   perror("Couldn't open Encrypted File");
	   fcloseall();
	   exit(1);
	  }

       if (  (newfile =fopen(argv[4],"w+b") )==0)
	  {
	   perror("Couldn't create new PCX file");
	   fcloseall();
	   exit(1);
	  }

	pcxinfo();
	sethighbit(bits);
	copyfile();
	inject();

	break;

   case 'E':

      if (argc<4 || argc>5) helpscreen();

      if (argc==5) bits=atoi(argv[4]);
	  else bits=1;



       if (  (original=fopen(argv[2],"rb") )==0)
	  {
	   perror("Couldn't open PCX file");
	   fcloseall();
	   exit(1);
	  };


       if (  (newfile =fopen(argv[3],"wb") )==0)
	  {
	   perror("Couldn't create LSB data file");
	   fcloseall();
	   exit(1);
	  }
       pcxinfo();
       sethighbit(bits);
       extract();
       break;

   case 'P':

       if (argc!=3) helpscreen();

       if (  (original=fopen(argv[2],"rb") )==0)
	  {
	   perror("Couldn't open PCX file");
	   fcloseall();
	   exit(1);
	  };

	pcxinfo();
	break;


   case '?': helpscreen();
   case 'H': helpscreen();
   default: printf("Unknown command %s\n",argv[1]); helpscreen();

   }


   printf("\rDone.                                             \n");

 fcloseall();

 return(0);

}






/*************************************************************************\
* rleflag(): returns true if a byte is an RLE indicator                   *
*                                                                         *
* NOTE: if your compiler doesn't support inline functions remove the      *
*       inline keyword.                                                   *
*                                                                         *
\*************************************************************************/

int rleflag(char i)
{
 return ((i & 0xC0)==0xC0);
}





/*************************************************************************\
*                                                                         *
* bitmix(): inject cypherbits bits with PCX bits to new PCX file          *
*                                                                         *
*           set inbits to 8 and read a char from wnsfile to shrreg        *
*           the first time you call this!!!!!                             *
*                                                                         *
*                                                                         *
*                                                                         *
\*************************************************************************/


void bitmix(int c)
{
 char o1;

 int j, a1,a2;


 o1=(c & (255-mask));           /* strip off lsb from original file using an*/
				/* mask. */

 for (j=highbit; (j & mask)!=0 && j>0; j=j>>1)
  {
   if ((shreg & 128)!=0) o1=o1 | j;

    shreg=shreg<<1;             /* Shift the shift register so we can get a */
    inbits--;	       	        /* bit for the next pass                    */

    if (inbits<=0)              /* If we're out of bits, get another byte   */
       {                        /* into the shift register from wnsfile     */
	  shreg=fgetc(wnfile);

	  inbits=8;             /* inbits is now 8                          */

	  if (shreg==EOF)       /* Did we reach eof?                        */
	   {
	    fcloseall();
	    printf("\rDone mixing bits...                                  \r");
	    exit(0);
	   }

	}
  }

  fputc((o1 & 255), newfile);  /* output stegotext to the newfile,  */

}



/*************************************************************************\
*                                                                         *
* bitput(): write extracted bits to the output file.                      *
*                                                                         *
*                                                                         *
\*************************************************************************/

void bitput(char c)
{
 char o1;
 int j;

 o1=c & mask;

 for (j=highbit; (j & mask)!=0 & j>0; j=j>>1)
  {
   /* First we shift the shift register one to the left to make room
      for a bit.  Then we compare the high bit of the char to insert
      to the stream with the highbit variable.  If their binary AND
      operator produces a value, the !=0 operation will produce a 1
      (true) and since we are inserting the high bit of o1 into the
      shift register, we want it as bit 0, so this works. */

   shreg=(shreg<<1);
   shreg=(shreg |   ( (o1 & highbit)!=0 )   ) ;
   o1=((o1<<1) & mask); /* Move the bit in C */
   inbits++;

   if (inbits>=8)
      {
       fputc((shreg & 255), newfile);
       inbits=0;
       shreg=0;
      }
  }



 /* The last bits in the file will be lost, however since WNSTORM needs a
    whole byte, it is better to simply let the last one be unused */

}



/***************************************************************************\
* extract(): do the data extraction from the PCX file.                      *
*                                                                           *
*                                                                           *
\***************************************************************************/

void extract()
{

 int c1;
 int done;
 long cposition;

 shreg=0;
 inbits=0;


 puts("Extracting lsb's from PCX image:");


  do
    {

     c1=fgetc(original);

     cposition=ftell(original);

     done=(cposition>=original_end);

     if ((cposition & 511)==0) thermometer();

     if (c1!=EOF && rleflag(c1) && !done)
	{
	 c1=fgetc(original);
	 cposition=ftell(original);
	 if ((cposition & 511)==0) thermometer();
	 continue;
	} /* Skip next byte if RLE */

     if (!done) bitput(c1);

    } while (!done);


 printf("\r                                                      \r");

 return;

}






/***************************************************************************\
* inject(): do the data injection into the PCX file.                        *
*                                                                           *
*                                                                           *
\***************************************************************************/

void inject(void)
{


 int c1;
 int done;
 long cposition;




 puts("Injecting cyphertext in new PCX image:");


 /* Initialize bitmix function */


 fseek(wnfile,0,SEEK_SET);
 shreg=fgetc(wnfile);        /* Read a byte into the shift register */
 inbits=8;              /* inbits is now 8                        */



  do
    {

     c1=fgetc(original);   /* Get a byte from the original picture */

     cposition=ftell(original);

     done=(cposition>=original_end);

     if ((cposition & 511)==0) thermometer();


     if (c1!=EOF && rleflag(c1) && !done)
	{
	 fputc(c1,newfile);    /* Put the byte back in the newfile   */
	 c1=fgetc(original);   /* Get another byte from the original */
	 cposition=ftell(original);
	 if ((cposition & 511)==0) thermometer();
	 fputc(c1,newfile);    /* Write it to the new file           */

	 continue;
	} /* Skip next byte if RLE */

     if (!done) bitmix(c1);   /* Write the stego bits with the bits
			       from the original fille */

    } while (!done);


 printf("\r                                                           \r");
 return;

}



/*************************************************************************\
* coordprint(): return a string containing the two coordinates passed as  *
*               parameters.  Used by pcxinfo() screen to print window size*
*               etc.                                                      *
*                                                                         *
*               It uses rotating static buffers.  No more than seven sets *
*               of coordinates should be printed with printf at a time    *
*                                                                         *
\*************************************************************************/



char *coordprint(long x, long y)
{
  static i;
  static char buffer[7][30];


  i++;
  i=(i & 7);

  sprintf(buffer[i],"(%ld,%ld)",x,y);

  return buffer[i];

}



/*************************************************************************\
* coord4print(): return a string containing the 4  coordinates passed as  *
*               parameters.  Used by pcxinfo() screen to print window size*
*               etc.                                                      *
*                                                                         *
*               It uses rotating static buffers.  No more than seven sets *
*               of coordinates should be printed with printf at a time    *
*                                                                         *
\*************************************************************************/



char *coord4print(long x1, long y1,long x2,long y2)
{
  static i;
  static char buffer[7][30];


  i++;
  i=(i & 7);

  sprintf(buffer[i],"(%ld,%ld)-(%ld,%ld)",x1,y1,x2,y2);

  return buffer[i];

}





/*************************************************************************\
* pcxinfo() - display PCX header information and setup header for future  *
*             processing.  Must call this function before extract or      *
*             inject                                                      *
\*************************************************************************/


void pcxinfo(void)
{

char colorbit;
long colors;
char c;


 puts("PCX File Info:");



       fseek(original,0L,SEEK_END);
       actualfilesize=original_end=ftell(original);


       fseek(original,0L,SEEK_SET);
       fread(&pcxheader,128,1,original);

       colorbit=pcxheader.numcolorplanes  * pcxheader.bitsperpixel;
       colors=(1L<<(long)colorbit);



       printf("Manufacturer:       %3x          Version:     %3d    Encoding:  %3d\n",
				     (int)    pcxheader.manufacturer,
				     (int)    pcxheader.version,
				     (int)    pcxheader.encoding);


       printf("Bits/Pixel/Plane:   %3d          Color Planes:%3d    Colors:      %ld\n",
				     pcxheader.bitsperpixel,
				     pcxheader.numcolorplanes,
				     colors);

       printf("Window:               %-19s            %s DPI\n",
	     coord4print(pcxheader.windowx1,pcxheader.windowy1,
			pcxheader.windowx2,pcxheader.windowy2),
	     coordprint(pcxheader.xdpi,    pcxheader.ydpi));


     XSIZE = pcxheader.windowx2 - pcxheader.windowx1 + 1;
     YSIZE = pcxheader.windowy2 - pcxheader.windowy1 + 1;


       printf("Image Size:           %-18s        Size:%s pels\n",
	     coordprint(XSIZE,YSIZE),
	     coordprint(pcxheader.xscrsize,pcxheader.yscrsize));


     LineBytes = pcxheader.numcolorplanes * pcxheader.bytesperline;

       printf("Bytes/line/plane:     %-7d    Bytes/Line:    %d\n",
				    pcxheader.bytesperline,
				    LineBytes);


     PCXTotalBytes = LineBytes*YSIZE;

       printf("Bytes to hold image:  %-7ld    File Size      %ld bytes\n",
				    PCXTotalBytes,
				    actualfilesize);

     if ((PCXTotalBytes-actualfilesize-128)>0)
       printf("RLE saved:            %-7ld    ",(PCXTotalBytes-actualfilesize-128));
     else
       printf("RLE saved:            none       ");


     if (colors==16)
     {
      printf("PCX Pallette:  16 color [EGA]\n");

     }


     if (colors==4)
     {
      printf("PCX Pallette:  4  color [CGA]\n");

     }

     if (colors<4)
     {
      printf("PCX Pallett:   NONE [monochrome]\n");

     }



     if (pcxheader.version==5 && colors==256L)
      {
       fseek(original,-769,SEEK_END);
       c=fgetc(original);
       if (c==12) {
		  original_end=ftell(original);
		  printf("PCX Pallette:  256color [Ver 5 PCX]\n");
		 }
       else
		  printf("PCX Pallette:  %x type\n",         pcxheader.palletteinfo);
      }


     fseek(original,128,SEEK_SET);

     puts("");

}



/************************************************************************\
* copyfile(): used by the inject command.  This function duplicates the  *
*             pcx image to the stego.pcx file.  The stego file is then   *
*             reopened in "w+" mode.                                     *
\************************************************************************/

void copyfile(void)
{
 char *buffer;
 int   blksize=8192;
 long  targetleft=actualfilesize;

 errno=0;
 buffer=malloc(blksize);  /* Get a 16K buffer */
 if (errno || buffer==NULL)
    {
     perror("There isn't enough memory!");
     exit(9);
    }


 fseek(original,0L,SEEK_SET);
 fseek(newfile, 0L,SEEK_SET);


 puts("Copying original PCX file to STEGOED PCX file...");


 while (targetleft>blksize)
 {

  errno=0;
  thermometer();
  fread (buffer,blksize,1,original);
  fwrite(buffer,blksize,1,newfile);
  targetleft-=blksize;

  if (errno)
    {
     perror("ERROR WRITING STEGOED PCX FILE!");
     exit(10);
    }
 }


 if (targetleft>0)
    {
     thermometer();
     fread (buffer,(size_t) targetleft,1,original);
     fwrite(buffer,(size_t) targetleft,1,newfile );
    }

  thermometer();

  fflush(newfile);

  fseek(newfile, 128L,SEEK_SET);
  fseek(original,128L,SEEK_SET);

  if (errno)
    {
     perror("ERROR WRITING STEGOED PCX FILE!");
     exit(10);
    }

  printf("\r                                              \r");

  free(buffer);

  return;
}