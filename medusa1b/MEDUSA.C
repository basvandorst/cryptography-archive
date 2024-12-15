/**************************************************************************\
*                                                                          *
*                         MEDUSA's Tentacles (tm)                          *
*                                                                          *
*             SOURCE CODE REVISION ALPHA-940821-16:00 EST                  *
*                                                                          *
*                                                                          *
*    DEVEL:[ ]   ALPHA/DEVEL: [X]   BETA TESTING:[ ]    PRODUCTION:[ ]     *
*                                                                          *
*               Compiled with Borland C++ 3.1, Huge Model.                 *
*                                                                          *
*  Except for the use of the C++ style comments, this should compile on    *
*  any plain vanilla C compiler.                                           *
*                                                                          *
*==========================================================================*
*                                                                          *
*               Copyright (C) 1994 by Ray (Arsen) Arachelian,              *
*          All Rights Reserved, General Technical Data Restricted.         *
*                                                                          *
*==========================================================================*
*                                                                          *
*    WARNING: This source code is the sole property of the author and is   *
*    to be treated as such.  The use of this source code is subject to     *
*    certain limitations stated in the documentation and in the conditions *
*    of the license described in this file.  Using this source code or a   *
*    machine executable version of this source code is subject to these    *
*    conditions.                                                           *
*                                                                          *
*    The ideas/algorithms of this software MAY not be patented by other    *
*    parties.  The author, at this time, has no interest in patenting any  *
*    of the algorithms in this software.  The purpose of this paragraph is * 
*    to protect these algorithms from being patented by others by          *
*    establishing them as "prior art" and making them useable by all.      *                                 
*                                                                          *
*    Ray (Arsen) Arachelian is the author and sole owner of MEDUSA(tm)     *
*    and maintains exclusive proprietary interests and exclusive           *
*    copyright for this forementioned program.                             *
*                                                                          *
*    The words "MEDUSA'S TENTACLES" and the shorter version "MEDUSA" are   *
*    trademarks  of the author reffering to this software and its source   *
*    code as well as any compiled versions thereof.                        *
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
*                   Don't bother suing me.   I'm not rich.                 *
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
* written consent of the author.  You may not use pieces of this software  *
* in your programs without the written consent of the author.  Should you  *
* find a bug, or make a major improvement in the way this software works,  *
* you are encouraged to contact the author.                                *
*                                                                          *
* If you develop similar software based on the concepts of this system,    *
* you must place the following notice in your software to be displayed     *
* wherever your copyright notice is displayed and in the documentation:    *
*                                                                          *
*     This software is based in part on Ray (Arsen) Arachelian's           *
*     `Medusa's Tentacles' software.                                       *
*                                                                          *
* You may modify this software in any way shape or for for your personal   *
* use as long as neither this license, nor any copyright/ownership notices *
* are removed, nor the name of the software is changed.                    *
*                                                                          *
* Should any statement in this license be deemed invalid, only that        *
* statement shall be revoked, leaving the rest of this license valid.      *
*                                                                          *
* This license terminates when the user destroys all copies of this        *
* software from all media on which it resides.  This license will be re-   *
* established should the user use/get another copy or version of Medusa(tm)*
*                                                                          *
* Usage of this software indicates an agreement to the above license.      *
*                                                                          *
\**************************************************************************/



/**************************************************************************\
*                                                                          *
*  Bugs to fix:      QuickSort.   [fixed??]                                *
*                                                                          *
*                                                                          *
*  Things to do:     1. Automatic Querry using fuzzy-KNN or similar alg.   *
*                                                                          *
*                    2. Fuzzybase should be automatically sorted in        *
*                       the middle of appending every few hundred lines    *
*                       to optimize performance [have to think this over]  *
*                                                                          *
*                    3. rtoken function is too slow, need to rewrite or    *
*                       optimize it somehow so it won't move text, but copy*
*                       it instead.                                        *
*                                                                          *
*                    4. Check acuracy of fuzzy set variables L1-L4         *
*                                                                          *
\**************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <conio.h>  /* for getch() */
#include <ctype.h>

#include <math.h>   /* for sqrtl & powl  [these are long double versions
		       of pow and sqrt.  If your compiler doesn't support
		       long doubles, change them to double and hope for
		       the best.


		       If your machine & compiler have support for an FPU
		       you should compile with the FPU option on for speed
		       rather than emulating floating point numbers.

		    */


#include "medusa.h"
#include "rtoken.h"
#include "qsort.h"



#define PIGS_DONT_FLY 1
// Well, they really don't. //



/* Number of files to break up our database into.  keep this number very
   small for DOS... say around 4 or 5..  Must not be greater than 255!


   A word's token is it's hashvalue in the high byte of a word,
   followed by a 24 bit record number into a file:


   ie: suppose for word w, hash=24, and recnum=63.  The token value is

      token=(hash & 255)<<24 | recnum;

   Because of this we can't use more than 255 files.  (Not that we'd want
   to use 255 files anyway!)  For unix 8 files is a good number to limit
   ourselves to.

*/

#define HASHFILES 5

/* How many bytes are in a fuzzybase header */
#define FZBHEADER 8




/* used by findword() */

#define WORDFIND       0
#define WORDINCMASTER  1
#define WORDINCCURRENT 2
#define WORDDELETE     3


masterheader *mh;

FILE *mheader;             /* Master header file */

FILE *flist[HASHFILES+1];    /* We'll hash our dictionary in upto many files
			      for speedier access. */

long  fsize[HASHFILES+1];    /* the sizes of each of out files   */


long getnumrec(long i);

void flopen(char *s);
void flclose(void);
void sortfzb(char *s);
void purgefzb(char *s);

long ahash(char *word, long div);
void eread(FILE *f, long recnum, entry *e);
void ewrite(FILE *f, long recnum, entry *e);

long maxrecs=0L;


long findword(char *word, int command);
long addword(entry *e);
void deleterecs(char *s);
void appendtofzb(char *fnfzb, char *fntext, int masterflag);
void extractset(char *, char *, char *);

void helpscreen(void);
void more(void);



// debugging stub
long double lsqrtl(long double x)
{


 // printf("Getting sqrt of %Lf\n",x);


 return (sqrtl(x));

 // return x;  // disable all sqrt operations.
}


long double lpowl(long double x, long double y)
{

 // printf("Squaring %Lf\n",x);

 if (y==2.0) return (x*x);

 return (powl(x,y));
}





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

 if (fsize<=0)  return;

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





/************************************************************************\
*                                                                        *
* getnumrecs(): Get number of records in a hash file                     *
*                                                                        *
\************************************************************************/

long getnumrec(long i)
{

 fflush(flist[i]);
 fseek(flist[i],0L,SEEK_END);
 fsize[i]=ftell(flist[i]);
 fseek(flist[i],0L,SEEK_SET);

 return (fsize[i]-FZBHEADER)/sizeof(entry);

}





/**************************************************************************\
* putdelim(): put a delimited field out to a file                          *
* params:     file, text, recflag                                          *
\**************************************************************************/

