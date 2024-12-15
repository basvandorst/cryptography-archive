/***********************************************************************\
*                                                                       *
*                        White Noise Storm                              *
*                                                                       *
*                                                                       *
*             SOURCE CODE REVISION BETA-940411-01:18 EST                *
*                                                                       *
*              Compiled with Borland C 3.1, Large Model.                *
*                                                                       *
*=======================================================================*
*                                                                       *
*        Copyright (C) 1992,1993,1994 by Ray (Arsen) Arachelian,        *
*        All Rights Reserved, General Technical Data Restricted.        *
*                                                                       *
*=======================================================================*
*                                                                       *
*  WARNING: This source code is the sole property of the author and is  *
*  to be treated as such.  The use of this source code is subject to    *
*  certain limitations stated in the documentation.  Use of this source *
*  code is subject to the conditions stated therein. Using this source  *
*  code signifies acceptance to the terms listed in the documentation.  *
*                                                                       *
*  The ideas/algorithms of this software MAY not be patented by other   *
*  parties.                                                             *
*                                                                       *
\***********************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <conio.h>          /* for getch() prototype only */
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <values.h>


short int     stealth=0;     /* No header flag                       */

short int     statson=0;     /* do statistics flag                   */

short int     bitfix=0;      /* statistical bit fix flag             */

short int     trace=0;       /* Trace mode flag                      */

unsigned long fsize;         /* Input file size, needed by thermo fn */

	 long rsize;         /* Random input device file size        */

FILE          *infile;       /* Input file name handle               */

FILE          *outfile;      /* Output file name handle              */

FILE          *rndfile;      /* input of random numbers              */

unsigned int  limitchnl;     /* Max number of channels, user defined */

	 int maxchnl;        /* Size of bytes in the current window  */

unsigned int command;        /* command byte index                   */


unsigned int  passkey;       /* Password elementry encryption key.
				This is used as part of the password
				encryption so as to make every
				encryption very sensitive to even 1
				bit of change, and thus produce
				garbage between two very similar
				passwords (diffusion)                */

unsigned char oldxor;        /* Previous data char.  Used by encrypt
				and decrypt for recursive encryption
				with the previous pass's charachter.
				Thus if ANY char is decrypted the
				wrong way, all further chars will
				also be decrypted the wrong way.     */

unsigned char password[151]; /* Password passkey.  This isn't really
				limited to 150 chars, you can have more,
				but then you can't get'em from the
				command line.                        */

int DataBit[8];              /* Array of data bit values.  What bit of
				the data window is our bit in?       */

int DataByte[8];             /* Data bit byte pointers.  Which byte
				of the window is our bit in?         */



			     /* Define maximum stream size           */
#define MAXSTREAM 32

unsigned char
	 stream[MAXSTREAM];  /* Encryption Stream buffer             */

unsigned char
	xstream[MAXSTREAM];  /* Bit fix Stream                       */

char junk[129];              /* Junk buffer used for version check.  */

char *header="Encrypted by White Noise Storm.  V2.0 940322\n\r\0x04\0x1A                                                    ";
/*
		  DO NOT CHANGE THIS PORTION  OF
		  THE HEADER LINE!!!!!!!!!!!!!!!
		  If you do, the modified program
		  won't be able to decrypt what
		  was written by other copies of
		  it except in stealth mode.
*/



/*-----------------------------------------------------------------------

   Define byte to start data file from.  Should be greater than size
   of the header line, plus 3 other bytes for EOF's for DOS & Unix.

   If you customize this program, thus making it incompatible with the
   normal version, you should also change the header line to include
   your company name as well, so that standard versions will not
   be confused and attempt to decrypt something it can't.  You should
   however LEAVE the "Encrypted by White Noise Storm." portion of the
   header in the text.  Also change the size of the junk[] buffer
   declared above to be slightly greater than your possible header
   line, and the STARTBYTE as well.

   There was a weaker beta version of this program created in Sep 92
   which didn't split the data up into 8 separate bits, but rather
   worked as nibbles (4 low bytes, and 4 high bytes.)  That version
   has the same signature as this program, but is incompatible. Hey,
   that's what you get for using a beta test version.

   This version should be four times as secure as the previous beta
   as it not only swaps the bits randomly around, but also swaps the
   bytes in which they reside.



 ------------------------------------------------------------------------*/

#define STARTBYTE 100



/*-------------------- Function Prototypes ------------------------------*/

  void help(void);

  void victimbit(int *, int *, int, int);

  void statbitfix       (void);

  unsigned char lrotate (char in, int times);

  unsigned char rrotate (char in, int times);

  unsigned char prexor  (char in, int index);

  void seedpassword     (void);

  void nextpasscommand  (int index);

  void encrypt          (void);

  void decrypt          (void);

  void thermometer      (void);


  /* Debugging code */

  void traceout(void);

  int  vlx              (int, int);

  void collision(int line);

/*----------------------------------------------------------------------*/


/*-------------------------------------------------------------------*\
| FUNCTION: traceout                                                  |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Display a trace of the random number window and            |
|          the data channels.                                         |
|                                                                     |
| PARAMS:  none                                                       |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
|                                                                     |
\*-------------------------------------------------------------------*/




void traceout(void)
{
 int i,b;

 if (!trace) return;

 printf("[%d] ",maxchnl);

 for (i=0; i<=7; i++)
 {
  switch (DataBit[i])
  {
   case   1: b=0; break;
   case   2: b=1; break;
   case   4: b=2; break;
   case   8: b=3; break;
   case  16: b=4; break;
   case  32: b=5; break;
   case  64: b=6; break;
   case 128: b=7; break;
   default: b=-1; break;
  }
  printf("%d:%d ",DataByte[i],b);
 }

  printf(" pos=%6ld/%-6ld :",ftell(infile),fsize);

  for (i=0; i<=maxchnl; i++)
   printf("%02x",stream[i]);

  puts("");

}



