# include	<stdio.h>
# include	<stdlib.h>

/* # define	KEY001	(68150631L) */
# define	KEY001	(60150631L)
# define	INVALID	(-987)
# define	UK

static int	magic = 24;
static long	key = 8000000L;

long		debug = 1;

extern void	lookup( long i, long *outtime, long *outdur );
extern void	confirm( long index );

/* function that performs initial scrambling */
long
mixup( long x, long y )
{
	long	i, j, k, sum;
	long	a[12], b[12], out[20] ;

	y += key;	/* experiment */

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



/* first function */
long
f1( long inval )
{
	long	ndigits, hashval ;
	long	pwr ;


	ndigits = 0;
	pwr = 1;
	while(inval >= pwr) { ndigits++; pwr *= 10; }
	if(ndigits > 8) printf("\nPANIC: %ld has %ld digits\n", inval, ndigits);
	pwr = pwr / 10 ;

	hashval = inval;
again:	hashval = mixup(hashval, KEY001) % (pwr * 10) ;
	if(hashval < pwr) goto again ;

	return(hashval);
}


/*
 * The American routine shuffles the bits differently from the British.
 * Two surplus bits are stored in "S1" and "S6" for now, because we don't
 * know where they truly belong (though they *are* part of 'tval' for
 * certain - Steve). They happen to be stored in bits 9 and 10 of tval
 * for now, but may move at any time hence their unlinked names.
 *
 * I suppose we ought to get this routine to output them via a parameter
 * called 'sval', but I couldn't be bothered since it looks like they'll
 * be an offical part of 'tval' any minute now.... (Steve).
 */

void
bit_shuffle( long t8c5, long t2c1, long *tval, long *cval)
{
	long	tt, cc, b;

	tt = 0 ;
	cc = 0 ;

	/* print extra bits */
	if (b = (t8c5 >> 10) && debug)
		printf("Incoming spare bits in 't8c5': 0x%0lX\n", b);

	/* get t8 thru t3 */
	b = 0x00000001 & (t8c5 >> 9) ;
# ifdef USA
	tt += (b << 8);
# endif
# ifdef UK
	tt += (b << 10);	/* US t8 -> UK s6 (Steve) */
# endif

	b = 0x00000001 & (t8c5 >> 7) ;
# ifdef USA
	tt += (b << 7);
# endif
# ifdef UK
	tt += (b << 8);		/* US t7 -> UK t8 (Doug & Steve) */
# endif

	b = 0x00000001 & (t8c5 >> 5) ;
	tt += (b << 6);

	b = 0x00000001 & (t8c5 >> 4) ;
	tt += (b << 5);

	b = 0x00000001 & (t8c5 >> 3) ;
	tt += (b << 4);

	b = 0x00000001 & t8c5 ;
	tt += (b << 3);


	/* get c5 thru c2 */
	b = 0x00000001 & (t8c5 >> 8) ;
# ifdef USA
	cc += (b << 5);
# endif
# ifdef UK
	tt += (b << 9);		/* US c5 -> UK s1 (Doug & Steve) */
# endif

	b = 0x00000001 & (t8c5 >> 6) ;
# ifdef USA
	cc += (b << 4);
# endif
# ifdef UK
	tt += (b << 7);		/* US c4 -> UK t7 (Doug) */
# endif

	b = 0x00000001 & (t8c5 >> 2) ;
	cc += (b << 3);

	b = 0x00000001 & (t8c5 >> 1) ;
	cc += (b << 2) ;


	/* get t2 thru t0 */
	b = 0x00000001 & (t2c1 >> 4) ;
	tt += (b << 2);

	b = 0x00000001 & (t2c1 >> 2) ;
	tt += (b << 1);

	b = 0x00000001 & t2c1 ;
	tt += b;


	/* get c1 thru c0 */
	b = 0x00000001 & (t2c1 >> 3) ;
	cc += (b << 1);

	b = 0x00000001 & (t2c1 >> 1) ;
	cc += b ;

	*tval = tt;
	*cval = cc;
}



long
map_top( long day, long year, long top, long digits )
{
	long	d3, d2, d1, d0, y, poot ;
	long	n3, n2, n1, n0, f4, f3, f2, f1, f0;

	y = year % 16 ;
	d3 = top / 1000;
	d2 = (top % 1000) / 100 ;
	d1 = (top % 100) / 10 ;
	d0 = top % 10 ;

	f0 = 1;
	f1 = (y + 1) % 10;
	f2 = ( ((y+1)*(y+2)) / 2 ) % 10 ;
	f3 = ( ((y+1)*(y+2)*(y+3)) / 6 ) % 10 ;
	f4 = ( ((y+1)*(y+2)*(y+3)*(y+4)) / magic ) % 10 ;


	if(digits == 1)
	{
		n0 = ( (d0*f0) + (day*f1) ) % 10 ;
		n1 = 0;
		n2 = 0;
		n3 = 0;
	}

	if(digits == 2)
	{
		n0 = ( (d0*f0) + (d1*f1) + (day*f2) ) % 10 ;
		n1 = ( (d1*f0) + (day*f1) ) % 10 ;
		n2 = 0;
		n3 = 0;
	}

	if(digits == 3)
	{
		n0 = ( (d0*f0) + (d1*f1) + (d2*f2) + (day*f3) ) % 10 ;
		n1 = ( (d1*f0) + (d2*f1) + (day*f2) ) % 10 ;
		n2 = ( (d2*f0) + (day*f1) ) % 10 ;
		n3 = 0;
	}

	if(digits == 4)
	{
		n0 = ( (d0*f0) + (d1*f1) + (d2*f2) + (d3*f3) + (day*f4) ) % 10 ;
		n1 = ( (d1*f0) + (d2*f1) + (d3*f2) + (day*f3) ) % 10 ;
		n2 = ( (d2*f0) + (d3*f1) + (day*f2) ) % 10 ;
		n3 = ( (d3*f0) + (day*f1) ) % 10 ;
	}


	poot =  (1000*n3) + (100*n2) + (10*n1) + n0 ;

	return( poot );
}





void
offset( long day, long year, long top, long *ofout, long *topout )
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
	/*
		if (debug)
			printf("Burp %ld = %ld\n", i, burp);
	 */

		off += burp ;
		}

	t = map_top(day, year, t, digits) ;
