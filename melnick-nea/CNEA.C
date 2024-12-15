/*
			      CNEA

	     CNEA is a DOS command line utility used to encrypt and
	decrypt disk files.  It is meant to be used mainly as a vehicle
	to demonstrate the operation of the NEA encryption algorithm.
	     CNEA can optionally be used with command line arguments
	of the form:

		cnea infile outfile options

	where the options can be "E" or "D" for encrypt or decrypt plus
	optionally "C" to specify cypher block chaining.  If command
	line arguments are not used, file specifications and options
	will be prompted for during program execution.


								*/
#define ENCODE 0x01
#define	DECODE 0x02
#define CHAIN 0x04
#define INGOT 0x08
#define OUTGOT 0x10
#define BUFLEN 4096
#define FILL '\32'
#define MINLEN 4

#include "CTYPE.H"
#include "stdio.h"
#include "fcntl.h"
#include "sys\types.h"
#include "sys\stat.h"
#include "io.h"
#include "conio.h"
#include "nea.h"

/*								*/
int flags=0;
char key[8],iv[8],buf[BUFLEN];
/*								*/
main (argc,argv)
	int argc;
	char *argv[];
	{

	char *infil,*outfl,*optn;
	int arg,inhand,outhand,ansc,err,actby,rem,pad,i,j,k,temp;
	static char ans[] = {"\40\0                                "};
	static char an2[] = {"\40\0                                "};
	static char msgbf[40];

	cputs ("\15\12");
	cputs("CNEA  2.0\r\n");
	cputs ("\15\12");
	for ( arg = 2; (arg <= argc) && (arg < 5); ++arg )
		{
		switch (arg)
			{
			case 2:
				infil = argv [1];
				flags |= INGOT;
				break;
			case 3:
				outfl = argv [2];
				flags |= OUTGOT;
				break;
			case 4:
				for (optn=argv[3]; *optn!=0 && optn!=argv[3]+2; ++optn)
				{
				if (*optn == 'E' || *optn == 'e')
					flags |= ENCODE;
				if (*optn == 'D' || *optn == 'd')
					flags |= DECODE;
				if (*optn == 'C' || *optn == 'c')
					flags |= CHAIN;
				}
				break;
			}
		}
	if ( !(flags & INGOT) )
		{
		cputs ("INPUT FILE: ");
		infil = cgets (ans);
		cputs ("\15\12");
		}
	while ( (inhand = open(infil,O_BINARY | O_RDONLY)) == -1 )
		{
		sprintf (msgbf,"UNABLE TO OPEN %s ",infil);
		perror (msgbf);
		cprintf ("\15\12INPUT FILE: ");
		infil = cgets (ans);
		cputs ("\15\12");
		}
	if ( !(flags & OUTGOT) )
		{
		cputs ("OUTPUT FILE: ");
		outfl = cgets (an2);
		cputs ("\15\12");
		}
	if ( !(strcmp(infil,outfl)) )
		{
		cprintf ("\7CAN NOT OVERWRITE INPUT FILE");
		_exit (0);
		}
	else
		while ( (outhand = open(outfl,O_RDWR | O_BINARY | O_CREAT,S_IREAD | S_IWRITE)) == -1 )
			{
			sprintf (msgbf,"UNABLE TO CREATE %sOUTPUT FILE: ",outfl);
			perror (msgbf);
			cprintf ("\15\12OUTPUT FILE: ");
			outfl = cgets (ans);
			cputs ("\15\12");
			}
	if ( !( flags & (ENCODE | DECODE) ) )
		{
		cputs ("ENCODE OR DECODE? E/D: ");
		while ( ((ansc=getch()) != 'E') && (ansc!='e') 
		 && (ansc!='D') && (ansc!='d') );
		if ( (ansc == 'E') || (ansc == 'e') )
			flags |= ENCODE;
		if ( (ansc == 'D') || (ansc == 'd') )
			flags |= DECODE;
		cprintf ("%c\15\12CIPHER BLOCK CHAINING? Y/N: ",
		 (char)ansc);
		while ( ((ansc=getch()) != 'Y') && (ansc!='y') 
		 && (ansc!='N') && (ansc!='n') );
		if ( (ansc == 'Y') || (ansc == 'y') )
			flags |= CHAIN;
		if ( (ansc == 'N') || (ansc == 'n') )
			flags &= ~CHAIN;
		cprintf ("%c\15\12",(char)ansc);
		}
	getnum (key,"KEY");
	if ( flags & CHAIN )
		getnum ( iv,"INITIAL VECTOR" );
	keyp (key);
	 cputs ("\12\15\12\15PROCESSING\12\15");
	while ( (actby = read(inhand,buf,BUFLEN)) && (actby == BUFLEN) )
		{
		convert (actby);			
		if ( write(outhand,buf,BUFLEN) == -1 )
			{
			perror ("DISK WRITE ERROR!\15\12");
			_exit (0);
			}
		}
	if ( actby == -1 )
		{
		perror ("\7 DISK READ ERROR!\15\12");
		_exit (0);
		}
	if ( rem = actby % 8 )
		{
		pad = 8 - rem;
		for ( i = 0; i < pad; ++i )
			buf [actby + i] = FILL;
		rem = actby + pad;
		}
	else
		rem = actby;
	convert (rem);					
	if ( write(outhand,buf,rem) == -1 )
		{
		perror ("\7DISK WRITE ERROR!\15\12");
		_exit (0);
		}
	close ( outhand );
	close ( inhand );
	}
