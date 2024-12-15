/*
 * Copyright (c) 1992 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * version - determine the version of calc
 */

#include <stdio.h>

#define MAJOR_VER 1	/* major version */
#define MINOR_VER 24	/* minor version */
#define PATCH_LEVEL 7	/* patch level */

void
version(stream)
	FILE *stream;	/* stream to write version on */
{
	fprintf(stream, "calc version: %d.%d.%d\n", 
	    MAJOR_VER, MINOR_VER, PATCH_LEVEL);
}

/* END CODE */
