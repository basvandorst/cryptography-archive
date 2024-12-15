/* fifo.h */

/* Bytes per block in a fifo. Should be a multiple of 8 */
#define FIFO_BLOCK_SIZE 512

struct fifo {
  struct fifo_block *first;
  unsigned pos;
  struct fifo_block *last;
  FILE *fp;
  word32 floc;
  word32 flen;
  byte flac;
  };

struct fifo_block {
  struct fifo_data *data;
  unsigned size;
  struct fifo_block *next;
  };

struct fifo_data {
  byte array[FIFO_BLOCK_SIZE];
  unsigned links;
  };

/* Create a new fifo in memory */
struct fifo *fifo_mem_create(void);

/* Create a disk file fifo */
struct fifo *fifo_file_create(FILE *fp);

/* Put a character into a fifo */
void fifo_put(byte c,struct fifo *f);

/* Get a character from a fifo, returns -1 if empty */
int fifo_get(struct fifo *f);

/* Put an array into a fifo */
void fifo_aput(void *c,word32 num,struct fifo *f);

/* Get num characters from a fifo, returns num actually copied */
word32 fifo_aget(void *c,word32 num,struct fifo *f);

/* Random-access get a byte from a fifo */
byte fifo_rget(word32 loc,struct fifo *f);

/* Random-access put a byte to a fifo */
void fifo_rput(byte c,word32 loc,struct fifo *f);

/* Return length of a fifo */
word32 fifo_length(struct fifo *f);

/* Create a duplicate of a fifo */
struct fifo *fifo_copy(struct fifo *f);

/* Clear last-accessed field of a disk fifo. Used when you want to access
   two copies of a disk fifo simultaneously. Will not do any damage if
   done to a memory fifo. */
void fifo_clearlac(struct fifo *f);

/* Skip length bytes in fifo */
void fifo_skipn(struct fifo *f,word32 length);

/* Move n characters from i to o */
void fifo_moven(struct fifo *i,struct fifo *o,word32 n);

/* Find pattern in fifo f, returns true if found, false if not
   Fifo is at end if false, just after pattern if true */
int fifo_find(byte *pattern,struct fifo *f);

/* Append b to a, returning new fifo as a, destroying b */ 
void fifo_append(struct fifo *a,struct fifo *b);

/* De-allocate a fifo */
void fifo_destroy(struct fifo *f);