/*
	if (debug)
		printf("Topout = %ld\n", t);
 */

	if(t < pwr) goto again ;


	*ofout = (off % 32) ;
	*topout = (t) ;
}

long
end_time( long start, long dur )
{
	int	min, hr;

	min = (start % 100) + (dur % 60);
	hr = (start / 100) + (dur / 60);

	while (min >= 60) {
		min -= 60;
		hr++;
	}

	start = hr*100 + min;

	if (start >= 2400)
		start -= 2400;

	return start;
}



static char *mon[] = {  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };



long
decode( long month_today, long date_today, long year_today, long newspaper )
{
	long	s1_out, bot3, top5, quo, rem ;
	long	s5_out, mtout, ofout, tval, cval ;
	long	day_out, channel_out;
	long	starttime_out, duration_out ;

	year_today = year_today % 100 ;
	debug = 1;

	if( (month_today>12) || (month_today<1) || (date_today<1) || (date_today>31) ) {
		fputs("Invalid date\n", stdout);
	}

	if((newspaper < 1) || (newspaper > 9999999)) {
		fputs("Sorry, I can't process that code value\n", stdout);
		fputs("	I can only do codes with 7 digits or fewer\n", stdout);
	}

	ofout = INVALID ;
	mtout = INVALID ;

	s1_out = f1(newspaper); 
	bot3 = s1_out % 1000;
	top5 = s1_out / 1000;
	quo = (bot3 - 1) / 32 ;
	rem = (bot3 - 1) % 32 ;
	day_out = quo + 1;

	/* spot instances of next month's/year's codes - S.Hosgood, 8.Feb.93 */
	if (day_out < date_today)
		month_today++;
	if (month_today > 12) {
		month_today = 1;
		year_today = (year_today+1) % 100;
	}

	if(newspaper >= 1000)
		offset(day_out, year_today, top5, (&ofout), (&mtout)) ;
	else {
		mtout = 0;
		ofout = 0;
	}

	s5_out = (rem + (day_out*(month_today+1)) + ofout) % 32 ;


	bit_shuffle(mtout, s5_out, &tval, &cval);
	channel_out = cval + 1;
	lookup(tval, (&starttime_out), (&duration_out));

	printf("\t%s  %2ld  %2ld\n\tChannel= %2ld\n", 
		mon[month_today-1], day_out, year_today, channel_out);
	printf("\tStart_time= %04ld  Length= %3ld\n",
		starttime_out, duration_out);
	printf("\tEnd_time= %04ld\n",
		end_time(starttime_out, duration_out));

	return tval;
}


