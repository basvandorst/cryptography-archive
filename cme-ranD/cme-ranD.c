From cme@clark.net Wed Jan 25 01:37:59 1995
Received: from clark.net by scss3.cl.msu.edu (4.1/4.7)  id AA21630; Wed, 25 Jan 95 01:37:55 EST
Received: (cme@localhost) by clark.net (8.6.9/8.6.5) id BAA09709; Wed, 25 Jan 1995 01:37:53 -0500
Date: Wed, 25 Jan 1995 01:37:53 -0500
From: Carl Ellison <cme@clark.net>
Message-Id: <199501250637.BAA09709@clark.net>
To: rsaref-administrator@rsa.com
Subject: ranD.c
Cc: mrr@scss3.cl.msu.edu
Content-Length: 3116
Status: ORr

This isn't on the FTP server because clark.net isn't set up to
distribute crypto code.  If you'd care to do so, you're welcome.

 - Carl

==============================================================================

/***************************************************************************/
/* ranD.c -- process stdin by sending it through triple-DES, using keys    */
/*   built from the input (via MD5).                                       */
/*                                                                         */
/*   Copyright (c) 1993, 1994, 1995 Carl M. Ellison                        */
/*                                                                         */
/*   This software may be copied and distributed for any purposes          */
/*   provided that this copyright and statement are included in all such   */
/*   copies.                                                               */
/***************************************************************************/

#include <stdio.h>

#include "global.h"		/* RSAREF global defs */
#include "rsaref.h"           /* RSAREF definitions */

#define BLOCK_SIZE  (256)	/* processing block */

static void do_stream() ;

/***************************************************************************/
/* main                                                                    */
/***************************************************************************/

main(argc, argv)
int argc;
char **argv;
{
  int	ch, sl ;
  int  c ;

  do_stream() ;

  exit(0);
} /* main */

/***************************************************************************/
/* do_stream -- read stdin, gathering input for MD5, writing blocks to     */
/*       stdout.                                                           */
/***************************************************************************/

void do_stream()
{
  MD5_CTX ctx ;			/* context for MD5 */
  long nb, nb2r, nbvar ;	/* # bytes read, # to read, # variation */
  unsigned int i ;
  unsigned char key[32] ;	/* output from MD5 */
  unsigned char *iv ;		/* last 16 bytes of key */
  unsigned char bin[BLOCK_SIZE],
                bout[BLOCK_SIZE] ; /* do blocks of BLOCK_SIZE bytes */
  DES3_CBC_CTX dctx ;		/* DES context */

  iv = &(key[24]) ;		/* last 16 bytes */

  MD5Init( &ctx ) ;		/* init the first MD5 */
  nb = fread( bin, 1, BLOCK_SIZE, stdin ) ; /* get the first batch */
  MD5Update( &ctx, bin, nb ) ;
  MD5Final( key, &ctx ) ;

  MD5Init( &ctx ) ;		/* the second MD5 */
  nb = fread( bin, 1, BLOCK_SIZE, stdin ) ; /* second batch */
  MD5Update( &ctx, bin, nb ) ;
  MD5Final( &(key[16]), &ctx ) ;

  DES3_CBCInit( &dctx, key, iv, 1 ) ; /* encrypting (not that it matters) */
				/* you can't decrypt this stream */
				/* which suggests that ranD should be */
				/* exportable.... */

  while (!feof( stdin )) {
    nb = fread( bin, 1, BLOCK_SIZE, stdin ) ; /* get a block of input */
    nb = nb - (nb & 7) ;	/* nb now == 0 mod 8 */
    DES3_CBCUpdate( &dctx, bout, bin, nb ) ;
    fwrite( bout, 1, nb, stdout ) ;
  } /* while */

} /* do_stream */




