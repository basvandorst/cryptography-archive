/* Permute and encrypt           Feb 17 1990        Public Domain        */

/* Syntax:   garble -Kxxxxxxxxxxxxxxxx [-encrypt | -decrypt] <in >out    */

/*   xxxxxxxxxxxxxxxx  Encryption / decryption key.  1-16 chars long     */
/*   -encrypt          Optional flag that causes encryption [default]    */
/*   -decrypt          Optional flag that causes decryption              */


/* No warranty is expressed or implied; use at your own risk             */



/*   METHOD                                                              */
/*   1.  The user key is split into two 32-bit words, "ekey" and "pkey". */
/*   2.  The time of day and other system-dependent data are combined    */
/*       to produce two perturbation values ("salt1" and "salt2")        */
/*       which modify the keys.                                          */
/*   3.  Salts are written to the output, for eventual use in decryption */
/*   4.  Salt values are added (mod 2^32) to ekey and pkey.              */
/*   5.  The input cleartext is divided into blocks of 1024 bytes.       */
/*       Each block is transposed (permuted) under control of pkey.      */
/*   6.  The permuted cleartext is encrypted.  It is XOR'd with a        */
/*       sequence produced by a pseudorandom-sequence generator from     */
/*       Knuth (the 'additive' generator, algorithm A, section 3.2.2),   */
/*       under control of ekey.                                          */
/*   7.  Permuted, encrypted blocks are written to the output.           */


/*    "FEATURES"                                                         */
/*       1.  The salt makes it (a bit) harder to use a known-plaintext   */
/*           cryptanalysis, since encrypting the same cleartext file     */
/*           twice gives two different cyphertexts.                      */
/*       2.  Input "cleartext" that has been run through UNIX "compress" */
/*           doesn't compromise security; the known preamble or encoding */
/*           table is mixed into the data by the permutation step.       */
/*       3.  Relatively fast; 8.6 seconds to encrypt or decrypt a        */
/*           1,000,000 byte file on a SPARCstation.   (compiled with -O) */


/*      BUGS                                                             */
/*       1.  Assumes it's running on a 32-bit machine (or in 32b         */
/*           compatibility mode).  Lots of arithmetic operations assume  */
/*           they are performed "mod 2^32".  Works fine on VAX, 386,     */
/*           68k, 486, SPARC, RS-6000, MIPS, S/370, HPPA, Clipper, etc.  */
/*       2.  The code that hides the key from other, snoopy users typing */
/*           "ps" or "top", only works on some O/S flavors.              */
	



#include <stdio.h>
#include <string.h>

#define		BLOCKSIZE	(1024)        /* permute in blocks of 1k */
#define		BIGINT		(2147483647)

#define		ENCRYPT		(77)
#define		DECRYPT		(99)

#define		AX	(60554741)
#define		CX	(937642419)
#define		AY	(16733205)
#define		CY	(907633407)
#define		AZ	(719685677)
#define		CZ	(907620093)





/* Sequencer function that creates the permutations */
int p_seq(pkey)
	int	pkey ;
{
	static int	y[63] ;
	static int	j = 24 ;
	static int	k = 55 ;
	static int	virgin = 1 ;
	int		i, n ;

	/* fill the y[] array using the user-supplied key */
	if( virgin == 1 ) {
		virgin = 0;
		n = 271828183 ;
		for(i=0; i<1000; i++) { n = CY + (AY * n); }
		for(i=0; i<63; i++) {
			n = CY + (AY * n) ;
			y[i] = n ;
		}
		for(i=0; i<1000; i++) {
			y[k] = y[k] + y[j] ;
			if(y[k] < 0) y[k] = 0 - y[k] ;
			k--; j--;
			if(j <= 0) j = 55;
			else { if(k <= 0) k = 55; }
		}
		/* user key is added in iteratively */
		n = pkey ;
		for(i=0; i<800; i++) { n = CY + (AY * n); }
		for(i=0; i<1000; i++) {
			n = CY + (AY * n) ;
			y[k] = y[k] + y[j] + n ;
			if(y[k] < 0) y[k] = 0 - y[k] ;
			k--; j--;
			if(j <= 0) j = 55;
			else { if(k <= 0) k = 55; }
		}
	}

	/* Normal operation; return the next sequencer integer */
	y[k] = y[k] + y[j] ;
	if(y[k] < 0) y[k] = 0 - y[k] ;
	k--; j--;
	if(j <= 0) j = 55;
	else { if(k <= 0) k = 55; }
	n = y[k] ;
	return(n) ;
}






