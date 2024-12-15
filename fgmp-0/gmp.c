/*
 * FREE GMP - a public domain implementation of a subset of the 
 *           gmp library
 *
-----BEGIN PRIVACY-ENHANCED MESSAGE-----
Proc-Type: 2001,MIC-CLEAR
Originator-Name: markh@wimsey.bc.ca
Originator-Key-Asymmetric:
 MIGeMAoGBFUIAQECAgQAA4GPADCBiwKBgQD6pxeLMchQF50lf9TbhnVcF15yObiT
 q5oDvpG6pf+Al1W2s714ATGsERdyEfDwvhqPCK+OujqZxkrx2hJoVNs6Wpma125s
 MMeCBmkj1wIiNO8fYYpeYXJo4kbw/MhdiYclV1aJrH51KXbGdCWoiFezK5tgoW8d
 fb8uPMQULJagIwIFAIAAAIc=
MIC-Info: RSA-MD5,RSA,
 KbBGxWz9HUe27rWZqGV0fmpITQW157NlWjYf1PjakTSsT66fK9Hw9Qb43Bxj0YRd
 RpM3dpNvTZMQuAPibgjWY1LzczcekTv9oiEyXg+/KQv7bzt9jow7/oGsiQs4Pkn5
 yEjWvzsXUMsf1O5OddiQCwUWebmn1R2A8S+swi+TN9w=

I hearby place FGMP in the public domain.
-----END PRIVACY-ENHANCED MESSAGE-----

 * Do whatever you want with this code. Change it. Sell it. Claim you
 *  wrote it. 
 * Bugs, complaints, flames, rants: please send email to 
 *    Mark Henderson <markh@wimsey.bc.ca>
 * I'm already aware that fgmp is considerably slow than gmp
 * VERSION 0.2
 */

#include "gmp.h"
#ifndef NULL
#define NULL ((void *)0)
#endif

#define iabs(x) ((x>0) ? (x) : (-x))
#define imax(x,y) ((x>y)?x:y)
#define LONGBITS (sizeof(long) * 8)
#define DIGITBITS (LONGBITS - 2)
#define HALFDIGITBITS ((LONGBITS -2)/2)
#ifndef init
#define init(g) { g = (MP_INT *)malloc(sizeof(MP_INT));  mpz_init(g); }
#endif
#ifndef clear
#define clear(g) { mpz_clear(g); free(g); }
#endif
#ifndef even
#define even(a) (!((a)->p[0] & 1))
#endif
#ifndef odd
#define odd(a) (((a)->p[0] & 1))
#endif


/* 
 * The values below are for 32 bit machines
 * You'll need to change them, if you're using something else
 */
#define LMAX 0x3fffffff
#define LC   0xc0000000
#define OVMASK 0x2
#define CMASK (LMAX+1)
#define FC ((double)CMASK)
#define HLMAX 0x7fff
#define HCMASK (HLMAX + 1)
#define HIGH(x) (((x) & 0x3fff8000) >> 15)
#define LOW(x)  ((x) & 0x7fff)


#define hd(x,i)  (((i)>=2*(x->sz))? 0:(((i)%2) ? HIGH((x)->p[(i)/2]) : LOW((x)->p[(i)/2])))
#define dg(x,i) ((i < x->sz) ? (x->p)[i] : 0)

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

#define lowdigit(x) (((x)->p)[0])

struct is {
    int v;
    struct is *next;
};


INLINE static void push(i,sp)
int i;struct is **sp;
{
    struct is *tmp;
    tmp = *sp;
    *sp = (struct is *) malloc(sizeof(struct is));
    (*sp)->v = i;
    (*sp)->next=tmp;
}

INLINE static int pop(sp)
struct is **sp;
{
    struct is *tmp;
    int i;
    if (!(*sp))
        return (-1);
    tmp = *sp;
    *sp = (*sp)->next;
    i = tmp->v;
    tmp->v = 0;
    free(tmp);
    return i;
}

void fatal(s)
char *s;
{
    fprintf(stderr,"%s\n",s);
    exit(123);
}

void mpz_init(s)
MP_INT *s;
{
    s->p = (long *)malloc(sizeof(long)*2);
#ifdef DEBUG
    printf("malloc: 8 bytes, %08x\n", s->p);
#endif
    if (!(s->p))
        fatal("mpz_init: cannot allocate memory");
    (s->p)[0] = 0;
    (s->p)[1] = 0;
    s->sn=0;
    s->sz=2;
}

void mpz_init_set(s,t)
MP_INT *s,*t;
{
    int i;
    s->p = (long *)malloc(sizeof(long) * t->sz);
    if (!(s->p))
        fatal("mpz_init: cannot allocate memory");
    for (i=0;i < t->sz ; i++)
        (s->p)[i] = (t->p)[i];

    s->sn = t->sn;
    s->sz = t->sz;
}

void mpz_init_set_ui(s,v)
MP_INT *s;
unsigned long v;
{
    s->p = (long *)malloc(sizeof(long)*2);
    if (!(s->p))
        fatal("mpz_init: cannot allocate memory");
    s->p[0] = (v & LMAX);
    s->p[1] = (v & LC);
    if (v) 
        s->sn = 1;
    else
        s->sn = 0;
    s->sz = 2;
}

void mpz_init_set_si(y,v)
MP_INT *y;
long v;
{
    y->p = (long *)malloc(sizeof(long)*2);
    if (!(y->p))
        fatal("mpz_init: cannot allocate memory");
    if (v < 0) {
        y->sn = -1;
        y->p[0] = (-v) & LMAX;
        y->p[1] = (-v) & LC;
    }
    else if (v > 0) {
        y->sn = 1;
        y->p[0] = v & LMAX;
        y->p[1] = v & LC;
    }
    else {
        y->sn=0;
        y->p[0] = 0;
        y->p[1] = 0;
    }
    y -> sz = 2;
}



