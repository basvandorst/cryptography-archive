/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"
#include "cms.h"

#include "cms_proto.h"

int
tc_init_context (TC_CONTEXT **p, TC_MemoryMgr *memMgr, tc_sign_t *s,
				 void *sigdata, tc_verify_t *v, void *verdata)
{
	TC_CONTEXT *localcontext;
	int status;

	if (p == NULL)
		return TC_E_INVARGS; /* invalid argument */

	if (memMgr == NULL)
		memMgr = &PKIdefaultMemoryMgrStruct;
	else if (memMgr->allocProc == NULL ||
			 memMgr->reallocProc == NULL ||
			 memMgr->deallocProc == NULL)
		return TC_E_INVARGS; /* must have all the callbacks */

	localcontext = TC_Alloc (memMgr, sizeof (TC_CONTEXT));
	if (localcontext == NULL)
		return TC_E_NOMEMORY;

	localcontext->roots = (TC_LIST *)0;
	localcontext->handlers = (TC_LIST *)0;
	localcontext->crl = (TC_LIST *)0;
	localcontext->memMgr = memMgr;
	localcontext->verify = v;
	localcontext->verfuncdata = verdata;
	localcontext->sign = s;
	localcontext->sigfuncdata = sigdata;

	/* make the compiler generated code contest */
	localcontext->certasnctx = TC_Alloc(memMgr, sizeof(PKICONTEXT));
	if (localcontext->certasnctx == NULL)
	{
		TC_Free (memMgr, localcontext);
		return TC_E_NOMEMORY;
	}
	localcontext->certasnctx->customValue = NULL;
	localcontext->certasnctx->memMgr = memMgr;

	if ((status = InitAVAList(&localcontext->avaList, memMgr)) != 0)
	{
		TC_Free (memMgr, localcontext);
		return status;
	}

	/* register the critical extensions handled by default so that
	   tc_validate_path() does not return an error.  this is ok to do here
	   because the user can always override this value with their own
	   handler */
	tc_register_handler (PKIid_ce_basicConstraints_OID,
						PKIid_ce_basicConstraints_OID_LEN,
						NULL, /* handled directly by tc_validate_path */
						localcontext);

	tc_register_handler (PKIid_ce_nameConstraints_OID,
						 PKIid_ce_nameConstraints_OID_LEN,
						 NULL, /* handled directly */
						 localcontext);

	*p = localcontext;

	return 0; /* success */
}

void tc_free_cert (TC_CERT *p, TC_CONTEXT *ctx)
{
     if (p != NULL) {
	PKIFreeXCertificate (ctx->certasnctx, p->cert);
	PKIFreeCertificate(ctx->certasnctx, p->asnCertificate);
	TC_Free(ctx->memMgr, p);
     }
}

int tc_free_context (TC_CONTEXT *p)
{
  TC_LIST *tmp;
  TC_MemoryMgr *memMgr = p->memMgr;

  if (!p)
    return TC_E_INVARGS; /* invalid argument */

  /* free certificates */
  for (tmp = p->roots ; tmp ; tmp = p->roots)
  {
    p->roots = p->roots->next;
    tc_free_cert((TC_CERT *) tmp->data, p);
    TC_Free(memMgr, tmp);
  }

  /* free crls */
  for (tmp = p->crl; tmp; tmp = p->crl)
  {
      p->crl = p->crl->next;
      tc_free_crl(tmp->data, p);
      TC_Free(memMgr, tmp);
  }

  tc_free_handlers(&p->handlers, memMgr);

  FreeAVAList(p->avaList, memMgr);

  TC_Free(memMgr, p->certasnctx);

  TC_Free(memMgr, p);

  return 0; /* success */
}
