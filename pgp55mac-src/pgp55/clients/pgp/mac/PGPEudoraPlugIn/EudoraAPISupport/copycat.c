/* ======================================================================

   Yet another attempt to make sense of Mac string and buffer manipulation
   
   Last Edited: Apr 18, 1996
   
   Filename: copycat.c

   Laurence Lundblade <lgl@qualcomm.com>
   Myra Callen <t_callen@qualcomm.com> 

   Copyright 1995, 1996 QUALCOMM Inc.
   
   This supports:
      Pascal Strings - one length byte plus string
      C Strings      - Null terminated string in buffer
      Buffers        - String data and a length
      
   These mostly return the length of the result
*/

#include <ctype.h>
#include "copycat.h"

/* Copy a pascal string into a buffer, return length */
long copyPB(ConstStringPtr source, char *dest)
{
    BlockMoveData(source+1, dest, *source);
    return(*source);
}

/* Copy a buffer to a Pascal string, return length */
long copyBP(const char *source, long sourceLen, StringPtr dest)
{
    BlockMoveData(source, dest + 1, sourceLen);
    return(*dest = sourceLen);
}

/* Append a pascal string into a buffer, return length */
long catPB(ConstStringPtr addition, long destLen, char *dest)
{
    BlockMoveData(addition + 1, dest + destLen, *addition);
    return(destLen += *addition);
}

/* Append a buffer to a Pascal string, return length */
long catBP(const char *addition, long additionLen, StringPtr dest)
{
    BlockMoveData(addition, dest + 1 + *dest, additionLen);
    return(*dest += additionLen);
}

/* Make a copy of a Pascal string, return length */
long copyPP(ConstStringPtr source, StringPtr dest)
{
    BlockMoveData(source+1, dest + 1, *source);
    return(*dest = *source);
}

/* Append a Pascal string to another, return length */
long catPP(ConstStringPtr addition, StringPtr dest)
{
    BlockMoveData(addition + 1,  dest + 1 + *dest, *addition);
    return(*dest += *addition);
}

/* Copy a Pascal string into a C string, return length */
long copyPC(ConstStringPtr source, char *dest)
{
    BlockMoveData(source+1, dest, *source);
    *(dest + *source) = '\0';
    return(*source);
}

/* Copy C string into a Pascal string, return length */
long copyCP(const char *source, StringPtr dest)
{
    const char *	s;
    uchar *			d;
    
    for(s = source, d = dest +1; *s; *d++ = *s++)
    	{
    	}
    *dest = s - source;
    return(*dest);
}


/* ----------------------------------------------------------------------
     A case insensitive strncmp() like thing     
  
   Args: o - First string to compare (NULL terminated)
         r - Second buffer (string) to compare 
         n - Length of second string
         caseignore 

 Result: integer indicating which is greater
   
  ----*/
short cmpCB(const char *o, const char *r, long n, short caseignore)
{
    if(r == NULL && o == NULL)
      return(0);
    if(o == NULL)
      return(1);
    if(r == NULL)
      return(-1);

    n--;
    while(n && *o &&
           (caseignore ? 
              ((isupper(*o) ?
              	tolower(*o): *o) == (isupper(*r)? tolower(*r): *r))  :
              (*o == *r))
           )
    {
        o++; r++; n--;
    }
    return(caseignore ? 
             ((isupper(*o) ?
             	tolower(*o): *o) - (isupper(*r)? tolower(*r): *r)) :
             (*o - *r));
}
	
/* ----------------------------------------------------------------------
     A case insensitive strncmp() like thing     
  
   Args: o - First string to compare - a PASCAL string
         r - Second buffer (string) to compare 
         n - length of second string
         caseignore 

 Result: integer indicating which is greater
   
  ----*/
short cmpPB(ConstStringPtr o, const char *r, long n, short caseignore)
{
    const uchar *o_end;;
    
    if(r == NULL && o == NULL)
      return(0);
    if(o == NULL)
      return(1);
    if(r == NULL)
      return(-1);
      
 	o_end = o + (*o + 1);
    n--;
    o++;
    while(n && o < o_end &&
           (caseignore ? 
              ((isupper(*o) ?
              tolower(*o): *o) == (isupper(*r)? tolower(*r): *r))  :
              (*o == *r))
           )
    {
        o++; r++; n--;
    }
    return(caseignore ? 
             ((isupper(*o) ?
             tolower(*o): *o) - (isupper(*r)? tolower(*r): *r)) :
             (*o - *r));
}
		




