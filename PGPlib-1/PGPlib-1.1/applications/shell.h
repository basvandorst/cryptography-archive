/*********************************************************************
 * Filename:      shell.h
 * Description:   
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Tue May 13 02:06:51 1997
 * Modified by:   Tage Stabell-Kulo <tage@acm.org>
 * 	
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


void 
add	(int, char * []);
void
base_open	(int, char *[]);
void
get_key	(int, char * []);
void
help	(int, char * []);
void
scan	(int, char * []);
void
sign	(int, char * []);
void
verify	(int, char * []);

void
quit	(int, char * []);

#define	PROMPT	">"

extern char Arg[];
