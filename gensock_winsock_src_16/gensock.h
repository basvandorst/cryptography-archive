#define ERR_CANT_MALLOC			4001
#define ERR_SENDING_DATA		4002
#define ERR_INITIALIZING		4003
#define ERR_VER_NOT_SUPPORTED		4004
#define ERR_EINVAL			4005
#define ERR_SYS_NOT_READY		4006
#define ERR_CANT_RESOLVE_HOSTNAME	4007
#define ERR_CANT_GET_SOCKET		4008
#define ERR_READING_SOCKET		4009
#define ERR_NOT_A_SOCKET		4010
#define ERR_BUSY			4011
#define ERR_CLOSING			4012
#define WAIT_A_BIT			4013
#define ERR_CANT_RESOLVE_SERVICE	4014
#define ERR_CANT_CONNECT		4015
#define ERR_NOT_CONNECTED		4016
#define ERR_CONNECTION_REFUSED		4017

typedef void FAR * socktag;

#ifdef __cplusplus
extern "C" {
#endif
/* function prototypes */

int	FAR PASCAL gensock_connect 	(char FAR * hostname,
					 char FAR * service,
					 socktag FAR * pst);

int	FAR PASCAL gensock_getchar 	(socktag st, int wait, char FAR * ch);

int	FAR PASCAL gensock_put_data 	(socktag st,
					 char FAR * data,
					 unsigned long length);

int	FAR PASCAL gensock_close	(socktag st);

#ifdef __cplusplus
}
#endif

