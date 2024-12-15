/* Use conditional compile so header file is only included once */
#ifndef _BFSTREAM_H_
#define _BFSTREAM_H_ 1

#ifdef __STDC__
# define  P(s) s
#else
# define P(s) ()
#endif

typedef struct {
  unsigned char *buffer;
  /* point points to the first byte after the data in buffer.  It is
       incremented as data is written to buffer and gives the length
       of the valid data. */
  unsigned int point;
  unsigned int maxBufferLen;                         /* Total size of buffer */
} BufferStream;

void BufferStreamConstructor P((BufferStream *));
void BufferStreamDestructor P((BufferStream *));
char *BufferStreamWrite P((unsigned char *, unsigned int, BufferStream *));
char *BufferStreamPuts P((char *, BufferStream *));
char *BufferStreamPutc P((int, BufferStream *));
void BufferStreamRewind P((BufferStream *));

#undef P

#endif
