From msuinfo!agate!spool.mu.edu!darwin.sura.net!europa.eng.gtefsd.com!uunet!decwrl!nntp.crl.com!acsys!mycal Sun Sep  5 14:34:29 1993
Path: msuinfo!agate!spool.mu.edu!darwin.sura.net!europa.eng.gtefsd.com!uunet!decwrl!nntp.crl.com!acsys!mycal
From: mycal@NetAcsys.com (Mycal)
Newsgroups: sci.crypt
Subject: Zipcrack.c
Distribution: world
Message-ID: <746751216snx@NetAcsys.com>
Date: Mon, 30 Aug 93 22:53:36 GMT
Organization: ACSYS, Inc.
Lines: 609


I don't know how everyone found out I had this program, but they did, so
rather than sending it a bunch of times I'm going to post the source here.
Hope that's cool with everyone. Could someone stuff it in an archive or
somthing.

I wrote zipcrack a few years ago when v1.1 was still new.  I know it is
dated but it could be usefull in writeing a newer zipcrack.  Right now
it only tries lower case chars, but this could be changed quite simply.

Other mods that I thought about that I didn't do is a dictionary hack
instead of brute force.  This would be a good project for someone. :')

Enough blather here is the code :

--- Cut ---

//////////////////////////////////////////////////////////////////////////
// Zipcrack.c
//
// This program tries to crack password protected Zip(v1.1) files.
//
// (c)1991 By Mycal Johnson, the one who does many cool hacks but still
//  gets no respect.        mycal@netacsys.com
//
// Please send me any cool enhancements for this code.
//
// Compiled with microsquishy C 6.0 with max optimization.
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

unsigned char   dos_string[128];
unsigned char   delete_file[128];
#define DWORD unsigned long
#define WORD  unsigned int
#define BYTE  unsigned char

#define USGC    unsigned char
#define USGI    unsigned
#define USGL    unsigned long

#define MAXDIGHACK  8
#define GOOD_CRC32_RESIDUAL 0xdebb20e3L

USGL crctable[256];     // global accessible //
USGI    crc2find;       // crc 2 find //

BYTE    sig[] = {0x50,0x4b,0x03,0x04};
BYTE    error_sig[] ={'e','r','r','o','r','s','!'};

unsigned long crc32();
void generatetable();
unsigned char decrypt_byte();


int maxdighack =    MAXDIGHACK;



//unsigned char preamble[10];
//unsigned char postamble[10];

int ffflag=0;


