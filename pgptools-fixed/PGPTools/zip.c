/* Support code for the zip/unzip code - just handles error messages.  To
   get exact errors, define ZIPDEBUG */

#include <stdio.h>
#include <stdlib.h>
#include "usuals.h"
#include "fifo.h"
/* #include "fileio.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
Unnecessart stuff commented out */

#include "ziperr.h" /* for ZE_MEM (and errors[] if ZIPDEBUG defined) */

/* Clean error exit: c is a ZE_-class error, *msg is an error message.
   Issue a message for the error, clean up files and memory, and exit */

void err(int c, char *msg)
	{

#ifdef ZIPDEBUG
	if (PERR(c))
		perror("zip error");
	fprintf(stderr, "zip error: %s (%s)\n", errors[c-1], msg);
#endif /* ZIPDEBUG */

	/* Complain and return and out of memory error code */
	if(c==ZE_MEM)
	{  /*	fprintf( stderr, PSTR("\nOut of memory\n") );
		exitPGP( 7 );   */     out_of_memory(); 
	}
	else
	{    /*	fprintf( stderr, PSTR("\nCompression/decompression error\n") );*/
		/* Yuck */ /* You got it! We need a better way. */  /*
		exitPGP( 23 );     */ printf("ZIP error %s\n",msg);
				      exit(1);
	}
	}

/* Internal error, should never happen */

void zip_error(char *msg)
	{
	err(-1, msg);
	}
