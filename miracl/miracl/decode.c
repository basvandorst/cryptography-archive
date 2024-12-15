/*
 *   Program to decode message using RSA private key.
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
{  /*  decode using private key  */
    big e,e1,e2,m,ke,kd,p,q,c,mn,mx;
    FILE *ifile;
    FILE *ofile;
    char ifname[13],ofname[13];
    bool flo;
    mirsys(100,MAXBASE);
    e=mirvar(0);
    e1=mirvar(0);
    e2=mirvar(0);
    m=mirvar(0);
    kd=mirvar(0);
    ke=mirvar(0);
    mn=mirvar(0);
    mx=mirvar(0);
    p=mirvar(0);
    q=mirvar(0);
    c=mirvar(0);
    ifile=fopen("private.key","r");
    IOBASE=60;
    cinnum(kd,ifile);
    cinnum(p,ifile);
    cinnum(q,ifile);
    fclose(ifile);
    multiply(p,q,ke);
    root(ke,3,mn);
    multiply(mn,mn,m);
    multiply(mn,m,mx);
    subtract(mx,m,mx);
    xgcd(p,q,c,c,c);  /* c.p = 1 mod q */
    do
    { /* get input file */
        printf("file to be decoded = ");
        gets(ifname);
    } while (strlen(ifname)==0);
    strip(ifname);
    strcat(ifname,".rsa");
    printf("output filename = ");
    gets(ofname);
    flo=FALSE;
    if (strlen(ofname)>0) 
    { /* set up output file */
        flo=TRUE;
        ofile=fopen(ofname,"w");
    }
    printf("decoding message\n");
    ifile=fopen(ifname,"r");
    forever
    { /* decode line by line */
        IOBASE=60;
        cinnum(m,ifile);
        if (size(m)==0) break;
        powmod(m,kd,p,e1);        /* find result modulo p ...  */
        powmod(m,kd,q,e2);        /* ..and q seperately as its */
        subtract(e2,e1,e2);       /* 4 times quicker, and then */
        multiply(e2,c,e2);        /* combine e1 and e2 using   */
        divide(e2,q,q);           /* chinese remainder theorem */
        if (size(e2)<0) add(e2,q,e2);
        multiply(e2,p,e);
        add(e,e1,e);
        if (compare(e,mx)>=0) divide(e,mn,mn);
        IOBASE=128;
        if (flo) cotnum(e,ofile);
        cotnum(e,stdout);
    }
    fclose(ifile);
    if (flo) fclose(ofile);
    printf("message ends\n");
}
