#include <stdlib.h> /* defines exit() and malloc()                       */
#include <string.h> /* defines strlen etc.                               */
#include <stdio.h>  /* defines NULL, EOF, stdin, stdout, stderr,         */
		    /* fprintf(), fileno(), ... for standard i/o         */

extern int errno;

char  name[] = "CRYN - A data encryption system written in C, named files";
char  version[] = "Version 6.00, 1992-11-15, 1400 hours";
char  author[]  = "Copyright (c) 1987-1992 by author: Harry J. Smith";
char  address[] =
	  "19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.";

char  usage1[]  =  "usage: CRYN key [infile outfile [D]]";
char  usage2[]  =  "  key only => Init CRY.CON file using key and exit";
char  usage3[]  =  "      no D => Encipher and update CRY.CON file";
char  usage4[]  =  "         D => Decipher";

#define  POOLSIZE 2048	     /* Number of integers in random number pool */
#define  CONSIZE  18	     /* Number of characters in CRY.CON file  	 */
#define  BUFSIZE  16384	     /* Number of characters in file i/o buffer	 */
#define  MAXKEY   24	     /* Number of characters in short key	 */
#define  TRUE     1
#define  FALSE    0

int  ri[9],	    /* Current random integers from the 9 generators	 */
     rs[9],	    /* Initial random seeds for the 9 generators	 */
     rc[9],	    /* Current value of clear continuation seeds	 */
     rr,	    /* Random running combination of ri[0] thru ri[8]	 */
     *pool,	    /* Pool of POOLSIZE random integers			 */
     encipher,	    /* Boolean, Enciphering a file, FALSE => Deciphering */
     next = 0,	    /* Next random no. routine to use, cycles 0 thru 8	 */
     poolmask =	    /* Mask for index to random pool = 2**N - 1		 */
       POOLSIZE-1;
char *buf,	    /* Buffer for file i/o				 */
     key[MAXKEY+1]; /* The short key string in standard form		 */
FILE *infile,	    /* File pointer of input file			 */
     *outfile,	    /* File pointer of output file			 */
     *confile;	    /* File pointer of continuation file "CRY.CON"	 */

int  init( int argc, char *argv[]);  /* Initialize the process		 */
int  term( void);		     /* Terminate processing		 */
void standard( char *cp);	     /* Standardize a key string	 */
void convkey( void);		     /* Convert key to 9 seeds		 */
int  rn( int n);		     /* random integer from ri[n], n<9	 */
int  rc9( void);		     /* Combination of rn(0) thru rn(8)	 */
void erase( void);		     /* Erase all trace of the key	 */

/* --------------------------------------------------------------------- */
void main( int argc, char *argv[])
{
   int  *ip,		    /* Pointer to current integer in buf	 */
	*jp,		    /* Pointer after last integer in buf	 */
	ind,		    /* Random index into pool of random integers */
	nr,		    /* Number of bytes read into buf		 */
	rc;		    /* Return code from init() and term()	 */

					  /* Validate arguments		 */
   if( !((argc == 2) || (argc == 4) ||
	((argc == 5) && ((*argv[4] == 'D') || (*argv[4] == 'd')))))
   {
      fprintf( stderr, "\n%s\n%s\n%s\n\n", name, version, author);
      fprintf( stderr, "%s\n%s\n%s\n%s\n", usage1, usage2, usage3, usage4);
      exit( 1);
   }

   rc = init( argc, argv);		  /* Initialize the process	 */

   if( rc)                                /* If error in init or done    */
   {  erase();				  /* erase key			 */
      exit( rc);			  /* and exit			 */
   }
					  /* Read input file		 */

   while ( (nr = fread( buf, 1, BUFSIZE, infile)) > 0)
   {
      for( ip = (int *)buf, jp = ip + (nr+1)/2;   ip != jp;   ++ip)
      {
	 /*
	  * This is the heart of the encryption method
	  */
	 ind = rr & poolmask;		  /* Random index to random pool */
	 *ip ^= pool[ind];		  /* XOR data with random int	 */
	 pool[ind] ^= ( rr += rn( next)); /* XOR random int with next rr */
	 if( ++next == 9)		  /* Cycle to next generator	 */
	    next = 0;
      }
      if( fwrite( buf, 1, nr, outfile) != nr) /* Write output file	 */
      {				              /* If write error, exit	 */
	 erase();
	 fprintf( stderr, "CRYN: write error no. %d\n", errno);
	 exit( 2);
      }
   }
   rc = term();				  /* Terminate processing	 */
   erase();				  /* Erase all trace of the key	 */
   fprintf( stderr, "return = %d\n", rc); /* Diagnostic, may be removed	 */
   exit( rc);				  /* Exit main			 */
}					  /* End of main (CRYN)		 */

