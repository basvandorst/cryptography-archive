/*From msuinfo!agate!howland.reston.ans.net!cs.utexas.edu!not-for-mail Fri Mar 11 23:00:03 1994
Path: msuinfo!agate!howland.reston.ans.net!cs.utexas.edu!not-for-mail
From: boucher@csl.sri.com (Peter K. Boucher)
Newsgroups: sci.crypt
Subject: Modified Jenkins PRNG source
Date: 11 Mar 1994 19:24:03 -0600
Organization: UTexas Mail-to-News Gateway
Lines: 149
Sender: daemon@cs.utexas.edu
Message-ID: <9403120123.AA15215@redwood.csl.sri.com>
NNTP-Posting-Host: cs.utexas.edu
 */
/* --------------------------------------------------------------------------
  "Random.  Fast.  Easy to code.  Probably unbreakable.  This rng was
   designed for encryption and simulation.  It is now unpatentable and in
   the worldwide public domain.  Please use it!"

					  - Bob Jenkins
					  rjenkins@us.oracle.com

  "Modified to take 128-bits of seed.  Buffer size increased to 1024.
   Function rand32_mod() added."

					  - Peter K. Boucher
					  boucher@csl.sri.com
-------------------------------------------------------------------------- */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

The following 3 defines are related.  If you change one, change all three.

   WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
#define BSIZE 1024 /* #longs in secret state (must be power of 2) */
#define BMASK 1023 /* BSIZE - 1 */
#define BSHFT 22   /* 32 - #bits in BMASK */

static unsigned long num_rands = 0;
static unsigned long *rand_ptr;
static unsigned long m[BSIZE], r[BSIZE];
static unsigned long A, B;
static unsigned long *me = &(m[BSIZE]);

static unsigned long	refresh_rng_state()
{
	unsigned long a,b,x,y,*mi,*ri;

	a 	= A;
	b 	= B;
	mi 	= m;
	ri 	= r;
	while (mi < me)
	{
		x       = *mi;
		y       = m[x>>BSHFT];                    /* one of m[0]..m[BMASK] */
		a       = ((a<<27)^(a>>5));               /* barrelshift a */
		a       = a+y;                            /* accumulator */
		*(mi++) = a+b;                            /* secret memory */
		*(ri++) = b+y;                            /* result given to user */
		b       = x;
	}
	B 	= b;
	A 	= a;

	num_rands 	= BMASK;
	rand_ptr 	= &(r[1]);
	return( r[0] );
}

unsigned long	rand32()
{
	return( (num_rands--) ? *(rand_ptr++) : refresh_rng_state() );
}

unsigned long   rand32_mod(unsigned long max)
{
	unsigned long rdm_val;
	unsigned long limit = ((0xFFFFFFFFL/max)*max);

	if (max < 2L) return(0L);

	do
	{
		rdm_val = ( (num_rands--) ? *(rand_ptr++) : refresh_rng_state() );
	} while (rdm_val >= limit);

	return(rdm_val % max);
}

void	srand128(unsigned long seed1,unsigned long seed2,unsigned long seed3,unsigned long seed4)
{
#define LC1(X) (X * 66049 + 3907864577)
#define LC2(X) (X * 69069)

	register int i,j;
	unsigned long rdm_val=0L;
	unsigned long x=seed1;
	unsigned long y=seed2;

	while ((x&1) == 0)
	{
		x = LC1(x);
	}
	for (i=BSIZE; i--;)
	{
		for (j=32;j--;)
		{
			if (y == 0) y = LC1(x);
			y ^= (y >> 15);
			y ^= (y << 17);
			rdm_val = (rdm_val>>1) | (0x80000000 & ((x=LC2(x))+y));
		}
		m[i] = rdm_val;
	}
	for (i=32;i--;)
	{
		if (y == 0) y = LC1(x);
		y ^= (y >> 15);
		y ^= (y << 17);
		rdm_val = (rdm_val>>1) | (0x80000000 & ((x=LC2(x))+y));
	}
	A = rdm_val;
	for (i=32;i--;)
	{
		if (y == 0) y = LC1(x);
		y ^= (y >> 15);
		y ^= (y << 17);
		rdm_val = (rdm_val>>1) | (0x80000000 & ((x=LC2(x))+y));
	}

	x	= seed3;
	y	= seed4;
	while ((x&1) == 0)
	{
		x = LC1(x);
	}
	for (i=0; i<BSIZE; i++)
	{
		for (j=32;j--;)
		{
			if (y == 0) y = LC1(x);
			y ^= (y >> 15);
			y ^= (y << 17);
			rdm_val = (rdm_val<<1) | (((x=LC2(x))+y) >> 31);
		}
		m[i] ^= rdm_val;
	}
	for (i=32;i--;)
	{
		if (y == 0) y = LC1(x);
		y ^= (y >> 15);
		y ^= (y << 17);
		rdm_val = (rdm_val<<1) | (((x=LC2(x))+y) >> 31);
	}
	A ^= rdm_val;
	for (i=32;i--;)
	{
		if (y == 0) y = LC1(x);
		y ^= (y >> 15);
		y ^= (y << 17);
		rdm_val = (rdm_val<<1) | (((x=LC2(x))+y) >> 31);
	}
	B ^= rdm_val;

	for (i=100; i--;)
	{
		(void)refresh_rng_state();          /* mix it up some */
	}
	for (i=1+(m[(seed1^seed2^seed3^seed4)&BMASK]&BMASK); i--;)
	{
		(void)rand32();
	}
}

