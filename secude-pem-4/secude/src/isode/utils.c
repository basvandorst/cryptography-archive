/* util.c */

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/pepsy2/RCS/utils.c,v 10.0 1993/02/18 20:15:32 awy Rel $";
#endif

/* 
 * $Header: /home/isode-consortium/isode/src/lib/asn1/pepsy2/RCS/utils.c,v 10.0 1993/02/18 20:15:32 awy Rel $
 *
 *
 * $Log: utils.c,v $
 * Revision 10.0  1993/02/18  20:15:32  awy
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
#include	"pepsy-driver.h"
#include	"asn1/asn1.h"
#include	"asn1/pepsy.h"
#include	 <varargs.h>
#ifdef DEBUG
#include 	"compat/tailor.h"
#endif
#include 	"compat/cmd_srch.h"

#ifndef	PEPYPARM
#define PEPYPARM	char *
#endif
#define pname(t)	((t)->pe_typename ? *(t)->pe_typename : "???")

char *pr_petype ();

extern void exit();

#ifdef lint
/* VARARGS4 */

int	pepsylose (module, p, pe, str)
modtyp	*module;
ptpe	*p;
char	*str;
PE	pe;
{
	return pepsylose (module, p, pe, str);
}

#else
int	pepsylose (va_alist)
va_dcl
{
	va_list	ap;
	modtyp	*module;
	ptpe	*p, *p1;
	char	*cp;
	PE	pe;
	char	buffer[BUFSIZ];

	va_start (ap);

	module = va_arg (ap, modtyp *);
	p = va_arg (ap, ptpe *);
	pe = va_arg (ap, PE);

	_asprintf (buffer, NULLCP, ap);
	(void) sprintf (PY_pepy, "%s: module %s",
			buffer, module ? module -> md_name : "<none>");
	if (p) {
	    for (p1 = p; p1 -> pe_type != PE_END; p1++)
		continue;
	    cp = PY_pepy + strlen (PY_pepy);
	    (void) sprintf (cp, "encoding %s expecting %s,class=%s/id=%d",
			    pname(p1),
			    pr_petype (p -> pe_type),
			    pe_classlist[p -> pe_flags & FL_CLASS],
			    p -> pe_tag);
	}
	if (pe && ((int)pe -> pe_class) < pe_maxclass) {
	    cp = PY_pepy + strlen (PY_pepy);
	    (void) sprintf (cp, " found %s/%d", pe_classlist[pe -> pe_class],
			    pe -> pe_id);
	}

#ifdef DEBUG
	SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP, ("%s", PY_pepy));
	if (pe) {
		SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP, ("current PE is:"));
		pe2text (psap_log, pe, 1, NOTOK);
	}
#endif

	va_end (ap);
	return NOTOK;
}
#endif

#ifdef lint
/* VARARGS4 */

int	ppepsylose (module, p, pe, str)
modtyp	*module;
ptpe	*p;
char	*str;
PE	pe;
{
	return ppepsylose (module, p, pe, str);
}

#else
int	ppepsylose (va_alist)
va_dcl
{
	va_list	ap;
	modtyp	*module;
	ptpe	*p;
	char	*cp;
	PE	pe;
	char	buffer[BUFSIZ];

	va_start (ap);

	module = va_arg (ap, modtyp *);
	p = va_arg (ap, ptpe *);
	pe = va_arg (ap, PE);

	_asprintf (buffer, NULLCP, ap);
	(void) sprintf (PY_pepy, "%s: module %s",
			buffer, module ? module -> md_name : "<none>");
	if (p) {
	    cp = PY_pepy + strlen (PY_pepy);
	    (void) sprintf (cp, " %s/class=%s/id=%d",
			    pr_petype (p -> pe_type),
			    pe_classlist[p -> pe_flags & FL_CLASS],
			    p -> pe_tag);
	    if (p->pe_typename)
		(void) sprintf (cp, "(%s)", *p -> pe_typename);
	}
	if (pe && ((int)pe -> pe_class) < pe_maxclass) {
	    cp = PY_pepy + strlen (PY_pepy);
	    (void) sprintf (cp, " got %s/%d", pe_classlist[pe -> pe_class],
			    pe -> pe_id);
	}

#ifdef DEBUG
	SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP, ("%s", PY_pepy));
#endif

	va_end (ap);
	return NOTOK;
}
#endif

/*
 * Useful little routines
 */
/*
 * print out the message and if the arguement is greater than 0
 * terminate
 */
