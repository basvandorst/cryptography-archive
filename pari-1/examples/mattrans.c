#include "genpari.h"

GEN matexp(GEN,long);

main()
{
  GEN x,y;
  long prec,d;
  char s[512];

  init(1000000,2); /* take a million bytes of memory for the stack */
  printf("precision of the computation in decimal digits? ");
  scanf("%d",&d);if(d<0) prec=3;else prec=(long)(d*K1+3);
  printf("input your matrix in GP format:\n");
  s[0]=0;while(!s[0]) gets(s); x=lisexpr(s);
  y=matexp(x,prec);
  sor(y,'g',d,0);
  return 0;
}

GEN matexp(GEN x,long prec)
{
  GEN y,r,s,p1,p2;
  long tx=typ(x),lx=lg(x),i,k,n,lbot,ltop;

/* check that x is a square matrix */

  if(tx!=19) {printf("This expression is not a matrix\n");return(gzero);}
  if(lx!=lg((GEN)x[1])) {printf("Not a square matrix\n");return(gzero);}

/* compute the L2 norm of x */

  ltop=avma;s=gzero;
  r=cgetr(prec+1);gaffsg(1,r);p1=gmul(r,x);
  for(i=1;i<lx;i++) s=gadd(s,gnorml2((GEN)p1[i]));
  if(typ(s)==2) setlg(s,3);
  s=gsqrt(s,3); /* we do not need much precision on s */

/* if s<1 we are happy */

  if(expo(s)<0) n=0;
  else {n=expo(s)+1;p1=gmul2n(p1,-n);setexpo(s,-1);}

/* initializations before the loop */

  y=gscalmat(r,lx-1); /* this creates the scalar matrix r*Id */
  p2=p1;r=s;k=1;
  lbot=avma;y=gadd(y,p2);

/* now the main loop */

  while(expo(r) >= -32*(prec-1))
    {
      k++;p2=gdivgs(gmul(p2,p1),k);r=gdivgs(gmul(s,r),k);
      lbot=avma;y=gadd(y,p2);
    }

/* now square back n times if necessary */

  for(i=0;i<n;i++) {lbot=avma;y=gmul(y,y);}
  y=gerepile(ltop,lbot,y);
  return(y);
}
