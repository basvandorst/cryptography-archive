
/* $Log:	mod.c,v $
 * Revision 1.1  91/09/20  14:48:11  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  13:56:30  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  13:54:35  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  13:53:41  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  13:46:02  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  13:44:35  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  13:41:11  morain
 * Initial revision
 *  */

/**********************************************************************
                           Modular arithmetic
**********************************************************************/


/* Last modified: 16/08 15h00 */

#include "mod.h"

BigNum CONSTANTES;
BigNum SCRATCH;
int debuglevel = 0;

#ifdef CHECK
BmCheck(n, s)
BigNum n;
char s[];
{
    if(BnGetSize(n) != (Modul+1)){
	fprintf(stderr, "%%!%% Error in %s\n", s);
	exit(0);
    }
}
#endif

/* Creation of a BigNum */
BigNum BNC(l)
int l;
{
#ifdef DEBUG
    if(debuglevel >= 15) printf("I create a BigNum of size %d\n", l);
#endif
    return(BnCreate((BigNumType)1, l));
}

BigNum BnConvert(n, nd, nl)
BigNum n;
int nd, nl;
{
    int ml = BnNumDigits(n, nd, nl);
    BigNum m = BnCreate((BigNumType)1, ml);
    
    BnAssign(m, 0, n, nd, ml);
    return(m);
}

BigNum BnCopy(n)
BigNum n;
{
    int nl = BnNumDigits(n, 0, BnGetSize(n));
    BigNum m = BNC(nl);
    
    BnAssign(m, 0, n, 0, nl);
    return(m);
}

BigNumCmp FixCompareBn(f, n, nl)
BigNum n;
BigNumDigit f;
int nl;
{
    BnSetDigit(SCRATCH, 0, f);
    return(BnCompare(SCRATCH, 0, 1, n, 0, nl));
}

BmFree(n)
BigMod n;
{
#ifdef CHECK
    BmCheck(n, "BmFree");
#endif
    BnFree(n);
}

/*===========================================================================
 .Section "About buffers"
===========================================================================*/

/*===========================================================================
 .Section "Calculs Modulaires"
===========================================================================*/

/*
    La philosophie est la suivante. On met le modulo dont on se sert toujours
  dans la variable #:mod:M (#:mod:Ml pour son nombre de chiffres). Toutes
  les ope'rations, a` savoir MdAdd, ... se font implicitement avec ce modulo.
  On change le modulo par (MdInitialize newmodulo), ou` newmodulo est un 
  entier quelconque (y compris un fix qui est automatiquement boxe').
     Un "entier modulaire" est cre'e par (BmCreate) et n'est rien d'autre
  qu'un bignum de longueur :Ml+1.
*/

/*---------------------------------------------------------------------------
 .SSection "Les tampons de calcul"
---------------------------------------------------------------------------*/

/* Le modulo.*/
BigNum Modu;
int    Modul;

/* Est utilise' dans MdMultiply.*/
BigNum Mbuf;
int Mtl;

/* Est utilise' dans MdExp.*/
BigNum Ebuf;
int Etl;

/* Cre'ation.*/
BigNum Abuf, Dbuf, Gbuf, Xbuf, Ybuf, Hbuf, Pbuf, Zbuf;

/* Filling CONSTANTES */
ModInit()
{
    if(SCRATCH == NULL) SCRATCH = BNC(1);
    if(CONSTANTES == NULL){
	CONSTANTES = BNC(20);
	BnSetDigit(CONSTANTES, 0, 0);
	BnSetDigit(CONSTANTES, 1, 1);
	BnSetDigit(CONSTANTES, 2, 2);
	BnSetDigit(CONSTANTES, 3, 3);
	BnSetDigit(CONSTANTES, 4, 4);
	BnSetDigit(CONSTANTES, 6, 6);
	BnSetDigit(CONSTANTES, 7, 7);
	BnSetDigit(CONSTANTES, 8, 8);
	BnSetDigit(CONSTANTES, 16, 16);
	BnSetDigit(CONSTANTES, 19, 64);
    }
}