/*-------------------------------------------------------------------*\
| FUNCTION: vlx                                                       |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Check for array overflows.  Used for debugging             |
|                                                                     |
|                                                                     |
| PARAMS:  value of index, line number                                |
|                                                                     |
| RETURNS: nothing                                                    |
|                                                                     |
|                                                                     |
\*-------------------------------------------------------------------*/




int vlx(int x,int y)
{

 if (limitchnl<2 || limitchnl>32)
  {
   printf("limitchnl out of range in line %d (max=%d, lim=%d)\n",
				y,maxchnl,limitchnl);

  }



 if (x<0 || x>7)
  {
   printf("Array out of bounds (%d) at line %d\n\r",x,y);
   exit(0);
  }

 return(x);
}




/*-------------------------------------------------------------------*\
| FUNCTION: collision                                                 |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Check for a bit collision, or other error.                 |
|          Used for debugging.                                        |
|                                                                     |
| PARAMS:  line number to display in error message.                   |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
|                                                                     |
\*-------------------------------------------------------------------*/



void collision(int line)
{

 unsigned i, j;

if (limitchnl<2 || limitchnl>32)
  {
  printf("limitchnl out of range in line %d (max=%d, lim=%d)\n",
				line,maxchnl,limitchnl);
  }



 j=0;

 for (i=0; i<=7; i++)
   j=j^DataBit[i];

 if (j!=255)
 {
  printf("\7\7Data bit collision in line %d\n\r",line);

  for (i=0; i<=7; printf("Bit %d = %d, \n\r",i,DataBit[i]),i++);

  printf("\n\r");

  exit(0);
 }


}




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
 if (rndfile==NULL) return (char) (rand() & 0xff);
 if (rsize<MAXLONG) rsize--;

 if (rsize<-1)
  {
   fcloseall();
   perror("!!!END OF RANDOM FILE REACHED BEFORE ENCRYPTION COMPLETED!!!");
   exit(-99);
  }

 return (char) fgetc(rndfile);
}



/*-------------------------------------------------------------------*\
| FUNCTION: statistics                                                |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Keep statistical information of incoming random            |
|          data and outgoing random data to the stream. 	      |
|                                                                     |
| PARAMS:  mode, invalue                                              |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
| NOTE:    This function should be replaced by a better one that      |
|          does more analysis than this one.                          |
|                                                                     |
\*-------------------------------------------------------------------*/

void statistics(int inflag, int val)
{
 static float cnt[2];     /* Count of values              */
 static float sum[2];     /* Sum of all numbers           */
 static float ssum[2];    /* Sum of squares of numbers    */
 static float bit0[2];    /* Total bits off               */
 static float bit1[2];    /* Total bits on                */
 static float bcn1[2][8]; /* count of 1 bits individually */
 static float bcn0[2][8]; /* count of 0 bits individually */


 static long  vcnt[2][256]; /* byte value counts          */
 static long  vdis[2][256]; /* byte value distance        */
 static long  vlst[2][256]; /* byte value last            */


 /* Closeness of bits of same type */

 static float b0d[2];     /* bit zero distance            */
 static float b1d[2];     /* bit one distance             */
 static float b0c[2];     /* bit zero count               */
 static float b1c[2];     /* bit one count                */
 static int lastone, lastzero;

 float  average;
 float  variance;
 float  bit0ave, bit1ave;


 int i,j;


 if (inflag==-1) /* Reset statistics */
  {
    sum[0]=0.0;
   ssum[0]=0.0;
   bit0[0]=0.0;
   bit1[0]=0.0;
    sum[1]=0.0;
   ssum[1]=0.0;
   bit0[0]=0.0;
   bit1[0]=0.0;

   b0d[0]=0.0;
   b1d[0]=0.0;
   b0c[0]=0.0;
   b1c[0]=0.0;

   b0d[1]=0.0;
   b1d[1]=0.0;
   b0c[1]=0.0;
   b1c[1]=0.0;

   for (i=0; i<7; i++)
    {bcn1[0][i]=0.0;
     bcn1[1][i]=0.0;
     bcn0[0][i]=0.0;
     bcn0[1][i]=0.0;
    }

   for (i=0; i<=255; i++)
    {
     vcnt[0][i]=0L; /* byte value counts          */
     vdis[0][i]=0L; /* byte value distance        */
     vlst[0][i]=0L; /* byte value last            */
     vcnt[1][i]=0L; /* byte value counts          */
     vdis[1][i]=0L; /* byte value distance        */
     vlst[1][i]=0L; /* byte value last            */
    }
    lastone=0;
    lastzero=0;


   return;
  }

 if (inflag==-2) /* Display stats */
  {



  puts("");
  puts("---------------------------------------------------------------");
  puts("        Statistical Incoming Random Number Report:");
  puts("---------------------------------------------------------------");
  puts("");
  printf("Number of incoming values     : %f\n",cnt[0]);
  printf("Sum of incoming values        : %f\n",sum[0]);
  printf("Sum of squared incoming values: %f\n",ssum[0]);


  if (cnt[0]!=0.0)  /* Prevent divide by zero */
  {
  average= (sum[0]/cnt[0]);
  variance=(ssum[0])-(average*average);  /* Not too sure of this!*/
  variance=variance/cnt[0];

  printf("Average incoming value        : %f\n",average);
  printf("Variance of incoming values   : %f\n",variance);
  }

  bit0ave=b0d[0]/b0c[0];
  bit1ave=b1d[0]/b1c[0];

  printf("Ave dist bit=0 to next bit=0  : %f\n",bit0ave);
  printf("Ave dist bit=1 to next bit=1  : %f\n",bit1ave);

  printf("Count of all bits set to zero : %f\n",bit0[0]);
  printf("Count of all bits set to one  : %f\n",bit1[0]);

  for (i=0; i<=7; i++)
     printf("Count of bit %d set to 0/1  : %f/%f\n",i,bcn0[0][i],bcn1[0][i]);

  puts("");
  puts("");

  puts("<MORE>");
  getch();  /* Wait for a keypress */

  puts("");
  puts("---------------------------------------------------------------");
  puts("        Statistical Outgoming Random Number Report:");
  puts("---------------------------------------------------------------");
  puts("");

  printf("Statistical bit fixing mode   : ");
  if (bitfix) puts("On"); else puts("Off");

  printf("Number of outgoing values     : %f\n",cnt[1]);
  printf("Sum of outgoing values        : %f\n",sum[1]);
  printf("Sum of squared outgoing values: %f\n",ssum[1]);


  if (cnt[1]!=0.0)  /* Prevent divide by zero */
  {
  average= (sum[1]/cnt[1]);
  variance=(ssum[1])-(average*average);  /* Not too sure of this!*/
  variance=variance/cnt[1];

  printf("Average outgoing value        : %f\n",average);
  printf("Variance of outgoing values   : %f\n",variance);
  }

  bit0ave=b0d[1]/b0c[1];
  bit1ave=b1d[1]/b1c[1];

  printf("Ave dist bit=0 to next bit=0  : %f\n",bit0ave);
  printf("Ave dist bit=1 to next bit=1  : %f\n",bit1ave);


  printf("Count of all bits set to zero : %f\n",bit0[1]);
  printf("Count of all bits set to one  : %f\n",bit1[1]);

  for (i=0; i<=7; i++)
     printf("Count of bit %d set to 0/1  : %f/%f\n",i,bcn0[1][i],bcn1[1][i]);

  puts("");
  puts("");

  puts("<MORE>");
  getch();


  puts("");
  puts("---------------------------------------------------------------");
  puts("     Statistical Byte Value Distance / Frequency Report:");
  puts("---------------------------------------------------------------");
  puts("");

  puts("Value         InCount InAveDist OutCount OutAveDist");
  puts("---------------------------------------------------------------");

  for (i=0; i<=255; i++)
  {
   printf("%8d   %8ld %8ld %8ld %8ld\n",i,
	  vcnt[0][i],(long) (vdis[0][i]/vcnt[0][i]),
	  vcnt[1][i],(long) (vdis[1][i]/vcnt[1][i]));

   if ((i & 15)==15) {puts("<MORE>"); getch();};

  }


  puts("");
  return;
  }

   sum[inflag]+=val;           /* Sum         */
  ssum[inflag]+=(val*val);     /* Squared sum */


  /* Bit statistics */

  for (i=1,j=0; j<=7; j++,i=i*2)
    if ((val & i)!=0)
     { bcn1[inflag][j] +=1;       bit1[inflag]+=1;
       b1d[inflag]+=lastone;
       b1c[inflag]+=1;
       lastzero++;
       lastone=0;
     }
    else
     { bcn0[inflag][j] +=1;       bit0[inflag]+=1;
       b0d[inflag]+=lastzero;
       b0c[inflag]+=1;
       lastone++;
       lastzero=0;
     }

  /* Byte valued statistics */
  vcnt[inflag][val]++;
  vdis[inflag][val]+=vlst[inflag][val];
  /* Increase distance for other bytes */
  for (i=0; i<=255; i++) vdis[inflag][i]++;
  vlst[inflag][val]=0L;


  cnt[inflag]+=1;
}