void putdelim(FILE *fi, char *text, int rec)
{
 int i;

 /* Make sure the delimiter chars are not in the text  */
 for (i=0; text[i]!=0; i++)
   if (text[i]=='"' || text[i]=='\n') text[i]=' ';

 if (rec)
       fprintf(fi,"\"%s\"\n",text);  /* Change this as needed */
 else  fprintf(fi,"\"%s\",", text);
}



/************************************************************************\
*                                                                        *
* cfreq(): insert the character frequencies into the masterheader        *
*                                                                        *
\************************************************************************/

void cfreq(char *c, int masterflag)
{
long i;

for (i=0; c[i]!=0 ; i++)
 {
  if (masterflag)
  {

   mh->cfreq[ c[i] ]++;                        /* Character frequency */
   if (c[i+1]!=0) mh->tfreq[ c[i] ][ c[i+1] ]++;   /* touple frequency    */

  }

  else // current mode

  {

   curfreq.ccfreq[ c[i] ]++;                        /* Character frequency */
   if (c[i+1]!=0) curfreq.ctfreq[ c[i] ][ c[i+1] ] ++;   /* touple frequency    */

  }
 }

 return;
}



// comparison function used by qsort.

int cmp_tok(const void *a1, const void *b1)
{
 tokfreq *a;
 tokfreq *b;

  a=(tokfreq *) a1;
  b=(tokfreq *) b1;
  if (a->freq == b->freq) return  0;
  if (a->freq <  b->freq) return -1;
  if (a->freq >  b->freq) return  1;
  return 0;
}




/**************************************************************************\
*                                                                          *
* addtouple(): Add a token touple frequency to the list.  The list will    *
*              then be sorted, and the touple with the least freq will be  *
*              at the bottom of the list to be replaced in the next pass   *
*                                                                          *
\**************************************************************************/

void  addtouple(long a, long b, int masterflag)
{
long i;

 if (!masterflag)
 {

  for (i=0; i<=MAXTOUPLE && mh->ctoklist[i].token1==a &&
		     mh->ctoklist[i].token2==b; i++);

  if (mh->ctoklist[i].token1==a &&
      mh->ctoklist[i].token2==b)

      mh->ctoklist[i].freq++;
  else
  {
    mh->ctoklist[MAXTOUPLE].token1 =a;
    mh->ctoklist[MAXTOUPLE].token1 =b;
    mh->ctoklist[MAXTOUPLE].freq =1;
  }

  qsort((void *) (&mh->ctoklist[0]),
	MAXTOUPLE,
	sizeof(mh->ctoklist[0]),
	cmp_tok);

 }
 else
 {

  for (i=0; i<=MAXTOUPLE && mh->toklist[i].token1==a &&
		     mh->toklist[i].token2==b; i++);

  if (mh->toklist[i].token1==a &&
      mh->toklist[i].token2==b)

      mh->toklist[i].freq++;
  else
  {
    mh->toklist[MAXTOUPLE].token1 =a;
    mh->toklist[MAXTOUPLE].token1 =b;
    mh->toklist[MAXTOUPLE].freq =1;
  }

  qsort(mh->toklist,MAXTOUPLE,sizeof(mh->toklist[0]),cmp_tok);
 }

}





/************************************************************************\
* ahash(): ascii hash.  Not as good as rhash, but can be used to alpha   *
*          sort the files by reading them in order after they're indivi- *
*          dually sorted.                                                *
\************************************************************************/

long ahash(char *word, long div)
{
long v;
int ascii;

 /* Since ascii is 0-255, dividing it by 255 gives us a percentage of
    how close that char is to 255.  Since we want to break this up into
    "div" pieces, multiplying it by (div+1) gives us div divisions.


    However since only chars 32-126 are actually printable, we can divide
    by 126-32 instead, giving us 94.  But just incase our string is out
    of range, we can still provide for it by moving it into one of the
    extremes.


    To make this hash a bit more acurate we'll also take the 2nd byte in
    the word.

 */

 ascii=word[0]-32;
 if (ascii<0)  ascii=0;
 if (ascii>94) ascii=94;
 v=ascii;

 ascii=word[1]-32;
 if (ascii<0)  ascii=0;
 if (ascii>94) ascii=94;
 v=(v*95)+ascii;

 v=(v*div/(95*95));

 return ( v );


}




/************************************************************************\
* flclose(): close all database lists                                    *
*                                                                        *
\************************************************************************/

void flclose(void)
{
 int i;

 for (i=0; i<HASHFILES; i++) fclose(flist[i]);

 fseek(mheader,0L,SEEK_SET);
 fwrite(mh,sizeof(masterheader),1,mheader);
 puts("Fuzzybase closed");
 fclose(mheader);

}




/************************************************************************\
*                                                                        *
* flopen(): open file database lists, prepare new header, process recs   *
*                                                                        *
*                                                                        *
\************************************************************************/

