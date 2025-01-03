/* qbuf2pe.c - build PE(s) from an SSDU assumed to be in qbuf(s) */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/qbuf2pe_f.c,v 10.0 1993/02/18 20:14:20 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/ber/RCS/qbuf2pe_f.c,v 10.0 1993/02/18 20:14:20 awy Rel $
 *
 *
 * $Log: qbuf2pe_f.c,v $
 * Revision 10.0  1993/02/18  20:14:20  awy
 * Release IC-R1
 *
 */

/*
 *				  NOTICE
 *
 * Acquisition, use, and distribution of this module and related
 * materials are subject to the restrictions of a license agreement.
 * Consult the Preface in the User's Manual for the full terms of
 * this agreement.
 *
 */


/* LINTLIBRARY */

#include "stdio.h"
#include "asn1/asn1.h"
#ifdef DEBUG
#include "compat/tailor.h"
#endif

/*  */

static	int  qb_read_cons ();
static	qbuf2data();

/*
 * The following macro takes one byte from a qbuf, stuffs it in c,
 * and adjusts the qbuf.
 */

#ifdef DEBUG
#define qbuf2char(c)	{ \
				qp = Qb; \
				if (qp->qb_len > 0 && qp != Hqb) { \
					(c) = *qp->qb_data++; \
					Byteno++; \
					if(--qp->qb_len == 0) { \
						if(!(Qb = qp->qb_forw)) \
							abort(); \
					} \
				} \
				else { \
					SLOG (psap_log, LLOG_EXCEPTIONS, \
					  NULLCP, \
					  ("bad qbuf length=%d", \
					  ((qp == Hqb) ? 0: qp->qb_len))); \
					*result = PS_ERR_EOF; \
					if (pe) pe_free(pe); \
					return(NULLPE); \
				} \
			}
#else
#define qbuf2char(c)	{ \
				qp = Qb; \
				if (qp->qb_len > 0 && qp != Hqb) { \
					(c) = *qp->qb_data++; \
					Byteno++; \
					if(--qp->qb_len == 0) { \
						if(!(Qb = qp->qb_forw)) \
							abort(); \
					} \
				} \
				else { \
					*result = PS_ERR_EOF; \
					if (pe) pe_free(pe); \
					return(NULLPE); \
				} \
			}
#endif


int		Byteno = 0;
int		Qbrefs = 0;
struct qbuf    *Hqb = (struct qbuf *) NULL;
struct qbuf    *Fqb = (struct qbuf *) NULL;
struct qbuf    *Qb = (struct qbuf *) NULL;

static int pe_id_overshift = PE_ID_MASK << (PE_ID_BITS - PE_ID_SHIFT);

/*  */

PE
qbuf2pe_f (result)
int	*result;
{
	register PE	pe;
	register struct qbuf *qp;
	byte c, d;
	register PElementClass class;
	register PElementForm form;
	register PElementID id;
	register PElementLen   len;
	register int    i;
	register PElementLen j;

	pe = NULLPE;

	/*
	 * Just take the qbuf directly and build PEs.
	 * First, decode the id.
	 */

	qbuf2char(c);

	class = ((int)(c & PE_CLASS_MASK)) >> PE_CLASS_SHIFT;
	form = ((int)(c & PE_FORM_MASK)) >> PE_FORM_SHIFT;
	j = (c & PE_CODE_MASK);

	if (j == PE_ID_XTND)
		for (j = 0;; j <<= PE_ID_SHIFT) {
			qbuf2char(d);
			j |= d & PE_ID_MASK;
			if (!(d & PE_ID_MORE))
				break;
			if (j & pe_id_overshift) {
#ifdef DEBUG
			        DLOG (psap_log, LLOG_DEBUG,
				      ("j is %x d is %x %x %x %x\n",
				       j, d, *(Qb->qb_data-1),
				       *(Qb->qb_data-2),
				       *(Qb->qb_data-3)));
#endif
				*result = PS_ERR_OVERID;
				return (NULLPE);
			}
		}

	id = j;

#ifdef DEBUG
	DLOG (psap_log, LLOG_DEBUG,
		("class=%d form=%d id=%d", class, form, id));
#endif

	if ((pe = pe_alloc (class, form, id)) == NULLPE) {
		*result = PS_ERR_NMEM;
		return (NULLPE);
	}

	qbuf2char(c);

	if ((i = c) & PE_LEN_XTND) {
		if ((i &= PE_LEN_MASK) > sizeof (PElementLen)) {
#ifdef DEBUG
		        DLOG (psap_log, LLOG_DEBUG,
			      ("c (%x) i (%x) %d is %x %x %x %x\n",
			       c, i, sizeof(PElementLen),
			       *(Qb->qb_data-1),
			       *(Qb->qb_data-2),
			       *(Qb->qb_data-3),
			       *(Qb->qb_data-4)));
			qbprintf();
#endif
			*result = PS_ERR_OVERLEN;
			return (NULLPE);
		}

		if (i) {
			for (j = 0; i-- > 0;) {
				qbuf2char(c);
				j = (j << 8) | (c & 0xff);
			}
			len = j;
		}
		else
			len = PE_LEN_INDF;
	}
	else
		len = i;

#ifdef DEBUG
	SLOG (psap_log, LLOG_DEBUG, NULLCP, ("len=%d", len));
#endif
	pe -> pe_len = len;

	/* Now get the value.  */

	switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
		if (len == PE_LEN_INDF) {
			*result = PS_ERR_INDF;
			goto you_lose;
		}
		if (len > 0) {
			qp = Qb;
			if (qp->qb_len >= len) {
				pe->pe_prim = (PElementData) qp->qb_data;
				pe->pe_inline = 1;
				Qbrefs++;
				Byteno += len;
				if (qp->qb_len == len) {
					if(!(Qb = qp->qb_forw))
						abort();
				}
				else {
					qp->qb_len -= len;
					qp->qb_data += len;
				}
			}
			else {
				if ((pe->pe_prim = PEDalloc (len)) == NULLPED) {
					*result = PS_ERR_NMEM;
					goto you_lose;
				}
				if (qbuf2data(pe->pe_prim, len) != len) {
#ifdef DEBUG
					SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
						    ("bad qbuf lenght=%d",len));
#endif
					*result = PS_ERR_EOF;
					goto you_lose;
				}
			}
		}
		break;

	case PE_FORM_CONS: 
		if ((len == PE_LEN_INDF || len > 0) && 
			qb_read_cons (&pe -> pe_cons, len, result) == NOTOK)
				goto you_lose;
		break;
	}

	return pe;

