/*
 *   Program to encode text using RSA public key.
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
{  /*  encode using public key  */
    big e,m,y,ke,mn,mx;
    FILE *ifile;
    FILE *ofile;
    static char line[500];
    char ifname[13],ofname[13];
    bool fli,last;
    int i,ipt,klen;
    mirsys(100,MAXBASE);
    e=mirvar(0);
    m=mirvar(0);
    y=mirvar(0);
    ke=mirvar(0);
    mn=mirvar(0);
    mx=mirvar(0);
    ifile=fopen("public.key","r");
    IOBASE=60;
    cinnum(ke,ifile);
    fclose(ifile);
    root(ke,3,mn);
    multiply(mn,mn,m);
    multiply(mn,m,mx);
    subtract(mx,m,mx);
    klen=0;
    copy(mx,m);
    while (size(m)>0)
    { /* find key length in characters */
        klen++;
        subdiv(m,128,m);
    }
    klen--;
    printf("file to be encoded = ");
    gets(ifname);
    fli=FALSE;
    if (strlen(ifname)>0) fli=TRUE;
    if (fli)
    { /* set up input file */
        strcpy(ofname,ifname);
        strip(ofname);
        strcat(ofname,".rsa");
        ifile=fopen(ifname,"r");
        printf("encoding message\n");
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
        strcat(ofname,".rsa");
        printf("input message - finish with cntrl z\n");
    }
    ofile=fopen(ofname,"w");
    WRAP=OFF;
    ipt=0;
    last=FALSE;
    while (!last)
    { /* encode line by line */
        if (fgets(&line[ipt],132,ifile)==NULL) last=TRUE;
        if (line[ipt]==EOF) last=TRUE;
        ipt=strlen(line);
        if (ipt<klen && !last) continue;
        while (ipt>=klen)
        { /* chop up into klen-sized chunks and encode */
            for (i=0;i<klen;i++)
                IBUFF[i]=line[i];
            IBUFF[klen]='\0';
            for (i=klen;i<=ipt;i++)
                line[i-klen]=line[i];
            ipt-=klen;
            IOBASE=128;
            cinnum(m,ifile);
            power(m,3,ke,e);
            IOBASE=60;
            cotnum(e,ofile);
        }
        if (last && ipt>0)
        { /* now deal with left overs */
            strcpy(IBUFF,line);
            IOBASE=128;
            cinnum(m,ifile);
            if (compare(m,mn)<0)
            { /* pad out with random number if necessary */
                bigrand(mn,y);
                multiply(mn,mn,e);
                subtract(e,y,e);
                multiply(mn,e,y);
                add(m,y,m);
            }
            power(m,3,ke,e);
            IOBASE=60;
            cotnum(e,ofile);
        }
    }
    fclose(ofile);
    if (fli) fclose(ifile);
}   