ModClose()
{
    if(Gbuf != NULL) BnFree(Gbuf);
    if(Dbuf != NULL) BnFree(Dbuf);
    if(Pbuf != NULL) BnFree(Pbuf);
    if(Xbuf != NULL) BnFree(Xbuf);
    if(Ybuf != NULL) BnFree(Ybuf);
    if(Abuf != NULL) BnFree(Abuf);
    if(Zbuf != NULL) BnFree(Zbuf);
    if(Hbuf != NULL) BnFree(Hbuf);
}

/* Updating all the buffers */
BufInit(nl)
int nl;
{
    int gl = 2*(nl+1), al = 2 * nl, tl = nl + 1;
    
    Gbuf = BufUpdate(Gbuf, gl);
    Dbuf = BufUpdate(Dbuf, gl);
    Pbuf = BufUpdate(Pbuf, gl);
    Xbuf = BufUpdate(Xbuf, gl);
    Ybuf = BufUpdate(Ybuf, gl);
    Abuf = BufUpdate(Abuf, al);
    Zbuf = BufUpdate(Zbuf, tl);
    Hbuf = BufUpdate(Hbuf, tl);
}
	      
/*---------------------------------------------------------------------------
 .SSection "Pre'paration aux calculs"
---------------------------------------------------------------------------*/

/* Updates Modu, Modul; Mbuf, Mtl (for *mod); Ebuf, Etl (for **mod) */
MdInitialize(module)
BigNum module;
{
    int tl = 0;
    
    Modu  = BnCopy(module);
    Modul = BnGetSize(Modu);
    tl = 1 + 2 * Modul;
    Mbuf = BufUpdateL(Mbuf, tl, &Mtl);
    Ebuf = BufUpdateL(Ebuf, tl, &Etl);
}

MdFree()
{
    if(Modu != NULL) BnFree(Modu);
}

BmInit(module)
BigNum module;
{
    MdFree();
    MdInitialize(module);
    ModInit();
    BufInit(Modul);
    /* and also the random number generator */
    srandom(1);
}

/* Creation and update of a buffer */
BigNum BufUpdate(buf, bufl)
BigNum buf;
int bufl;
{
    if(!buf)
	return(BNC(bufl));
    else{
	if(bufl > BnGetSize(buf)){
	    BnFree(buf);
	    return(BNC(bufl));
	}
	else{
	    BnnSetToZero(buf, BnGetSize(buf));
	    return(buf);
	}
    }
}

/* Another version */
BigNum BufUpdateL(buf, newbufl, p_bufl)
BigNum buf;
int newbufl, *p_bufl;
{
    BigNum newbuf = BufUpdate(buf, newbufl);

    *p_bufl = newbufl;
    return(newbuf);
}

/* 
 .SSection "Cre'ation d'un entier, dit modulaire"
*/

BigMod MdFix(x)
BigNumDigit x;
{
    BigMod n = BmCreate();
    
    BnnSetDigit(n, x);
    return(n);
}

/* n <- m, et on padde avec des ze'ros.*/
MdAssign(n, m, ml)
BigMod n, m;
int ml;
{
    BnnAssign(n, m, ml);
    BnnSetToZero((n+ml), Modul - ml + 1);
}        

#if 0
/* ---------------------------------------------------------------------------*/
/* .SSection "Division acce'le're'e"*/
/* ---------------------------------------------------------------------------*/