main(argc, argv)
int argc;
char *argv[];
{
    unsigned char passcode[25];
    unsigned char buffer[25];
    unsigned char lookfile[25];
    unsigned char inbuff[25];
    unsigned char zipname[25];
    FILE *zipfile;
    int end,i,j,skip,len;

    printf("\nZipcrack V0.1b - (c)1991 by Mycal Johnson - mycal@netacsys.com\n\n");

    if(argc==1)
    {
        printf("\n Usage:  'zipcrack zipfile crackfile'");
        printf("\n  zipfile   = filename.zip ");
        printf("\n  crackfile = file in the zipfile to crack\n\n");
        exit(2);
    }

    i=strlen(argv[1]);
    if(i>8)
    {
        printf("\n Filename must be 8 characters or less, do not incude .zip extention \n\n");
        exit(2);
    }
    strcpy(zipname,argv[1]);
    strcat(zipname,".zip");
    i=strlen(argv[2]);
    if(i>12)
    {
        printf("\n Filename to hack cannot be more than 12 characters including extention\n\n");
        exit(2);
    }
    strcpy(lookfile,argv[2]);

    strupr(lookfile);
    strupr(zipname);
    //
    // make unzip try string
    //
    strcpy(dos_string,"unzip ");
    strcat(dos_string,argv[1]);
    strcat(dos_string," ");
    strcat(dos_string,lookfile);
    strcat(dos_string," -s");

    strcpy(delete_file,"del ");
    strcat(delete_file,lookfile);
    //
    // check to see if target already exists
    //
    if((zipfile = fopen(lookfile,"r+b")) != NULL)
    {
        close(zipfile);
        printf("\nWarning %s already exists on disk.",lookfile);
        printf("\n Delete this file and try again.");
        exit(1);
    }


    if ((zipfile = fopen(zipname,"r+b")) == NULL )
    {
        printf("fopen failed.  Cannot find/open %s.\n",zipname);
        exit(2);
    }

    i = 0;
    end=0;
    //
    // Here we search for file to hack on inside the zip file.  First we
    // look for the local file sig then we check to see if its our file.
    //
    while(!end)
    {
        if(fread(buffer, sizeof(char),1,zipfile))
        {
            if(buffer[0] == sig[i])
                i = i + 1;
            else
                i = 0;
        if (i == 4)
        {                   // sig is 4 digits
            len=strlen(lookfile);
            skip = 26;
            if(fseek(zipfile, (long)skip, SEEK_CUR)!=0)
            {
                printf("fseek fail\n");
                exit(1);
            }
            if(fread(inbuff, sizeof(char),len,zipfile))
            {
                if(!memcmp(lookfile,inbuff,len))
                    end=1;
                else
                    i=0;
            }
            else
            {
                printf("read failed\n");
                exit(1);
            }
        }
        }
        else
            end = 2;            // end of file
    }
    //
    // We couldn't find the file to hack if end = 2;
    //
    if(end == 2) {
        printf("File to hack not in found in Zipfile.\n");
        exit(2);
    }
    //
    // Read encryption header
    //
    if(!fread(buffer, sizeof(char),12,zipfile))
    {
        printf("Cannot read from Zipfile.  fread fail\n");
        exit(2);
    }

    skip = -(len+16+12);    // - len of filename + crc offset + encript headder
    if(fseek(zipfile, (long)skip, SEEK_CUR)!=0)
    {
        printf("fseek fail\n");
        exit(1);
    }
    if(fread(inbuff, sizeof(char),4,zipfile))
    {
        printf(" crc from file = %x : %x  \n",((inbuff[3]<<8)|inbuff[2]),
                    ((inbuff[1]<<8)|inbuff[0]));
        crc2find = ((inbuff[3]<<8)|inbuff[2]);
    }
    else
    {
        printf("fread2 fail");
        exit(2);
    }
    close(zipfile);

    buffer[12]=0;

    if(crackzip(passcode,buffer))
    {                               // buffer contains encription header //
                                    // returns hacked key in passcode    //
        printf("\nWe hack success!\n\nKey is '%s'\n\n",passcode);
        printf("Thanx to your pal mycal@netacsys.com\n");
        exit(0);
    }
    else
    {
        printf("\nBummer, I couldn't hack it.\n");
        exit(1);
    }
}


crackzip(passcode, buffer)
    char *passcode;
    char *buffer;
{
    unsigned long key[3];
    unsigned char hackbuff[19];
    unsigned char password[50];
    unsigned char dos_try[128];
    unsigned int i,end, result,slen;

    generatetable();        // generate 32-bit crc table  //
//
// Set password to 'a' and the rest nil, set up end flag
//
    end = 0;
    for(i=0;i<50;i++)           // init password to nil //
        password[i]=0;
    strcpy(password,"a");
//
// this is where the main hack loop is, we loop here tell we crack the
// password or forever, whichever is first.
//
    while(!end){
        memcpy(hackbuff,buffer,15); // reset buffer to origin
        key[0] = 305419896L;        // reset key to start
        key[1] = 591751049L;
        key[2] = 878082192L;
        //
        // initilize key with password
        //
        slen = strlen(password);
        for(i=0; i<slen;i++)
            update_keys(key,password[i]);
        //
        // try the decript
        //
        if(!decript_head(hackbuff,key))
        {
            //
            // no CRC match, get next password to try, try again
            //
            update_password(password);
            if(strlen(password)>maxdighack)
            {
                return(0);
                end=2;
            }
        }
        else
        {
            //
            // We have a CRC match, so lets try to unzip the file
            //
            printf(" -- possible password is %s. Trying it now... \n",password);
            strcpy(dos_try,dos_string);
            strcat(dos_try,password);
            strcat(dos_try," > zipcrack.err");
            result=system(dos_try);
            if(!chk_4_errors())
            {
                // we hack success!
                strcpy(passcode,password);
                return(1);
            }
            printf("...failed\n");
            system(delete_file);
            update_password(password);
            if(strlen(password)>maxdighack){
                return(0);
                end=2;
            }
        }
    }// end while //
}


//////////////////////////////////////////////////////////////////////////
//
// chk_4_errors() - search the zipcrack.err file for the error_sig -
//		"error!".  If found return 1 else return 0
//
/////////////////////////////////////////////////////////////////////////
int
chk_4_errors()
{
    FILE *errfile;
    int end;
    int i;
    char buffer[10];

    if ((errfile = fopen("zipcrack.err","r+b")) == NULL )
    {
        printf("\nCannot find zipcrack.err file.");
        printf("\nCannot continue.\n");
        exit(1);
    }

    i=0;
    end=0;
    while(!end)
    {
        if(fread(buffer, sizeof(char),1,errfile))
        {
            if(buffer[0] == error_sig[i])
                i = i + 1;
            else
                i = 0;
            if (i == 7)
            {                   // sig is 4 digits
                close(errfile);
                return(1);
            }
        }
        else
        	end=1;
    }
    close(errfile);
    return(0);
}

