/*
 *   Program to investigate palindromic reversals.
 *   Gruenberger F. Computer Recreations, Scientific American. April 1984.
 */

#include <stdio.h>
#include "miracl.h"

bool reverse(x,y)
big x;
big y;
{ /* reverse digits of x into y       *
   * returns TRUE if x is palindromic */
    int m,n;
    int i,k;
    bool palin;
    copy(x,y);
    palin=TRUE;
    k=numdig(y)+1;
    for (i=1;i<=(numdig(x)+1)/2;i++)
    {
        k--;
        m=getdig(x,k);
        n=getdig(x,i);
        if (m!=n) palin=FALSE;
        putdig(m,x,i);
        putdig(n,x,k);
    }
    return palin;
}

main()
{  /*  palindromic reversals  */
    int iter;
    big x,y;
    mirsys(100,10);
    x=mirvar(0);
    y=mirvar(0);
    printf("palindromic reversals program\n");
    printf("input number\n");
    innum(x,stdin);
    iter=0;
    while (!reverse(x,y))
    {
        iter++;
        add(x,y,x);
        otnum(x,stdout);
    }
    printf("palindromic after %d iterations\n",iter);
}

