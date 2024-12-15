/*
** identd.c                       A TCP/IP link identification protocol server
**
** Version: 1.9beta.5
**
** This program is in the public domain and may be used freely by anyone
** who wants to. 
**
** Last update: 10 June 1992
**
** Please send bug fixes/bug reports to: Peter Eriksson <pen@lysator.liu.se>
*/

#define VERSION "[in.identd, version 1.9beta.5]"

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <nlist.h>
#include <pwd.h>
#include <signal.h>
#include <syslog.h>
#include <setjmp.h>


#ifdef HAVE_KVM
#include <kvm.h>
#else
#include "kvm.h"
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>

#ifdef __convex__
#define _KERN_UTIL
#endif

#include <sys/socketvar.h>

#ifdef __convex__
#undef _KERN_UTIL
#endif

#include <sys/ioctl.h>


#ifndef HPUX7
#define KERNEL
#endif

#include <sys/file.h>

#ifndef HPUX7
#undef KERNEL
#endif

#include <fcntl.h>

#ifdef ultrix
#include <sys/dir.h>
#endif

#include <sys/user.h>
#include <sys/wait.h>
  
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/ip_var.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <arpa/inet.h>

#ifdef MIPS
#include <sysv/sys/var.h>
extern int errno;
#endif


extern void *calloc();


struct nlist nl[] =
{
#ifdef hp9000s800	/* HP9000 Series 800, HP-UX 8.0 */

#define N_FILE  0  
#define N_NFILE 1
#define N_TCB   2

  { "file" },
  { "nfile" },
  { "tcb" },
  
#else
#ifdef HPUX7		/* HP-UX 7.0 */

#define N_FILE  0  
#define N_NFILE 1
#define N_TCB   2

  { "_file" },
  { "_nfile" },
  { "_tcp_cb" },

#else
#ifdef MIPS		/* MIPS RISC/OS */

#define N_FILE 0  
#define N_V    1
#define N_TCB  2

  { "_file" },
  { "_v" },
  { "_tcb" },

#else			/* SUN or BSD or other systems */

#define N_FILE 0  
#define N_NFILE 1
#define N_TCB 2

  { "_file" },
  { "_nfile" },
  { "_tcb" },

#endif
#endif
#endif
  { "" }
};


char *path_unix = NULL;
char *path_kmem = NULL;

kvm_t *kd;

int verbose_flag = 0;
int debug_flag   = 0;
int syslog_flag  = 0;
int multi_flag   = 0;
int other_flag   = 0;
int unknown_flag = 0;
int number_flag  = 0;

int child_pid;
#ifdef HAVE_FORKCLOSE_BUG
int pipe_fd[2];
#endif

#ifdef LOG_DAEMON
int syslog_facility = LOG_DAEMON;
#endif

jmp_buf error_recovery;


struct file *xfile;
int nfile;

#ifdef MIPS
struct var v;
#endif

int lport, fport;
struct inpcb tcb;


/*
** Error handling macros
*/
#define ERROR(fmt) \
    ((syslog_flag ? (syslog(LOG_ERR, fmt),0) : 0), \
     (debug_flag ? (fprintf(stderr, "%d , %d : ERROR : X-DBG : ", \
			    lport, fport), \
		    fprintf(stderr, fmt), perror(""), 0) : \
      (printf("%d , %d : ERROR : UNKNOWN-ERROR\r\n", lport, fport), \
       longjmp(error_recovery, 1), 0)))

#define ERROR1(fmt,v1) \
    ((syslog_flag ? (syslog(LOG_ERR, fmt, v1),0) : 0), \
     (debug_flag ? (fprintf(stderr, "%d , %d : ERROR : X-DBG : ", \
			    lport, fport), \
		    fprintf(stderr, fmt, v1), perror(""), 0) : \
      (printf("%d , %d : ERROR : UNKNOWN-ERROR\r\n", lport, fport), \
       longjmp(error_recovery, 1), 0)))

