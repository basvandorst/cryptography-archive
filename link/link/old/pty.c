
/* this program just opens a pty master and slave,
 * exec's a shell and connects your original tty (or pty)
 * to the pty.
 */

#include <termios.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
struct termios save;    /* a copy of the original attributes */


done(i)    /* signals send us here,  we put back things that we touched */
int i; 
{
  printf("\nSignal #%d\n\n",i);
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

  getpty(&master,&slave);
  if (tcgetattr(0,&fattr) == -1) {
    printf("Ouch, cant get attributes of your stdin!.\n");
    exit(1);
  }
  mattr = fattr;             /* start of w/ attributes of terminal */
  sattr = fattr;         
  save  = fattr;             /* save it for when we quit */
  condition_tty(&fattr);

/*  if (tcsetattr(master,TCSANOW,&mattr) == -1) {
 *   fprintf(stderr,"Can't set up master tty.\n");
 * }
 * if (tcsetattr(slave,TCSANOW,&sattr) == -1) {
 *   fprintf(stderr,"Can't set up slave tty.\n");
 * }
 */

  if (tcsetattr(0,TCSANOW,&fattr) == -1) {
    fprintf(stderr,"Can't set up stdin.\n");
  }
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
printf("opened %s fine.\n",name);
         name[PTYP]='t';      /* try the tty,  the slave  */
         *sfd=open(name,O_RDWR);
         if(*sfd>=0)  {
printf("opened %s fine.\n",name);
           return(0);          /* we got it ma! */
         }
printf("cant open %s.\n",name);
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
    printf("Couldnt open slave!\n");
    exit(1);
  }
  tcsetattr(slave,TCSANOW,save);
  dup2(slave,0);
  dup2(slave,1);
  dup2(slave,2);
  if(slave >2)  
    close(slave); 
  execl("/bin/csh","csh",0);
  printf("Exec failed..  dying.\n");
  exit(1);
}

do_copy(master,slave) 
int master,slave;
{
  struct fd_set rd,wr,ex;
#define BUFSIZE 8
  char buffer[BUFSIZE];
  int a;
  
  close(slave);
  while(1) {       /* SIGCLD gets us out of this loop */
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&ex); 
    FD_SET(0,&rd);       /* can we read from master or stdin */
    FD_SET(master,&rd);
    if(select(NFDBITS,&rd,&wr,&ex,0)) {
      if(FD_ISSET(0,&rd)) {
        a=read(0,buffer,BUFSIZE);
        de_out(buffer,a,master);     /* decrypt and send to pty */
      }
      if(FD_ISSET(master,&rd)) {
        a=read(master,buffer,BUFSIZE);
        en_out(buffer,a,1);          /* encrypt and output */
      }
    }  /* if select */
  }    /* while 1   */
}

de_out(buf,len,fd)     /* decrypt and output on fd */
{
  if(len>0) 
    write(fd,buf,len);
}

en_out(buf,len,fd)    /* encrypt and output on fd */
{
  if(len>0)
    write(fd,buf,len);
}
