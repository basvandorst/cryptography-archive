/*
 *   Program to encipher text using OKAMOTO public key.
 */

#include <stdio.h>
#include "miracl.h"

void strip(name)
char name[];
{ /* strip off filename extension */
    int i;
    for (i=0;name[i]!='\0';i++)
    {
        if (name[i]!='.') continue;
        name[i]='\0';
        break;
    }
}

main()
{  /*  encipher using public key  */
    big e,m,m1,m2,n,u,mx;
    FILE *ifile;
    FILE *ofile;
    static char line[500];
    char ifname[13],ofname[13];
    bool fli,last;
    int i,ipt,klen[4],kl,turn;
    mirsys(100,MAXBASE);
    e=mirvar(0);
    m=mirvar(0);
    m1=mirvar(0);
    m2=mirvar(0);
    n=mirvar(0);
    u=mirvar(0);
    mx=mirvar(0);
    IOBASE=60;
    ifile=fopen("public.key","r");
    cinnum(n,ifile);
    cinnum(u,ifile);
    fclose(ifile);
    root(n,9,mx);
    kl=0;
    while (size(mx)>0)
    { /* find max message length < n^(1/9) */
        kl++;
        subdiv(mx,128,mx);
    }
    klen[0]=klen[1]=kl-1;
    copy(n,mx);
    kl=0;
    while (size(mx)>0)
    { /* find max message length  < n */
        kl++;
        subdiv(mx,128,mx);
    }
    klen[2]=klen[3]=kl-1;
    printf("file to be enciphered = ");
    gets(ifname);
    fli=FALSE;
    if (strlen(ifname)>0) fli=TRUE;
    if (fli)
    { /* set up input file */
        strcpy(ofname,ifname);
        strip(ofname);
        strcat(ofname,".oka");
        ifile=fopen(ifname,"r");
        printf("enciphering message\n");
    }
    else
    { /* accept input from keyboard */
        ifile=stdin;
        do
        {
            printf("output filename = ");
            gets(ofname); 
        } while (strlen(ofname)==0);
        strip(ofname);    
        strcat(ofname,".oka");
        printf("input message - finish with cntrl z\n");
    }
    ofile=fopen(ofname,"w");
    WRAP=OFF;
    ipt=0;
    last=FALSE;
    turn=0;
    while (!last)
    { /* encipher line by line */
        if (fgets(&line[ipt],132,ifile)==NULL) last=TRUE;
        if (line[ipt]==EOF) last=TRUE;
        ipt=strlen(line);
        if (ipt<klen[turn] && !last) continue;
        while (ipt>=klen[turn])
        { /* chop up into klen-sized chunks and encipher */
            kl=klen[turn];
            for (i=0;i<kl;i++)
                IBUFF[i]=line[i];
            IBUFF[kl]='\0';
            for (i=kl;i<=ipt;i++)
                line[i-kl]=line[i];
            ipt-=kl;
            IOBASE=128;
            if (turn==0)
            {
                turn=1;
                cinnum(m1,ifile);
                continue;
            }
            cinnum(m,ifile);
            if (turn==1)
            {
                copy(m,m2);
                multiply(m1,u,e);
                add(e,m2,e);
                mad(e,e,e,n,n,e);   /* e = (m2+m1.u)^2 mod n */
            }
            if (turn==2) mad(m2,m,m,n,n,e);
            if (turn==3) mad(m1,m,m,n,n,e);
            IOBASE=60;
            cotnum(e,ofile);
            turn++;
            if (turn>3) turn=0;
        }
        if (last && (ipt>0  || turn==1))
        { /* now deal with left overs */
            strcpy(IBUFF,line);
            IOBASE=128;
            if (turn==0)
            {
                zero(m2);
                cinnum(m1,ifile);
            }
            else if (ipt>0) cinnum(m,ifile);
            if (turn==1)
            {
                copy(m,m2);
                multiply(m1,u,e);
                add(e,m2,e);
                mad(e,e,e,n,n,e);
            }
            if (turn==2) mad(m2,m,m,n,n,e);
            if (turn==3) mad(m1,m,m,n,n,e);
            IOBASE=60;
            cotnum(e,ofile);
        }
    }
    fclose(ofile);
    if (fli) fclose(ifile);
}   


