/* can be overridden by means of the -p option */
#define PORTNUM			10005

#define MAXHNAMELEN	128	
#define MAXUSERNAMELEN	32
#define MAXPWDLEN	10	/* max password length for unix auth */

/* Connection is closed if it takes more that logintimeout seconds
for the user to be authenticated */
#ifdef DEBUG
#define LOGINTIMEOUT	99999999
#else
#define LOGINTIMEOUT	(60 * 3) /* seconds */
#endif

/* User gets disconnected when idle timeout expires.
This feature can be overridden by means of server's -t option.
this option is disabled by default
*/
#define IDLETIMEOUT	0

/* circular skey buffer size */
#define SKEYBUFSIZE	1024

/* Bellcore S/Key prompt.
The following should catch most of commonly used skey application
skey prompts.  fixed to support seeds composed by any character
*/
#define SKEYPATTERN	\
"[\n\r]*[[]*[Ss][/]?[Kk]ey[ \t]+[0-9]+[ \t]+[^ \t\n\r]+[]]*[\n\r]*"

/* general makeio buffer */
#define MAKEIOBUF	4096
/* max # of packets inside MAKEIOBUF */
#define MAXCOMBONUM     (MAKEIOBUF / 10)

#define STELCLIENT	"stel"
#define STELSERVER	"steld"
#define VERSION		5
#ifdef ASCIIBANNER
#define BANNER		"This is STEL, protocol id = %3d\n\r"
#endif

/* initial ack (from client to server) */
#define	FLG_NO_ENCRYPTION	0x01
#define FLG_USE_IDEA		0x02
#define FLG_USE_SINGLE		0x04
#define FLG_SMALL_MODULUS	0x08
#define FLG_LARGE_MODULUS	0x10
#define FLG_BE_VERBOSE		0x20
#define FLG_DEBUG		0x40
#define FLG_NO_ESCAPE		0x80
#define FLG_ACTIVE_ATTACKS	0x100
#define FLG_USE_RC4		0x200

/* second ack (from server to client) */
#define MSG_OKAY_DATA			0x01
#define MSG_CORRUPTED_DATA		0x02
#define MSG_LOGIN_AUTH_REQUIRED		0x04
#define MSG_MUST_AUTH			0x08
#define MSG_SYSTEM_AUTH_REQUIRED	0x10
#define MSG_WRONG_VERSION		0x20

#define STEL_SECRET_MAX_LEN	128
#define STEL_SECRET_NAME	".stelsecret"
#define ETC_STEL_SECRET_NAME	"/etc/stelsecret"

/* sides */
#define CLIENT_SIDE		0
#define SERVER_SIDE		1

#define NOTIMEOUT		-1
#define USRAUTH			"userauthenticated"

/****************************************************************/
/* first encrypted packet which is xmited from client to server */
/****************************************************************/
typedef struct {
	char	random[16]; /* stream randomizer (IV is first 8 bytes) */
	char	username[MAXUSERNAMELEN];
	/* some environment variables */
	char	command[1024];
	char	TERM[8], LINES[8], COLUMNS[8], DISPLAY[32], WINDOWID[32];
	/* char	PATH[512]; */
	int	rows, cols;
	int	mode;
	int	version;
	unsigned char	digest[16];
} cpacket;

/******************************************************/
/* first packet which is xmited from server to client */
/******************************************************/
typedef struct {
	char	random[16]; /* stream randomizer (IV is first 8 bytes) */
	int	sreply;
	unsigned char	digest[16];
} spacket;

/* used to protect against active attacks */
typedef struct {
	unsigned long   seq;
#ifdef COMBOFLAG
	int		flag;
#endif /* COMBOFLAG */
	long            crc32;
	int             len;
	char		*data;
} combopacket;

#ifdef COMBOFLAG
#define COMBO_FLG_DATA	0x01
#define COMBO_FLG_PUT	0x02
#define COMBO_FLG_GET	0x04
#define XFERTOKEN	"xferrequested" /* some random phrase */

#define WHAT_ACK	"serverisready" /* S -> C, 1 */

typedef struct {			/* C -> S, 1 */
	int		cmd;
	char		filename[256];
	unsigned long	filesize;
	unsigned char	checksum[16];
} combocmd;

typedef struct {			/* S -> C, 2 */
	int		answer;
	unsigned long	filesize;
	char		message[128];
	unsigned char	checksum[16];
} comboanswer;
#endif /* COMBOFLAG */

/* mode */
#define FLG_DONTUSE_PTY		0x01

/* ciphers */
#define		DES_CIPHER		0
#define		TRIPLEDES_CIPHER	1
#define		IDEA_CIPHER		2
#define		RC4_CIPHER		3

/* free bsd needs this; _H* */
#ifndef SIGCLD
#ifdef SIGCHLD
#define SIGCLD SIGCHLD
#endif
#endif

#ifdef DESCORE
#include "desCore.h"

typedef union {
	unsigned char	x;
} des_key_schedule[128];

typedef union {
	unsigned char	x;
} des_cblock[8];
#define DES_ENCRYPT	1
#define DES_DECRYPT	0
#define ENCRYPT		DES_ENCRYPT
#define DECRYPT		DES_DECRYPT
#else
#include "des.h"
#endif

#ifdef hpux
#define seteuid(x)	setresuid(-1, x, x)
#define setegid(x)	setresgid(-1, x, x)
#endif

#ifdef SOLARIS24
#define bzero(x, y)	memset(x, 0, y)
#endif
