/*	idea.h - header file for idea.c
*/

typedef unsigned char boolean;	/* values are TRUE or FALSE */
typedef unsigned char byte;	/* values are 0-255 */
typedef byte *byteptr;	/* pointer to byte */
typedef char *string;	/* pointer to ASCII character string */
typedef unsigned short word16;	/* values are 0-65535 */
typedef unsigned long word32;	/* values are 0-4294967295 */

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif	/* if TRUE not already defined */

#ifndef min	/* if min macro not already defined */
#define min(a,b) ( (a)<(b) ? (a) : (b) )
#define max(a,b) ( (a)>(b) ? (a) : (b) )
#endif	/* if min macro not already defined */

#define IDEAKEYSIZE 16
#define IDEABLOCKSIZE 8
#define IDEA32

void initkey_idea(byte key[16], boolean);
void idea_ecb(word16 *, word16 *);
void close_idea(void);

#if !defined(__i386__) && !defined(__i486__)
#define HIGHFIRST
#endif