void flopen(char *s)
{
 long i,j;
 entry e;
 char fname[255];
 char tmp[255];
 long numrecinfile;
 long numdeleted=0;
 long rs,ru;

 long sortedrec=0;
 long unsortedrec=0;

 long delquerry=0;


 sprintf(fname,"%s.msf",s);
 if ( (mheader=fopen(fname,"r+b"))==NULL )
     {
       sprintf(tmp,"ERROR ACCESSING FUZZYBASE MASTER HEADER [%s] FILE\n",fname);
       perror(tmp);
       free(mh);
       exit(1);
     }

  fread(mh,sizeof(masterheader),1,mheader);


 /* Clear current character based frequencies */
  for (i=0; i<94; i++)
  {
   curfreq.ccfreq[i]=0L;
   for (j=0; j<94; j++)  curfreq.ctfreq[i][j]=0L;
  }



 /* Clear current token based touple frequencies */
 for (i=0; i<MAXTOUPLE; i++)
    {
      mh->ctoklist[i].token1=0L;
      mh->ctoklist[i].token2=0L;
      mh->ctoklist[i].freq=0L;
    }

 
  printf("Opening Fuzzybase %s.\n",s);

puts("===========================================================================");

  if (strlen(mh->description[0])>1) printf(":%s",mh->description[0]);
  if (strlen(mh->description[1])>1) printf(":%s",mh->description[1]);
  if (strlen(mh->description[2])>1) printf(":%s",mh->description[2]);
  if (strlen(mh->description[3])>1) printf(":%s",mh->description[3]);

puts("===========================================================================");


 mh->mastersum=0L;
 mh->totalwords=0L;
 mh->highestfreq=0L;
 mh->avefreq=0.0;

 for (i=0; i<HASHFILES; i++)
  {
   sprintf(fname,"%s.%02lx",s,i);

   printf("Scanning %s\r",fname);

   if (  (flist[i]=fopen(fname,"r+b"))==NULL )
      {
       sprintf(tmp,"ERROR ACCESSING FUZZYBASE [%s]:",fname);
       perror(tmp);
       free(mh);
       exit(1);
      }

   fseek(flist[i],0L,SEEK_END);
   fsize[i]=ftell(flist[i]);


   fseek(flist[i],0,SEEK_SET);

   fread(&rs,sizeof(rs),1,flist[i]);
   fread(&ru,sizeof(rs),1,flist[i]);

   numrecinfile=rs+ru;

   sortedrec  = sortedrec   + rs;
   unsortedrec= unsortedrec + ru;

   for (j=0; j<numrecinfile; j++)
    {
     eread(flist[i],j,&e);
     mh->totalwords++;

     mh->mastersum+=e.masterfreq;

     if (e.masterfreq<=0L)
       {
	e.flag=1;
	delquerry++;
       }


     if (e.flag & 1) numdeleted++;

     if (e.masterfreq>mh->highestfreq)
	mh->highestfreq=e.masterfreq;

     /*
	zap last current frequencies incase we're about to do a querry or
	a generate.
     */

     if (e.currentfreq!=0L || (e.flag & 1)) {e.currentfreq=0L; ewrite(flist[i],j,&e);}

    }


  }


  if (mh->totalwords>0L)
       mh->avefreq=(double)(mh->mastersum) / (double)(mh->totalwords);
  else mh->avefreq=0.0;


  printf("%15ld words stored in database\n",mh->totalwords);
  if (sortedrec)   printf("%15ld words are sorted\n",sortedrec);
  if (unsortedrec) printf("%15ld words are not sorted\n",unsortedrec);
  if (numdeleted)  printf("%15ld words marked for deletion, and need to be purged\n",numdeleted);
  puts("");
  if (mh->totalwords>0L)
  printf("Average master word frequency:%2.8f\n\n",mh->avefreq);

  // Advise user to purge if more than 10% of fuzzybase is marked for deletion
  if (0.1<=(numdeleted/(1+ mh->totalwords)) ) puts("You should purge the deleted records from fuzzybase.)");

  // If the fuzzybase isn't tiny, advise the user to sort it.
  if (mh->totalwords>50*HASHFILES)
   {
    if      ((unsortedrec*1)>sortedrec) puts("Please sort this fuzzybase now, as performance will suffer!");
    else if ((unsortedrec*2)>sortedrec) puts("Please sort this fuzzybase soon.");
    else if ((unsortedrec*4)>sortedrec) puts("You may wish to sort this fuzzybase.");
   }


 puts("");


 fseek(mheader,0L,SEEK_SET);
 fwrite(mh,sizeof(masterheader),1,mheader);

 // puts("Fuzzybase has been successfully opened...");
}


/************************************************************************\
*                                                                        *
* flnew(): create fuzzybase lists, prepare new header, etc.              *
*                                                                        *
*                                                                        *
\************************************************************************/

void flnew(char *s)
{
 long i,j;
 char fname[255];
 char tmp[255];
 long rs;


 sprintf(fname,"%s.msf",s);
 if ( (mheader=fopen(fname,"wb"))==NULL )
      {
       sprintf(fname,"ERROR ACCESSING FUZZYBASE MASTER HEADER [%s] FILE\n",fname);
       perror(fname);
       free(mh);
       exit(1);
      }


  puts("=======================================================================");


  printf("I'm about to create fuzzybase %s.\n",s);

  puts("=======================================================================");
  puts("Please enter four lines of text that describe this fuzzybase.");
  puts("The text you enter now will be displayed whenever you open this");
  puts("fuzzybase");
  printf("1>");

  fgets(mh->description[0],80,stdin);
  mh->description[0][strlen(mh->description[0])]=0;

  printf("2>");
  fgets(mh->description[1],80,stdin);
  mh->description[1][strlen(mh->description[1])]=0;

  printf("3>");
  fgets(mh->description[2],80,stdin);
  mh->description[2][strlen(mh->description[2])]=0;

  printf("4>");
  fgets(mh->description[3],80,stdin);
  mh->description[3][strlen(mh->description[3])]=0;

  puts("Thank you.");

 puts("=======================================================================");



 mh->mastersum=0L;
 mh->totalwords=0L;
 mh->highestfreq=0L;
 mh->avefreq=0L;


  /* Clear master character based frequencies */
  for (i=0; i<94; i++)
  {
   mh->cfreq[i]=0L;
   for (j=0; j<94; j++)  mh->tfreq[i][j]=0L;
  }



 /* Clear master token touples frequencies */
 for (i=0; i<MAXTOUPLE; i++)
    {
      mh->toklist[i].token1=0L;
      mh->toklist[i].token2=0L;
      mh->toklist[i].freq=0L;
      mh->ctoklist[i].token1=0L;
      mh->ctoklist[i].token2=0L;
      mh->ctoklist[i].freq=0L;
    }


 for (i=0; i<HASHFILES; i++)
  {
   sprintf(fname,"%s.%02lx",s,i);

   if (  (flist[i]=fopen(fname,"wb"))==NULL )
      {
       sprintf(tmp,"ERROR ACCESSING FUZZYBASE [%s]:",fname);
       perror(tmp);
       free(mh);
       exit(1);
      }

   rs=0L;
   fseek(flist[i],0,SEEK_SET);
   fwrite(&rs,sizeof(rs),  1, flist[i]);
   fwrite(&rs,sizeof(rs),  1, flist[i]);
   fclose (flist[i]);
  }

 fseek(mheader,0L,SEEK_SET);
 fwrite(mh,sizeof(masterheader),1,mheader);
 fclose(mheader);

}



/*************************************************************************\
* eread(): read a record from a file                                      *
*                                                                         *
\*************************************************************************/
void eread(FILE *f, long recnum, entry *e)
{
 long rs,ru,numrec;

 fseek(f,0L,SEEK_SET);
 fread(&rs,sizeof(rs),1,f);
 fread(&ru,sizeof(ru),1,f);
 numrec=rs+ru;

 if (recnum<1L || recnum>=numrec) {memset(e,0,sizeof(*e)); return;}

 errno=0;
 fseek(f,FZBHEADER +(recnum*sizeof(*e)),SEEK_SET);
 fread(e,sizeof(entry),1,f);
 if (errno) memset(e,0,sizeof(*e));
}




/*************************************************************************\
* ewrite(): write a record to a file                                      *
*                                                                         *
\*************************************************************************/
void ewrite(FILE *f, long recnum, entry *e)
{
char *i;
int  j;


 if (recnum<1L) return;
 errno=0;
 fseek(f,FZBHEADER +(recnum*sizeof(*e)),SEEK_SET);


 /* Clear unused text so that when we use a disk driver level compressor,
    (stacker, doublespace, etc.) the fuzzybases will compress well, and
    also be faster to search. */

 j=strlen(e->word)+1;
 i=&(e->word[j]);
 j=sizeof(e->word)-j;
 memset(i,0,j);


 fwrite(e,sizeof(entry),1,f);
 if (e->masterfreq>mh->highestfreq)
    mh->highestfreq=e->masterfreq;

 // fflush(f);
}





