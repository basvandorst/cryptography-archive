/*   *******  EXPERIMENTAL ONLY  !!!!!!!!!!!!!
 *   MIRACL Montgomery's method for modular arithmetic without division.
 *   monty.c 
 *
 *   Test program to evaluate Montgomery's method
 *   See "Modular Multiplication Without Trial Division", Math. Comp. 
 *   Vol 44, Number 170, April 1985, Pages 519-521
 *   NOTE - there is an important correction to this paper mentioned as a
 *   footnote in  "Speeding the Pollard and Elliptic Curve Methods", 
 *   Math. Comput., Vol. 48, January 1987, 243-264
 *
 *   This method could be elegantly implemented in C++, with the details
 *   of conversion to/from the internal number representation hidden in a 
 *   header file like NUMBER.H
 *  
 *   Not significantly faster than standard method with pre-normalisation
 *   on most computers.
 *
 *   HOWEVER it may be significantly superior:-
 *
 *   (1) on systems with fast multiply hardware, but little or no support 
 *       for division, as on a Digital Signal Processor, as it requires no
 *       division whatsoever, whereas the standard method requires a division 
 *       to guess each trial quotient. 
 *
 *   (2) if specialised high precision (say 512 bit) multiplication is 
 *       supported (using Monty's original method on page 519). See Bertin 
 *       et al., "Introduction to Programmable Active Memories", Research 
 *       report 3, Digital Paris Research Lab, June 1989. Note that 
 *       multiplication hardware is much simpler than that required
 *       for division.
 *
 *   (3) for very large numbers, in which case 'fast-fourier' methods of
 *       multiplication, (see mrfast.c) would be applicable.
 *   
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>
#include <time.h>

/* Access global variables */

extern small mr_base;    /* number base     */
extern big  mr_w0;       /* workspace big   */
extern big  mr_w6;

static small ND;
static big modn;

void setmod(n)
big n;
{ /* set modulus */  
    modn=mirvar(0);
    convert(1,mr_w6);
    shift(mr_w6,n[0],mr_w6);
    xgcd(mr_w6,n,mr_w6,modn,mr_w6);
    ND=modn[1];
    copy(n,modn);
}

void nresidue(x)
big x;
{ /* convert x to n-residue format */
    shift(x,modn[0],mr_w0);
    divide(mr_w0,modn,modn);
    copy(mr_w0,x);
}

void redc(x,y)
big x,y;
{ /* Montgomery's REDC function p. 520 */
    int i,j,rn,rn2p1;
    small c,d,m;
    copy(x,mr_w0);
    c=0;
    rn=modn[0];
    rn2p1=rn+rn+1;
    for (i=1;i<=rn;i++) 
    {
        muldiv(mr_w0[i],ND,0,mr_base,&m);
        d=0;
        for (j=1;j<=rn;j++)
            d=muldiv(modn[j],m,mr_w0[i+j-1]+d,mr_base,&mr_w0[i+j-1]);
        mr_w0[rn+i]+=(c+d);
        c=0;
        if (mr_w0[rn+i]>=mr_base)
        {
            mr_w0[rn+i]-=mr_base;
            c=1; 
        }
    }
    mr_w0[rn2p1]=c;
    mr_w0[0]=rn2p1;
    shift(mr_w0,(-rn),mr_w0);
    lzero(mr_w0);

    if (c>0) psub(mr_w0,modn,y);
    else     copy(mr_w0,y);
}
    
main()
{
    time_t start,finish;
    int i;
    double tf;
    big x,y,z,nn,N;
    mirsys(50,MAXBASE);
    x=mirvar(0);
    y=mirvar(0);
    z=mirvar(0);
    N=mirvar(0);
    nn=mirvar(0);
    cinstr(N,"305857640984447875158564505276610372932639");
    setmod(N);
    cinstr(x,"111111111111111111111111111111111111111111");
    copy(x,y);

    time(&start);
    for (i=0;i<10000;i++) 
    {
        multiply(x,x,z);
        divide(z,N,N);
    }
    time(&finish);
    tf=difftime(finish,start);
    printf("Usual way - time= %f\n",tf);
    cotnum(z,stdout);

    normalise(N,nn);
    time(&start);
    for (i=0;i<10000;i++) 
    {
        multiply(x,x,z);
        divide(z,nn,nn);
    }
    time(&finish);
    tf=difftime(finish,start);
    divide(z,N,N);
    printf("Usual way with pre-normalisation - time= %f\n",tf);
    cotnum(z,stdout);

    nresidue(x);
    time(&start);
    for (i=0;i<10000;i++)
    {
        multiply(x,x,y);
        redc(y,z);
    }
    time(&finish);
    tf=difftime(finish,start);
    redc(z,z);
    printf("Monty's way - time= %f\n",tf);
    cotnum(z,stdout);
}

