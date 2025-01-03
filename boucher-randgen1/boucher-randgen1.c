From msuinfo!uwm.edu!math.ohio-state.edu!sdd.hp.com!hplabs!unix.sri.com!csl.sri.com!boucher Wed Oct 27 21:46:00 1993
Path: msuinfo!uwm.edu!math.ohio-state.edu!sdd.hp.com!hplabs!unix.sri.com!csl.sri.com!boucher
From: boucher@csl.sri.com (Peter K. Boucher)
Newsgroups: sci.crypt
Subject: Re: What are the best random numbers for genetic algorithms (was: Re: GA random no. gen.
Date: 18 Oct 1993 21:16:12 GMT
Organization: Computer Science Lab, SRI International
Lines: 82
Distribution: world
Message-ID: <29v12sINNehe@roche.csl.sri.com>
References: <9310132045.AA27577@redwood.csl.sri.com> <CEyDr3.LwK@ryn.mro4.dec.com> <1993Oct17.073805.22511@cactus.org>
NNTP-Posting-Host: redwood.csl.sri.com

ritter@cactus.org (Terry Ritter) writes:
|>  On the other hand, the alternatives seem fairly grim:  If we do
|>  much processing between the RNG and combiner, we can easily lose
|>  the good distribution expected from the RNG, as well as speed.
|>  To do nothing is to leave the RNG open to trivial attack.  And
|>  I am not aware of a faster, better RNG with large state, long
|>  cycles and arbitrary initialization.

Hi, Terry.

I don't know it you have time or are interested, but I Think I have a 
good approach that's slightly different from what you describe.  I use
three internal (fast) RNG's.  Two produce output bits (32 bits each, 64
bits total).  The third controls a transposition of the 64 bits.  I've
enclosed a code fragment for your perusal.

-Peter K. Boucher

===================== CODE FRAGMENT ===============================
#define DEG_1 367
#define SEP_1 171 /* irreducible trinomial x**367 + x**171 + 1 */

#define DEG_2 887
#define SEP_2 336 /* irreducible trinomial x**887 + x**336 + 1 */

#define DEG_3 1489
#define SEP_3 252 /* irreducible trinomial x**1489 + x**252 + 1 */

#define ROR_32(A,X) (((A)>>(X))|((A)<<(32-(X))))
#define SWAP_BITS(A,B,C,D,X) A=((C)&(X))|((D)&(~(X))); B=((D)&(X))|((C)&(~(X)))

static unsigned long statea[DEG_1], *rptra, *fptra, *enda = &statea[DEG_1],
                     stateb[DEG_2], *rptrb, *fptrb, *endb = &stateb[DEG_2],
                     statec[DEG_3], *rptrc, *fptrc, *endc = &statec[DEG_3];
static int           have_one = 0;


void vrand1(out1,out2)
unsigned long *out1, *out2;
{
    unsigned long msk  = *fptra += *rptra;
    unsigned long tmp1 = *fptrb += *rptrb;
    unsigned long tmp2 = *fptrc += *rptrc;
    char *sb = (char *)(&msk);

    tmp1 = ROR_32(tmp1,(sb[1]&31));
    tmp2 = ROR_32(tmp2,(sb[2]&31));
    msk  = ROR_32(msk, (sb[3]&31));
    SWAP_BITS(*out1,*out2,tmp1,tmp2,msk);
    if ( ++fptra  >=  enda )  {
        fptra = statea;
        ++rptra;
    } else  {
        if ( ++rptra  >=  enda )  rptra = statea;
    }
    if ( ++fptrb  >=  endb )  {
        fptrb = stateb;
        ++rptrb;
    } else  {
        if ( ++rptrb  >=  endb )  rptrb = stateb;
    }
    if ( ++fptrc  >=  endc )  {
        fptrc = statec;
        ++rptrc;
    } else  {
        if ( ++rptrc  >=  endc )  rptrc = statec;
    }
}

unsigned long rand1()
{
    static unsigned long out1, out2;

    if ( have_one ) {
        have_one = 0;
        return( out1 );
    } else {
        have_one = 1;
        vrand1(&out1,&out2);
        return( out2 );
    }
}

