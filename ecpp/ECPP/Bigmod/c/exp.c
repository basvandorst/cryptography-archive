
/* $Log:	exp.c,v $
 * Revision 1.1  91/09/20  14:47:57  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:07:18  morain
 * Initial revision
 *  */

/**********************************************************************
                        Modular Exponentiation
**********************************************************************/

#include "mod.h"
#include "exp.h"

int currentmary = 0;

/* Et <- Et * a */
#define EtMul(el, a, al, tmp) \
{   BnnSetToZero(Mbuf, Mtl); \
    BnnMultiply(Mbuf, Mtl, Ebuf, (el), (a), (al)); \
    (el) = BnnNumDigits(Mbuf, Mtl);\
    if(BnnCompare(Mbuf, (el), Modu, Modul) != -1){ \
      BnnDivide(Mbuf, (el)+1, Modu, Modul); \
      (el) = BnnNumDigits(Mbuf, Modul); \
    } \
    (tmp) = Mbuf; Mbuf = Ebuf; Ebuf =(tmp);}

/* Et <- Et^2*/
#define EtSquare(el, tmp) \
{   BnSquare(Mbuf, Mtl, Ebuf, (el)); \
    (el) = BnnNumDigits(Mbuf, Mtl);\
    if(BnnCompare(Mbuf, (el), Modu, Modul) != -1){ \
      BnnDivide(Mbuf, (el)+1, Modu, Modul); \
      (el) = BnnNumDigits(Mbuf, Modul); \
    } \
    (tmp) = Mbuf; Mbuf = Ebuf; Ebuf =(tmp);}

BmExp(y, p_yl, a, al, e, el)
BigMod y, a, e;
int *p_yl, al, el;
{
    if(el <= EXP_LIMIT)
	BmExpPlain(y, p_yl, a, al, e, el);
    else{
	if(el <= 16)
	    BmExpBlock(y, p_yl, a, al, e, el, 4);
	else
	    BmExpBlock(y, p_yl, a, al, e, el, 6);
    }
}

/********** The ordinary binary method **********/

/* Ebuf <- a**e mod n, then y <- 0, y <- Ebuf.*/
int BmExpPlain(y, p_yl, a, al, e, el)
BigMod y, a, e;
int *p_yl, al, el;
{
    BigNumDigit C[1];
    BigNum tmp;
    int es, tl = al, i, ed;
    
    /* initialisation de Ebuf */
    BnnAssign(Ebuf, a, tl);
    /* on enle`ve les ze'ros en trop */
    BnnSetToZero((Ebuf+tl), Etl - tl);
    /* demarrage du calcul, ed devient le chiffre courant de e */
    ed = el - 1;
    BnnAssign(C, (e+ed), 1);
    /* recherche du bit le plus significatif */
    for(es = BN_DIGIT_SIZE; es; es--){
	if(BnnShiftLeft(C, 1, 1))
	    break;
    }
#ifdef DEBUG
    if(debuglevel >= 15) printf("es=%d\n", es);
#endif
    if(!es){
	fprintf(stderr, "Error, es is 0\n");
	exit(0);
    }
    es--;
    while(el >= 1){
	for(i = 0; i < es; i++){
	    /* Et <- Et^2 mod Modu */
	    EtSquare(tl, tmp);
#ifdef DEBUG
	    if(debuglevel >= 15) printf("Et^2=%s\n", From(Ebuf, tl));
#endif
	    if(BnnShiftLeft(C, 1, 1)){
		/* Et <- Et * a mod Modu */
		EtMul(tl, a, al, tmp);
#ifdef DEBUG
		if(debuglevel >= 15) printf("Et * a=%s\n", From(Ebuf,tl));
#endif
	    }
	}
#ifdef DEBUG
	if(debuglevel >= 10) printf("Et(%d)=%s\n",el,From(Ebuf, tl));
#endif
	--el;
	if(el){
	    --ed;
	    es = BN_DIGIT_SIZE;
	    BnnAssign(C, (e+ed), 1);
	}
    }
    BnnSetToZero(y, Modul + 1);
    BnnAssign(y, Ebuf, tl);
#ifdef DEBUG
    if(debuglevel >= 15) printf("a**e=%s\n", From(y, tl));
#endif
    *p_yl = tl;
}

/********** Blocks **********/

/* After the article by H. Cohen and A. K. Lenstra: 
   "Implementation of a new primality test",
   Math. of Comp., volume 48, number 177, January 1987, pp. 103-121.*/

int TableL[TWOPOWERMARYMAX], TableU[TWOPOWERMARYMAX];

/* Pour 0 <= i < 2**m, i=(2**li)*ui, avec ui impair et 0 <= li < m. Alors :*/
/* TableL[i]=li et TableU[i]=logshift(ui, -1), qui est l'adresse de a**ui*/
/* dans le tableau OddPower (ceci est un hack pour e'conomiser de la place).*/
int MakeTables(m)
int m;
{
    if(m > MARYMAX)
	return(0);
    else{
	int i, twopowerm = (1 << m), li, ui;

#ifdef DEBUG
	if(debuglevel >= 15) printf("%% Computing tables\n");
#endif
	for(i = 1; i < twopowerm; i++){
	    ui = i; li = 0;
	    while(!(ui & 1)){ui >>= 1; li++;}
	    TableL[i] = li;
	    TableU[i] = (ui >> 1);
	}
	return(1);
    }
}

