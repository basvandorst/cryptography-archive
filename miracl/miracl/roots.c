/*
 * Program to calculate roots
 */

#include <stdio.h>
#include "miracl.h"

main()
{ /* Find roots */
    flash x,y;
    int n;
    mirsys(-45,MAXBASE);
    x=mirvar(0);
    y=mirvar(0);
    POINT=ON;
    printf("enter number\n");
    cinnum(x,stdin);
    printf("to the power of 1/");
    scanf("%d",&n);
    getchar();
    froot(x,n,y);
    cotnum(y,stdout);
    fpower(y,n,x);
    printf("to the power of %d = ",n);
    cotnum(x,stdout);
    if (EXACT) printf("Result is exact!\n");
}
