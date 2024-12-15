/* this is probably the most system dependent part of the package */
/* prandom.c - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

static struct termios tty_orig,tty_new;
static FILE *tty;

#define RBUFLEN 65536

#ifndef DEBUG
#define seedin(p,obj) \
{ \
    if ((j + sizeof(obj) < RBUFLEN) && initialised) {\
    memcpy(&rbuf[j],p,sizeof(obj)); \
    j+=sizeof(obj); \
    } \
} 
#else
#define seedin(p,obj) \
    if (j + sizeof(obj) < RBUFLEN && initialised) {\
    memcpy(&rbuf[j],p,sizeof(obj)); \
    fprintf(stderr, "seedin (%d,%d)--",sizeof(obj),j); \
    for (kk = 0; kk<sizeof(obj);kk++) \
        fprintf(stderr, "%02x", rbuf[j + kk]); \
    fprintf(stderr, "\n"); \
    j+=sizeof(obj); \
} 
int kk;
#endif


/* add your system here if you can do getrusage */
#if defined(linux) || defined (_AIX) || defined(sun) || defined(sgi)
#define GETRUSAGE
#endif

#ifdef GETRUSAGE
#include <sys/resource.h>
#endif

static unsigned char rbuf[RBUFLEN];
static int j = 0;   /* keeps track of number of bytes seeded in */
static int initialised = 0;

unsigned char b1[16];
unsigned char b2[16];
unsigned char b3[16];
unsigned char b4[16];

static void eh(int n)
{
    tcdrain(fileno(tty));
    tcsetattr(fileno(tty), TCSAFLUSH, &tty_orig);
    fprintf(stderr, "\nreceived signal %d -- execution aborted\n",n);
    exit(1);
}

static void install_handlers()
{
#ifdef SIGINT
signal(SIGINT, eh);
#endif
#ifdef SIGHUP
signal(SIGHUP, eh);
#endif
signal(SIGHUP, eh);
#ifdef SIGSEGV
signal(SIGSEGV, eh);
#endif
#ifdef SIGBUS
signal(SIGBUS,eh);
#endif
#ifdef SIGEMT
signal(SIGEMT,eh);
#endif
#ifdef SIGILL
signal(SIGILL, eh);
#endif
#ifdef SIGQUIT
signal(SIGQUIT, eh);
#endif
#ifdef SIGTERM
signal(SIGTERM, eh);
#endif
}
static void restore_default_handlers()
{
#ifdef SIGINT
signal(SIGINT, SIG_DFL);
#endif
#ifdef SIGHUP
signal(SIGHUP, SIG_DFL);
#endif
signal(SIGHUP, SIG_DFL);
#ifdef SIGSEGV
signal(SIGSEGV, SIG_DFL);
#endif
#ifdef SIGBUS
signal(SIGBUS,SIG_DFL);
#endif
#ifdef SIGEMT
signal(SIGEMT,SIG_DFL);
#endif
#ifdef SIGILL
signal(SIGILL, SIG_DFL);
#endif
#ifdef SIGQUIT
signal(SIGQUIT, SIG_DFL);
#endif
#ifdef SIGTERM
signal(SIGTERM, SIG_DFL);
#endif
}


void initrandom()
{
    struct timeval foo;
    struct tms foo2;
    struct timezone foo1;
    struct stat qq;
    int i;
#ifdef GETRUSAGE
    struct rusage foo3;
#endif

    if (!initialised) {
        initialised = 1;
        j = 0;
        stat(".",&qq);
        seedin(&qq,struct stat);
        gettimeofday(&foo,&foo1);   
        seedin(&foo,struct timeval);
        i=getpid(); seedin(&i,int);
        i=getppid(); seedin(&i,int);
        i=times(&foo2);
        seedin(&foo2,struct tms);
#ifdef GETRUSAGE
        getrusage(RUSAGE_SELF, &foo3);
        seedin(&foo3, struct rusage);
#endif
    }
}

void seedrandom(unsigned char *buf, unsigned int len)
{
    int i;
    if (j < RBUFLEN) {

        /* if we're going to overflow toss some data */ 
        if (j+len >= RBUFLEN) 
            len = RBUFLEN - 1 - j;
    
        memcpy(&rbuf[j],buf,len); j+=len;
    }
#ifdef DEBUG
    fprintf(stderr," seedrandom - %d bytes\n", len);
    for (i=0;i<len;i++)
        fprintf(stderr,"%02x", *(buf+i));
    fprintf(stderr, "\n");
#endif
}

