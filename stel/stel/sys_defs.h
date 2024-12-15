 /*
  * Cope with different major UNIX streams, and what the vendors did to them.
  * 
  * Author:Wietse Venema < wietse @ wzv.win.tue.nl >
  * 
  * Beginning of generic (vendor-independent) features.
  */

#ifdef HAS_UTMPX
#include <utmpx.h>
#else /* HAS_UTMPX */
#include <utmp.h>
#endif /* HAS_UTMPX */
#ifndef UT_NAMESIZE
#define UT_NAMESIZE     sizeof(((struct UTMP_STRUCT *)0)->ut_name)
#endif /* UT_NAMESIZE */

#if defined(SYSV2) || defined(SYSV3)
#define NO_LASTLOG_H			/* no <lastlog.h> file */
#define _PATH_LOGIN "/bin/login"
#endif

#if defined(SYSV2) || defined(SYSV3) || defined(SYSV4)
#define SYSV_UTMP			/* insist on existing utmp entry */
#define SYSV_ENV			/* TERM=value login arg, no ucb path */
#define NO_TTYENT			/* no <ttyent.h> stuff */
#define NO_MOTD				/* leave motd to the shell */
#define USE_GETCWD			/* getcwd() instead of getwd() */
#define SYSV_LS				/* "ls -l" lists groups */
#endif

#if defined(SYSV3) || defined(SYSV4)
#define SYSV_SHADOW			/* shadow pwds, password expiry */
#define SYSV_LOGINDEFS			/* has /etc/default/login */
#endif

#ifdef SYSV4
#define HAS_UTMPX			/* utmp+utmpx, wtmp+wtmpx files */
#define STREAM_PTY			/* ptys are streams devices */
#define USE_SYS_MNTTAB_H		/* <sys/mnttab.h> */
#endif

#ifndef SOLARIS /* canno handle this!? */
#if (defined(BSD) && (BSD >= 199306))    /* 4.4BSD-based */
#define HAS_PATHS_H                   /* <paths.h> */
#define HAS_SETLOGIN          /* setlogin() */
#define SIGCLD  SIGCHLD
#endif
#endif /* SOLARIS */
 

 /*
  * End of generic (vendor-independent) features.
  * 
  * Beginning of vendor-specific exceptions.
  */

#ifdef hpux
#define SYSV_UTMP			/* login requires utmp entry */
#define NO_TTYENT			/* no <ttyent.h> stuff */
#define NO_MOTD				/* leave motd to the shell */
#define NO_LASTLOG_H			/* no <lastlog.h> file */
#define USE_GETCWD			/* getcwd() instead of getwd() */
#define USE_SETRESXID			/* setresuid(), setresgid() */
/*#define USE_PTYOPEN */		/* ptyopen(), ptyexecvp() */
#define SYSV_LS				/* "ls -l" lists groups */
#define _PATH_LOGIN "/bin/login"
#endif

#ifdef IRIX
#define	SYSV_UTMP			/* login requires utmp entry */
#endif

#ifdef linux
#define	SYSV_UTMP			/* login requires utmp entry */
#endif

 /*
  * End of vendor-specific exceptions.
  */

#ifdef HAS_UTMPX
#define UTMP_STRUCT	utmpx
#define UTMP_INIT	utmpx_init
#define UTMP_LOGIN	utmpx_login
#define UTMP_LOGOUT	utmpx_logout
#define MAKE_UTMP_ID	utmpx_ptsid
#else
#define UTMP_STRUCT	utmp
#define UTMP_INIT	utmp_init
#define UTMP_LOGIN	utmp_login
#define UTMP_LOGOUT	utmp_logout
#define MAKE_UTMP_ID	utmp_ptsid
#endif
