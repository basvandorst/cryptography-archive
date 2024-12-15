# include	<stdio.h>
# include	<stdlib.h>

# define	KEY001		(9371L)
# define	INVALID		(-987)
# define	UK

long	debug = 0;

extern int	scanfor( long start, long duration );

/* function that performs initial scrambling */
long
mixup( long x, long y )
{
	long	i, j, k, sum;
	long	a[12], b[12], out[20] ;

	/* get the digits of x into a[] */
	j = x ;
	for(i=0; i<9; i++)
	{
		k = j % 10;
		a[i] = k;
		j = (j - k) / 10 ;
	}

	/* get the digits of y into b[] */
	j = y ;
	for(i=0; i<9; i++)
	{
		k = j % 10;
		b[i] = k;
		j = (j - k) / 10 ;
		out[i] = 0;
	}


	for(i=0; i<=8; i++)
	{
		for(j=0; j<=8; j++)
		{
			out[i+j] += (b[j] * a[i]) ;
		}
	}

	j = 1;
	sum = 0;
	for(i=0; i<=8; i++)
	{
		sum += j * (out[i] % 10);
		j = j * 10 ;
	}
	return( sum ) ;
}

/* how many digits in a number? */
int
nd( long inval, long *p )
{
	int	ndigits;
	long	pwr;

	ndigits = 0;
	pwr = 1;
	while (inval >= pwr) {
		ndigits++;
		pwr *= 10;
	}

	*p = pwr;
	return ndigits;
}

/* first function */
long
f1( long inval )
{
	long	hashval, pwr;
	int	ndigits;

	if ((ndigits = nd(inval, &pwr)) > 8)
		printf("\nPANIC: %ld has %ld digits\n", inval, ndigits);

	pwr = pwr / 10 ;

	hashval = inval;
again:	hashval = mixup(hashval, KEY001) % (pwr * 10) ;
	if(hashval < pwr) goto again ;

	return(hashval);
}



void
interleave( long tblidx, long cval, long *t8c5, long *t2c1 )
{
	long	small, big, b;

	small = 0;
	big = 0;

	/* build t2c1 */
	b = 0x00000001 & (tblidx >> 2) ;
	small += (b << 4);

	b = 0x00000001 & (tblidx >> 1) ;
	small += (b << 2);

	b = 0x00000001 & tblidx ;
	small += b;

	b = 0x00000001 & (cval >> 1) ;
	small += (b << 3);

	b = 0x00000001 & cval ;
	small += (b << 1);


	/* build t8c5 */
# ifdef US
	b = 0x00000001 & (tblidx >> 8) ;
# endif
# ifdef UK
	b = 0x00000001 & (tblidx >> 10) ;
# endif
	big += (b << 9);

# ifdef US
	b = 0x00000001 & (tblidx >> 7) ;
# endif
# ifdef UK
	b = 0x00000001 & (tblidx >> 8) ;
# endif
	big += (b << 7);

	b = 0x00000001 & (tblidx >> 6) ;
	big += (b << 5);

	b = 0x00000001 & (tblidx >> 5) ;
	big += (b << 4);

	b = 0x00000001 & (tblidx >> 4) ;
	big += (b << 3);

	b = 0x00000001 & (tblidx >> 3) ;
	big += b;

# ifdef US
	b = 0x00000001 & (cval >> 5) ;
# endif
# ifdef UK
	b = 0x00000001 & (tblidx >> 9) ;
# endif
	big += (b << 8);

# ifdef US
	b = 0x00000001 & (cval >> 4) ;
# endif
# ifdef UK
	b = 0x00000001 & (tblidx >> 7) ;
# endif
	big += (b << 6);

	b = 0x00000001 & (cval >> 3) ;
	big += (b << 2);

	b = 0x00000001 & (cval >> 2) ;
	big += (b << 1);


	*t8c5 = big;
	*t2c1 = small;
}





long
map_top( long day, long year, long top, long digits )
{
	long	d2, d1, d0, y, poot ;
	long	n2, n1, n0, f3, f2, f1, f0;

	y = year % 16 ;
	d2 = top / 100 ;
	d1 = (top % 100) / 10 ;
	d0 = top % 10 ;

	f0 = 1;
	f1 = (y + 1) % 10;
	f2 = ( ((y+1)*(y+2)) / 2 ) % 10 ;
	f3 = ( ((y+1)*(y+2)*(y+3)) / 6 ) % 10 ;


	if(digits == 1)
	{
		n0 = ( (d0*f0) + (day*f1) ) % 10 ;
		n1 = 0;
		n2 = 0;
	}

	if(digits == 2)
	{
		n0 = ( (d0*f0) + (d1*f1) + (day*f2) ) % 10 ;
		n1 = ( (d1*f0) + (day*f1) ) % 10 ;
		n2 = 0;
	}

	if(digits == 3)
	{
		n0 = ( (d0*f0) + (d1*f1) + (d2*f2) + (day*f3) ) % 10 ;
		n1 = ( (d1*f0) + (d2*f1) + (day*f2) ) % 10 ;
		n2 = ( (d2*f0) + (day*f1) ) % 10 ;
	}


	poot =  (100*n2) + (10*n1) + n0 ;

	return( poot );
}





