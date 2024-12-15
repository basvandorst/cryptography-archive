/*
 *   Program to create .KST files for authorised users of PC-SCRAMBLER
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#include <stdio.h>
#include <miracl.h>

#define DIGITS 155    /* >= 155 */

static char fname[20];
static char *fixed="AUTHORISED PC-SCRAMBLER USER -> ";
static char string[100];
static char work[200];
static char ptext[200];
static char qtext[200];

/* NOTE: This one way function is experimental */

static void onewayhash(string,n,x)
char *string;      
big n,x;
{ /* nonlinear (portable) permutation *
   * x=(fixed|string xor R(n))^2 mod n      */
    int i,len;
    char t;
    static char buff[256];
    strcpy(work,fixed);
    strcat(work,string);
    INPLEN=otstr(n,buff);
    for (i=0;i<INPLEN/2;i++)
    { /* swap bytes of buff */
        t=buff[i];
        buff[i]=buff[INPLEN-i-1];
        buff[INPLEN-i-1]=t;
    }
    len=strlen(work);
    for (i=0;i<len;i++) buff[i]^=work[i];
    instr(x,buff);
    mad(x,x,x,n,n,x);
}

static void strongp(p,rd,n,type)
big p,rd;
int n,type;
{ /* generate strong prime number p suitable  for encryption. *
   * p will be > n decimal digits in length, such that p=2q+1 *
   * where q is also prime. If type=0  then q = 1 mod 4,      *
   * otherwise q= 3 mod 4. (McCurley's construction)          *
   * See "A Key Distribution System Equivalent to Factoring", *
   * Kevin McCurley, Journal of Cryptology, 1988 pp95-105     *
   * Note that the primes p generated are of the form         *
   * p = 11 mod 12 such that both p = 3 mod 4 and p = 2 mod 3 *
   * The product of two such primes is both a Blum Integer,   *
   * and also suitable for RSA use with exponent of 3         *
   * rd is a large 'randomising' contribution                 */
    big q,tq,w;
    int k,rem,sp;
    q=mirvar(0);
    tq=mirvar(0);
    w=mirvar(0);
    bigdig(n,10,q);
    add(q,rd,q);
    rem=subdiv(q,12,w);
    if (type==0) incr(q,5-rem,q);     /* q = 5 mod 12  */
    else         incr(q,11-rem,q);    /* q = 11 mod 12 */
    forever
    { /* find suitable prime */
        incr(q,12,q);
        if (!isprime(q)) continue;  /* is q prime? */
        premult(q,2,tq);            
        incr(tq,1,p);
        k=0;
        while ((sp=PRIMES[k++])!=0) if (subdiv(p,sp,w)==0) break;
        if (sp!=0) continue;
        powltr(3,tq,p,w);
        if (size(w)==1) break;   /* is p=2.q+1 prime? */
    }
    free(w);
    free(tq);
    free(q);
}

void main(argc,argv)
int argc;
char **argv;
{ /* create members of network of authorised users of PC-SCRAMBLER */
    int i,k,ch;
    long seed;
    big p[2],m,ke,kd,ep[2],kp[2],w,root;
    FILE *fp;
    argv++;
    argc--;
    mirsys(200,256);
    gprime(15000);
    for (i=0;i<2;i++)
    {
        p[i]=mirvar(0);
        ep[i]=mirvar(0);
        kp[i]=mirvar(0);
    }
    w=mirvar(0);
    m=mirvar(0);
    ke=mirvar(0);
    kd=mirvar(0);
    root=mirvar(0);
    IOBASE=60;
    if (argc!=1)
    {
        printf("Incorrect Usage\n");
        printf("keyids <name>\n");
        printf("where <name> is the name of the Network which is to be created or added to.\n");
        exit(0);
    }

    strcpy(fname,argv[0]);
    strcat(fname,".IDS");
    if ((fp=fopen(fname,"r"))!=NULL)
    { /* read in key data */
        cinnum(p[0],fp);
        cinnum(p[1],fp);
        fclose(fp);
        printf("Adding new member(s) to existing Network - %s\n",
               argv[0]);
    }
    else
    {
        printf("Creating a new Network - %s\n",argv[0]);
        printf("Enter 9  digit random number = ");
        scanf("%ld",&seed);
        getchar();
        irand(seed);
        IOBASE=10;
        printf("Enter 25 digit random number = ");
        cinnum(w,stdin);
        IOBASE=60;
        printf("generating key - please wait\n");

        strongp(p[0],w,(DIGITS-5)/2,0);
        strongp(p[1],w,(DIGITS+5)/2,1);

        printf("\n");
        printf("If the Network file is saved new  members can be added to the\n");
        printf("group at a later stage. However if the %s.IDS file created\n",
              argv[0]);
        printf("were to fall into the wrong hands the security of the Network\n");
        printf("would be compromised!\n\n");
        printf("Save secret Network file (Y or N)? ");
        ch=getchar();
        getchar();
        if (ch=='Y' || ch=='y')
        {
            fp=fopen(fname,"w");
            cotnum(p[0],fp);
            cotnum(p[1],fp);
            fclose(fp);
        }
    }

  /* calculate signing key */

    convert(1,ke);
    convert(1,kd);
    for (i=0;i<2;i++)
    {
        multiply(ke,p[i],ke);
        decr(p[i],1,p[i]);
        multiply(kd,p[i],kd);
    }
    premult(kd,2,kd);
    incr(kd,1,kd);
    subdiv(kd,3,kd);
    for (i=0;i<2;i++)
    {
        copy(kd,kp[i]); 
        divide(kp[i],p[i],p[i]);   /* kp = kd mod (p-1) */
        incr(p[i],1,p[i]);
    }
    crt_init(2,p);  /* initialise for application of CRT */

    printf("\nCreate key files for new users - press return on its own when finished\n");
    forever
    { /* create .KST files for users */
        printf("\nFilename (.KST extension understood): ");
        gets(fname);
        if (strlen(fname)==0) break;
        strcat(fname,".kst");
        fp=fopen(fname,"w");
        printf("Input identifying text - one line only < 36 chars \n");
        printf("                                    <\n");
        gets(string);
        string[36]=0;
        instr(w,string);
  
  /* randomise text to prevent multiplicative forgeries */
    
        onewayhash(string,ke,m);

  /* sign text - pushing it through the RSA trapdoor */

        for (i=0;i<2;i++) powmod(m,kp[i],p[i],ep[i]);  
        crt(ep,root);   /* Chinese Remainder Thereom */

        otnum(w,fp);
        fprintf(fp,"\n");
        cotnum(root,fp);
        cotnum(ke,fp);
        fclose(fp);
    }
    zero(p[0]);   /* throw away factors of ke */
    zero(p[1]);
    crt_end();
    printf("\nEach .KST  file  should now be distributed to a member of the\n");
    printf("secure Network, each of whom should then rename their file to\n");
    printf("SCRAMBLE.KST, for use with the main SCRAMBLER program.\n");
}

