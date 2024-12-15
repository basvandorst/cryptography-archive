/**************************************************************************\
*                                                                          *
*                         MEDUSA's Tentacles (tm)                          *
*              Global Structures & Data Types Include File.                *
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
*    of the license described in the main file [medusa.c]                  *
*                                                                          *
\**************************************************************************/


/* How many touple frequencies to keep track of.  This number should be
   fairly large */

#define MAXTOUPLE 10


typedef struct {
 long token1;
 long token2;
 long freq;
}     tokfreq;





typedef struct
{
 long   mastersum;                /*    Master frequency                 */
 long   totalwords;               /*    Number words                     */
 long   csum;                     /*    Current frequency                */
 long   highestfreq;              /*    highest word freq                */
 double avefreq;                  /*    average freqency                 */
 char   description[4][80];       /*    database description             */

 /*          singleton, touple character frequencies                   */

 long lasttoken;

 tokfreq  toklist[MAXTOUPLE];   /*  mas most common two word combos   */
 tokfreq ctoklist[MAXTOUPLE];   /*  cur most common two word combos   */


 long cfreq[95];                /*Mast Character frequency             */
 long tfreq[95][95];            /*Mast touple frequency                */
} masterheader;


struct
{
 long ccfreq[95];               /*Cur Character frequency              */
 long ctfreq[95][95];           /*Cur touple frequency                 */
} curfreq;


typedef struct
{
 char word[57];
 long masterfreq;          /* The master frequency for this word       */
 long currentfreq;         /* the current frequency for this word      */
 char flag;                /* if bit 0 is set, this entry will be      */
			   /* deleted from the fuzzybase the next time */
			   /* the fuzzybase is sorted/packed.          */

}
entry;


