/*
 *   Program to calculate square roots
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

main()
{ /* Find roots */
    flash x,y;
    mirsys(40,0);
    x=mirvar(0);
    y=mirvar(0);
    POINT=ON;
    printf("enter number\n");
    cinnum(x,stdin);
    printf("to the power of 1/2\n");
    froot(x,2,y);
    cotnum(y,stdout);
    fpower(y,2,x);
    printf("to the power of 2 = \n");
    cotnum(x,stdout);
    if (EXACT) printf("Result is exact!\n");
}

