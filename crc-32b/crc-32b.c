From msuinfo!agate!howland.reston.ans.net!gatech!udel!princeton!gw1!att-out!undergrad.math.uwaterloo.ca!watserv2.uwaterloo.ca!ccnga.uwaterloo.ca!csbruce Mon Jun 27 16:02:57 1994
Newsgroups: comp.programming,alt.sources
Path: msuinfo!agate!howland.reston.ans.net!gatech!udel!princeton!gw1!att-out!undergrad.math.uwaterloo.ca!watserv2.uwaterloo.ca!ccnga.uwaterloo.ca!csbruce
From: csbruce@ccnga.uwaterloo.ca (Craig Bruce)
Subject: Re: CRC-16/CRC-32
Message-ID: <Cs1uJE.Lxr@watserv2.uwaterloo.ca>
Sender: news@watserv2.uwaterloo.ca
Organization: University of Waterloo, Canada (eh!)
Date: Mon, 27 Jun 1994 09:33:13 GMT
Lines: 92
Xref: msuinfo comp.programming:10356 alt.sources:10476

>Does anyone have a code fragment or algorithm for this function. Any 
>language accepted (well, not APL perhaps). 

Damn... had to translate the following from APL into C.   :-)

Keep on Hackin'!

-Craig Bruce
csbruce@ccnga.uwaterloo.ca
"Things always look better when you can't see them."
-----=-----
/*  CRC-32b version 1.03 by Craig Bruce, 27-Jan-94
**
**  Based on "File Verification Using CRC" by Mark R. Nelson in Dr. Dobb's
**  Journal, May 1992, pp. 64-67.  This program DOES generate the same CRC
**  values as ZMODEM and PKZIP
**
**  v1.00: original release.
**  v1.01: fixed printf formats.
**  v1.02: fixed something else.
**  v1.03: replaced CRC constant table by generator function.
*/

#include <stdio.h>

int main();
unsigned long getcrc();
void crcgen();

unsigned long crcTable[256];

/****************************************************************************/
int main( argc, argv )
	int argc;
	char *argv[];
{
	int	i;
	FILE   *fp;
	unsigned long crc;

	crcgen();
	if (argc < 2) {
		crc = getcrc( stdin );
		printf("crc32 = %08lx for <stdin>\n", crc);
	} else {
		for (i=1; i<argc; i++) {
			if ( (fp=fopen(argv[i],"rb")) == NULL ) {
				printf("error opening file \"%s\"!\n",argv[i]);
			} else {
				crc = getcrc( fp );
				printf("crc32 = %08lx for \"%s\"\n",
					crc, argv[i]);
				fclose( fp );
			}
		}
	}
	return( 0 );
}

/****************************************************************************/
unsigned long getcrc( fp )
	FILE *fp;
{
	register unsigned long crc;
	int c;

	crc = 0xFFFFFFFF;
	while( (c=getc(fp)) != EOF ) {
		crc = ((crc>>8) & 0x00FFFFFF) ^ crcTable[ (crc^c) & 0xFF ];
	}
	return( crc^0xFFFFFFFF );
}

/****************************************************************************/
void crcgen( )
{
	unsigned long	crc, poly;
	int	i, j;

	poly = 0xEDB88320L;
	for (i=0; i<256; i++) {
		crc = i;
		for (j=8; j>0; j--) {
			if (crc&1) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc >>= 1;
			}
		}
		crcTable[i] = crc;
	}
}