/* Sequencer function that creates the encryption */
int c_seq(ekey)
	int	ekey ;
{
	static int	y[63] ;
	static int	j = 24 ;
	static int	k = 55 ;
	static int	virgin = 1 ;
	int		i, n ;

	/* fill the y[] array using the user-supplied key */
	if( virgin == 1 ) {
		virgin = 0;
		n = 6457513 ;
		for(i=0; i<1000; i++) { n = CX + (AX * n); }
		for(i=0; i<63; i++) {
			n = CX + (AX * n);
			y[i] = n ;
		}
		for(i=0; i<44444; i++) {
			y[k] = y[k] + y[j] ;
			if(y[k] < 0) y[k] = 0 - y[k] ;
			k--; j--;
			if(j <= 0) j = 55;
			else { if(k <= 0) k = 55; }
		}
		/* user key is added in iteratively */
		n = ekey ;
		for(i=0; i<33333; i++) { n = CX + (AX * n); }
		for(i=0; i<1000; i++) {
			n = CX + (AX * n);
			y[k] = y[k] + y[j] + n ;
			if(y[k] < 0) y[k] = 0 - y[k] ;
			k--; j--;
			if(j <= 0) j = 55;
			else { if(k <= 0) k = 55; }
		}
		/* the y[] array is set, so warm up the seq gen */
		for(i=0; i<5500; i++) {
			y[k] = y[k] + y[j] ;
			if(y[k] < 0) y[k] = 0 - y[k] ;
			k--; j--;
			if(j <= 0) j = 55;
			else { if(k <= 0) k = 55; }
		}
		/* set the pointers to known spots in the array */
		j = 24 ;
		k = 55 ;
	}

	/* Normal operation; return the next sequencer integer */
	y[k] = y[k] + y[j] ;
	if(y[k] < 0) y[k] = 0 - y[k] ;
	k--; j--;
	if(j <= 0) j = 55;
	else { if(k <= 0) k = 55; }
	n = y[k] ;
	return(n) ;
}






/* Transpose the order of bytes in a block */
int permute(byte_array, n, pkey, enc_flag)
	int	byte_array[BLOCKSIZE] ;
	int	n, pkey, enc_flag ;
{
	int	i, j, k, temp ;
	int	rearrange[BLOCKSIZE] ;
	double	factor, x ;

	factor = (double) BIGINT ;
	factor = 1.0 / factor ;

	if(enc_flag == ENCRYPT) /* permute */
	{
		i = n-1 ;
		while(i > 0)
		{
			j = p_seq(pkey);
			x = ( (double) j ) * factor ;
			k = (int) (x * i) ;
			temp = byte_array[k] ;
			byte_array[k] = byte_array[i] ;
			byte_array[i] = temp ;
			i--;
		}
	}
	else /* de-permute */
	{
		for(i=(n-1); i>0; i--)
		{
			j = p_seq(pkey);
			x = ( (double) j ) * factor ;
			k = (int) (x * i) ;
			rearrange[i] = k ;
		}
		for(i=1; i<n; i++)
		{
			k = rearrange[i] ;
			temp = byte_array[k] ;
			byte_array[k] = byte_array[i] ;
			byte_array[i] = temp ;
		}
	}
}






/* Encrypt the bytes in a block */
int encr(byte_array, n, ekey)
	int	byte_array[BLOCKSIZE] ;
	int	n, ekey ;
{
	int	i, j ;

	i = 0 ;
	while(i < n)
	{
		j = c_seq(ekey);
		j = (j>>3) & 0x000000ff ;
		byte_array[i] ^= j ;
		i++ ;
	}
}






/* read a block from standard input */
int getblock(byte_array, count)
	int	byte_array[BLOCKSIZE] ;
	int	*count ;
{
	int	i, k ;

	k = 0 ;
	while( k < BLOCKSIZE ) {
		if( (i=getchar()) == EOF ) break ;
		byte_array[k] = i;
		k++;
	}

	*count = k ;
	if(i == EOF) return(i);
	return(k);
}






/* write a block to standard output */
int putblock(byte_array, count)
	int	byte_array[BLOCKSIZE] ;
	int	count ;
{
	int	i ;
	char	c ;
	for(i=0; i<count; i++) { c = byte_array[i]; putchar(c) ; }
	return(EOF);
}





