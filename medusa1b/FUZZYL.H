/**************************************************************************\
*                                                                          *
*                              FUZZLYL.H                                   *
*                 Fuzzy Logic Operators/Function Library                   *
*                                                                          *
*             SOURCE CODE REVISION ALPHA-940606-18:06 EST                  *
*                                                                          *
*    DEVEL:[X]   ALPHA DEVEL: [ ]   BETA TESTING:[ ]    PRODUCTION:[ ]     *
*                                                                          *
*               Compiled with Borland C++ 3.1, Huge Model.                 *
*                                                                          *
*       E X P E R I M E N T A L  /  D E V E L O P M E N T   C O D E        *
*                            MAY BE BUGGY!                                 *
*                                                                          *
*              THIS IS PROPRIETARY, PUBLISHED SOURCE CODE.                 *
*                                                                          *
*==========================================================================*
*                                                                          *
*               Copyright (C) 1994 by Ray (Arsen) Arachelian,              *
*          All Rights Reserved, General Technical Data Restricted.         *
*                                                                          *
*==========================================================================*
* The use of this source code is subject to certain restrictions outlined  *
* in the main file of this package.                                        *
\**************************************************************************/

/***************************************************************************\
*                                                                           *
* FUZZYL.H: Fuzzy logic function primitives using integer math. Note that   *
*           1 is not used to indicate total membership.  MAXINT is used for *
*           this purpose instead.  Use the normalize function if you need   *
*           fuzzy vs in the range of 0-1, however using integers instead of *
*           fractions in this way is much faster.                           *
*                                                                           *
*           This include file would be better off as a C++ class however,   *
*           in the interest of compatibility with older systems which might *
*           not have C++ compilers available to them, it is given as a C    *
*           include file.                                                   *
*                                                                           *
\***************************************************************************/

#include <values.h>

typedef int fuzzy;


fuzzy max(fuzzy v1, fuzzy v2)
{
   return ( (v1 > v2) ? v1 : v2);
}



fuzzy min(fuzzy v1, fuzzy v2)
{
   return ( (v1 < v2) ? v1 : v2);
}



fuzzy and(fuzzy v1, fuzzy v2)
{
  return (min(v1,v2));
}


fuzzy or(fuzzy v1, fuzzy v2)
{
  return (max(v1,v2));
}


fuzzy not(fuzzy v)
{
  return (MAXINT - v);
}



fuzzy nor(fuzzy v1, fuzzy v2)
{
 return(not(or(v1,v2)));
}



fuzzy nand(fuzzy v1, fuzzy v2)
{
 return(not(and(v1,v2)));
}


fuzzy nxor(fuzzy v1,fuzzy v2)
{
 return(or(nor(v1,v2),and(v1,v2)));
}



fuzzy xor(fuzzy v1, fuzzy v2)
{
 return( not( nxor(v1,v2) ) );
}



float normalize(fuzzy v)
{
 return ( ((float) v)/((float) MAXINT));
}


fuzzy normtofuzzy(float v)
{
 return ( (fuzzy) (v*MAXINT));
}


/* a=>b */
fuzzy iff(fuzzy v1, fuzzy v2)
{
 return(min(v1,not(v2)));
}


/* a<=>b :  1-|a-b| -> ~|a-b| -> not(abs(a-b))*/
fuzzy equ(fuzzy v1, fuzzy v2)
{
 return not(abs(v1-v2));
}
