/*
 * Copyright (c) 1990 David G. Koontz.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertisiing materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the above mentioned individual.
 * THIS SOFTWARE IS PROVIDED AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE
 */
#ifndef lint
char copyright[]=
    "@(#) Copyright (c) 1990 David G. Koontz\n All rights reserved.\n";
#endif

/*  des -- perform encryption/decryption using DES algorithm */

#define TRUE		1
#define FALSE		0
#define SHIFT		1
#define NOSHIFT		0
extern int  optind,opterr;
extern char *optarg;
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
char *getpass();

static char null_salt[8] = {0x2e,0x2e,0,0,0,0,0,0}; 

#define BYTES_PER_BLOCK	    8
#define NYBBLES_PER_BLOCK   16

#define NUMBER_OF_KEY_BYTES 8
#define BITS_PER_NYBBLE	    4
#define BITS_PER_BYTE	    8
#define BITS_PER_BLOCK	    64
#define BIT(x)		    ( 1 << x)
#define TODEC		    16
#define ENCRYPT		    0
#define DECRYPT		    1
 
main (argc,argv)
int argc;
char **argv;
{
    unsigned char input[BYTES_PER_BLOCK], output[BYTES_PER_BLOCK];
    unsigned char bits[BITS_PER_BLOCK];
    unsigned char key[NUMBER_OF_KEY_BYTES];
    unsigned char testinput[128];
    unsigned char testkey[NYBBLES_PER_BLOCK+1];
    unsigned char testplain[NYBBLES_PER_BLOCK+1];
    unsigned char testcipher[NYBBLES_PER_BLOCK+1];
    unsigned char testresult[NYBBLES_PER_BLOCK+1];
    int len,c;
    int verbose = FALSE;
    int testerrors = 0;
    int totalerrors = 0;
    int testcount = 0;
    int needkey = TRUE;
    int desmode = ENCRYPT;
    int testmode = FALSE;
    char padchar = 0x20;	/* default pad character is space */

    if (argc < 2)
	goto usage;

    crypt(null_salt,null_salt);	      /* initialize the E[] array */

    while (( c = getopt(argc,argv,"edk:K:p:tv")) != EOF) {
	switch (c){
	    case 'e':
		desmode = ENCRYPT;
		break;
	    case 'd':
		desmode = DECRYPT;
		break;
	    case 'k':
		if (needkey) {
		    needkey = FALSE;
		    if ( (c = optarg[0]) == '-')  /* prevent lost data */
			goto usage;
		    if ( (c = strlen(optarg)) < 8) {
			fprintf(stderr,"%s: key must be 8 char\n",argv[0]);
			exit(1);
		    }
		    loadkey(optarg,SHIFT);
		}
		else {
		    fprintf(stderr,"%s: too many keys\n",argv[0]);
		    exit(1);
		}
		break;
	    case 'K':
		if ( needkey ) {
		    needkey = FALSE;
		    strxtoc(optarg,key);  /* will complain about bad format */
		    while (*optarg) *optarg++ = 0; 
		    loadkey(key,NOSHIFT);
		}
		else {
		    fprintf(stderr,"%s: too many keys\n",argv[0]);
		    exit(1);
		}
		break;
	    case 'p':
		padchar = (unsigned char) strtol(optarg,0,TODEC);
		break;
	    case 't':
		testmode = TRUE;
		break;
	    case 'v':
		verbose = TRUE;
		break;
	    case '?':
usage:		fprintf(stderr,"Usage: %s -e | -d ",argv[0]);
		fprintf(stderr,"[-k key | -K hex_key] "); 
		fprintf(stderr,"[-p hex_pad_char]\n\n"); 
		fprintf(stderr,"   Or: %s -t [-v]\n\n",argv[0]);
		exit(1);
		break;
	}
    }
    if (needkey && !testmode ) {
	strncpy(key,getpass("key: "),8);
	if ( (c = strlen(key)) < 8)  {
	    fprintf(stderr,"%s: key must be 8 char\n",argv[0]);
	    exit(1);
        }
	loadkey(key,SHIFT);
    }

    if ( !testmode)	    /* regular operation loop */

	while ((len = fread(input, 1, BYTES_PER_BLOCK, stdin)) > 0) {
	    if ( len < BYTES_PER_BLOCK ) {	/* encrypt mode only */
		while (len < BYTES_PER_BLOCK)
		input[len++]=padchar;
	    } 
	    bytes_to_bits(input, bits);
	    encrypt(bits,desmode);
	    bits_to_bytes(bits,output);
	    fwrite(output, 1, BYTES_PER_BLOCK, stdout);
	}
    else    /* test mode */
    {
 	while (fgets(testinput,(sizeof testinput) -1, stdin) != NULL ) {

	    if ( strncmp(testinput,"encrypt",7) == 0) { /* mode = encode */
	        desmode = ENCRYPT;
	        fprintf(stderr,"%s",testinput);
	    }
	    else
	    if ( strncmp(testinput,"decrypt",7) == 0) { /* mode = decode */
	        fprintf(stderr,"%s",testinput);
	        desmode = DECRYPT;
	    }
	    else 
	    if ( strncmp(testinput," ",1) == 0) { /* key, plain & cipher */
		testcount++;
	        len = sscanf(testinput,"%s%s%s*",
	    	    testkey, testplain, testcipher);
		if ( verbose )  {
		    fprintf(stderr," %s %s %s\n", testkey, testplain,
							   testcipher);
		}
		strxtoc(testkey,key);
		loadkey(key,NOSHIFT);
		strxtoc(testplain,input);
		bytes_to_bits(input,bits);
		encrypt(bits,desmode);
		bits_to_bytes(bits,output);
		strctox(output,testresult);
		if ( (len = strncasecmp(testcipher,testresult,16)) != 0 ) {
		    fprintf(stderr,"Test: %d -- ERROR expected %s got %s\n",
			    testcount,testcipher,testresult);
		    testerrors++;
		}
	    }
	    else {				  /* nothing but eyewash */
		if ( testcount ) {
		    fprintf(stderr," %d tests performed\n",testcount);
		    fprintf(stderr," ERRORS on these tests : %d\n",testerrors);
		    totalerrors +=testerrors;
		    testcount = 0;
		    testerrors = 0;
		}
		fprintf(stderr,"%s",testinput);
	    }
	}
    fprintf(stderr,"Total Errors = %d\n",totalerrors);
    }
    return(0);
}