#define ERROR2(fmt,v1,v2) \
    ((syslog_flag ? (syslog(LOG_ERR, fmt, v1, v2),0) : 0), \
     (debug_flag ? (fprintf(stderr, "%d , %d : ERROR : X-DBG : ", \
			    lport, fport), \
		    fprintf(stderr, fmt, v1, v2), perror(""), 0) : \
      (printf("%d , %d : ERROR : UNKNOWN-ERROR\r\n", lport, fport), \
       longjmp(error_recovery, 1), 0)))


/*
** The structure passing convention for GCC is incompatible with
** Suns own C compiler, so we define our own inet_ntoa() function.
** (This should only affect GCC version 1 I think, a well, this works
** for version 2 also so why bother.. :-)
*/
#if defined(__GNUC__) && defined(__sparc__)

#ifdef inet_ntoa
#undef inet_ntoa
#endif

char *inet_ntoa(ad)
  struct in_addr ad;
{
  unsigned long int s_ad;
  int a, b, c, d;
  static char addr[20];
  
  s_ad = ad.s_addr;
  d = s_ad % 256;
  s_ad /= 256;
  c = s_ad % 256;
  s_ad /= 256;
  b = s_ad % 256;
  a = s_ad / 256;
  sprintf(addr, "%d.%d.%d.%d", a, b, c, d);
  
  return addr;
}
#endif


/*
** Return the name of the connecting host, or the IP number as a string.
*/
char *gethost(addr)
  struct in_addr *addr;
{
  struct hostent *hp;

  
  hp = gethostbyaddr(addr, sizeof(struct in_addr), AF_INET);
  if (hp)
    return hp->h_name;
  else
    return inet_ntoa(*addr);
}


/*
** Get a piece of kernel memory with error handling.
** Always return 1.
*/
int getbuf(addr, buf, len, what)
  long addr;
  char *buf;
  int len;
  char *what;
{

  if (kvm_read(kd, addr, buf, len) < 0)
    ERROR2("getbuf: kvm_read(%08x, %d)", addr, len);
  
  return 1;
}



/*
** Traverse the inpcb list until a match is found.
** Returns NULL if no match.
*/
struct socket *getlist(pcbp, faddr, fport, laddr, lport)
  struct inpcb *pcbp;
  struct in_addr *faddr;
  int fport;
  struct in_addr *laddr;
  int lport;
{
  struct inpcb *head;
  

  if (!pcbp)
    return NULL;
  
  head = pcbp->inp_prev;
  do 
  {
    if ( pcbp->inp_faddr.s_addr == faddr->s_addr &&
	 pcbp->inp_laddr.s_addr == laddr->s_addr &&
	 pcbp->inp_fport        == fport &&
	 pcbp->inp_lport        == lport )
      return pcbp->inp_socket;

  } while (pcbp->inp_next != head &&
	   getbuf((long) pcbp->inp_next,
		  pcbp,
		  sizeof(struct inpcb),
		  "tcblist"));

  return NULL;
}



/*
** Locate the 'ucred' information for a certain connection.
*/
struct ucred *getinfo(faddr, fport, laddr, lport)
  struct in_addr faddr;
  int fport;
  struct in_addr laddr;
  int lport;
{
  static struct ucred ucb;
  long addr;
  struct socket *sockp;
  int i;
  
  

  /* -------------------- FILE DESCRIPTOR TABLE -------------------- */
#ifdef MIPS
  getbuf(nl[N_V].n_value, &v, sizeof(v), "v");
  nfile = v.v_file;
  addr = nl[N_FILE].n_value;
#else
  getbuf(nl[N_NFILE].n_value, &nfile, sizeof(nfile), "nfile");
  getbuf(nl[N_FILE].n_value, &addr, sizeof(addr), "&file");
#endif
  
  xfile = (struct file *) calloc(nfile, sizeof(struct file));
  if (!xfile)
    ERROR2("getinfo: calloc(%d,%d)", nfile, sizeof(struct file));
  
  getbuf(addr, xfile, sizeof(struct file)*nfile, "file[]");


  /* -------------------- TCP PCB LIST -------------------- */
  getbuf(nl[N_TCB].n_value, &tcb, sizeof(tcb), "tcb");
  tcb.inp_prev = (struct inpcb *) nl[N_TCB].n_value;
  sockp = getlist(&tcb, faddr, fport, laddr, lport);
  if (!sockp)
    return NULL;


  /*
  ** Locate the file descriptor that has the socket in question
  ** open so that we can get the 'ucred' information
  */
  for (i = 0; i < nfile; i++)
    if (xfile[i].f_count &&
	xfile[i].f_type == DTYPE_SOCKET &&
	(struct socket *) xfile[i].f_data == sockp)
    {
      getbuf(xfile[i].f_cred, &ucb, sizeof(ucb));
      return &ucb;
    }

  return NULL;
}



