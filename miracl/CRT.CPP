/*
 *   MIRACL Chinese Remainder Thereom routines (for use with crt.h) 
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <crt.h>

Crt::Crt(int r,Big *moduli)
{ /* constructor */
    int i,j,k;
    if (r<2)
    {
        cout << "MIRACL C++ Error - Bad 1st parameter to Crt Constructor\n";
        exit(0);
    }
    for (i=0;i<r;i++) if (moduli[i]<2)
    {
        cout << "MIRACL C++ Error - Bad 2nd parameter to Crt Constructor\n";
        exit(0);
    }
    np=r;
    type=BIGS;
    C=new Big[np*(np-1)/2];
    V=new Big[np];
    M=new Big[np];
    if (C==NULL || V==NULL || M==NULL)
    {
        cout << "MIRACL C++ Error - Out of space in Crt Constructor\n";
        exit(0);
    }
    for (k=0,i=0;i<np;i++)
    { 
        M[i]=moduli[i];
        for (j=0;j<i;j++,k++)
            C[k]=inverse(M[j],M[i]);
    }    
}

Crt::Crt(int r,utype *moduli)
{ /* constructor */
    int i,j,k;
    if (r<2)
    {
        cout << "MIRACL C++ Error - Bad 1st parameter to Crt Constructor\n";
        exit(0);
    }
    for (i=0;i<r;i++) if (moduli[i]<2)
    {
        cout << "MIRACL C++ Error - Bad 2nd parameter to Crt Constructor\n";
        exit(0);
    }
    np=r;
    type=SMALLS;
    c=new utype[np*(np-1)/2];
    v=new utype[np];
    m=new utype[np];
    if (c==NULL || v==NULL || m==NULL)
    {
        cout << "MIRACL C++ Error - Out of space in Crt Constructor\n";
        exit(0);
    }
    for (k=0,i=0;i<np;i++)
    {
        m[i]=moduli[i];
        for (j=0;j<i;j++,k++)
            C[k]=invers(m[j],m[i]);
    }
}

Big Crt::eval(Big *rem)
{           
    int i,j,k;
    Big x,t;
    V[0]=rem[0];
    for (k=0,i=1;i<np;i++)
    { /* Knuth P. 274 */
        V[i]=rem[i]-V[0];
        V[i]=modmult(V[i],C[k],M[i]);
        k++;
        for (j=1;j<i;j++,k++)
        {
            V[i]-=V[j];
            V[i]=modmult(V[i],C[k],M[i]);
        }
        if (V[i]<0) V[i]+=M[i];
    }
    for (x=0,t=1,i=0;i<np;i++)
    {
        x+=(t*V[i]);
        t*=M[i];
    }   
    return x;
}

Big Crt::eval(utype *rem)
{
    int i,j,k;
    Big x,t;
    v[0]=rem[0];
    for (k=0,i=1;i<np;i++)
    { /* Knuth P. 274 */
        v[i]=rem[i]-v[0];
        v[i]=smul(v[i],c[k],m[i]);
        k++;
        for (j=1;j<i;j++,k++)
        {
            v[i]-=v[j];
            v[i]=smul(v[i],c[k],m[i]);
        }
    }
    for (x=0,t=1,i=0;i<np;i++)
    {
        x+=(t*v[i]);
        t*=m[i];
    }   
    return x;
}

