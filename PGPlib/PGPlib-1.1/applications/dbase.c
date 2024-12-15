/*********************************************************************
 * Filename:      dbase.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:47:54 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


#include <pgplib.h>
#include "shell.h"
#include <ctype.h>

void
base_open(int argc, char *argv[])
{
    int		force, writer;
    char	*p;

    if (argc !=2 && argc != 3 ) {
	fprintf(stdout, "DBOPEN name.  Opens name.{keys,string,keyid} as database\n");
	return;
    }
    force = writer = 0;
    if ( argc == 3 ) {
	for(p=argv[2]; *p != 0; p++)
	    *p = tolower(*p);
	if ( ! strcmp(argv[2], "force"))
	    force = 1;
	else if ( ! strcmp(argv[2], "writer"))
	    writer = 1;
	else {
	    fprintf(stderr, "DBOPEN: arg 3 MUST be \"writer\" or \"force\" or nothing (not \"%s\")\n",
		    argv[2]);
	    return;
	}
    }
    if ( force == 1 )
	keydb_init(argv[1], NEWDB);
    else if ( writer == 1 )
	keydb_init(argv[1], WRITER);
    else
	keydb_init(argv[1], READER);

    return;
}
