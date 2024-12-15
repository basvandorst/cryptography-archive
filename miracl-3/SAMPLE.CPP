/*
 *   Example program
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <flash.h>

miracl precision=(-35);

void main()
{ /* Brents example program */
    Flash x;
    cout << pi() << "\n" << flush;
    x=exp(pi()*sqrt((Flash)"163/9"));
    cout << x << "\n" << flush;
    cout << pow(x,3) << "\n" << flush;
}

