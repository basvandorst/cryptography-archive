
/* $Log:	gcd.c,v $
 * Revision 1.1  91/09/20  14:48:00  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:22  morain
 * Initial revision
 *  */

/* Various versions of pgcd 
   Last modified: 12/08/89 11h00 */

#include "mod.h"

#define NBITS   14 /* for Lehmer */
#define LEHMER  1000

/*=========================================================================
 .Section "Classical code"
=========================================================================*/

long LongGcdLong(a, b)
long a, b;
{
    long r;

    while(b){
	r = a % b; a = b; b = r;
    }
    return(a);
}

/*-------------------------------------------------------------------------
 .SSection "GCD"
-------------------------------------------------------------------------*/

/* we just want size(g) >= max(size(a), size(b)) */
/* g <- gcd(a, b), returns gl */
int BnnGcdWithCopy(g, a, al, b, bl)
BigNum g, a, b;
int al, bl;
{
    int gl;
    BigNum aa = BnCreate((BigNumType)1, al+1);
    BigNum bb = BnCreate((BigNumType)1, bl+1);
    
#ifdef DEBUG
    if(debuglevel >= 15) printf("(BnGcd) al=%d bl=%d\n", al, bl);
#endif
    BnnAssign(aa, a, al);
    BnnAssign(bb, b, bl);
    if(BnnCompare(a, al, b, bl) != -1){
	gl = BnnGcd(g, aa, al, bb, bl);
    }
    else{
	gl = BnnGcd(g, bb, bl, aa, al);
    }
    BnFree(aa);
    BnFree(bb);
#ifdef DEBUG
    if(debuglevel >= 10){printf("G="); BnnPrint(g, gl); printf("\n");}
#endif
    return(gl);
}

/* 
  g <- gcd(n, m); we must have size(n) > nl, size(m) > ml and n >= m; n and 
  m are destroyed; returns the number of digits of g.
*/
BnnGcd(g, n, nl, m, ml)
BigNum g, n, m;
int nl, ml;
{
    
    if(ml >= LEHMER)
	return(BnnGcdLehmer(g, n, nl, m, ml));
    else
	return(BnnGcdPlain(g, n, nl, m, ml));
}

/* 
  g <- gcd(n, m); we must have size(n) > nl, size(m) > ml and n >= m; n and 
  m are destroyed; returns the number of digits of g.
*/
BnnGcdPlain(g, n, nl, m, ml)
BigNum g, n, m;
int nl, ml;
{
    int tmpl;
    BigNum tmp;
    
    /* ordinary euclid */
    while(!BnnIsZero(m, ml)){
#ifdef DEBUG
	if(debuglevel >= 10){
	    printf("m="); BnnPrint(m, ml); printf("\n");
	    printf("n="); BnnPrint(n, nl); printf("\n");
	}
#endif
	/* division of n[0..nl-1][0] by m[0..ml-1] */
	BnnSetDigit((n+nl), 0);
	BnnDivide(n, nl+1, m, ml);
	tmpl = BnnNumDigits(n, ml);
	nl = ml; ml = tmpl; tmp = n;
	n = m; m = tmp;
    }
    BnnAssign(g, n, nl);
#ifdef DEBUG
    if(debuglevel >= 10){printf("g="); BnnPrint(g, nl); printf("\n");}
#endif
    return(nl);
}

/*-------------------------------------------------------------------------
 .SSection "Extended Euclidean algorithm"
-------------------------------------------------------------------------*/

/* Returns 1/m mod n. */
BigZ BzEea(m, n)
BigNum m, n;
{
    int nl = BnNumDigits(n, 0, BnGetSize(n));
    int ml = BnNumDigits(m, 0, BnGetSize(m));
    int al, gl;
    BigNum a = BNC(nl), g = BNC(nl);

    if(BnnEeaWithCopy(g, &gl, a, &al, n, nl, m, ml)) return((BigZ)a);
    else return(BzNegate(g));
}

