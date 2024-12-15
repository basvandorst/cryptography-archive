/*
 *   Program to decipher message using OKAMOTO private key.
 */

#include <stdio.h>
#include "miracl.h"

void strip(name)
char name[];
{ /* strip extension off filename */
    int i;
    for (i=0;name[i]!='\0';i++)
    {
        if (name[i]!='.') continue;
        name[i]='\0';
        break;
    }
}

main()
{  /*  decipher using private key  */
    big m,m1,m2,pq,p,q,n,x,y,z,w,u1,u2,a1,a2,b1,b2,ab,c;
    FILE *ifile;
    FILE *ofile;
    int turn;
    char ifname[13],ofname[13];
    bool flo;
    mirsys(100,MAXBASE);
    m=mirvar(0);
    m1=mirvar(0);
    m2=mirvar(0);
    pq=mirvar(0);
    p=mirvar(0);
    q=mirvar(0);
    n=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);
    w=mirvar(0);
    z=mirvar(0);
    u1=mirvar(0);
    u2=mirvar(0);
    a1=mirvar(0);
    a2=mirvar(0);
    b1=mirvar(0);
    b2=mirvar(0);
    ab=mirvar(0);
    c=mirvar(0);
    IOBASE=60;
    ifile=fopen("private.key","r");
    cinnum(p,ifile);
    cinnum(q,ifile);
    cinnum(u1,ifile);
    cinnum(u2,ifile);
    fclose(ifile);
    multiply(p,q,pq);
    multiply(pq,p,n);
    copy(u1,a1);
    divide(a1,pq,b1);      /* get a1, b1 */
    copy(u2,a2);
    divide(a2,pq,b2);      /* get a2, b2 */
    multiply(a1,b2,ab);
    multiply(a2,b1,c);
    subtract(ab,c,ab);
    copy(u1,u2);
    mad(u1,u1,u1,pq,pq,u1); /* u1 = u1.u1 mod pq */
    mad(u2,u2,u2,n,n,u2);   /* u2 = u1.u1 mod n  */
    xgcd(ab,p,ab,ab,ab);    /* ab = 1/ab mod p   */
    do
    { /* get input file */
        printf("file to be deciphered = ");
        gets(ifname);
    } while (strlen(ifname)==0);
    strip(ifname);
    strcat(ifname,".oka");
    printf("output filename = ");
    gets(ofname);
    flo=FALSE;
    if (strlen(ofname)>0) 
    { /* set up output file */
        flo=TRUE;
        ofile=fopen(ofname,"w");
    }
    printf("deciphering message\n");
    ifile=fopen(ifname,"r");          
    turn=0;
    forever
    { /* decipher line by line */
        IOBASE=60;                    
        cinnum(c,ifile);
        if (size(c)==0) break;
        IOBASE=128;
        if (turn==0)
        {
            mad(c,u1,u1,pq,pq,x);     /* x = c.u1 mod pq     */
            mad(c,u2,u2,n,n,y);
            divide(y,pq,y);           /* y = (c.u2 mod n)/pq */
            root(x,2,z);              /* z = x^(1/2)         */
            premult(z,2,x);
            xgcd(x,p,x,x,x);          /* x = 1/(2z) mod p    */
            mad(y,x,x,p,p,w);         /* w = y.x mod p       */
            multiply(a1,w,m1);
            multiply(b1,z,c);
            subtract(m1,c,m1);
            mad(m1,ab,ab,p,p,m1);     /* m1 = (a1.w - b1.z).ab mod p */
            if (size(m1)<0) add(m1,p,m1);
            if (flo) cotnum(m1,ofile);
            cotnum(m1,stdout);
            multiply(b2,z,m2);
            multiply(a2,w,c);
            subtract(m2,c,m2);
            mad(m2,ab,ab,p,p,m2);     /* m2 = (b2.z - a2.w).ab mod p */
            if (size(m2)<0) add(m2,p,m2);
            if (flo) cotnum(m2,ofile);
            cotnum(m2,stdout);
        }
        else
        {
            if (turn==1) xgcd(m2,n,m,m,m);
            if (turn==2) xgcd(m1,n,m,m,m);
            mad(c,m,m,n,n,m);              /* m = c/m mod n */
            if (size(m)<0) add(m,n,m);
            if (flo) cotnum(m,ofile);
            cotnum(m,stdout);
        }
        turn++;
        if (turn>2) turn=0;
    }
    fclose(ifile);
    if (flo) fclose(ofile);
    printf("message ends\n");
}

