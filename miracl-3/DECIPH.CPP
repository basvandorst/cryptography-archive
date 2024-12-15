/*
 *   Program to decipher message using Blum-Goldwasser probabalistic
 *   Public key method
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <fstream.h>
#include <big.h>   /* include MIRACL system */
#include <crt.h>
#include <string.h>

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
{  /*  decipher using private key  */
    Big x,t,ke,p[2],rem[2];
    ifstream private_key="private.key";
    ifstream input_file,key_file;
    ofstream output_file;
    int k;
    char ch;
    long i,ipt;
    char ifname[13],ofname[13];
    bool flo;
    IOBASE=60;
    private_key >> p[0] >> p[1];
    ke=p[0]*p[1];
    cout << "file to be deciphered = ";
    cin >> ifname;
    strip(ifname);
    strcat(ifname,".key");
    key_file.open(ifname);
    if (!key_file)
    {
        cout << "Unable to open file " << ifname << "\n";
        exit(0);
    }
    key_file >> ipt >> x;
    strip(ifname);
    strcat(ifname,".blg");
    cout << "output filename = ";
    cin.sync();
    cin.getline(ofname,13);
    flo=FALSE;
    if (strlen(ofname)>0) 
    { /* set up output file */
        flo=TRUE;
        output_file.open(ofname);
    }
    cout << "deciphering message" << endl;

    Crt chinese(2,p);
    
/* recover "one-time pad" */
   
    for (k=0;k<2;k++)
    {
        t=pow((p[k]+1)/4,(Big)ipt,p[k]-1);
        rem[k]=pow(x%p[k],t,p[k]);
    }

    x=chinese.eval(rem);       /* using Chinese remainder thereom */

    input_file.open(ifname,ios::binary);
    if (!input_file)
    {
        cout << "Unable to open file " << ifname << "\n";
        exit(0);
    }  
    for (i=0;i<ipt;i++)
    { /* decipher character by character */
        input_file.get(ch);
        ch^=x[1];               /* XOR with last byte of x */
        if (flo) output_file << ch << flush;
        else     cout << ch << flush;
        x=(x*x)%ke;
    }
    cout << "\nmessage ends\n";
}

