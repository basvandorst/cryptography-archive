#include "crypt.h"

int	GFinv[257] = 
	   {  0,  1,129, 86,193,103, 43,147,225,200,180,187,150,178,
		202,120,241,121,100,230, 90, 49,222,190, 75, 72, 89,238,
		101,195, 60,199,249,148,189,235, 50,132,115,145, 45,163,
		153,  6,111, 40, 95,175,166, 21, 36,126,173, 97,119,243,
		179,248,226, 61, 30, 59,228,102,253, 87, 74,234,223,149,
		246,181, 25,169, 66, 24,186,247,201,244,151,165,210, 96,
		205,127,  3, 65,184, 26, 20,209,176,152,216, 46, 83, 53,
		139,135, 18, 28, 63,  5,215,164,177,245,188,224,250, 44,
		218,116,124, 38,113,134,159, 54, 15, 17,158,140,114,220,
		 51, 85,255,  2,172,206, 37,143,117, 99,240,242,203, 98,
		123,144,219,133,141, 39,213,  7, 33, 69, 12, 80, 93, 42,
		252,194,229,239,122,118,204,174,211, 41,105, 81, 48,237,
		231, 73,192,254,130, 52,161, 47, 92,106, 13, 56, 10, 71,
		233,191, 88,232, 76, 11,108, 34, 23,183,170,  4,155, 29,
		198,227,196, 31,  9, 78, 14,138,160, 84,131,221,236, 91,
		 82,162,217,146,251,104, 94,212,112,142,125,207, 22, 68,
		109,  8, 58,197, 62,156, 19,168,185,182, 67, 35,208,167,
		 27,157,136, 16,137, 55, 79,107, 70, 77, 57, 32,110,214,
		154, 64,171,128,256};



/* find x^y in GF257. x,y long and positive. */
long int GFpow(x,y)
long int x,y;
{
	long int		i,z;
	
	z=1;
	if(y!=0) {
	  for(i=1;i<=y;i++) {
	    z *= x;
	    if(z>=257) z %= 257;
	  }
	} /*end if*/
	return z;
}



/* Solve a system of equations with n variables in n unknowns in GF(257) */
/* a is a 2d array. first index is equation number, second is variable number.*/
/* the variable number is the power of x for that coefficiant. Array is n X n+1 */
long int solve_system(long int **a, long int n)
{
	long int eqn, var, inv, i;
	
	for(eqn = n;eqn>=0;eqn--){ /* Eliminate orders from highest down */
	  inv = GFinv[a[eqn][eqn]];
	  for(var=0;var <= eqn;var++){ /* mult row by inv */
	    a[eqn][var] = (a[eqn][var] * inv) % 257;
	  }
	  a[eqn][n+1] = (a[eqn][n+1] * inv) % 257;
	  for(i=0;i<eqn;i++){ /* eliminate variable from all lower equations */
	    a[i][n+1] = (a[i][n+1] - (a[eqn][n+1] * a[i][eqn]) % 257 + 257) % 257;
	    for(var=0;var <= eqn;var++){
	      a[i][var] = (a[i][var] - (a[eqn][var] * a[i][eqn]) % 257 + 257) % 257;
	    }
	  }
	}
	return a[0][n+1];
}

shadow_type *make_shadow(n,m,sec)
long int n,m,sec;
{
	long int	i,j;
	shadow_type *s;
	unsigned coef;
	
	s = (shadow_type *) malloc((m*sizeof(shadow_type)));
	if(s== NULL) {
	  printf("Could not allocate memory for shadows\n");
	  exit(0);
	}
	
	for(j=1;j<=m;j++) {
	  s[j-1].y = sec; /* initialize shadows with secret */
	  s[j-1].x = j;
	}
	for(i=1;i<n;i++){ /* create the polynomial x...x^(n-1) */
	  coef = prand(1) % 257;
	  for(j=1;j<=m;j++){ /* calculate the shadows */
	    s[j-1].y = (s[j-1].y + (coef * GFpow(j,i))) % 257;		/* add in orders */
	  }
	}
	return(s);
}

/* takes an array of n shadows and shadownumbers */
/* s is a 2Xn array first is shadow number, then the shadow */
/* creates the matrix that must be solved. returns the secret. */
long int combine_shadows(n,s)
long int n;
shadow_type *s;
{
	long int		i,j, **a;
	
	a=make_matrix(0L,n-1,0L,n);
	if(a==NULL) exit(0);
	
	for(i=0;i<n;i++){ /*by rows*/
	  a[i][n] = s[i].y;
	  a[i][0] = 1;
	  for(j=1;j<n;j++) { /* collomns in row i */
	    a[i][j] = GFpow(s[i].x,j);
	  }
	}
	return solve_system(a,n-1);
}