/* ---------------------------------------------------------------------------
 .SSection "Me'thode par blocs"
---------------------------------------------------------------------------*/

/* Calcule p=a**e mod n. On e'crit : e=e[t]*(2**m)**t + ... + e[0] et */
/* e[i]=(2**li)*ui, e[t] <> 0.*/
BmExpBlock(y, p_yl, a, al, e, el, lbloc)
BigMod y, a, e;
int *p_yl, al, el, lbloc;
{
    BigNumDigit C[1];
    BigNum ee = BNC(el), tmp, OddPower[TWOPOWERMARYMAX];
    int twopowermminus1 = (1 << (lbloc-1)), es = BN_DIGIT_SIZE, i, j;
    int et = 0, qs, rs, nbits, nzeros, tl;

    if(lbloc != currentmary){
#ifdef DEBUG
	if(debuglevel >= 15) printf("%% Changing m-ary to %d\n", lbloc);
#endif
	currentmary = lbloc;
	MakeTables(lbloc);
    }
    /* copy of e */
    BnnAssign(ee, e, el);
    /* precompute odd powers of a */
    BmExpPrecompute(OddPower, a, al, lbloc);
    /* starting the computation */
    BnnAssign(C, (ee+el-1), 1);
    /* looking for the most significant bit */
    for(nbits = BN_DIGIT_SIZE; nbits; nbits--){
	if(BnnShiftLeft(C, 1, 1))
	    break;
    }
    /* nbits is the number of bits of ee[el-1] */
    if(!nbits){
	 fprintf(stderr, "Error, nbits is 0\n");
        exit(0);
     }
/*    nbits--;*/
    nzeros = BN_DIGIT_SIZE - nbits;
    nbits = nbits + (el-1) * BN_DIGIT_SIZE;
    qs = nbits / lbloc; rs = nbits % lbloc;
    if(!rs){rs = lbloc; qs--;}
#ifdef DEBUG
    if(debuglevel >= 10) printf("nbits=%d qs=%d rs=%d\n", nbits, qs, rs);
#endif
    /* dealing with the most significant digit */
    /* C[0] <- e[t]=(2**lt)*ut */
    rs += nzeros;
    if(rs < BN_DIGIT_SIZE){
	/* no pb, let's shift */
	et = BnnShiftLeft(ee, el, rs);
    }
    else{
	/* hum, hum... */
	rs -= BN_DIGIT_SIZE;
	BnnShiftLeft(ee, el, rs);
	el--;
	et = BnnGetDigit((ee+el));
    }
    /* Et <- a^ut, with ut odd */
    BnnAssign(Ebuf, OddPower[TableU[et]], Modul);
    tl = BnnNumDigits(Ebuf, Modul);
    /* skipping the zeroes */
    BnnSetToZero((Ebuf+Modul), Etl - Modul);
    /* Et <- Et**(2**lt) */
    for(i = 0; i < TableL[et]; i++)
	EtSquare(tl, tmp);
#ifdef DEBUG
    if(debuglevel >= 10) printf("Et(0)=%s\n", From(Ebuf, Modul));
#endif
    /* loop */
    for(i = 0; i < qs; i++){
	/* re'cupe'ration du chiffre courant e[i]=(2**li)*ui */
	et = BnnShiftLeft(ee, el, lbloc);
	/* Et <- Et**(2**(m-lt)) */
	for(j = 0; j < lbloc-TableL[et]; j++)
	    EtSquare(tl, tmp);
	if(et){
	    /* Et <- Et*a**vt */
	    EtMul(tl, OddPower[TableU[et]], Modul, tmp);
	    /* Et <- Et**(2**lt) */
	    for(j = 0; j < TableL[et]; j++)
		EtSquare(tl, tmp);
	}
#ifdef DEBUG
	if(debuglevel >= 10) printf("Et(%d)=%s\n", i, From(Ebuf, Modul));
#endif
    }
    BnnSetToZero(y, Modul+1);
    BnnAssign(y, Ebuf, tl);
    BnFree(ee);
    for(i = 1; i < twopowermminus1; i++)
	BmFree(OddPower[i]);
    *p_yl = tl;
}

/* Precomputing odd powers of a */
int BmExpPrecompute(OddPower, a, al, lbloc)
BigMod OddPower[];
BigNum a;
int al, lbloc;
{
    BigNum a2 = BmCreate();
    int i, a2l = 0, twopowermminus1 = (1 << (lbloc-1)), l, ll;

    BmSquare(a2, &a2l, a, al);
    OddPower[0] = BmCreate();
    BnnAssign(OddPower[0], a, al);
    l = al;
    for(i = 1; i < twopowermminus1; i++){
	OddPower[i] = BmCreate();
	BmMultiply(OddPower[i], &ll, a2, a2l, OddPower[i-1], l);
	l = ll;
#ifdef DEBUG
	if(debuglevel >= 10) 
	    printf("Odd[%d]=%s\n", i, From(OddPower[i], l));
#endif
    }
    BmFree(a2);
}

