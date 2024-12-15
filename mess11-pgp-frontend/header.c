#include "header.h"

/*	Header v1.0 Copyright 1994 Stuart Smith */

/*	Given a file containing a Usenet news article, header splits
	off the header and body into two specified files */


int	split_header ( char *article_file, char *header_file, char *body_file )
{

	char	*input;

	FILE	*article,
			*header,
			*body,
			*output;

	article = header = body = NULL;

/*	 Attempt to open all files */
			
	if ( (article = fopen ( article_file, "rt" ) ) == NULL ) {
		perror ( "open article file" );
		return 1;
	}
	if ( (header = fopen ( header_file, "wt+" ) ) == NULL ) {
		perror ( "open header file" );
		return 1;
	}
	if ( (body = fopen ( body_file, "wt+" ) ) == NULL ) {
		perror ( "open body file" );
		return 1;
	}

/*	Until the first blank line, all output goes to the header file */

	output = header;

	input = (char *) malloc ( max_line_length);
	
	while ( fgets ( input, max_line_length, article ) != NULL ) {
		if ( input != NULL ) fputs ( input, output );

		if ( ! strcmp ( input, "\n" ) )	{
			output = body;  /* ye old switcheroo! */
		}
		
	} 
	
	fclose ( header );
	fclose ( body );
	fclose ( article );
	
	return 0;
	
}
	
