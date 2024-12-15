/*
 *   Program to decode message using RSA private key.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <big.h>   /* include MIRACL system */
#include <crt.h>   /* chinese remainder thereom */

miracl precision=100;

void strip(char *name)
{ /* strip extension off filename */
    int i;
    for (i=0;name[i]!='\0';i++)
    {
        if (name[i]!='.') continue;
        name[i]='\0';
        break;
    }
}

void main()
{  /*  decode using private key  */
    int i,np;
    Big e,ep[2],m,ke,kd,p[2],kp[2],mn,mx;
    ifstream private_key="private.key";
    ifstream input_file;
    ofstream output_file;
    char ifname[13],ofname[13];
    bool flo;
    np=2;         /* two primes - could be used with more */
    IOBASE=60;
    for (i=0;i<np;i++)
        private_key >> p[i];
 /* construct public and private keys */
    ke=kd=1;
    for (i=0;i<np;i++)
    { /* Calculate ke and kd */
        ke*=p[i];
        p[i]-=1;;
        kd*=p[i];
    }
    kd=(2*kd+1)/3;
    for (i=0;i<np;i++)
    { /* prepare for Chinese remainder thereom *
       * reduce exponent mod p[i]-1  Euler TF  */
        kp[i]=kd%p[i];
        p[i]+=1;
    }
    mn=root(ke,3);
    mx=mn*mn*mn-mn*mn;
    do
    { /* get input file */
        cout << "file to be decoded = ";
        cin >> ifname;
    } while (strlen(ifname)==0);
    strip(ifname);
    strcat(ifname,".rsa");
    cout << "output filename = ";
    cin.sync();
    cin.getline(ofname,13);
    flo=FALSE;
    if (strlen(ofname)>0) 
    { /* set up output file */
        flo=TRUE;
        output_file.open(ofname);
    }
    cout << "decoding message\n";
    input_file.open(ifname);
    if (!input_file)
    {
        cout << "Unable to open file " << ifname << "\n";
        exit(0);
    }
    Crt chinese(np,p);   /* use chinese remainder thereom */
    forever
    { /* decode line by line */
        IOBASE=60;
        input_file >> m;
        if (m==0) break;
        for (i=0;i<np;i++) ep[i]=pow(m,kp[i],p[i]);  
        e=chinese.eval(ep);
        if (e>=mx) e%=mn;
        IOBASE=128;
        if (flo) output_file << e;
        cout << e << flush;
    }
    cout << "message ends\n";
}

