/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "tc.h"
#include "cms_proto.h"
#include "str2.h"

#if 0
/* not used ??? */
void *str2vbl_blk( char *str, TC_CONTEXT *ctx )
{
  PKIVariableBlock *asn = PKINewVariableBlock(ctx->certasnctx);
  PKIPutOctVal(ctx->certasnctx, (PKIVariableBlock *)asn,
	       (unsigned char *)str, strlen(str) );
  return asn;
}
#endif

static int encode_number (unsigned char **p, size_t *plen, long n,
			  TC_CONTEXT *ctx)
{
  int j;
  int bytes;

  if (n < 128)
    bytes = 1;
  else if (n < 128 * 128)
    bytes = 2;
  else if (n < 128 * 128 * 128)
    bytes = 3;
  else
    bytes = 4;

  if (*p)
    TC_Realloc(ctx->memMgr, (void **)&(*p), *plen + bytes);
  else
    *p = TC_Alloc(ctx->memMgr, *plen + bytes);
  if (!*p)
    return TC_E_NOMEMORY;

  (*p)[*plen + bytes - 1] = n % 128;
  n /= 128;

  for (j = bytes - 2 ; j >= 0; j--)
  {
    (*p)[*plen + j] = (n % 128) | 0x80;
    n /= 128;
  }

  *plen += bytes;

  return 0;
}

/* create a DER-encoded OBJECT-IDENTIFIER from a string representation */
int tc_create_oid (unsigned char **p, size_t *plen, const char *oid,
		   TC_CONTEXT *ctx)
{
  char *oidCopy = NULL;
  char *curPosition;
  char *newPosition;
  int firstIntNumber = 0;
  int firstDERNumber = 0;
  long curNumber = 0;
  int status = 0;

  if (p == NULL || plen == NULL)
      return TC_E_INVARGS;

  oidCopy = TC_Alloc(ctx->memMgr, strlen(oid) );
  if (oidCopy == NULL)
      return TC_E_NOMEMORY;
  strcpy(oidCopy, oid);
  curPosition = oidCopy;

  *plen = 0;
  *p = NULL;

    /* While there are valid number in the provided string
       convert them */
    while (1) {
        curNumber = strtol(curPosition, &newPosition, 10);
        if (newPosition == curPosition && curNumber == 0)
	    break;

        switch (firstIntNumber) {
        case 0:
	    firstDERNumber = 40 * curNumber;
	    break;
        default:
	    if ((status = 
                   encode_number(p, plen,
                                 firstDERNumber + curNumber,
				 ctx)) != 0) {
	        TC_Free(ctx->memMgr, oidCopy);
	        /* don't really need to free *p since the only way
                   encode_number() fails is if malloc fails and that
                   leaves *p NULL */
		if (*p != NULL) TC_Free(ctx->memMgr, *p);
		*plen = 0;
	        return status;
	    }
	    firstDERNumber = 0;
	    break;
       }

       curPosition = newPosition + 1; /* need to skip over '.' */
       firstIntNumber++;

    } /* while */

    TC_Free(ctx->memMgr, oidCopy);
    return 0;
}