/*-------------------------------------------------------------------*\
| FUNCTION: taken                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: See if a bit in the window is taken.                       |
|                                                     		      |
|                                                                     |
| PARAMS:  victim byte, bit                                           |
|                                                                     |
| RETURNS: 0 if bit is unused, 1 if taken                             |
|                                                                     |
|                                                                     |
\*-------------------------------------------------------------------*/



int taken(int vbyte, int vbit) /* Is a bit taken */
{
 int k;
 int collisionflag=0;

  for (k=0; k<7 && !collisionflag; k++)
    if (DataByte[k]==vbyte && DataBit[k]==vbit)
      collisionflag=1;

return collisionflag;
}




/*-------------------------------------------------------------------*\
| FUNCTION: victimbit                                                 |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Randomly pick a victim bit to be changed by the            |
|          statbitfix function.                       		      |
|                                                                     |
| PARAMS:  victim bit, byte, and value prefrence                      |
|                                                                     |
| RETURNS: byte in stream to change, and bit value to alter           |
|                                                                     |
| NOTE:    This function is useful for statistics against the count   |
|          of bits that are 0 or 1, but not their place.              |
|                                                                     |
|          (ie: if bit 128 is set to 1, bit 3 may be the victim,      |
|           while bit 128 of another byte is a more prefered victim)  |
|                                                                     |
\*-------------------------------------------------------------------*/

void victimbit(int *vbit, int *vbyte, int setting, int bval)
{

 int collisionflag;
 int i, qbit, k, q=0;


/* How long should we keep trying to find a victim? */
#define MAXVICTIMPICK 100



 /* See if we can find unused bit with same value in the
    stream. If so, we change that one. */

 for (i=0; i<=maxchnl; i++)
  if (taken(i,bval)==0 && (stream[i] & bval)!=setting )
  {
   *vbit=bval;
   *vbyte=i;
   return;
  }


 do
  {
   *vbyte=0;

   collisionflag=0;

   q++;

   qbit=(rand() & 7);
   switch (qbit)
   {
    case 0: *vbit =   1; break;
    case 1: *vbit =   2; break;
    case 2: *vbit =   4; break;
    case 3: *vbit =   8; break;
    case 4: *vbit =  16; break;
    case 5: *vbit =  32; break;
    case 6: *vbit =  64; break;
    case 7: *vbit = 128; break;
   }

   *vbyte=(rand() % maxchnl);


   collisionflag=taken(*vbyte,*vbit);

  /* We want a bit whose value is different than the one we set */
  if ( ((xstream[*vbyte] & (*vbit))!=0)  == (setting!=0) )
     collisionflag=1;

 } while (collisionflag!=0 && q<MAXVICTIMPICK);


 if (collisionflag)
   {
    *vbit=-1; *vbyte=-1;
   }


 return;
}





