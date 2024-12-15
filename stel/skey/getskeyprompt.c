/*   get prompt code for S/KEY Authentication.  S/KEY is a trademark
 *   of Bellcore.
 *
 *   Mink is the former name of the S/KEY authentication system.
 *   Many references for mink  may still be found in this program.   */
 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include "skey.h"

char *skipspace();
#ifdef SKEYD
/* this violates standard skeylookup() syntax */
int skeylookup __ARGS((struct skey *mp,char *name, int staylocal));
#else
int skeylookup __ARGS((struct skey *mp,char *name));
#endif



/* Issue a skey challenge for user 'name'. If successful,
 * fill in the caller's skey structure and return 0. If unsuccessful
 * (e.g., if name is unknown) return -1.
 *
 * The file read/write pointer is left at the start of the
 * record.
 */
int
getskeyprompt(mp,name,prompt)
struct skey *mp;
char *name;
char *prompt;
{
	int rval;

	sevenbit(name);
#ifdef SKEYD
	rval = skeylookup(mp,name, 0);
#else
	rval = skeylookup(mp,name);
#endif
	strcpy(prompt,"s/key 55 latour1\n");
	switch(rval){
	case -1:	/* File error */
		return -1;
	case 0:		/* Lookup succeeded, return challenge */
		sprintf(prompt,"s/key %d %s\n",mp->n - 1,mp->seed);
		return 0;
	case 1:		/* User not found */
		fclose(mp->keyfile);
		return -1;
	}
	return -1;	/* Can't happen */
}	
