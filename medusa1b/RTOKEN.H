/**************************************************************************\
*                                                                          *
*                            RTOKENIZER (tm)                               *
*                       Primitive Text tokenizer                           *
*                                                                          *
*             SOURCE CODE REVISION  BETA-940727-17:29 EST                  *
*                                                                          *
*    DEVEL:[ ]   ALPHA DEVEL: [ ]   BETA TESTING:[X]    PRODUCTION:[ ]     *
*                                                                          *
*                                                                          *
*==========================================================================*
*                                                                          *
*               Copyright (C) 1994 by Ray (Arsen) Arachelian,              *
*          All Rights Reserved, General Technical Data Restricted.         *
*                                                                          *
*                THIS IS PUBLISHED, PROPRIETARY SOURCE CODE                *
*                                                                          *
*==========================================================================*
*                                                                          *
*    WARNING: This source code is the sole property of the author and is   *
*    to be treated as such.  The use of this source code is subject to     *
*    certain limitations stated in the documentation and in the main files *
*    of this software package.  Using this source code or a machine        *
*    executable version of this source code is subject to these conditions.*
*                                                                          *
\**************************************************************************/


#include <string.h>
#include <stdio.h>

#define RTOK_TOKENS        0
#define RTOK_TOKENS_SEP    1
#define RTOK_TOKENS_JSEP   2

#define RTOK_EATNULL       4
#define RTOK_SQUOTE        8
#define RTOK_DQUOTE       16
#define RTOK_PAREN        32
#define RTOK_BRACKET      64
#define RTOK_BRACE       128



/**************************************************************************\
*                                                                          *
* rhash(): text hashing function.                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/



long rhash(char *s, long div)
{
int i;
unsigned long retval;

 i=0;
 retval=0L;

 if (div<3 || strlen(s)==0) return 0;


 do
 {
  retval=retval<< 1;
  retval+=s[i];
  retval=retval^(s[i]<<13);
  retval=retval ^ ((retval>>11)+(i<<19));

 } while(s[i++]!=0);


 return (retval % div);

}






/***************************************************************************\
*                                                                           *
* isnill(): returns true if either the strng pointer passed to it is NULL or*
*           if it points to a string terminator.                            *
\***************************************************************************/

int isnill(char *s)
{
 if (s==NULL) return 1;
 if (*s=='\0')  return 1;
 return 0;
}






/***************************************************************************\
*                                                                           *
* rtoken(): tokenize a string.  Somewhat similar to strtok, however this    *
*           is a one pass function, and can return the separators as tokens *
*           only call this function once per string.                        *
*                                                                           *
*           this can be used as part of a parser.                           *
*                                                                           *
* parameters: string    - the string to tokenize.  Will have chr(0) inserted*
*                         in it after every string.                         *
*             strsize   - max size of the buffer allocated to string        *
*             separator - string containing token separator characters.     *
*             array     - pointer to array of string pointers.  rtoken will *
*                         fill this array with pointers into string.        *
*             arsize    - max size of number of elements in string array    *
*                                                                           *
*             mode        0: return only tokens                             *
*                             ie: a,,b,c -> "A" "" "B" "C"                  *
*                         1: return tokens and individual separators        *
*                             ie: a,,b,c -> "A" "," "" "," "C"              *
*                         2: return tokens and combined separators.         *
*                             ie: a,,b,c -> "A" ",," "B" "C"                *
*                      +  4: eat nulls (for mode 0 and 1) if two separators *
*                            are together a null is returned between them   *
*                            This option prevents that from happening       *
*                      +  8: separators in single quotes not counted        *
*                      + 16: separators in double quotes not counted        *
*                      + 32: separators in parens not counted               *                                                                           *
*                      + 64: separators in brackets not counted             *
*                      +128: separators in braces not counted               *                                                                                *
*                                                                           *
* returns:             0 - no error                                         *
*                      1 - string is empty.  No tokens.                     *
*                     -1 - null string pointer                              *
*                     -2 - not enough elements in array parameter           *
*                     -3 - string buffer overflow                           *
*                     -4 - unexpected close paren                           *
*                     -5 - unexpected close bracket                         *
*                     -6 - unexpected close brace                           *
*                     -9 - stacker overflow! too many nexted parens, etc.   *
*                                                                           *
* MAKE SURE YOU ALLOCATE 1.5X the space needed for the string as this will  *
* insert string terminators in the string.                                  *
*                                                                           *
*                                                                           *
\***************************************************************************/