/* 
  g[0..gl-1] <- gcd(n, m), a <- 1/m mod n (if g=1). Suppose that m < n
  and size(m) > ml. Returns 0 if g<>1 and 1 otherwise.
*/
BnnEeaWithCopy(g, p_gl, a, p_al, n0, nl, m0, ml)
BigNum g, a, n0, m0;
int *p_gl, *p_al, nl, ml;
{
    BigNum gg = BNC(nl+1), n = BNC(nl+1), m = BNC(nl+1);
    int inv;

    BnnAssign(n, n0, nl);
    BnnAssign(m, m0, ml);
    inv = BnnEea(gg, p_gl, a, p_al, n, nl, m, ml);
    BnnAssign(g, gg, *p_gl);
    BnFree(gg); BnFree(n); BnFree(m);
    return(inv);
}

BnnEea(g, p_gl, a, p_al, n, nl, m, ml)
BigNum g, a, n, m;
int *p_gl, *p_al, nl, ml;
{
    if(ml <= LEHMER)
	return(BnnEeaPlain(g, p_gl, a, p_al, n, nl, m, ml));
    else
	return(BnnEeaLehmer(g, p_gl, a, p_al, n, nl, m, ml));
}

/* m and n are destroyed; cannot have a = n or m (resp. g). g and a must
   have at least nl+1 digits. */
BnnEeaPlain(g, p_gl, a, p_al, n, nl, m, ml)
BigNum g, a, n, m;
int *p_gl, *p_al, nl, ml;
{
    BigNum aa = BNC(nl+1), b = BNC(nl+1), tm, bb;
    int d = 1, bl = 1, al = 1, ql, gl = nl, tmp, inv;
    
#ifdef DEBUG
    if(debuglevel >= 13){
	printf("m="); BnnPrint(m, ml); printf("\n");
	printf("n="); BnnPrint(n, nl); printf("\n");
    }
#endif
    BnnSetDigit(aa, 1);
    BnnAssign(g, n, nl);
    while(BnnIsGtOne(m, ml)){
	d = 0 - d;
	/* division of g[0..gl-1][0] by m[0..ml-1] */
	BnnSetDigit((g+gl), 0);
	/* g = q * m + r = g[0..ml-1][ml..gl] */
	BnnDivide(g, gl+1, m, ml);
	/* accumulation in aa */
	ql = 1 + gl - ml;
	bl = al + ql;
	/* b <- b + q * aa */
	BnnMultiply(b, bl, aa, al, (g+ml), ql);
	tmp = BnnNumDigits(g, ml); gl = ml; ml = tmp;
	tmp = BnnNumDigits(b, bl); bl = al; al = tmp;
	tm = g; g = m; m = tm;
	tm = b; b = aa; aa = tm;
    }
    /* m[0]=0 ou 1 */
    inv = 0;
    if(BnnGetDigit(m) == 1){
	/* there is an inverse */
	inv = 1;
	BnnSetDigit(g, 1);
	gl = 1;
	if(d == -1){
	    BnnComplement(aa, nl);
	    BnnAdd(aa, nl, n, nl, 1);
	    al = BnnNumDigits(aa, nl);
	}
#ifdef DEBUG
	if(debuglevel >= 13)
	    {printf("1/m mod n="); BnnPrint(aa, al); printf("\n");}
#endif
    }
    *p_gl = gl; *p_al = al;
    BnnAssign(a, aa, al);
    BnFree(aa); BnFree(b);
    return(inv);
}

/*=========================================================================
 .Section "Lehmer"
=========================================================================*/

/*-------------------------------------------------------------------------
 .SSection "Gcd"
-------------------------------------------------------------------------*/

/* 
   g[gd] <- gcd(n[nd..nd+nl-1], m[md..md+ml-1]; returns the number of
   digits of g. Assumes n >= m. 
*/
int BnnGcdLehmer(g, n, nl, m, ml)
BigNum g, n, m;
int nl, ml;
{
    int l = 2 + (nl > ml ? nl : ml);
    BigNumCmp cmp = BnnCompare(n, nl, m, ml);
    BigNum u = BNC(l), v = BNC(l);

    printf("Entering BnGcdLehmer\n");
    if(cmp == 1){
	BnnAssign(u, n, nl);
	BnnAssign(v, m, ml);
	return(GcdLehmer(g, u, nl, v, ml));
    }
    else{
	if(cmp == -1){
	    BnnAssign(u, m, ml);
	    BnnAssign(v, n, nl);
	    return(GcdLehmer(g, u, ml, v, nl));
	}
	else{
	    BnnAssign(g, n, nl);
	    return(nl);
	}
    }
}

