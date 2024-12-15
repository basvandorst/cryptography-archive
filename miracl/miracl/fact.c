/*
 *   Program to calculate factorials.
 */

#include <stdio.h>
#include "miracl.h"   /* include MIRACL system */

main()
{ /* calculate factorial of number */
    big nf;           /* declare "big" variable nf */
    int n;
    mirsys(500,10);   /* initialise system to base 10 ,500 digits per "big" */
    nf=mirvar(1);     /* initialise "big" variable nf=1 */
    printf("factorial program\n");
    printf("input number n= \n");
    scanf("%d",&n);
    getchar();
    while (n>1) premult(nf,n--,nf);   /* nf=n!=n*(n-1)*(n-2)*....3*2*1  */
    printf("n!= \n");
    otnum(nf,stdout); /* output result */ 
}
