/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

#include <time.h>	/* needed for time() */
#include "csu.h"	/* needed for MAX_LENGTH */

/* This function just gets the time and process id and converts it to a string. */
char *genstr(void) {
	char *randomstring;

	/* I know that I probably don't need so much space for this string
	 * but it doen't matter much.  If you are tight on space then change
	 * it to something around 30 */

	randomstring = (char *)malloc(sizeof(randomstring) * MAX_LENGTH);
	sprintf(randomstring, "%d%d", (int)time(NULL), (int)getpid());

#ifdef DEBUG
	printf("String: %s\n",randomstring);
#endif
	return(randomstring);
}
