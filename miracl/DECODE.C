/*
 *   Program to decode message using RSA private key.
 *
 *   **** For Demonstration use only ******
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>
#include <stdlib.h>
#include <string.h>

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
    int i,np;
    big e,ep[2],m,ke,kd,p[2],kp[2],mn,mx;
    FILE *ifile;
    FILE *ofile;
    char ifname[13],ofname[13];
    bool flo;
    mirsys(100,0);
    np=2;         /* two primes - could be used with more */
    for (i=0;i<np;i++)
    {
        p[i]=mirvar(0);
        ep[i]=mirvar(0);
        kp[i]=mirvar(0);
    }
    e=mirvar(0);
    m=mirvar(0);
    kd=mirvar(0);
    ke=mirvar(0);
    mn=mirvar(0);
    mx=mirvar(0);
    IOBASE=60;
    if ((ifile=fopen("private.key","r"))==NULL)
    {
        printf("Unable to open file private.key\n");
        exit(0);
    }
    for (i=0;i<np;i++)
        cinnum(p[i],ifile);
    fclose(ifile);
 /* generate public and private keys */
    convert(1,ke);
    convert(1,kd);
    for (i=0;i<np;i++)
    {
        multiply(ke,p[i],ke);
        decr(p[i],1,p[i]);
        multiply(kd,p[i],kd);
    }
    premult(kd,2,kd);
    incr(kd,1,kd);
    subdiv(kd,3,kd);
    for (i=0;i<np;i++)
    {
        copy(kd,kp[i]); 
        divide(kp[i],p[i],p[i]);   /* kp = kd mod (p-1) */
        incr(p[i],1,p[i]);
    }
    crt_init(np,p);
    nroot(ke,3,mn);
    multiply(mn,mn,m);
    multiply(mn,m,mx);
    subtract(mx,m,mx);
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
    if ((ifile=fopen(ifname,"r"))==NULL)
    {
        printf("Unable to open file %s\n",ifname);
        exit(0);
    }
    forever
    { /* decode line by line */
        IOBASE=60;
        cinnum(m,ifile);
        if (size(m)==0) break;
        for (i=0;i<np;i++) powmod(m,kp[i],p[i],ep[i]);  
        crt(ep,e);    /* Chinese remainder thereom */
        if (compare(e,mx)>=0) divide(e,mn,mn);
        IOBASE=128;
        if (flo) cotnum(e,ofile);
        cotnum(e,stdout);
    }
    crt_end();
    fclose(ifile);
    if (flo) fclose(ofile);
    printf("message ends\n");
}

