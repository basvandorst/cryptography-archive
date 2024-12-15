#include <stdio.h>
unsigned char rotor[131072];    /* encryption/decryption rotors */
unsigned char rndout[16960];    /* pseudo-rnd output for block trans */

/*                       cipher

This encryption program uses a generalized rotor encryption system
it is called using the following format:
crypt2 flag1 flag2 file1 file2 CR
flag1 is either a e for encrypt or a d for decrypt
flag2 is 0, 1, or 2 (if flag2 is not present it is assumed to =0)
file1 is the file to be processed
file2 is the file in which the output is stored
the rotor set is read in from a file "rotors ", which must be present

references that explain this encryption system are given in articles by
J. Michener in Cryptologia #1 & 2, 1985

*/

main(argc, argv)
int argc;
char *argv[];
{
int readin();           /* read "blocksize" # of chars into block[] */
void makernd();         /* store necessary PR #'in rndout[] */
void rnd();             /* calculates the PR #'s */
void bittrans();        /* perform bit transposition on 32 byte block */
void bytetrans();       /* perform byte transposition on 256 byte block */
void trans();   /* determine transposition tables used in above routines */
unsigned char sub();    /* perform byte substitution */
unsigned char unsub();  /* perform byte substitution decryption */

FILE *fopen(), *f_input, *f_output, *frotor_set;

unsigned char block[256];  /* material to be transformed is put in here */
int i,j,ios;
int rndofset;   /* offset to access apropriate PR #'s in rndout[] */
int en_de_code; /* flag for encode (=0) or decode */
int blocksize;  /* block size, =1, 32, or 256 */
int bbofset;    /* bit block offset, for bittrans in 256 byte block */

ios=0; /* adjust the arg of argv[] for the number of terms in command line */
blocksize=1;

        /* are there too many or too few arguments */
if((argc<4)||(argc>5))
    {
    fprintf(stderr,"error,use cipher e/d option file_in file_out\n");
    exit(0);
    }

/* determine what the program option is.  No option or a 0 option ==>>
blocksize=1 and the resulting cipher will be a stream substitution cipher.
a 1 option implies a 32 byte bit complete block code, blocksize=32, and
a 2 option implies a 256 byte bit complete block code, blocksize=256. */

if (argc==5)     /* there exists a command line option */
    {
    ios=1;
    if (argv[2][0]=='0')
        blocksize=1;
    else if (argv[2][0]=='1')
        blocksize=32;
    else if (argv[2][0]=='2')
        blocksize=256;
    else
        {
    fprintf(stderr,"improper crypt option, allowed options are 0, 1, 2\n");
        exit(0);
        }
    }

        /* do we need to encode or decode the supplied file? */
if ((argv[1][0]=='e')||(argv[1][0]=='E'))
    en_de_code=0;                       /* encode ==> en_de_code=0 */
else if ((argv[1][0]=='d')||(argv[1][0]=='D'))
    en_de_code=1;                       /* decode ==> en_de_code=1 */
else
    {
    fprintf(stderr,"error,allowed parameters are e=encrypt, d=deccrypt\n");
    exit(0);
    }

                /* read in the rotor contents */
f_input=fopen(argv[2+ios],"r");         /* input file */
f_output=fopen(argv[3+ios],"w");        /* output file */
frotor_set=fopen("rotors","r");         /* rotor sets */

fread(rotor,1,131072,frotor_set);       /* read in rotor sets */
fclose (frotor_set);

makekey();              /* get and calculate the key */

if (blocksize==1)       /* stream substitution cipher */
    {
    rndofset=0; /* stream cipher, need only 8 pseudo-random # */
    if (en_de_code==0)  /* encrypt file */
        {
        while((readin(1,block,f_input))!=0)
            {
            makernd(1);
            block[0]=sub(block[0],0);
            fwrite(block,1,1,f_output);
            }
        exit(0);
        }
    else
        {
        while((readin(1,block,f_input))!=0)
            {
            makernd(1);
            block[0]=unsub(block[0],0);
            fwrite(block,1,1,f_output);
            }
        exit(0);
        }
    }

else if (blocksize==32) /* 32 byte bit complete block code */
    {
    if (en_de_code==0)  /* encrypt file */
        {
        while((readin(32,block,f_input))!=0)
            {
            makernd(32);
            rndofset=0;
            bbofset=0;
            for(j=0;j<6;j++)
                {
                bittrans(block,0,rndofset,0);
                rndofset+=8;
                for(i=0;i<32;i++)
                    {
                    block[i]=sub(block[i],rndofset);
                    rndofset+=8;
                    }
                }
            fwrite(block,1,32,f_output);
            }
        exit(0);
        }
    else
        {
        while((readin(32,block,f_input))!=0)
            {
            makernd(32);
            bbofset=0;
            rndofset=1576;      /* (32+1)*6*8-8 */
            for(j=0;j<6;j++)
                {
                for(i=31;i>=0;i--)
                    {
                    block[i]=unsub(block[i],rndofset);
                    rndofset-=8;
                    }
                bittrans(block,0,rndofset,1);
                rndofset-=8;
                }
            fwrite(block,1,32,f_output);
            }
        exit(0);
        }
    }

else    /* blocksize=256,   256 byte bit complete block code */
    {
    if (en_de_code==0)  /* encrypt file */
        {
        while((readin(256,block,f_input))!=0)
            {
            makernd(256);
            rndofset=0;
            for(j=0;j<8;j++)
                {
                bytetrans(block,rndofset,0);
                rndofset+=8;
                for(bbofset=0;bbofset<256;bbofset+=32)
                    {
                    bittrans(block,bbofset,rndofset,0);
                    rndofset+=8;
                    }
                for(i=0;i<256;i++)
                    {
                    block[i]=sub(block[i],rndofset);
                    rndofset+=8;
                    }
                }
            fwrite(block,1,256,f_output);
            }
        exit(0);
        }
    else
        {
        while((readin(256,block,f_input))!=0)
            {
            makernd(256);
            rndofset=16952;     /* (256+8+1)*8*8-8 */
            for(j=0;j<8;j++)
                {
                for(i=255;i>=0;i--)
                    {
                    block[i]=unsub(block[i],rndofset);
                    rndofset-=8;
                    }
                for(bbofset=224;bbofset>=0;bbofset-=32)
                    {
                    bittrans(block,bbofset,rndofset,1);
                    rndofset-=8;
                    }
                bytetrans(block,rndofset,1);
                rndofset-=8;
                }
            fwrite(block,1,256,f_output);
            }
        exit(0);
        }
    }
}
/*________________________________________________________________*/