/*************************************************************************\
*                                                                         *
* findword(): find a word in the database.  Returns it's token if found   *
*             or -1 if the word isn't in the database                     *
*                                                                         *
*       command:  WORDFIND       - Just find the word.                    *
*                 WORDINCMASTER  - increment the word's master value      *
*                 WORDINCCURRENT - increment current value                *
*                 WORDDELETE     - set the delete flag                    *
*                                                                         *
*                                                                         *
\*************************************************************************/

long findword(char *word, int command)
{
 long hashval;                   /* file number of fuzzybase    */
 long token;                     /* token value to return       */
 long numrecinfile;              /* num records in file         */
 long rs,ru;                     /* num sorted records in file  */

 FILE *f;
 entry top,mid,bot, *e;          /* binary search comparisons    */
 long ltop,lmid,lbot;            /* record numbers for bin search*/


 int cmp1, cmp2, cmp3;



 hashval=ahash(word,HASHFILES);


 f=flist[hashval];

 fseek(f,0L,SEEK_SET);

 /* 1st long word in the db indicates how many records are sorted  */
 /* this allows us to quickly append records to a fzb file without */
 /* having to constantly sort it.                                  */


 fread(&rs,sizeof(long),1,f);
 fread(&ru,sizeof(long),1,f);

 fseek(f,FZBHEADER,SEEK_SET);
 numrecinfile=rs+ru;


// printf("Searching for: [%s] Sorted:%ld Unsorted:%ld\n",word,rs,ru);

 token=0L;
 ltop=0L;
 lmid=rs/2L;
 lbot=rs;

 if (rs!=0L)
 while (PIGS_DONT_FLY)
   {
    lmid=((lbot+ltop)/2);

    eread(f,ltop,&top);
    eread(f,lmid,&mid);
    eread(f,lbot,&bot);


    if ( (cmp1=strcmp(word,top.word))==0) {e=&top; token= ltop; goto tokcommand;}
    if ( (cmp2=strcmp(word,mid.word))==0) {e=&mid; token= lmid; goto tokcommand;}
    if ( (cmp3=strcmp(word,bot.word))==0) {e=&bot; token= lbot; goto tokcommand;}

    /* Is the word inside the range? If not reading at the end*/
    if ( cmp1<0 ) break;
    if ( cmp3>0 ) break;

    /* If we didn't find it, try reading it at the end. */
    if (ltop>=lmid || lmid>=lbot) break;


    /* Is it in the top half? */
    if ( cmp2<0)  lbot=lmid;
    else /* No?  it must be in the bottom half then */
		  ltop=lmid;

   }


   token=rs;

   // Are there enough records to search through?
   if (numrecinfile<=token || ru==0L) return -1;


   // Okay, we now revert to slow ass mode because the record we
   // are looking for isn't in the sorted partition.  If we don't
   // find it here in the unsorted region, it's not in the fuzzybase.

   while (token<numrecinfile)
   {
    eread(f,token,&top);
    if (strcmp(top.word,word)==0) {e=&top; goto tokcommand;}
    token++;
   }

   return -1;  /* Not found.. too bad */


tokcommand:


  // Okay, so we found it, now what do we do with it?

  if (command==WORDINCMASTER)   // increment master record
   {
    e->masterfreq++;
    ewrite(f,token,e);
   }


  if (command==WORDINCCURRENT)  // increment current comparison
   {
    e->currentfreq++;
    ewrite(f,token,e);
   }


  if (command==WORDDELETE)      // delete the word (just flag it)
   {
    e->flag= e->flag | 1;
    ewrite(f,token,e);
   }

  return ((hashval<<24L) | token);  // return its token value.
}




/**************************************************************************\
*                                                                          *
* addword(): add a word to our fuzzybases. If it exists, update it's freqs *
*           : returns the token value                                      *                                                               *
*                                                                          *
\**************************************************************************/

long addword(entry *e)
{

 entry olde;
 long h,rs,ru, numrecinfile, token;
 FILE *f;

 h=ahash(e->word,HASHFILES);
 f=flist[h];


 fseek(f,0L,SEEK_SET);
 fread(&rs,sizeof(long),1,f);
 fread(&ru,sizeof(long),1,f);
 numrecinfile=rs+ru;

 token=findword(e->word,WORDFIND);

 if (token==-1L)
    {
     token=(rs+ ru);

     ewrite(f,token,e);

     mh->totalwords++;

     mh->mastersum+=e->masterfreq;

     ru++;

     fseek(f,0L,SEEK_SET);
     fwrite(&rs,sizeof(long),1,f);
     fwrite(&ru,sizeof(long),1,f);

     fseek(mheader,0L,SEEK_SET);
     fwrite(mh,sizeof(masterheader),1,mheader);

     return ((h<<24L) | (token));

    }
 else
    {
      eread( flist[token>>24L],(token & 0x00ffffffL),&olde);

      mh->mastersum = mh->mastersum + e->masterfreq;

      e->masterfreq +=olde.masterfreq;
      e->currentfreq+=olde.currentfreq;

      ewrite(flist[token>>24L],(token & 0x00ffffffL), e);

    //  fseek(f,0L,SEEK_SET);
    //
    //  fwrite(&rs,sizeof(long),1,f);
    //  fwrite(&ru,sizeof(long),1,f);


      fseek(mheader,0L,SEEK_SET);
      fwrite(mh,sizeof(masterheader),1,mheader);

      return token;
    }

}





/**************************************************************************\
*                                                                          *
* addwrec() : add to freq of existing word.                                *
*           : same as addwrec, but uses hash and rec numbers.  A bit faster*
*           : but assumes that the entry already exists, and just updates  *
*           : returns the token value                                      *                                                               *
*                                                                          *
\**************************************************************************/

long addwrec(entry *e, long token)
{

 entry olde;

 eread( flist[token>>24],token & 0x00ffffffL,&olde);

 mh->mastersum = mh->mastersum + e->masterfreq;

 e->masterfreq+=olde.masterfreq;
 e->currentfreq+=olde.currentfreq;

 ewrite(flist[token>>24],token & 0x00ffffffL, e);
 return token;

}



// Swap two records from the same fuzzybase partition: used by qsort.h

void swapfzb(void *s, long i,long j)
{
  FILE *fzb;
  entry e1, e2;

  fzb=(FILE *) s;

//  printf("Swapping %ld,%ld\n",i,j);

  eread(fzb,i  , &e1);
  eread(fzb,j  , &e2);

  e1.word[56]=0;
  e2.word[56]=0;


//  printf("Swapping %s and %s\n",e1.word,e2.word);

  if (i<=maxrecs) ewrite(fzb,i,  &e2);
  if (j<=maxrecs) ewrite(fzb,j,  &e1);
}


// Compare two words from the same fuzzybase partition: used by qsort.h