ferr(n, mesg)
char   *mesg;
{
    (void) printf(mesg);
    if (n > 0)
	exit(n);
}
/*
 * print out the message and number and if the arguement is greater
 * than 0 terminate
 */
ferrd(n, mesg, d)
char   *mesg;
int     d;
{
    (void) printf(mesg, d);
    if (n > 0)
	exit(n);
}

/*
 * 0 = Encoding table, 1 = Decoding table, 2 = Printing table
 */
#define TYP_ENC		0
#define TYP_DEC		1
#define TYP_PRINT	2
#define TYP_LAST	2

dmp_tpe(s, p, mod)
char   *s;
modtyp *mod;			/* Module it is from */
ptpe    *p;
{
    int     typ, i, j;
    ptpe   **par, **prev;
    char   *name;

    (void) printf("%s: (%s)", s, mod->md_name);
    /*
     * Calculate what table it is in - we assume they are in order of
     * increasing address
     */

    par = NULL;
    for (typ = 0; typ <= TYP_LAST; typ++) {
	switch (typ) {
	case TYP_ENC:
	    if (mod->md_etab != NULL && mod->md_etab[0] < p) {
		par = mod->md_etab;
		name = "Encoding:";
	    }
	    break;

	case TYP_DEC:
	    if (mod->md_dtab != NULL && mod->md_dtab[0] < p) {
		par = mod->md_dtab;
		name = "Decoding:";
	    }
	    break;

	case TYP_PRINT:
	    if (mod->md_ptab != NULL && mod->md_ptab[0] < (ptpe *) p) {
		par = (ptpe **) mod->md_ptab;
		name = "Printing:";
	    }
	    break;

	default:
	    (void) pepsylose (mod, p, NULLPE, "dmp_tpe:typ = %d internal error\n",
		typ);
	    return;
	}
    }
    if (par == NULL) {
	(void) printf("can't find entry 0x%x\n", p);
	return;
    }
    prev = par;
    for (i = mod->md_nentries; i > 0; i--) {
	if (*par > p)
	    break;
	par++;
    }
    if (par == prev)
	(void) pepsylose (mod, p, NULLPE,
	    "dmp_tpe:par == prev == 0x%x internal error\n", (int) par);
    par--;
    j = p - *par;

    (void) printf("%s type %d + %d ", name, par - prev, j);
    pr_entry(p);
}
#define NENTRY(x)	((sizeof (x)/sizeof (x[0])))
/*
 * Print out a ptpe entry
 */

static CMD_TABLE pepsy_type_tbl[] = {
	"PE_START",	PE_START,
	"PE_END",	PE_END,
	"XOBJECT",	XOBJECT,
	"UCODE",	UCODE,
	"MEMALLOC",	MEMALLOC,
	"SCTRL",	SCTRL,
	"CH_ACT",	CH_ACT,
	"OPTL",	OPTL,
	"BOPTIONAL",	BOPTIONAL,
	"FFN_CALL",	FFN_CALL,
	"FREE_ONLY",	FREE_ONLY,
	"TYPE_DATA",	TYPE_DATA,
	"ANY",	ANY,
	"INTEGER",	INTEGER,
	"BOOLEAN",	BOOLEAN,
	"OBJECT",	OBJECT,
	"BITSTRING",	BITSTRING,
	"OCTETSTRING",	OCTETSTRING,
	"SET_START",	SET_START,
	"SEQ_START",	SEQ_START,
	"SEQOF_START",	SEQOF_START,
	"SETOF_START",	SETOF_START,
	"CHOICE_START",	CHOICE_START,
	"REALTYPE",	REALTYPE,
	"T_NULL",	T_NULL,
	"T_OID",	T_OID,
	"ETAG",	ETAG,
	"IMP_OBJ",	IMP_OBJ,
	"EXTOBJ",	EXTOBJ,
	"EXTMOD",	EXTMOD,
	"OBJID",	OBJID,
	"DFLT_F",	DFLT_F,
	"DFLT_B",	DFLT_B,
	"T_STRING",	T_STRING,
	"OCTET_PTR",	OCTET_PTR,
	"OCTET_LEN",	OCTET_LEN,
	"BITSTR_PTR",	BITSTR_PTR,
	"BITSTR_LEN",	BITSTR_LEN,
	"FN_CALL",	FN_CALL,
	"STYPE_DATA",	STYPE_DATA,
	"STYPE_DATA",	STYPE_DATA,
	"SBITSTRING",	SBITSTRING,
	"SOBJID",	SOBJID,
	"SREALTYPE",	SREALTYPE,
	"SANY",	SANY,
	"SEXTOBJ",	SEXTOBJ,
	"SOBJECT",	SOBJECT,
	"SOCTETSTRING",	SOCTETSTRING,
	"SEXTERNAL",	SEXTERNAL,
	"SSEQ_START",	SSEQ_START,
	"SSET_START",	SSET_START,
	"SSEQOF_START",	SSEQOF_START,
	"SSETOF_START",	SSETOF_START,
	"SCHOICE_START",	SCHOICE_START,
	NULLCP,		-10,
};

