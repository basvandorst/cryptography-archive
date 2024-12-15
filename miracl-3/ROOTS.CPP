/*
 *   Program to calculate roots
 * 
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <flash.h>

miracl precision=50;

void main()
{ /* Find roots */
    Flash x,y;
    int n;
    POINT=ON;
    cout << "enter number\n";
    cin >> x;
    cout << "to the power of 1/2\n";
    y=sqrt(x);
    cout << y;
    x=y*y;
    cout << "\nto the power of 2 = \n";
    cout << x;
    if (EXACT) cout << "\nResult is exact!\n";
}