/* --------------------------------------------------------------------- */
int init( int argc, char *argv[])	  /* Initialize the process	 */
{
   /*  Initialize key etc.
    *  Returns a non-zero flag if error
    */

   int  i, j, len;
   char *chp;

   buf  = malloc( BUFSIZE);		  /* Allocate memory for arrays	 */
   pool = (int *) malloc( 2 * POOLSIZE);

   if( buf == (char *) NULL || pool == (int *) NULL)
   {
      fprintf( stderr, "CRYN: not enough memory\n");
      return( 3);
   }

   if( (infile = fopen( argv[1], "rt")) == NULL) /* Try to open key file */
   {
      strcpy(buf, argv[1]);
      len = strlen( buf);
      for( i = 0;  i < len;  ++i)
	 argv[1][i] = 0;		  /* Erase input key		 */
   }
   else
   {
      fgets( buf, 256, infile);
      fclose( infile);
      if( (chp = strchr( buf, '\n')) != NULL)
	 *chp = '\0';
   }

   standard( buf);			  /* Standardize long key	 */

   fprintf( stderr, "%s = input key in standard form\n", buf);

   if( (len = strlen( buf)) > MAXKEY)
   {
      for( i = 1; i < MAXKEY; ++i)	  /* Hash long key		 */
	 buf[i] += buf[i-1];
      for( i = len-2; i >= MAXKEY; --i)
	 buf[i] += buf[i+1];
   }

   for( i = 0;  i <= MAXKEY;  ++i)	  /* Clear short key		 */
      key[i] = 0;

   for( i = j = 0;  i < len;  ++i)	  /* Compress long key		 */
   {
      key[j] += buf[i];
      buf[i] = 0;			  /* Erase input key		 */
      if( ++j == MAXKEY)
	 j = 0;
   }

   for( i = 0;  i < MAXKEY;  ++i)	  /* Fill out key replacing	 */
      if( !key[i])			  /* nulls with spaces		 */
	 key[i] = ' ';

   standard( key);			  /* Standardize short key	 */
   convkey();				  /* Convert short key to random */
					  /* number seeds		 */

   if( argc == 2)			  /* If only key given		 */
   {
      if( (confile = fopen( "CRY.CON", "wb")) == NULL)
      {
	 fprintf( stderr,
	    "CRYN: cannot open CRY.CON, error no. %d\n", errno);
	 return( 4);
      }
      for( i = 0;  i < 9;  ++i)	 	  /* Initialize CRY.CON file	 */
	 ri[i] = rs[i];

      for( i = POOLSIZE-1;  i >= 0;  --i) /* Build a random record	 */
	 pool[i] = rc9();

					  /* Write continuation file	 */

      if( fwrite( pool, 1, CONSIZE, confile) != CONSIZE)
      {
	 fprintf( stderr,
	    "CRYN: cannot write CRY.CON, error no. %d\n", errno);
	 return( 5);
      }
      fclose( confile);
      fprintf( stderr, "Continuation file CRY.CON updated\n");
      return( 6);
   }
					  /* If file names given	 */

   if( (infile = fopen( argv[2], "rb")) == NULL) /* Open input file      */
   {
      fprintf( stderr,
	 "CRYN: cannot open %s error no. %d\n", argv[2], errno);
      return( 7);
   }
   if( (outfile = fopen( argv[3], "wb")) == NULL) /* Open output file    */
   {
      fprintf( stderr,
	 "CRYN: cannot open %s error no. %d\n", argv[3], errno);
      return( 8);
   }

   encipher = (argc == 4);		  /* Set encipher/decipher flag	 */

   if( encipher)			  /* If encipher, open con. file */
   {
      if( (confile = fopen( "CRY.CON", "rb")) == NULL)
      {
	 fprintf( stderr,
	    "CRYN: cannot open CRY.CON, error no. %d\n", errno);
	 return( 9);
      }
					  /* Read continuation file	 */

      if( fread( pool, 1, CONSIZE, confile) != CONSIZE)
      {
	 fprintf( stderr,
	    "CRYN: cannot read CRY.CON, error no. %d\n", errno);
	 return( 10);
      }
      fclose( confile);
					  /* Write 1st record of outfile */

      if( fwrite( pool, 1, CONSIZE, outfile) != CONSIZE)
      {
	 fprintf( stderr, "CRYN: write error no. %d\n", errno);
	 return( 11);
      }
   }
   else					  /* Else decipher		 */
   {
					  /* Read first record of file	 */

      if( (i = fread( pool, 1, CONSIZE, infile)) != CONSIZE)
      {
	 fprintf( stderr, "CRYN: read error no. %d\n", errno);
	 return( 12);
      }
   }

   for( i = 0;  i < 9;  ++i)		  /* Save and encode		 */
   {					  /* continuation seeds		 */
      rc[i] = pool[i];
      ri[i] = rs[i] ^ pool[i];
      if( !ri[i])			  /* Do not allow a zero seed	 */
	 ri[i] = 1;
   }
   rr = rc9();				  /* Init first word of running	 */
					  /* combination of ri[0]- ri[8] */

   for( i = 0;  i < POOLSIZE;  ++i)	  /* Initialize pool, random int */
   {
      pool[i] = rr += rn( next);
      if( ++next == 9)
	 next = 0;
   }
   rr = rc9();				  /* Init rr for first random	 */
					  /* index into pool		 */

   fprintf( stderr, "%s = short key in standard form\n", key);

   for( i = 0;  i < 9;  ++i)
      fprintf( stderr, "rs[%d] = %-7d ", i, rs[i]);

   for( i = 0;  i < 9;  ++i)
      fprintf( stderr, "ri[%d] = %-7d ", i, ri[i]);

   if( encipher)
      fprintf( stderr, "\nEncipher\n");
   else
      fprintf( stderr, "\nDecipher\n");
   return( 0);
}					  /* End of init		 */