char *
pr_petype (type)
int	type;
{
    static char nbuf[30];
    char *p;

    if ((p = rcmd_srch (type, pepsy_type_tbl)) == NULLCP)
	    (void) sprintf (p = nbuf, "%d", type);
    return p;
}

pr_entry(p)
ptpe    *p;
{
    (void) printf ("%s, ", pr_petype (p -> pe_type));
    (void) printf("%d, %d, %d}\n", p->pe_ucode, p->pe_tag, p->pe_flags);
}


/*
 * null function for what evr purposes
 */
f_null()
{
}

/*
 * compare a given number of bits pointed to by the two character
 * pointers return 0 if they are the same non zero otherwise
 */
bitscmp(p1, p2, len)
register char *p1, *p2;
int     len;
{
    register int i;
    register unsigned int mask;

    if (len >= 8 && bcmp(p1, p2, len / 8))
	return (1);

    if (len % 8 == 0)
	return (0);
    /* Check those last few bits */
    i = len / 8;
    mask = (0xff00 >> len % 8) & 0xff;
    if ((p1[i] & mask) != (p2[i] & mask))
	return (1);

    return (0);
}

#define MIN(a, b)	(a < b ? a : b)
/*
 * compare an octet string and a qb and return 0 if they are the same
 * and non zero otherwise
 */
ostrcmp(p, len, qb)
register char *p;
register int len;
register struct qbuf *qb;
{
    register struct qbuf *qp;

    if (len < 0 || qb == NULL || p == NULL)
	return (1);
    qp = qb;
    do {
	if (qp->qb_data != NULL) {
	    if (qp->qb_len < 0)
		ferrd(1, "ostrcmp:qb_len %d < 0", qp->qb_len);
	    if (qp->qb_len > len)
		return (1);
	    if (bcmp(qp->qb_data, p, qp->qb_len))
		return (1);
	    if ((len -= qp->qb_len) == 0)
		return (0);
	    p += qp->qb_len;
	}
	qp = qp->qb_forw;
    } while (qp != qb);

    return (len);
}

/*
 * Is data present for the optional item? 1 for yes 0 for no
 */
hasdata(parm, p, mod, popt, optcnt)
PEPYPARM parm;
ptpe    *p;
modtyp *mod;			/* Module it is from */
int    *popt, *optcnt;
{
    int	val;

    switch (p->pe_type) {
    case INTEGER:
    case REALTYPE:
    case BOOLEAN:
    case T_NULL:
	if (DEFAULT(p)) {
	    /* Default's don't have bit map */
	    if ((p[1].pe_type == DFLT_B && same(p, p + 1, parm, mod))
	    || (p[-1].pe_type == DFLT_F && same(p, p - 1, parm, mod)))
		goto next;
	    break;
	}
	if (!TESTBIT(*popt, (*optcnt)++))
	    goto next;		/* Missing so skip */
	break;

    case ETAG:
	if (!hasdata(parm, p + 1, mod, popt, optcnt))
	    goto next;
	break;

    case IMP_OBJ:
	if ((p[1].pe_type == SOBJECT && parm == NULL)
	    || *((char **) (parm + p[1].pe_ucode)) == NULL)
	    goto next;
	break;

    case FN_CALL:	/* function call */
	if ((val = (FN_PTR(mod, p))(parm, (PE *)0)) == NOTOK)
	    return pepsylose (mod, p, NULLPE,
			      "hasdata:FN_CALL:call failed");
	if (val != OK)
	    goto next;
	break;

    default:
	if (*((char **) (parm + p->pe_ucode)) == NULL)
	    goto next;
	break;
    }
    return (1);

next:
    return (0);
}

/*
 * determine if the default value is the same as the value in the
 * structure and if so return greater than zero (meaning don't encode this
 * item). On error return NOTOK
 */
