/*******************************************************************************
*									       *
*	Copyright (c) Martin Nicolay,  22. Nov. 1988			       *
*									       *
*	Wenn diese (oder sinngemaess uebersetzte) Copyright-Angabe enthalten   *
*	bleibt, darf diese Source fuer jeden nichtkomerziellen Zweck weiter    *
*	verwendet werden.						       *
*									       *
*	martin@trillian.megalon.de					       *
*									       *
*******************************************************************************/

#include	<stdio.h>
#include	<string.h>
#ifndef AMIGA
#include	<memory.h>
#endif  AMIGA

#include	"arith.h"
#include	"nio.h"

#define	ENCODE	"rsaencode"
#define	DECODE	"rsadecode"

char *prog;

int	clear_siz;			/* clear-text blocksize		*/
int	enc_siz;			/* encoded blocksize		*/
					/* clear_siz < enc_siz		*/

do_crypt( s, d, len, e )
char *s;
char *d;
NUMBER *e;
{
	static char hex[] = "0123456789ABCDEF";
	NUMBER n;
	char buf[ STRLEN + 1 ];
	char *pb,*ph;
	int i,c;
	
	ph = buf + STRLEN;
	ph[1] = '\0';
	
	for (i=len; i; i--) {
		c = *s++;
		*ph-- = hex[ (c >> 4) & 0xF ];
		*ph-- = hex[ c & 0xF ];
	}
	ph++;
	
	num_sget( &n, ph );
	
	m_exp( &n, e, &n );
	
	num_sput( &n, buf, STRLEN +1 );
	
	ph = buf + (i=strlen(buf)) -1;
	
	for (; len; len--) {
		if (i-- > 0) {
			c = (strchr( hex, *ph ) - hex) << 4;
			ph--;
		}
		else
			c=0;
		if (i-- > 0) {
			c |= strchr( hex, *ph ) - hex;
			ph--;
		}

		*d++ = c;
	}
}
	
#ifdef STANDALONE 	
int get_clear( p )
char *p;
{
	int n;
	
	n = fread( p, 1, clear_siz, stdin );
	
	if (n <= 0)
		return(0);
	
	
	return(1);
}

int get_enc( p )
char *p;
{
	int n;

	n = fread( p, 1, enc_siz, stdin );
	
	if (n != enc_siz)
		return(0);
	
	return(1);
}

int put_clear( p )
char *p;
{
	int n;
	
	n = fwrite( p, 1, clear_siz, stdout );
	
	if (n != clear_siz)
		return(0);

	return(1);
}

int put_enc( p )
char *p;
{
	int n;

	n = fwrite( p, 1, enc_siz, stdout );
	
	if (n != enc_siz)
		return(0);
	
	return(1);
}


main( argc, argv )
char **argv;
{
	char buf[ STRLEN*2 ];
	NUMBER n,e;
	FILE *keys;
	int (*inp)() = 0;
	int (*out)() = 0;
	
	if ( ! (prog=strrchr( argv[0], '/' )) )
		prog=argv[0];
	
	
	if ( ! strcmp( prog, DECODE ) ) {
		inp = get_enc;
		out = put_clear;
	}
	if ( ! strcmp( prog, ENCODE ) ) {
		inp = get_clear;
		out = put_enc;
	}
	
	if (! inp) {
		fprintf(stderr,"%s: don't know who i am (%s or %s)\n",prog,ENCODE,DECODE);
		exit(1);
	}
	
	if (argc != 2) {
		fprintf(stderr,"usage: %s keyfile\n",prog);
		exit(1);
	}
	if ( !( keys= fopen(argv[1],"r")) ) {
		perror(argv[1]);
		exit(1);
	}
	
	num_fget( &n, keys ); getc(keys);
	num_fget( &e, keys );
	if (a_cmp(&n,&e) <= 0 || e.n_len == 0 || getc(keys) != EOF) {
		fprintf(stderr,"%s: corrupt keyfile\n",prog);
		fprintf(stderr,"keyfile format:\n");
		fprintf(stderr,"\t<n in hex>\n");
		fprintf(stderr,"\t<delimiter> (1 char)\n");
		fprintf(stderr,"\t<e/d in hex>\n");
		fprintf(stderr,"white spaces are ignored\n");
		exit(1);
	}
	
	enc_siz = ( n_bitlen( &n ) + 7 ) / 8;
	clear_siz = enc_siz -1;
	
	m_init( &n, NUM0P );
	
	while ( (*inp)( buf ) ) {
		do_crypt( buf, buf, enc_siz, &e );
		if (! (*out)( buf ) ) {
			perror("output");
			exit(1);
		}
	}
	
	exit(0);
}

#else STANDALONE

do_rsa(keyfile,buffer,buflen,maxlen )
int buflen;
char *keyfile,*buffer;
{
	NUMBER n,e;
	FILE *keys;
	
	if ( !( keys= fopen(keyfile,"r")) ) {
		perror(keyfile);
		return(-1);
	}
	
	num_fget( &n, keys ); getc(keys);
	num_fget( &e, keys );
	if (a_cmp(&n,&e) <= 0 || e.n_len == 0 || getc(keys) != EOF) {
		fprintf(stderr,"%s: corrupt keyfile\n",prog);
		fprintf(stderr,"keyfile format:\n");
		fprintf(stderr,"\t<n in hex>\n");
		fprintf(stderr,"\t<delimiter> (1 char)\n");
		fprintf(stderr,"\t<e/d in hex>\n");
		fprintf(stderr,"white spaces are ignored\n");
		return(-11);
	}
	
	enc_siz = ( n_bitlen( &n ) + 7 ) / 8;
	clear_siz = enc_siz -1;
	
	m_init( &n, NUM0P );
	
        if(buflen > enc_siz) {
          fprintf(stderr,"Public Key must be larger than data!\n");
          return(-1);
        } 
        if(maxlen < enc_siz) {
          fprintf(stderr,"Public key is too big for buffer.\n");
          return(-1);
        }
	memset( buffer+buflen, 0, enc_siz - buflen ); /* pad out block */
	do_crypt( buffer, buffer, enc_siz, &e );
        return(enc_siz);
	
}

#endif   STANDALONE

