
#include "bcopy.h"

#define OSIZE         128   /* size of buffers for I/O */

/* packet stuff */
#define MAXEXTEND     64
#define EXTEND        0x80
#define NEWKEY        0x01
#define ACK           0x02
#define ACK2          0x03

#define FLUSHTIME     50000  /* 50 msec */

/* files for rsa keys */
#define PRIVFILE  ".linkprv"
#define PUBFILE   ".linkpub"

/* null des key */
#define NULLKEY "\0\0\0\0\0\0\0\0"

/* character to start with */
#define STARTCHAR '%'

/* all i/o done into these buffers */
extern int loc_len,rem_len;
extern char loc_out[],rem_out[];
#define WRITE_LOC(data,len)   (bcopy(data,loc_out+loc_len,len),(loc_len)+=len)
#define WRITE_REM(data,len)   (bcopy(data,rem_out+rem_len,len),(rem_len)+=len)