void mpz_clear(s)
MP_INT *s;
{
#ifdef DEBUG
    printf("free: %08x\n", s->p);
#endif
    if (s->p)
        free(s->p);
    s->p=NULL;
    s->sn=0;
    s->sz=0;
}

/* number of leading zero bits in digit */
static int lzb(a)
long a;
{
    long i; int j;
    j = 0;
    for (i = (1 << (DIGITBITS-1)); i && !(a&i) ; j++,i>>=1) 
        ;
    return j;
}

void _mpz_realloc(x,size)
MP_INT *x; long size;
{
    if (size > 1 && x->sz < size) {
        int i;
#ifdef DEBUG
    printf("realloc %08x to size = %d ", x->p,size);
#endif

        if (x->p) 
            x->p=(long *)realloc(x->p,size * sizeof(long));
        else
            x->p=(long *)malloc(size * sizeof(long));
#ifdef DEBUG
    printf("becomes %08x\n", x->p);
#endif

        if (!(x->p))
            fatal("_mpz_realloc: cannot allocate memory");
        for (i=x->sz;i<size;i++)
            (x->p)[i] = 0;
        x->sz = size;
    }
}

void dgset(x,i,n)
MP_INT *x; unsigned int i; long n;
{
    if (n) {
        if (i >= x->sz) {
            _mpz_realloc(x,i+1);
            x->sz = i+1;
        }
        (x->p)[i] = n;
    }
}

static unsigned int digits(x)
MP_INT *x;
{
    int i;
    for (i = (x->sz) - 1; i>=0 && (x->p)[i] == 0 ; i--) 
        ;
    return i+1;
}
        
/* y = x */
void mpz_set(y,x)
MP_INT *y; MP_INT *x; 
{
    unsigned int i,k = x->sz;
    if (y->sz < k) {
        k=digits(x);
        _mpz_realloc(y,k);
    }
    if (y->sz > x->sz) {
        mpz_clear(y); mpz_init(y); _mpz_realloc(y,x->sz);
    }

    for (i=0;i < k; i++)
        (y->p)[i] = (x->p)[i];

    for (;i<y->sz;i++)
        (y->p)[i] = 0;

    y->sn = x->sn;
}

void mpz_set_ui(y,v)
MP_INT *y; unsigned long v; {
    int i;
    for (i=1;i<y->sz;i++)
        y->p[i] = 0;
    y->p[0] = (v & LMAX);
    y->p[1] = (v & LC) >> (DIGITBITS);
    if (v)
        y->sn=1;
    else
        y->sn=0;
}

unsigned long mpz_get_ui(y) 
MP_INT *y; {
    return (y->p[0] | (y->p[1] << DIGITBITS));
}

long mpz_get_si(y)
MP_INT *y; {
    if (y->sn == 0)
        return 0;
    else
        return (y->sn * (y->p[0] | (y->p[1] & 1) << DIGITBITS));
}
        
void mpz_set_si(y,v)
MP_INT *y; long v; {
    int i;
    for (i=1;i<y->sz;i++)
        y->p[i] = 0;
    if (v < 0) {
        y->sn = -1;
        y->p[0] = (-v) & LMAX;
        y->p[1] = ((-v) & LC) >> DIGITBITS;
    }
    else if (v > 0) {
        y->sn = 1;
        y->p[0] = v & LMAX;
        y->p[1] = (v & LC) >> DIGITBITS;
    }
    else {
        y->sn=0;
        y->p[0] = 0;
        y->p[1] = 0;
    }
}

/* z = x + y, without regard for sign */
static void uadd(z,x,y)
MP_INT *z; MP_INT *x; MP_INT *y;
{
    long c;
    int i;
    MP_INT *t;

    if (y->sz < x->sz) {
        t=x; x=y; y=t;
    }

    /* now y->sz >= x->sz */

    _mpz_realloc(z,(y->sz)+1);

    c=0;
    for (i=0;i<x->sz;i++) {
        if (( z->p[i] = y->p[i] + x->p[i] + c ) & CMASK) {
            c=1; 
            (z->p[i]) &=LMAX;
        }
        else 
            c=0;
    }
    for (;i<y->sz;i++) {
        if ((z->p[i] = (y->p[i] + c)) & CMASK)
            z->p[i]=0;
        else
            c=0;
    }
    (z->p)[y->sz]=c;
}

/* z = y - x, ignoring sign */
/* precondition: abs(y) >= abs(x) */
static void usub(z,y,x)
MP_INT *z; MP_INT *y; MP_INT *x; 
{
    int i;
    long b,m;
    _mpz_realloc(z,(y->sz));
    b=0;
    for (i=0;i<y->sz;i++) {
        m=((y->p)[i]-b)-dg(x,i);
        if (m < 0) {
            b = 1;
            m = LMAX + 1 + m;
        }
        else
            b = 0;
        z->p[i] = m;
    }
}

/* compare abs(x) and abs(y) */
static int ucmp(y,x)
MP_INT *y;MP_INT *x;
{
    int i;
    for (i=imax(x->sz,y->sz);i>=0;i--) {
        if (dg(y,i) < dg(x,i))
            return (-1);
        else if (dg(y,i) > dg(x,i))
            return 1;
    }
    return 0;
}

int mpz_cmp(x,y)
MP_INT *x; MP_INT *y;
{
    int abscmp;
    if (x->sn < 0 && y->sn > 0)
        return (-1);
    if (x->sn > 0 && y->sn < 0)
        return 1;
    abscmp=ucmp(x,y);
    if (x->sn >=0 && y->sn >=0)
        return abscmp;
    if (x->sn <=0 && y->sn <=0)
        return (-abscmp);
}
    
