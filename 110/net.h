#include "arch.h"

#define SERVER_PORT 9897
#define WAKEUP_CALL 0x3A
#define NETTIMEOUT 120000000

#define NETTIMEOUTMIN 2 /* timeout of net transcation in minutes */
#define BLOCKTIMEOUT 12 /* timeout of keyblock in hours */

typedef struct {
	UINT4  request;  /* 0  = ?->?: Ack ok keyspace request - I've got it!
			  * 1   =  Request keyspace,
			  * 2   =  Report keyspace faliure,
			  * 3   =  Report keyspace success! :) :) :)
			  * 128 =  Here is your keyspace
			  * 129 =  Cannot give you keyspace, sorry
			  */
	UINT4 keyspace;	
	char RandomVec[8];	

} Server_Client_Request;

typedef struct {
  UINT4 request;
  UINT4 keyspace;
  char RandomVec[8];
  char platform;
  char email[111];        /* Should end up nicely being 128 bytes */
} Client_Server_Request;

#ifndef WIN32
#define HIWORD(x) ((x & 0xFFFF0000) >> 16)
#define LOWORD(x) (x & 0x0000FFFF)
#define HIBYTE(x) ((x & 0xFF00) >> 8)
#define LOBYTE(x) (x & 0x00FF)
#endif

unsigned int ServerConnect(char *server, int ipprint);

#ifdef BIT64
#define long int
#endif
unsigned long RequestKeyblock(int ServerSocket,char* Rvec);
unsigned long AckKeyblock(int ServerSocket,unsigned long keyspace);
unsigned long ReportKeyblock(int ServerSocket,char *email,UINT4 keyspace,int command,char* Rvec);
#ifdef BIT64
#undef long
#endif

#define swapl(x) {char t; t=x[0];x[0]=x[3];x[3]=t;t=x[1];x[1]=x[2];x[2]=t;}
#define swapll(x) {char t; t=x[0];x[0]=x[7];x[7]=t;\
       t=x[1];x[1]=x[6];x[6]=t;\
				      t=x[2];x[2]=x[5];x[5]=t;\
								t=x[3];x[3]=x[4];x[4]=t;}
#define swaplongs(x) {char t[4];\
    t[0]=x[0];t[1]=x[1];t[2]=x[2];t[3]=x[3];\
					        x[0]=x[4];x[1]=x[5];x[2]=x[6];x[3]=x[7];\
											  x[4]=t[0];x[5]=t[1];x[6]=t[2];x[7]=t[3];}