int cmpfzb(void *s, long i, long j)
{
  FILE *fzb;
  entry e1, e2;

  fzb=(FILE *) s;

//  printf("Comparing %ld,%ld\n",i,j);

  eread(fzb,i  , &e1);
  eread(fzb,j  , &e2);

  e1.word[56]=0;
  e2.word[56]=0;

  return (strcmp(e1.word,e2.word));
}


// Compare a record to a memory block copy of a record: used by qsort.h

int cmpptrfzb(void *s, long i, void *e)
{
  FILE *fzb;
  entry e1, *e2;

  e2=(entry *) e;
  fzb=(FILE *) s;

  eread(fzb,i  , &e1);

  e1.word[56]=0;
  e2->word[56]=0;

  return (strcmp(e1.word,e2->word));
}

void *getfzb(void *s, long i)
{

 FILE *fzb;
 entry *e;
 fzb=(FILE *) s;

 e=malloc(sizeof(entry));
 if (e==NULL)
  {
   puts("OUT OF MEMORY!");
   free(mh);
   exit(5);
  }
 eread(fzb,i  , e);


 return e;
}



/*************************************************************************\
*                                                                         *
* sortfzb(): sort the fuzzybase records.                                  *
*                                                                         *
*                                                                         *
*                                                                         *
\*************************************************************************/

void sortfzb(char *s)
{

 int swapflag=0;
 long i,j,rs, ru, numrecinfile;
 long unsorted;
 FILE *fzb;
 entry e1, e2;
 char fname[255];


 for (i=0; i<HASHFILES; i++)
  {
   sprintf(fname,"%s.%02lx",s,i);

   if (  (fzb=fopen(fname,"r+b"))==NULL )
      {
       sprintf(fname,"ERROR ACCESSING FUZZYBASE [%s]:",fname);
       perror(fname);
       free(mh);
       exit(1);
      }

   fseek(fzb,0L,SEEK_SET);
   fread(&rs,sizeof(rs),1,fzb);
   fread(&ru,sizeof(rs),1,fzb);

   fseek(fzb,0L,SEEK_END);
   numrecinfile=rs+ru;

   unsorted=ru;

   if (!numrecinfile)  {printf("Skipping %s because it is empty.\n",fname); continue;}
   if (!unsorted)      {printf("Skipping %s because it is sorted.\n",fname); continue;}

   printf("Sorting: %s which has %ld unsorted records out of %ld records.\n",fname,unsorted,numrecinfile);

   maxrecs=numrecinfile;
   // Let qsort.h take over now.
   quicksort(fzb, numrecinfile, getfzb, cmpfzb, cmpptrfzb, swapfzb);

  fseek(fzb,0L,SEEK_SET);

  fwrite(&numrecinfile,sizeof(numrecinfile),1,fzb);
  ru=0;
  fwrite(&ru,          sizeof(ru),          1,fzb);

  fclose(fzb);

 }

 return;

}




/*************************************************************************\
*                                                                         *
* purgefzb(): purge deleted fuzzybase records.                            *
*                                                                         *
*                                                                         *
\*************************************************************************/

void purgefzb(char *s)
{

 long i,j,k,rs,ru, numrecinfile, newrecinfile;
 long newsorted, deleted;
 FILE *fzb;
 FILE *tmp;
 entry e;
 char fname[255];
 char tname[255];

 for (i=0; i<HASHFILES; i++)
  {
   sprintf(fname,"%s.%02.fzb",s,i);
   sprintf(tname,"%s.%02.tmp",s,i);

   if (  (fzb=fopen(fname,"r+b"))==NULL )
      {
       sprintf(fname,"ERROR ACCESSING FUZZYBASE [%s]:",fname);
       perror(fname);
       free(mh);
       exit(1);
      }


   if ( (tmp=fopen(tname,"wb"))==NULL )
     {
       sprintf(fname,"ERROR ACCESSING TEMP FUZZYBASE [%s]:",tname);
       perror(fname);
       free(mh);
       exit(1);
     }


    fseek(fzb,0L,SEEK_SET);
    fread(&rs,sizeof(rs),1,fzb);
    fread(&ru,sizeof(rs),1,fzb);

    fseek(fzb,0L,SEEK_END);
    numrecinfile=rs+ru; //getnumrec(i);;

    k=0;
    newsorted=0L;
    newrecinfile=0L;
    deleted=0L;

    // do the purge for this portion //
    for (j=0; j<numrecinfile; j++)
    {
     eread(fzb,j,&e);
     if (e.masterfreq==0L) e.flag=e.flag | 1;
     if ((e.flag & 1)==0)
       {
	ewrite(tmp,k++,&e);
	if (j<=rs) newsorted++;
	newrecinfile++;
       }
     else
       {
	deleted++;
       }
    }

    // set new heading results //
    fseek(tmp,0L,SEEK_SET);

    fwrite(&newsorted,sizeof(newsorted),1,tmp);
    ru=newrecinfile-newsorted;
    fwrite(&ru,sizeof(ru),1,tmp);


    fclose(tmp);
    fclose(fzb);

    unlink(fname);            // delete old fuzzybase  hash file.
    rename(tname,fname);      // rename new tempfile to old name.

    printf("%9ld words deleted from %s.\n",deleted,fname);

  }

  return;

}




/*************************************************************************\
*                                                                         *
* str_cmp(): Comparison function used for qsort() function in appendtofzb *
*            This is used to pass a pointer to this function to qsort.    *
*            Null/Empty strings are moved to the bottom                   *
*                                                                         *
\*************************************************************************/

int str_cmp(const void *a, const void *b)
{
 char *c1, *c2;


 c1=(char *) a;
 c2=(char *) b;

   if (a==NULL) return -1;
   if (b==NULL) return  1;

   if (*c1 == '\0') return -1;
   if (*c2 == '\0') return  1;

   return( strcmp((char *)a,(char *)b) );

}





/*************************************************************************\
*                                                                         *
* appendtofzb(): append/update entries in a fuzzybase by adding text from *
*                a textfile.  Masterflag parameter specifies whether this *
*                will append to the master entries or the comparison      *
*                entries.                                                 *
*                                                                         *
\*************************************************************************/

