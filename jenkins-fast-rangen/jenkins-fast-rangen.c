[Note:  It is claimed that this code violates a patent.]

From msuinfo!agate!ihnp4.ucsd.edu!swrinde!sgiblab!gatekeeper.us.oracle.com!oracle!unrepliable!bounce Thu Mar 10 10:26:17 1994
Path: msuinfo!agate!ihnp4.ucsd.edu!swrinde!sgiblab!gatekeeper.us.oracle.com!oracle!unrepliable!bounce
Newsgroups: sci.crypt
From: rjenkins@us.oracle.com (Robert Jenkins)
Subject: secure RNG
Message-ID: <1994Mar8.032019.8153@oracle.us.oracle.com>
Originator: rjenkins@kr2seq.us.oracle.com
Sender: usenet@oracle.us.oracle.com (Oracle News Poster)
Nntp-Posting-Host: kr2seq.us.oracle.com
Organization: Oracle Corp., Redwood Shores CA
Date: Tue, 8 Mar 1994 03:20:19 GMT
X-Disclaimer: This message was written by an unauthenticated user
              at Oracle Corporation.  The opinions expressed are those
              of the user and not necessarily those of Oracle.
Lines: 90


A random number generator is secure if XORing its numbers with data 
is a safe way to encrypt that data.  How fast can a secure RNG be?  
This RNG produces 32 bits every 13 instructions and might be secure.  
On a 66 Mhz machine it should encrypt over 100 megabits per second.

=====================================================================

typedef  unsigned int  u4;         /* unsigned four bytes, 32 bits */
typedef  unsigned char u1;            /* unsigned one byte, 8 bits */
static u4 myrandi = 0;

static void rng(mm,rr,aa,bb)
u4 *mm;  /* secret memory             */
u4 *rr;  /* results given to the user */
u4 *aa;  /* accumulator */
u4 *bb;  /* the old m[127] */
{
  register u4 a,b,x,y,*mi,*ri,*me;

  a = *aa;  b = *bb;
  for (mi=mm, ri=rr, me = mm+128; mi<me; ++mi, ++ri)
  {
    x = *mi;
    y = *(u4 *)((u1 *)mm+(x&0x1f4));        /* one of m[0]..m[127] */
    a = ((a<<27)^(a>>5));                         /* barrelshift a */
    a   = a+y;                                      /* accumulator */
    *mi = a+b;                                    /* secret memory */
    *ri = b+y;                             /* result given to user */
    b = x;
  }
  *bb = b;  *aa = a;
}

#define myrand(val,m,r,a,b) \
  if (1) { \
    if (++myrandi >= 128) { rng((m),(r),(a),(b)); myrandi = 0;} \
    (val) = (r)[myrandi]; \
  }
    
int main()                        /* produce a billion random bits */
{
  u4 i, a, b, m[128], r[128];
  a = b = 1;   for (i=0; i<(128); ++i) m[i] = 0;           /* seed */
  for (i=0; i<100; ++i) rng(m,r,&a,&b);          /* mix it up some */
  myrand(i,m,r,&a,&b);                   /* one possible interface */
  for (i=0; i<(1000000000/(32*128)); ++i) rng(m,r,&a,&b);
  printf("test: %lx %lx %lx %lx\n",r[0],r[1],r[2],r[3]);   /* test */
}

=====================================================================
test: a490ec5c 9eb14359 f556e629 c1c811a9
=====================================================================

Any rng with n bits of internal state has at most 2^n possible 
states.  It cannot step through 1+2^n states without cycling, and all
statistical tests will fail shortly after that.  I varied the array
length and word size for this generator with these results:

          2-bit 3-bit  4-bit  5-bit  6-bit  7-bit
   m[2]   (8)8 (12)11 (16)15 (20)15 (24)17 (28)17 .. run fails at 17
   m[4]        (18)15 (24)17 (30)24 (36)24 (42)29 .. too big to test
   m[8]               (40)27 (50)30+ .. too big to test

