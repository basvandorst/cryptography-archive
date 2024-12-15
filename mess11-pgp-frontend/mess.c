/*

mess - automagic Mail Encryption Signing system for mail & news

Copyright 1994 Stuart Smith
May be distributed under the terms of the GNU General Public License.

	To be run from a mail or news program in lieu of an editor.
	mess will call the editor and then encrypt or sign as required
	
	Usage:  mess <editor> <file>

	Version 1.1

*/

#include "mess.h"


int	search_header ( char *header_file, char *to, char *from )

/*	Search file for strings indicating sender and recipient */
/*  	returns 0 for normal operation, 1 if these are not headers */


#define to_header "To: "
#define from_header "From: "
#define subject_header "Subject: "

/*	I don't know enough to know which headers would be the best
	to read.  "From: " appears in both mail and news though, so it looks
	good.  Perhaps future versions would have multiple headers. */

/*  "Sender: " would probably be good to add - trn does pass this in posts */

{
	char	*buffer,
			*scratch;
	
	int		L_to, L_from, L_subject, is_header;
	
	FILE	*header;
	
/*	Attempt to open input file */

	if ( ( header = fopen ( header_file, "rt" ) ) == NULL ) {
		perror ( "Open header file" );
		return 1;
	}

	L_to = strlen ( to_header );
	L_from = strlen ( from_header );
	L_subject = strlen ( subject_header );
	
	buffer = (char *) malloc ( line_length );

	is_header = 0;
	
	while ( fgets (buffer, line_length, header) != NULL ) {
		if ( ! strncmp ( buffer, to_header, L_to ) ) {
			strcpy ( to, buffer + L_to );
			scratch = strstr ( to, " " );		/* Truncate at space or newline) */
			if ( scratch == NULL ) {
				scratch = strstr ( to, "\n" );
			}
			*scratch = NULL;
			is_header = 1;
			continue;
		}
		if ( ! strncmp ( buffer, from_header, L_from ) ) {
			strcpy ( from, buffer + L_from );
			scratch = strstr ( from, " " );
			if ( scratch == NULL ) {
				scratch = strstr ( from, "\n" );
			}
			*scratch = NULL;
			is_header = 1;
			continue;
		}		
	/*  Now we have to check for other possible headers to determine whether
		or not these are headers - trn, for instance, does not pass the 
		From: header to the editor, so without another test, the headers are
		treated as part of the body, since To: would not occur in a news post */

	/*  Subject: looks like a good bet.  Some problems though.. what if your
		mail or newsreader includes no subject header at all if you specify
		no subject?  This would again cause mess to fail, treating the headers
		as part of the message body.  Comments? */

		if ( ! strncmp ( buffer, subject_header, L_subject ) ) {
			is_header = 1;
			continue;
		}		
	

	}
	
	fclose ( header );
	
	return (! is_header);
}




int  main ( int argc, char *argv[] )

