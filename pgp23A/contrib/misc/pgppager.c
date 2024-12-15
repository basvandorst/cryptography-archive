
/* pgppager, designed to be possibly integrated with elm mail reader
 * 
 * by Alessandro Bottonelli (c)1993, A.Bottonelli@it12.bull.it
 * version 1.0 - 29/Jan/93
 *
 * This program is put in the public domain and permission is granted
 * to use it for any purpose, provided this copyright notice is retained.
 * NO WARRANTIES expressed or implied are given. USE THIS PROGRAM AT
 * YOUR OWN RISK!
 *
 * This programs reads from a specified file or from stdin if no file is
 * specified and creates three temporary files (header, encrypted, and 
 * trailer) as needed, in order to store the header portion in cleartext,
 * the encrypted portion still in cypher text, and the trailer portion of
 * the clear text. Then, if applicable, the cleartext header is outputted,
 * the encrypted portion is piped through pgp as needed, then the trailer
 * (if any) is outputted. THIS PROCESS IS TRANSPARENT TO NON PGP ENCRYPTED
 * TEXTS
 *
 * For use with ELM, it is necessary to specify the display to be:
 *
 *    <whatever_path>/pgppager | pg
 *
 * in the elm option menu.
 * 
 */

#include <stdio.h>

/*
 * you may need to customize the next DEFINEs
 */

#define PGP "/usr/local/bin/pgp -f"
#define BEGIN "-----BEGIN PGP MESSAGE---"
#define END "-----END PGP MESSAGE---"


#define TEMP "/usr/tmp"		/* Temporary dir of your choice       */
#define CAT "cat -s"		/* -s option to avoid cat complaining */
				/* about missing files                */

/*
 * program should be invoked as "pgppager [file] | pg"
 * actually no check is made to verify that the program
 * is piped through "pg" or "more", just in case someone
 * wants to redirect it to a file or a printer.
 * BEWARE: redirecting it to a file or printer may be
 * not good for your security if the file or printer level
 * of security is unknown or poor ...
 */

/*
 * This three variables global to be visible to the Exit() routine
 */

char fh[128];	/* Header temporary file        */
char fe[128];	/* Encrypted temporary file     */
char fr[128];	/* Trailer temporary file       */

main(ac,av)
int ac;
char **av;
{
	FILE *fp;	/* File pointer to read file(s) */
	FILE *ft;	/* File pointer to temp file(s) */
	char line[256]; /* Read/Write buffer            */
	char cmd[128];	/* command string               */



/* 
 * Must be invoked as "pgppager [file_name]"
 */

	if ( ac > 2 )
	{
		fprintf(stderr,"Usage: %s [file_name]\n", av[0]);
		Exit(1);
	}

/*
 * File ( if specified ) must exist and be readable
 */

	if ( ac == 2 )
	{
		if ( ( fp = fopen( av[1], "r") ) == NULL )
		{
			fprintf(stderr,"%s: cannot read %s\n", av[0], av[1]);
			Exit(2);
		}
	}
	else
		fp=stdin;	/* if no file specified assume standard input */

/*
 * Build temporary file names
 *
 * please note how, as files are created for writing, they are
 * also chmod(ed) to 0600. I expect most PGP users to have their
 * umask set to 077, but just to be on the safe side ... :-)
 */

	sprintf ( fh, "%s/pgpm.head.%d", TEMP, (int )getpid() );
	sprintf ( fe, "%s/pgpm.encr.%d", TEMP, (int )getpid() );
	sprintf ( fr, "%s/pgpm.trai.%d", TEMP, (int )getpid() );


/* 
 * READ file until Eof, redirect to "fe"  only those portions of the
 * file that are between the BEGIN PGP and END PGP MESSAGE lines.
 */
	if ( ( ft = fopen ( fh, "w") ) == NULL )
	{
		fprintf(stderr,"%s: error opening temp file\n", av[0]);
		Exit(4);
	}
	
	if ( (int )chmod( fh, 0600) != 0 )
	{
		fprintf(stderr,"%s: cannot chmod temp file\n", av[0]);
		Exit(5);
	}

	fprintf( ft, "\n");	/* Just to start with a blank line */

	while ( fgets( line, 128, fp) != NULL )
	{
		if ( !strncmp( BEGIN, line, strlen( BEGIN) ))	
		{
			fprintf(ft, "\n===pgp encrypted message begins===\n");
			fclose(ft);

			if ( ( ft = fopen ( fe, "w") ) == NULL )
			{
				fprintf(stderr,"%s: error opening temp file\n", av[0]);
				Exit(4);
			}

			if ( (int )chmod( fe, 0600) != 0 )
			{
				fprintf(stderr,"%s: cannot chmod temp file\n", av[0]);
				Exit(5);
			}
		}

		fprintf( ft, "%s", line);

		if ( !strncmp( END, line, strlen( END) ))
		{
			fclose(ft);

			if ( ( ft = fopen ( fr, "w") ) == NULL )
			{
				fprintf(stderr,"%s: error opening temp file\n", av[0]);
				Exit(4);
			}

			if ( (int )chmod( fr, 0600) != 0 )
			{
				fprintf(stderr,"%s: cannot chmod temp file\n", av[0]);
				Exit(5);
			}

			fprintf(ft, "\n===pgp encrypted message ends===\n");
		}
	}
	
	fclose(fp);
	fclose(ft);	

	sprintf( cmd, "%s %s", CAT, fh);
	system(cmd);
	sprintf( cmd, "%s %s | %s", CAT, fe, PGP);
	system(cmd);
	sprintf( cmd, "%s %s", CAT, fr);
	system(cmd);

	Exit(0);	/* That's all folks ... */
}

/*
 * Exit(s) are centralized in this routine to make sure we wipe out
 * temporary files in any case ...
 */

Exit(n)
int n;		/* value to be returned to the shell (0 = OK) */
{
/*
 * Remove temporary files (if any)
 * and sync() just for the sake of it
 */

	unlink(fh);
	unlink(fe);
	unlink(fr);
	sync();

	exit(n);
}

