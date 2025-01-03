/*
 * Configuration information for use by NNTP server and support
 * programs.  Change these as appropriate for your system.
 */

/*
 * Compile time options.
 */

#define	ALONE		/* True if we're running without inetd */
#undef	FASTFORK	/* True if we don't want to read active file on start */
#undef	BSD_42		/* 4.2 compatability code -- if this is defined, */
			/* DBM probably wants to be defined as well. */

#undef NDBM		/* Use new-style (4.3) ndbm(3x) libraries */

#define	DBM		/* True if we want to use the old dbm(3x) libraries */
			/* IF YOU DEFINE THIS, change CFLAGS in makefile to */
			/* be -ldbm */

#undef	USGHIST		/* Use USG style history file (no DBM) */
			/* IF YOU DO NOT DEFINE NDBM or DBM, this is DEFAULT! */

#define	USG		/* System V support */
#define	EXCELAN		/* Excelan EXOS 205 support */

#undef U_LONG		/* Define this if your <sys/types.h> is missing */
			/* typedefs for u_long */

/*
 * If you DON'T have vfork, make this "#define vfork fork"
 * vfork will speed up article transfer nntpds by about 2.5 times.
 */

#define	vfork fork

/*
 * If you have the syslog library routine, define SYSLOG to
 * be thef syslog facility name under which stats should be
 * logged.  Newer 4.3 systems might choose LOG_NEWS;
 * LOG_LOCAL7 is an acceptable substitute.
 *
 * If you don't have support for syslog, but want a facsimile,
 * define FAKESYSLOG to be the name of a file to which to log stuff,
 * then define SYSLOG and LOG, too.  e.g.,
 *
 *	#define	FAKESYSLOG	"/usr/lib/news/nntplog"
 *
 * If you don't want any syslog-type activity, #undef SYSLOG.
 * Obviously, this means that you can't define LOG, either.
 */

#define	FAKESYSLOG "/usr/lib/news/nntplog"

#define	SYSLOG	LOG_NEWS

#ifdef SYSLOG		/* Define LOG if you want copious logging info */
#	define LOG	/* undef it if you don't */
#endif			/* but you can only have LOG if you have SYSLOG */

#ifdef BSD_42		/* This is a logical, warranted assumption */
#   ifndef DBM		/* which will probably get me in trouble. */
#	define DBM	/* Kill it if you have 4.2 *and* ndbm.  */
#   endif /* not DBM */
#endif /* BSD_42 */

#ifdef USG		/* Another similar assumption */
#   ifndef USGHIST
/*#       define USGHIST*/
#   endif /*  not USGHIST */
#endif /* USG */

#undef	IHAVE_DEBUG	/* Copious debugging output from ihave */

#define	XHDR		/* Optional XHDR command.  Defining this will */
			/* speed up '=' command in rn, but will load */
			/* the server more.  If your server is heavily */
			/* loaded already, defining this may be a bad idea */

#undef	SUBNET		/* If you have 4.3 subnetting */
#undef	DAMAGED_NETMASK	/* If your subnet mask is not a multiple of */
			/* four bits (e.g., UCSD) */

#undef	NETMASK		/* If you don't have subnet ioctls, define */
			/* this to be a hex constant of your subnet */
			/* mask, e.g., #define NETMASK 0xffffff00 */
			/* Of course, you must define SUBNET above, too. */
#undef	DECNET		/* If you want decnet support */

#define	GHNAME		/* Define if you have gethostname() */
#undef	UUNAME		/* Define to use /etc/uucpname */
			/* If neither of these are defined, */
			/* inews will use the contents of */
			/* /usr/include/whoami.h */

/*
 * System V compatability
 */

#ifdef USG
# define	FCNTL			/* If O_etc is defined in <fcntl.h> */
# define	NDIR			/* If you need ndir library support */
# define	index	strchr
# define	rindex	strrchr
# ifdef U_LONG
   typedef	unsigned long	u_long;
   typedef	unsigned short	u_short;
# endif /* U_LONG */
# define	IPPORT_NNTP	119
#endif /* USG */

/*
 * How long you want nntp servers to hang out without receiving
 * commands before they close the connection with an error message.
 *
 * You CANNOT have TIMEOUT while running in standalone (ALONE) mode,
 * as SIGALRM is used for different things.
 *
 * If you don't want any timeout, #undef it, i.e.,
 *
 *	#undef	TIMEOUT
 *
 * TIMEOUT should be at least two hours, which allows users some time
 * away from their terminal (e.g., at lunch) while reading news.
 */

#ifndef ALONE
#    define	TIMEOUT	(2 * 3600)
#endif /* ALONE */

/*
 * How long you want nntp servers to wait without receiving data
 * during article transfers.  You CANNOT have XFER_TIMEOUT while
 * running in standalond (ALONE) mode.
 *
 * If you don't want any transfer timeouts, #undef it, as above.
 */

#ifndef ALONE
#   define	XFER_TIMEOUT	(30 * 60)
#endif /* ALONE */

/*
 * Your domain.  This is for the inews generated From: line,
 * assuming that it doesn't find one in the article's head.
 * Suggestions are .UUCP if you don't belong to the Internet.
 * If your hostname returns the fully-qualified domain name
 * as some 4.3 BSD systems do, simply undefine DOMAIN.
 *
 * e.g.  #define	DOMAIN		"berkeley.edu"
 */

#define	DOMAIN	".UUCP"

/*
 * A file containing the name of the host which is running
 * the news server.  This will have to match what rrn thinks,
 * too.
 */

#define	SERVER_FILE	"/usr/lib/news/rn/server"

/*
 * Person (user name) to post news as.
 */

#define	POSTER		"news"

/*
 * These files are generated by the support programs, and are needed
 * by the NNTP server.  Make sure that whatever directory you
 * decide these files should go is writable by whatever uid you
 * have the sypport programs run under.
 */

#define STAT_FILE	"/usr/lib/news/mgdstats"
#define NGDATE_FILE	"/usr/lib/news/groupdates"

/*
 * Some commonly used programs and files.
 */

#define	ACTIVE_FILE	"/usr/lib/news/active"
#define ACCESS_FILE	"/usr/lib/news/nntp_access"
#define HISTORY_FILE	"/usr/lib/news/history"
#define	SPOOLDIR	"/usr/spool/news"
#define INEWS		"/usr/lib/news/inews"
#define RNEWS		"/usr/bin/rnews"		/* Link to inews? */

/*
 * Some miscellaneous stuff you probably don't want to change.
 */

#define	MAX_ARTICLES	4096		/* Maximum number of articles/group */
#define READINTVL	60 * 10		/* 10 minutes b/n chking active file */
