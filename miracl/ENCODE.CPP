/*
 *   Program to encode text using RSA public key.
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <big.h>   /* include MIRACL system */

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
{  /*  encode using public key  */
    Big e,m,y,ke,mn,mx;
    ifstream public_key="public.key";
    ifstream input_file;
    ofstream output_file;
    static char line[500];
    static char buff[256];
    static char ifname[13],ofname[13];
    bool fli,last;
    int i,ipt,klen;
    IOBASE=60;
    public_key >> ke;
    mn=root(ke,3);
    mx=mn*mn*mn-mn*mn;
 /* find key length in characters */
    klen=bits(mx)/7;
    cout << "file to be encoded = " ;
    cin.getline(ifname,13);
    fli=FALSE;
    if (strlen(ifname)>0) fli=TRUE;
    if (fli)
    { /* set up input file */
        strcpy(ofname,ifname);
        strip(ofname);
        strcat(ofname,".rsa");
        input_file.open(ifname); 
        if (!input_file)
        {
            cout << "Unable to open file " << ifname << "\n";
            exit(0);
        }
        cout << "encoding message\n";
    }
    else
    { /* accept input from keyboard */
        cout << "output filename = ";
        cin >> ofname; 
        strip(ofname);    
        strcat(ofname,".rsa");
        cout << "input message - finish with cntrl z\n";
    }
    output_file.open(ofname);
    ipt=0;
    last=FALSE;
    while (!last)
    { /* encode line by line */
        if (fli)
        {
            input_file.getline(&line[ipt],132);
            if (input_file.eof() || input_file.fail()) last=TRUE;
        }
        else
        {
            cin.getline(&line[ipt],132);
            if (cin.eof() || cin.fail()) last=TRUE;
        }
        strcat(line,"\n");
        ipt=strlen(line);
        if (ipt<klen && !last) continue;
        while (ipt>=klen)
        { /* chop up into klen-sized chunks and encode */
            for (i=0;i<klen;i++)
                buff[i]=line[i];
            buff[klen]='\0';
            for (i=klen;i<=ipt;i++)
                line[i-klen]=line[i];
            ipt-=klen;
            IOBASE=128;
            m=buff;
            e=pow(m,3,ke);
            IOBASE=60;
            output_file << e << endl;
        }
        if (last && ipt>0)
        { /* now deal with left overs */
            IOBASE=128;
            m=line;
            if (m<mn)
            { /* pad out with random number if necessary */
                m+=mn*(mn*mn-rand(mn));
            }
            e=pow(m,3,ke);
            IOBASE=60;
            output_file << e << endl;
        }
    }
}   