/*
  g <- gcd(u, v); returns the number of digits of g. Assumes u >= v and so
  ul >= vl.
*/
GcdLehmer(g, u, ul, v, vl)
BigNum g, u, v;
int ul, vl;
{
    BigNumCmp s = 1;
    int sh = 0;
    BigNumDigit uc = 0, vc = 0;
    BigNumDigit a, b, c, d, tt, q, qq, vcc, vcd, FirstBits();
    BigNum tmp, ba = BNC(1);
    BigNum bt = BNC(ul+2), bw = BNC(ul+2), bz = BNC(ul+2); 

    BnnSetToZero(g, ul);
    while(!BnHasLessBits(v, vl, NBITS)){
	a = 1; b = 0; c = 0; d = 1; s = 1;
	if(vl >= ul-1){
	    sh = NumShift(u, ul, NBITS);
	    uc = FirstBits(u, ul, sh);
	    vc = FirstBits(v, ul, sh);
#ifdef DEBUG
	    printf("uc=%ld vc=%ld\n", uc, vc);
#endif
	    vcc = vc; vcd = vc + 1;
	    if(vcc && vcd){
		q  = (uc + s * a) / vcc;
		qq = (uc - s * b) / vcd;
#ifdef DEBUG
		printf("vcc=%ld vcd=%ld q=%ld qq=%ld\n", vcc, vcd, q, qq);
#endif
		while(q == qq){
		    tt = a + q * c; a = c; c = tt;
		    tt = b + q * d; b = d; d = tt;
		    tt = uc - q * vc; uc = vc; vc = tt;
		    s = -s;
		    vcc = vc - s * c;
		    vcd = vc + s * d;
		    if((!vcc) || (!vcd))
			break;
		    else{
			q  = (uc + s * a) / vcc;
			qq = (uc - s * b) / vcd;
#ifdef DEBUG
			printf("vcc=%ld vcd=%ld q=%ld qq=%ld\n",vcc,vcd,q,qq);
#endif
		    }
		}
	    }
	}
#ifdef DEBUG
	printf("b=%ld\n", b);
#endif
	if(!b){
	    /* one step of ordinary euclide */
	    BnnSetDigit((u+ul), 0);
	    BnnDivide(u, ul+1, v, vl);
	    tmp = u; u = v; v = tmp;
	    ul = vl; vl = BnnNumDigits(v, vl);
	    /* caution...*/
	    BnnSetDigit((u+ul), 0);
	    BnnSetDigit((v+vl), 0);
	}
	else{
	    if(s == 1)
		LehmerUpdate(u, ul, v, vl, a, b, c, d, bt, bw, bz, ba);
	    else
		LehmerUpdate(v, vl, u, ul, b, a, d, c, bt, bw, bz, ba);
	    tmp = u; u = bt; bt = tmp;
	    tmp = v; v = bw; bw = tmp;
	    ul = BnnNumDigits(u, ul);
	    vl = BnnNumDigits(v, vl);
	}
    }
    /* at this point, v is a digit */
    if(BnnIsZero(v, vl)){
	/* v = 0 */
	BnnAssign(g, u, ul);
	return(ul);
    }
    else{
	if(!BnHasLessBits(u, ul, NBITS)){
	    /* u is not a digit */
	    BnnSetDigit((u+ul), 0);
	    BnDivideDigit(u, 1, u, 0, u, 0, ul+1, v, 0);
	}
	uc = BnnGetDigit(u); vc = BnnGetDigit(v);
	while(vc){
	    tt = uc % vc; uc = vc; vc = tt;
	}
	BnnSetDigit(g, uc);
	return(1);
    }
}

