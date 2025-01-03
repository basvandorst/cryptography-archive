/* enc.c */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/pepsy2/RCS/enc.c,v 10.0 1993/02/18 20:15:15 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/pepsy2/RCS/enc.c,v 10.0 1993/02/18 20:15:15 awy Rel $
 *
 *
 * $Log: enc.c,v $
 * Revision 10.0  1993/02/18  20:15:15  awy
 * Release IC-R1
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include	<stdio.h>
#include	<ctype.h>
#include	"pepsy-driver.h"
#include	"asn1/asn1.h"
#include	"asn1/pepsy.h"
#ifdef DEBUG
#include 	"compat/tailor.h"
#endif

#ifndef	PEPYPARM
#define PEPYPARM	char *
#endif


extern ptpe *next_tpe(), *fdflt_f();
extern char *pr_petype();
char   *idname(), *clname();

#define NEXT_TPE(p) (p = next_tpe(p))
#define CHKTAG(mod, p, pe)	ismatch(p, mod, pe->pe_class, pe->pe_id)

static char oomsg[] = "Out of memory";
#define oom(a,b)	pepsylose ((a), (b), NULLPE, oomsg);
#define RET_OK(rpe, pe)		*(rpe) = (pe), (OK)
#define pname(t)	((t)->pe_typename ? *(t)->pe_typename : "???")

static int en_obj ();
static int en_type();
static int en_seq();
static int en_set();
static int en_seqof();
static int en_setof();
static int en_choice();
static int en_etype();

/*
 * encode the specified type of the specified module into the given
 * pe
 */
enc_f(typ, mod, pe, explicit, len, buf, parm)
/* ARGSUSED */
int     typ;			/* which type it is */
modtyp *mod;			/* Module it is from */
register PE *pe;
int     explicit;
int     len;
char   *buf;
char	*parm;
{
    register ptpe *p;

    if (typ < 0 || typ >= mod->md_nentries) {
	(void) pepsylose (mod, NULLTPE, NULLPE, "enc_f:Illegal typ %d", typ);
	return NOTOK;
    }

    p = mod->md_etab[typ];
    if (p->pe_type != PE_START) {
	return (pepsylose (mod, NULLTPE, NULLPE, "enc_f: missing PE_START"));
    }
#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encoding %s.%s",
				 mod -> md_name, pname(p)));
#endif

    p++;

    if (en_obj(parm, p, mod, pe) == NOTOK) {
#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG, ("Encoding %s.%s failed",
					 mod -> md_name, pname(p)));
#endif
	    return NOTOK;
    }
#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encoding %s.%s suceeded",
				 mod -> md_name, pname(p)));
#endif
    return OK;
}

/*
 * Encode an object. If the object is a simple type it may have a
 * compressed type reference. If it is a compound type it will not
 * have an offset. This is very important as it means we cannot just
 * use en_type to handle this which must always assume the field can
 * have an offset.
 */
static int
en_obj(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    PE      pe = NULLPE;	/* for pepsylose calls */
    int     cnt = 0;

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode object %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif

    *rpe = NULLPE;	/* default case */

    while (p->pe_type != PE_END) {

	switch (p->pe_type) {
	case PE_END:
	case PE_START:
	    return (pepsylose (mod, p, pe, "en_obj:END/START type"));

	/*
	 * This allows Functions to be called at the very end of the 
	 * encoding -- With the encoded data - very messy
	 */
	case UCODE:
	    if (mod->md_eucode == NULLIFP
	    || (*mod->md_eucode) (parm, &pe, p) == NOTOK)
		goto bad;
	    break;

	default:
	    if (en_type(parm, p, mod, &pe) == NOTOK)
		goto bad;
	    break;
	}
	if (ISDTYPE(p) && cnt++ > 0)
	    return pepsylose (mod, p, NULLPE, "en_obj:compound type found");

	if (ISDTYPE(p)) {
	    if (pe == NULLPE)
		return pepsylose (mod, p, NULLPE,
				  "en_obj: missing mandatory value");
	}
	/* make sure any final UCODEs get run
	if (ISDTYPE(p) && pe != NULLPE)
	    return (RET_OK(rpe, pe));
	 */

	if (NEXT_TPE(p) == NULLTPE)
	    goto bad;
    }

    return (RET_OK(rpe, pe));

bad:
    return (NOTOK);
}

/*
 * Encode a single type. If a basic type encode it, if a compound
 * type call the appropriate encoding routine
 */