/* is abs(x) == 0 */
static int uzero(x)
MP_INT *x;
{
    int i;
    for (i=0; i < x->sz; i++)
        if ((x->p)[i] != 0)
            return 0;
    return 1;
}

static void zero(x)
MP_INT *x;
{
    int i;
    x->sn=0;
    for (i=0;i<x->sz;i++)
        (x->p)[i] = 0;
}


/* w = u * v */
void mpz_mul(ww,u,v)
MP_INT *ww;MP_INT *u; MP_INT *v; {
    int i,j;
    long t0,t1,t2,t3;
    long cc;
    MP_INT *w;

    w = (MP_INT *)malloc(sizeof(MP_INT));
    mpz_init(w);
    _mpz_realloc(w,u->sz + v->sz);
    for (j=0; j < 2*u->sz; j++) {
        cc = 0;
        t3 = hd(u,j);
            for (i=0; i < 2*v->sz; i++) {
                t0 = t3 * hd(v,i);
                t1 = HIGH(t0); t0 = LOW(t0);
                if ((i+j)%2) 
                    t2 = HIGH(w->p[(i+j)/2]);
                else
                    t2 = LOW(w->p[(i+j)/2]);
                t2 += cc;
                if (t2 & HCMASK) {
                    cc = 1; t2&=HLMAX;
                }
                else
                    cc = 0;
                t2 += t0;
                if (t2 & HCMASK) {
                    cc++ ; t2&=HLMAX;
                }
                cc+=t1;
                if ((i+j)%2)
                    w->p[(i+j)/2] = LOW(w->p[(i+j)/2]) |
                        (t2 << HALFDIGITBITS);
                else
                    w->p[(i+j)/2] = (HIGH(w->p[(i+j)/2]) << HALFDIGITBITS) |
                        t2;
                    
            }
        if (cc) {
            if ((j+i)%2) 
                w->p[(i+j)/2] += cc << HALFDIGITBITS;
            else
                w->p[(i+j)/2] += cc;
        }
    }
    w->sn = (u->sn) * (v->sn);
    if (w != ww) {
        mpz_set(ww,w);
        mpz_clear(w);
        free(w);
    }
}

/* n must be < DIGITBITS */
static void urshift(c1,a,n)
MP_INT *c1;MP_INT *a;int n;
{
    long cc = 0;
    if (n >= DIGITBITS)
        fatal("urshift: n >= DIGITBITS");
    if (n == 0)
        mpz_set(c1,a);
    else {
        MP_INT c; int i;
        long rm = ((1<<n) - 1);
        mpz_init(&c); _mpz_realloc(&c,a->sz);
        for (i=a->sz-1;i >= 0; i--) {
            c.p[i] = ((a->p[i] >> n) | cc) & LMAX;
            cc = (a->p[i] & rm) << (DIGITBITS - n);
        }
        mpz_set(c1,&c);
        mpz_clear(&c);
    }
}
    
/* n must be < DIGITBITS */
static void ulshift(c1,a,n)
MP_INT *c1;MP_INT *a;int n;
{
    long cc = 0;
    if (n >= DIGITBITS)
        fatal("ulshift: n >= DIGITBITS");
    if (n == 0)
        mpz_set(c1,a);
    else {
        MP_INT c; int i;
        long rm = ((1<<n) - 1) << (DIGITBITS -n);
        mpz_init(&c); _mpz_realloc(&c,a->sz + 1);
        for (i=0;i < a->sz; i++) {
            c.p[i] = ((a->p[i] << n) | cc) & LMAX;
            cc = (a->p[i] & rm) >> (DIGITBITS -n);
        }
        c.p[i] = cc;
        mpz_set(c1,&c);
        mpz_clear(&c);
    }
}

void mpz_div_2exp(z,x,e) 
MP_INT *z; MP_INT *x; unsigned long e;
{
    short sn = x->sn;
    if (e==0)
        mpz_set(z,x);
    else {
        unsigned long i;
        long digs = (e / DIGITBITS);
        unsigned long bs = (e % 30);
        MP_INT y; mpz_init(&y);
        _mpz_realloc(&y,(x->sz) - digs);
        for (i=0; i < (x->sz - digs); i++)
            (y.p)[i] = (x->p)[i+digs];
        if (bs) {
            urshift(z,&y,bs);
        }
        else {
            mpz_set(z,&y);
        }
        if (uzero(z))
            z->sn = 0;
        else
            z->sn = sn;
        mpz_clear(&y);
    }
}

void mpz_mul_2exp(z,x,e) 
MP_INT *z; MP_INT *x; unsigned long e;
{
    short sn = x->sn;
    if (e==0)
        mpz_set(z,x);
    else {
        unsigned long i;
        long digs = (e / DIGITBITS);
        unsigned long bs = (e % 30);
        MP_INT y; mpz_init(&y);
        _mpz_realloc(&y,(x->sz)+digs);
        for (i=digs;i<((x->sz) + digs);i++)
            (y.p)[i] = (x->p)[i - digs];
        if (bs) {
            ulshift(z,&y,bs);
        }
        else {
            mpz_set(z,&y);
        }
        z->sn = sn;
        mpz_clear(&y);
    }
}

void mpz_mod_2exp(z,x,e) 
MP_INT *z; MP_INT *x; unsigned long e;
{
    short sn = x->sn;
    if (e==0)
        mpz_set_ui(z,0);
    else {
        unsigned long i;
        MP_INT y;
        long digs = (e / DIGITBITS);
        unsigned long bs = (e % 30);
        if (digs > x->sz || (digs == (x->sz) && bs)) {
            mpz_set(z,x);
            return;
        }
            
        mpz_init(&y);
        if (bs)
            _mpz_realloc(&y,digs+1);
        else
            _mpz_realloc(&y,digs);
        for (i=0; i<digs; i++)
            (y.p)[i] = (x->p)[i];
        if (bs) {
            (y.p)[digs] = ((x->p)[digs]) & ((1 << bs) - 1);
        }
        mpz_set(z,&y);
        if (uzero(z))
            z->sn = 0;
        else
            z->sn = sn;
        mpz_clear(&y);
    }
}
    

