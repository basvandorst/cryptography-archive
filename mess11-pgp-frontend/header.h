#include <stdio.h>
#include <string.h>

/*	Header v1.0 Copyright 1994 Stuart Smith */

/*	Given a file containing a Usenet news article, header splits
	off the header and body into two specified files */
	
#define max_line_length 256


int	split_header ( char *article_file, char *header_file, char *body_file );
