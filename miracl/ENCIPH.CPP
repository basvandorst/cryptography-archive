/*
 *   Program to encipher text using Blum-Goldwasser Probabalistic
 *   Public Key method
 *   See "Modern Cryptology - a tutorial" by Gilles Brassard. 
 *   Published by Springer-Verlag, 1988
 *
 *   Note: This implementation uses only the Least Significant Byte
 *   of the big random number x in encipherment/decipherment, as it has 
 *   proven to be completely secure. However it is conjectured that
 *   that up to half the bytes in x (the lower half) are also secure.
 *   They could be used to considerably speed up this method.
 *   
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <fstream.h>
#include <big.h>   /* include MIRACL system */
#include <string.h>

miracl precision=100;

void strip(char *name)
{ /* strip off filename extension */
    int i;
    for (i=0;name[i]!='\0';i++)
    {
        if (name[i]!='.') continue;
        name[i]='\0';
        break;
    }
}

void main()
{  /*  encipher using public key  */
    Big x,ke;
    ifstream public_key="public.key";
    ifstream input_file;
    ofstream output_file,key_file;
    char ifname[13],ofname[13];
    bool fli;
    char ch;
    long seed,ipt;

    IOBASE=60;
    public_key >> ke;
    cout << "Enter 9 digit random number seed  = ";
    cin >> seed;
    irand(seed);
    x=rand(ke);
    cout << "file to be enciphered = ";
    cin.sync();
    cin.getline(ifname,13);
    fli=FALSE;
    if (strlen(ifname)>0) fli=TRUE;
    if (fli)
    { /* set up input file */
        strcpy(ofname,ifname);
        strip(ofname);
        strcat(ofname,".blg");
        input_file.open(ifname);
        if (!input_file)
        {
            cout << "Unable to open file " << ifname << "\n";;
            exit(0);
        }
        cout << "enciphering message" << endl;
    }
    else
    { /* accept input from keyboard */
        cout << "output filename = ";
        cin >> ofname;
        strip(ofname);    
        strcat(ofname,".blg");
        cout << "input message - finish with cntrl z" << endl;
    }
    output_file.open(ofname,ios::binary);
    ipt=0;
    forever
    { /* encipher character by character */
        x=(x*x)%ke;
        if (fli) 
        {
            if (input_file.eof()) break;
            input_file.get(ch);
        }
        else
        {
            if (cin.eof()) break;
            cin.get(ch);
        }
        ch^=x[1];              /* XOR with last byte of x */
        output_file << ch;
        ipt++;
    }
    strip(ofname);
    strcat(ofname,".key");
    key_file.open(ofname);
    key_file << ipt << "\n";
    key_file << x << endl;
}   

