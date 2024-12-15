
/* $Log:	bmsmall.c,v $
 * Revision 1.1  91/09/20  14:47:35  morain
 * Initial revision
 * 
 * Revision 1.1  91/09/20  14:06:51  morain
 * Initial revision
 *  */

static long pratttp[5] = {2, 3, 5, 7, 11};

/* n is very small, say 1 <= n <= 100 */
SmallIfactor(n, tn, p_nf)
long n, tn[];
int *p_nf;
{
    int i, nf;

    nf = 0;
    if(n <= 3)
	tn[nf++] = n;
    else{
	for(i = 0; i < 5; i++){
	    if(!(n % pratttp[i])){
		n /= pratttp[i];
		tn[nf++] = pratttp[i];
		while(!(n % pratttp[i]))
		    n /= pratttp[i];
	    }
	}
	if(n > 1) tn[nf++] = n;
    }
    *p_nf = nf;
}

int SmallFindPrimitiveRoot(p_g, p, tn, nf)
long *p_g, p, tn[];
int nf;
{
    long g, ee;
    int i, ok;

    for(g = 2; g < 100; g++){
	for(i = 0; i < nf; i++){
	    ee = (p-1)/tn[i];
	    if(!(ok = (SmallExpMod(g, ee, p) != 1)))
		break;
	}
	if(ok) break;
    }
    *p_g = g;
    return(ok);
}

long SmallExpMod(a, e, m)
long a, e, m;
{
    long b = 1, c = a;

    if(e % 2) b = a;
    e >>= 1;
    while(e > 0){
	c = (c*c) % m;
	if(e % 2) b = (b*c) % m;
	e >>= 1;
    }
    return(b);
}