/* internal routine to compute x/y and x%y ignoring signs */
static void udiv(qq,rr,xx,yy)
MP_INT *qq; MP_INT *rr; MP_INT *xx; MP_INT *yy;
{
    MP_INT *q, *x, *y, *r;
    int ns,xd,yd,j,f,i,ccc;
    long zz,z,qhat,b,u,m;
    ccc=0;

    if (uzero(yy))
        return;
    q = (MP_INT *)malloc(sizeof(MP_INT));
    r = (MP_INT *)malloc(sizeof(MP_INT));
    x = (MP_INT *)malloc(sizeof(MP_INT)); y = (MP_INT *)malloc(sizeof(MP_INT));
    if (!x || !y || !q || !r)
        fatal("udiv: cannot allocate memory");
    mpz_init(q); mpz_init(x);mpz_init(y);mpz_init(r);
    _mpz_realloc(x,(xx->sz)+1);
    yd = digits(yy);
    ns = lzb(yy->p[yd-1]);
    ulshift(x,xx,ns);
    ulshift(y,yy,ns);
    xd = digits(x); 
    _mpz_realloc(q,xd);
    xd*=2; yd*=2;
    z = hd(y,yd-1);;
    for (j=(xd-yd);j>=0;j--) {
#ifdef DEBUG
    printf("y=");
    for (i=yd-1;i>=0;i--)
        printf(" %04x", hd(y,i));
    printf("\n");
    printf("x=");
    for (i=xd-1;i>=0;i--)
        printf(" %04x", hd(x,i));
    printf("\n");
    printf("z=%08x\n",z);
#endif
        
        if (z == LMAX)
            qhat = hd(x,j+yd);
        else {
            qhat = ((hd(x,j+yd)<< HALFDIGITBITS) + hd(x,j+yd-1)) / (z+1);
        }
#ifdef DEBUG
    printf("qhat=%08x\n",qhat);
    printf("hd=%04x/%04x\n",hd(x,j+yd),hd(x,j+yd-1));
#endif
        b = 0; zz=0;
        if (qhat) {
            for (i=0; i < yd; i++) {
                zz = qhat * hd(y,i);
                u = hd(x,i+j);
                u-=b;
                if (u<0) {
                    b=1; u+=HLMAX+1;
                }
                else
                    b=0;
                u-=LOW(zz);
                if (u < 0) {
                    b++;
                    u+=HLMAX+1;
                }
                b+=HIGH(zz);
                if ((i+j)%2) 
                    x->p[(i+j)/2] = LOW(x->p[(i+j)/2]) | (u << HALFDIGITBITS);
                else
                    x->p[(i+j)/2] = (HIGH(x->p[(i+j)/2]) << HALFDIGITBITS) | u;
            }
            if (b) {
                if ((j+i)%2) 
                    x->p[(i+j)/2] -= b << HALFDIGITBITS;
                else
                    x->p[(i+j)/2] -= b;
            }
        }
#ifdef DEBUG
        printf("x after sub=");
        for (i=xd-1;i>=0;i--)
            printf(" %04x", hd(x,i));
        printf("\n");
#endif
        for(;;zz++) {
            f=1;
            if (!hd(x,j+yd)) {
                for(i=yd-1; i>=0; i--) {
                    if (hd(x,j+i) > hd(y,i)) {
                        f=1;
                        break;
                    }
                    if (hd(x,j+i) < hd(y,i)) {
                        f=0;
                        break;
                    }
                }
            }
            if (!f)
                break;
            qhat++;
            ccc++;
#ifdef DEBUG
            printf("corrected qhat = %08x\n", qhat);
#endif
            b=0;
            for (i=0;i<yd;i++) {
                m = hd(x,i+j)-hd(y,i)-b;
                if (m < 0) {
                    b = 1;
                    m = HLMAX + 1 + m;
                }
                else
                    b = 0;
                if ((i+j)%2) 
                    x->p[(i+j)/2] = LOW(x->p[(i+j)/2]) | (m << HALFDIGITBITS);
                else
                    x->p[(i+j)/2] = (HIGH(x->p[(i+j)/2]) << HALFDIGITBITS) | m;
            }
            if (b) {
                if ((j+i)%2) 
                    x->p[(i+j)/2] -= b << HALFDIGITBITS;
                else
                    x->p[(i+j)/2] -= b;
            }
#ifdef DEBUG
            printf("x after cor=");
            for (i=2*(x->sz)-1;i>=0;i--)
                printf(" %04x", hd(x,i));
            printf("\n");
#endif
        }
        if (j%2) 
            q->p[j/2] |= qhat << HALFDIGITBITS;
        else
            q->p[j/2] |= qhat;
#ifdef DEBUG
            printf("x after cor=");
            for (i=xd - 1;i>=0;i--)
                printf(" %04x", hd(q,i));
            printf("\n");
#endif
    }
    _mpz_realloc(r,yy->sz);
    zero(r);
    urshift(r,x,ns);
    mpz_set(rr,r);
    mpz_set(qq,q);
    mpz_clear(x); mpz_clear(y);
    mpz_clear(q);  mpz_clear(r);
    free(q); free(x); free(y); free(r);
}

