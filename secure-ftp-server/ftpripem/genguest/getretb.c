/*--- getretb.c ---*/

#include <stdio.h>
#include <string.h>
#include "boolean.h"
#include "strutilp.h"
#include "getretbp.h"

/*--- function GetEmailAddress ------------------------------------------
 *
 *  Read the header section of a file containing an electronic mail
 *  message, and extract the email address of the sender.
 *  Look for a "Reply-To:", or else a "From:" line.
 *
 *  Entry	instream		is a stream containing lines of an email message.
 *
 *	 Exit		Returns a pointer to the sender's address, else NULL.
 *				The string (if any) is alloc'ed.
 *				instream is positioned after the first blank line.
 */
 
char *
GetEmailAddress(instream,outstream)
FILE *instream;
FILE *outstream;
{
#define LINELEN 1024
	char line[LINELEN];
	BOOL processing = TRUE, inheader=TRUE;
	char *reply_addr = NULL, *cptr;
	
	while(processing) {
		if(!fgets(line,LINELEN,instream)) break;
			
		if(inheader) fputs(line,outstream);
		if(LineIsWhiteSpace(line)) inheader=FALSE;
		
		fputs(line,stderr);
		
		if(inheader && matchn(line,"To: ",4)) {
			cptr = ExtractEmailAddr(line+4);
			if(reply_addr) free(reply_addr);
			strcpyalloc(&reply_addr,cptr);
		}	
	}
	
	return reply_addr;
}

