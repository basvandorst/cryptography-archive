/***************************************************************************/
/* Ranm.c -- process stdin by algorithm M, from Knuth vol.2                */
/***************************************************************************/

#include <sys/param.h>
#include <sys/errno.h>
extern int errno;
#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define BULK_SIZ  (32771)	/* (prime) number of bytes in bulk[] */

static unsigned char bulk[BULK_SIZ] ; /* buffer of bytes */

static unsigned char
  next_byte() ;			/* get next ranno byte from bulk/stdin */

static unsigned char
  ind_byte() ;			/* get next ranno byte from bulk/stdin */

FILE *indf = NULL ;		/* file for index bytes */

/***************************************************************************/
/* main                                                                    */
/***************************************************************************/

main(argc, argv)
int argc;
char **argv;
{
  long i ;			/* index into arrays */
  long lim ;			/* limit */


  for (i=0;i<BULK_SIZ;i++) bulk[i] = getchar() ; /* fill the bulk array */

  switch (argc) {
  case 1:			/* use stdin and rnd() */
    set_rnd_seed( bulk ) ;	/* init the PRNG */
    while (!(feof(stdin)||ferror(stdin)))
      putchar( next_byte() ) ;
    exit(0) ;

  case 2:			/* use the file given for indexes */
    indf = fopen( argv[1], "rb" ) ; /* read the file in binary */
    if (indf == NULL) {
      fprintf( stderr, "could not open %s\n", argv[1] ) ;
      exit(1) ;
    }
    while (!(feof(stdin)||ferror(stdin)||feof(indf)||ferror(indf)))
      putchar( ind_byte() ) ;
    exit(0) ;

  default:
    fprintf( stderr, "Usage:  %s [<ransource>]\n\
Where <ransource> is an optional file or stream of random bytes\n\
(2 per output byte) used to drive the algorithm.  If <ransource> is\n\
omitted, %s uses stdin and a relatively weak PRNG for generating\n\
index values.\n", argv[0], argv[0] ) ;
  } /* switch */

  exit(0);
} /* main */

/***************************************************************************/
/* next_byte -- get the next byte from bulk[].                             */
/***************************************************************************/

static unsigned char
next_byte()
{
  unsigned char r ;
  long i = rnd() ;		/* get ranno value for index */

  i %= BULK_SIZ ;		/* restrict it to a bulk[] index */
  r  = bulk[i] ;		/* get the saved char */
  bulk[i] = getchar() ;		/* and replace it */
  return ( r ) ;		/* return the saved char */
} /* next_byte */

/***************************************************************************/
/* ind_byte -- get the next byte from bulk[].                              */
/***************************************************************************/

static unsigned char
ind_byte()
{
  unsigned char r ;
  long i ;			/* ranno value for index */

  i = fgetc( indf ) * 256 + fgetc( indf ) ;
  i %= BULK_SIZ ;		/* restrict it to a bulk[] index */
  r  = bulk[i] ;		/* get the saved char */
  bulk[i] = getchar() ;		/* and replace it */
  return ( r ) ;		/* return the saved char */
} /* ind_byte */