/* query the runtime environment to get a pseudorandom value */
int	getsalt(s1, s2)
	int	*s1, *s2 ;
{
	int	i, j, k, sum1, sum2 ;
	FILE	*fopen(), *ugly ;

	/* put some arcane, time-varying data into a temp file */
	system("date             > /tmp/mjurp37Z");
	system("whoami          >> /tmp/mjurp37Z");
	system("wc /etc/termcap >> /tmp/mjurp37Z");
	system("wc /etc/passwd  >> /tmp/mjurp37Z");
	system("ls /tmp         >> /tmp/mjurp37Z");

	ugly = fopen("/tmp/mjurp37Z", "r");

	sum1 = sum2 = 0;
	while( (i=getc(ugly)) != EOF )
	{
		k = i & 0x0000007f ;  /* mask off the MSB */
		k *= 13 ;
		sum1 = sum1 + sum1 + k ;
		if(sum1 < 0) sum1 = 0 - sum1 ;

		j = getc(ugly) ;
		k = j & 0x0000007f ;
		k *= 17 ;
		sum2 = sum2 + sum2 + k ;
		if(sum2 < 0) sum2 = 0 - sum2 ;
	}

	*s1 = sum1 ;  *s2 = sum2 ;

	fclose(ugly);
	system("rm /tmp/mjurp37Z");
}






main(argc, argv)
	char **argv;
	int argc;
{

	int	i, j, k, direction, s[20] ;
	int	ekey, pkey, salt1, salt2 ;
	char	word1[300], word2[300], c ;

	int	array[BLOCKSIZE] ;


	if(argc < 2) {
		fprintf(stderr, "NEED COMMAND LINE ARG FOR KEY!\n");
		fprintf(stderr, "Usage: %s -Kxxxxxxxxxxxxxxxx [-encrypt | -decrypt] <in >out\n", (argv[0]));
			exit(1); }

	strcpy(word1, argv[1]);
	k = strlen(word1);
	if( (k<3) || (k>18) || (word1[0] != '-') || (word1[1] != 'K') )
		{ fprintf(stderr, "NEED ARG -Kxxxxxxxxxxxxxxxx FOR KEY!\n");
		  fprintf(stderr, "  not >%s<  or >%s<  \n", (argv[1]), word1);
		  fprintf(stderr, "Usage: %s -Kxxxxxxxxxxxxxxxx [-encrypt | -decrypt] <in >out\n", (argv[0]));
			exit(1); }

	/* overwrite the argument string to foil "ps" */
	strcpy(argv[1], "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");

	/* get rid of leading "-K" and extend on the left with blanks */
	for(j=2; j<strlen(word1); j++) word1[j-2] = word1[j] ;
	sprintf(word2, "%16s", word1);

	/* get permutation key from most sig. word of user key */
	pkey = 0;
	i = 0 ;
	for(j=0; j<8; j++)
	{
		if((word2[j] >= '0') && (word2[j] <= '9')) k = word2[j] - '0';
		else k = 10 + word2[j] - 'a' ;
		k = k & 0x000000ff ;
		pkey += (k << i) ;
		i += 4;
	}
	/* get encryption key from least sig. word of user key */
	ekey = 0;
	i = 0 ;
	for(j=8; j<16; j++)
	{
		if((word2[j] >= '0') && (word2[j] <= '9')) k = word2[j] - '0';
		else k = 10 + word2[j] - 'a' ;
		k = k & 0x000000ff ;
		ekey += (k << i) ;
		i += 4;
	}


	direction = ENCRYPT ;
	if(argc > 2) /* figure out whether to encrypt or decrypt */
	{
		strcpy(word1, argv[2]);
		if( 0 == strcmp(word1, "-encrypt") ) direction = ENCRYPT ;
		else if( 0 == strcmp(word1, "-decrypt") ) direction = DECRYPT ;
		else { fprintf(stderr, "\nflag %s not understood\n", word1);
			exit(-1);
		     }
	}



	/* alter the user keys by the pseudorandom salt */

	if(direction == ENCRYPT)
	{  getsalt(&salt1, &salt2) ;
           j = 24;
	   for(i=0; i<4; i++) { s[i] = (salt1 >> j) & 0x000000ff; j -= 8; }
           j = 24;
	   for(i=4; i<8; i++) { s[i] = (salt2 >> j) & 0x000000ff; j -= 8; }
	   for(i=0; i<8; i++) { c = s[i]; putchar(c); }
	}
	else  /* for decryption, 1st 8 cyphertext bytes are the salt */
	{  for(i=0; i<8; i++)   { s[i] = getchar(); }
	   salt1 = 0;  j = 24 ;
	   for(i=0; i<4; i++)   { salt1 += (s[i] << j);  j -= 8; }
	   salt2 = 0;  j = 24 ;
	   for(i=4; i<8; i++)   { salt2 += (s[i] << j);  j -= 8; }
	}
	ekey += salt1 ;
	pkey += salt2 ;





	/* now go do the work */
	i = 0 ;
	while(i != EOF)
	{
		i = getblock(array,&j) ;
		if(j == 0) continue ;

		if(direction == ENCRYPT) {
			permute(array, j, pkey, direction) ;
			encr(array, j, ekey) ;
		}
		else {
			encr(array, j, ekey) ;
			permute(array, j, pkey, direction) ;
		}
		putblock(array,j) ;
	}


} /* end of main() */