void appendtofzb(char *fnfzb, char *fntext, int masterflag)
{

char *separator=" !@#$%^&*()-_+=[]{}\\|:;\"'<>,./?~`\15\12\n\r";

entry e;
FILE *t;
long tsize;
long line=0L;
char *buffer;
char *words[1000];
int x;
int i;

long curtoken;


flopen(fnfzb);             /* open database */


 mh->csum=0;

 if ( (t=fopen(fntext,"rt"))==NULL)
  {
   perror("I couldn't open the text file you asked me to add to the fuzzybase\n");

   flclose();
  }


 fseek(t,0L,SEEK_END);
 tsize=ftell(t);
 fseek(t,0L,SEEK_SET);


 errno=0;
 buffer=NULL;
 //buffer=malloc(tsize*1.5);    // fast mode not yet implemented

 if (buffer==NULL)
 {
  errno=0;
  buffer=malloc(1024);
  if (buffer==NULL)
      {
       perror("Not enough memory for operation");
       flclose();
       fclose(t);
       free(mh);
       exit(1);
      }

 //  puts("Switching to slow mode: Not enough memory to read whole file at once.");
 }
 else
 {
  /* FAST MODE: We read the whole file in memory, we then sort the
		file, and count duplicate words.  By counting duplicate
		words instead of just adding them to the database on
		each occurance, we save ourselves a lot of disk access
		and a lot of processing time.
   */

  puts("Entering fast parsing mode");

  memset(buffer,0,tsize);
  fread(buffer,tsize,1,t);
  buffer[tsize]=0;

  puts("Text read");

  cfreq(buffer, masterflag);

  puts("Tokenizing");
  x=rtoken(buffer, tsize*1.5, separator, words, 1000, RTOK_TOKENS_JSEP);

  if (x==-2 || x==-3)
    {
     /* We don't have enough memory in the buffer to use fast mode, */
     /* or enough elements in our array... */

     printf("Switching to slow mode: %s\n",
       (x==-2 ? "Buffer overflow" : "Not enough word array elements"));

     fseek(t,0L,SEEK_SET);
     goto slowmode;
    }


  puts("inserting...");
  i=-1;
  do
   {
     i++;

//     /* For much faster results, but far acuracy remove the next lines */
//
//#ifdef ACURATE
//
//     curtoken=findword(words[i], WORDFIND);
//     if (curtoken<0L)
//	{
//	  strncpy(e.word,words[i],56);  e.word[57]=0;
//
//	  if (masterflag) {e.masterfreq++;  mh->mastersum++}
//	  else            {e.currentfreq++; mh->csum++;}
//
//	  e.flag=0;
//
//	  addwrec(e,curtoken); /* Update frequencies */
//	}
//
//     addtouple(mh->lastword, curtoken);
//     mh->lastword=curtoken;
//
//#endif

     /**************************************************************/
   }
  while(!isnill(words[i]));

 }



slowmode:

 errno=0;


 do
 {

  memset(buffer,0,768);
  fgets(buffer,768,t);
  line++;
  buffer[512]=0;

  cfreq(buffer, masterflag);

  x=rtoken(buffer, 1024, separator, words, 400, RTOK_TOKENS_JSEP);

  if (x<-1)
   switch (x)
    {
     case -2: printf("ERROR: Too many words in line:\n%s\nSome text was ignored",buffer);
     case -3: printf("ERROR: Buffer too short. Recompile program with larger buffer\n:%s\n",buffer);
    }

  //if ((line & 0x000007)==0)
  thermometer(t,tsize);

  i=0;
  while (!isnill(words[i]))
    {

     strncpy(e.word,words[i],56);      e.word[57]=0;
     if (masterflag)
	{
	 e.masterfreq=1;
	 e.currentfreq=0;
	 mh->mastersum++;
	}
     else
	{
	 e.masterfreq=0;
	 e.currentfreq=1;
	 mh->csum++;
	}

     e.flag=0;

     #ifdef ACURATE

     curtoken=findword(words[i], WORDFIND);
     if (curtoken<0L)
	{
	  strncpy(e.word,words[i],56);  e.word[57]=0;

	  if (masterflag) {e.masterfreq++;  mh->mastersum++}
	  else            {e.currentfreq++; mh->csum++;}

	  e.flag=0;

	  addwrec(e,curtoken); /* Update frequencies */
	}

     addtouple(mh->lastword, curtoken);
     mh->lastword=curtoken;

#else
     addword(&e);
#endif

     i++;


    }


 } while ( (!feof(t)) && (!errno) && (ftell(t)<tsize) );

 thermometer(t,tsize);

 if (masterflag)
    {
     puts("Append operation completed.");
    }
 else
    {
     puts("Pass 1 completed.");
    }


    free(buffer);

}




/*************************************************************************\
*                                                                         *
* extractset():  Extract a fuzzy set from the fuzzybase for later         *
*                processing.  Meanwhile this function can be used to make *
*                quite a bit of pre-processing and its output can be used *
*                by a human for closer inspection.                        *
*                                                                         *
*                This function invokes a version of the token closeness   *
*                algorithm used by the tree resolution system described   *
*                in my first paper.                                       *
*                                                                         *
*                In the future, some other algorithm will be used to find *
*                how close two sets are to each other on a deeper more    *
*                meaningful level using some other fuzzy function, perhaps*
*                a form of the Kth Nearest Neighbor algorithm adapted to  *
*                map sets to each other rather than elements to sets, or  *
*                perhaps the fuzzy relation function to see if the cur.set*
*                is a fuzzy subset of the master fuzzy set.               *
*                                                                         *
\*************************************************************************/


