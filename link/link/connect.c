
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <termios.h>

#define FLUSHTIME  50000    /* .5 seconds */
#ifndef PORT
#define PORT 3012
#endif  PORT

/* i/o from common.c */
#define OSIZE 128
char loc_out[OSIZE],rem_out[OSIZE];
int loc_len=0,rem_len=0;
#define LOCAL(fd)  if(loc_len&&write(fd,loc_out,loc_len)) loc_len=0
#define REMOTE(fd) if(rem_len&&write(fd,rem_out,rem_len)) rem_len=0
struct termios save;    /* a copy of the original attributes */

/* DEBUG */
int errfd;  /* debugging fd */
int debug;

do_connect()
{
  int s,s2;
  struct sockaddr_in addr;

  s=socket(AF_INET,SOCK_STREAM,0);
  if(s<0) {
    printf("cant grab a socket\n");
    return(-1);
  }
  addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(PORT);
  addr.sin_family = AF_INET;

  if(connect(s,&addr,sizeof(addr))==-1)
    return(-1);
  else
    return(s);
}

int done();

main()
{
  int s;

/*DEBUG*/
  errfd=open("Error.con",O_WRONLY|O_CREAT|O_TRUNC,0666);
  dup2(errfd,2);
  debug=open("Debug.con",O_WRONLY|O_CREAT|O_TRUNC,0666);
  s=  do_connect();  
  if(s<0) {
    printf("Error..  couldnt connect.\n");
    exit(1);
  }
  if (tcgetattr(0,&save) == -1) {
    printf("Ouch, cant get attributes of your stdin!.\n");
    exit(1);
  }
  fcntl(s,F_SETFL,O_NDELAY);
#ifndef DEBUG
  fcntl(0,F_SETFL,O_NDELAY);
  signal(SIGINT,done);   /* */
  signal(SIGCLD,done);
  signal(SIGSEGV,done);
  signal(SIGQUIT,done);  /* */
  signal(SIGBUS,done);
#endif DEBUG
  do_copy(s);

  done(0);    /* put things back */
  return;
}

int numin=1;
int numout=1;

done(i)    /* signals send us here,  we put back things that we touched */
int i; 
{
  if(i)
    printf("\nSignal #%d\n\n",i);
/*
  printf("packet bytes %d\n",numin);
  printf("data bytes   %d\n",numout);
  printf("efficiency   %2.2d\n",100*numout/numin);
*/
  tcsetattr(0,TCSANOW,save);
  fcntl(0,F_SETFL,   
            fcntl(0,F_GETFL,0) & ~O_NDELAY);
  exit(0); 
}


condition_tty(t)
struct termios *t;
{
  t->c_lflag &= ~ICANON;   /* async */
  t->c_lflag &= ~ECHO;      /* no echo */
 /* t->c_lflag &= ~ISIG;      /* no signals */
  t->c_cc[VMIN] = 1;        /* 1 character at least */
  t->c_cc[VTIME]= 0;        /* no time-out */

  /* am i missing anythin?  */

}


do_copy(remote)           /* this one is opposite of links one */
int remote;
{
  struct fd_set rd,wr,ex;
  struct timeval timeout;
  struct termios attr;
#define BUFSIZE 8
  char buffer[BUFSIZE];
  int a;
  
  timeout.tv_sec  = 0;
  timeout.tv_usec = FLUSHTIME;
  if(startup(remote,1)<0)              /* we request a new key */
    return;
  attr=save;                           /* get saved tty attributes */ 
  if(tcgetattr(0,&attr)==-1){
    printf("couldnt get attributes.\n");
    exit(1);
  }
  condition_tty(&attr);
#ifndef DEBUG
  tcsetattr(0,TCSANOW,attr);
#endif DEBUG
  while(1) {       /* SIGCLD gets us out of this loop */
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&ex); 
    FD_SET(0,&rd);       /* can we read from remote or stdin */
    FD_SET(remote,&rd);
    if(select(NFDBITS,&rd,&wr,&ex,&timeout)!=-1) {
      if(FD_ISSET(0,&rd)) {
        a=read(0,buffer,BUFSIZE);
        if(a<0) return;
        en_out(buffer,a);     /* encrypt and send to remote */
        REMOTE(remote);
      }
      else {
        en_out(buffer,0);    /* might flush */
        REMOTE(remote);
      }
      if(FD_ISSET(remote,&rd)) {
        a=read(remote,buffer,BUFSIZE);
        if(a<0) return;
        de_out(buffer,a);          /* decrypt and output */
        LOCAL(0);
      }
    }  /* if select */
  }    /* while 1   */
}

