/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/


#include <string.h>

#include "tc.h"
#include "cms.h"

/* write an AVA to the specified structure.  Since the value is of type ANY,
 * the user must specify the tag value to be used.  This routine then
 * encodes the value as an ASN.1 blob.
 */
/* TODO: move away from this routine, CMS code should not have to
   hand craft DER data, instead use compiler calls to make the
   needed DER and then call tc_set_avader below.  */
int tc_set_ava (PKIAttributeTypeAndValue *p,
		const unsigned char *type,
		size_t typelen,
		unsigned char valtag,
		const unsigned char *val,
		size_t vallen,
		TC_CONTEXT *ctx)
{
  size_t bytesused;
  unsigned char *buf;

  if (p == NULL)
      return TC_E_INVARGS;

  PKIPutOctVal(ctx->certasnctx, &p->type, type, typelen);

  /* length of the data + tag byte + bytes for length value */
  p->value.len = vallen + 1 + PKILengthSize(vallen);
  p->value.val = TC_Alloc(ctx->memMgr, p->value.len);
  if (p->value.val == NULL)
  {
    TC_Free(ctx->memMgr, p->type.val);
    p->type.val = NULL;
    return TC_E_NOMEMORY;
  }

  buf = p->value.val;
  bytesused = PKIPutByte(buf, valtag);	/* write the tag */
  bytesused += PKIPutLength(buf+bytesused, vallen);
  (void)memcpy(buf+bytesused, val, vallen); /* write the value data */

  return 0;
}

/* write an AVA to the specified structure.  This version assumes the
 * user provided valid DER for the attributes value.
 */
int tc_set_avader (PKIAttributeTypeAndValue *p,
		const unsigned char *type,
		size_t typelen,
		const unsigned char *val,
		size_t vallen,
		TC_CONTEXT *ctx)
{
  if (p == NULL)
      return TC_E_INVARGS;

  PKIPutOctVal(ctx->certasnctx, &p->type, type, typelen);

  PKIPutOctVal(ctx->certasnctx, &p->value, val, vallen);

  return 0;
}

int tc_get_ava (unsigned char *valtag,
		unsigned char **val,
		size_t *vallen,
		PKIAttributeValue *p)
{
  unsigned char *buf;
  size_t bytesused;

  if (!valtag || !val || !vallen || !p)
    return TC_E_INVARGS;

  buf = p->val;
  bytesused = PKIGetByte(buf, valtag);    /* extract the tag */
  bytesused += PKIGetLength(buf+bytesused,
			    vallen); /* extract length of data */
  *val = buf+bytesused;			/* return a pointer to the data */

  return 0; /* success */
}