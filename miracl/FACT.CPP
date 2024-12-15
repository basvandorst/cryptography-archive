/*
 *   Program to calculate factorials.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <big.h>   /* include MIRACL system */

miracl precision(500,10);   /* bigs are 500 decimal digits long */

void main()
{ /* calculate factorial of number */
    Big nf=1;           /* declare "Big" variable nf */
    int n;
    cout << "factorial program\ninput number n= \n";
    cin >> n;
    while (n>1) nf*=(n--);   /* nf=n!=n*(n-1)*(n-2)*....3*2*1  */
    cout << "n!= \n" << nf << "\n";
}