/* --------------------------------------------------------------------- */
int term( void)
{
   /*  Terminate processing
    *  Returns a non-zero flag if error
    */

   int  i;

   if( ferror( infile))		  	  /* Test for infile read error	 */
   {
      fprintf( stderr, "CRYN: read error no. %d\n", errno);
      return( 13);
   }

   fclose( infile);			  /* Close files		 */
   fclose( outfile);

   if( encipher)			  /* If encipher, open con. file */
   {
      if( (confile = fopen( "CRY.CON", "wb")) == NULL)
      {
	 fprintf( stderr,
	    "CRYN: cannot open CRY.CON, error no. %d\n", errno);
	 return( 14);
      }

      for( i = 0;  i < 9;  ++i)	  	  /* Restore clear continuation	 */
	 ri[i] = rc[i];			  /* seeds			 */

      for( i = POOLSIZE-1;  i >= 0;  --i) /* Build a random record	 */
	 pool[i] = rc9();
					  /* Write continuation file	 */

      if( fwrite( pool, 1, CONSIZE, confile) != CONSIZE)
      {
	 fprintf( stderr,
	    "CRYN: cannot write CRY.CON, error no. %d\n", errno);
	 return( 15);
      }
      fclose( confile);
   }
   return( 0);
}					  /* End of term		 */