/*
** This function will eat whitespace characters until
** either a non-whitespace character is read, or EOF
** occurs. This function is only used if the "-m" option
** is enabled.
*/
int eat_whitespace()
{
  int c;

  while ((c = getchar()) != EOF &&
	 (c == ' ' || c == '\t' || c == '\n' || c == '\r'))
    ;

  if (c != EOF)
    ungetc(c, stdin);
  
  return (c != EOF);
}



/*
** This is used to clean up zombie child processes
** if the -w or -b options are used.
*/
child_handler()
{
  int status;
  int pid;

  
  while ((pid = waitpid(-1, &status, WNOHANG)) && pid != -1)
#ifdef HAVE_FORKCLOSE_BUG
    if (pid == child_pid)
    {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
    }
#else
    ;
#endif
  
}



/*
** Main entry point into this daemon
*/
int main(argc,argv)
  int argc;
  char *argv[];
{
  int i, len, try;
  struct sockaddr_in sin;
  struct in_addr laddr, faddr;
  struct ucred *ucp;
  struct passwd *pwp;

  int background_flag = 0;
  int timeout = 0;
  int portno = 113;
  int set_uid = 0;
  int set_gid = 0;

  
  /*
  ** Parse the command line arguments
  */
  for (i = 1; i < argc && argv[i][0] == '-'; i++)
    switch (argv[i][1])
    {
      case 'b':    /* Start as standalone daemon */
        background_flag = 1;
	break;

      case 'w':    /* Start from Inetd, wait mode */
	background_flag = 2;
	break;

      case 'i':    /* Start from Inetd, nowait mode */
	background_flag = 0;
	break;

      case 't':
	timeout = atoi(argv[i]+2);
	break;

      case 'p':
	portno = atoi(argv[i]+2);
	break;

      case 'u':
	set_uid = atoi(argv[i]+2);
	break;
	
      case 'g':
	set_gid = atoi(argv[i]+2);
	break;
	
      case 'l':    /* Use the Syslog daemon for logging */
	syslog_flag++;
	break;

      case 'o':
	other_flag = 1;
	break;

      case 'e':
	unknown_flag = 1;
	break;

      case 'n':
	number_flag = 1;
	break;
	
      case 'V':    /* Give version of this daemon */
	puts(VERSION);
	exit(0);
	break;

      case 'v':    /* Be verbose */
	verbose_flag++;
	break;
	  
      case 'd':    /* Enable debugging */
	debug_flag++;
	break;

      case 'm':    /* Enable multiline queries */
	multi_flag++;
	break;
    }

  /*
  ** Path to kernel namelist file specified on command line
  */
  if (i < argc)
    path_unix = argv[i++];

  /*
  ** Path to kernel memory device specified on command line
  */
  if (i < argc)
    path_kmem = argv[i++];


  /*
  ** Errors in the setup phase are handled by terminating
  */
  if (setjmp(error_recovery))
    exit(1);
  

  /*
  ** Open the kernel memory device
  */
  if (!(kd = kvm_open(path_unix, path_kmem, NULL, O_RDONLY, NULL)))
    ERROR("main: kvm_open");
  
  /*
  ** Extract offsets to the needed variables in the kernel
  */
  if (kvm_nlist(kd, nl) != 0)
    ERROR("main: kvm_nlist");


  if (background_flag == 1)
  {
    struct sockaddr_in addr;
    int fd;

    
    if (fork())
      exit(0);

    close(0);
    close(1);
    close(2);

    if (fork())
      exit(0);
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
      ERROR("main: socket");
    
    if (fd != 0)
      dup2(fd, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = portno;
    
    if (bind(0, &addr, sizeof(addr)) < 0)
      ERROR("main: bind");

    if (listen(0, 3) < 0)
      ERROR("main: listen");

    if (set_gid)
      if (setgid(set_gid) == -1)
	ERROR("main: setgid");
  
    if (set_uid)
      if (setuid(set_uid) == -1)
	ERROR("main: setuid");
  }
  
  if (background_flag)
  {
    int nfds, fd;
    fd_set read_set;


    /*
    ** Set up the SIGCHLD signal child termination handler so
    ** that we can avoid zombie processes hanging around and
    ** handle childs terminating before being able to complete the
    ** handshake.
    */
#ifdef	hp9000s800
    signal(SIGCHLD, SIG_IGN);
#else	hp9000s800
    signal(SIGCHLD, child_handler);
#endif	hp9000s800
    
    /*
    ** Loop and dispatch client handling processes
    */
    do
    {
      FD_ZERO(&read_set);
      FD_SET(0, &read_set);

      /*
      ** Terminate if we've been idle for 'timeout' seconds
      */
      if (background_flag == 2 && timeout)
	alarm(timeout);

      /*
      ** Wait for a connection request to occur
      */
      do
      {
	nfds = select(FD_SETSIZE, &read_set, 0, 0, 0);
      } while (nfds < 0  && errno == EINTR);

      /*
      ** An error occured in select? Just die
      */
      if (nfds <= 0)
	ERROR("main: select");

      /*
      ** Accept the new client
      */
      fd = accept(0, 0, 0);
      if (fd == -1)
	ERROR("main: accept");

#ifdef HAVE_FORKCLOSE_BUG
      /*
      ** ARGH! I hate this bug in most Unix kernels. (It's there in
      ** atleast Suns SunOS 4.1.1 and Sequents Dynix 2.1.1). If I close
      ** this file descriptor to quick (ie, before the forked off child
      ** has begun execution it will close it in the child too -> it will
      ** have big problems trying to work with it...
      **
      ** So I kludge this by creating a Unix pipe and have the
      ** parent block on a read() for one character, and then
      ** have the child transmit one character to unblock the
      ** parent. If the child dies before getting a chance to
      ** transmit this lone character the the SIGCHLD handler
      ** will close the pipe -> forcing read() to unblock even
      ** though a potential Unix uses restarting syscalls.
      */
      if (pipe(pipe_fd) == -1)
	ERROR("main: pipe");
#endif
      
      /*
      ** Ok, now we can fork safely
      */
      child_pid = fork();
      if (child_pid)
      {
#ifdef HAVE_FORKCLOSE_BUG	
	/*
	** In parent...
	**
	** Now, block in the read() syscall waiting for the child
	** to begin execution
	*/
	char c;

	
	if (read(pipe_fd[0], &c, 1) != 1)
	{
	  /*
	  ** Child died before being able to complete the handshake!
	  **
	  ** Now we're in deep sh*t...
	  */
	  ERROR("main: fork/read handshake failed. HELP!");
	}
	
	close(pipe_fd[0]);
	close(pipe_fd[1]);
#endif
	
	/*
	** It's now safe to close the client socket file descriptor
	*/
	close(fd);
      }
#ifdef HAVE_FORKCLOSE_BUG	
      else
      {
	/*
	** In child...
	**
	** Ok, now we transmit one character to the parent so that it
	** will now that it can safely close the socket file descriptor
	*/

	write(pipe_fd[1], "a", 1);
	close(pipe_fd[1]);
	close(pipe_fd[0]);
      }
#endif      
    } while (child_pid);

    /* We are now in child */
    if (dup2(fd, 0) == -1)
      ERROR("main: dup2: failed");
    
    dup2(fd, 1);
    dup2(fd, 2);
  }

  /*
  ** Get foreign internet address
  */
  len = sizeof(sin);
  if (getpeername(0,&sin,&len) == -1)
  {
    /*
    ** A user has tried to start us from the command line or
    ** the network link died, in which case this message won't
    ** reach to other end anyway, so lets give the poor user some
    ** errors.
    */
    perror("in.identd: getpeername()");
    exit(1);
  }
  
  faddr = sin.sin_addr;


  /*
  ** Open the connection to the Syslog daemon if requested
  */
  if (syslog_flag)
  {
#ifdef LOG_DAEMON
    openlog("identd", LOG_PID, syslog_facility);
#else
    openlog("identd", LOG_PID);
#endif
    
    syslog(LOG_NOTICE, "Connection from %s", gethost(&faddr));
  }
  

  /*
  ** Get local internet address
  */
  len = sizeof(sin);
  if (getsockname(0,&sin,&len) == -1)
  {
    /*
    ** We can just die here, because if this fails then the
    ** network has died and we haven't got anyone to return
    ** errors to.
    */
    exit(1);
  }
  laddr = sin.sin_addr;


  /*
  ** Get the local/foreign port pair from the luser
  */
  do
  {
    /*
    ** Handle errors by either handling next request or terminate
    ** according to the multi_flag
    */
    if (setjmp(error_recovery))
      continue;
    
    lport = fport = 0;
    if (scanf("%d , %d", &lport, &fport) != 2 ||
	lport < 1 || lport > 65535 || fport < 1 || fport > 65535)
    {
      if (syslog_flag)
	syslog(LOG_INFO, "scanf: invalid-port(s): %d , %d", lport, fport);
      
      printf("%d , %d : ERROR : %s\r\n",
	     lport, fport,
	     unknown_flag ? "UNKNOWN-ERROR" : "INVALID-PORT");
      continue;
    }

    
    try = 0;

    /*
    ** Handle errors by retrying
    */
    if (setjmp(error_recovery))
      ;
    
    /*
    ** Next - get the specific TCP connection and return the
    ** 'ucred' - user credentials - information
    **
    ** Try to fetch the information 5 times incase the
    ** kernel changed beneath us and we missed or took
    ** a fault.
    */
    while (try < 5 &&
	   !(ucp = getinfo(&faddr, htons(fport), &laddr, htons(lport))))
      try++;

    if (!ucp)
    {
      if (syslog_flag)
	syslog(LOG_DEBUG, "Returned: %d , %d : NO-USER", lport, fport);
      
      printf("%d , %d : ERROR : %s\r\n",
	     lport, fport,
	     unknown_flag ? "UNKNOWN-ERROR" : "NO-USER");
      continue;
    }


    /*
    ** At this point we handle errors by either handling the next
    ** request or terminate
    */
    if (setjmp(error_recovery))
      continue;
    
    /*
    ** Then we should try to get the username. If that fails we
    ** return it to as an OTHER identifier
    */
    pwp = getpwuid(ucp->cr_ruid);
    if (!pwp || number_flag)
    {
      if (syslog_flag && !pwp)
	syslog(LOG_WARNING, "getpwuid() could not map uid (%d) to name",
	       ucp->cr_ruid);
      
      printf("%d , %d : USERID : OTHER : %d\r\n",
	     lport, fport,
	     ucp->cr_ruid);
      continue;
    }

    /*
    ** Hey! We finally made it!!!
    */
    if (syslog_flag)
      syslog(LOG_DEBUG, "Successful lookup: %d , %d : %s\n",
	     lport, fport, pwp->pw_name);
    
    printf("%d , %d : USERID : %s : %s\r\n",
	   lport, fport,
	   other_flag ? "OTHER" : "UNIX",
	   pwp->pw_name);
  } while(fflush(stdout), multi_flag && eat_whitespace());

  exit(0);
}

