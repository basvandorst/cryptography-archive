/*--- trim.c -- strip trailing blanks from text lines read from input.
 *
 *  Usage:  trim <in >out
 * 
 *  Mark Riordan  10 April 92
 */
#include <stdio.h>

int
main(argc,argv)
int argc;
char *argv[];
{
#define LINELEN 4000
	int mylen;
	char line[LINELEN];

	while(fgets(line,LINELEN,stdin)) {
		mylen = strlen(line)-1;
		while(line[mylen-1] == ' ' && mylen>0) {
			mylen--;
		}
		line[mylen] = '\0';
		puts(line);
	}
}