The number in parentheses is the number of bits of state.  The other
number, hm, if statistical tests start failing after 2^x calls to rng,
then the other number is x.  The C code given here is for m[128],
32-bit, which is way off the scale.  If my previously posted rngs 
failed at about 2^x, this one fails at 2^(2x).  Tests take so long to
fail that I could not recognize a better rng if I was given one.

This rng MIGHT be secure.  Each pass through the loop gives you three 
linear equations
  a[i] =            a[i-1]_barrelshifted + y[i]
  m[i] = m[i-129] + a[i-1]_barrelshifted + y[i]
  r[i] = m[i-129]                        + y[i]
and four unknowns (a[i],m[i],a[i-1]_barrelshifted,y[i]).  The equation
  a[i-1]_barrelshifted = barrelshift(a[i-1])
doesn't count because it's nonlinear.  The equation
  y[i] = m[i-f(x)]
doesn't count because there are no tools for dealing with indirection.
If you could use the barrelshift equation, you'd still need to find 130 
more equations to have as many equations as unknowns.

Random.  Fast.  Easy to code.  Probably unbreakable.  This rng was
designed for encryption and simulation.  It is now unpatentable and in
the worldwide public domain.  Please use it!

					- Bob Jenkins
					rjenkins@us.oracle.com

From msuinfo!agate!dog.ee.lbl.gov!ihnp4.ucsd.edu!usc!elroy.jpl.nasa.gov!decwrl!oracle!unrepliable!bounce Thu Mar 10 10:26:29 1994
Path: msuinfo!agate!dog.ee.lbl.gov!ihnp4.ucsd.edu!usc!elroy.jpl.nasa.gov!decwrl!oracle!unrepliable!bounce
Newsgroups: sci.crypt
From: rjenkins@uucp (Robert Jenkins)
Subject: Re: secure RNG
Message-ID: <1994Mar8.181014.29191@oracle.us.oracle.com>
Sender: usenet@oracle.us.oracle.com (Oracle News Poster)
Nntp-Posting-Host: kr2seq.us.oracle.com
Organization: /usr/local/news/organization
References: <1994Mar8.032019.8153@oracle.us.oracle.com>
Date: Tue, 8 Mar 1994 18:10:14 GMT
X-Disclaimer: This message was written by an unauthenticated user
              at Oracle Corporation.  The opinions expressed are those
              of the user and not necessarily those of Oracle.
Lines: 43

This morning I received mail from cjames@nyx.cs.du.edu stating

> The assertions that what you posted is a prng and can not be
> patented by the act of publishing it are both mistaken.
> 
> Donald Knuth in _The Art of Computer Programming_ published a
> superset implementation of your source code and warned it is
> not a good source for random numbers.  This was about 1973.
> In 1971, Knuth is listed as the inventor with assignee Burroughs
> of a US Patent which implements his 1973 algorithm in part in
> hardware (and hence software).  That patent has expired.
>  
> Hence what you demonstrated was already invented by Knuth over
> 20 years ago.
>  
> However, it is possible to make Knuth's original published
> algorithm a good source of random numbers.  This is done in
> US Patent No 5_251_165, "Two phase random number generator",
> which operates even faster than what you posted on Internet.
>
> Please be advised that your post infringes one or more claims
> of that patent.  Consequently you are hereby requested to
> state on Internet sci.crypt that your code may be in violation
> of the named patent above and is retracted as "unpatentable"
> and something "everyone can use."
 
I hereby retract that the posted rng is unpatentable, because it may 
be covered by patent 5_251_165 or some other previous patent.  I 
restrict my claim that anyone can use it to simply, *I* don't mind if 
anyone uses it.  Finally, I was responsible for developing and posting 
the rng, Oracle was not involved (except that that is where my mail 
account is).

I do not know the rng used in patent 5_251_165, and I don't know what 
claims the posted rng violates, so I can't be any more specific than 
this.  I continue to claim that what I posted is a good prng.

					- Bob Jenkins

The opinions and responsibility for this note are my own, not Oracle's.




