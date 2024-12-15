# include	<stdio.h>
# include	<stdlib.h>

# define	INVALID	(-987)
# define	UK

extern long	debug;

# ifdef US
# define	LIMIT	480
# endif
# ifdef UK
# define	LIMIT	2202	/* 2048 plus 154 of the 4hr-and-longer codes */
# endif

static int		start[LIMIT];
static int		leng[LIMIT];
static unsigned char	flags[LIMIT];

# define	GUESS		0x01
# define	SUSPECTDUR	0x02
# define	SUSPECT		0x04
# define	CLASH		0x08
# define	CHANGED		0x80

# define	F_BAK		"lookup.bak"
# define	F_TBL		"lookup.tbl"
# define	F_TMP		"lookup.tmp"

static	int	initializer = 0;
static	int	change = 0;

int		scanfor( long start, long dur );
long		tdiff( long a, long b );

void
settables( int start[], int leng[] )
{
	int	j, s;
# ifdef UK
	char	buffer[64], *cp;
# endif
	FILE	*extras;


/*	for(j=0; j<512; j++) {		 BUG */
	for(j=0; j<LIMIT; j++) {
		start[j] = leng[j] = INVALID;
		flags[j] = 0;
	}

# ifdef US
# include	"us_tbl.c"
# endif
# ifdef UK
/* # include	"uk_tbl.c" */

	if (extras = fopen(F_TBL, "r")) {
		while (cp = fgets(buffer, sizeof buffer, extras)) {
			if (*cp == '#' || *cp == '\n')
				/* comment line */
				continue;
	
			if ((j = ( int ) strtol(cp, &cp, 0)) < LIMIT) {
				start[j] = ( int ) strtol(cp, &cp, 0);
				leng[j] = ( int ) strtol(cp, &cp, 0);

				/* scan for flags */
				while (*cp && *cp != '\n') {
					switch (*cp) {
					case 'g':
						flags[j] |= GUESS;
						break;

					case 'D':
						flags[j] |= SUSPECTDUR;
						break;

					case 'S':
						flags[j] |= SUSPECT;
						break;

					case 'c':
						flags[j] |= CLASH;
					}

					cp++;
				}
			}
			else
				printf("Illegal entry %d in lookup.tbl\n", j);
		}

		fclose(extras);
	}
	else {
		fputs("Couldn't open lookup.tbl\n", stderr);
		exit(-1);
	}
# endif
	return;
}

void
confirm( long index )
{
	char	buffer[20];

	if (flags[index] & GUESS) {
		fputs("\nConfirm this? [y/n] ", stdout);
		fgets(buffer, sizeof buffer, stdin);

		if (buffer[0] == 'y' || buffer[0] == 'Y') {
			flags[index] = CHANGED;
			change = 1;
		}
	}

	return;
}



/* insert movie (and other) structured tables */
int tlist[] = { 5, 10, 20, 25,
		0, 5, 10, 15, 20, 25,	/* -5 */
		0, 5, 10, 15, 20, 25,	/* -10 */
		0, 5, 10, 15, 20, 25,	/* -15 */
		0, 5, 10, 15, 20, 25,	/* -20 */
		0, 5, 10, 15, 20, 25	/* -25 */
};

int dlist[] = { 0, 0, 0, 0,
		-5, -5, -5, -5, -5, -5,
		-10, -10, -10, -10, -10, -10,
		-15, -15, -15, -15, -15, -15,
		-20, -20, -20, -20, -20, -20,
		-25, -25, -25, -25, -25, -25
};

fill( int index, int time, int dur)
{
	int	i, s, d;

	for (i = 0; i < 34; i++, index++) {
		s = time + tlist[i];
		d = dur + dlist[i];

		if (start[index] == INVALID) {
			start[index] = s;
			leng[index] = d;
			flags[index] = GUESS | CHANGED;
		}
		else if (start[index] != s || leng[index] != d) {
			/* clash */
			if (flags[index] & GUESS) {
				/* override old guess */
				start[index] = s;
				leng[index] = d;
				flags[index] |= CHANGED;
				printf("Overridden old guess value at %d\n", index);
			}
			else {
				printf("Panic at index=%d\n", index);
				printf("Duff guess: %04d %d\n", s, d);
				exit(-1);
			}
		}
	}

	change = 1;
	return;
}

