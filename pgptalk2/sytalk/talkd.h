#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define NAME_SIZE 9
#define TTY_SIZE 16

/* Control Message structure for earlier than BSD4.2
 */
typedef struct {
	char	type;
	char	l_name[NAME_SIZE];
	char	r_name[NAME_SIZE];
	char	filler;
	u_long	id_num;
	u_long	pid;
	char	r_tty[TTY_SIZE];
	struct	sockaddr_in addr;
	struct	sockaddr_in ctl_addr;
} CTL_MSG;

/* Control Response structure for earlier than BSD4.2
 */
typedef struct {
	char	type;
	char	answer;
	u_short	filler;
	u_long	id_num;
	struct	sockaddr_in addr;
} CTL_RESPONSE;

/* Control Message structure for BSD4.2
 */
typedef struct {
	u_char	vers;
	char	type;
	u_short	filler;
	u_long	id_num;
	struct	sockaddr_in addr;
	struct	sockaddr_in ctl_addr;
	long	pid;
	char	l_name[NAME_SIZE];
	char	l_name_filler[3];
	char	r_name[NAME_SIZE];
	char	r_name_filler[3];
	char	r_tty[TTY_SIZE];
} CTL_MSG42;

/* Control Response structure for BSD4.2
 */
typedef struct {
	u_char	vers;
	char	type;
	char	answer;
	char	filler;
	u_long	id_num;
	struct	sockaddr_in addr;
} CTL_RESPONSE42;


#define	TALK_VERSION	1		/* protocol version */

/* Types */
#define LEAVE_INVITE	0
#define LOOK_UP		1
#define DELETE		2
#define ANNOUNCE	3

/* Pause between sending announcements */
#define RING_WAIT	15