same(typ, dflt, parm, mod)
ptpe    *typ, *dflt;
char   *parm;
modtyp *mod;			/* Module it is from */
{
    int     val;
    int	len;
    char   *p1;
    PE      pe;
    struct qbuf *qb;

    switch (typ->pe_type) {
    case INTEGER:
	val = IVAL(mod, dflt) == *(integer *) (parm + typ->pe_ucode);
	break;

#ifdef	PEPSY_REALS
    case REALTYPE:
	val = RVAL(mod, dflt) == *(double *) (parm + typ->pe_ucode);
	break;
#endif

    case BOOLEAN:
	val = IVAL(mod, dflt) == *(char *) (parm + typ->pe_ucode);
	break;

    case T_NULL:
	val = 1;		/* Only one value */
	break;

    case SBITSTRING:
	if ((pe = (PE) parm) == NULL) {
	    val = 1;
	    break;
	}
	goto bstring;

    case BITSTRING:
	if ((pe = *(PE *) (parm + typ->pe_ucode)) == NULL) {
	    val = 1;
	    break;
	}
bstring:
	if ((p1 = bitstr2strb(pe, &val)) == NULL) {
	    (void) pepsylose (mod, typ, pe, "same:bad bitstring\n");
	    return (NOTOK);
	    /* Should really abort encoding here but how can we comunicate this
	     * to the routine that calls us?
	     */
	}
bstring2:
	if (val != IVAL(mod, dflt) || bitscmp(PVAL(mod, dflt), p1, val))
	    val = 0;
	else
	    val = 1;
	if (typ->pe_type != BITSTR_PTR)
	    free(p1);
	break;

    case BITSTR_PTR:
	if (typ[1].pe_type != BITSTR_LEN)
	    return pepsylose (mod, typ, pe, "same:missing BITSTR_LEN\n");
	if ((p1 = *(char **) (parm + typ->pe_ucode)) == NULL) {
	    val = 1;
	    break;
	}
	val = *(int *) (parm + (typ + 1)->pe_ucode);
	goto bstring2;

    case SOCTETSTRING:
	if ((qb = (struct qbuf *) parm) == NULL) {
	    val = 1;
	    break;
	}
	goto ostring;

    case OCTETSTRING:
	if ((qb = *(struct qbuf **) (parm + typ->pe_ucode)) == NULL) {
	    val = 1;
	    break;
	}
ostring:
	if (ostrcmp(PVAL(mod, dflt), (int ) IVAL(mod, dflt), qb))
	    val = 0;
	else
	    val = 1;
	break;

    case OCTET_PTR:
	if (typ[1].pe_type != OCTET_LEN)
	    return pepsylose (mod, typ, pe, "same:missing OCTET_LEN\n");
	if ((p1 = *(char **) (parm + typ->pe_ucode)) == NULL) {
	    val = 1;
	    break;
	}
	len = *(int *) (parm + (typ + 1)->pe_ucode);
	goto o1string;

    case T_STRING:
	if ((p1 = *(char **) (parm + typ->pe_ucode)) == NULL) {
	    val = 1;
	    break;
	}
	len = strlen(p1);
o1string:
	if (len != IVAL(mod, dflt)) {
	    val = 0;
	    break;
	}
	if (bcmp(PVAL(mod, dflt), p1, len))
	    val = 0;
	else
	    val = 1;
	break;

    case OBJECT:
	if (*(char **) (parm + typ->pe_ucode) == NULL) {
	    val = 1;		/* to conform with pepy's way of
				 * doing default */
	    break;
	}
	val = same(mod->md_etab[typ->pe_tag] + 1, dflt,
		   *(char **) (parm + typ->pe_ucode), mod);
	break;

    case SOBJECT:
	if ((char *) parm == NULL) {
	    val = 1;		/* to conform with pepy's way of
				 * doing default */
	    break;
	}
	val = same(mod->md_etab[typ->pe_tag] + 1, dflt, parm, mod);
	break;

    case IMP_OBJ:
	typ++;			/* fall through */

    case ANY:
    case SANY:
    case SEXTOBJ:
    case EXTOBJ:
    case OBJID:
    case SOBJID:
    case SEQ_START:
    case SET_START:
    case -1:			/* Just use the pepy method of null
				 * pointers */
	/*
	 * This is the posy/pepy hack way of doing things at the
	 * moment
	 */
	val = *(char **) (parm + typ->pe_ucode) == NULL;
	break;

    case FN_CALL:	/* function call */
	if ((val = (FN_PTR(mod, typ))(parm, (PE *)0)) == NOTOK)
	    return pepsylose (mod, typ, NULLPE,
			      "same:FN_CALL:call failed");
	if (val != OK)
	    val = 1;
	else
	    val = 0;
	break;


    default:
	(void) pepsylose (mod, typ, NULLPE, "same: %d not implemented\n",
	    typ->pe_type);
	return (NOTOK);
    }

    return (val);
}

