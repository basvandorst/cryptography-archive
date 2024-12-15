/*
 *   Program to investigate palindromic reversals.
 *   Gruenberger F. Computer Recreations, Scientific American. April 1984.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

bool rever(x,y)
big x;
big y;
{ /* reverse digits of x into y       *
   * returns TRUE if x is palindromic */
    int m,n;
    int i,k,swaps;
    bool palin;
    copy(x,y);
    palin=TRUE;
    k=numdig(y)+1;
    swaps=k/2;
    for (i=1;i<=swaps;i++)
    {
        k--;
        m=getdig(y,k);
        n=getdig(y,i);
        if (m!=n) palin=FALSE;
        putdig(m,y,i);
        putdig(n,y,k);
    }
    return palin;
}

main()
{  /*  palindromic reversals  */
    int iter;
    big x,y;
    mirsys(120,10);
    x=mirvar(0);
    y=mirvar(0);
    printf("palindromic reversals program\n");
    printf("input number\n");
    innum(x,stdin);
    iter=0;
    while (!rever(x,y))
    {
        iter++;
        add(x,y,x);
        otnum(x,stdout);
    }
    printf("palindromic after %d iterations\n",iter);
}