void mpz_add(zz,x,y)
MP_INT *zz;MP_INT *x;MP_INT *y;
{
    int mg;
    MP_INT *z;
    if (x->sn == 0) {
        mpz_set(zz,y);
        return;
    }
    if (y->sn == 0) {
        mpz_set(zz,x);
        return;
    }
    z = (MP_INT *)malloc(sizeof(MP_INT));
    mpz_init(z);

    if (x->sn > 0 && y->sn > 0) {
        uadd(z,x,y); z->sn = 1;
    }
    else if (x->sn < 0 && y->sn < 0) {
        uadd(z,x,y); z->sn = -1;
    }
    else {
        /* signs differ */
        if ((mg = ucmp(x,y)) == 0) {
            zero(z);
        }
        else if (mg > 0) {  /* abs(y) < abs(x) */
            usub(z,x,y);    
            z->sn = (x->sn > 0 && y->sn < 0) ? 1 : (-1);
        }
        else { /* abs(y) > abs(x) */
            usub(z,y,x);    
            z->sn = (x->sn < 0 && y->sn > 0) ? 1 : (-1);
        }
    }
    mpz_set(zz,z);
    mpz_clear(z);
    free(z);
}

void mpz_add_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_add(x,y,&z);
    mpz_clear(&z);
}

int mpz_cmp_ui(x,n)
MP_INT *x; unsigned long n;
{
    MP_INT z; int ret;
    mpz_init_set_ui(&z,n);
    ret=mpz_cmp(x,&z);
    mpz_clear(&z);
    return ret;
}

int mpz_cmp_si(x,n)
MP_INT *x; long n;
{
    MP_INT z;
    mpz_init(&z);
    mpz_set_si(&z,n);
    mpz_cmp(x,&z);
    mpz_clear(&z);
}


void mpz_mul_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_mul(x,y,&z);
    mpz_clear(&z);
}
    
    
/* z = x - y  -- just use mpz_add - I'm lazy */
void mpz_sub(z,x,y)
MP_INT *z;MP_INT *x; MP_INT *y;
{
    MP_INT u;
    mpz_init(&u);
    mpz_set(&u,y);
    u.sn = -(u.sn);
    mpz_add(z,x,&u);
    mpz_clear(&u);
}

void mpz_sub_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_sub(x,y,&z);
    mpz_clear(&z);
}

void mpz_div(q,x,y)
MP_INT *q; MP_INT *x; MP_INT *y;
{
    MP_INT r; 
    short sn1 = x->sn, sn2 = y->sn;
    mpz_init(&r);
    udiv(q,&r,x,y);
    q->sn = sn1*sn2;
    mpz_clear(&r);
}

void mpz_mdiv(q,x,y)
MP_INT *q; MP_INT *x; MP_INT *y;
{
    MP_INT r; 
    short sn1 = x->sn, sn2 = y->sn;
    mpz_init(&r);
    udiv(q,&r,x,y);
    q->sn = sn1 * sn2;
    /* now if r != 0 and q < 0 we need to round q towards -inf */
    if (!uzero(&r) && q->sn < 0) 
        mpz_sub_ui(q,q,1);
    mpz_clear(&r);
}

void mpz_mod(r,x,y)
MP_INT *r; MP_INT *x; MP_INT *y;
{
    MP_INT q;
    short sn = x->sn;
    mpz_init(&q);
    if (x->sn == 0) {
        zero(r);
        return;
    }
    udiv(&q,r,x,y);
    r->sn = sn;
    if (uzero(r))
        r->sn = 0;
    mpz_clear(&q);
}

void mpz_divmod(q,r,x,y)
MP_INT *q; MP_INT *r; MP_INT *x; MP_INT *y;
{
    short sn1 = x->sn, sn2 = y->sn;
    if (x->sn == 0) {
        zero(r);
        return;
    }
    udiv(q,r,x,y);
    q->sn = sn1*sn2;
    r->sn = sn1;
    if (uzero(r))
        r->sn = 0;
}

void mpz_mmod(r,x,y)
MP_INT *r; MP_INT *x; MP_INT *y;
{
    MP_INT q;
    short sn1 = x->sn, sn2 = y->sn;
    mpz_init(&q);
    if (sn1 == 0) {
        zero(r);
        return;
    }
    udiv(&q,r,x,y);
    if (uzero(r)) {
        r->sn = 0;
        return;
    }
    q.sn = sn1*sn2;
    if (q.sn > 0) 
        r->sn = sn1;
    else if (sn1 < 0 && sn2 > 0) {
        r->sn = 1;
        mpz_sub(r,y,r);
    }
    else {
        r->sn = 1;
        mpz_add(r,y,r);
    }
}

void mpz_mdivmod(q,r,x,y)
MP_INT *q;MP_INT *r; MP_INT *x; MP_INT *y;
{
    short sn1 = x->sn, sn2 = y->sn;
    if (sn1 == 0) {
        zero(r);
        return;
    }
    udiv(q,r,x,y);
    q->sn = sn1*sn2;
    if (uzero(r)) {
        r->sn = 0;
        return;
    }
    if (q->sn > 0) 
        r->sn = sn1;
    else if (sn1 < 0 && sn2 > 0) {
        r->sn = 1;
        mpz_sub(r,y,r);
    }
    else {
        r->sn = 1;
        mpz_add(r,y,r);
    }
    /* now if r != 0 and q < 0 we need to round q towards -inf */
    if (!uzero(r) && q->sn < 0) 
        mpz_sub_ui(q,q,1);
}

void mpz_mod_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init(&z);
    mpz_set_ui(&z,n);
    mpz_mod(x,y,&z);
    mpz_clear(&z);
}

void mpz_mmod_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init(&z);
    mpz_set_ui(&z,n);
    mpz_mmod(x,y,&z);
    mpz_clear(&z);
}

void mpz_div_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_div(x,y,&z);
    mpz_clear(&z);
}

void mpz_mdiv_ui(x,y,n)
MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_mdiv(x,y,&z);
    mpz_clear(&z);
}
void mpz_divmod_ui(q,x,y,n)
MP_INT *q;MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_divmod(q,x,y,&z);
    mpz_clear(&z);
}

