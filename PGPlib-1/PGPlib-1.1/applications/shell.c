/*********************************************************************
 * Filename:      shell.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:11:27 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <pgplib.h>
#include "shell.h"

#ifndef HAVE_STRSEP
#  include "strsep.c"
#endif


int verbose = 1;


static struct {
  void    (*command)(int ,char* [] );
  char    *name;
  char    *doc;  /* NULL when help info should not be printed */
} cmdtbl[] = {
    { add,	"ADD",		"ADD file [armor]: add keys and/or certificates"},
    { base_open,"DBOPEN",	"DBOPEN name [force|writer]: open name as a database"},
    { get_key,	"GET",		"GET [0x12345678 | 0x1234567890123456 | Foo Bar]"},
    { help,	"HELP",		"HELP: list the commands"},
    { quit,	"QUIT",		"QUIT: (guess what)"},
    { scan, 	"SCAN",		"SCAN file [armor]"},
    { verify,	"VERIFY", 	"VERIFY 0x1234567890123456"},
    { NULL, 	NULL, 		NULL}
};

void
help(int argc, char *argv[])
{
    int 	i;

    switch (argc) {
    case 1:
	for( i=0; cmdtbl[i].name; i++ )
	    fprintf(stdout, "\t%s\t%s\n", cmdtbl[i].name, cmdtbl[i].doc);
	return;
    case 2:
	for( i=0; cmdtbl[i].name; i++ )
	    if ( ! strcasecmp(cmdtbl[i].name, argv[1])) {
		fprintf(stdout, "\t%s\t%s\n", cmdtbl[i].name, cmdtbl[i].doc);
		return;
	    }
	fprintf(stdout, "HELP: %s: Unknown command (try HELP)\n", argv[1]);
	return;
    default:
	for(i=0; cmdtbl[i].command != NULL; i++)
	    fprintf(stdout, "\t%s\t%s\n", cmdtbl[i].name, cmdtbl[i].doc);
    }
}

void
quit(int argc, char *argv[])
{
    keydb_end();
    exit(0);
}

char 	Arg[MAXLINE];

void
main(void)
{
    char 	line[MAXLINE];
    char 	*p, *val, **commands, **tmp;
    int 	num_arg, i;
    void 	(*func)(int, char* []);

    commands = NULL;
    fprintf(stdout, PROMPT);
    while ( fgets(line, MAXLINE, stdin) != NULL ) {

	/* free(NULL) is OK */
	if ( commands != NULL)
	    free(commands);
	commands = 0;

	p = strrchr(line, '\n');
	assert(*p == '\n');
	*p = 0;

	/* Keep a copy of the arguments on the command line */
	strncpy(Arg, line, MAXLINE);

	num_arg = 0;
	p = line;
	while(p != NULL ) {
	    while ( (val = strsep(&p, " \t")) != NULL && *val == '\0')
		/*void*/;
	    if ( val == NULL )
		break;
	    if ( commands == NULL )
		tmp = calloc ((num_arg+1) * sizeof(char *), 1);
	    else
		tmp = realloc (commands, (num_arg+1) * sizeof(char *));
	    if ( tmp == NULL ) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	    }
	    commands = tmp;
	    commands[num_arg] = val;
	    num_arg += 1;
	}
	func = NULL;

	/* a blank line will leave us with commands==NULL which will
	 * core dump inside the for loop if not trapped here
	 */
	if (commands == NULL) {
	    fprintf(stderr, "Unknown command: %s (try HELP)\n", "");
	    fprintf(stdout, PROMPT);
	    continue;
	}

	for( i=0; cmdtbl[i].name; i++ )
	    if ( ! strcasecmp(cmdtbl[i].name, commands[0])) {
		func = cmdtbl[i].command;
		break;
	    }
	if ( func == NULL ) {
	    fprintf(stderr, "Unknown command: %s (try HELP)\n", commands[0]);
	    fprintf(stdout, PROMPT);
	    continue;
	}
	else
	    (*func)(num_arg, commands);
	fprintf(stdout, PROMPT);

    }
    /* End of commands */
    quit(0, NULL);
}