unsigned char sub(ch,ro)
unsigned char ch;       /* character to be encrypted */
int ro;                 /* offset in random number array */
    {
    int i;

    for(i=0;i<4;i++)
ch=rotor[(int)((rndout[i+ro])<<8)+(int)((rndout[i+4+ro]+ch)&255)];
    return(ch);
    }

/*________________________________________________________________*/
unsigned char unsub(ch,ro)
unsigned char ch;       /* character to be decrypted */
int ro;                 /* offset in random number array */
    {
    int i;

    for(i=3;i>=0;i--)
ch=(rotor[65536+(int)((rndout[i+ro])<<8)+(int)ch]-rndout[i+4+ro])&255;
    return(ch);
    }

/*________________________________________________________________*/

int readin(blocksize,block,f_input)
int blocksize;
unsigned char *block;
FILE *f_input;
{
int i,num;

num=fread(block,1,blocksize,f_input);
if (num==0)
    return(0);
if (num<blocksize)
    for(i=num;i<blocksize;i++)
        block[i]=' ';
return(blocksize);
}
/*________________________________________________________________*/

/* the following union is used by the pseudo-random number generators */
union   {
unsigned long keygen[512]; /* random number generator contents */
unsigned char ca[2048];
        } rd;

/* PR generator parameters "r"suffix implies fixed reference values */
int ros[8]={'\020','\047','\100','\135','\174','\235','\306','\365'};
int roind[8]={'\005','\031','\056','\102','\152','\211','\261','\333'};
int roedge[8]={'\021','\050','\101','\136','\175','\236','\307','\366'};
int os[8],oind[8];      /* operational vaues */
int rndpos;

