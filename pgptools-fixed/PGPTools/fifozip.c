/* fifozip.c */

#include <stdio.h>
#include <string.h>
#include "usuals.h"
#include "fifo.h"
#include "fifozip.h"
#include "zipup.h"
#include "zunzip.h"

/* ZIP compresses a fifo, returns error as defined in ziperr.h */
/* 0 is good, anything else means you've got a problem */
int pgp_create_zip(struct fifo *in,struct fifo *out)
{
int error;
fifo_put('\xa3',out);
fifo_put('\1',out);
error=zipup(in,out);
fifo_destroy(in);
return(error);
}

/* ZIP decompresses a fifo, returns error as defined in ziperr.h */
/* 0 is good, anything else means you've got a problem */
int pgp_extract_zip(struct fifo *in,struct fifo *out)
{
int error;
fifo_get(in);
fifo_get(in);
error=unzip(in,out);
fifo_destroy(in);
return(error);
}