/* bt <- a*u-b*v, bw <- d*v-c*u. Conditions are:
 ul >= vl >= ul-1, bt, bw, bz are of size >= ul+1, ba is of size 1.
*/
LehmerUpdate(u, ul, v, vl, a, b, c, d, bt, bw, bz, ba)
BigNum u, v, bt, bw, bz, ba;
int ul, vl;
BigNumDigit a, b, c, d;
{
    int l = (ul > vl) ? (ul+1) : (vl+1);
    
    /* bt <- a*u */
    BnnSetToZero(bt, l);
    BnnSetDigit(ba, a);
    BnnMultiplyDigit(bt, l, u, ul, *ba);
    /*Print("bt=", bt, 0, l);*/
    /* ; bz <- b*v */
    BnnSetToZero(bz, l);
    BnnSetDigit(ba, b);
    BnnMultiplyDigit(bz, l, v, vl, *ba);
    /*Print("bz=", bz, 0, l);*/
    /* ; bt <- bt - bz=a*u-b*v */
    BnnSubtract(bt, l, bz, l, 1);
    /* ; bw <- d*v */
    BnnSetToZero(bw, l);
    BnnSetDigit(ba, d);
    BnnMultiplyDigit(bw, l, v, vl, *ba);
    /*Print("bw=", bw, 0, l);*/
    /* ; bz <- c*u */
    BnnSetToZero(bz, l);
    BnnSetDigit(ba, c);
    BnnMultiplyDigit(bz, l, u, ul, *ba);
    /*Print("bz=", bz, 0, l);*/
    /* ; bw <- bw - bz=d*v-c*u */
    BnnSubtract(bw, l, bz, l, 1);
}

/* Calcule de combien il faut shifter u pour en avoir nbits bits.*/
int NumShift(u, ul, nbits)
BigNum u;
int ul, nbits;
{
    return(nbits + BnNumLeadingZeroBitsInDigit(u, ul-1)-BN_DIGIT_SIZE);
}

/* Returns the nbits first bits of u */
BigNumDigit FirstBits(u, ul, sh)
BigNum u;
int ul, sh;
{
    BigNumDigit C[1];

    if(sh <= 0){
	BnnAssign(C, (u+ul-1), 1);
	BnnShiftRight(C, 1, -sh);
	return(BnnGetDigit(C));
    }
    else{
	if(ul == 1)
	    return(BnnGetDigit(u));
	else{
	    BnnAssign(C, (u+ul-2), 2);
	    BnnShiftLeft(C, 2, sh);
	    /* result is in C[1] */
	    return(BnnGetDigit(C+1));
	}
    }
}

/* Returns 0 if u has more than nbits bits.*/
BnHasLessBits(u, ul, nbits)
BigNum u;
int ul, nbits;
{
    if((ul == 1) && (BnNumBitsInDigit(u, 0) <= nbits))
	return(1);
    else return(0);
}

/*-------------------------------------------------------------------------
 .SSection "Extended Euclidean Algorithm"
-------------------------------------------------------------------------*/

/* bt <- a*uu+b*vv, bw <- d*vv+c*uu. On a toujours vv >= uu.*/
#define LehmerEeaUpdate() {\
    /* bt <- a*uu */\
    BnnSetToZero(bt, l);\
    BnnSetDigit(ba, a);\
    BnnMultiplyDigit(bt, uul+1, uu, uul, *ba);\
    /* bt <- bt + b*vv */\
    BnnSetDigit(ba, b);\
    BnnMultiplyDigit(bt, vvl+2, vv, vvl, *ba);\
    /*Print("bt=", bt, 0, l);*/\
    /* bw <- d * vv */\
    BnnSetToZero(bw, l);\
    BnnSetDigit(ba, d);\
    BnnMultiplyDigit(bw, vvl+1, vv, vvl, *ba);\
    /* bw <- bw + c * uu */\
    BnnSetDigit(ba, c);\
    /* bw a au plus vvl+1 <= l chiffres, c*uu au plus uul+1 <= l */\
    BnnMultiplyDigit(bw, vvl+2, uu, uul, *ba);\
    /*Print("bw=", bw, 0, l);*/\
}

