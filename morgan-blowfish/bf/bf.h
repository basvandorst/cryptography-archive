#ifndef _bf_h
#define _bf_h
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_LOCAL_BUFFER_DEF
typedef unsigned long UINT32;
typedef unsigned int UINT16;
typedef int INT16;
// We will define BUFFER here...
typedef struct{
	char	*buff;/* buffer */
	int	N; /* Number of elements in buffer */
} BUFFER;
typedef struct{
        UINT32  *buff;
        int     N;
} LBUFFER;
#else // USE_LOCAL_BUFFER_DEF
// BUFFER and LBUFFER are defined elsewhere...
#include "..\buffer\buffer.h"
#endif // USE_LOCAL_BUFFER_DEF


#ifdef USE_LOCAL_ERRORS
#define NOT_DIVISABLE           10
#define NO_ERROR 		0
#else  // USE_LOCAL_ERRORS
#include "..\errors\errors.h"
#endif // USE_LOCAL_ERRORS


#define MAX_P_ELEMENTS 226
#define MIN_P_ELEMENTS 2


// class Specifier...
class blow_this{

// Private data...
private:
UINT16  p_N;// Number of elments in P array
UINT32 	*P,	// P array
// NOTE:  I just had a theory that if S<0:3> were
//        one demensional arrays, the program would
//        run faster.  I never verified this.
		*S0,
		*S1,
		*S2,
		*S3;
// public data...
public:
blow_this(UINT16 P_elements=18);        // constructor
~blow_this(void);                       // destructor
void    initbf(BUFFER key);             // inits subkey with key
void    bfe(UINT32 *xl,UINT32 *xr);     // encrypts, general
void    bfe16(UINT32 *xl, UINT32 *xr);  // encrypts, 16 rounds only
void    bfd(UINT32 *xl,UINT32 *xr);     // decrypts, general
void    bfd16(UINT32 *xl,UINT32 *xr);   // decrypts, 16 rounds only
bfebuffer(BUFFER *buffer);              // encrypts, buffer, general
bfebuffer(LBUFFER *lbuffer);            // encrypts, lbuffer, general
bfebuffer16(BUFFER *buffer);            // encrypts, buffer, 16 rounds only
bfebuffer16(LBUFFER *lbuffer);          // encrypts, lbuffer, 16 rounds only
bfdbuffer(BUFFER *buffer);              // decrypts, buffer, general
bfdbuffer(LBUFFER *lbuffer);            // decrypts, lbuffer, general
bfdbuffer16(BUFFER *buffer);            // decrypts, buffer, 16 rounds only
bfdbuffer16(LBUFFER *lbuffer);            // decrypts, lbuffer, 16 rounds only
};
#endif // _bf_h
