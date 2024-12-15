/*
 *   Program to decipher message using Blum-Goldwasser probabalistic
 *   Public key method
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
{  /*  decipher using private key  */
    big x,y,ke,p,q,n,a,b,alpha,beta,t;
    FILE *ifile;
    FILE *ofile;
    int ch;
    long i,ipt;
    char ifname[13],ofname[13];
    bool flo;
    mirsys(100,0);
    x=mirvar(0);
    ke=mirvar(0);
    p=mirvar(0);
    q=mirvar(0);
    n=mirvar(0);
    y=mirvar(0);
    alpha=mirvar(0);
    beta=mirvar(0);
    a=mirvar(0);
    b=mirvar(0);
    t=mirvar(0);
    IOBASE=60;
    if ((ifile=fopen("private.key","r"))==NULL)
    {
        printf("Unable to open file private.key\n");
        exit(0);
    }
    cinnum(p,ifile);
    cinnum(q,ifile);
    fclose(ifile);
    multiply(p,q,ke);
    do
    { /* get input file */
        printf("file to be deciphered = ");
        gets(ifname);
    } while (strlen(ifname)==0);
    strip(ifname);
    strcat(ifname,".key");
    if ((ifile=fopen(ifname,"r"))==NULL)
    {
        printf("Unable to open file %s\n",ifname);
        exit(0);
    }
    fscanf(ifile,"%ld\n",&ipt);
    cinnum(x,ifile);
    fclose(ifile);
    strip(ifname);
    strcat(ifname,".blg");
    printf("output filename = ");
    gets(ofname);
    flo=FALSE;
    if (strlen(ofname)>0) 
    { /* set up output file */
        flo=TRUE;
        ofile=fopen(ofname,"w");
    }
    else ofile=stdout;
    printf("deciphering message\n");

    xgcd(p,q,a,b,t);
    lgconv(ipt,n);            /* first recover "one-time pad" */
    incr(p,1,alpha);
    subdiv(alpha,4,alpha);
    decr(p,1,y);
    powmod(alpha,n,y,alpha);
    incr(q,1,beta);
    subdiv(beta,4,beta);
    decr(q,1,y);
    powmod(beta,n,y,beta);
    copy(x,y);
    divide(x,p,p);
    divide(y,q,q);
    powmod(x,alpha,p,x);    /* using chinese remainder thereom */
    powmod(y,beta,q,y);
    mad(x,q,q,ke,ke,t);
    mad(t,b,b,ke,ke,t);
    mad(y,p,p,ke,ke,x);
    mad(x,a,a,ke,ke,x);
    add(x,t,x);
    divide(x,ke,ke);
    if (size(x)<0) add(x,ke,x);

    if ((ifile=fopen(ifname,"rb"))==NULL)
    {
        printf("Unable to open file %s\n",ifname);
        exit(0);
    }  
    for (i=0;i<ipt;i++)
    { /* decipher character by character */
        ch=fgetc(ifile);
        ch^=x[1];               /* XOR with last byte of x */
        fputc((char)ch,ofile);
        mad(x,x,x,ke,ke,x);
    }
    fclose(ifile);
    if (flo) fclose(ofile);
    printf("\nmessage ends\n");
}

