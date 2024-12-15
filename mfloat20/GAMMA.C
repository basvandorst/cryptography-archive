extern unsigned _stklen = 40000U;       /* use a large stack segment */

#include <conio.h>
#include "mfloat.h"                     /* use the mfloat-library */
#define maxbernoulli 22
#define mingamma 220

/*------------------------------------------------------------------------*/

void realinput(mfloat x, char * name) {

  char ch[100];

  do {
    printf("%s",name);
    scanf("%s",&ch);
    if (strtomf(x,ch) == 0) break;
    printf("Error in input!\n\n");  /* error-message */
  } while (1);
}

/*------------------------------------------------------------------------*/

int main() {

  mfloat B[maxbernoulli+1];
  mfloat factorial[2*maxbernoulli+1];
  mfloat error,maxerr;
  mfloat z,gamma,fak;
  mfloat h1,h2;
  int k,i;
  char ch[100];

  mfloatTest80386();
  clrscr();
  printf("\n\n");
  printf("          Calculation of the Gamma Function â(z)\n");
  printf("          ======================================\n");
  printf("\n");
  printf("                           ì\n");
  printf("                           ô  z-1  -t\n");
  printf("                    â(z) = ³ t    e   dt\n");
  printf("                           õ \n");
  printf("                           0 \n\n");
  realinput(z,"z = ");
  ldtomf(fak,1);
  ldtomf(h2,mingamma);
  ldtomf(h1,1);
  while (gtm(h2,z)) {
    multm(fak,z);
    addm(z,h1);
  }
  inversm(fak);
  subm(multm(subm(equm(gamma,z),ldtomf(h1,0.5)),logm(equm(h2,z))),z);
  addm(gamma,divi(logm(multi(getpim(h2),2)),2));
  ldtomf(maxerr,1E300);
  ldtomf(B[0],1);
  ldtomf(factorial[0],1);
  ldtomf(factorial[1],1);
  for (k=1; k < maxbernoulli; k++) {
    multi(equm(factorial[2*k],factorial[2*k-1]),2*k);
    multi(equm(factorial[2*k+1],factorial[2*k]),2*k+1);
    ldtomf(B[k],0.5);
    for (i=0; i < k; i++) {
      multm(equm(h1,factorial[2*i]),factorial[2*k+1-2*i]);
      multm(equm(h2,factorial[2*k]),B[i]);
      subm(B[k],divm(h2,h1));
    }
    divi(divi(fabsm(equm(error,B[k])),2*k),2*k-1);
    multm(error,powi(fabsm(equm(h1,z)),1-2*k));
    if (gtm(maxerr,error)) {
      equm(maxerr,error);
      if (k > 1) {
	divi(divi(equm(h1,B[k-1]),2*k-2),2*k-3);
	powi(equm(h2,z),3-2*k);
	addm(gamma,multm(h1,h2));
      }
    }
    else break;
  }
  multm(expm(gamma),fak);
  printf("\ngamma = \n");
  printf("%s\n\n", mftostr(ch,gamma,100, ".70E"));
  printf("Relative Error of the calculation due to termination of the series: ");
  printf("%s\n\n", mftostr(ch,maxerr,100, ".5E"));
  return 0;
}
