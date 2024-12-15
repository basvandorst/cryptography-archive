#include <stdio.h>
#include "CADB-types.h"


# line 5 "cadb.py"
      /* surrounding global definitions */
#include "cadb.h"

	static OctetString * tmp_ostr, * save_serialnumber;
	static int i,j;

extern caddr_t _ZptrtabCADB[];	/* forward decl */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static ptpe et_IssuedCertificateCADB[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabCADB[0] },
	{ UCODE, 0, 0, 0, (char **)&_ZptrtabCADB[0] }, /* line 24 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabCADB[0] },
	{ EXTOBJ, OFFSET(IssuedCertificate , serial), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabCADB[1] },
	{ EXTMOD, 2, 0, 0, (char **)&_ZptrtabCADB[1] },
	{ T_STRING, OFFSET(IssuedCertificate , date_of_issue), 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[0] },
	{ UCODE, 1, 0, 0, (char **)&_ZptrtabCADB[0] }, /* line 48 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[0] }
	};

static ptpe et_IssuedCertificateSetCADB[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabCADB[3] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabCADB[3] },
	{ OBJECT, OFFSET(SET_OF_IssuedCertificate , element), _ZIssuedCertificateCADB, 0, (char **)&_ZptrtabCADB[3] },
	{ PE_END, OFFSET(SET_OF_IssuedCertificate , next), 0, 0, (char **)&_ZptrtabCADB[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[3] }
	};

static ptpe dt_IssuedCertificateCADB[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabCADB[0] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabCADB[0] },
	{ MEMALLOC, 0, sizeof (IssuedCertificate ), 0, (char **)&_ZptrtabCADB[0] },
	{ EXTOBJ, OFFSET(IssuedCertificate , serial), _ZOURINTEGERSEC, 0, (char **)&_ZptrtabCADB[1] },
	{ EXTMOD, 2, 0, 0, (char **)&_ZptrtabCADB[1] },
	{ T_STRING, OFFSET(IssuedCertificate , date_of_issue), 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[0] },
	{ UCODE, 0, 0, 0, (char **)&_ZptrtabCADB[0] }, /* line 54 */
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[0] }
	};

static ptpe dt_IssuedCertificateSetCADB[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabCADB[3] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabCADB[3] },
	{ MEMALLOC, 0, sizeof (SET_OF_IssuedCertificate ), 0, (char **)&_ZptrtabCADB[3] },
	{ OBJECT, OFFSET(SET_OF_IssuedCertificate , element), _ZIssuedCertificateCADB, 0, (char **)&_ZptrtabCADB[3] },
	{ PE_END, OFFSET(SET_OF_IssuedCertificate , next), 0, 0, (char **)&_ZptrtabCADB[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[3] }
	};

static ptpe pt_IssuedCertificateCADB[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabCADB[0] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ EXTOBJ, 0, _ZOURINTEGERSEC, 0, (char **)&_ZptrtabCADB[1] },
	{ EXTMOD, 2, 0, 0, NULLVP },
	{ T_STRING, -1, 23, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[0] }
	};

static ptpe pt_IssuedCertificateSetCADB[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabCADB[3] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZIssuedCertificateCADB, 0, (char **)&_ZptrtabCADB[0] },
	{ PE_END, OFFSET(SET_OF_IssuedCertificate , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabCADB[3] }
	};


/*VARARGS*/
static	int
efn_CADB(__p, ppe, _Zp)
caddr_t	__p;
PE	*ppe;
ptpe	*_Zp;
{
		
	/* 2 cases */
    switch(_Zp->pe_ucode) {

#define parm	((IssuedCertificate * )__p)
	case 0: /*  */
		{
# line 24 "cadb.py"

	tmp_ostr = NULLOCTETSTRING;

	if(parm->serial && parm->serial->noctets > 0 && (parm->serial->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = (OctetString * )malloc(sizeof(OctetString));
		tmp_ostr->noctets = parm->serial->noctets + 1;
		tmp_ostr->octets = (char * ) calloc( 1 , tmp_ostr->noctets );
		tmp_ostr->octets[0] = 0x00;
		for ( i = 0, j = 1; i < parm->serial->noctets; i++, j++)
			tmp_ostr->octets[j] = parm->serial->octets[i];
		save_serialnumber = parm->serial;
		parm->serial = tmp_ostr;
	}
	;
		}
	    break;

#undef parm

#define parm	((IssuedCertificate * )__p)
	case 1: /*  */
		{
# line 48 "cadb.py"

	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serial = save_serialnumber;
	}
	;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, *ppe, "enf_CADB:Bad table entry: %d",
             _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}

/*VARARGS*/
static	int
dfn_CADB(__p, pe, _Zp, _val)
caddr_t	__p;
PE	pe;
ptpe	*_Zp;
int _val;
{
		
	/* 1 cases */
    switch(_Zp->pe_ucode) {

#define parm	((IssuedCertificate ** )__p)
	case 0: /*  */
		{
# line 54 "cadb.py"

	if((*parm)->serial->noctets > 1 && !((*parm)->serial->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serial->noctets - 1; i++)
			(*parm)->serial->octets[i] = (*parm)->serial->octets[i + 1];
		(*parm)->serial->noctets -= 1;
		(*parm)->serial->octets = (char *)realloc((*parm)->serial->octets, (*parm)->serial->noctets);
	}
	;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, pe, "dnf_CADB:Bad table entry: %d",
            _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}
static ptpe *etabl[] = {
	et_IssuedCertificateCADB,
	et_IssuedCertificateSetCADB,
	};

static ptpe *dtabl[] = {
	dt_IssuedCertificateCADB,
	dt_IssuedCertificateSetCADB,
	};

static ptpe *ptabl[] = {
	pt_IssuedCertificateCADB,
	pt_IssuedCertificateSetCADB,
	};


/* Pointer table 4 entries */
caddr_t _ZptrtabCADB[] = {
    (caddr_t ) "IssuedCertificate",
    (caddr_t ) "serial",
    (caddr_t ) &_ZSEC_mod,
    (caddr_t ) "IssuedCertificateSet",
};

modtyp _ZCADB_mod = {
	"CADB",
	2,
	etabl,
	dtabl,
	ptabl,
	efn_CADB,
	dfn_CADB,
	0,
	_ZptrtabCADB,
	};


#if	defined(lint) || defined(PEPSY_LINKABLE_FUNCS)

#undef parse_CADB_IssuedCertificate
int	parse_CADB_IssuedCertificate(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
IssuedCertificate  **parm;
{
  return (dec_f(_ZIssuedCertificateCADB, &_ZCADB_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_CADB_IssuedCertificate
void free_CADB_IssuedCertificate(val)
IssuedCertificate  *val;
{
(void) fre_obj((char *) val, _ZCADB_mod.md_dtab[_ZIssuedCertificateCADB], &_ZCADB_mod, 1);
}

#undef parse_CADB_IssuedCertificateSet
int	parse_CADB_IssuedCertificateSet(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_IssuedCertificate  **parm;
{
  return (dec_f(_ZIssuedCertificateSetCADB, &_ZCADB_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_CADB_IssuedCertificateSet
void free_CADB_IssuedCertificateSet(val)
SET_OF_IssuedCertificate  *val;
{
(void) fre_obj((char *) val, _ZCADB_mod.md_dtab[_ZIssuedCertificateSetCADB], &_ZCADB_mod, 1);
}

#endif	/* lint||PEPSY_LINKABLE_FUNCS */
