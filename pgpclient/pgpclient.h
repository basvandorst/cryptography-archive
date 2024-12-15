
#ifndef _H_PGPCLIENT /* Avoid double-inclusion as a matter of course */
#define _H_PGPCLIENT 000001 /* version 0.0.1 */
/*
 * Pgp server client program, based on the webster program by:
 *
 * David A. Curry
 * Purdue University
 * davy@ee.purdue.edu
 * April, 1986
 */

#include <stdio.h>

/* You can site-customise this header file by altering this to whichever
   host you want keys from - by default, it causes an error at runtime.
   Personally, I'd leave it as is and use an environment variable, to
   avoid accidents when someone else gets a binary copy of your prog...
 */

/* DEFAULT: #define PGPHOST "undefined" */
#define PGPHOST		"undefined"	/* host where the server lives	*/

#define PGPPORT		13013		/* port where the server lives	*/
#define PGPNAME		"pgp"		/* name of the service		*/
#define MINDELAY	60		/* no more often than one a min	*/

#define N_CLIENTERR 1000                /* Server return code must be <= 999 */

/* hostname may be NULL, portno may be 0                              */
/* returns FILE* to be used in pgp commands, or NULL                  */
FILE *open_pgp_server(char *hostname, int portno);

/* This version of command outputs its results to a file.             */
/* Other versions may follow after some thought...                    */
/* returns server rc, or >= N_CLIENTERR (rc > 999)                    */
/* (subtract N_CLIENTERR to get unix errno                            */
int pgp_fcommand(FILE *PGPSock, char *command, FILE *output);

/* Always remember to close the connection or you'll piss people off! */
/* returns TRUE or FALSE                                              */
int close_pgp_server(FILE *PGPSock);

#endif /* _H_PGPCLIENT */