/* 
  g <- gcd(u, v), a <- 1/v mod u, with u > v and size(u), size(v) >= ul+2, 
  vl+2. Everything destroyed ?
  */
BnnEeaLehmer(g, p_gl, ia, p_ial, u, ul, v, vl)
BigNum g, ia, u, v;
int *p_gl, *p_ial, ul, vl;
{
    BigNumCmp s = 1, ss = 1;
    int sh = 0, uul = 1, vvl = 1, ql = 0, btl = 0, l = ul + 1;
    BigNumDigit uc, vc;
    BigNumDigit a, b, c, d, tt, q, qq, vcc, vcd;
    BigNum tmp, ba = BNC(1);
    BigNum bt = BNC(ul+2), bw = BNC(ul+2), bz = BNC(ul+2); 
    BigNum uu = BNC(ul+2); BigNum vv = BNC(ul+2); 
    
    BnnAssign(ia, u, ul);
    BnnSetToZero(g, ul);
    BnnSetDigit(vv, 1);
    while(BnnIsGtOne(v, vl)){
	a = 1; b = 0; c = 0; d = 1; s = 1;
	if(vl >= ul-1){
	    sh = NumShift(u, ul, NBITS);
	    uc = FirstBits(u, ul, sh);
	    vc = FirstBits(v, ul, sh);
#ifdef DEBUG
	    printf("uc=%ld vc=%ld\n", uc, vc);
#endif
	    vcc = vc; vcd = vc + 1;
	    if(vcc && vcd){
		q  = (uc + s * a) / vcc;
		qq = (uc - s * b) / vcd;
#ifdef DEBUG
		printf("vcc=%ld vcd=%ld q=%ld qq=%ld\n", vcc, vcd, q, qq);
#endif
		while(q == qq){
		    tt = a + q * c; a = c; c = tt;
		    tt = b + q * d; b = d; d = tt;
		    tt = uc - q * vc; uc = vc; vc = tt;
		    s = -s;
		    vcc = vc - s * c;
		    vcd = vc + s * d;
		    if((!vcc) || (!vcd))
			break;
		    else{
			q  = (uc + s * a) / vcc;
			qq = (uc - s * b) / vcd;
#ifdef DEBUG
			printf("vcc=%ld vcd=%ld q=%ld qq=%ld\n",vcc,vcd,q,qq);
#endif
		    }
		}
	    }
	}
	/* go on */
#ifdef DEBUG
	printf("b=%ld\n", b);
#endif
	if(!b){
	    /* one step of plain Euclide */
	    ss = - ss;
	    BnnSetDigit((u+ul), 0);
	    BnnDivide(u, ul+1, v, vl);
	    /* q=u[vl..ul], r=u[0..vl-1] */
	    ql = BnnNumDigits((u+vl), ul-vl+1);
	    /* updating uu and vv */
	    btl = ql + vvl + 1;
	    BnnSetToZero(bt, btl);
	    /* bt <- uu+q*vv  has at most ql+vvl+1 digits */
	    BnnAssign(bt, uu, uul);
	    BnnMultiply(bt, btl, vv, vvl, (u+vl), ql);
	    /* uu <- vv, vv <- bt */
	    uul = vvl; vvl = BnnNumDigits(bt, btl);
	    tmp = uu; uu = vv; vv = bt; bt = tmp;
	    /* updating u and v */
	    tmp = u; u = v; v = tmp;
	    ul = vl; vl = BnnNumDigits(v, vl);
	    /* prudence... */
	    BnnSetDigit((u+ul), 0);
	    BnnSetDigit((v+vl), 0);
	}
	else{
	    /* updatings variables */
	    if(s == 1)
		LehmerUpdate(u, ul, v, vl, a, b, c, d, bt, bw, bz, ba);
	    else{
		LehmerUpdate(v, vl, u, ul, b, a, d, c, bt, bw, bz, ba);
		ss = - ss;
	    }
	    tmp = u; u = bt; bt = tmp;
	    tmp = v; v = bw; bw = tmp;
	    ul = BnnNumDigits(u, ul);
	    /*Print("newu=", u, 0, ul);*/
	    vl = BnnNumDigits(v, vl);
	    /*Print("newv=", v, 0, vl);*/
/*	    LehmerEeaUpdate(uu, uul, vv, vvl, a, b, c, d, bt, bw, ba, l);*/
	    LehmerEeaUpdate();
	    /* test bizarre sur les longueurs... */
	    vvl = (vvl+2) <= l ? (vvl+2) : l;
	    uul = BnNumDigits(bt, 0, vvl);
	    tmp = uu; uu = bt; bt = tmp;
	    /*Print("newuu=", uu, 0, uul);*/
	    vvl = BnNumDigits(bw, 0, vvl);
	    tmp = vv; vv = bw; bw = tmp;
	    /*Print("newvv=", vv, 0, vvl);*/
	}
    }
    /* on conclut */
    if(BnnIsDigitZero(*v)){
	/* ; pgcd non trivial -> pb */
	BnnAssign(g, u, ul);
	*p_gl = ul;
	return(0);
    }
    else{
	/* g = 1 */
	BnnSetDigit(g, 1);
	*p_gl = 1;
	if(ss == -1){
	    /* ia contient de'ja` u, de longueur l-1, l'inverse est ia-vv*/
	    BnnSubtract(ia, l-1, vv, vvl, 1);
	    *p_ial = BnnNumDigits(ia, l-1);
	    return(1);
	}
	else{
	    BnnAssign(ia, vv, vvl);
	    *p_ial = vvl;
	    l = l - vvl - 1;
	    if(l > 0)
		BnnSetToZero((ia+vvl), l);
	    return(1);
	}
    }
}

