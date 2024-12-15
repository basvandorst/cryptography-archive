/***************************************************************************/
/* ranD.c -- process stdin by sending it through triple-DES, using keys    */
/*   built from the input (via MD5).                                       */
/***************************************************************************/

#include <sys/param.h>
#include <sys/errno.h>
extern int errno;
#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"		/* RSAREF global defs */
#include "md5.h"		/* MD5 definitions */
#include "des.h"		/* DES definitions */

static void process_message() ;

/***************************************************************************/
/* main                                                                    */
/***************************************************************************/

main(argc, argv)
int argc;
char **argv;
{
  int	ch, sl ;
  int  c ;

  process_message() ;

  exit(0);
} /* main */

/***************************************************************************/
/* process_message -- read stdin, gathering input for MD5, writing blocks  */
/*    to stdout.                                                           */
/***************************************************************************/

void process_message()
{
  unsigned char ibuf[BUFSIZ] ;	/* input line buffer */
  MD5_CTX ctx ;			/* context for MD5 */
  long nb, nb2r, nbvar ;	/* # bytes read, # to read, # variation */
  unsigned int i ;
  unsigned char key[32] ;	/* output from MD5 */
  unsigned char *iv ;		/* last 16 bytes of key */
  unsigned char bin[256], bout[256] ; /* do blocks of 256 bytes */
  DES3_CBC_CTX dctx ;		/* DES context */

  iv = &(key[24]) ;		/* last 16 bytes */

  MD5Init( &ctx ) ;		/* init the first MD5 */
  nb = fread( ibuf, 1, BUFSIZ, stdin ) ; /* get the first batch */
  MD5Update( &ctx, ibuf, nb ) ;
  MD5Final( key, &ctx ) ;

  MD5Init( &ctx ) ;		/* the second MD5 */
  nb = fread( ibuf, 1, BUFSIZ, stdin ) ; /* second BUFSIZ */
  MD5Update( &ctx, ibuf, nb ) ;
  MD5Final( &(key[16]), &ctx ) ;

  DES3_CBCInit( &dctx, key, iv, 1 ) ; /* encrypting (not that it matters) */

  while (!feof( stdin )) {
    nb = fread( bin, 1, 256, stdin ) ; /* get a block of input */
    nb = nb - (nb & 7) ;	/* nb now == 0 mod 8 */
    DES3_CBCUpdate( &dctx, bout, bin, nb ) ;
    fwrite( bout, 1, nb, stdout ) ;
  } /* while */

} /* process_message */