/*________________________________________________________________*/
void makekey()  /* read in key and expand it for the GR system */
{
int c,i,j,len,seed;

do
    {
    len=0;
    printf("\n enter key, key not a word, key>=8 char\n");

    while((c=getchar())!='\n')
        rd.ca[len++]=c;
    } while(len<8);

if((len&1)==0)          /* make len odd to distribute the characters */
    {
    rd.ca[len-1]+=rd.ca[len];
    len--;
    }

seed=(30>len) ? 30 : len;
j=i=0;
while(i<seed)   /* fill the array sequentially with pwd */
    {
    rd.ca[i++]=rd.ca[j++];
    if (j==len)
        j=0;
    }

/* randomize the array by rotor substitutions in auto-code fashion*/
i=0;
j=28;
for(i=0;i<4000;i++)
    {
    j++;
    if(j==2048)
        j=0;

rd.ca[j]=rotor[((int)((rd.ca[j-1])<<8))+(int)((rd.ca[j-2]+rd.ca[j])&255)];
rd.ca[j]=rotor[((int)((rd.ca[j-5])<<8))+(int)((rd.ca[j-8]+rd.ca[j])&255)];
rd.ca[j]=rotor[((int)((rd.ca[j-14])<<8))+(int)((rd.ca[j-21]+rd.ca[j])&255)];
rd.ca[j]=rotor[((int)((rd.ca[j-24])<<8))+(int)((rd.ca[j-29]+rd.ca[j])&255)];
    }

for(i=0;i<8;i++)
    os[i]=ros[i], oind[i]=roind[i];
return;
}

/*________________________________________________________________*/
void makernd(blocksize) /* store appropriate # of PR #'s in rndout */
int blocksize;
{
int offset;

rndpos=0;
if(blocksize==1)
    rnd(0);             /* rnd() performs PR # generation */

else if (blocksize==32)
    for(offset=0;offset<1584;offset+=8)
        rnd(offset);

else
    for(offset=0;offset<16960;offset+=8)
        rnd(offset);
return;
}

/*________________________________________________________________*/
void rnd(rndofset)      /* generate PR #'s, an array of 8 per function call */
int rndofset;   /* store them in rndout sequentially starting */
{               /* with rndofset */
int i;

if(rndpos==4)
    {
    rndpos=0;
    for(i=0;i<8;i++)
        {
        if(++os[i]==roedge[i]);
            os[i]=ros[i];
        if(++oind[i]==roedge[i]);
            oind[i]=roind[i];
        rd.keygen[os[i]]+=rd.keygen[oind[i]];
        }
    }
for(i=0;i<8;i++)
    rndout[i+rndofset]=rd.ca[(os[i]<<2)+rndpos];
rndpos++;

return;
}
/*________________________________________________________________*/

unsigned char transfer[256];    /* transposition transfer table */
unsigned char bitmask[8]={'\1','\2','\4','\10','\20','\40','\100','\200'};
/*________________________________________________________________*/

void bytetrans(block,rndofset,en_de_code)
unsigned char *block;   /* perform byte transposition on block[] */
int rndofset;
int en_de_code; /* flag for forward or reverse transposition */
    {
    int i;
    unsigned char temp[256];

    for(i=0;i<256;i++)
        temp[i]=block[i];

    trans(rndofset,en_de_code);

    for(i=0;i<256;i++)
        block[((int)transfer[i])]=temp[i];

    return;
    }
/*_________________________________________________________________________*/

void bittrans(block,bbofset,rndofset,en_de_code)
unsigned char *block;   /* perform bit transposition on 32 bytes of */
int bbofset;            /* of block[], starting with block[bbofset] */
int rndofset;
int en_de_code;         /* perform forward or reverse transposition */
    {
    int i,j;
    unsigned char temp[32];

    for(i=0;i<32;i++)
        {
        temp[i]=block[j=i+bbofset];
        block[j]=0;
        }

    trans(rndofset,en_de_code);

    for(i=0;i<256;i++)
        if((temp[i>>3] & bitmask[i & 7])!=0)
      block[bbofset+(int)(transfer[i]>>3)] |= bitmask[(int)(transfer[i]&7)];

    return;
    }
/*_________________________________________________________________________*/

void trans(rndofset,en_de_code)
int rndofset;           /* determine transposition mapings needed by */
int en_de_code;         /* bittrans and bytetrans */
    {
    int i;
    if(en_de_code==0)   /* determine encode transposition table */
        for(i=0;i<256;i++)
            transfer[i]=sub((unsigned char)i,rndofset);

    else                /* determine decode transposition table */
        for(i=0;i<256;i++)
            transfer[i]=unsub((unsigned char)i,rndofset);

    return;
    }