void extractset(char *s, char *txt, char *cmpf)
{

double mfreq, cfreq, sfreq=0.0, touplefreq=0.0,
	    chartouple=0.0, charfreq=0.0;

long ccfreq=0L, ctouplefreq=0L;
long cmsum=0L, ccsum=0L;
long ctmsum=0L, ctcsum=0L;
entry e;
long i,j;
long numrecinfile, rs,ru;



FILE *set;

  if ( (set=fopen(cmpf,"a+t"))==0)   // output file
   {
    perror("I couldn't append/create the fuzzy set file");
    flclose();
    free(mh);
    exit(1);
   }


  for (i=0; i<HASHFILES; i++)
  {
   printf("Scanning: %i/%i  \r",i+1,HASHFILES+1);
   fseek(flist[i],0L,SEEK_SET);
   fread(&rs,sizeof(rs),1,flist[i]);
   fread(&ru,sizeof(rs),1,flist[i]);
   numrecinfile=rs+ru;

   for (j=0; j<numrecinfile; j++)
    {
     eread(flist[i],j,&e);


     if (e.flag & 1) {
		      printf(":");
		      continue;  /* Skip deleted Records */
		     }

    /* Calculate the frequency for the master and current set elements.
       freq=occurance_of_word / total_occurances_of_all_words. */


    if (mh->mastersum)    mfreq=( (double) (e.masterfreq) / ( double) ( mh->mastersum));
    else                  {
			   if (!j) fprintf(set,"Master Freq is empty!\n");
			   mfreq=0.0;
			  }


    if (mh->csum)         cfreq=( (double) (e.currentfreq)/(double) (mh->csum));
    else                  {
			   if (!j) fprintf(set,"Current Freq is empty!\n");
			   cfreq=0.0;
			  }

  //  if (mh->mastersum)
  //    if (mfreq==0.0 && e.masterfreq!=0L)
  //     fprintf(set,"Master Freq: %f  Count:%ld\n",mfreq,e.masterfreq);

  //  if (mh->csum)
  //    if (cfreq==0.0 && e.currentfreq!=0L)
  //     fprintf(set,"Current Freq: %f  Count:%ld\n",cfreq,e.currentfreq);



    /* If the master frequency is better than 1/1000th of a percent,
       we should consider it.  Average folks have a vocabulary of
       around 100,000 words, so we want to discard frequencies that
       show up as much smaller than this value which acts as a low level
       discriminator.
    */


   // if (mfreq>0.0000001)
       {
	/*sfreq is the sum of the distance between mfreq and cfreq.
	  Here we use the pythagorean distance formula sqrtl(a**2-b**2)
	  as a way of exagerating larger distances.  That is if two
	  distances are fairly close to each other the distance formula
	  will give a very small value for their distance.  If their are
	  fairly distance from each other, it will give a much larger
	  distance.

	  These distances will then be summed together into a single
	  variable so as to give us the average distance.  For text
	  written by the same person, the average distance will be
	  small.  For text written by the same person about the same
	  topics, the distance will be very small.  The more formal
	  the writing the less this program will be able to discriminate
	  between two individuals, in which case it can be used to
	  discriminate between two different topics.  */

	  if (mfreq!=0.0 && cfreq!=0.0) printf(".");

	  sfreq=sfreq + sqrt(fabs( (mfreq*mfreq) - (cfreq*cfreq) ));

	  ccfreq++;

   //     fprintf(set,"SFreq: %f count %ld\n\n",sfreq,ccfreq);

       }


    }

  }


  fprintf(set,"Sfreq: %10.8f\n", sfreq);


  if (ccfreq) sfreq=(sfreq / ( (double) ccfreq ));
  else        {
	       fprintf(set,"No words matched in token list [level 2]\n");
	       sfreq=0.0;
	      }

 /* The smaller the number, the less likely that the author of the */
 /* fuzzy base is the author of the text file */
 /* When the output file is sorted */


 /* Match and compare touples list.  If we so much as find a match
    between two touples that have the same tokens, we should strogly
    consider the result.   If their relative positions in the touple
    set are close, we have a very strong match.  If they are far,
    we still have a match, but a weaker one.   This variable is a
    far stronger indicator than the above classifier.   */

 for (i=0; i<=MAXTOUPLE; i++)
  for (j=0; j<=MAXTOUPLE; j++)
   if (mh->ctoklist[i].token1 == mh->toklist[j].token2    &&
       mh->ctoklist[i].token2 == mh->toklist[j].token2)
   {
			       //   +---<-- need to think on this coeff. a bit longer
			       //   |       as this will make this level very sensitive
			       //   v       perhaps too sensitive.
    touplefreq = touplefreq +      ( 1.0 -  abs(i-j)/MAXTOUPLE  ) *
     lsqrtl(fabs(  pow((mh->ctoklist[MAXTOUPLE].freq / mh->mastersum),2.0) -
		   pow((mh->ctoklist[MAXTOUPLE].freq / mh->csum),     2.0)));

    ctouplefreq++;
   }


 if (ctouplefreq) touplefreq=touplefreq/((double) ctouplefreq);
 else             touplefreq=0.0;



 //------ Deal with character frequencies -----//
 for (i=0; i<=95; i++)
   {
    cmsum+=     mh->cfreq[i];
    ccsum+=curfreq.ccfreq[i];
    for (j=0; j<=95; j++)
     {
      ctmsum+=     mh->tfreq[i][j];
      ctcsum+=curfreq.ctfreq[i][j];
     }
   }



 for (i=0; i<=95; i++)
 {
  /*Cur Character frequency              */
  if (cmsum>0 && ccsum>0)
  charfreq+=lsqrtl(fabsl(
		 lpowl(     mh->cfreq[i]/cmsum,2.0)  -
		 lpowl(curfreq.ccfreq[i]/ccsum,2.0)));

  for (i=0; i<=95; i++)
  {
   if (ctmsum>0 && ctcsum>0)
   chartouple+=lsqrtl(fabsl(
		    lpowl((     mh->tfreq[i][j]/ctmsum),2.0) -
		    lpowl((curfreq.ctfreq[i][j]/ctcsum),2.0) ));
  }
 }

 charfreq  =charfreq   / 96.0;        // 96 printable characters
 chartouple=chartouple / (96.0*96.0); // 96*96 printable characters




 fprintf(set,"SOURCE TEXT: %s\n",txt);
 fprintf(set,"FUZZYBASE  : %s\n",s);

 if (cmsum== 0L) fprintf(set,"Oops! Master  Char Sum is Empty!");
 if (ccsum== 0L) fprintf(set,"Oops! Current Char Sum is Empty!");
 if (ctmsum==0L) fprintf(set,"Oops! Master  Touple Char Sum is Empty!");
 if (ctcsum==0L) fprintf(set,"Oops! Current Touple Char Sum is Empty!");


// fprintf(set,"\n\nMaster Chars:\n");
// for (i=0; i<95; i++)
// {
// fprintf(set,"(%4ld  ",mh->cfreq[i]);
// for (j=0; j<95; j++)
//  fprintf(set,"%4ld:",mh->tfreq[i][j]);
//
//  fprintf(set,"\n");
// }
//
// fprintf(set,"\nCurrent Chars:\n");
// for (i=0; i<95; i++)
// {
// fprintf(set,"(%4ld  ",curfreq.ccfreq[i]);
// for (j=0; j<95; j++)
//  fprintf(set,"%4ld:",curfreq.ctfreq[i][j]);
//
//  fprintf(set,"\n");
// }

 fprintf(set,"L1: %10.8f deg of closeness  [TOKEN TOUPLES]\n", touplefreq);
 fprintf(set,"L2: %10.8f deg of difference [TOKEN FREQ   ]\n", sfreq);
 fprintf(set,"L3: %10.8f deg of difference [CHAR  TOUPLES]\n", chartouple);
 fprintf(set,"L4: %10.8f deg of difference [CHAR  FREQ   ]\n", charfreq);
 fclose(set);
 fprintf(set,"\n");
 puts("Done.");
}




/*************************************************************************\
*                                                                         *
* outputdelim():  Extract a fuzzybase into a standard delimited file      *
*                 for database processing.                                *
*                                                                         *
\*************************************************************************/


void outputdelim(char *sdfname)
{
entry e;
long i,j,k,l;
long numrecinfile,rs,ru,token;
FILE *sdf;
char temp[64];


printf("Extracting database to SDF file %s...\n",sdfname);

  if ( (sdf=fopen(sdfname,"wb"))==0)
   {
    perror("I couldn't create the sdf file");
    flclose();
    free(mh);
    exit(1);
   }


  for (i=0; i<HASHFILES; i++)
  {

   fseek(flist[i],0L,SEEK_SET);
   fread(&rs,sizeof(rs),1,flist[i]);
   fread(&ru,sizeof(rs),1,flist[i]);
   numrecinfile=rs+ru;

   for (j=0; j<numrecinfile; j++)
    {
     eread(flist[i],j,&e);
     if (e.flag & 1) continue;  /* Skip deleted Records */

     token=((i<<24) | j);

     memset(temp,0,64);


     for (l=0,k=0; e.word[l]!=0; k++)
       if (e.word[k]<32)
	   {temp[l++]='^';
	    temp[l++]=e.word[k]+64;
	   }
	else temp[l++]=e.word[k];

     putdelim(sdf, temp, 0);
     sprintf(temp,"%ld",e.masterfreq);
     putdelim(sdf,temp,0);
     sprintf(temp,"%8lx",token);
     putdelim(sdf,temp,1);

    }
  }



 flclose();
 fclose(sdf);
 puts("Output Operation completed.");

}




