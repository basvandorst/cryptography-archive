/*--- genguest -- Program to respond to an email request for FTP access,
 *  Creating an FTP "account".
 *
 *  This program responds to an incoming mail message
 *  by creating a host-specific "account" for my custom FTP server.
 *  It reads the request message from standard input and
 *  constructs a reply message from the headers of that message. 
 *  The reply message is written to stdout.
 *
 *  By Mark Riordan   21 Nov 1992.
 *  This program is in the public domain.
 */

#include <stdio.h>
#include "boolean.h"
#include "strutilp.h"
#include "global.h"
#include "rsaref.h"
#include "r_random.h"
#include "getretbp.h"
#include "normstrp.h"
#include "getsyspr.h"
#include "ftpfile.h"

void MakeUser(char *hostname, char *email_addr, char *username, char *pw);
void MapToAscii(char *str, int nchars);
int CopyFile(char *filenm, FILE *outstream);

extern int mkstemp(char *template);

int Debug = 0;
FILE *DebugStream = stderr;

int
main(argc,argv)
int argc;
char *argv[];
{
#define LINELEN 240
#define LICENSE_FILE "/home/scss3/ftp/pub/crypt/ripem/rsaref-license.txt"
	int msgfd;
	char *subject = "RIPEM Public Keys";
	char *return_addr;
	int ch;
	extern char *optarg;
	FILE *file_stream, *instream=stdin, *outstream=stdout, *termstream;
	int gotuser=0, gothost=0, gotfile=0;
	char hostname[LINELEN];
	char username[LINELEN], pw[LINELEN];
	char email_addr[LINELEN];
	char file_name[LINELEN];
	
		
   while(-1 != (ch = getopt(argc,argv,"h:e:f:"))) {
      switch(ch) {
			case 'e':  /* email addr */
				strncpy(username,optarg,LINELEN);
				gotuser=1;
				break;
			case 'h':  /* hostname */
				strncpy(hostname,optarg,LINELEN);
				gothost=1;
				break;
			case 'f':  /* name of user auth file */
				strncpy(file_name,optarg,LINELEN);
				gotfile=1;
				break;
			default:
				fputs("genguest: program to generate FTP account.\n",stderr);
				fputs("Usage:  genguest [-h hostname] [-e emailaddr] [-f authfile]\n",stderr);
				return 1;
				break;
		}
	}

	return_addr = GetEmailAddress(instream,outstream);
	if(!return_addr) {
		fprintf(stderr,"Can't find return address.\n");
		return 2;
	}
	strncpy(email_addr,return_addr,LINELEN);
	
	termstream = fopen("/dev/tty","r");
	EchoOn();
	fprintf(stderr,"Hostname? ");
	fgets(hostname,LINELEN,termstream);
	EchoRestore();
	NormalizeString(hostname);
	
	MakeUser(hostname,email_addr,username,pw);
	
	if(!gotfile) strcpy(file_name,FTP_GUEST_FILE);
	file_stream = fopen(file_name,"a");
	if(!file_stream) {
		fprintf(stderr,"Can't open file %s\n",file_name);
		return 1;
	}
	fprintf(file_stream,"%s %s %s %s\n",username,pw,hostname,email_addr);
	
	fprintf(outstream,"You have been assigned an FTP \"account\" on host rpub.cl.msu.edu.\n");
	fprintf(outstream,"The username is  guest.%s  and the password is  %s \n\n",
	   username,pw);
	fprintf(outstream,"This account can be used for FTP only, and it can be used\n");
	fprintf(outstream,"only from the host  %s.\n\n",hostname);
	fprintf(outstream,"You are encouraged to share software you find at this site,\n");
	fprintf(outstream,"as the processing of FTP account requests is only semi-automated\n");
	fprintf(outstream,"(i.e., requires effort from me to read your statement of citizenship).\n");
	fprintf(outstream,"However, please abide by USA export regulations and \n");
	fprintf(outstream,"RSAREF license terms.  A copy of this license is provided below.\n\n");
	fprintf(outstream,"(Do not reply to this semi-automated message; direct correspondence\n");
	fprintf(outstream,"to me at  mrr@scss3.cl.msu.edu.)\n\n");
	fprintf(outstream,"Thank you.\n\nMark Riordan\n");
		
	CopyFile(LICENSE_FILE,outstream);
	return 0;
}

/*--- function CopyFile ---------------------------------------------
 *
 *  Copy a named file to a stream.
 *
 *  Entry:	filenm		is the name of the file.
 *				outstream	is the stream.
 *
 *  Exit:	The file has been copied.
 */
int
CopyFile(filenm,outstream)
char *filenm;
FILE *outstream;
{
	FILE *instream;
	int ch;
	
	instream = fopen(filenm,"r");
	if(instream) {
		while(EOF!=(ch=getc(instream))) putc(ch,outstream);
	}
	fclose(instream);
	return 0;
}

/*--- function MakeUser ---------------------------------------------
 *
 *  Create an "account" for MRR's FTP server by creating a username
 *  and password. 
 *
 *  Entry:	hostname 	is the desired hostname.
 *				email_addr  is the desired email addr.
 *
 *  Exit:   username    is the generated username.
 * 			pw	         is the generated pw.
 */

void
MakeUser(char *hostname, char *email_addr, char *username, char *pw)
{
#define USER_LEN 8
#define PW_LEN   6
	R_RANDOM_STRUCT RanStruct;
	unsigned int bytes_needed;
	unsigned char ranbytes[LINELEN];
	int nbytes, retval;
	
	if(Debug>1) fprintf(DebugStream,"user=%x %x orig\n",username[0], username[1]);

	nbytes = GetRandomBytes(ranbytes,LINELEN);
	if(Debug) fprintf(DebugStream,"%d random bytes obtained.\n",nbytes);
	/* Seed the random number generator with the key. */
	R_RandomInit(&RanStruct);
	do {
		retval=R_RandomUpdate(&RanStruct,ranbytes,nbytes);
		if(Debug) fprintf(DebugStream,"R_Update ret %d\n",retval);
		R_RandomUpdate(&RanStruct,(unsigned char *)hostname,strlen(hostname));
		R_RandomUpdate(&RanStruct,(unsigned char *)email_addr,
		 strlen(email_addr));
		R_GetRandomBytesNeeded(&bytes_needed,&RanStruct);
	} while(bytes_needed);
	
	retval=R_GenerateBytes((unsigned char *)username,USER_LEN,&RanStruct);
	if(Debug>1) fprintf(DebugStream,"user=%x %x  retval=%d\n",username[0], username[1],retval);
	username[USER_LEN] = '\0';
	MapToAscii(username,USER_LEN);
	
	R_GenerateBytes((unsigned char *)pw,PW_LEN,&RanStruct);
	pw[PW_LEN] = '\0';
	MapToAscii(pw,PW_LEN);
}

/*--- function MapToAscii --------------------------------------------
 *
 *  Map a string of bytes to printable ASCII characters.
 *
 *  Entry: 	str		is a string of bytes.
 *				nbytes	is the number of bytes.
 *
 *	 Exit:	str		has been replaced with printable ASCII characters.
 */
void
MapToAscii(char *str, int nchars)
{
	static char map[] = {'a','b','c','d','e','f','g','h','i','j','k',
	 'm','n','p','q','r','s','t','u','v','w','x','y','z','2','3','4',
	 '5','6','7','8','9'};
	 
	while(nchars--) {
		*str = map[0x1f & *str];
		str++;
	}
}

