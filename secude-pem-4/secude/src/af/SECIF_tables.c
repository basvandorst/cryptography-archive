#include <stdio.h>
#include "SECIF-types.h"


# line 7 "if.py"

#include "If-types.h"

extern caddr_t _ZptrtabSECIF[];	/* forward decl */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static ptpe et_AttributeTypeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[0] },
	{ SOBJID, 0, 6, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[0] }
	};

static ptpe et_SETOFAttributeTypeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[1] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[1] },
	{ OBJECT, OFFSET(SET_OF_AttrType , element), _ZAttributeTypeSECIF, 0, (char **)&_ZptrtabSECIF[1] },
	{ PE_END, OFFSET(SET_OF_AttrType , next), 0, 0, (char **)&_ZptrtabSECIF[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[1] }
	};

static ptpe et_AttributeValueSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[2] },
	{ SANY, 0, -1, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[2] }
	};

static ptpe et_AttributeValueAssertionSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[3] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[3] },
	{ OBJID, OFFSET(AttrValueAssertion , element_IF_0), 6, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[3] },
	{ ANY, OFFSET(AttrValueAssertion , element_IF_1), -1, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[3] }
	};

static ptpe et_RelativeDistinguishedNameSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[4] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[4] },
	{ OBJECT, OFFSET(RDName , member_IF_0), _ZAttributeValueAssertionSECIF, 0, (char **)&_ZptrtabSECIF[4] },
	{ PE_END, OFFSET(RDName , next), 0, 0, (char **)&_ZptrtabSECIF[4] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[4] }
	};

static ptpe et_NameSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[5] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[5] },
	{ OBJECT, OFFSET(DName , element_IF_2), _ZRelativeDistinguishedNameSECIF, 0, (char **)&_ZptrtabSECIF[5] },
	{ PE_END, OFFSET(DName , next), 0, 0, (char **)&_ZptrtabSECIF[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[5] }
	};

static ptpe et_AttributeValuesSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[6] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[6] },
	{ ANY, OFFSET(AttrValues , member_IF_1), -1, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[6] },
	{ PE_END, OFFSET(AttrValues , next), 0, 0, (char **)&_ZptrtabSECIF[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[6] }
	};

static ptpe et_AttributeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[7] },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[7] },
	{ OBJID, OFFSET(Attr , type), 6, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[7] },
	{ OBJECT, OFFSET(Attr , values), _ZAttributeValuesSECIF, 0, (char **)&_ZptrtabSECIF[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[7] }
	};

static ptpe et_SETOFAttributeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[8] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[8] },
	{ OBJECT, OFFSET(SET_OF_Attr , element), _ZAttributeSECIF, 0, (char **)&_ZptrtabSECIF[8] },
	{ PE_END, OFFSET(SET_OF_Attr , next), 0, 0, (char **)&_ZptrtabSECIF[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[8] }
	};

static ptpe dt_AttributeTypeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[0] },
	{ SOBJID, 0, 6, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[0] }
	};

static ptpe dt_SETOFAttributeTypeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[1] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[1] },
	{ MEMALLOC, 0, sizeof (SET_OF_AttrType ), 0, (char **)&_ZptrtabSECIF[1] },
	{ OBJECT, OFFSET(SET_OF_AttrType , element), _ZAttributeTypeSECIF, 0, (char **)&_ZptrtabSECIF[1] },
	{ PE_END, OFFSET(SET_OF_AttrType , next), 0, 0, (char **)&_ZptrtabSECIF[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[1] }
	};

static ptpe dt_AttributeValueSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[2] },
	{ SANY, 0, -1, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[2] }
	};