//////////////////////////////////////////////////////////////////////////
//  update_password(password) - we get the next password to try here.
//      This is the routine you would modify to make a dictionary or
//      other than sequential hack.   Also you can modify the range
//      of char's to try here.
///////////////////////////////////////////////////////////////////////
update_password(password)
char *password;
{
    int g,i,len,err;
    char newlen[12];

    err=0;

    len = strlen(password);
    i = len-1;
    g = 0;

    while(!err){
        password[i] = password[i] + 1;      // inc to next char in series
        if(password[i] > 'z') {
            password[i] = 'a';
            i = i - 1;
            g = g + 1;
            if(g==2)
                printf("password = %s \015",password);
        }else
            err = 1;

        if(i<0) {                   // we need to increase the length by one
    //      strcpy(newlen,preamble);    // I was thinking about a pre and
                                        // postamble in case you knew the
                                        // first or las couple of letters
                                        // of the password
            strcpy(newlen,"a");     // new start of string //
            strcat(newlen,password);
            strcpy(password,newlen);
            err = 1;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// decript_head(buffer,key) - this is where the actual work takes place,
//      We decript the buffer with the key generated with our password and
//      check if we have a CRC match, if we do return with a 1.
////////////////////////////////////////////////////////////////////////////
decript_head(buffer,key)
    unsigned char *buffer;
    unsigned long *key;
{
    int i;
    unsigned int crc1;
    char c;

    for(i = 0; i<12 ; i++)
    {
        c = buffer[i] ^ decrypt_byte(key);    /* buffer xor decrypt_byte */
        update_keys(key,c);
        buffer[i] = c;
    }
    crc1 = ((buffer[11]<<8)|(buffer[10]));

    /* crc1 = ~crc1; */
    if(crc1 == crc2find)
    {
        printf("Yippi Ki-aye M*&!#R F*^@#R ");
        return(1);
    }
    return(0);
}


update_keys(key,val)
unsigned long *key;
char    val;
{
    key[0] = crc32(key[0],val);
    key[1] = key[1] + (key[0] & 0x000000ff);
    key[1] = key[1] * 134775813 + 1;
    key[2] = crc32(key[2],(key[1] >> 24));
}


unsigned char
decrypt_byte(key)
    unsigned long *key;
{
    unsigned short temp;

    temp = key[2] | 2;
    return((temp*(temp^1))>>8);
}


//
// cRc routine
//
unsigned long
crc32(oldcrc, newchar)
    USGL oldcrc;
    char newchar;
{
    int i,index;
    USGL Crc;

    Crc = oldcrc;

        index = ((Crc ^ newchar) & 0x000000ffL);
        Crc = ((Crc >> 8) & 0x00ffffffL) ^ crctable[index];

    return Crc;            /* return a 1's complement */
}


void generatetable()        /* generate the crc look-up table */
{
    union
    { USGI i;
        struct
        {
            USGI i1 :1;     /* MSC low order bit */
            USGI i2 :1;
            USGI i3 :1;
            USGI i4 :1;
            USGI i5 :1;
            USGI i6 :1;
            USGI i7 :1;
            USGI i8 :1;     /* MSC high order bit */
            USGI    :8;     /* unused bits        */
        }bit;
    }iUn;

    union
    { USGL entry;
        struct
        {
            USGI b1 :1;     /* MSC low order bit */
            USGI b2 :1;
            USGI b3 :1;
            USGI b4 :1;
            USGI b5 :1;
            USGI b6 :1;
            USGI b7 :1;
            USGI b8 :1;
            USGI b9 :1;
            USGI b10:1;
            USGI b11:1;
            USGI b12:1;
            USGI b13:1;
            USGI b14:1;
            USGI b15:1;
            USGI b16:1;
            USGI b17:1;
            USGI b18:1;
            USGI b19:1;
            USGI b20:1;
            USGI b21:1;
            USGI b22:1;
            USGI b23:1;
            USGI b24:1;
            USGI b25:1;
            USGI b26:1;
            USGI b27:1;
            USGI b28:1;
            USGI b29:1;
            USGI b30:1;
            USGI b31:1;
            USGI b32:1;     /* MSC high order bit */
        } entrybit;
    }entryun;

    for (iUn.i = 0; iUn.i < 256; iUn.i++)
    {
        entryun.entry = 0;     /* zero out the value */

        entryun.entrybit.b32 = (iUn.bit.i2 ^ iUn.bit.i8);
        entryun.entrybit.b31 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i7 ^ iUn.bit.i8);
        entryun.entrybit.b30 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i6 ^ iUn.bit.i7 ^
                                iUn.bit.i8);
        entryun.entrybit.b29 = (iUn.bit.i1 ^ iUn.bit.i5 ^
                                iUn.bit.i6 ^ iUn.bit.i7);

        entryun.entrybit.b28 = (iUn.bit.i2 ^ iUn.bit.i4 ^
                                iUn.bit.i5 ^ iUn.bit.i6 ^
                                iUn.bit.i8);
        entryun.entrybit.b27 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i3 ^ iUn.bit.i4 ^
                                iUn.bit.i5 ^ iUn.bit.i7 ^
                                iUn.bit.i8);
        entryun.entrybit.b26 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i3 ^ iUn.bit.i4 ^
                                iUn.bit.i6 ^ iUn.bit.i7 );
        entryun.entrybit.b25 = (iUn.bit.i1 ^ iUn.bit.i3 ^
                                iUn.bit.i5 ^ iUn.bit.i6 ^
                                iUn.bit.i8);

        entryun.entrybit.b24 = (iUn.bit.i4 ^ iUn.bit.i5 ^
                                iUn.bit.i7 ^ iUn.bit.i8);
        entryun.entrybit.b23 = (iUn.bit.i3 ^ iUn.bit.i4 ^
                                iUn.bit.i6 ^ iUn.bit.i7);
        entryun.entrybit.b22 = (iUn.bit.i3 ^ iUn.bit.i5 ^
                                iUn.bit.i6 ^ iUn.bit.i8);
        entryun.entrybit.b21 = (iUn.bit.i4 ^ iUn.bit.i5 ^
                                iUn.bit.i7 ^ iUn.bit.i8);

        entryun.entrybit.b20 = (iUn.bit.i2 ^ iUn.bit.i3 ^
                                iUn.bit.i4 ^ iUn.bit.i6 ^
                                iUn.bit.i7 ^ iUn.bit.i8 );
        entryun.entrybit.b19 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i3 ^ iUn.bit.i5 ^
                                iUn.bit.i6 ^ iUn.bit.i7 );
        entryun.entrybit.b18 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i4 ^ iUn.bit.i5 ^
                                iUn.bit.i6 );
        entryun.entrybit.b17 = (iUn.bit.i1 ^ iUn.bit.i3 ^
                                iUn.bit.i4 ^ iUn.bit.i5 );

        entryun.entrybit.b16 = (iUn.bit.i3 ^ iUn.bit.i4 ^
                                iUn.bit.i8);
        entryun.entrybit.b15 = (iUn.bit.i2 ^ iUn.bit.i3 ^
                                iUn.bit.i7);
        entryun.entrybit.b14 = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i6);
        entryun.entrybit.b13 = (iUn.bit.i1 ^ iUn.bit.i5);

        entryun.entrybit.b12 = (iUn.bit.i4);
        entryun.entrybit.b11 = (iUn.bit.i3);
        entryun.entrybit.b10 = (iUn.bit.i8);
        entryun.entrybit.b9  = (iUn.bit.i2 ^ iUn.bit.i7 ^
                                iUn.bit.i8);

        entryun.entrybit.b8  = (iUn.bit.i1 ^ iUn.bit.i6 ^
                                iUn.bit.i7);
        entryun.entrybit.b7  = (iUn.bit.i5 ^ iUn.bit.i6 );
        entryun.entrybit.b6  = (iUn.bit.i2 ^ iUn.bit.i4 ^
                                iUn.bit.i5 ^ iUn.bit.i8);
        entryun.entrybit.b5  = (iUn.bit.i1 ^ iUn.bit.i3 ^
                                iUn.bit.i4 ^ iUn.bit.i7 );

        entryun.entrybit.b4  = (iUn.bit.i2 ^ iUn.bit.i3 ^
                                iUn.bit.i6);
        entryun.entrybit.b3  = (iUn.bit.i1 ^ iUn.bit.i2 ^
                                iUn.bit.i5);
        entryun.entrybit.b2  = (iUn.bit.i1 ^ iUn.bit.i4);
        entryun.entrybit.b1  = (iUn.bit.i3);

        crctable[iUn.i] = entryun.entry;
    }
}


---Cut---

--------mycal@netacsys.com--------------------------------------------------
Newmerica is here and will operate on two basic fundimentals :
        1) Make every thing illegal
        2) Enforce as needed
Any Questions?
