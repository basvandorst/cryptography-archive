
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define FLUSHTIME  50000    /* .5 seconds */
/* why does ultrix csh return right away? */
#ifdef ultrix
#define PROGRAM    "/bin/sh","-sh",0
#else
#define PROGRAM    "/bin/csh","-csh",0
#endif

/* i/o from common.c */
#define OSIZE 128
char loc_out[OSIZE],rem_out[OSIZE];
int loc_len=0,rem_len=0;
#define LOCAL(fd)  if(loc_len&&write(fd,loc_out,loc_len)) loc_len=0
#define REMOTE(fd) if(rem_len&&write(fd,rem_out,rem_len)) rem_len=0

/* from common.c */
extern int de_out(),en_out(),startup();

struct termios save;    /* a copy of the original attributes */
int saved=0;

/*DEBUG */
int debug;
done(i)    /* signals send us here,  we put back things that we touched */
int i; 
{
  if(saved) {
    tcsetattr(0,TCSANOW,save);
    fcntl(0,F_SETFL,   
            fcntl(0,F_GETFL,0) & ~O_NDELAY);
  }
  if(i)
    fprintf(stderr,"\nSignal #%d\n\n",i);
/* DEBUG */
close(debug);
  exit(0); 
}

/* assumes stdin == stdout == the terminal connected to the modem*/
/* is this a bad assumption? dont set up signals till after      */
/* setting up save */

int errfd;

main()
{
  struct termios fattr;         /* fd = 0's attributes */
  int master,slave,fd;
/* DEBUG */
#ifdef SOCKET
int s;  /* socket for debugging */
#endif
debug=open("Debug",O_CREAT|O_TRUNC|O_WRONLY,0666);

  errfd=open("Error",O_CREAT|O_TRUNC|O_WRONLY,0666);
  if(errfd<0)
    errfd=open("/dev/null",O_WRONLY);
  dup2(errfd,2);   /* our stderr goes here */
  getpty(&master,&slave);
  if (tcgetattr(0,&fattr) == -1) {
    fprintf(stderr,"Ouch, cant get attributes of your stdin!.\n");
    exit(1);
  }
  save  = fattr;             /* save it for when we quit */
  saved=1;
  condition_tty(&fattr);

/* DEBUG */
#ifndef SOCKET
  if (tcsetattr(0,TCSANOW,&fattr) == -1) {
    fprintf(stderr,"Can't set up stdin.\n");
  } 
#else  SOCKET
  
fprintf(stderr,"getting connections.\n");
s=get_connect();
if(s<0) {
  fprintf(stderr,"ERROR: cant establish remote link.\n");
  exit(1);
}
close(0);   /* dubugging..  socket is input and output*/
close(1);
dup2(s,1);
dup2(s,0);
#endif SOCKET
  fcntl(0,F_SETFL,O_NDELAY);
  fcntl(master,F_SETFL,O_NDELAY); 
  fchmod(0,0600);    /* we dont need to be bothered */
  
  signal(SIGCLD,done);
#ifdef SOCKET
#ifdef DEBUG
/*these should be moved to do_copy */
  /* signal(SIGINT,done);   /* */
  signal(SIGBUS,done);
  signal(SIGSEGV,done);
  /* signal(SIGQUIT,done);  /* */
#endif
#endif  SOCKET && DEBUG
  switch(fork()) {
    case -1:   fprintf(stderr,"Cant find my fork().\n");
               done(0);
               break;
    case 0:   /* child */
               do_shell(master,slave);    /* connect shell to master */
               break;
    default:  /* parent */
               do_copy(master,slave);     /* connect tty to slave */
               break;
  } 

  done(0);    /* put things back */
  return;
}