void mpz_mdivmod_ui(q,x,y,n)
MP_INT *q;MP_INT *x;MP_INT *y; unsigned long n;
{
    MP_INT z;
    mpz_init_set_ui(&z,n);
    mpz_mdivmod(q,x,y,&z);
    mpz_clear(&z);
}


/* 2<=base <=36 - this overestimates the optimal value, which is OK */
unsigned int mpz_sizeinbase(x,base)
MP_INT *x; int base;
{
    int i,j;
    int bits = digits(x) * DIGITBITS;
    if (base < 2 || base > 36)
        fatal("mpz_sizeinbase: invalid base");
    for (j=0,i=1; i<=base;i*=2,j++)
        ;
    return ((bits)/(j-1)+1);
}

char *mpz_get_str(s,base,x)
char *s;  int base; MP_INT *x; {
    MP_INT xx,q,r,bb;
    char *p,*t,*ps;
    int sz = mpz_sizeinbase(x,base);
    int d;
    if (base < 2 || base > 36)
        return s;
    t = (char *)malloc(sz+2);
    if (!t) 
        fatal("cannot allocate memory in mpz_get_str");
    if (!s) {
        s=(char *)malloc(sz+2);
        if (!s) 
            fatal("cannot allocate memory in mpz_get_str");
    }
    if (uzero(x)) {
        *s='0';
        *(s+1)='\0';
        return s;
    }
    mpz_init(&xx); mpz_init(&q); mpz_init(&r); mpz_init(&bb);
    mpz_set(&xx,x);
    mpz_set_ui(&bb,base);
    ps = s;
    if (x->sn < 0) {
        *ps++= '-';
        xx.sn = 1;
    }
    p = t;
    while (!uzero(&xx)) {
        udiv(&xx,&r,&xx,&bb);
        d = r.p[0];
        if (d < 10) 
            *p++  = (char) (r.p[0] + '0');
        else 
            *p++  = (char) (r.p[0] + -10 + 'a');
    }

    p--;
    for (;p>=t;p--,ps++)
        *ps = *p;
    *ps='\0';
    
    mpz_clear(&q); mpz_clear(&r); free(t);  
    return s;
}


int mpz_set_str(x,s,base)
MP_INT *x; char *s; int base;
{
    int l,i;
    int retval = 0;
    MP_INT t,m,bb;
    short sn;
    unsigned int k;
    mpz_init(&m);
    mpz_init(&t);
    mpz_init(&bb);
    mpz_set_ui(&m,1);
    zero(x);
    while (*s==' ' || *s=='\t' || *s=='\n')
        s++;
    if (*s == '-') {
        sn = -1; s++;
    }
    else
        sn = 1;
    if (base == 0) {
        if (*s == '0') {
            if (*(s+1) == 'x' || *(s+1) == 'X') {
                base = 16;
                s+=2;   /* toss 0x */
            }
            else {
                base = 8;
                s++;    /* toss 0 */
            }
        }
        else
            base=10;
    }
    if (base < 2 || base > 36)
        fatal("mpz_set_str: invalid base");
    mpz_set_ui(&bb,base);
    l=strlen(s);
    for (i = l-1; i>=0; i--) {
        if (s[i]==' ' || s[i]=='\t' || s[i]=='\n') 
            continue;
        if (s[i] >= '0' && s[i] <= '9')
            k = (unsigned int)s[i] - (unsigned int)'0';
        else if (s[i] >= 'A' && s[i] <= 'Z')
            k = (unsigned int)s[i] - (unsigned int)'A'+10;
        else if (s[i] >= 'a' && s[i] <= 'z')
            k = (unsigned int)s[i] - (unsigned int)'a'+10;
        else {
            retval = (-1);
            break;
        }
        if (k >= base) {
            retval = (-1);
            break;
        }
        mpz_mul_ui(&t,&m,k);
        mpz_add(x,x,&t);
        mpz_mul(&m,&m,&bb);
#ifdef DEBUG
        printf("k=%d\n",k);
        printf("t=%s\n",mpz_get_str(NULL,10,&t));
        printf("x=%s\n",mpz_get_str(NULL,10,x));
        printf("m=%s\n",mpz_get_str(NULL,10,&m));
#endif
    }
    if (uzero(&m))
        x->sn = 0;
    else 
        x->sn = sn;
    mpz_clear(&m);
    mpz_clear(&bb);
    mpz_clear(&t);
    return retval;
}

void mpz_init_set_str(x,s,base)
MP_INT *x; char *s; int base;
{
    mpz_init(x); mpz_set_str(x,s,base);
}

#define mpz_randombyte (rand()& 0xff)

void mpz_random(x,size)
MP_INT *x; unsigned int size;
{
    unsigned int bits = size * LONGBITS;
    unsigned int digits = bits/DIGITBITS;
    unsigned int oflow = bits % DIGITBITS;
    unsigned int i,j;
    long t;
    if (oflow)
        digits++;
    _mpz_realloc(x,digits);

    for (i=0; i<digits; i++) {
        t = 0;
        for (j=0; j < DIGITBITS; j+=8)
            t = (t << 8) | mpz_randombyte; 
        (x->p)[i] = t & LMAX;
    }
    if (oflow) 
        (x->p)[digits-1] &= ((1 << oflow) - 1);
}
void mpz_random2(x,size)
MP_INT *x; unsigned int size;
{
    unsigned int bits = size * LONGBITS;
    unsigned int digits = bits/DIGITBITS;
    unsigned int oflow = bits % DIGITBITS;
    unsigned int i,j;
    long t;
    if (oflow)
        digits++;
    _mpz_realloc(x,digits);

    for (i=0; i<digits; i++) {
        t = 0;
        for (j=0; j < DIGITBITS; j+=8)
            t = (t << 8) | mpz_randombyte; 
        (x->p)[i] = (t & LMAX) % 2;
    }
    if (oflow) 
        (x->p)[digits-1] &= ((1 << oflow) - 1);
}