/* the 512-entry tables of starting time and pgm duration */
/* that's a BUG! Tables are LIMIT long. (Spotted by someone on the net) */
void
lookup( long i, long *outtime, long *outdur )
{

	if(initializer == 0) {
		initializer = 1;
		settables(start, leng);
	}

# ifdef UK
	if (i & (~0x1FF)) {
		/* bits set above t8 */
		if (debug) {
			printf("Extra time bits (s6/s1): ");
			putchar( (i & 0x400)? '1': '0' );
			putchar('/');
			putchar( (i & 0x200)? '1': '0' );
			putchar('\n');
		}
# ifdef OLD
		switch (( int )(i >> 9)) {
		case 0x01:
			/* just dump extra bits for now */
			i &= 0x1FF;
			i += 858;
			break;

		case 0x02:
			/* just dump extra bits for now */
			i &= 0x1FF;
			i += 1370;
			break;

		case 0x03:
			/* Doug checks c4 and c5 and corrects by 186 always */
			/* I check s1 and s0, correct depending on range */
			/* Basically, 90, 120, 150, 180 min programmes have */
			/* 6 digit codes */
			i &= 0x1FF;

			if (i >= 378)
				i -= 186;
			else
				i += 480; /* hoist codes 0-377 out of the way */
		}
# endif /* OLD */
	}
# ifdef OLD
	else if (i >= 192)
		i += (1882 - 192);
# endif /* OLD */
# endif /* UK */

	if (debug)
		printf("Index used = %d\n", i);
	
	if((i >= LIMIT) || (i < 0))
		printf("\nPANIC, Illegal table index %ld\n", i);

	fflush(stdout);
	*outtime = (start[i]) ;
	*outdur = (leng[i]) ;

	if( (*outtime == INVALID) || (*outdur == INVALID) )
	{
		int	k;
		long	tstart, dur;
		char	buffer[32];

		printf("Oops, I don't have an entry for index %ld\a\n", i);
		fputs("Start time? ", stdout);
		fgets(buffer, sizeof buffer, stdin);

		if (buffer[0] >= '0' && buffer[0] <= '9') {
			tstart = atol(buffer);
			fputs("End time? ", stdout);
			dur = atol(fgets(buffer, sizeof buffer, stdin));
			dur = tdiff(dur, tstart);

			if ((k = scanfor(tstart, dur)) >= 0) {
				/* clash! */
				printf("Clash with %d\a\n", k);
			}

			start[i] = tstart;
			leng[i] = dur;
			flags[i] = CHANGED;
			change = 1;
		}
		else
			tstart = dur = 0;

		(*outtime) = tstart;
		(*outdur) = dur;
	}
	else {
		/* time was OK - how about flags? */
		if (flags[i] & GUESS)
			fputs("This is an educated guess!\n", stdout);
		if (flags[i] & SUSPECTDUR)
			fputs("Duration is suspect - please check\n", stdout);
		if (flags[i] & SUSPECT)
			fputs("Generally suspect - please check\n", stdout);
		if (flags[i] & CLASH)
			fputs("Clash with another entry\n", stdout);
		if (flags[i])
			fputc('\a', stdout);
	}

	return;
}

pbin( int k )
{
	int	count;

	for (count = 9; count; count--) {
		putchar( (k & 0x100)? '1': '0' );
		k <<= 1;
	}

	return;
}

int
scanfor( long s, long d )
{
	int	j;

	if (initializer == 0) {
		initializer = 1;
		settables(start, leng);
	}

	for (j = 0; j < LIMIT; j++) {
		if (start[j] == s && leng[j] == d) {
			/* printf("Match at %d", j); */
			return j;
		}
	}

	return -1;
}

dump()
{
	static char	buffer[128];
	int		j, k;
	FILE		*in, *out;

	if (initializer == 0)
		/* don't write if we never read! */
		return;

	if (!change)
		return;

	if ((in = fopen(F_TBL, "r")) && (out = fopen(F_TMP, "w"))) {
		j = 0;
		
		while (j < LIMIT) {
			while (j < LIMIT && !(flags[j] & CHANGED))
				j++;
			
			/* found a change - run thru old table file */
			while (fgets(buffer, sizeof buffer, in)) {
				/* comment lines won't trigger this test.. */
				if ((k = atoi(buffer)) >= j)
					break;

				fputs(buffer, out);
			}

			if (feof(in))
				/* special case */
				k = LIMIT;

			/* insert one or more changed lines */
			while (j < LIMIT && j <= k) {
				fprintf(out, "%d\t%d\t%d", j, start[j],
							leng[j]);

				if (flags[j]) {
					fputc('\t', out);

					if (flags[j] & GUESS)
						fputc('g', out);
					if (flags[j] & SUSPECTDUR)
						fputc('D', out);
					if (flags[j] & SUSPECT)
						fputc('S', out);
					if (flags[j] & CLASH)
						fputc('c', out);
				}

				fputc('\n', out);
				j++;

				while (j <= k && !(flags[j] & CHANGED))
					j++;

			}

			if (k < LIMIT && !(flags[k] & CHANGED))
				/* keep line we read in earlier too */
				fputs(buffer, out);
		}

		fclose(out);
		fclose(in);

		/* rename files */
		unlink(F_BAK);
		rename(F_TBL, F_BAK);
		rename(F_TMP, F_TBL);
	}

	return;
}

long
tdiff( long a, long b )
{
	int	hr, min;

	hr = (a / 100) - (b / 100);
	min = (a % 100) - (b % 100);

	if (min < 0) {
		min += 60;
		hr--;
	}

	if (hr < 0)
		hr += 24;

	return (hr*60) + min;
}