loadkey(key,shift)
char *key;
int shift;
{
    int i;
    char bits[BITS_PER_BLOCK];
    if (shift)
	for (i=0;i<NUMBER_OF_KEY_BYTES;i++)
	    key[i] = (key[i] << 1);
    bytes_to_bits (key, bits);
    setkey(bits);
    while (*key) *key++ = 0;
}

bytes_to_bits (bytes,bits)  
unsigned char *bytes,*bits;
{
register int i,j,k;

    for(i=0,j=0;i < BYTES_PER_BLOCK;i++) {
	for (k=7;k >= 0;k--) {
	    if (bytes[i] & BIT(k))
		bits[j++]=1;
	    else
		bits[j++]=0;
	}
    }
}

bits_to_bytes (bits,bytes)
unsigned char *bits,*bytes;
{
register int i,j,k;
    
    for (i=0,j=0;i < BYTES_PER_BLOCK;i++)
	bytes[i]=0;

    for (i=0,j=0;i < BITS_PER_BYTE;i++) {
	for (k=7;k >= 0;k--)
	    if(bits[j++])
		bytes[i] |= BIT(k);
    }	
}

strxtoc(hexstr,charstr)  /* converts 16 hex digit strings to char strings */
char *hexstr,*charstr; 
{
#define UPPER_NYBBLE	( !(val & 1) )

    unsigned char c;
    int val;
    for (val = 0; val < NYBBLES_PER_BLOCK;val++) {
	if ((hexstr[val] >= '0') && (hexstr[val] <= '9'))
	    if (UPPER_NYBBLE)
	        c = (hexstr[val] - '0') << BITS_PER_NYBBLE;
	    else
		c += hexstr[val] - '0';
	else 
	if ((hexstr[val] >= 'A') && (hexstr[val] <= 'F'))
	    if (UPPER_NYBBLE)
	        c = (hexstr[val] - 'A' +10) << BITS_PER_NYBBLE;
	    else
		c += hexstr[val] - 'A' +10;
	else
	if ((hexstr[val] >= 'a') && (hexstr[val] <= 'f'))
	    if (UPPER_NYBBLE)
	        c = (hexstr[val] - 'a' +10) << BITS_PER_NYBBLE;
	    else
		c += hexstr[val] - 'a' +10;
	else {
	    fprintf(stderr,"hex conversion error: %s - char %d\n",hexstr,val);
	    if ((val = strlen(hexstr)) != NYBBLES_PER_BLOCK)
		fprintf(stderr,"hex string length != 16\n");
	    exit(1); 
	}
	if ( UPPER_NYBBLE)	    /* 2nd nybble of each char */
	    charstr[val>>1] = 0;
	else		    
	    charstr[val>>1] = c;
    }
}

strctox(charstr,hexstr)  /* converts 8 char string to 16 hex digit string */
char *charstr,*hexstr;
{
    unsigned char c;
    int i;
    for (i = 0; i < 8; i++) {
	c = charstr[i] >> BITS_PER_NYBBLE;  /* uppper nybble */
	if ( c <= 9)
	    *hexstr++ = c + '0';
	else
	    *hexstr++ = c + '7';
	    
	c = (charstr[i] & 0xf);
	if ( c <= 9)
	    *hexstr++ = c + '0';
	else
	    *hexstr++ = c + '7';
	
    }
    *hexstr = 0;	    /* following NULL terminator */
}