/*								*/
convert (len)
	int len;
	{
	char *bufpnt,*end,*xend,temp[8];
	int ce=0,cd=0,i;
	if ( flags & CHAIN )
		{
		ce = ( flags & ENCODE );
		cd = ( flags & DECODE );
		}
	end = buf + len;
	for ( bufpnt = buf; bufpnt < end; bufpnt += 8 )
		{
		if ( ce )
			{
			for ( i = 0; i < 8; ++i )
				*(bufpnt + i) ^= iv[i];
			nea ( bufpnt,bufpnt,encrypt );
			for ( i = 0; i < 8; ++i )
				iv [i] = *(bufpnt + i);
			}
		else if ( cd )
			{
			for ( i = 0; i < 8; ++i )
				temp [i] = *(bufpnt + i);
			nea (bufpnt,bufpnt,decrypt);
			for ( i = 0; i < 8; ++i )
				*(bufpnt + i) ^= iv[i];
			for ( i = 0; i < 8; ++i )
				iv [i] = temp [i];
			}
		else
		       {
			if ( flags & ENCODE )
				nea (bufpnt,bufpnt,encrypt);
			else
				nea (bufpnt,bufpnt,decrypt);
			}
		}
	}
/*								*/
/*                           GETNUM				*/
getnum (key,prompt)
	char *prompt,key[];
	{
	static char hexc[] = {"0123456789AaBbCcDdEeFf"};
	static char ans[] = {"\21\0                 "};
	static char stout[] = {'\0','\1','\2','\3','\4','\5','\6','\7','\10','\11','\12','\13','\14','\15','\16','\17'};
	static char stin[] = {"0123456789ABCDEF"};
	char *ascno,*hexno;
	int i,j,k,temp;
	char hexhld[] ={'\0','\0'};

	cprintf ("ENTER %s: ",prompt);
	ascno = cgets (ans);
	if ( (*(ans+1) == 1) && (*ascno == 'H' || *ascno == 'h') )
		{
		cputs ("\12\15ENTER HEX NUMBER: ");
		while ( strspn(hexno = cgets(ans),hexc) != 16 )
			{
			cputs ("\15\12HEX NUMBER MUST HAVE 16");
			cputs (" VALID HEX DIGITS!");
			cputs ("\15\12ENTER HEX NUMBER: ");
			}
		cputs ("\12\15");
		for ( i = 0,j = 0; i < 16; ++i,++j )
			{
			strupr (hexno);
			hexhld[0] = hexno[j++];
			k = strcspn (stin,hexhld);
			key[i] = stout[k] << 4;
			hexhld[0] = hexno[j];
			k |= strcspn (stin,hexhld);
			key[i] |= stout[k];
			}
		}

	else
		{
		while ( *(ans+1) < MINLEN )

			{
			temp = MINLEN;
			cprintf ("\15\12\7AT LEAST %d CHARACTERS REQUIRED!\r\n",temp);
			cprintf ("ENTER %s: ",prompt);
			ascno = cgets (ans);
			}
		cputs ("\15\12");
		hash (key,ascno);
		}
	}
/*                            HASH				*/
hash (key,ascstr)
	char *ascstr,*key;
	{
	int i,j,k;
	char inbl[8],otbl[8];
	static char ikey[] = {"(C) 1985"};
	static char ivect[] = {"AMELNICK"};
	keyp (ikey);
	for ( i = 0; i < 8; ++i )
		{
		inbl [i] = ivect [i];
		otbl [i] = ivect [i];
		}
	nea (otbl,otbl,encrypt);
	j = strlen (ascstr);
	for ( i = 0; i < j; ++i )
		{
		for ( k = 0; k < 7; ++k )
			inbl [k] = inbl [k+1];
		inbl [7] = otbl [0] ^ toupper (ascstr[i]);
		for ( k = 0; k < 8; ++k )
			otbl [k] = inbl [k];
		nea (otbl,otbl,encrypt);
		}
	for ( i = 0; i < 8; ++i )
		key [i] = otbl [i];
	}