condition_tty(t)
struct termios *t;
{
  t->c_lflag &= ~ICANON;   /* async */
  t->c_lflag &= ~ECHO;      /* no echo */
  t->c_lflag &= ~ISIG;      /* no signals */
  t->c_cc[VMIN] = 1;        /* 1 character at least */
  t->c_cc[VTIME]= 0;        /* no time-out */
  t->c_iflag &= ~ISTRIP;   /* dont input strip to 7 bits */
  t->c_iflag &= ~IXON;     /* ignore xon and xoff chars  */
  t->c_iflag &= ~IXOFF;   
  t->c_oflag &= ~OPOST;    /* no post processing of output */

  /* am i missing anythin?  */

}

char *name = "/dev/ptyXX";

int getpty(mfd,sfd)
int *sfd,*mfd;
{
#define PTYP   5
#define PTYLET 8
#define PTYNUM 9
  char *num = "0123456789abcdef";
#define NUMNUM 16
  char *let = "pqr";
#define NUMLET 3

  int i,j,k;

  for(i=0;i<NUMLET;i++) {
    name[PTYLET]=let[i]; 
    for(j=0;j<NUMNUM;j++) {
      name[PTYNUM]=num[j];
      
      *mfd=open(name,O_RDWR|O_NOCTTY);
      if(*mfd >= 0) {
         name[PTYP]='t';      /* try the tty,  the slave  */
         *sfd=open(name,O_RDWR);
         if(*sfd>=0)  {
           return(0);          /* we got it ma! */
         }
fprintf(stderr,"cant open %s.\n",name);
         name[PTYP]='p';
      }
    }
  }
  fprintf(stderr,"Cant grab any pty's!! ARGGHHH!.\n");
  done(-1);
}

 
do_shell(master,slave)
int master,slave;
{
  int fd,i;
 
 /* fd=open("/dev/tty",O_RDWR); 
  * if(fd>=0) {
  *   ioctl(fd,TIOCNOTTY,(char *)0);
  *   close(fd);
  * }
  */
  for(i=0;i<32;i++) 
    close(i); 
  setsid(); 
                  /* shell has the slave pty on its stdin and stdout */
  slave=open(name,O_RDWR);  /* name still has the tty name in it */
  if(slave<0) {
    fprintf(stderr,"Couldnt open slave!\n");
    done(-1);
  }
  tcsetattr(slave,TCSANOW,save);
  dup2(slave,0);
  dup2(slave,1);
  dup2(slave,2);
  if(slave >2)  
    close(slave); 
  execl(PROGRAM);
  fprintf(stderr,"Exec failed..  dying.\n");
  done(-1);
}

do_copy(master,slave) 
int master,slave;
{
  struct fd_set rd,wr,ex;
  struct timeval timeout;
#define BUFSIZE 8
  char buffer[BUFSIZE];
  int a;
/* DEBUG */
signal(SIGINT,done);
  
  timeout.tv_sec  = 0;
  timeout.tv_usec = FLUSHTIME;
  close(slave);
  if(startup(0,0)<0)    /* 0 is remote,  dont request key change */
    return;
  while(1) {            /* SIGCLD gets us out of this loop */
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&ex); 
    FD_SET(0,&rd);       /* can we read from master or stdin */
    FD_SET(master,&rd);
    if(select(NFDBITS,&rd,&wr,&ex,&timeout)!=-1) {
      if(FD_ISSET(0,&rd)) {
        a=read(0,buffer,BUFSIZE);
        if(a<0) {
          fprintf(stderr,"EOF: remote quit.\n");
          done(0);
        }
        de_out(buffer,a);     /* decrypt and send to pty */
        LOCAL(master);
      }
      if(FD_ISSET(master,&rd)) {
        a=read(master,buffer,BUFSIZE);
        if(a<0) {
          fprintf(stderr,"EOF: shell quit.\n");
          done(0);
        }
        en_out(buffer,a);          /* encrypt and output */
	REMOTE(1);
      }
      else {
        en_out(buffer,0);          /* no output but might flush */
        REMOTE(1);
      }
    }  /* if select */
  }    /* while 1   */
}



