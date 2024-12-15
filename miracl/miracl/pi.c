/*
 *  Program to calculate pi
 */

#include <stdio.h>
#include "miracl.h"

main()
{ /* Calculate pi using Guass-Legendre method */
    flash pi;
    mirsys(-35,MAXBASE);
    pi=mirvar(0);
    printf("Calculating pi... \n");
    POINT=ON;
    fpi(pi);
    cotnum(pi,stdout);
}
