/* fifozip.h */

/* ZIP compresses a fifo, returns error as defined in ziperr.h */
/* 0 is good, anything else means you've got a problem */
int pgp_create_zip(struct fifo *in,struct fifo *out);

/* ZIP decompresses a fifo, returns error as defined in ziperr.h */
/* 0 is good, anything else means you've got a problem */
int pgp_extract_zip(struct fifo *in,struct fifo *out);