you_lose: ;
#ifdef DEBUG
	if (psap_log -> ll_events & LLOG_PDUS) {
	    LLOG (psap_log, LLOG_PDUS, ("PE read thus far"));
	    pe2text (psap_log, pe, 1, *result);
	}
#endif
	
	pe_free (pe);
	return NULLPE;
}

/*  */

static	int  qb_read_cons (pe, len, cresult)
register PE	*pe;
register PElementLen len;
register int *cresult;
{
	register int    cc;
	register PE    p, q;
	int result;

	*pe = NULLPE;
	cc = Byteno + len;

	if ((p = qbuf2pe_f (&result)) == NULLPE) {
no_cons: ;
#ifdef	DEBUG
		if (len == PE_LEN_INDF)
			LLOG (psap_log, LLOG_DEBUG,
			  ("error building indefinite cons, %s",
				   ps_error (result)));
		else
			LLOG (psap_log, LLOG_DEBUG,
			  ("error building cons, stream at %d, wanted %d: %s",
				   Byteno, cc, ps_error (result)));
#endif

		*cresult = result;
		return NOTOK;
	}
	*pe = p;

	if (len == PE_LEN_INDF) {
		if (p->pe_class == PE_CLASS_UNIV && p->pe_id == PE_UNIV_EOC) {
			pe_free (p);
			*pe = NULLPE;
			return OK;
		}

		for(q = p; p = qbuf2pe_f(&result); q = q->pe_next = p) {
			if (p->pe_class == PE_CLASS_UNIV &&
						p->pe_id == PE_UNIV_EOC) {
				pe_free (p);
				return OK;
			}
		}
		goto no_cons;
	}

	for (q = p;; q = q -> pe_next = p) {
		if (cc < Byteno) {
#ifdef DEBUG
			DLOG (psap_log, LLOG_DEBUG,
			      ("cc %d Byteno %d last length was %d\n",
			       cc, Byteno));
			qbprintf();
#endif
			*cresult = PS_ERR_LEN;
			return (NOTOK);
		}
		if (cc == Byteno) {
			return OK;
		}
		if ((p = qbuf2pe_f (&result)) == NULLPE)
			goto no_cons;
	}
}

/*  */

static	qbuf2data(data, len)
PElementData data;
PElementLen len;
{
	register struct qbuf *qp;
	register int i, cc;

	for (qp = Qb, cc = 0; len > 0; data += i, cc += i, len -= i) {
		if (qp == Hqb)
			goto leave;

		i = min (qp -> qb_len, len);
		bcopy (qp -> qb_data, (char *) data, i);

		qp -> qb_len -= i;
		if (qp -> qb_len <= 0) {
			if(!(Qb = (qp = qp->qb_forw)))
				abort();
		}
		else
			qp->qb_data += i;
	}

leave:
	Byteno += cc;
	return cc;
}

/*  */

#ifdef DEBUG
qbprintf()
{
	int len;
	struct qbuf *qb;
	char *cp;

	for(qb = Fqb, cp = qb->qb_data;qb != Hqb;qb = qb->qb_forw) {
		for(len = 0;len < qb->qb_len;cp++, len++) {
#ifdef DEBUG
		    (void) ll_printf (psap_log, "%x ", *cp);
		    if((len % 15) == 0) (void) ll_printf(psap_log, "\n");
#endif
		}
	}
#ifdef DEBUG
	(void) ll_sync (psap_log);
#endif
}
#endif