static ptpe dt_AttributeValueAssertionSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[3] },
	{ UCODE, 0, 0, 0, (char **)&_ZptrtabSECIF[3] }, /* line 34 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[3] },
	{ MEMALLOC, 0, sizeof (AttrValueAssertion ), 0, (char **)&_ZptrtabSECIF[3] },
	{ OBJID, OFFSET(AttrValueAssertion , element_IF_0), 6, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[3] },
	{ ANY, OFFSET(AttrValueAssertion , element_IF_1), -1, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[3] }
	};

static ptpe dt_RelativeDistinguishedNameSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[4] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[4] },
	{ MEMALLOC, 0, sizeof (RDName ), 0, (char **)&_ZptrtabSECIF[4] },
	{ OBJECT, OFFSET(RDName , member_IF_0), _ZAttributeValueAssertionSECIF, 0, (char **)&_ZptrtabSECIF[4] },
	{ PE_END, OFFSET(RDName , next), 0, 0, (char **)&_ZptrtabSECIF[4] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[4] }
	};

static ptpe dt_NameSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[5] },
	{ SSEQOF_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[5] },
	{ MEMALLOC, 0, sizeof (DName ), 0, (char **)&_ZptrtabSECIF[5] },
	{ OBJECT, OFFSET(DName , element_IF_2), _ZRelativeDistinguishedNameSECIF, 0, (char **)&_ZptrtabSECIF[5] },
	{ PE_END, OFFSET(DName , next), 0, 0, (char **)&_ZptrtabSECIF[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[5] }
	};

static ptpe dt_AttributeValuesSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[6] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[6] },
	{ MEMALLOC, 0, sizeof (AttrValues ), 0, (char **)&_ZptrtabSECIF[6] },
	{ ANY, OFFSET(AttrValues , member_IF_1), -1, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[6] },
	{ PE_END, OFFSET(AttrValues , next), 0, 0, (char **)&_ZptrtabSECIF[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[6] }
	};

static ptpe dt_AttributeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[7] },
	{ UCODE, 1, 0, 0, (char **)&_ZptrtabSECIF[7] }, /* line 68 */
	{ SSEQ_START, 0, 16, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[7] },
	{ MEMALLOC, 0, sizeof (Attr ), 0, (char **)&_ZptrtabSECIF[7] },
	{ OBJID, OFFSET(Attr , type), 6, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[7] },
	{ OBJECT, OFFSET(Attr , values), _ZAttributeValuesSECIF, 0, (char **)&_ZptrtabSECIF[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[7] }
	};

static ptpe dt_SETOFAttributeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[8] },
	{ SSETOF_START, 0, 17, FL_UNIVERSAL, (char **)&_ZptrtabSECIF[8] },
	{ MEMALLOC, 0, sizeof (SET_OF_Attr ), 0, (char **)&_ZptrtabSECIF[8] },
	{ OBJECT, OFFSET(SET_OF_Attr , element), _ZAttributeSECIF, 0, (char **)&_ZptrtabSECIF[8] },
	{ PE_END, OFFSET(SET_OF_Attr , next), 0, 0, (char **)&_ZptrtabSECIF[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[8] }
	};

static ptpe pt_AttributeTypeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[0] },
	{ SOBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[0] }
	};

static ptpe pt_SETOFAttributeTypeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[1] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAttributeTypeSECIF, 0, (char **)&_ZptrtabSECIF[0] },
	{ PE_END, OFFSET(SET_OF_AttrType , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[1] }
	};

static ptpe pt_AttributeValueSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[2] },
	{ SANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[2] }
	};

static ptpe pt_AttributeValueAssertionSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[3] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[3] }
	};

static ptpe pt_RelativeDistinguishedNameSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[4] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAttributeValueAssertionSECIF, 0, (char **)&_ZptrtabSECIF[3] },
	{ PE_END, OFFSET(RDName , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[4] }
	};

static ptpe pt_NameSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[5] },
	{ SSEQOF_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZRelativeDistinguishedNameSECIF, 0, (char **)&_ZptrtabSECIF[4] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[5] }
	};

static ptpe pt_AttributeValuesSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[6] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ PE_END, OFFSET(AttrValues , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[6] }
	};

static ptpe pt_AttributeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[7] },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAttributeValuesSECIF, 0, (char **)&_ZptrtabSECIF[6] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[7] }
	};

static ptpe pt_SETOFAttributeSECIF[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabSECIF[8] },
	{ SSETOF_START, -1, 17, FL_UNIVERSAL, NULLVP },
	{ OBJECT, 0, _ZAttributeSECIF, 0, (char **)&_ZptrtabSECIF[7] },
	{ PE_END, OFFSET(SET_OF_Attr , next), 0, 0 },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabSECIF[8] }
	};


