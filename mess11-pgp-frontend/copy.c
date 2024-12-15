/*
Copyright 1994 Stuart Smith
May be distributed under the terms of the GNU General Public License.
*/

#include <stdio.h>
#include <stdlib.h>
#include "copy.h"


#define BUFFER_SIZE 65535

/*	copies or appends output file to input file */

int  copy_file ( char *input_name, char *output_name, int append )
{
	FILE *input, *output;

	int data_byte, count, buffer_count = 0;
	unsigned char *buffer;
	
	if ( (input = fopen ( input_name, "rb" ) ) == NULL )
	{
		perror ("copy");
		return 2;
	}

	if ( (output = fopen ( output_name, append ? "ab" : "wb+" ) ) == NULL )
	{
		perror ("copy");
		return 2;
	}

/* Do whatever */

	buffer = (unsigned char *) malloc ( BUFFER_SIZE + 1);  /* Extra byte for NULL */

	*(buffer + BUFFER_SIZE) = NULL;  /* integer to pointer without a cast ? */

	do {
		if ( buffer_count == BUFFER_SIZE ) {  /* Buffer full */
			for ( count = 0; count < BUFFER_SIZE; count++ )
			{
				fputc ( *(buffer + count), output );
			}
			buffer_count = 0;
		}
		
		data_byte = fgetc ( input );
		
		if ( data_byte != EOF )
		{
			*(buffer + buffer_count++) = (unsigned char) data_byte;
		}
	}  while ( data_byte != EOF );

	/* Empty buffer */

	for ( count = 0; count < buffer_count; count++ )
	{
		fputc ( *(buffer + count), output );
	}
	
	free ( buffer );

	if ( fclose ( input ) == EOF )
	{
		fprintf ( stderr, "Cannot close file:  %s\n", input_name );
		return 2;
	}

	if ( fclose ( output ) == EOF )
	{
		fprintf ( stderr, "Cannot close file:  %s\n", output_name );
		return 2;
	}
		
	return 0;
}

int	append ( char *input_name, char *output_name )
{
	return copy_file ( input_name, output_name, 1 );
}

int	copy ( char *input_name, char *output_name )
{
	return copy_file ( input_name, output_name, 0 );
}

