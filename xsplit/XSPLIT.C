/**************************************************************************\
*                                                                          *
*                      Cryptographic File Splitter                         *
*                                                                          *
*                                                                          *
*               SOURCE CODE REVISION BETA-940712-01:18 EST                 *
*                                                                          *
*                Compiled with Borland C 3.1, Large Model.                 *
*                                                                          *
*==========================================================================*
*                                                                          *
*          Copyright (C) 1992,1993,1994 by Ray (Arsen) Arachelian,         *
*          All Rights Reserved, General Technical Data Restricted.         *
*                                                                          *
*==========================================================================*
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
* Ray (Arsen) Arachelian is the author and sole owner of XSPLIT(tm)        *
* and maintains exclusive proprietary interests and exclusive copyright    *
* for the forementioned program.                                           *
*                                                                          *
* XSPLIT is a trademarks of the author                                     *
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
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <values.h>



// remove for non-dos machines.
#define DOS 1


int  numparts;
long rndread=0;
long rsize=0L;
long dsize=0L;
FILE *rndfile; //=NULL; /* input of random numbers              */
FILE *datafile;
FILE *part[10];




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

void thermometer(FILE *infile, long fsize)
{
 unsigned long a,i, percent;

 char filled='Û';
 char empty ='±';



/*-----For non DOS systems, swap these blocks.----------------------
 char filled='#';
 char empty ='_';
 ------------------------------------------------------------------*/

 if (fsize<=0)	return;

 percent=100*ftell(infile)/fsize;

 printf("\r");

 a=(percent/5);

 for (i=0; i<=20; i++)
  if (i<=a)
	 putchar(filled);
  else
	 putchar(empty);


 printf(" %3ld%% ",percent);

 scanrot();

}




#ifndef DOS
void fcloseall(void)
{
 int i;

 if (rndfile) fclose(rndfile);
 fclose(datafile);
 for (i=0; i<=numparts; fclose(part[i]));

}
#endif

/*-------------------------------------------------------------------*\
| FUNCTION: getrand                                                   |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Returns a random value, either from a random file,         |
|          random number device, or random number function.           |
|                                                                     |
| PARAMS:  none                                                       |
|                                                                     |
| RETURNS: random value (char)                                        |
|                                                                     |
|                                                                     |
\*-------------------------------------------------------------------*/


char getrand(void)
{

 rndread++;

 if (rndfile==NULL) return (char) (rand() & 0xff);

 if (rsize!=MAXLONG) rsize--;

 if (rsize<1)
  {
   fcloseall();
   perror("\n\7\7\7!!!END OF RANDOM FILE REACHED BEFORE ENCRYPTION COMPLETED!!!\n");
   getch();
   exit(-99);
  }

 return (fgetc(rndfile));
}




void xjoin(void)
{
int c;
int i;
int endflag=0;
int xorkey;



 while (ftell(part[0])<dsize)
 {
  xorkey=0;
  for (i=0; i<numparts; i++)
   {
    c=fgetc(part[i]);
    if (c==EOF) endflag=1;
    xorkey=xorkey ^ c;
   }
   if (!endflag) fputc(xorkey,datafile);
   if ( (ftell(datafile) & 0x000000ff)==0) thermometer(part[0], dsize);
  }

 thermometer(part[0], dsize);
 fcloseall();

}



void xsplit(void)
{

 int picked;
 int i;
 int xorkey=0;
 int curkey=0;
 int c;

 //
 //
 // Encrypting:
 //
 //
 //  Key=X1 ^ X2 ^ X3 ^ ... ^ Xn
 //
 //  C=Key^Text
 //
 //  C is randomly picked and inserted as one of the key files.
 //
 //                             '
 //
 // Decrypting:
 //
 //  Key=X1 ^ X2 ^... ^ Xn
 //
 //  Text=Key ^ C
 //
 //


while (ftell(datafile)<dsize)
 {
  c=fgetc(datafile);
  xorkey=0;
  picked=getrand() % (numparts);
  for (i=0; i<numparts; i++)
   if (i!=picked)
   {
    curkey=getrand();
    xorkey=xorkey ^ curkey;

    fputc(curkey,part[i]);
   }
   fputc(xorkey ^ c,part[picked]);

   if ((ftell(datafile) & 0x000000ff)==0) thermometer(datafile, dsize);

  }

 thermometer(datafile, dsize);
 fcloseall();

}