/*VARARGS*/
static	int
dfn_SECIF(__p, pe, _Zp, _val)
caddr_t	__p;
PE	pe;
ptpe	*_Zp;
int _val;
{
		
	/* 2 cases */
    switch(_Zp->pe_ucode) {

#define parm	((AttrValueAssertion ** )__p)
	case 0: /*  */
		{
# line 34 "if.py"

        if (((*parm) = (AttrValueAssertion *) calloc (1, sizeof (AttrValueAssertion))) == ((AttrValueAssertion *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm

#define parm	((Attr ** )__p)
	case 1: /*  */
		{
# line 68 "if.py"

        if (((*parm) = (Attr *) calloc (1, sizeof (Attr))) == ((Attr *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        ;
		}
	    break;

#undef parm
    default:
        return (pepsylose(NULLMODTYP, _Zp, pe, "dnf_SECIF:Bad table entry: %d",
            _Zp->pe_ucode));
		}	/* switch */
    return (OK);
}
static ptpe *etabl[] = {
	et_AttributeTypeSECIF,
	et_SETOFAttributeTypeSECIF,
	et_AttributeValueSECIF,
	et_AttributeValueAssertionSECIF,
	et_RelativeDistinguishedNameSECIF,
	et_NameSECIF,
	et_AttributeValuesSECIF,
	et_AttributeSECIF,
	et_SETOFAttributeSECIF,
	};

static ptpe *dtabl[] = {
	dt_AttributeTypeSECIF,
	dt_SETOFAttributeTypeSECIF,
	dt_AttributeValueSECIF,
	dt_AttributeValueAssertionSECIF,
	dt_RelativeDistinguishedNameSECIF,
	dt_NameSECIF,
	dt_AttributeValuesSECIF,
	dt_AttributeSECIF,
	dt_SETOFAttributeSECIF,
	};

static ptpe *ptabl[] = {
	pt_AttributeTypeSECIF,
	pt_SETOFAttributeTypeSECIF,
	pt_AttributeValueSECIF,
	pt_AttributeValueAssertionSECIF,
	pt_RelativeDistinguishedNameSECIF,
	pt_NameSECIF,
	pt_AttributeValuesSECIF,
	pt_AttributeSECIF,
	pt_SETOFAttributeSECIF,
	};


/* Pointer table 9 entries */
caddr_t _ZptrtabSECIF[] = {
    (caddr_t ) "AttributeType",
    (caddr_t ) "SETOFAttributeType",
    (caddr_t ) "AttributeValue",
    (caddr_t ) "AttributeValueAssertion",
    (caddr_t ) "RelativeDistinguishedName",
    (caddr_t ) "Name",
    (caddr_t ) "AttributeValues",
    (caddr_t ) "Attribute",
    (caddr_t ) "SETOFAttribute",
};

modtyp _ZSECIF_mod = {
	"SECIF",
	9,
	etabl,
	dtabl,
	ptabl,
	0,
	dfn_SECIF,
	0,
	_ZptrtabSECIF,
	};


#if	defined(lint) || defined(PEPSY_LINKABLE_FUNCS)

#undef parse_SECIF_AttributeType
int	parse_SECIF_AttributeType(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AttrType  **parm;
{
  return (dec_f(_ZAttributeTypeSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SECIF_SETOFAttributeType
int	parse_SECIF_SETOFAttributeType(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_AttrType  **parm;
{
  return (dec_f(_ZSETOFAttributeTypeSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_SETOFAttributeType
void free_SECIF_SETOFAttributeType(val)
SET_OF_AttrType  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZSETOFAttributeTypeSECIF], &_ZSECIF_mod, 1);
}

#undef parse_SECIF_AttributeValue
int	parse_SECIF_AttributeValue(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AttrValue  **parm;
{
  return (dec_f(_ZAttributeValueSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef parse_SECIF_AttributeValueAssertion
int	parse_SECIF_AttributeValueAssertion(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AttrValueAssertion  **parm;
{
  return (dec_f(_ZAttributeValueAssertionSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_AttributeValueAssertion
void free_SECIF_AttributeValueAssertion(val)
AttrValueAssertion  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZAttributeValueAssertionSECIF], &_ZSECIF_mod, 1);
}

#undef parse_SECIF_RelativeDistinguishedName
int	parse_SECIF_RelativeDistinguishedName(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
RDName  **parm;
{
  return (dec_f(_ZRelativeDistinguishedNameSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_RelativeDistinguishedName
void free_SECIF_RelativeDistinguishedName(val)
RDName  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZRelativeDistinguishedNameSECIF], &_ZSECIF_mod, 1);
}

#undef parse_SECIF_Name
int	parse_SECIF_Name(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
DName  **parm;
{
  return (dec_f(_ZNameSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_Name
void free_SECIF_Name(val)
DName  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZNameSECIF], &_ZSECIF_mod, 1);
}

#undef parse_SECIF_AttributeValues
int	parse_SECIF_AttributeValues(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
AttrValues  **parm;
{
  return (dec_f(_ZAttributeValuesSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_AttributeValues
void free_SECIF_AttributeValues(val)
AttrValues  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZAttributeValuesSECIF], &_ZSECIF_mod, 1);
}

#undef parse_SECIF_Attribute
int	parse_SECIF_Attribute(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
Attr  **parm;
{
  return (dec_f(_ZAttributeSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_Attribute
void free_SECIF_Attribute(val)
Attr  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZAttributeSECIF], &_ZSECIF_mod, 1);
}

#undef parse_SECIF_SETOFAttribute
int	parse_SECIF_SETOFAttribute(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
SET_OF_Attr  **parm;
{
  return (dec_f(_ZSETOFAttributeSECIF, &_ZSECIF_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_SECIF_SETOFAttribute
void free_SECIF_SETOFAttribute(val)
SET_OF_Attr  *val;
{
(void) fre_obj((char *) val, _ZSECIF_mod.md_dtab[_ZSETOFAttributeSECIF], &_ZSECIF_mod, 1);
}

#endif	/* lint||PEPSY_LINKABLE_FUNCS */
