#include "crypt.h"


long int **make_matrix(nrl,nrh,ncl,nch)
long int nrl,nrh,ncl,nch;
{
	int i, fail, *tmp;
	long int **m;

	fail=0;
	i = (nrh-nrl+1)*sizeof(int *);
	tmp = (int *) malloc(i);
	if (tmp==NULL) return(NULL);
	tmp -= nrl;
	m = (long int **) tmp;

	for(i=nrl;i<=nrh;i++) {
		m[i]=(long int *)malloc((size_t)(nch-ncl+1)*sizeof(long int));
		if (m[i]==NULL) return(NULL);
		m[i] -= ncl;
	}
	return(m);
}