/* --------------------------------------------------------------------- */
void standard( char *cp)
{
   /*  Standardize a key string
    *  Output characters are from 33 to 95 inclusive
    */

   for(  ;  *cp;  ++cp)
   {
      *cp &= 127;

      if( *cp > 95)
	  *cp -= 32;

      if( *cp < 32)
	  *cp += 32;

      if( *cp == ' ')
	  *cp = '/';
   }
}					  /* End of standard		 */

/* --------------------------------------------------------------------- */
void convkey( void)
{
   /*  Convert the encryption key to 9 seeds
    *
    *  input: key[i], i = 0, ..., MAXKEY-1
    *  output: rs[j], j = 0, ..., 8
    *
    *    [0]    [1]    [2]    [3]    [4]    [5]    [6]   ...  [22]   [23]
    *  111111 111111 111111 111111 111111 111111 111111      111111 111111
    *     6      6     4  2    6      6    2  4     6		6      6
    *  ****** seed0 *****...... seed1 ......***** seed2  ...  seed8 ******
    *	     16 BITS	       16 BITS		 16 BITS     16 BITS
    *
    *  144 bit key converted to 144 bits of seed,
    *  seeds of zero are changed to 1.
    */

   int  i, j,
	keyi[MAXKEY];		 /* Key in integer form, 6 bits each int */

   for( i = 0;  i < MAXKEY;  ++i)
      keyi[i] = key[i] & 63;

   for( i = 0, j = 0;  j < 9;  i += 8, j += 3)
   {
      rs[j]   =  ( keyi[i]   << 10) |
		 ( keyi[i+1] <<  4) |
		 ( keyi[i+2] >>  2);

      rs[j+1] =  ( keyi[i+2] << 14) |
		 ( keyi[i+3] <<  8) |
		 ( keyi[i+4] <<  2) |
		 ( keyi[i+5] >>  4);

      rs[j+2] =  ( keyi[i+5] << 12) |
		 ( keyi[i+6] <<  6) |
		   keyi[i+7];
   }
   for( j = 0;  j < 9;  ++j)		  /* Do not allow a zero seed	 */
      if( !rs[j])
	 rs[j] = 1;

   for( i = 0;  i < MAXKEY;  ++i)
      keyi[i] = 0;			  /* Erase local key		 */
}					  /* End of convkey		 */

