#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX_SHADOWS 128
#define TRUE 1
#define FALSE 0

typedef short unsigned Ukey[9];
typedef unsigned Ikey[7][10];
typedef unsigned Idata[5];
struct shadow_struct {
	int		x;
	unsigned	y;
};

typedef struct shadow_struct shadow_type;

long int **make_matrix(long int nrl,long int nrh,long int ncl,long int nch);
long int GFpow(long int x,long int y);
shadow_type *make_shadow(long int n,long int m,long int sec);
long int solve_system(long int **a, long int n);
long int combine_shadows(long int n,shadow_type *s);
unsigned prand(unsigned n);
unsigned inv(unsigned	xin);
