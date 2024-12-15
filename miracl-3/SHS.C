/*
 * Implementation of the Draft Secure Hashing Standard (SHS)
 * specified for use with the NIST Digital Signature Standard (DSS)
 *
 * Generates a 160 bit message digest. It should be impossible to come
 * come up with two messages that hash to the same value ("collision free").
 *
 * For use with byte-oriented messages only. Could/Should be speeded
 * up by unwinding loops in shs_transform(), and assembly patches.
 */

#define H0 0x67452301L
#define H1 0xefcdab89L
#define H2 0x98badcfeL
#define H3 0x10325476L
#define H4 0xc3d2e1f0L

#define K0 0x5a827999L
#define K1 0x6ed9eba1L
#define K2 0x8f1bbcdcL
#define K3 0xca62c1d6L

#define PAD  0x80
#define ZERO 0

/* functions */

#define S(n,x) ((x<<n) | (x>>(32-n)))

#define F0(x,y,z) ((x&y) | (~x&z))
#define F1(x,y,z) (x^y^z)
#define F2(x,y,z) ((x&y) | (x&z) | (y&z)) 
#define F3(x,y,z) (x^y^z)

/* local workspace */

static unsigned long length[2],h[5],w[80];

/* prototypes */

void shs_init(void);
void shs_transform(void);
void shs_process(int);
void shs_hash(unsigned char *);

void shs_transform()
{ /* basic transformation step */
    unsigned long a,b,c,d,e,temp;
    int t;
    for (t=16;t<80;t++) w[t]=w[t-3]^w[t-8]^w[t-14]^w[t-16];
    a=h[0]; b=h[1]; c=h[2]; d=h[3]; e=h[4];
    for (t=0;t<20;t++)
    { /* 20 times - mush it up */
        temp=K0+F0(b,c,d)+S(5,a)+e+w[t];
        e=d; d=c;
        c=S(30,b);
        b=a; a=temp;
    }
    for (t=20;t<40;t++)
    { /* 20 more times - mush it up */
        temp=K1+F1(b,c,d)+S(5,a)+e+w[t];
        e=d; d=c;
        c=S(30,b);
        b=a; a=temp;
    }
    for (t=40;t<60;t++)
    { /* 20 more times - mush it up */
        temp=K2+F2(b,c,d)+S(5,a)+e+w[t];
        e=d; d=c;
        c=S(30,b);
        b=a; a=temp;
    }
    for (t=60;t<80;t++)
    { /* 20 more times - mush it up */
        temp=K3+F3(b,c,d)+S(5,a)+e+w[t];
        e=d; d=c;
        c=S(30,b);
        b=a; a=temp;
    }
    h[0]+=a; h[1]+=b; h[2]+=c;
    h[3]+=d; h[4]+=e;
} 

void shs_init()
{ /* re-initialise */
    int i;
    for (i=0;i<80;i++) w[i]=0L;
    length[0]=length[1]=0L;
    h[0]=H0;
    h[1]=H1;
    h[2]=H2;
    h[3]=H3;
    h[4]=H4;
}

void shs_process(byte)
int byte;
{ /* process the next message byte */
    int cnt;
    
    cnt=((length[0]/32)%16);
    
    w[cnt]<<=8;
    w[cnt]|=(unsigned long)byte;

    length[0]+=8;
    if (length[0]==0L) { length[1]++; length[0]=0L; }
    if ((length[0]%512)==0) shs_transform();
}

void shs_hash(hash)
unsigned char hash[20];
{ /* pad message and finish - supply 20 byte digest */
    int i;
    unsigned long len0,len1;
    len0=length[0];
    len1=length[1];
    shs_process(PAD);
    while ((length[0]%512)!=448) shs_process(ZERO);
    w[14]=len1;
    w[15]=len0;    
    shs_transform();
    for (i=0;i<20;i++)
    { /* convert to bytes */
        hash[i]=(unsigned char)((h[i/4]>>(8*(i%4))) & 0xffL);
    }
    shs_init();
}

/* test program: should produce digest
   e16e51d2 af5bfaac c4c1df33 4438e471 c834f19e 



#include <stdio.h>

char test[]="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

int main()
{
    unsigned char hash[20];
    int i;
    shs_init();
    for (i=0;test[i]!=0;i++) shs_process(test[i]);
    shs_hash(hash);    
    for (i=0;i<20;i++) printf("%x",hash[i]);
}
*/

