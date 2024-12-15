

#include <termios.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#define FLUSHTIME  50000    /* .5 seconds */

/* slip stuff ..  from slip.c */
struct _nd {
  struct _nd *next;
  int len;
  char *data;
};
typedef struct _nd nd;
extern int send_packet();
extern nd *decode_packets();

/* des stuff from des.c */
extern des();
extern undes();

struct termios save;    /* a copy of the original attributes */

done(i)    /* signals send us here,  we put back things that we touched */
int i; 
{
  if(i)
    fprintf(stderr,"\nSignal #%d\n\n",i);
  tcsetattr(0,TCSANOW,save);
  fcntl(0,F_SETFL,   
            fcntl(0,F_GETFL,0) & ~O_NDELAY);
  exit(0); 
}

/* assumes stdin == stdout == the terminal connected to the modem*/
/* is this a bad assumption? dont set up signals till after      */
/* setting up save */


main()
{
  struct termios mattr,sattr;   /* master and slave term attributes */
  struct termios fattr;         /* fd = 0's attributes */
  int master,slave,fd;
/* DEBUG */
int s;  /* socket for debugging */

  getpty(&master,&slave);
  if (tcgetattr(0,&fattr) == -1) {
    fprintf(stderr,"Ouch, cant get attributes of your stdin!.\n");
    exit(1);
  }
  mattr = fattr;             /* start of w/ attributes of terminal */
  sattr = fattr;         
  save  = fattr;             /* save it for when we quit */
  condition_tty(&fattr);

/* DEBUG */
 /* if (tcsetattr(0,TCSANOW,&fattr) == -1) {
  *  fprintf(stderr,"Can't set up stdin.\n");
  *}
  */
fprintf(stderr,"getting connections.\n");
s=get_connect();
if(s<0) {
  fprintf(stderr,"ERROR: cant open socket!\n");
  exit(1);
}
des_init();
close(0);   /* dubugging..  socket is input and output*/
close(1);
dup2(s,1);
dup2(s,0);
  fcntl(0,F_SETFL,O_NDELAY);
  fcntl(master,F_SETFL,O_NDELAY); 
  fchmod(0,0600);    /* we dont need to be bothered */
  
  /* signal(SIGINT,done);   /* */
  signal(SIGCLD,done);
  signal(SIGSEGV,done);
  /* signal(SIGQUIT,done);  /* */
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
  exit(1);
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
    exit(1);
  }
  tcsetattr(slave,TCSANOW,save);
  dup2(slave,0);
  dup2(slave,1);
  dup2(slave,2);
  if(slave >2)  
    close(slave); 
  execl("/bin/csh","csh",0);  
  fprintf(stderr,"Exec failed..  dying.\n");
  exit(1);
}

do_copy(master,slave) 
int master,slave;
{
  struct fd_set rd,wr,ex;
  struct timeval timeout;
#define BUFSIZE 8
  char buffer[BUFSIZE];
  int a;
  
  timeout.tv_sec  = 0;
  timeout.tv_usec = FLUSHTIME;
  close(slave);
  while(1) {       /* SIGCLD gets us out of this loop */
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&ex); 
    FD_SET(0,&rd);       /* can we read from master or stdin */
    FD_SET(master,&rd);
    if(select(NFDBITS,&rd,&wr,&ex,&timeout)!=-1) {
      if(FD_ISSET(0,&rd)) {
        a=read(0,buffer,BUFSIZE);
        de_out(master,buffer,a);     /* decrypt and send to pty */
      }
      if(FD_ISSET(master,&rd)) {
        a=read(master,buffer,BUFSIZE);
        en_out(1,buffer,a);          /* encrypt and output */
      }
      else
        en_out(1,buffer,0);          /* no output but might flush */
    }  /* if select */
  }    /* while 1   */
}


de_out(fd,buf,len)     /* decrypt and output on fd */
{
  nd *root,*t;
  char l;

  root=decode_packets(buf,len);
  for(t=root;t !=0; t=t->next) {
   if (t->len !=8 ) 
     continue;           /* just drop bad packets */
   undes(t->data);
   l=t->data[0];            /* num of characters */
   write(fd,t->data+1,(int)l); 
  }
  return;

  if(len>0) 
    write(fd,buf,len);
}



en_out(fd,buf,len)    /* encrypt and output on fd */
char *buf;            /* flush at least every FLUSHTIME */
int len,fd;           /* if called with len =0 just check */
{                     /* time */
  static char buffer[10];
  static int l=1;
  static struct timeval lastflush = {0,0};
  struct timeval t;
  int flushed=0;
  char a;

  while(len--) {
    a= *buf++;
    buffer[l++] =a;
    if(l==8) {                   /* send out a full packet */
      buffer[0] = (char)7;
      des(buffer);
      send_packet(fd,buffer,8);
      l=1;
      flushed=1;
    }
  }
  if(flushed || l==1) {
    gettimeofday(&lastflush);
    return;
  }
 
  gettimeofday(&t);
  if (t.tv_usec < lastflush.tv_usec) {
    t.tv_usec += 1000000;   /* add a second */
    t.tv_sec --;            /* take off a second */
  }
  if((t.tv_sec > lastflush.tv_sec) ||
     ((t.tv_usec-lastflush.tv_usec) > FLUSHTIME )) {
    /* we have to flush it out now */
    buffer[0]=(char)(l-1);
    des(buffer);
    send_packet(fd,buffer,8);
    l=1;
    gettimeofday(&lastflush);
  }
}