void helpscreen(void)
{
 puts("");
 puts("Usage: XSPLIT cmd {-RD:device|-RF:file} datafile part1 part2 .. partn");
 puts("");
 puts("Where cmd: J         Join the parts back into the datafile");
 puts("           S         Split the data file into parts.");
 puts("");
 puts(" Or       -RD:device Use a hardware device random number generator");
 puts("          -RF:file   Use a file for the random numbers");
 puts("                     If neither -RD: nor -RF: are used, the internal");
 puts("                     random number generator is used.");
 puts("");
 puts("          datafile   The name of the file to cryptographically join or split");
 puts("");
 puts("          part       Is the file name of one of the parts.");

 exit(69);
}



int main(int argc, char *argv[])
{
 int i,pos;
 int start=0;

 puts("");
 puts("-----------------------------------------------------------------------");
 puts(" XSPLIT(tm) V1.0 Rel 1.00 Copyright (C) 1994 by Ray (Arsen) Arachelian,");
 puts("                         All Rights Reserved");
 puts("-----------------------------------------------------------------------");
 puts("");
 puts("                    A Cryptographic File Splitter");
 puts("");
 puts("");


 if (argc<5) helpscreen();


 rsize=0;

 start=2;

 if (argv[2][0]=='-' || argv[2][0]=='/') pos=1; else pos=0;

 if (argv[2][pos]=='R')
 if ( argv[2][pos+1]==':' && (toupper(argv[2][pos])=='F' || toupper(argv[2][pos])=='D'))
     {
	    errno=0;
	    rndfile=fopen(&argv[2][pos+2],"rb");
	    start=3;
	    if (rndfile==NULL || errno)
	      {
		perror("Can't open random data input file/device");
		return(1);
	      }

	    if (toupper(argv[2][pos])=='F')
	      { fseek(rndfile,0L,SEEK_END);
		   rsize=ftell(rndfile);
		   fseek(rndfile,0L,SEEK_SET);
		   printf("RANDOM FILE [%s] IS %ld BYTES\n",&argv[2][pos+2],rsize);
	      }
	    else
	      {
		   rsize=MAXLONG;
		   printf("RANDOM DEVICE DRIVER [%s]\n",&argv[2][pos+2]);
	      }

     }


if (strcmp(strupr(argv[1]),"S")==0 || strcmp(strupr(argv[1]),"-S")==0)
 {

  for (i=0; i+start+1<argc; i++)
  {
   numparts++;
   part[i]=fopen(argv[start+1+i],"wb");
   if (part[i]==NULL)
     {
      perror(argv[start+1+i]);
      exit(1);
     }
  }

  datafile=fopen(argv[start],"rb");
  if (!datafile)
   {
    perror("Couldn't create output file.");
   }
  fseek(datafile,0L,SEEK_END);
  dsize=ftell(datafile);
  fseek(datafile,0L,SEEK_SET);

  xsplit();
  return 0;
 }



if (strcmp(strupr(argv[1]),"J")==0 || strcmp(strupr(argv[1]),"-J")==0)
 {

  for (i=0; i+start+1<argc; i++)
  {
   numparts++;
   part[i]=fopen(argv[start+1+i],"rb");
   if (part[i]==NULL)
     {
      perror(argv[start+1+i]);
      exit(1);
     }
   fseek(part[i],0L,SEEK_END);
   if (i==0) dsize=ftell(part[i]);
   else if (ftell(part[i])!=dsize)
     {
      fprintf(stderr,"The split parts must be the same size.\n");
      exit(2);
     }
   fseek(part[i],0L,SEEK_SET);

  }


  datafile=fopen(argv[start],"wb");
  if (!datafile)
   {
    perror("Couldn't create output file.");
   }


  xjoin();
  return 0;
 }
 helpscreen();


 return 0;
}
