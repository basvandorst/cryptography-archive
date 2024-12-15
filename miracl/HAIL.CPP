/*
 *   Program to investigate hailstone numbers.
 *   Gruenberger F. 'Computer Recreations' Scientific American. April 1984.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <big.h>

miracl precision(400,10);

void main()
{  /*  hailstone numbers  */
    Big r,x,mx=0;
    int iter;
    iter=0;
    cout << "number = \n";   
    cin >> x;    
    do
    {
        if (x>mx) mx=x;    
        r=x%2;   
        if (r!=0) x=3*x+1;
        else      x/=2;
        cout << x << endl;
        iter++;
    } while (x!=1); 
    cout <<  "path length = " << iter << "\n";
    cout <<  "maximum = " << mx << "\n";
}