static int
en_type(parm, p, mod, rpe)
register PEPYPARM parm;
register ptpe    *p;
register modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    PE      pe = NULLPE;
    int     cnt = 0;
    int     i;			/* Integer for encoding type */
    ptpe    *tmp;
    char   *cp;

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode type %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif

    *rpe = NULLPE;
    while (p->pe_type != PE_END) {

	switch (p->pe_type) {
	case PE_END:
	case PE_START:
	    return (pepsylose (mod, p, pe, "en_type:END/START type"));

	case DFLT_F:
	    tmp = p;
	    p = FDFLT_F(p);
	    if ((i = same(p, tmp, parm, mod)) == NOTOK)
		return (NOTOK); /* Error */
	    if (i)
		return (RET_OK(rpe, NULLPE));/* don't encode it */
	    p = tmp + 1;
	    continue;

	case BOPTIONAL:
	    if (IF_USELECT(p)) {
		if (mod -> md_eucode == NULLIFP)
		    goto bad;
		if ((*mod->md_eucode) (parm, &pe, p) == 0)
		    return (RET_OK(rpe,NULLPE)); /* don't encode it */
	    } else if (!OPT_PRESENT(p, parm))
		return (RET_OK(rpe, NULLPE));/* don't encode it */
	    p++; /* encode it */
	    continue;

	case UCODE:
	    if (mod->md_eucode == NULLIFP
	    || (*mod->md_eucode) (parm, &pe, p) == NOTOK)
		goto bad;
	    break;

	case ETAG:
	    if ((pe = pe_alloc(CLASS(p), PE_FORM_CONS, TAG(p))) == NULLPE)
		return oom (mod, p);

	    switch (p->pe_ucode) {

	    default:
		p++;
		if (en_etype(parm, p, mod, &pe->pe_cons) == NOTOK)
		    goto bad;
	    }
	    break;

	case SEQ_START:
	    if (en_seq(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEQOF_START:
	    if (en_seqof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SET_START:
	    if (en_set(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SETOF_START:
	    if (en_setof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SSEQ_START:
	    if (en_seq(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SSEQOF_START:
	    if (en_seqof(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SSET_START:
	    if (en_set(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SSETOF_START:
	    if (en_setof(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case IMP_OBJ:
	    tmp = p++;
	    if (p->pe_type == EXTOBJ) {
		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		      0, (char *)0, *(char **) (parm + p->pe_ucode)) == NOTOK)
		      return (NOTOK);
	    } else if (p->pe_type == SEXTOBJ) {
		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		      0, (char *)0, (parm + p->pe_ucode)) == NOTOK)
		      return (NOTOK);
	    } else if (p->pe_type == SOBJECT) {
		if (en_obj((char *) (parm + p->pe_ucode), mod->md_etab[p->pe_tag] + 1, mod, &pe)
		  == NOTOK)
		     goto bad;;
	    } else
		if (en_obj(*(char **) (parm + p->pe_ucode),
			mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		    goto bad;

	    pe->pe_class = CLASS(tmp);
	    pe->pe_id = TAG(tmp);
	    break;

	case SOBJECT:
	    if (en_obj((char *) (parm + p->pe_ucode), mod->md_etab[p->pe_tag] + 1, mod, &pe)
	      == NOTOK)
		goto bad;
	    break;

	case OBJECT:
	    if (en_obj(*(char **) (parm + p->pe_ucode),
	      mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case CHOICE_START:
	    if (en_choice(*(char **) (parm + p->pe_ucode), p, mod, &pe) ==NOTOK)
		goto bad;
	    break;

	case SCHOICE_START:
	    if (en_choice(parm + p->pe_ucode, p, mod, &pe) ==NOTOK)
		goto bad;
	    break;

	case SEXTOBJ:
	    if (p[1].pe_type != EXTMOD)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_seq: missing EXTMOD");
	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		  (char *) (parm + p->pe_ucode)) == NOTOK)
		  return (NOTOK);
	    break;

	case EXTOBJ:
	    if (p[1].pe_type != EXTMOD)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_seq: missing EXTMOD");
	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		  *(char **) (parm + p->pe_ucode)) == NOTOK)
		  return (NOTOK);
	    break;

	case INTEGER:
#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG,
		  ("Encode INTEGER %s.%s value %d",
		   mod -> md_name, pname(p),
		   *(integer *)(parm + p->pe_ucode)));
#endif

	    if ((pe = num2prim(*(integer *) (parm + p->pe_ucode),
			       CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

#ifdef	PEPSY_REALS
	case REALTYPE:
#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG, ("Encode REAL %s.%s value %g",
					 mod -> md_name, pname(p),
					 *(double *) (parm + p->pe_ucode)));
#endif

	    if ((pe = real2prim(*(double *) (parm + p->pe_ucode),
			       CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

#endif
	case BOOLEAN:
#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG,
		  ("Encode BOOLEAN %s.%s value %d",
		   mod -> md_name, pname(p),
		   *(char *) (parm + p->pe_ucode)));
#endif

	    if ((pe = flag2prim(*(char *) (parm + p->pe_ucode),
				CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

	case T_NULL:
#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG,
		  ("Encode NULL %S.%S",
		   mod -> md_name, pname(p)));
#endif


	    if ((pe = pe_alloc(CLASS(p), PE_FORM_PRIM,
			       TAG(p))) == NULLPE)
		return oom(mod,p);

	    break;

	case SANY:
	    (pe = (PE) (parm + p->pe_ucode))->pe_refcnt++;
	    break;

	case ANY:
	    if ((parm + p->pe_ucode) == 0 || *(PE *) (parm + p->pe_ucode) == 0)
#if ROSAP_HACK
		/* hack for ROSAP. expects this strangeness */
		pe = pe_alloc(PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_NULL);
#else
		pe = NULLPE;
#endif
	    else
		(pe = *(PE *) (parm + p->pe_ucode))->pe_refcnt++;
	    break;

	case SOCTETSTRING:
	    if ((pe = qb2prim((struct qbuf *) (parm + p->pe_ucode), CLASS(p), TAG(p)))
		== NULLPE)
		return oom(mod, p)
	    break;

	case OCTETSTRING:
	    if ((pe = qb2prim(*(struct qbuf **) (parm + p->pe_ucode),
			      CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

	case T_STRING:
	    if (*(char **) (parm + p->pe_ucode) == NULLCP)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:T_STRING missing pointer");
	    if ((pe = str2prim(*(char **) (parm + p->pe_ucode),
			      strlen(*(char **) (parm + p->pe_ucode)),
			      CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

	case OCTET_PTR:
	    if (p[1].pe_type != OCTET_LEN)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:missing OCTET_LEN");

	    if (*(char **) (parm + p->pe_ucode) == NULLCP)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:OCTET_PTR  missing pointer");

	    if (*(int *) (parm + (p + 1)->pe_ucode) < 0)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:OCTET_LEN negative length");

	    if ((pe = str2prim(*(char **) (parm + p->pe_ucode),
			      *(int *) (parm + (p + 1)->pe_ucode),
			      CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

	case BITSTR_PTR:
	    if (p[1].pe_type != BITSTR_LEN)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:missing BITSTR_LEN");

	    if (*(char **) (parm + p->pe_ucode) == NULLCP)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:BITSTR_PTR  missing pointer");

	    if (*(int *) (parm + (p + 1)->pe_ucode) < 0)
		return pepsylose (mod, &p[1], NULLPE,
				  "en_type:BITSTR_LEN negative length");

	    if ((pe = strb2bitstr(*(char **) (parm + p->pe_ucode),
			      *(int *) (parm + (p + 1)->pe_ucode),
			      CLASS(p), TAG(p))) == NULLPE
		|| (pe = bit2prim(pe)) == NULLPE)
		return oom(mod, p);
	    break;

	case SBITSTRING:
	    if ((cp = bitstr2strb((PE) (parm + p->pe_ucode), &i)) == NULL)
		return oom(mod, p);
	    if ((pe = strb2bitstr(cp, i, CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    free(cp);
	    if ((pe = bit2prim(pe)) == NULLPE)
		return oom(mod, p);
	    break;

	case BITSTRING:
	    if ((cp = bitstr2strb(*(PE *) (parm + p->pe_ucode), &i))
		== NULL)
		return oom(mod, p);

	    if ((pe = strb2bitstr(cp, i, CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);

	    free(cp);
	    if ((pe = bit2prim(pe)) == NULLPE)
		return oom(mod, p);
	    break;

	case SOBJID:
	    if ((pe = obj2prim((OID) (parm + p->pe_ucode), CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

	case OBJID:
	    if ((pe = obj2prim(*(OID *) (parm + p->pe_ucode), CLASS(p), TAG(p)))
		== NULLPE)
		return oom(mod, p);
	    break;

	case FN_CALL:
	    if ((FN_PTR(mod, p))(parm, &pe) == NOTOK)
		return pepsylose (mod, p, NULLPE,
				  "en_type:FN_CALL:call failed");
	    if (STAG(p) >= 0) {
		pe->pe_class = CLASS(p);
		pe->pe_id = TAG(p);
	    }
	    break;

	default:
	    return pepsylose (mod, p, NULLPE, "en_type: type not implemented");
	}
	if (ISDTYPE(p) && cnt++ > 0)
	    return pepsylose (mod, p, NULLPE, "en_type:compound type found");

	if (ISDTYPE(p)) {
	    if (pe == NULLPE)
		return pepsylose (mod, p, NULLPE,
				  "en_type: missing mandatory value");
	}
	if (ISDTYPE(p) && pe != NULLPE)
	    return (RET_OK(rpe, pe));
	if (NEXT_TPE(p) == NULLTPE)
	    goto bad;
    }

    return (RET_OK(rpe, pe));

bad:
    return (NOTOK);
}

/*
 * Build a sequence, calling appropriate routines to build each sub
 * type
 */
static int
en_seq(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    PE      head;
    PE      pe = NULLPE;
    ptpe    *tmp;		/* first entry in list */
    int    *popt = NULL;	/* Pointer to optional field */
    int     optcnt = 0;		/* Number of optionals bits so far */
    int	    val;

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode SEQUENCE %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif

    if (p->pe_type != SEQ_START && p->pe_type != SSEQ_START)
	    return (pepsylose (mod, p, pe, "en_seq: missing SEQ_START\n"));

    if ((head = pe_alloc(CLASS(p), PE_FORM_CONS, TAG(p))) == NULLPE)
	return oom (mod, p);
    p++;

    while (p->pe_type != PE_END) {

#ifdef DEBUG
	DLOG (psap_log, LLOG_DEBUG, ("Encode SEQUENCE member %s.%s type %s",
				     mod -> md_name, pname(p),
				     pr_petype(p->pe_type)));
#endif

	if (ISDTYPE(p) && OPTIONAL(p)) {
	    switch (p->pe_type) {
	    case INTEGER:
	    case REALTYPE:
	    case BOOLEAN:
	    case T_NULL:
		if (!TESTBIT(*popt, optcnt++))
		    goto next;	/* Missing so skip */
		break;

	    case FN_CALL:
		if ((val = hasdata(parm, p, mod, popt, &optcnt)) == NOTOK)
		    goto bad;
		if (val == 0)
		    goto next;
		break;

	    case ETAG:
		if ((val = hasdata(parm, p + 1, mod, popt, &optcnt)) == NOTOK)
		    goto bad;
		if (val == 0)
		    goto next;
		break;

	    case IMP_OBJ:
		if ((p[1].pe_type == SOBJECT && parm == NULL)
		    || *((char **) (parm + p[1].pe_ucode)) == NULL)
		    goto next;
		break;

	    case SOBJECT:
		if (((char *) parm) == NULL)
		    goto next;
		break;

	    default:
		if (*((char **) (parm + p->pe_ucode)) == NULL)
		    goto next;
		break;
	    }
	}
	switch (p->pe_type) {
	case OPTL:
	    popt = (int *) (parm + p->pe_ucode);
	    break;

	case UCODE:
	    if (mod->md_eucode == NULLIFP
	    || (*mod->md_eucode) (parm, &pe, p) == NOTOK)
		goto bad;
	    break;

	case ETAG:
	    if (en_type(parm, p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEQ_START:
	    if (en_seq(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEQOF_START:
	    if (en_seqof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SET_START:
	    if (en_set(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SETOF_START:
	    if (en_setof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case IMP_OBJ:
	    tmp = p++;
	    if (p->pe_type == EXTOBJ || p->pe_type == SEXTOBJ) {
		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		      0, (char *)0, *(char **) (parm + p->pe_ucode)) == NOTOK)
		      return (NOTOK);
	    } else if (p->pe_type == SOBJECT) {
		if (en_obj((char *) (parm + p->pe_ucode), mod->md_etab[p->pe_tag] + 1, mod,
		      &pe) == NOTOK)
		    goto bad;
	    } else
		if (en_obj(*(char **) (parm + p->pe_ucode),
			mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		    goto bad;
	    pe->pe_class = CLASS(tmp);
	    pe->pe_id = TAG(tmp);
	    break;

	case SOBJECT:
	    if (en_obj((char *) (parm + p->pe_ucode), mod->md_etab[p->pe_tag] + 1, mod, &pe)
		    == NOTOK)
		goto bad;
	    break;

	case OBJECT:
	    if (en_obj(*(char **) (parm + p->pe_ucode),
		    mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case CHOICE_START:
	    if (en_choice(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SCHOICE_START:
	    if (en_choice((char *) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEXTOBJ:
	    if (p[1].pe_type != EXTMOD)
		return pepsylose (mod, &p[1], NULLPE, "en_seq: missing EXTMOD");
	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		  (char *) (parm + p->pe_ucode)) == NOTOK)
		  return (NOTOK);
	    break;

	case EXTOBJ:
	    if (p[1].pe_type != EXTMOD)
		return pepsylose (mod, &p[1], NULLPE, "en_seq: missing EXTMOD");

	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		  *(char **) (parm + p->pe_ucode)) == NOTOK)
		  return (NOTOK);
	    break;

	default:
	    if (en_type(parm, p, mod, &pe) == NOTOK)
		goto bad;
	    break;
	}

	if ((ISDTYPE(p) || p->pe_type == BOPTIONAL) && pe != NULLPE) {
	    if (seq_add(head, pe, -1) == NOTOK)
		return pepsylose (mod, p, NULLPE, "en_seq bad sequence: %s",
		       pe_error(pe->pe_errno));
	}
next:
	if (NEXT_TPE(p) == NULLTPE)
	    return (NOTOK);
    }

    return (RET_OK(rpe, head));

bad:
    return (NOTOK);
}


/*
 * Parse a set, calling appropriate routines to parse each sub type
 */
static int
en_set(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    PE      head;
    PE      pe = NULLPE;
    ptpe    *tmp;
    int    *popt = NULL;	/* Pointer to optional field */
    int     optcnt = 0;		/* Number of optionals bits so far */
    int	    val;

    if (p->pe_type != SET_START && p->pe_type != SSET_START)
	return pepsylose (mod, p, pe, "en_set: missing SET_START");

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode SET %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif

    if ((head = pe_alloc(CLASS(p), PE_FORM_CONS, TAG(p))) == NULLPE)
	return oom(mod, p);

    p++;
    while (p->pe_type != PE_END) {

#ifdef DEBUG
	DLOG (psap_log, LLOG_DEBUG, ("Encode SET member %s.%s",
				     mod -> md_name, pname(p),
				     pr_petype(p->pe_type)));
#endif

	if (ISDTYPE(p) && OPTIONAL(p)) {
	    switch (p->pe_type) {
	    case INTEGER:
	    case REALTYPE:
	    case BOOLEAN:
	    case T_NULL:
		if (!TESTBIT(*popt, optcnt++))
		    goto next;	/* Missing so skip */
		break;

	    case FN_CALL:
		if ((val = hasdata(parm, p, mod, popt, &optcnt)) == NOTOK)
		    goto bad;
		if (val == 0)
		    goto next;
		break;

	    case ETAG:
		if ((val = hasdata(parm, p + 1, mod, popt, &optcnt)) == NOTOK)
		    goto bad;
		if (val == 0)
		    goto next;
		break;

	    case IMP_OBJ:
		if ((p[1].pe_type == SOBJECT && parm == NULL)
		    || *((char **) (parm + p[1].pe_ucode)) == NULL)
		    goto next;
		break;

	    case SOBJECT:
		if (((char *) (parm + p->pe_ucode)) == NULL)
		    goto next;
		break;

	    default:
		if (*((char **) (parm + p->pe_ucode)) == NULL)
		    goto next;
		break;
	    }
	}
	switch (p->pe_type) {
	case OPTL:
	    popt = (int *) (parm + p->pe_ucode);
	    break;

	case UCODE:
	    if (mod->md_eucode == NULLIFP
	    || (*mod->md_eucode) (parm, &pe, p) == NOTOK)
		goto bad;
	    break;

	case ETAG:
	    if (en_type(parm, p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEQ_START:
	    if (en_seq(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEQOF_START:
	    if (en_seqof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SET_START:
	    if (en_set(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SETOF_START:
	    if (en_setof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case IMP_OBJ:
	    tmp = p++;
	    if (p->pe_type == EXTOBJ || p->pe_type == SEXTOBJ) {
		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		      0, (char *)0, *(char **) (parm + p->pe_ucode)) == NOTOK)
		      return (NOTOK);
	    } else if (p->pe_type == SOBJECT) {
		if (en_obj((char *) parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1, mod,
			&pe) == NOTOK)
		    goto bad;
	    } else
		if (en_obj(*(char **) (parm + p->pe_ucode),
			mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		    goto bad;
	    pe->pe_class = CLASS(tmp);
	    pe->pe_id = TAG(tmp);
	    break;

	case SOBJECT:
	    if (en_obj(parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case OBJECT:
	    if (en_obj(*(char **) (parm + p->pe_ucode),
		    mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case CHOICE_START:
	    if (en_choice(*(char **) (parm + p->pe_ucode), p, mod, &pe) ==NOTOK)
		goto bad;
	    break;


	case SCHOICE_START:
	    if (en_choice((char *) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		goto bad;
	    break;

	case SEXTOBJ:
	    if (p[1].pe_type != EXTMOD)
		return pepsylose (mod, p, NULLPE, "en_set: missing EXTMOD");

	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		  parm + p->pe_ucode) == NOTOK)
		  return (NOTOK);
	    break;

	case EXTOBJ:
	    if (p[1].pe_type != EXTMOD) 
		return pepsylose (mod, p, NULLPE, "en_set: missing EXTMOD");

	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		  *(char **) (parm + p->pe_ucode)) == NOTOK)
		  return (NOTOK);
	    break;

	default:
	    if (en_type(parm, p, mod, &pe) == NOTOK)
		goto bad;
	    break;
	}

	if ((ISDTYPE(p) || p->pe_type == BOPTIONAL) && pe != NULLPE) {
	    if (set_add(head, pe) == NOTOK)
		return pepsylose (mod, p, NULLPE, "en_set bad set: %s",
				  pe_error(pe->pe_errno));
	}
next:
	NEXT_TPE(p);
    }

    return (RET_OK(rpe, head));

bad:
    return (NOTOK);
}


/*
 * Parse a sequence of calling appropriate routines to parse each sub
 * type
 */
static int
en_seqof(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    PE      head;
    PE      pe = NULLPE;
    ptpe    *start;		/* first entry in list */
    ptpe    *tmp;

    if (p->pe_type != SEQOF_START && p->pe_type != SSEQOF_START)
	return pepsylose (mod, p, NULLPE, "en_seqof: missing SEQOF_START");

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode SEQUENCE of %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif

    if ((head = pe_alloc(CLASS(p), PE_FORM_CONS, TAG(p))) == NULLPE)
	return oom(mod, p);

    start = p;
    while ((char *) parm != NULL) {
	p++;
	while (p->pe_type != PE_END) {

#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG,
		  ("Encode SEQUENCE OF member %s.%s type %s",
		   mod -> md_name, pname(p),
		   pr_petype(p->pe_type)));
#endif

	    switch (p->pe_type) {
	    case UCODE:
		if (mod->md_eucode == NULLIFP
		|| (*mod->md_eucode) (parm, &pe, p) == NOTOK)
		    goto bad;
		break;

	    case ETAG:
		if (en_type(parm, p, mod, &pe) == NOTOK)
		    goto bad;
		break;

	    case SEQ_START:
		if (en_seq(*(char **) (parm + p->pe_ucode), p, mod, &pe)==NOTOK)
		    goto bad;
		break;

	    case SEQOF_START:
		if (en_seqof(*(char **) (parm + p->pe_ucode), p, mod, &pe)
			== NOTOK)
		    goto bad;
		break;

	    case SET_START:
		if (en_set(*(char **) (parm + p->pe_ucode), p, mod, &pe)==NOTOK)
		    goto bad;
		break;

	    case SETOF_START:
		if (en_setof(*(char **) (parm + p->pe_ucode), p, mod, &pe)
			== NOTOK)
		    goto bad;
		break;

	    case IMP_OBJ:
		tmp = p++;
		if (p->pe_type == EXTOBJ || p->pe_type == SEXTOBJ) {
		    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		       0, (char *)0, *(char **) (parm + p->pe_ucode)) == NOTOK)
		       return (NOTOK);
		} else if (p->pe_type == SOBJECT) {
		    if (en_obj((char *) (parm + p->pe_ucode), mod->md_etab[p->pe_tag] + 1,
			    mod, &pe) == NOTOK)
			goto bad;
		} else if (en_obj(*(char **) (parm + p->pe_ucode),
			    mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
			goto bad;
		pe->pe_class = CLASS(tmp);
		pe->pe_id = TAG(tmp);
		break;

	    case SOBJECT:
		if (en_obj(parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1, mod, &pe) ==NOTOK)
		    goto bad;
		break;

	    case OBJECT:
		if (en_obj(*(char **) (parm + p->pe_ucode),
			mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		    goto bad;
		break;

	    case CHOICE_START:
		if (en_choice(*(char **) (parm + p->pe_ucode), p, mod, &pe)
			== NOTOK)
		    goto bad;
		break;


	    case SCHOICE_START:
		if (en_choice((char *) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		    goto bad;
		break;

	    case SEXTOBJ:
		if (p[1].pe_type != EXTMOD)
		    return pepsylose (mod, p+1, NULLPE,
				      "en_seqof: missing EXTMOD");
		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		      parm + p->pe_ucode) == NOTOK)
		      return (NOTOK);
		break;

	    case EXTOBJ:
		if (p[1].pe_type != EXTMOD) 
		    return pepsylose (mod, p+1, NULLPE,
				      "en_seqof: missing EXTMOD");

		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		      *(char **) (parm + p->pe_ucode)) == NOTOK)
		      return (NOTOK);
		break;

	    default:
		if (en_type(parm, p, mod, &pe) == NOTOK)
		    goto bad;
		break;
	    }
	    if (ISDTYPE(p) && pe != NULLPE) {
		if (seq_add(head, pe, -1) == NOTOK)
		    return pepsylose (mod, p, NULLPE,
				      "en_seqof bad sequence: %s",
				      pe_error(pe->pe_errno));
	    }
	    if (NEXT_TPE(p) == NULLTPE)
		goto bad;
	}
	parm = *(char **) (parm + p->pe_ucode);	/* Any more ? */
	p = start;
    }

    return (RET_OK(rpe, head));

bad:
    return (NOTOK);
}

/*
 * Parse a setof, calling appropriate routines to parse each sub type
 */
static int
en_setof(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    PE      head;
    PE      pe = NULLPE, last = NULLPE;
    ptpe    *start;
    ptpe    *tmp;

    if (p->pe_type != SETOF_START && p->pe_type != SSETOF_START)
	return pepsylose (mod, p, NULLPE, "en_setof: missing SETOF_START");

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode SET OF %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif
    if ((head = pe_alloc(CLASS(p), PE_FORM_CONS, TAG(p))) == NULLPE)
	return oom(mod,p);

    start = p;
    while ((char *) parm != NULL) {
	p++;
	while (p->pe_type != PE_END) {
#ifdef DEBUG
	    DLOG (psap_log, LLOG_DEBUG, ("Encode SET OF memeber %s.%s type %s",
					 mod -> md_name, pname(p),
					 pr_petype(p->pe_type)));
#endif

	    switch (p->pe_type) {
	    case UCODE:
		if (mod->md_eucode == NULLIFP
		|| (*mod->md_eucode) (parm, &pe, p) == NOTOK)
		    goto bad;
		break;

	    case ETAG:
		if (en_type(parm, p, mod, &pe) == NOTOK)
		    goto bad;
		break;

	    case SEQ_START:
		if (en_seq(*(char **) (parm + p->pe_ucode), p, mod, &pe)==NOTOK)
		    goto bad;
		break;

	    case SEQOF_START:
		if (en_seqof(*(char **) (parm + p->pe_ucode), p, mod, &pe)
			== NOTOK)
		    goto bad;
		break;

	    case SET_START:
		if (en_set(*(char **) (parm + p->pe_ucode), p, mod, &pe)==NOTOK)
		    goto bad;
		break;

	    case SETOF_START:
		if (en_setof(*(char **) (parm + p->pe_ucode), p, mod, &pe)
			== NOTOK)
		    goto bad;
		break;

	    case IMP_OBJ:
		tmp = p++;
		if (p->pe_type == EXTOBJ || p->pe_type == SEXTOBJ) {
		    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		       0, (char *)0, *(char **) (parm + p->pe_ucode)) == NOTOK)
		       return (NOTOK);
		} else if (p->pe_type == SOBJECT) {
		    if (en_obj((char *) parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1,
			    mod, &pe) == NOTOK)
			goto bad;
		} else
		    if (en_obj(*(char **) (parm + p->pe_ucode),
			    mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
			goto bad;
		pe->pe_class = CLASS(tmp);
		pe->pe_id = TAG(tmp);
		break;

	    case SOBJECT:
		if (en_obj(parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1, mod, &pe) ==NOTOK)
		    goto bad;
		break;

	    case OBJECT:
		if (en_obj(*(char **) (parm + p->pe_ucode),
			mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		    goto bad;
		break;

	    case CHOICE_START:
		if (en_choice(*(char **) (parm + p->pe_ucode), p, mod, &pe)
			== NOTOK)
		    goto bad;
		break;


	    case SCHOICE_START:
		if (en_choice((char *) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
		    goto bad;
		break;

	    case SEXTOBJ:
		if (p[1].pe_type != EXTMOD)
		    return pepsylose (mod, p + 1, NULLPE,
				      "en_setof: missing EXTMOD");
		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		      parm + p->pe_ucode) == NOTOK)
		      return (NOTOK);
		break;

	    case EXTOBJ:
		if (p[1].pe_type != EXTMOD)
		    return pepsylose (mod, p + 1, NULLPE,
				      "en_setof: missing EXTMOD");

		if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
		      *(char **) (parm + p->pe_ucode)) == NOTOK)
		      return (NOTOK);
		break;

	    default:
		if (en_type(parm, p, mod, &pe) == NOTOK)
		    goto bad;
		break;
	    }

	    if (ISDTYPE(p) && pe != NULLPE) {
		if (set_addon(head, last, pe) == NOTOK)
		    return pepsylose (mod, p, NULLPE, "en_setof bad set: %s",
				      pe_error(pe->pe_errno));
		else
		    last = pe;
	    }

	    if (NEXT_TPE(p) == NULLTPE)
		goto bad;;
	}
	parm = *(char **) (parm + p->pe_ucode);	/* Any more ? */
	p = start;
    }

    return (RET_OK(rpe, head));


bad:
    return (NOTOK);
}

/*
 * encode a choice field. This means find which choice is taken and
 * call en_type to encode it
 */
static int
en_choice(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    int     cnt;
    ptpe *p2;
    
    if (p->pe_type != CHOICE_START && p->pe_type != SCHOICE_START)
	return pepsylose (mod, p, NULLPE, "en_choice:missing CHOICE_START");

    p2 = p++;

    if (p->pe_type != SCTRL)
	return pepsylose (mod, p, NULLPE, "en_choice:missing SCTRL");

    if (IF_USELECT(p)) {
	if (mod->md_eucode == NULLIFP ||
	    (cnt = (*mod -> md_eucode) (parm, &rpe, p)) == NOTOK)
	    return pepsylose (mod, p, NULLPE, "choice selection failed");
    }
    else
	cnt = *(int *) (parm + p->pe_ucode);

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode CHOICE %s.%s member %d type %s",
				 mod -> md_name, pname(p2),
				 cnt,
				 pr_petype(p2->pe_type)));
#endif

    if (cnt != 0)
	cnt--;
    if (cnt < 0)
	return pepsylose (mod, p, NULLPE, "en_choice:offset %d negative", cnt);

    for (p++; p->pe_type != PE_END; NEXT_TPE(p)) {
	if (cnt == 0 && p->pe_type == UCODE) {
		if (mod->md_eucode == NULLIFP
		|| (*mod->md_eucode) (parm, &rpe, p) == NOTOK)
		    return (NOTOK);
	}
	if (ISDTYPE(p)) {
	    if (cnt == 0) {
		if (en_etype(parm, p, mod, rpe) == NOTOK)
		    return (NOTOK);
		return (OK);
	    }
	    cnt--;
	}
    }

    return pepsylose (mod, p, NULLPE, "en_choice: no choice taken");
}


/*
 * check to see if the object is present or not
 */
static int
chkobj(mod, p, head)
modtyp *mod;
ptpe    *p;
PE      head;
{

    for (; p->pe_type != PE_END; NEXT_TPE(p)) {
	if (!ISDTYPE(p))
	    continue;

	if (p->pe_type == OBJECT) {
	    if (chkobj(mod, p, head))
		return (1);
	} else if (CHKTAG(mod, p, head))
	    return (1);

	if (OPTIONAL(p) || DEFAULT(p))
	    continue;

	return (0);
    }
    return (0);
}

/*
 * Encode a single type for an explicit tag field If a basic type
 * encode it, if a compound type call the appropriate encoding
 * routine. Similar to en_type except we do the indirection on the
 * ucode field
 */
static int
en_etype(parm, p, mod, rpe)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
PE	*rpe;		/* Return value PE */
{
    ptpe    *tmp;
    PE      pe = NULLPE;

#ifdef DEBUG
    DLOG (psap_log, LLOG_DEBUG, ("Encode EXPLICIT TAG %s.%s type %s",
				 mod -> md_name, pname(p),
				 pr_petype(p->pe_type)));
#endif

    switch (p->pe_type) {
    case PE_END:
    case PE_START:
	return (pepsylose (mod, p, pe, "en_etype:END/START type"));

    case UCODE:
	if (mod->md_eucode == NULLIFP
	|| (*mod->md_eucode) (parm, &pe, p) == NOTOK)
	    goto bad;
	break;

    case ETAG:
	if ((pe = pe_alloc(CLASS(p), PE_FORM_CONS, TAG(p))) == NULLPE)
	    return oom(mod, p);
	switch (p->pe_ucode) {
	default:
	    p++;
	    if (en_etype(parm, p, mod, &pe->pe_cons) == NOTOK)
		goto bad;
	}
	break;

    case SEQ_START:
	if (en_seq(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SEQOF_START:
	if (en_seqof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SET_START:
	if (en_set(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SETOF_START:
	if (en_setof(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SSEQ_START:
	if (en_seq(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SSEQOF_START:
	if (en_seqof(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SSET_START:
	if (en_set(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SSETOF_START:
	if (en_setof(parm + p->pe_ucode, p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case IMP_OBJ:
	tmp = p++;
	if (p->pe_type == EXTOBJ || p->pe_type == SEXTOBJ) {
	    if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1,
		  0, (char *)0, *(char **) (parm + p->pe_ucode)) == NOTOK)
		  return (NOTOK);
	} else if (p->pe_type == SOBJECT) {
	    if (en_obj((char *) parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1, mod, &pe)
		    == NOTOK)
		goto bad;
	} else
	    if (en_obj(*(char **) (parm + p->pe_ucode),
		    mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
		goto bad;
	pe->pe_class = CLASS(tmp);
	pe->pe_id = TAG(tmp);
	break;

    case SOBJECT:
	if (en_obj(parm + p->pe_ucode, mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case OBJECT:
	if (en_obj(*(char **) (parm + p->pe_ucode),
		mod->md_etab[p->pe_tag] + 1, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case CHOICE_START:
	if (en_choice(*(char **) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
	    goto bad;
	break;

    case SCHOICE_START:
        if (en_choice((char *) (parm + p->pe_ucode), p, mod, &pe) == NOTOK)
          goto bad;
      break;

    case INTEGER:
#ifdef DEBUG
	DLOG (psap_log, LLOG_DEBUG, ("Encode INTEGER %s.%s value %d",
				     mod -> md_name, pname(p),
				     *(integer *) (parm + p->pe_ucode)));
#endif
	if ((pe = num2prim(*(integer *) (parm + p->pe_ucode), CLASS(p), TAG(p))) == NULLPE)
	    return oom (mod, p);

	break;

#ifdef	PEPSY_REALS
    case REALTYPE:
#ifdef DEBUG
	DLOG (psap_log,  LLOG_DEBUG, ("Encode REAL %s.%s value %g",
				      mod -> md_name, pname(p),
				      *(double *) (parm + p->pe_ucode)));
#endif
	    if ((pe = real2prim(*(double *) (parm + p->pe_ucode),
			       CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    break;

#endif
    case BOOLEAN:
#ifdef DEBUG
	DLOG (psap_log, LLOG_DEBUG, ("Encode BOOLEAN %s.%s value %d",
				     mod -> md_name, pname(p),
				     *(char *) (parm + p->pe_ucode)));
#endif

	if ((pe = flag2prim(*(char *) (parm + p->pe_ucode), CLASS(p), TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case T_NULL:
#ifdef DEBUG
	DLOG (psap_log, LLOG_DEBUG, ("Encode NULL %s.%s",
				     mod -> md_name, pname(p)));
#endif

	if ((pe = pe_alloc(CLASS(p), PE_FORM_PRIM, TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case SANY:
	(pe = (PE) (parm + p->pe_ucode))->pe_refcnt++;
	break;

    case ANY:
	if ((parm + p->pe_ucode) == 0 || *(PE *) (parm + p->pe_ucode) == 0)
#if ROSAP_HACK
	    /* hack for ROSAP. expects this strangeness */
	    pe = pe_alloc(PE_CLASS_UNIV, PE_FORM_PRIM, PE_PRIM_NULL);
#else
	    pe = NULLPE;
#endif
	else
	    (pe = *(PE *) (parm + p->pe_ucode))->pe_refcnt++;
	break;

    case SEXTOBJ:
	if (p[1].pe_type != EXTMOD)
	    return pepsylose (mod, p+1, NULLPE, "en_etype: missing EXTMOD");
	if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
	    parm + p->pe_ucode) == NOTOK)
	    return (NOTOK);
	break;

    case EXTOBJ:
	if (p[1].pe_type != EXTMOD)
	    return pepsylose (mod, p+1, NULLPE, "en_etype: missing EXTMOD");
	if (enc_f(p->pe_tag, EXT2MOD(mod, (p + 1)), &pe, 1, 0, (char *)0,
	      *(char **) (parm + p->pe_ucode)) == NOTOK)
	      return (NOTOK);
	break;

    case SOCTETSTRING:
	if ((pe = qb2prim((struct qbuf *) (parm + p->pe_ucode), CLASS(p), TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case SBITSTRING:
	{
	    char   *cp;
	    int     i;

	    if ((cp = bitstr2strb((PE) (parm + p->pe_ucode), &i)) == NULL)
		return oom(mod, p);
	    if ((pe = strb2bitstr(cp, i, CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    free(cp);
	    if ((pe = bit2prim (pe)) == NULLPE)
		return oom(mod, p);
	}
	break;

    case OCTETSTRING:
	if ((pe = qb2prim(*(struct qbuf **) (parm + p->pe_ucode), CLASS(p), TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case T_STRING:
	if (*(char **) (parm + p->pe_ucode) == NULLCP)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype:T_STRING missing pointer");

	if ((pe = str2prim(*(char **) (parm + p->pe_ucode),
			  strlen(*(char **) (parm + p->pe_ucode)),
			  CLASS(p), TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case OCTET_PTR:
	if (p[1].pe_type != OCTET_LEN)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype: missing OCTET_LEN");

	if (*(char **) (parm + p->pe_ucode) == NULLCP)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype:OCTET_PTR missing pointer");

	if (*(int *) (parm + (p + 1)->pe_ucode) < 0)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype:OCTET_LEN negative length");

	if ((pe = str2prim(*(char **) (parm + p->pe_ucode),
			  *(int *) (parm + (p + 1)->pe_ucode),
			  CLASS(p), TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case BITSTR_PTR:
	if (p[1].pe_type != BITSTR_LEN)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype:missing BITSTR_LEN");

	if (*(char **) (parm + p->pe_ucode) == NULLCP)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype:BITSTR_PTR  missing pointer");

	if (*(int *) (parm + (p + 1)->pe_ucode) < 0)
	    return pepsylose (mod, &p[1], NULLPE,
			      "en_etype:BITSTR_LEN negative length");

	if ((pe = strb2bitstr(*(char **) (parm + p->pe_ucode),
			  *(int *) (parm + (p + 1)->pe_ucode),
			  CLASS(p), TAG(p))) == NULLPE
	    || (pe = bit2prim(pe)) == NULLPE)
	    return oom(mod, p);
	break;

    case BITSTRING:
	{
	    char   *cp;
	    int     i;

	    if ((cp = bitstr2strb(*(PE *) (parm + p->pe_ucode), &i)) == NULL)
		return oom(mod, p);
	    if ((pe = strb2bitstr(cp, i, CLASS(p), TAG(p))) == NULLPE)
		return oom(mod, p);
	    free(cp);
	    if ((pe = bit2prim(pe)) == NULLPE)
		return oom(mod, p);
	}
	break;

    case SOBJID:
	if ((pe = obj2prim((OID) (parm + p->pe_ucode), CLASS(p), TAG(p))) == NULLPE)
	    return oom(mod, p);
	break;

    case OBJID:
	if ((pe = obj2prim(*(OID *) (parm + p->pe_ucode), CLASS(p), TAG(p)))
	    == NULLPE)
	    return oom(mod, p);
	break;

    case FN_CALL:
	if ((FN_PTR(mod, p))(parm, &pe) == NOTOK)
	    return pepsylose (mod, p, NULLPE,
			      "en_etype:FN_CALL:call failed");
	if (STAG(p) >= 0) {
	    pe->pe_class = CLASS(p);
	    pe->pe_id = TAG(p);
	}
	break;

    default:
	return pepsylose (mod, p, NULLPE, "en_etype: type not implemented");
    }
    return (RET_OK(rpe, pe));

bad:
    return (NOTOK);
}