/*-------------------------------------------------------------------*\
| FUNCTION: statbitfix                                                |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 04/09/1994                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Fix bits of the random stream to give same/similar         |
|          statistics as infile random noise.   		      |
|                                                                     |
| PARAMS:  none                                                       |
|                                                                     |
| RETURNS: nothing                                                    |
|                                                                     |
\*-------------------------------------------------------------------*/

void statbitfix(void)
{
int i,j, vbit, vbyte;

 for (i=0; i<maxchnl; i++) /* DataByte processing loop */
  for (j=1; j<=128; j=j<<1)  /* DataBits processing loop */
   if ((xstream[i] & j)!=(stream[i] & j))
   {
     /* Find a victim bit */
     victimbit(&vbit,&vbyte, (stream[i] & j),j );
     if (vbyte>0)  /* If found */
      {
	/* filter victim bit to zero, then OR the old value*/
	stream[vbyte]=stream[vbyte] & (255 ^ vbit);

	/* set victim bit if old one was set*/
       if ( (xstream[i] & j)!=0 )
	  stream[vbyte]=stream[vbyte] | vbit;
      }
    }

}






/*-------------------------------------------------------------------*\
| FUNCTION: lrotate                                                   |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Rotate a character (8 bit word) to the left.               |
|                                                                     |
| PARAMS:  in - character to rotate                                   |
|          times - number of times to rotate the character            |
|                                                                     |
| RETURNS: roatated character                                         |
|                                                                     |
| NOTE:    This may need to be modified on non-80x86 machines since   |
|          it assumes that the leftmost bit is the highest.           |
|                                                                     |
\*-------------------------------------------------------------------*/

unsigned char lrotate(char in, int times)
{
unsigned char ch;
int i;

collision(472);

ch = in;
for (i=0; i<=(times & 7)+1; i++)
 if (ch & 128)
	ch=((ch<<1) | 1);
 else
	ch=(ch<<1);
collision(480);
return(ch);
}






/*-------------------------------------------------------------------*\
| FUNCTION: rrotate                                                   |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Rotate a character (8 bit word) to the right.              |
|                                                                     |
| PARAMS:  in - character to rotate                                   |
|          times - number of times to rotate the character            |
|                                                                     |
| RETURNS: rotated character                                          |
|                                                                     |
| NOTE:    This may need to be modified on non-80x86 machines since   |
|          it assumes that the leftmost bit is the highest.           |
|                                                                     |
\*-------------------------------------------------------------------*/