(defvar :D (BnCreate '#:R:Q:N 1))
(defvar :normalisateur 0)
/* Contient les quotients approche's dans la division longue.*/
(defvar :qapp (BnCreate '#:R:Q:N 2))

/* Sauvegarde le chiffre le plus significatif du diviseur.*/
(defvar :Cdiv (BnCreate '#:R:Q:N 1))

(de IniDivide ()
    (unless (eqn :Modul 1)
	    (setq :normalisateur 
                 (BnNumLeadingZeroBitsInDigit :M (sub1 :Ml)))
	    (BnAssign :D 0 :M 0 :Ml)
            (BnShiftLeft :D 0 :Dl :qapp 0 :normalisateur)
            (BnAssign :Base-D 0 :D 0 :Dl)
            (Base-N :Base-D 0 :Ml)))

/* Impose a[al-1] < :M[:Ml-1]*/
(de MdMod (a ad al)
    ; a est multiplie' par 2**norm, et occupe toujours al mots.
    ; apres ce decalage, le chiffre al de a est
    ; toujours inferieur au chiffre :Ml de :m.
    (BnShiftLeft a ad al :qapp 0 :normalisateur)

    (:Ndivide n nd nl d dd dl)

    ; On divise n et d par 2**norm, les bits sortant
    ; a gauche sont necessairement nuls.
    (BnShiftRight a ad :Ml :qapp 0 :normalisateur))

/* Quotient de n[nd..nd+nl-1] par d[dd..dd+dl-1], avec le*/
/* modulo retourne' dans n[nd..nd+dl-1] et*/
/* le quotient dans n[nd+dl..nd+nl-1].*/
/* .br*/
/* On suppose que n[nd+nl-1] < d[dd+dl-1] et nl > dl.*/
/* .br*/
/* Une e'tape e'le'mentaire est la division de dl+1 chiffres de n par*/
/* les dl chiffres de d.*/
(de :Ndivide (n nd nl)
    ; Sauvegarde du chiffre significatif de :D:
    (BnAssign :Cdiv 0 :D (sub1 :Ml) 1)
    ; nl devient le rang du dernier chiffre courant de n
    (setq nl (add nd nl))
    (let ((ni (sub  nl :Ml)))
         (while (ge (decr1 ni) nd)
                ; Calcul du quotient approche':
                (if (eq 0 (BnCompareDigits n (decr1 nl) :Cdiv 0))
                    (BnAssign :qapp 0 :C-1 0 1)
                    (BnDivideDigit :qapp 0 :qapp 1 n (sub1 nl) 2 :Cdiv 0))
                ; Calcul du reste
                (BnMultiplyDigit n ni (add1 :Ml) d dd :Ml :qapp 0)
                (while  (<> (BnCompareDigits n nl :qapp 0) 0)
/*                         (print "Coucou")*/
                        ; Correction de quotient
                        (BnAdd n ni (add1 :Ml) :Base-D 0 :Ml 0)
                        (C-1 n nl)
                        (C-1 :qapp 0)))))
#endif

/* ---------------------------------------------------------------------------
 .SSection "Modular Multiplication"
 ---------------------------------------------------------------------------*/

/* p <- n * m mod Modu; p can be either n or m if its size is ok. */
BmMultiply(p, p_pl, n, nl, m, ml)
BigMod p, n, m;
int *p_pl, nl, ml;
{
    int tl = 0;
    
#ifdef CHECK
    BmCheck(p, "BmMultiply");
    BmCheck(n, "BmMultiply");
    BmCheck(m, "BmMultiply");
#endif
    BnnSetToZero(Mbuf, Mtl);
    /* Mbuf <- n * m */
    BnnMultiply(Mbuf, Mtl, n, nl, m, ml);
    tl = BnnNumDigits(Mbuf, Mtl);
    /* divide if Mbuf > Modu */
    if(BnnCompare(Mbuf, tl, Modu, Modul) != -1){
	BnnDivide(Mbuf, tl + 1, Modu, Modul);
	tl = BnnNumDigits(Mbuf, Modul);
    }
    /* back into p */
    BnnSetToZero(p, Modul + 1);
    BnnAssign(p, Mbuf, tl);
    *p_pl = tl;
}

/* p <- n^2 mod Modu, faster than BmMultiply ...*/
BmSquare(p, p_pl, n, nl)
BigMod p, n;
int *p_pl, nl;
{
    int tl = 0;

#ifdef CHECK
    BmCheck(p, "BmSquare");
#endif
    /* Mt <- n^2 */
    BnSquare(Mbuf, Mtl, n, nl);
    tl = BnnNumDigits(Mbuf, Mtl);
    /* divide if Mbuf > Modu */
    if(BnnCompare(Mbuf, tl, Modu, Modul) != -1){
	BnnDivide(Mbuf, tl + 1, Modu, Modul);
	tl = BnnNumDigits(Mbuf, Modul);
    }
    /* back into p */
    BnnSetToZero(p, Modul + 1);
    BnnAssign(p, Mbuf, tl);
    *p_pl = tl;
}

/*==========================================================================
 .Section "Fast Multiplications"
===========================================================================*/

/* Warning: in fact BnSquarePlain */
BnSquare(s, sl, n, nl)
BigMod s, n;
int sl, nl;
{
#ifdef DEBUG
    if(debuglevel >= 15){
	printf("s[%d]=%s\n", sl, From(s, sl));
	printf("n[%d]=%s\n", nl, From(n, nl));
    }
#endif
    BnSetToZero(s, 0, sl);
#ifdef DEBUG
    if(debuglevel >= 15)
	printf("s[%d]=%s\n", sl, From(s, sl));
#endif
    BnMultiply(s, 0, (nl << 1), n, 0, nl, n, 0, nl);
#ifdef DEBUG
    if(debuglevel >= 15) printf("s^2[%d,%d]=%s\n",sl,nl,From(s, sl));
#endif
}

BnSquareHack(s, sl, n, nl)
BigNum s, n;
int sl, nl;
{
    int i = 0, j = 0, l, ll, si;
    BigNum C = BNC(1);
    BigNumCarry b = 0;
    
    /* s <- 0 */
    BnSetToZero(s, 0, sl);
    sl = nl << 1;
    /* s <- sum(ni^2) */
    for(i = 0; i < nl; i++)
	/* s[2*i..2*i+1] <- n[i]*n[i] */
	BnMultiplyDigit(s, (i << 1), 2, n, i, 1, n, i);
    /* s <- s/2 */
    BnShiftRight(s, 0, sl, C, 0, 1);
    if(!BnIsDigitZero(C, 0))
	b = 1;
    /* somme double */
    l = nl - 1; ll = sl - 1; si = 1;
    for(i = 0; i < nl; i++){
	/* s[2i+1..2nl-1] <- s[2i+1..2nl-1]+n[i]*n[i+1..nl-1] */
	BnMultiplyDigit(s, si, ll--, n, i + 1, l--, n, i);
	si += 2;
    }
    /* s <- s*2+C */
    BnAdd(s, 0, sl, s, 0, sl, b);
}

/********** Addition **********/

/* n <- n + m mod Modu. Returns the number of digits of n. */
int BmAdd(n, p_nl, m, ml)
BigMod n, m;
int *p_nl, ml;
{
    int cmp, nl = *p_nl;

#ifdef CHECK
    BmCheck(n, "BmAdd");
    BmCheck(m, "BmAdd");
#endif
    /* clear n[nl..Modul] */
    BnnSetToZero((n+nl), Modul-nl+1);
    nl = Modul+1;
    BnnAdd(n, nl, m, ml, 0);
    nl = BnnNumDigits(n, nl);
    cmp = BnnCompare(n, nl, Modu, Modul);
    if(!cmp){
	BnnSetToZero(n, nl);
	*p_nl = 1;
    }
    else{
	if(cmp == 1){
	    BnnSubtract(n, nl, Modu, Modul, 1);
	    *p_nl = BnnNumDigits(n, Modul);
	}
	else *p_nl = nl;
    }
}

/********** Subtraction **********/

/* n <- n - m modulo Modu */
BmSubtract(n, p_nl, m, ml)
BigNum n, m;
int *p_nl, ml;
{
    int cmp, nl;

#ifdef CHECK
    BmCheck(n, "BmSubtract");
    BmCheck(m, "BmSubtract");
#endif
    /* updating length */
    nl = BnnNumDigits(n, *p_nl);
    ml = BnnNumDigits(m, ml);
    cmp = BnnCompare(n, nl, m, ml);
    if(cmp == 1){
	BnnSubtract(n, nl, m, ml, 1);
	*p_nl = BnnNumDigits(n, nl);
    }
    else{
	if(!cmp){
	    BnnSetToZero(n, nl);
	    *p_nl = 1;
	}
	else{
	    nl = Modul+1;
	    BnnAdd(n, nl, Modu, Modul, 0);
	    BnnSubtract(n, nl, m, ml, 1);
	    *p_nl = BnnNumDigits(n, nl);
	}
    }
}

#if 0
/* ---------------------------------------------------------------------------*/
/* .SSection "Division modulaire acce'le're'e"*/
/* ---------------------------------------------------------------------------*/

/* n <- n/a mod Modu, avec a un (petit) fix.*/
(de MdFastInv (n nd nl a)
    (selectq a
       (1 nl)
       (2 (BmShiftRight n nd nl))
       (4 (repeat 2 (setq nl (BmShiftRight n nd nl))))))

#endif
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/* .SSection "Division par une puissance de 2"*/
/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

/* n <- n / 2^nbits mod Modu. If n is even, n/2=Shr(n,1), otherwise
 n/2=Shr(n+M, 2). */
int BmShiftRight(n, nl, nbits)
BigNum n;
int nl, nbits;
{
    int i;

#ifdef CHECK
    BmCheck(n, "BmShiftRight");
#endif
    for(i = 0; i < nbits; i++){
	if(BnnIsDigitOdd(*n)){
	    /* n is odd, n <- n+M */
	    if(BnnAdd(n, Modul, Modu, Modul, 0))
		BnnSetDigit((n+Modul), 1);
	}
	BnnShiftRight(n, Modul+1, 1);
    }
    return(BnnNumDigits(n, Modul));
}

#if 0
/* Modulo les petites puissances de 2.*/
(de BnModPowerOfTwo (p pd s)
    (let ((C (BnCreate '#:R:Q:N 2)))
	 (BnAssign C 0 p pd 1)
	 (BnShiftLeft C 0 1 C 1 s)
	 (BnGetDigit C 1)))
    
/* ===========================================================================*/
/* .Section "Vers une nouvelle arithme'tique modulaire"*/
/* ===========================================================================*/

/* Toute re'fe'rence au module est supprime'e.*/
#endif

/* a <- a+b.*/
int ModAdd(a, b)
BigNum a, b;
{
    int al = Modul + 1;

#ifdef CHECK
    BmCheck(a, "ModAdd");
    BmCheck(b, "ModAdd");
#endif
    BnnAdd(a, al, b, Modul, 0);
    al = BnnNumDigits(a, al);
    if(BnnCompare(a, al, Modu, Modul) != -1)
	BnnSubtract(a, al, Modu, Modul, 1);
    return(BnnNumDigits(a, al));
}

/* a <- a+b[bd].*/
int ModAddDigit(a, b, bd)
BigMod a;
BigNum b;
int bd;
{
    int al = Modul+1;

#ifdef CHECK
    BmCheck(a, "ModAddDigit");
#endif
    BnnAdd(a, al, (b+bd), 1, 0);
    al = BnnNumDigits(a, al);
    if(BnnCompare(a, al, Modu, Modul) != -1){
	BnnSubtract(a, al, Modu, Modul, 1);
	al = BnnNumDigits(a, Modul);
    }
    return(al);
}

/* a <- a-b.*/
int ModSubtract(a, b)
BigMod a, b;
{
    int al = BnnNumDigits(a, Modul), bl = BnnNumDigits(b, Modul);

#ifdef CHECK
    BmCheck(a, "ModSubtract");
    BmCheck(b, "ModSubtract");
#endif
    if(BnnCompare(a, al, b, bl) == -1){
	al = Modul + 1;
	BnnAdd(a, al, Modu, Modul, 0);
	if(BnnIsDigitZero(*(a+Modul)))
	    al--;
    }
    BnnSubtract(a, al, b, bl, 1);
    return(BnnNumDigits(a, al));
}

int ModSubtractDigit(a, b, bd)
BigNum a, b;
int bd;
{
    int al = BnnNumDigits(a, Modul);

#ifdef CHECK
    BmCheck(a, "ModSubtractDigit");
#endif
    if(BnnCompare(a, al, (b+bd), 1) == -1){
	al = Modul + 1;
	BnnAdd(a, al, Modu, Modul, 0);
	if(BnnIsDigitZero(*(a+Modul))) al--;
    }
    BnnSubtract(a, al, (b+bd), 1, 1);
    return(BnnNumDigits(a, al));
}

/* n <- -n, assuming that 0 <= n < Modu; uses Zbuf.*/
int ModNegate(n)
BigMod n;
{
#ifdef CHECK
    BmCheck(n, "ModNegate");
#endif
    if(!BnnIsZero(n, BnnNumDigits(n, Modul))){
	BnnAssign(Zbuf, n, Modul);
	BnnAssign(n, Modu, Modul);
	BnnSubtract(n, Modul, Zbuf, Modul, 1);
    }
    return(BnnNumDigits(n, Modul));
}

/* p est un Mod, p[0..Ml+1] <- n*m mod Modu = p[0..Ml-1][0].*/
int ModMultiply(p, n, m)
BigNum p, n, m;
{
    int tl = 0;

#ifdef CHECK
    BmCheck(p, "ModMultiply");
    BmCheck(n, "ModMultiply");
    BmCheck(m, "ModMultiply");
#endif
    /* clear Mbuf*/
    BnnSetToZero(Mbuf, Mtl);
    /* Mt <- n*m */
    BnnMultiply(Mbuf, Mtl, n, Modul, m, Modul);
    tl = BnnNumDigits(Mbuf, Mtl);
    /* division only when Mbuf > Modu*/
    if(BnnCompare(Mbuf, tl, Modu, Modul) != -1)
	BnnDivide(Mbuf, tl+1, Modu, Modul);
    BnnSetDigit((p+Modul), 0);
    BnnAssign(p, Mbuf, Modul);
}

/* p <- p*n[nd].*/
ModMultiplyDigit(p, n, nd)
BigNum p, n;
int nd;
{
    int tl, pl = Modul+1;

#ifdef CHECK
    BmCheck(p, "ModMultiplyDigit");
#endif
    /* clearing Mbuf */
    BnnSetToZero(Mbuf, Mtl);
    /* Mt <- p*n[nd] */
    BnnMultiplyDigit(Mbuf, pl, p, Modul, *(n+nd));
    tl = BnnNumDigits(Mbuf, pl);
    /* on divise si Mbuf > Modu */
    if(BnnCompare(Mbuf, tl, Modu, Modul) != -1)
	/* division de Mbuf par Modu */
	BnnDivide(Mbuf, tl+1, Modu, Modul);
    /* retour dans p */
    BnnSetDigit((p+Modul), 0);
    BnnAssign(p, Mbuf, Modul);
}

/* Reduces n modulo Modu */
int ModMod(n, nl)
BigNum n;
int nl;
{
    if(BnnCompare(n, nl, Modu, Modul) != -1){
	/* n >= Modu */
	BnnSetDigit((n+nl), 0);
	nl++;
	BnnDivide(n, nl, Modu, Modul);
	BnnSetToZero((n+Modul), nl-Modul);
	nl = BnnNumDigits(n, Modul);
    }
    return(nl);
}

/* Hum, hum... */
int ModFromLong(u, l)
BigMod u;
long l;
{
    BnnSetToZero(u, Modul+1);
    BnnSetDigit(u, l);
    if(BnnCompare(u, 1, Modu, Modul) == 1){
	/* this means that Modul = 1 */
	BnnDivide(u, 2, Modu, 1);
	BnnSetDigit((u+1), 0);
    }
    return(1);
}

/***********************************************************************
			 	Miscanelleous
***********************************************************************/

double runtime() {
	struct rusage r;
	struct timeval t;

	getrusage(0, &r);
	t = r.ru_utime;
	return(1000*t.tv_sec + (t.tv_usec/1000.));
}

space() 
{
    struct rusage r;

    getrusage(0, &r);
#ifdef MIPS
    printf("maxrss=%ld ixrss=%ld ", r.ru_maxrss, r.ru_ixrss);
    printf("ismrss=%ld idrss=%ld\n", r.ru_ismrss, r.ru_idrss);
#endif
}

int BmIsMinusOne(n, nl)
BigNum n;
int nl;
{
    int ok;

    BnnAddCarry(n, Modul, 1);
    ok = BnnCompare(Modu, Modul, n, Modul);
    BnnSubtractBorrow(n, Modul, 0);
    return(!ok);
}

/* Extracts a BigZ z from s and puts z mod Modu in u */
ReadBigMod(u, s)
BigNum u;
char s[];
{
    if(s[0] == '#'){
	char t[STRMAX];

	sscanf(s, "#{%[^}]", t);
	ModFromString(u, t, 10);
#ifdef DEBUG
	if(debuglevel >= 8){
	    printf("String read=%s\n", t);
	    printf(" -> "); BnnPrint(u, Modul); printf("\n");
	}
#endif
    }
    else ModFromString(u, s, 10);
}
    
/* Reads a list of BigMod from a file */
int ReadBigModFromFile(u, p_ul, file)
BigMod u;
int *p_ul;
FILE *file;
{
    char s[STRMAX];

    fscanf(file, "%s", s);
    ModSetToZero(u);
    ReadBigMod(u, s);
    *p_ul = BnnNumDigits(u, Modul);
}

/********** Taken/modified from bz.c **********/

#define M_LN2	                0.69314718055994530942
#define M_LN10	                2.30258509299404568402
/* constants used by BzToString() and BzFromString() */
static double BzLog [] = 
{
    0, 
    0, 			/* log (1) */
    M_LN2,  		/* log (2) */
    1.098612, 		/* log (3) */
    1.386294, 		/* log (4) */
    1.609438, 		/* log (5) */
    1.791759, 		/* log (6) */
    1.945910, 		/* log (7) */
    2.079442, 		/* log (8) */
    2.197225, 		/* log (9) */
    M_LN10, 		/* log (10) */
    2.397895, 		/* log (11) */
    2.484907, 		/* log (12) */
    2.564949, 		/* log (13) */
    2.639057, 		/* log (14) */
    2.708050, 		/* log (15) */
    2.772588, 		/* log (16) */
};    

#define CTOI(c)			(c >= '0' && c <= '9' ? c - '0' :\
                                 c >= 'a' && c <= 'f' ? c - 'a' + 10:\
                                 c >= 'A' && c <= 'F' ? c - 'A' + 10:\
                                 0)

/* Fills a BigMod u whose value is represented by "string" in the
 * specified base.  The "string" may contain leading spaces,
 * followed by an optional sign, followed by a series of digits.
 */
/* Adapted from BzFromString */
ModFromString(u, s, base)
BigMod u;
char *s;
BigNumDigit base;
{
    BigNum 	z, p, t;
    BzSign 	sign;
    int 	zl;
    
#ifdef CHECK
    BmCheck(u, "ModFromString");
#endif
    /* Throw away any initial space */
    while (*s == ' ') 
        s++;
    
    /* Allocate BigNums */
    zl = strlen(s) * BzLog[base] / (BzLog[2] * BN_DIGIT_SIZE) + 2;

    z = BNC(zl);
    p = BNC(zl);

    if (!z || !p) 
        return (NULL);
    
    /* Set up sign, base, initialize result */
    sign = (*s == '-' ? (s++, BZ_MINUS) : *s == '+' ? (s++, BZ_PLUS)
						: BZ_PLUS);
    
    /* Multiply in the digits of the string, one at a time */
    for (;  *s != '\0';  s++)
    {
	BnnSetToZero(p, zl);
	BnnSetDigit(p, CTOI (*s));
	BnnMultiplyDigit(p, zl, z, zl, base);  

	/* exchange z and p (to avoid BzMove (z, p) */
	t = p,  p = z,  z = t;
    }
    
    zl = BnnNumDigits(z, zl);
    if(BnnCompare(z, zl, Modu, Modul) != -1){
	BnnSetDigit((z+zl), 0);
	BnnDivide(z, zl+1, Modu, Modul);
	zl = BnnNumDigits(z, Modul);
    }
    BnnAssign(u, z, zl);
    /* Takes care of sign */
    if(sign == -1)
	ModNegate(u);
    /* Free temporary BigNums */
    BnFree(z); BnFree(p);
    return(1);
}

