/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"
#include "cms.h"

#include "cms_proto.h"

/*****
*
* tc_delete_cert
*
* Remove a certificate from the context, if the certificate is not in
* the context, return an error.
*
* parameters
*   input
*	issuer - the issuer's name
*	serial - the certificate's serial number
*	ctx - the context to remove the cert. from
*
* return
*    0 - okay
*    TC_E_INVARGS - provided args were NULL
*    TC_E_NOTFOUND - a crl for the issuer was not found
*
*****/
int tc_delete_cert (TC_Name *issuer, TC_INTEGER *serial, TC_CONTEXT *ctx)
{
  TC_CERT *pc;
  TC_LIST *tl;
  TC_LIST *last = NULL;
  int r = TC_E_NOTFOUND;

  if (issuer == NULL || serial == NULL || ctx == NULL)
      return TC_E_INVARGS;

  for (tl = ctx->roots; tl; last = tl, tl = tl->next) {
      pc = (TC_CERT *)tl->data;

      if (tc_compare_serial(&pc->tbsCertificate->serialNumber, serial)) {

          if (tc_compare_dname(&pc->tbsCertificate->issuer,
			       issuer, ctx) == 0) {
	      if (last)
	          last->next = tl->next;
	      else
	          ctx->roots = tl->next;
	      tc_free_cert((TC_CERT *) tl->data, ctx);
	      TC_Free(ctx->memMgr, tl);
	      r = 0;
	      break;
          }
      } /* serial #'s are equal */
  } /* for each cert. */
  return (r);
}