/* --------------------------------------------------------------------- */
int rn( int n)
{
   /*
    *  Returns the next random integer from ri[n], 0 <= n <= 8
    */

   static int  repeat = TRUE;
   int  i;

   switch( n)
   {
   case 0:

	 /*  1st Congruential Generator, 16 bits
	  *  Generates Random integers from -32768 to 32767 inclusive
	  *  Cycle length = 65536 = 2**16
	  */

	  return( ri[0] = 25173 * ri[0] + 6925);

   case 1:

	 /*  1st Shift-register Generator, 16 bits
	  *  Random integer from -32768 to 32767 inclusive, 0 not generated
	  *  Generator = -22620, Cycle length = 65535 = 3 * 5 * 17 * 257
	  */

	  if( ri[1] & 1)
	     return( ri[1] = ( (unsigned) ri[1] >> 1) ^ -22620);

	  return( (unsigned) ri[1] >>= 1);

   case 2:

	 /*  2nd Congruential Generator, 16 bits
	  *  Random integer from -32768 to 32767 inclusive
	  *  Cycle length = 65537 = prime, zero repeats once
	  */

	  if( !ri[2])
	  {
	     if( repeat)
	     {
		repeat = FALSE;
		return( 0);
	     }
	     else repeat = TRUE;
	  }

	  return( ri[2] = 23629 * ri[2] + 13849);

   case 3:

	 /*  2nd Shift-register Generator, 16 bits
	  *  Random integer from -32768 to 32767, not all generated
	  *  Generator = -07493, Cycle length = 65521 = prime, (65536 - 15)
	  */

	  if( ri[3] & 1)
	     ri[3] = ( (unsigned) ri[3] >> 1) ^ -7493;
	  else
	     (unsigned) ri[3] >>= 1;

	  if( ri[3] == 1)
	     for( i = 0;  i < 14;  ++i)
		rn( 3);		          /* Throw 14 away		 */

	  return( ri[3]);

   case 4:

	 /*  3rd Congruential Generator, 16 bits
	  *  Random integer from -32768 to 32767, not all generated
	  *  Cycle length = 65519 = prime, (65536 - 17)
	  */

	  ri[4] = 4821 * ri[4] + 13001;
	  if( !ri[4])
	     for( i = 0;  i < 17;  ++i)
		rn( 4);		  	  /* Throw 17 away		 */

	  return( ri[4]);

   case 5:

	 /*  3rd Shift-register Generator, 16 bits
	  *  Random integer from -32768 to 32767, not all generated
	  *  Generator = -25501, Cycle length = 65497 = prime, (65536 - 39)
	  */

	  if( ri[5] & 1)
	     ri[5] = ( (unsigned) ri[5] >> 1) ^ -25501;
	  else
	     (unsigned) ri[5] >>= 1;

	  if( ri[5] == 1)
	     for( i = 0;  i < 38;  ++i)
		rn( 5);		  	  /* Throw 38 away		 */

	  return( ri[5]);

   case 6:

	 /*  4th Congruential Generator, 16 bits
	  *  Random integer from -32768 to 32767, not all generated
	  *  Cycle length = 65479 = prime, (65536 - 57)
	  */

	  ri[6] = 10349 * ri[6] + 7001;
	  if( !ri[6])
	     for( i = 0;  i < 57;  ++i)
		rn( 6);		  	  /* Throw 57 away		 */

	  return( ri[6]);

   case 7:

	 /*  4th Shift-register Generator, 16 bits
	  *  Random integer from -32768 to 32767, not all generated
	  *  Generator = -18916, Cycle length = 65449 = prime, (65536 - 87)
	  */

	  if( ri[7] & 1)
	     ri[7] = ( (unsigned) ri[7] >> 1) ^ -18916;
	  else
	     (unsigned) ri[7] >>= 1;

	  if( ri[7] == 1)
	     for( i = 0;  i < 86;  ++i)
		rn( 7);		  	  /* Throw 86 away		 */

	  return( ri[7]);

   case 8:

	 /*  5th Congruential Generator, 16 bits
	  *  Random integer from -32768 to 32767, not all generated
	  *  Cycle length = 65447 = prime, (65536 - 89)
	  */

	  ri[8] = 30133 * ri[8] + 14001;
	  if( !ri[8])
	     for( i = 0;  i < 89;  ++i)
		rn( 8);		  	  /* Throw 89 away		 */

	  return( ri[8]);
   }
   return( 0);                            /* This line is never reached  */
}					  /* End of rn			 */

/* --------------------------------------------------------------------- */
int rc9( void)
{
   /*
    *  Combination of rn(0) thru rn(8), one each
    */

   int  i,  rc;

   for( rc = 0, i = 0;  i < 9;  ++i)
      rc += rn( i);

   return( rc);
}					  /* End of rc9			 */

/* --------------------------------------------------------------------- */
void erase( void)
{
   /*
    *  Erase all trace of the encryption key
    */

   int  i;

   for( i = 0;  i < MAXKEY;  ++i)
      key[i] = 0;

   for( i = 0;  i < 9;  ++i)
      ri[i] = rs[i] = 0;

   for( i = 0;  i < POOLSIZE;  ++i)
      pool[i] = 0;

   rr = 0;
}					  /* End of erase		 */
					  /* End of CRYN.C		 */