/* get a string (and some random data)
 if pwd then the string is a password string and is not
    seeded in (and is returned)
 if (!pwd) then the string is seeded in with the timing data 
    (and is not returned)
*/
unsigned char *getstring(int pwd)
{
    struct timeval tv; struct timezone tz;
    int i;
    unsigned char *s;
    unsigned int len = 0;
    long vdis;
#ifdef GETRUSAGE
    struct rusage foo3;
#endif

    if (pwd)
        s=(unsigned char *)malloc(1024);

    if ((tty=fopen("/dev/tty","r")) == NULL) {
        fprintf(stderr, "cannot open /dev/tty\n");
        return NULL;
    }

    tcgetattr(fileno(tty), &tty_orig);
    install_handlers();
    memcpy(&(tty_new),&(tty_orig),sizeof(tty_orig));

#ifdef SPECCHAR
    /* allow special characters in the "random" string */
    if ((vdis = fpathconf(fileno(tty),_PC_VDISABLE)) < 0) {
        fprintf(stderr, 
            "warning: fpathconf error or _POSIX_VDISABLE not in effect\n");
    }
    else  {
        tty_new.c_cc[VINTR] = vdis;
        tty_new.c_cc[VQUIT] = vdis;
        tty_new.c_cc[VSUSP] = vdis;
    }
#endif

    tty_new.c_lflag &= ~(ECHO | ICANON);
    tty_new.c_cc[VMIN] = 1;
    tty_new.c_cc[VTIME] = 0;
    tcdrain(fileno(tty));
    tcsetattr(fileno(tty), TCSAFLUSH, &tty_new);

    do {
        i = getc(tty);
        if (!pwd) {
            seedin(&i,int);
        }

        if (pwd && len < 1024) 
            s[len++] = i;
            
        gettimeofday(&tv,&tz);
        seedin(&tv,struct timeval);
#ifdef GETRUSAGE
        getrusage(RUSAGE_SELF, &foo3);
        seedin(&foo3, struct rusage);
#endif
    }
    while (! (i == '\n' || i == (-1)));
    tcdrain(fileno(tty));
    tcsetattr(fileno(tty), TCSAFLUSH, &tty_orig);
    restore_default_handlers();
    putc('\n', stderr);
    fclose(tty);
    if (pwd) {
        s[len]=0;
        return(s);
    }
    else
        return NULL;
}

unsigned char randombyte()
{
    static int firstcall = 1;
    int i,ii;
    unsigned char s;
    static int used = 0;
    if (firstcall) {        /* we've got stuff to do */
        MD5_CTX mdContext;
        firstcall = 0;
        MD5Init(&mdContext);
        MD5Update(&mdContext, (unsigned char *) rbuf, j);
        MD5Final(&mdContext);
        memcpy(b1, mdContext.digest, 16);
        des2key(b1,0);
        initkey_idea(b1,0);

        /* idea ecb encrypt most of rbuf according to digest */
        for (i=0;i < j/8; i++)
            idea_ecb((word16 *)&rbuf[i*8], (word16*)&rbuf[i*8]);
        close_idea();
        
        /* generate second part of state */
        for (i=0;i<j;i+=16)
            D2des(&rbuf[i],&b3[0]);
            for (ii=0;ii<16;ii++)
                b2[ii] ^= b3[ii];

        /* our state consists of b1 and b2, b3 will be output */
        
        D2des(&b2[0],&b3[0]);
        initkey_idea(b2,0);
        idea_ecb((word16 *)&b1[0],(word16 *)&b4[8]);
        idea_ecb((word16 *)&b1[8],(word16 *)&b4[0]);
        for (i=0;i<16;i++)
            b3[i] ^= b4[i];

        used = 0;
        zap(rbuf,j);
        close_des();
        close_idea();
    }
    if (used == 16) {

        for (i=0;i<16;i++)
            b1[i] = (b1[15-i]+1) % 0xff;

        des2key(b1,0);
        D2des(&b2[0],&b3[0]);
        initkey_idea(b2,0);
        idea_ecb((word16 *)&b1[0],(word16 *)&b4[8]);
        idea_ecb((word16 *)&b1[8],(word16 *)&b4[0]);
        for (i=0;i<16;i++)
            b3[i] ^= b4[i];

        D2des(&b2[0],&b2[0]);
        used = 0;
        close_des();
        close_idea();
    }
    s=b3[used];
    used++;
#if DEBUG
    fprintf(stderr, "randombyte returns %02x\n", s);
#endif
    return s;
}

void closerandom()
{
    int i;
    for (i=0;i<16;i++) {
        b1[i]=b2[i]=b3[i]=b4[i]=0;
    }
    zap(rbuf,RBUFLEN);
}

#ifdef TEST
main()
{
    unsigned char *s; int i;
    initrandom();
    s=getstring(0); 

    for (i=0;i<100000;i++)
        putchar(randombyte());
}
#endif
    
    