void
offset( long day, long year, long top, long *ofout, long *topout)
{
	long	i, t, tx, off, digits, burp;
	long	pwr ;


	pwr = 1 ; digits = 0;
	while(top >= pwr) { digits++; pwr *= 10; }
	pwr = pwr / 10 ;

	t = tx = top ;
	off = 0;
	while(tx > 0) { off += (tx % 10);   tx /= 10; }

again:
	for(i=0; i<=(year%16); i++)
		{
		burp = ( (map_top(day, i, t, digits)) % 10 ) ;
		off += burp ;
		}

	t = map_top(day, year, t, digits) ;
	if(t < pwr) goto again ;


	*ofout = (off % 32) ;
	*topout = (t) ;
}






main( int argc, char *argv[] )
{
	long	j, doneflag, trailers, tblidx, limit;
	long	s1_out, bot3, top5, quo, rem, newspaper ;
	long	s4_out, s5_out, ofout, cval ;
	long	month, day, year, channel, starttime, duration ;



	if(argc == 7) goto clean;

upchuck:
	printf("Usage: %s  month  day  year  channel  starting_time  length_in_minutes\n",
		argv[0]);
	exit(-1) ;



clean:

	month = atol(argv[1]);
	day = atol(argv[2]);
	year = atol(argv[3]);
	channel = atol(argv[4]);
	starttime = atol(argv[5]);
	duration = atol(argv[6]);

	year = year % 100 ;

	if( (month>12) || (month<1) || (day<1) || (day>31) )
	{ printf("Invalid date\n"); goto upchuck; }

# ifdef US
	if( (channel<1) || (channel>48) ) {
# endif
# ifdef UK
	if( (channel<1) || (channel>16) ) {
# endif
		printf("Invalid channel number\n");
		goto upchuck;
	}

# ifdef US
	trailers = starttime % 100;
	if((trailers != 0) && (trailers != 30))
	{ printf("Invalid start time\n"); goto upchuck; }

	if((starttime < 0) || (starttime > 2330)) {
# endif
# ifdef UK
	if((starttime < 0) || (starttime > 2355)) {
# endif
		printf("Sorry, I cant process that starting time\n");
		goto upchuck;
	}

# ifdef US
	trailers = duration % 30;
	if((trailers!=0) || (duration<30) || (duration>300)) {
# endif
# ifdef UK
	if (duration < 0) {
# endif
		printf("Sorry, I can't process that program length\n");
		goto upchuck;
	}


	/* get the  t  bits and the  c  bits */
	cval = channel - 1 ;
	if ((tblidx = scanfor(starttime, duration)) < 0) {
		printf("Sorry, no index known.\n");
		goto upchuck;
	}
	printf("Index = %ld\n", tblidx);

	/* from them infer what must have been step 4 & step 5 results */
	interleave(tblidx, cval, (&s4_out), (&s5_out)) ;

	/* find the smallest unmapped_top giving correct mapped_top */
	nd(s4_out, &limit);
	top5 = limit / 10;

	printf("S4=%ld, top5 = %ld, limit = %d\n", s4_out, top5, limit);
	doneflag = 0;

	/* if the mapped_top is zero then top and offset are zero */
	if(s4_out == 0) { top5 = 0; ofout = 0; doneflag = 1; }

	while(doneflag == 0)
	{
		offset(day, year, top5, (&ofout), (&j)) ;
		putchar((top5 & 0x01)? '+': 'X');
		putchar('\r');

		if (j == s4_out) {
			printf("\nFound it: %ld\n", top5);
			break;
		}

		if (++top5 >= limit) {
			printf("Can't seem to find an encoding - sorry\n");
			break;
		}
	}

	/* have two of the three inputs to step 5; determine the rem */
	for(rem=0; rem<32; rem++)
	{
		j = (rem + (day*(month+1)) + ofout) % 32 ;
		if(j == s5_out) break ;
	}
	quo = (day - 1);


	/* assemble the output of step 1 */
	bot3 = 1 + rem + (32 * quo) ;
	s1_out = bot3 + (1000 * top5) ;
	printf("s1=%ld\n", s1_out);

	/* invert the mixing */
	newspaper = f1(s1_out);

	printf(" %2ld  %2ld  %2ld %3ld  %4ld  %3ld    VCRPLUS_CODE= %8ld\n",
		month, day, year, channel, starttime, duration, newspaper);

}