/* Solving a u + b v = q in (u, v), with q in {-1, 0, 1} */
/* a and b can be destroyed */
/* Code temporaire... */
int Bezout(u, v, a, b, q)
BigZ *u, *v;
BigZ a, b;
int q;
{
    BigNum na, nb, nu;
    BigZ un = BzFromInteger(1), c;
    int nul, gl, bl, signofb = 1, signofa = 1, sign = 1;

#ifdef DEBUG
    if(debuglevel >= 10){
	printf("Solving %s * u", BzToString(a, 10));
	printf("+ %s * v = %d\n", BzToString(b, 10), q);
    }
#endif
    if(!q){
	/* u = -b, v = a */
	*u = BzNegate(b);
	*v = BzCopy(a);
    }
    else{
	/* we want a > 0 */
	if(BzGetSign(a) == -1){
	    BzSetSign(a, 1); 
	    signofa = -1;
	    signofb = BzGetSign(b); 
	    BzSetSign(b, -signofb); 
	    q = -q;
	}
	/* first solve a * u + b * v = 1 */
	/* compute 1/a mod |b| */
	if(BzGetSign(b) == -1){
	    BzSetSign(b, 1);
	    sign = -1;
	}
	if(BzCompare(a, b) != -1)
	    c = BzMod(a, b);
	else c = BzCopy(a);
	BzSetSign(b, sign);
	bl = BzNumDigits(b);
	nu = BNC(bl);
	na = BzToBn(c); nb = BzToBn(b);
	BnnEeaWithCopy(Gbuf,&gl,nu,&nul,BzToBn(b),bl,BzToBn(c),BzNumDigits(c));
#ifdef DEBUG
	if(debuglevel >= 10) printf("nu=%s\n", From(nu, nul));
#endif
	*u = BzFromBigNum(nu, nul);
	/* v <- -(a*u-1)/b */
	*v = BzMultiply(a, *u);
	*v = BzSubtract(*v, un);
	*v = BzDiv(*v, b);
	BzSetSign(*v, -BzGetSign(*v));
	if(q == -1){
	    BzSetSign(*u, -BzGetSign(*u));
	    BzSetSign(*v, -BzGetSign(*v));
	}
	if(signofa == -1){
	    /* this proves that we have altered both a and b */
	    BzSetSign(a, signofa);
	    BzSetSign(b, signofb);
	}
	BzFree(c);
    }
    BzFree(un);
}