unsigned char rrotate(char in, int times)
{
unsigned char ch;
int i;
collision(516);

ch = in;
for (i=0; i<=(times & 7)+1; i++)
 if (ch & 1)
	ch=((ch>>1) | 128);
 else
	ch=(ch>>1);

collision(525);
return(ch);
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

void thermometer()
{
 unsigned long a,i;

 char filled='Û';
 char empty ='±';

 collision(553);


 if (trace) return;

/*-----For non DOS systems, swap these blocks.----------------------
 char filled='#';
 char empty ='_';
 ------------------------------------------------------------------*/

 if (fsize<=0)
	return;

 printf("\r");

 a=(20*ftell(infile)/fsize);

 for (i=0; i<=20; i++)
  if (i<=a)
	 putchar(filled);
  else
	 putchar(empty);
collision(572);
}





/*-------------------------------------------------------------------*\
| FUNCTION: prexor                                                    |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Preliminary exclusive or encryption of a character.        |
|                                                                     |
| PARAMS:  in - character to rotate                                   |
|          times - number of times to rotate the character            |
|                                                                     |
| RETURNS: encrypted character                                        |
|                                                                     |
| NOTE:    This may be changed at will, however, customized versions  |
|          will be incompatible with public versions.                 |
|                                                                     |
\*-------------------------------------------------------------------*/


unsigned char prexor(char in, int index)
{

 /* Do some preliminary XOR / bit-shift scrambling of the character
    with the password key.  For further security this function may
    be replaced with calls to something like a DES or RSA
    encryption function. */


 char ch, q;

 collision(629);

 ch=in;

 ch=(ch ^ password[index]);


 ch=(index & 1 ? ch^0x55 : ch^0xaa);
 ch=(password[index] & 2 ? ch^0x1f :  ch^0xf3);


 q =(password[index] & 7)+1;
 ch=(ch ^ (password[index]>>q));
 ch=(ch ^ (password[index]<<q));

 oldxor = ch;


 collision(647);

 return(ch);
}



/*-------------------------------------------------------------------*\
| FUNCTION: seedpassword                                              |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Do the encryption.                                         |
|                                                                     |
| MODIFIES: password, command, maxchannel index pointers, data bits,  |
|           data byte index.                                          |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
| NOTE:    This function seeds the stream channels with data based on |
|          the password for initiating encryption/decryption          |
|                                                                     |
\*-------------------------------------------------------------------*/


void seedpassword(void)
{

unsigned int q,i;
unsigned char ch,a;

 ch=0x55;
 q=strlen(password)-1;


 oldxor = 0xff;


 DataBit[7]=1;            /* These are bit pointers, their values    */
 DataBit[5]=2;            /* change as the encryption processes      */
 DataBit[3]=4;            /* text.  These values must NEVER be the   */
 DataBit[1]=8;            /* same to each other or the process will  */
 DataBit[0]=16;           /* fail.  These values are shuffled by     */
 DataBit[2]=32;           /* swaps as we go along.                   */
 DataBit[4]=64;           /* You may modify the order of these bits  */
 DataBit[6]=128;          /* to produce a non-standard private crypt.*/
			  /* The values must be 1,2,4,8,16,32,64,128 */
			  /* in any order.                           */

 /* If you're going to change the above, change the order.  IE: by
    default, we are using a skip method starting from the center,
    we have 0, then up we have 1, then below 0 we have 2, then above
    1 we have 3, etc.  Any initial order may be used as desired for
    private versions. */

 maxchnl=limitchnl;
 collision(709);

 passkey=0;
 for (i=0; i<=7; i++)
  DataByte[vlx(i,713)]=1;


 command=0;
 collision(719);


 for (i=0; i<q; i++)
  ch=prexor(ch,(q-i));

 /* Change 0xa5 for a private password, and the params to the rotates. */
 passkey=(0xa5 ^ ch) & 0xff ;

 for (i=0; i<q; i++);
   {
    passkey = passkey ^ lrotate(password[i],i+1);
    passkey = passkey ^ rrotate(password[q-i],i);
    passkey = passkey ^ ch;
   }

 /* Don't change the 0xff as it is used to filter the lower byte */
 ch = (passkey & 0xff);
 a=((3+ch) % (limitchnl+1));
 maxchnl = a;

 if (maxchnl>limitchnl)
	maxchnl=limitchnl;


 /*-----------------------------------------*/
 /* These are pointers that point into a    */
 /* data window.  They represent the channel*/
 /* number in which a particular bit resides*/
 /* in.  These values >MAY< be the same, but*/
 /* they must be less than the limitchnl    */
 /* value.  By swapping the bit positions   */
 /* and which channel they reside in, we    */
 /* provide a very powerful data hiding     */
 /* encryption.                             */
 /*-----------------------------------------*/

 for (i=0; i<=7; i++)
  DataByte[vlx(i,762)]=( password[i % (q+1)]) % (maxchnl+1);

 /*-----------------------------------------*/
 /* i % q+1                                 */
 /*                                         */
 /*        will ensure that we if we have a */
 /*        password shorter than 8 bytes,   */
 /*        (bad idea,) we'll use the first  */
 /*        bytes again.  Change  "i"  for a */
 /*        private version as needed.       */
 /*                                         */
 /* x % (maxchnl+1)                         */
 /*                                         */
 /*        will set the data bits in bytes  */
 /*        within the data channel and make */
 /*        sure they're less than limitchnl.*/
 /*        Never change this!               */
 /*-----------------------------------------*/




 command=(( (passkey>>3 ) &0x7f ) % (maxchnl + 1));

collision(770);
}







/*-------------------------------------------------------------------*\
| FUNCTION: nextpasscommand                                           |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Follow the orders of the command channel for next pass     |
|          This is the heart of the White Noise Storm Encryption!     |
|                                                                     |
| PARAMS:  password index                                             |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
| NOTE:    You may modify these values for a private, more secure, but|
|          incompatible version of this program.  Make sure that you  |
|          also change the header line.                               |
|                                                                     |
\*-------------------------------------------------------------------*/


void nextpasscommand(int index)
{

 unsigned char ch, ccl, cd1, cd2, cd3, cd4, rl, rr, ri, rx;
 unsigned long q,l,aff, oo;
 int i, s1, s2,  il, ih, cq;


 /*---------------------------------------------------------------------
   Figure out the next channel for the swap to be done by the command
   channel. The key is generated by arbitrary xor's,  shifts and rotates,
   then trimmed to a byte by and'ing it with 255.  These operations
   may be modified for a private version.
   -------------------------------------------------------------------*/
collision(818);
 oo  = maxchnl;
 ch  = stream[command] >>5;
 ccl = stream[command] & 0x0f;

 cd1 = stream[DataByte[0]];
 cd2 = stream[DataByte[1]];
 cd3 = stream[DataByte[2]];
 cd4 = stream[DataByte[3]];



 /* Prevent bit-changing analysis */
 for (i=0,cq=0; i<maxchnl; i++)
  cq=( cq<<(i+1) + stream[i]*(i+1) ^ stream[maxchnl-i]) % 0xff;


 rl  = lrotate(cd1,DataByte[4] + 1);
 rr  = rrotate(cd2,DataByte[5] + 7);
 ri  = lrotate(cd3,DataByte[6] + 2);
 rx  = rrotate(cd4,DataByte[7] + passkey);

 aff = (password[index] ^ ch)^ ccl;   /* Build change key             */
 aff = (aff ^ rl);
 aff = (aff ^ rr);
 aff = (aff ^ ri);
 aff = (aff ^ rx);
 aff = (aff ^ cq);

 collision(839);

 aff = (aff & 0xff);                  /* Trim change key to byte size */


 /*---------------------------------------------------------------------

   Build next channels for output and for max channel sizes by using
   the change key, arbitrarily xoring the change key with a bit pattern
   (f -> 1111  a->1010), multiplying it by the maximum size for the
   channel, and dividing it by 256 (the max value for a char.)

   The XOR value of 0xaa and 0xff may be changed for private versions.

   -------------------------------------------------------------------*/

 l = ((aff^0xaa) * (limitchnl) / 256 );
 q = ((aff^0xff) * (maxchnl )  / 256 );


collision(861);

 /*------- Assign the commanded channel to the new value -------------*/

 il=-1; /* By default we won't swap the bits */
 ih=-1;

 collision(868);

 /* The operations here in case 0 and 1 may be changed in private versions,
    however ih and il must result in values between 0 and 7!

    Case 2 and 3 may be swapped in private versions if it is desired.

 */

 switch (ch & 3)
 {
  case 0:  il=abs( ( (aff) >>3) & 0x07); ih= aff & 0x07; break;
  case 1:  ih=abs( ( (aff^rr) >>2) & 0x07); il= aff & 0x07; break;
  case 2:  command = q; break;
  case 3:  maxchnl = (l % (limitchnl +1)); break;
 }

collision(892);


/* Change three data byte indexes*/
 DataByte[(rx ^l) & 7]= cq   % (maxchnl+1);
 DataByte[(cq ^3) & 7]= aff  % (maxchnl+1);
 DataByte[(rx^cq) & 7]= rx   % (maxchnl+1);



 /*- do a bit swap 50% of the time, ie: if il was changed in case 0 or 1 --*/
 if (il >0 && il < 7)
  {
   s1=DataBit[vlx(il,855)];
   s2=DataBit[vlx(ih,856)];

   DataBit[vlx(ih,857)]=s1;
   DataBit[vlx(il,858)]=s2;
   collision(979);
  }



 /*--- Every once in a while (25% of the time), we'll rotate all the bits */
 /* The value of 0x0f and 4 in the if statement can be changed for private
    versions.  This represents 4 out of 16 possible values, thus 4/16 is
    25% of the time                                                       */

 collision(989);

 if ((rl & 0x0F)<4)
 {

   /* Should we rotate'em right, or left? */


  if ((rr & 0x02)==0)
  {
   /* Every once in a while, swap odd bits with their neighbors */
   if (rr & 0x04)
   {
    collision(999);
    for (i=0; i<=6; i+=2)
    {
     s1=DataBit[vlx(i,880)];
     s2=DataBit[vlx(i+1,881)];
     DataBit[vlx(i,882)]=s2;
     DataBit[vlx(i+1,883)]=s1;
     collision(1004);
    }
   }
   else
   if (rr & 0x08)
   {
    collision(1011);
    /* Every once in a while mirror bits around like a palindrome */
    for (i=0; i<=4; i++)
    {
     s1=DataBit[vlx(i,892)];
     s2=DataBit[vlx(i^0x07,893)];
     DataBit[vlx(i^0x07,894)]=s1;
     DataBit[vlx(i,895)]=s2;
     collision(1016);
    }


   }

  }

  else

  {
   /* The deciding bit 0x01 may be changed to another bit for private
   versions, but must be a power of 2.  ie: 1,2,4,8,16,32,64, or 128. */
   if ((rr & 0x01)==0)
     { /* Rotate left */

	collision(1034);
	s1=DataBit[7];
	DataBit[7]=DataBit[6];
	DataBit[6]=DataBit[5];
	DataBit[5]=DataBit[4];
	DataBit[4]=DataBit[3];
	DataBit[3]=DataBit[2];
	DataBit[2]=DataBit[1];
	DataBit[1]=DataBit[0];
	DataBit[0]=s1;


       collision(1036);
     }
   else
     {
       /* Rotate bits right*/
       collision(1051);

       s1=DataBit[0];
	DataBit[0]=DataBit[1];
	DataBit[1]=DataBit[2];
	DataBit[2]=DataBit[3];
	DataBit[3]=DataBit[4];
	DataBit[4]=DataBit[5];
	DataBit[5]=DataBit[6];
	DataBit[6]=DataBit[7];
	DataBit[7]=s1;

       collision(1044);
     }
   }

 }






 /*---maxchnl must be at least 2 bytes wide, at most limitchnl wide---*/

 if (maxchnl<2)
	maxchnl=2;

 if (maxchnl>limitchnl)
	maxchnl=limitchnl;


 collision(988);
 /*---------------------------------------------------------------------
   If maxchnl should shrink, it's possible that data1, data2 or command
   may wind up being outside of the range of maxchnl.  This portion of
   the code checks for this situation and corrects it by shrinking any
   channel which is over the size specified by maxchnl
 ---------------------------------------------------------------------*/


 if (command > maxchnl)
    command=(command * (maxchnl) / oo);

collision(1000);

 /*---------------------------------------------------------------------
   Set up the next set of data byte locations.
 ---------------------------------------------------------------------*/

 for (i=0; i<=7; i++)
{


  if ( (((i^rx)^ri)+rl) & 1)
    DataByte[vlx(i,963)]=( ( ((i+1)<<8) ^ ch) ^aff   )%(maxchnl + 1);
  else
    DataByte[vlx(i,965)]=( ( ((cd1+2)<<2) ^ cd4) ^rr )%(maxchnl + 1);

collision(1015);
}
 /* The decision in the IF statement can be changed. The values used
    for XOR and the shifts used can also be changed for private
    versions.  DO NOT change %(maxchnl +1)! */





 /*------------System failure warnings--------------------------------*/


 if (command > maxchnl)
    printf("\nFAILURE: COMMD: %d, MAX: %d  %lu\n",command, maxchnl,ftell(infile));


 for (i=0; i<7; i++);
  if (DataByte[vlx(i,983)]>maxchnl
		  || DataByte[vlx(i,984)]<0)
    {
     perror("Databyte error.");
    }

collision(1039);
}





/*-------------------------------------------------------------------*\
| FUNCTION: encrypt                                                   |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Do the encryption.                                         |
|                                                                     |
| PARAMS:  password                                                   |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
| NOTE:    This function uses several of the above functions.  It is  |
|          NOT a stand alone function.                                |
|                                                                     |
\*-------------------------------------------------------------------*/


void encrypt()
{
 /* Index pointers to actual data */

 unsigned char ch, ich;

 unsigned int a,i,j;
 unsigned int index;

 unsigned int command;
 unsigned char refresh;


 randomize();

 ch=0; index=0;
 puts("Encrypting...");


 if (!stealth)
 {
 /*-------------------------------------------------------------------

  Put a text header line, followed by an EOF character.  The purpose
  of this is so that when we try to decrypt a file, we can check for
  the header, and also if someone should try to type or cat the file,
  they will see the header and not binary garbage.

  -------------------------------------------------------------------*/

 fwrite(header,strlen(header)+1,1,outfile);

 /*-------------- Unix and DOS file terminators. --------------------*/

    fputc((char) 0x00,outfile);    /* Output a NULL */
    fputc((char) 0x1A,outfile);    /* DOS EOF       */
    fputc((char) 0x04,outfile);    /* Unix EOF      */

    fseek(outfile,(long) STARTBYTE,SEEK_SET);

 /*-------------- Write user select limitchnl -----------------------*/

    fputc((char) (limitchnl & 0xff), outfile);

 }

 /*---- Generate command, data and maxchnl from password ------------*/

 index=0;
 seedpassword();
 ch = 0; index=0; oldxor = 0;

collision(1135);
thermometer();
collision(1137);
refresh=0;

while(!feof(infile) && (ftell(infile) < fsize))
{

collision(1127);
 /*-------------- Seed all channels with random garbage -------------*/
 for (i=0; i<=maxchnl; i++)
  {
   collision(1186);
   stream[i]= getrand();  /* **** Replace with a better rand fn*/
   xstream[i]=stream[i];
   if (statson) statistics(0, stream[i]);
   collision(1190);
  }


 /*------------------------ Read in a byte --------------------------*/
 errno=0;
 collision(1156);
 ch=fgetc(infile);
 collision(1158);
 ich = ch;
 collision(1160);
 ch = ch ^ oldxor;
 collision(1162);
 if (errno)
 {
  perror("Error reading from input file");
  return;
 }


 /*----------- Display thermometer bar every 256 bytes --------------*/

 if (refresh++==0)
   {
    collision(1174);
    thermometer();
    collision(1176);
   }


 /*-------------- Encrypt the char with password --------------------*/


 ch=prexor(ch,index);
 collision(1184);
 index++;

 if (index==strlen(password)) index=0;

 collision(1189);

 /*------------ Split ch into data channels -------------------------*/

 i=0;
 for (j=1,i=0; i<=7; )
  {

  collision(1169);
  /* clear target bit, assume bit is 0 until we test it. */
  a=stream[DataByte[vlx(i,1120)]];
  a=(a  & (0xff^DataBit[vlx(i,1121)]));

  /* If the bit is set in the data, set it in the stream */

  collision(1176);
  if ((ch & j)!=0)
     a=(a | DataBit[vlx(i,1126)]);

  collision(1180);
  stream[DataByte[vlx(i,1131)]]=a;
  collision(1182);

  j=j*2; ++i;
  }


  if (bitfix) statbitfix();  /* Statistical bit fix routine */


  if (statson)                /* Do statistics if on */
    for (i=0; i<=maxchnl; i++)
	statistics(1, stream[i]);



 /*--------------- Write the stream out to the disk -----------------*/

 errno=0;





 traceout();
 fwrite(stream,maxchnl+1,1,outfile);
 if (errno)
  {
   perror("Error writing data");
   return;
  }



 /*---------- Set up the next oldxor for recursive encryption -------*/

 oldxor =ich;


 /*--------- Interpret the command, and prepare for next byte -------*/

  nextpasscommand(index);

}
collision(1242);
thermometer();  /* Flash thermometer out for effect.  */
collision(1224);
return;
}





/*-------------------------------------------------------------------*\
| FUNCTION: decrypt                                                   |
| ------------------------------------------------------------------- |
|                                                                     |
| Written  by: Ray Arachelian   on 07/14/1992                         |
| Modified by:                  on                                    |
| Reason for modification:                                            |
|                                                                     |
| ------------------------------------------------------------------- |
|                                                                     |
| PURPOSE: Do the decryption.                                         |
|                                                                     |
| PARAMS:  password                                                   |
|                                                                     |
| RETURNS: nothing.                                                   |
|                                                                     |
| NOTE:    This function uses several of the above functions.  It is  |
|          NOT a stand alone function.                                |
|                                                                     |
\*-------------------------------------------------------------------*/


void decrypt()
{
 /* Index pointers to actual data */

 char ch;
 int i,j;
 unsigned int index;

 unsigned int command;
 unsigned short refresh;

 randomize();
 ch=0; index=0;
 puts("Decrypting...");


 if (!stealth)
 {
 /*----------------------- Read the header line ---------------------*/

     fread(junk,strlen(header)+1,1,infile);  /* Read the header line */

     fgetc(infile);                      /* Ignore DOS and UNIX EOF  */
     fgetc(infile);
     fseek(infile,(long) STARTBYTE,SEEK_SET);


 /*------------ Read in user defined channel limit ------------------*/

     ch=fgetc(infile);
     limitchnl = ch;

  if (limitchnl<2 || limitchnl>(MAXSTREAM-1))
     {
      fprintf(stderr,"Bad max channel size value in file!\n");
      return;
     }

 /*------------------------ Check header line -----------------------*/

 if (strcmp(junk,header)!=0)
	{
	 puts("This is not a White Noise Storm 1.0 encrypted file.");
	 return;
	}

 }

 /*------Seed the command, data, and maxchnl w/password-------------*/

 index=0;
 ch=0;
 seedpassword();
 oldxor = 0;


collision(1328);
thermometer();
collision(1330);
refresh=0;

while( !feof(infile) && (ftell(infile)+maxchnl) < fsize )
{
 /*--------------------- Read the data streams ----------------------*/

 errno=0;
 fread(stream,maxchnl+1,1,infile);
 if (errno)
 {
  perror("Error reading from input file");
  return;
 }
 traceout();


 /*------- Display thermometer bar every 256 bytes of input ---------*/

 refresh += maxchnl+1;
 refresh = refresh & 0xff;

 if (refresh++==0)
    {
     collision(1354);
     thermometer();
     collision(1356);
    }

 /*--------------- Restore ch from data1 and data2 ------------------*/

 ch=0;

 for (j=1,i=0; i<=7;)
  {

  /* If the bit is set, or it with our character.     */

  if (stream[DataByte[vlx(i,1291)]] & DataBit[vlx(i,1291)])
     ch=ch | j;


  j=j*2; i++;
  }




 /*------- Decrypt ch with the password and last char decrypted -----*/

 ch=ch ^ oldxor;
 ch=prexor(ch,index);


 /*------------------ Increment password index ----------------------*/

 index++;
 if (index==strlen(password))
	index=0;


 /*----------------- Output the decrypted byte ----------------------*/

 errno=0;
 fputc(ch,outfile);
 oldxor = ch;
 if (errno)
  {
   perror("Error writing data");
   return;
  }

 /*--------- Interpret the command, and prepare for next byte -------*/

 nextpasscommand(index);


}
 collision(1421);
 thermometer();      /* Flash last thermometer for effect */
 return;
}





void help()
{
 puts("");
 puts("Usage: WNSTORM cmd infile outfile password {options}");
 puts("");
 puts("Where cmd: is e for encrypt, and d is for decrypt");
 puts("   infile: is the file to encrypt or decrypt");
 puts("  outfile: is the file to create");
 puts(" password: is the password to encrypt/decrypt the input file");
 puts("");
 getch();
 puts("Options:");
 puts("       -s: overrides the default 5 character window size.");
 printf("           -s2 is the minimum, -s%d is the maximum. Using\n\r",MAXSTREAM-1);
 puts("           larger size values will enlarge the size of the");
 puts("           output file, but will also increase security.");
 puts("       -v  View Statistics (only for encryption)");
 puts("       -b  Statistical bit fix (only for encryption)");
 puts("       -t  Trace channels (for debugging)");
 puts("       -n  no header (stealth mode)");
 puts("");
 puts("       -rf:filename");
 puts("           Use random data from filename for random number source");
 puts("       -rd:devname");
 puts("           Use random data from infinite random number generator device");
 puts("");
 puts("");

 return;
}



int main(int argc, char *argv[])
{
 char mode, swit;
 int pos, i;


 puts("");
 puts("-----------------------------------------------------------------------");
 puts("    WNSTORM(tm) V1.01 Copyright (C) 1994 by Ray (Arsen) Arachelian,");
 puts("                         All Rights Reserved");
 puts("-----------------------------------------------------------------------");
 puts("");
 puts("                   White Noise Storm (tm) Encryptor");
 puts("");
 puts("");
 puts("DO NOT EXPORT THIS SOFTWARE OUTSIDE OF THE UNITED STATES OF AMERICA!");
 puts("");

 limitchnl = 5;
 rsize=0;

 /*------------------ Check number of parameters --------------------*/

 if (argc<5)
  {
    if (argc>1)
       puts("Wrong number of parameters");
    help();
    return(1);
  }


 /*--------------- Check for valid command or help request ----------*/

 mode=toupper(argv[1][0]);
 if (mode=='-' || mode =='/')
    mode=toupper(argv[1][1]);

 if ( mode!='E' && mode!='D' )
    {
     if (mode!='H' && mode!='?')
	fprintf(stderr,"Unrecognized command %c in parameter %s\n",argv[1][0],argv[1]);
     help();
     return(1);
    }

 /*--------------- Check password size ------------------------------*/

 if (strlen(argv[4])<5)
    {
     fprintf(stderr,"Password must be at least 5 characters long!\n");
     help();
     return(1);
    }

 if (strlen(argv[4])>150)
    {
     fprintf(stderr,"Password must be less than 150 characters long!\n");
     help();
     return(1);
    }



 /*--------------- Start interpreting other parameters --------------*/

 if (argc>5)
  for (i=5; i<argc; i++)
   {
   swit =toupper(argv[i][0]);
   pos=1;

   if (swit=='-' || swit=='/');
     {
      swit=toupper(argv[i][1]);
      pos=2;
     }

   switch (swit) {

   case 'S': {
	      sscanf(argv[i]+pos,"%d",&limitchnl);
	      if (limitchnl<2 || limitchnl>=MAXSTREAM)
		 {
		  fprintf(stderr,"Bad max size value. Must be between 2 and %d\n",MAXSTREAM-1);
		  return(1);
		 }
	     } break;

   case 'T': trace=1; break;

   case 'B': bitfix=1; break;

   case 'V': statson=1; break;

   case 'N': stealth=1; break;

   case 'R': if ( argv[i][2]==':' && (toupper(argv[i][1])=='F' || toupper(argv[i][1])=='D'))
	     {
	      errno=0;
	      rndfile=fopen(&argv[i][3],"rb");
	      if (rndfile==NULL || errno)
	       {
		perror("Can't open random data input file/device");
		return(1);
	       }

	      if (argv[i][1]=='F')
	       { fseek(rndfile,0L,SEEK_END);
		 rsize=ftell(rndfile);
		 fseek(rndfile,0L,SEEK_SET);
	       }
	      else
		 rsize=MAXLONG;
	     }


   default:
    fprintf(stderr,"Unrecognized option %c in param %s\n",swit, argv[i]);
    return(1);
   }
  }



 if (statson && mode!='E')
  {
   puts("");
   puts("The View statistics option can only be used durring encryption.");
   puts("It will be ignored.");
   puts("");
  }

 if (bitfix && mode!='E')
  {
   puts("");
   puts("The BitFix option can only be used durring encryption.");
   puts("It will be ignored.");
   puts("");
  }





 /*------------------------ Open files ---------------------------*/
 errno=0;
 infile  = fopen(argv[2],"rb");
 if (errno)
 {
   errno=0;
   infile  = fopen(argv[2],"rb");
   if (errno)
   {
    perror("Can't open input file");
    return(1);
   }
 }

 /*-----------------Get file size of input file-------------------*/

 fseek(infile,0L,SEEK_END);
 fsize=ftell(infile);
 fseek(infile,0L,SEEK_SET);


 /*----------------- Open output file          -------------------*/
 errno=0;
 outfile = fopen(argv[3],"wb");
 if (errno)
 {
  errno=0;
  outfile = fopen(argv[3],"wb");
  if (errno)
   {
  perror("Can't create output file");
  return(1);
   }
 }


 /*--------------- Copy the password arguement -------------------*/

 strcpy(password,argv[4]);


 /*---------------- Do whatever needs to be done -----------------*/
 if (mode=='E')
    {
     if (statson) statistics(-1,-1);  /* Clear statistics values */
     encrypt();
     if (statson) statistics(-2,-2);  /* Display statistics */
    }
 else
     decrypt();


 /*----------- Let's clean up after ourseleves shall we? ---------*/

 fcloseall();
 printf("\r                                  \n");
 return(0);

}