int rtoken(char *string, int strsize, char *separator,
	     char *array[], int arsize, short mode)
{
long i;
int j;
int k;
int done;
int aridx=0;
long sizeleft;
char *tst;

/* Stack of open parens, quotes, etc. */
char stack[100];
int  stop=0;

int  eatnulls;


eatnulls=((mode & 4)!=0);

array[0]=string;  /* 1st token always points to 1st string */
aridx++;

sizeleft=(strsize-strlen(string));


/* Clear all other token pointers in array */
for (i=1; i<arsize; i++)
 array[i]=NULL;


if (string==NULL) return -1;
if (string[0]==0) return 1;

for (i=0; i<strsize && string[i+1]!=0; i++)
 {

  if (mode & (8+16+32+64+128))
  {
   if ( (mode &   8)!=0   && (string[i]=='`'))
      {
       stack[stop]='`';
       stop++;
      }

   if ( (mode &   8)==  8 && (string[i]=='\''))
      {
       if (stack[stop-1]=='\'' || stack[stop-1]=='`')
	    stop--;
       else
	    stack[stop]='\''; stop++;
      }

   if ( (mode &  16)!=0   && (string[i]=='\"'))
       {
	if (stack[stop-1]=='\"')
	   stop--;
       else
	   stack[stop]='\"'; stop++;
       }

   if ( (mode &  32)!=0   && (string[i]=='('))
       {
	stack[stop]='(';
	stop++;
       }

   if ( (mode &  32)!=0   && (string[i]==')'))
       {
	if (stack[stop-1]=='(')
	   stop--;
	else
	  {
	   return -4;
	  }
       }

   if ( (mode &  64)!=0   && (string[i]=='['))
       {
	stack[stop]='[';
	stop++;
       }

   if ( (mode &  64)!=0   && (string[i]==']'))
       {if (stack[stop-1]=='[')
	   stop--;
	else
	   return -5;
       }

   if ( (mode & 128)!=0   && (string[i]=='{'))
      {
       stack[stop]='{';
       stop++;
      }

   if ( (mode & 128)!=0   && (string[i]=='}'))
       {if (stack[stop-1]=='{')
	     stop--;
	else
	     return -6;
       }

   if (stop>=100)  return -9;
   if (stop<0) stop=0;

  }
  for (j=0; separator[j]!=0; j++)
   if  (string[i]==separator[j] && stop==0)
    switch (mode & 3)
     {
      case 0:

	   string[i]=0;
	   if (aridx>=arsize) return -2;


	   if (eatnulls) if (isnill(array[aridx-1])) aridx--;
	   array[aridx++]=string+i+1;




	   break;

      case 1:
	if (i)
	   {
	    /* i pts to separator */
	    /* i+1 to char after separator */


	    /* move separate two chars up */
	    memmove(&string[i+2],&string[i],strlen(&string[i])+1);

	    string[i+1]=string[i+2];
	    string[i+2]=0;
	    string[i+0]=0;


	    /* i+1 is now separator, i+2 is terminator char after sep */
	    /* i is terminator for current string */
	    /* i+3 is next string */

	    sizeleft--;
	    sizeleft--;

	    if (sizeleft<=0) return -3;

	    if (aridx>=arsize) return -2;
	    if (eatnulls) if (isnill(array[aridx-1])) aridx--;
	    array[aridx++]=&string[i+1];

	    if (aridx>=arsize) return -2;
	    if (eatnulls) if (isnill(array[aridx-1])) aridx--;
	    array[aridx++]=&string[i+3];



	    i+=2;
	   }
	else
	   {
	    memmove(&string[i+1],&string[i],strlen(&string[i])+1);
	    sizeleft--;
	    if (sizeleft<=0) return -3;
	    string[i+1]=0;  // should this be [i]?

	    if (aridx>=arsize) return -2;
	    if (eatnulls) if (isnill(array[aridx-1])) aridx--;
	    array[aridx++]=&string[i+2];
	   }

	 break;

      case 2:
	k=0;

	/////////////////////////////////////////////////////////////////
	///// THERE'S A BUG IN HERE THAT EATS THE 1ST CHAR OF THE NEXT
	///// TOKEN OR THIS TOKEN.
	/////////////////////////////////////////////////////////////////

	/* Find last separator */

	do {
	    done=0;
	    tst=strchr(separator,string[i+k]);
	    if (tst!=NULL) if ((*tst)==0) tst=0;

	    if (tst)
	       k++;
	    else
	       done=1;
	   }
	while (!done);


	 /* i=1st separator */
	 /*  i+k= next text block */

	 memmove(&string[i+k+1],&string[i+k],strlen(&string[i+k])+1);
	 sizeleft--;
	 if (sizeleft<=0) return -3;

	 memmove(&string[i+1],&string[i],strlen(&string[i])+1);
	 sizeleft--;

	 string[i+k+1]=0;
	 string[i]=0;

	 if (sizeleft<=0) return -3;
	 if (aridx>=arsize) return -2;
	 array[aridx++]=&string[i+1];

	 if (aridx>=arsize) return -2;
	 array[aridx++]=&string[i+k+2];
	 i=i+k+2;
	 //i=i+k+1;

      }
  }

 return 0;
}