{
	char	*editor,
			*messagename,
			*command,
			*options,
			*buffer,
			*header_file,
			*body_file,
			*temp_file,
			*to,
			*from,
			*scratch;
				
	FILE	*input,
			*output,
			*header,
			*body;
			
	int	encrypt, sign, L_encrypt, L_sign, result;
	
	fputs (	"mess - automagic Mail Encryption and Signing System for mail & news\n\n" \
			"Copyright 1994 Stuart Smith\n" \
			"May be distributed under the terms of the GNU General Public License.\n", stderr );

	if ( argc != 3 ) {
		fputs ( "Incorrect number of command-line args!", stderr );
		return 1;
	}
		
	editor = argv[1];
	messagename = argv[2];

	command = (char *) malloc ( command_length );
	*command = NULL;

	strcat ( command, editor );
	strcat ( command, " " );
	strcat ( command, messagename );

	if ( system ( command ) ) {
		fputs ( "Error executing editor!", stderr );
		return 2;
	}
	
/* Get temporary file names then open them */

	header_file = (char *) malloc ( L_tmpnam );
	body_file = (char *) malloc ( L_tmpnam );
	temp_file = (char *) malloc ( L_tmpnam );
	tmpnam ( header_file );	
	tmpnam ( body_file );
	tmpnam ( temp_file );
	
	if ( split_header ( messagename, header_file, body_file ) != 0 ) {
		fputs ( "Error splitting article into header & body!", stderr );
		return 5;
	}	

/*	Now we init to & from and search the header file for clues ... */

	to 		= (char *) malloc ( line_length);
	*to 	= NULL;
	from	= (char *) malloc ( line_length);
	*from	= NULL;

	result = search_header ( header_file, to, from );	

/*	Uh-oh .. what if there wasn't really a header??
	Check to see if at least from_string matched - if not, whatever
	is in the header file isn't a header after all - copy original
	message to body file */
	
	if ( result ) {
		copy ( messagename, body_file );
		remove ( header_file );
		*to = NULL;  /* in case of a false hit */
		*from = NULL;
	}

/* Now we have to search file for special encrypt & sign strings */

/*	Attempt to open body file */

	if ( ( input = fopen ( body_file, "rt" ) ) == NULL ) {
		perror ( "mess" );
		return 3;
	}

/*	Attempt to open temporary file to write new body text in */

	if ( ( output = fopen ( temp_file, "wt" ) ) == NULL ) {
		perror ( "mess" );
		return 3;
	}

	options = (char *) malloc ( options_length );
	*options = NULL;
	
	encrypt = 0;
	sign = 0;

	buffer = (char *) malloc ( line_length );
		
	/* Save some work in the loop ... */

	L_encrypt 	= strlen(encrypt_string);   /* Length of encrypt string */
	L_sign		= strlen(sign_string);		/* Length of signing string */
	
	while ( fgets (buffer, line_length, input) != NULL ) {
		if ( (!encrypt) && ( ! strncmp ( buffer, encrypt_string, L_encrypt ) ) ) {
			strcat ( options, encrypt_options );
			scratch = strstr ( buffer, " " );  /* Look for key_ID */

			if ( scratch != NULL ) {
				strcpy ( to, scratch + 1 );  /* Get ID, skipping space */

				if ( ( scratch = strstr ( to, " " ) ) == NULL ) {
					scratch = strstr ( to, "\n" );		/* Truncate at newline */
				}

				*scratch = NULL;
			}
			encrypt = 1;
			continue;
		}
		if ( (!sign) && ( ! strncmp ( buffer, sign_string, L_sign ) ) ) {
			strcat ( options, sign_options );
			scratch = strstr ( buffer, " " );  /* Look for key_ID */

			if ( scratch != NULL ) {
				strcpy ( from, scratch + 1 );  /* Get ID, skipping space */

				if ( ( scratch = strstr ( from, " " ) ) == NULL ) {
					scratch = strstr ( from, "\n" ); /* Truncate at newline */
				}

				*scratch = NULL;
			}
			sign = 1;
			continue;
		}		
		fputs ( buffer, output );

	}
	
	fclose (input);
	fclose (output);
	
	if ( (encrypt == 0) && ( sign == 0 ) ) {
		remove (temp_file);
		remove (body_file);
		remove (header_file);
		return 0;  /* No PGP functions requested */
	}

/* Build PGP command line */

	*command = NULL;
	strcat ( command, encrypt_command );
	strcat ( command, default_options );
	strcat ( command, options );

	strcat ( command, " " ); /* No collisions please ... */
	
	if ( *to != NULL ) {
		strcat ( command, to );		/* Public key ID for encryption */
	}

	if ( *from != NULL ) {
		strcat ( command, " -u " );
		strcat ( command, from );	/* Secret key ID for signing */
	}

	strcat ( command, " < " );
	strcat ( command, temp_file );
	strcat ( command, " > " );
	strcat ( command, body_file );  /* back and forth ... */

	if ( system ( command ) ) {
		fputs ( "Error executing pgp!", stderr );
		fprintf ( stderr, "Command-line: %s\n", command );
		return 2;
	}						

/*	Move the header_file back to the original message */

	remove ( messagename );
	rename ( header_file, messagename );

/*	Finally we have to append the pgp encrypted message in body_file to
	then end of the original message where we have just copied the original
	headers  */

	append ( body_file, messagename );


/*	Tidy up... */

	remove ( temp_file );
	remove ( body_file );
	remove ( header_file );

	free (command);
	free (options);
	free (buffer);		
	free (to);
	free (from);
	free (temp_file);
	free (header_file);
	free (body_file);
	
	return 0;

}

		