/*
 * Calculate the next tpe entry in the sequence. Count a sequence as
 * one element
 */
ptpe    *
next_tpe(p)
ptpe    *p;
{
    int     level;



    level = 0;
    if (p->pe_type == PE_END) {
	(void) pepsylose (NULLMODTYP, p, NULLPE,
	    "next_tpe:internal error: unexpected PE_END found");
	return (p);
    }
    do {
again:
	switch (p->pe_type) {
	case SSEQ_START:
	case SSEQOF_START:
	case SSET_START:
	case SSETOF_START:
	case SCHOICE_START:
	case SEQ_START:
	case SEQOF_START:
	case SET_START:
	case SETOF_START:
	case CHOICE_START:
	    level++;
	    break;

	case UCODE:
	case MEMALLOC:
	case SCTRL:
	case CH_ACT:
	case INTEGER:
	case REALTYPE:
	case BOOLEAN:
	case SANY:
	case ANY:
	case T_NULL:
	case OBJECT:
	case SOBJECT:
	case BITSTRING:
	case BITSTR_LEN:
	case SBITSTRING:
	case OCTETSTRING:
	case T_STRING:
	case OCTET_LEN:
	case SOCTETSTRING:
	case OBJID:
	case SOBJID:
	case OPTL:
	case EXTMOD:
	case DFLT_B:
	case FN_CALL:
	case FFN_CALL:
	    break;

	case IMP_OBJ:
	case ETAG:
	case EXTOBJ:
	case SEXTOBJ:
	case DFLT_F:
	case OCTET_PTR:
	case BITSTR_PTR:
	case BOPTIONAL:
	case FREE_ONLY:
	    p++;
	    goto again;

	case PE_END:
	    level--;
	    break;

	default:
	    ferrd(1, "next_tpe: unknown type %d\n", p->pe_type);
	}
	p++;
    } while (level > 0 || p->pe_type == DFLT_B);

    return (p);
}

/*
 * Is there a match at for this tag and class pair. Return 1 if yes 0
 * if no We will search through contained objects and through choices
 */
ismatch(p, mod, cl, tag)
ptpe    *p;
modtyp *mod;			/* Module it is from */
unsigned int cl, tag;
{
    while (!ISDTYPE(p) && p->pe_type != PE_END)
	p++;
    if (p->pe_type == PE_END)
	return(1);

    switch (p->pe_type) {
    case SOBJECT:
    case OBJECT:
	/* Needs to be changed for optional and default */
	return (ismatch(p = mod->md_dtab[p->pe_tag] + 1, mod, cl, tag));

    case SEXTOBJ:
    case EXTOBJ:
	if (p[1].pe_type != EXTMOD) {
	    dmp_tpe("ismatch: missing EXTMOD", p, mod);
	    ferr(1, "ismatch:internal error\n");
	}
	return (ismatch(EXT2MOD(mod, (p + 1))->md_dtab[p->pe_tag] + 1,
			EXT2MOD(mod, (p + 1)), cl, tag));

    case CHOICE_START:
    case SCHOICE_START:
	for (p++; p->pe_type != PE_END; p = next_tpe (p)) {
	    if (!ISDTYPE(p))
		continue;
	    if (ismatch(p, mod, cl, tag))
		return (1);
	}
	return (0);

    case SANY:
	return (1);

    case FN_CALL:
    case ANY:
	if (STAG(p) == -1)
	    return (1);
	/* else fall through - not sure if this is needed */

    default:
	return (tag == TAG(p) && cl == CLASS(p));
    }
}

/*
 * find the data entry that goes with this DFLT_F entry
 * bascially skip over any ETAGS that (an arbitary number but almost always 1)
 */
ptpe	*
fdflt_f(p)
register ptpe	*p;
{
    if (p->pe_type != DFLT_F)
	ferr(1, "fdlt_f:Internal Error missing DFLT_F\n");
    
    for (p++; p->pe_type != PE_END; p++) {
	if (p->pe_type != ETAG)
		return (p);
    }
    ferr(1, "fdlt_f:Internal Error PE_END found\n");
    /*NOTREACHED*/
}

/*
 * find the DFLT_B entry
 */
ptpe	*
fdflt_b(p)
register ptpe	*p;
{
    for (p++; p->pe_type != PE_END; p++) {
	if (p->pe_type == DFLT_B)
		return (p);
    }
    ferr(1, "fdflt_b:Internal Error PE_END found\n");
    /*NOTREACHED*/
}