/*************************************************************************\
*                                                                         *
* deleterecs():  Delete any records whose frequency are less than the     *
*                parameter...                                             *
*                                                                         *
\*************************************************************************/


void deleterecs(char *s)
{
long i,j;
long numrecinfile,rs,ru;
entry e;

long double freq;
long double limit;



  limit=(atof(s)/100);

  if (limit<0) {printf("I can't use negative percentages");
		flclose();
		free(mh);
		exit(1);
	       }

  for (i=0; i<HASHFILES; i++)
  {
   fseek(flist[i],0L,SEEK_SET);
   fread(&rs,sizeof(rs),1,flist[i]);
   fread(&ru,sizeof(rs),1,flist[i]);
   numrecinfile=rs+ru;

   for (j=0; j<numrecinfile; j++)
    {
     eread(flist[i],j,&e);
     if (e.flag & 1) continue;  /* Skip already deleted Records */


     freq=e.masterfreq/mh->mastersum;

     /* If the word's frequency is less than the requested freq.,
	mark it for deletion */

     if (freq<limit)
       {
	e.flag=1;
	ewrite(flist[i],j,&e);
       }

    }
  }


 flclose();
}





/*************************************************************************\
*                                                                         *
* more(): put a more prompt on the screen                                 *
*                                                                         *
*                                                                         *
\*************************************************************************/

void more()
{
 printf("[MORE]\r");
 getch();
 printf("      \r");
}





/*************************************************************************\
*                                                                         *
* helpscreen(): show command line options to user;                        *
*                                                                         *
*                                                                         *
*                                                                         *
*                                                                         *
\*************************************************************************/



void helpscreen()
{
  puts("Usage:");
  puts("");
  puts("MEDUSA cmd {cmd_options}");
  puts("");
  puts("where cmd:  N  - New FuzzyBase");
  puts("            S  - Sort Fuzzybase");
  puts("            P  - Purge Deleted Words from FuzzyBase");
  puts("            G  - Generate Fuzzy Set comparison of a text file");
  puts("            A  - Append words to Fuzzy Base");
  puts("            Q  - Querry sets comparison results");
  puts("            D  - Delete unused words less than a certain freq.");
  puts("            O  - Output the fuzzybase as sdf file");

  puts("");

 more();

  puts("Command Syntax:");
  puts("");
  puts("NEW FZB     medusa  N  fuzzybasename");
  puts("SORT:       medusa  S  fuzzybasename");
  puts("PURGE:      medusa  P  fuzzybasename");
  puts("SORT&PURGE: medusa  SP fuzzybasename");
  puts("GENERATE:   medusa  G  fuzzybasename texttocompare comparison_out");
  puts("and PURGE:  medusa  GP fuzzybasename texttocompare comparison_out");
  puts("APPEND:     medusa  A  fuzzybasename textfile");
  puts("and SORT:   medusa  AS fuzzybasename textfile");
  // puts("QUERRY:     not implemented yet");
  puts("DELETE:     medusa  D  fuzzybasename percentage");
  puts("and PURGE:  medusa  DP fuzzybasename percentage");
  puts("OUTPUT:     medusa  O  fuzzybasenme sdfname");
  puts("");
  free(mh);
  exit(1);
}





/**************************************************************************\
*                                                                          *
* main(): parse command line arguements and do preprocessing if any        *
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/


int main(int argc, char *argv[])
{
 char c, cmd, cmd2;
 int i;


 puts("===========================================================================");
 puts("                      Medusa's Tentacles(tm)  V áeta 1.0.0");
 puts("               PUBLIC ALPHA/BETA TEST VERSION.  BEWARE OF BUGS!!!");
 puts("---------------------------------------------------------------------------");
 puts("  Copyright(C) 1994 by Ray (Arsen) Arachelian, All Rights Reserved, GTDR.  ");
 puts("          rarachel@photon.poly.edu,RayDude@aol.com,sunder@intercom.com");
// puts("===========================================================================");
 puts("");

 mh=malloc(sizeof(masterheader));
 if (mh==NULL) {perror("Not enough memory to start up."); exit(2);}

 if (argc<3) helpscreen();

 cmd=argv[1][0];
 cmd2=argv[1][1];
 if (cmd=='-' || cmd=='/') {cmd=argv[1][1]; cmd2=argv[1][2];}

 cmd=toupper(cmd);
 cmd2=toupper(cmd2);



 /* Parse options if any were specified */
 if (argc>3)
   for (i=3; i<argc; i++)
   {
    c=argv[i][0];
    if (c=='-' || c=='/') c=argv[i][1];

    switch (c)  // no options specified yet
      {

      }

   }


 /* Parse command... */
 switch (cmd)
 {

    case 'N': puts("New fuzzybase");
	      flnew(argv[2]);      break;

    case 'S': if (cmd2=='P') {puts("Purging fuzzybase"); purgefzb(argv[2]);}
	      puts("Sorting fuzzybase");
	      sortfzb(argv[2]);
	      break;

    case 'P': puts("Purging fuzzybase");
	      purgefzb(argv[2]);

	      if (cmd2=='S')
	       {
	       puts("Sorting fuzzybase");
	       sortfzb(argv[2]);
	       }
	      break;

    case 'G': if (argc<5) helpscreen();
	      printf("Generating fuzzy comparison set in %s\n",argv[4]);
	      appendtofzb(argv[2],argv[3],0);
	      extractset( argv[2],argv[3],argv[4]);
	      flclose();
	      if (cmd2=='P') {puts("Purging fuzzybase"); purgefzb(argv[2]);}
	      break;

    case 'A': puts("Appending");
	      appendtofzb(argv[2],argv[3],1);
	      flclose();
	      if (cmd2=='S')
	       {
	       puts("Sorting fuzzybase");
	       sortfzb(argv[2]);
	       }

	      break;

    case 'O': puts("Extracting...");
	      flopen(argv[2]);
	      outputdelim(argv[3]);
	      break;


    case 'Q': puts("Command not yet implemented.");
	      break;

    case 'D': puts("Delete Records...");
	      flopen(argv[1]);
	      deleterecs(argv[2]);
	      flclose();
	      if (cmd2=='P') {puts("Purging fuzzybase"); purgefzb(argv[2]); break;}
	      printf("Purge deleted records? [y/n]");
	      c=getch();
	      if (c=='Y' || c=='y') purgefzb(argv[2]);
	      break;

   default: printf("Unknown command %s\n\n",argv[1]); helpscreen();
  }


 free(mh);


 return(0);
}