size_t mpz_size(x)
MP_INT *x;
{
    int bits = (x->sz)*DIGITBITS;
    size_t r;
    
    r = bits/LONGBITS;
    if (bits % LONGBITS)
        r++;
    return r;
}

void mpz_abs(x,y)
MP_INT *x; MP_INT *y;
{
    if (x!=y)
        mpz_set(x,y);
    if (uzero(x))
        x->sn = 0;
    else
        x->sn = 1;
}

void mpz_neg(x,y)
MP_INT *x; MP_INT *y;
{
    if (x!=y)
        mpz_set(x,y);
    x->sn = -(y->sn);
}

void mpz_fac_ui(x,n)
MP_INT *x; unsigned long n;
{
    mpz_set_ui(x,1);
    if (n==0 || n == 1)
        return;
    for (;n>1;n--)
        mpz_mul_ui(x,x,n);
}

void mpz_gcd(gg,aa,bb)
MP_INT *gg;
MP_INT *aa;
MP_INT *bb;
{
    MP_INT *g,*t,*a,*b;
    int freeg;
    
    t = (MP_INT *)malloc(sizeof(MP_INT));
    g = (MP_INT *)malloc(sizeof(MP_INT));
    a = (MP_INT *)malloc(sizeof(MP_INT));
    b = (MP_INT *)malloc(sizeof(MP_INT));
    if (!a || !b || !g || !t)
        fatal("mpz_gcd: cannot allocate memory");
    mpz_init(g); mpz_init(t); mpz_init(a); mpz_init(b);
    mpz_abs(a,aa); mpz_abs(b,bb); 

    while (!uzero(b)) {
        mpz_mod(t,a,b);
        mpz_set(a,b);
        mpz_set(b,t);
    }
    
    mpz_set(gg,a);
    mpz_clear(g); mpz_clear(t); mpz_clear(a); mpz_clear(b);
    free(g); free(t); free(a); free(b);
}

void mpz_gcdext(gg,xx,yy,aa,bb)
MP_INT *gg,*xx,*yy,*aa,*bb;
{
    MP_INT *x, *y, *g, *u, *q;

    if (uzero(bb)) {
        mpz_set(gg,aa);
        mpz_set_ui(xx,1);
        if (yy)
            mpz_set_ui(yy,0);
        return;
    }
    
    g = (MP_INT *)malloc(sizeof(MP_INT)); mpz_init(g);
    q = (MP_INT *)malloc(sizeof(MP_INT)); mpz_init(q);
    y = (MP_INT *)malloc(sizeof(MP_INT)); mpz_init(y);
    x = (MP_INT *)malloc(sizeof(MP_INT)); mpz_init(x);
    u = (MP_INT *)malloc(sizeof(MP_INT)); mpz_init(u);

    if (!g || !q || !y || !x || !u)
        fatal("mpz_gcdext: cannot allocate memory");

    mpz_divmod(q,u,aa,bb);
    mpz_gcdext(g,x,y,bb,u);
    if (yy) {
        mpz_mul(u,q,y);
        mpz_sub(yy,x,u);
    }
    mpz_set(xx,y);
    mpz_set(gg,g);
    mpz_clear(g); mpz_clear(q); mpz_clear(y); mpz_clear(x); mpz_clear(u);
    free(g); free(q); free(y); free(x); free(u);
}


/*
 *    a is a quadratic residue mod b if
 *    x^2 = a mod b      has an integer solution x.
 *
 *    J(a,b) = if a==1 then 1 else
 *             if a is even then J(a/2,b) * ((-1)^(b^2-1)/8))
 *             else J(b mod a, a) * ((-1)^((a-1)*(b-1)/4)))
 *
 *  b>0  b odd
 *
 *
 */

int mpz_jacobi(ac,bc)
MP_INT *ac, *bc;
{
    int sgn = 1;
    unsigned long c;
    MP_INT *t,*a,*b; 
    if (bc->sn <=0)
        fatal("mpz_jacobi call with b <= 0");
    if (even(bc))
        fatal("mpz_jacobi call with b even");

    init(t); init(a); init(b);

    /* if ac < 0, then we use the fact that 
     *  (-1/b)= -1 if b mod 4 == 3
     *          +1 if b mod 4 == 1
     */

    if (mpz_cmp_ui(ac,0) < 0 && (lowdigit(bc) % 4) == 3)
        sgn=-sgn;

    mpz_abs(a,ac); mpz_set(b,bc);

    /* while (a > 1) { */
    while(mpz_cmp_ui(a,1) > 0) {

        if (even(a)) {

            /* c = b % 8 */
            c = lowdigit(b) & 0x07;

            /* b odd, then (b^2-1)/8 is even iff (b%8 == 3,5) */

            /* if b % 8 == 3 or 5 */
            if (c == 3 || c == 5)
                sgn = -sgn;

            /* a = a / 2 */
            mpz_div_2exp(a,a,1); 

        } 
        else {
            /* note: (a-1)(b-1)/4 odd iff a mod 4==b mod 4==3 */

            /* if a mod 4 == 3 and b mod 4 == 3 */
            if (((lowdigit(a) & 3) == 3) && ((lowdigit(b) & 3) == 3))
                sgn = -sgn;

            /* set (a,b) = (b mod a,a) */
            mpz_set(t,a); mpz_mmod(a,b,t); mpz_set(b,t);
        } 
    }

    /* if a == 0 then sgn = 0 */
    if (uzero(a))
        sgn=0;

    clear(t); clear(a); clear(b);
    return (sgn);
}