int
channel( long month_today, long date_today, long year_today, long newspaper )
{
	long	s1_out, bot3, top5, quo, rem ;
	long	s5_out, mtout, ofout, tval, cval ;
	long	day_out, channel_out;
	long	starttime_out, duration_out ;

	year_today = year_today % 100 ;
	debug = 0;

	if( (month_today>12) || (month_today<1) || (date_today<1) || (date_today>31) ) {
		fputs("Invalid date\n", stdout);
	}

	if((newspaper < 1) || (newspaper > 9999999)) {
		fputs("Sorry, I can't process that code value\n", stdout);
		fputs("	I can only do codes with 7 digits or fewer\n", stdout);
	}

	ofout = INVALID ;
	mtout = INVALID ;

	s1_out = f1(newspaper); 
	bot3 = s1_out % 1000;
	top5 = s1_out / 1000;
	quo = (bot3 - 1) / 32 ;
	rem = (bot3 - 1) % 32 ;
	day_out = quo + 1;

	/* spot instances of next month's/year's codes - S.Hosgood, 8.Feb.93 */
	if (day_out < date_today)
		month_today++;
	if (month_today > 12) {
		month_today = 1;
		year_today = (year_today+1) % 100;
	}

	if(newspaper >= 1000)
		offset(day_out, year_today, top5, (&ofout), (&mtout)) ;
	else {
		mtout = 0;
		ofout = 0;
	}

	s5_out = (rem + (day_out*(month_today+1)) + ofout) % 32 ;


	bit_shuffle(mtout, s5_out, &tval, &cval);
	return cval + 1;
}




main(int argc, char *argv[])
{
	char	buffer[20];
	int	loop;
	long	month_today, date_today, year_today, newspaper, n;

	if (argc != 4 && argc != 5) {
		printf("Usage: %s  todays_month  todays_day  todays_year  [ Code_value_in_newspaper ]\n", argv[0]);
		exit(-1);
	}

	month_today = atol(argv[1]);
	date_today = atol(argv[2]);
	year_today = atol(argv[3]);

	if (argc == 5) {
		newspaper = atol(argv[4]);
		printf("Code = %ld\n", newspaper);
		decode(month_today, date_today, year_today, newspaper);
	}
	else {
		loop = 1;

		while (loop) {
			fputs("Code? ", stdout);
			fgets(buffer, sizeof buffer, stdin);

			switch (buffer[0]) {
			case 'c':
			case 'C':
				/* match this channel-no */
				/* experiment to find that elusive factor! */
				n = atol(buffer+1);

				for (magic = 1; magic < 50; magic++) {
					for (key = 0; key <= 9000000L; key += 1000000L) {
						if (channel(month_today, date_today, year_today, newspaper) == n) {
							printf("Magic = %d, ", magic);
							printf("Key = %ld\n", key);
						}
					}
				}

				break;

			case 'm':
			case 'M':
				/* set magic number */
				magic = atol(buffer+1);
				decode(month_today, date_today, year_today, newspaper);
				break;

			case 'k':
			case 'K':
				/* set key digit #7 */
				key = atol(buffer+1) * 1000000L;
				decode(month_today, date_today, year_today, newspaper);
				break;

			case 'q':
			case 'Q':
				loop = 0;
				break;

			default:
				newspaper = atol(buffer);
				n = decode(month_today, date_today, year_today, newspaper);

				/* if it was a guess, allow him to confirm */
				confirm(n);
			}

			putchar('\n');
		}
	}

	/* maybe write table (if it changed) */
	dump();
	exit(0);
}