void mpz_and(z,x,y) /* not the most efficient way to do this */
MP_INT *z,*x,*y;
{
    int i,sz;
    sz = imax(x->sz, y->sz);
    _mpz_realloc(z,sz);
    for (i=0; i < sz; i++)
        (z->p)[i] = dg(x,i) & dg(y,i);
    if (x->sn < 0 && y->sn < 0)
        z->sn = (-1);
    else
        z->sn = 1;
    if (uzero(z))
        z->sn = 0;
}

void mpz_or(z,x,y)  /* not the most efficient way to do this */
MP_INT *z,*x,*y;
{
    int i,sz;
    sz = imax(x->sz, y->sz);
    _mpz_realloc(z,sz);
    for (i=0; i < sz; i++)
        (z->p)[i] = dg(x,i) | dg(y,i);
    if (x->sn < 0 || y->sn < 0)
        z->sn = (-1);
    else
        z->sn = 1;
    if (uzero(z))
        z->sn = 0;
}

void mpz_xor(z,x,y)  /* not the most efficient way to do this */
MP_INT *z,*x,*y;
{
    int i,sz;
    sz = imax(x->sz, y->sz);
    _mpz_realloc(z,sz);
    for (i=0; i < sz; i++)
        (z->p)[i] = dg(x,i) ^ dg(y,i);
    if ((x->sn <= 0 && y->sn > 0) || (x->sn > 0 && y->sn <=0))
        z->sn = (-1);
    else
        z->sn = 1;
    if (uzero(z))
        z->sn = 0;
}
void mpz_pow_ui(zz,x,e)
MP_INT *zz, *x;
unsigned long e;
{
    MP_INT *t;
    unsigned long mask = (1 << (LONGBITS-1));
    
    if (e==0) {
        mpz_set_ui(zz,1);
        return;
    }

    init(t);
    mpz_set(t,x);
    for (;!(mask &e); mask>>=1) 
        ;
    mask>>=1;
    for (;mask!=0; mask>>=1) {
        mpz_mul(t,t,t);
        if (e & mask)
            mpz_mul(t,t,x);
    }
    mpz_set(zz,t);
    clear(t);
}

void mpz_powm(zz,x,ee,n)
MP_INT *zz,*x,*ee,*n;
{
    MP_INT *t,*e;
    struct is *stack = NULL;
    int k,i;
    
    if (uzero(ee)) {
        mpz_set_ui(zz,1);
        return;
    }

    if (ee->sn < 0) {
        return;
    }
    init(e); init(t); mpz_set(e,ee);

    for (k=0;!uzero(e);k++,mpz_div_2exp(e,e,1))
        push(lowdigit(e) & 1,&stack);
    k--;
    i=pop(&stack);

    mpz_mod(t,x,n);  /* t=x%n */

    for (i=k-1;i>=0;i--) {
        mpz_mul(t,t,t); 
        mpz_mod(t,t,n);  
        if (pop(&stack)) {
            mpz_mul(t,t,x); 
            mpz_mod(t,t,n);
        }
    }
    mpz_set(zz,t);
    clear(t);
}

void mpz_powm_ui(z,x,e,n)
MP_INT *z,*x,*n; unsigned long e;
{
    MP_INT f;
    mpz_init(&f);
    mpz_set_ui(&f,e);
    mpz_powm(z,x,&f,n);
    mpz_clear(&f);
}
    
    

/* Miller-Rabin */
static int witness(x,n)
MP_INT *x, *n;
{
    MP_INT *t,*e, *e1;
    struct is *stack = NULL;
    int trivflag = 0;
    int k,i;
    
    init(e1); init(e); init(t); mpz_sub_ui(e,n,1); mpz_set(e1,e);

    for (k=0;!uzero(e);k++,mpz_div_2exp(e,e,1))
        push(lowdigit(e) & 1,&stack);
    k--;
    i=pop(&stack);

    mpz_mod(t,x,n);  /* t=x%n */

    for (i=k-1;i>=0;i--) {
        trivflag = !mpz_cmp_ui(t,1) || !mpz_cmp(t,e1);
        mpz_mul(t,t,t); mpz_mod(t,t,n);  
        if (!trivflag && !mpz_cmp_ui(t,1)) {
            clear(t); clear(e); clear(e1);
            return 1;
        }
            
        if (pop(&stack)) {
            mpz_mul(t,t,x); 
            mpz_mod(t,t,n);
        }
    }
    if (mpz_cmp_ui(t,1))  { /* t!=1 */
        clear(t); clear(e); clear(e1);
        return 1;
    }
    else {
        clear(t); clear(e); clear(e1);
        return 0;
    }
}

unsigned int smallp[] = {2,3,5,7,11,13,17};
int mpz_probab_prime_p(nn,s)
MP_INT *nn; int s;
{   
    MP_INT *a,*n;
    int j,k,i;
    long t;

    if (uzero(nn))
        return 0;
    init(n); mpz_abs(n,nn);
    if (!mpz_cmp_ui(n,1)) {
        clear(n);
        return 0;
    }
    init(a);
    for (i=0;i<sizeof(smallp)/sizeof(unsigned int); i++) {
        mpz_mod_ui(a,n,smallp[i]);
        if (uzero(a)) {
            clear(a); clear(n); return 0;
        }
    }
    _mpz_realloc(a,n->sz);
    for (k=0; k<s; k++) {

        /* generate a "random" a */
            for (i=0; i<n->sz; i++) {
                t = 0;
                for (j=0; j < DIGITBITS; j+=8)
                    t = (t << 8) | mpz_randombyte; 
                (a->p)[i] = t & LMAX;
            }
            a->sn = 1;
            mpz_mod(a,a,n);

        if (witness(a,n)) {
            clear(a); clear(n);
            return 0;
        }
    }
    clear(a);clear(n);
    return 1;
}   
