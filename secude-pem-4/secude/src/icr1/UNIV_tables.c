#include <stdio.h>
#include "UNIV-types.h"


# line 25 "UNIV.py"

#ifndef	lint
static char *rcsid = "$Header: /home/isode-consortium/isode/src/lib/asn1/pepsy2/RCS/UNIV.py,v 10.0 1993/02/18 20:15:30 awy Rel $";
#endif

extern caddr_t _ZptrtabUNIV[];	/* forward decl */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static ptpe et_IA5StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[0] },
	{ SOCTETSTRING, 0, 22, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[0] }
	};

static ptpe et_NumericStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[1] },
	{ SOCTETSTRING, 0, 18, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[1] }
	};

static ptpe et_PrintableStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[2] },
	{ SOCTETSTRING, 0, 19, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[2] }
	};

static ptpe et_T61StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[3] },
	{ SOCTETSTRING, 0, 20, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[3] }
	};

static ptpe et_TeletexStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[4] },
	{ SOCTETSTRING, 0, 20, 0, (char **)&_ZptrtabUNIV[4] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[4] }
	};

static ptpe et_VideotexStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[5] },
	{ SOCTETSTRING, 0, 21, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[5] }
	};

static ptpe et_GeneralizedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[6] },
	{ SOCTETSTRING, 0, 24, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[6] }
	};

static ptpe et_GeneralisedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[7] },
	{ SOCTETSTRING, 0, 24, 0, (char **)&_ZptrtabUNIV[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[7] }
	};

static ptpe et_UTCTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[8] },
	{ SOCTETSTRING, 0, 23, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[8] }
	};

static ptpe et_UniversalTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[9] },
	{ SOCTETSTRING, 0, 23, 0, (char **)&_ZptrtabUNIV[9] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[9] }
	};

static ptpe et_GraphicStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[10] },
	{ SOCTETSTRING, 0, 25, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[10] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[10] }
	};

static ptpe et_VisibleStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[11] },
	{ SOCTETSTRING, 0, 26, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[11] }
	};

static ptpe et_ISO646StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[12] },
	{ SOCTETSTRING, 0, 26, 0, (char **)&_ZptrtabUNIV[12] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[12] }
	};

static ptpe et_GeneralStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[13] },
	{ SOCTETSTRING, 0, 27, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[13] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[13] }
	};

static ptpe et_CharacterStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[14] },
	{ SOCTETSTRING, 0, 28, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[14] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[14] }
	};

static ptpe et_EXTERNALUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[15] },
	{ SSEQ_START, 0, 8, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[15] },
	{ OBJID, OFFSET(struct type_UNIV_EXTERNAL, direct__reference), 6, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabUNIV[16] },
	{ DFLT_F,	0,	0,	0, (char **)&_ZptrtabUNIV[17] },
	{ INTEGER, OFFSET(struct type_UNIV_EXTERNAL, indirect__reference), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabUNIV[17] },
	{ OBJECT, OFFSET(struct type_UNIV_EXTERNAL, data__value__descriptor), _ZObjectDescriptorUNIV, FL_OPTIONAL, (char **)&_ZptrtabUNIV[18] },
	{ CHOICE_START, OFFSET(struct type_UNIV_EXTERNAL, encoding), 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ SCTRL, OFFSET(struct choice_UNIV_0, offset), 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabUNIV[20] },
	{ ANY, OFFSET(struct choice_UNIV_0, un.single__ASN1__type), -1, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[20] },
	{ OCTETSTRING, OFFSET(struct choice_UNIV_0, un.octet__aligned), 1, FL_CONTEXT, (char **)&_ZptrtabUNIV[21] },
	{ BITSTRING, OFFSET(struct choice_UNIV_0, un.arbitrary), 2, FL_CONTEXT, (char **)&_ZptrtabUNIV[22] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[15] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[15] }
	};

static ptpe et_ObjectDescriptorUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[23] },
	{ SOCTETSTRING, 0, 7, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[23] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[23] }
	};

static ptpe dt_IA5StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[0] },
	{ SOCTETSTRING, 0, 22, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[0] }
	};

static ptpe dt_NumericStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[1] },
	{ SOCTETSTRING, 0, 18, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[1] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[1] }
	};

static ptpe dt_PrintableStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[2] },
	{ SOCTETSTRING, 0, 19, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[2] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[2] }
	};

static ptpe dt_T61StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[3] },
	{ SOCTETSTRING, 0, 20, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[3] }
	};

static ptpe dt_TeletexStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[4] },
	{ SOCTETSTRING, 0, 20, 0, (char **)&_ZptrtabUNIV[4] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[4] }
	};

static ptpe dt_VideotexStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[5] },
	{ SOCTETSTRING, 0, 21, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[5] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[5] }
	};

static ptpe dt_GeneralizedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[6] },
	{ SOCTETSTRING, 0, 24, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[6] }
	};

static ptpe dt_GeneralisedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[7] },
	{ SOCTETSTRING, 0, 24, 0, (char **)&_ZptrtabUNIV[7] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[7] }
	};

static ptpe dt_UTCTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[8] },
	{ SOCTETSTRING, 0, 23, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[8] }
	};

static ptpe dt_UniversalTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[9] },
	{ SOCTETSTRING, 0, 23, 0, (char **)&_ZptrtabUNIV[9] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[9] }
	};

static ptpe dt_GraphicStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[10] },
	{ SOCTETSTRING, 0, 25, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[10] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[10] }
	};

static ptpe dt_VisibleStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[11] },
	{ SOCTETSTRING, 0, 26, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[11] }
	};

static ptpe dt_ISO646StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[12] },
	{ SOCTETSTRING, 0, 26, 0, (char **)&_ZptrtabUNIV[12] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[12] }
	};

static ptpe dt_GeneralStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[13] },
	{ SOCTETSTRING, 0, 27, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[13] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[13] }
	};

static ptpe dt_CharacterStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[14] },
	{ SOCTETSTRING, 0, 28, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[14] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[14] }
	};

static ptpe dt_EXTERNALUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[15] },
	{ SSEQ_START, 0, 8, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[15] },
	{ MEMALLOC, 0, sizeof (struct type_UNIV_EXTERNAL), 0, (char **)&_ZptrtabUNIV[15] },
	{ OBJID, OFFSET(struct type_UNIV_EXTERNAL, direct__reference), 6, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabUNIV[16] },
	{ INTEGER, OFFSET(struct type_UNIV_EXTERNAL, indirect__reference), 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabUNIV[17] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabUNIV[17] },
	{ OBJECT, OFFSET(struct type_UNIV_EXTERNAL, data__value__descriptor), _ZObjectDescriptorUNIV, FL_OPTIONAL, (char **)&_ZptrtabUNIV[18] },
	{ CHOICE_START, OFFSET(struct type_UNIV_EXTERNAL, encoding), 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ MEMALLOC, 0, sizeof (struct choice_UNIV_0), 0, (char **)&_ZptrtabUNIV[19] },
	{ SCTRL, OFFSET(struct choice_UNIV_0, offset), 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ ETAG, 0, 0, FL_CONTEXT, (char **)&_ZptrtabUNIV[20] },
	{ ANY, OFFSET(struct choice_UNIV_0, un.single__ASN1__type), -1, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[20] },
	{ OCTETSTRING, OFFSET(struct choice_UNIV_0, un.octet__aligned), 1, FL_CONTEXT, (char **)&_ZptrtabUNIV[21] },
	{ BITSTRING, OFFSET(struct choice_UNIV_0, un.arbitrary), 2, FL_CONTEXT, (char **)&_ZptrtabUNIV[22] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[15] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[15] }
	};

static ptpe dt_ObjectDescriptorUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[23] },
	{ SOCTETSTRING, 0, 7, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[23] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[23] }
	};

static ptpe pt_IA5StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[0] },
	{ SOCTETSTRING, -1, 22, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[0] }
	};

static ptpe pt_NumericStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[1] },
	{ SOCTETSTRING, 0, 18, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[1] }
	};

static ptpe pt_PrintableStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[2] },
	{ SOCTETSTRING, 0, 19, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[0] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[2] }
	};

static ptpe pt_T61StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[3] },
	{ SOCTETSTRING, -1, 20, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[3] }
	};

static ptpe pt_TeletexStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[4] },
	{ SOCTETSTRING, 0, 20, 0, (char **)&_ZptrtabUNIV[3] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[4] }
	};

static ptpe pt_VideotexStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[5] },
	{ SOCTETSTRING, -1, 21, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[5] }
	};

static ptpe pt_GeneralizedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[6] },
	{ SOCTETSTRING, 0, 24, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[6] }
	};

static ptpe pt_GeneralisedTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[7] },
	{ SOCTETSTRING, 0, 24, 0, (char **)&_ZptrtabUNIV[6] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[7] }
	};

static ptpe pt_UTCTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[8] },
	{ SOCTETSTRING, 0, 23, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[8] }
	};

static ptpe pt_UniversalTimeUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[9] },
	{ SOCTETSTRING, 0, 23, 0, (char **)&_ZptrtabUNIV[8] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[9] }
	};

static ptpe pt_GraphicStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[10] },
	{ SOCTETSTRING, -1, 25, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[10] }
	};

static ptpe pt_VisibleStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[11] },
	{ SOCTETSTRING, -1, 26, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[11] }
	};

static ptpe pt_ISO646StringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[12] },
	{ SOCTETSTRING, 0, 26, 0, (char **)&_ZptrtabUNIV[11] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[12] }
	};

static ptpe pt_GeneralStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[13] },
	{ SOCTETSTRING, -1, 27, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[13] }
	};

static ptpe pt_CharacterStringUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[14] },
	{ SOCTETSTRING, -1, 28, FL_UNIVERSAL, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[14] }
	};

static ptpe pt_EXTERNALUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[15] },
	{ SSEQ_START, -1, 8, FL_UNIVERSAL, NULLVP },
	{ OBJID, -1, 6, FL_UNIVERSAL|FL_OPTIONAL, (char **)&_ZptrtabUNIV[16] },
	{ INTEGER, -1, 2, FL_UNIVERSAL|FL_DEFAULT, (char **)&_ZptrtabUNIV[17] },
	{ DFLT_B,	0,	0,	0, (char **)&_ZptrtabUNIV[17] },
	{ OBJECT, 0, _ZObjectDescriptorUNIV, FL_OPTIONAL, (char **)&_ZptrtabUNIV[18] },
	{ CHOICE_START, -1, 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ SCTRL, 0, 0, 0, (char **)&_ZptrtabUNIV[19] },
	{ ETAG, -1, 0, FL_CONTEXT, (char **)&_ZptrtabUNIV[20] },
	{ ANY, -1, -1, FL_UNIVERSAL, NULLVP },
	{ OCTETSTRING, -1, 1, FL_CONTEXT, (char **)&_ZptrtabUNIV[21] },
	{ BITSTRING, -1, 2, FL_CONTEXT, (char **)&_ZptrtabUNIV[22] },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, NULLVP },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[15] }
	};

static ptpe pt_ObjectDescriptorUNIV[] = {
	{ PE_START, 0, 0, 0, (char **)&_ZptrtabUNIV[23] },
	{ SOCTETSTRING, 0, 7, FL_UNIVERSAL, (char **)&_ZptrtabUNIV[10] },
	{ PE_END, 0, 0, 0, (char **)&_ZptrtabUNIV[23] }
	};

static ptpe *etabl[] = {
	et_IA5StringUNIV,
	et_NumericStringUNIV,
	et_PrintableStringUNIV,
	et_T61StringUNIV,
	et_TeletexStringUNIV,
	et_VideotexStringUNIV,
	et_GeneralizedTimeUNIV,
	et_GeneralisedTimeUNIV,
	et_UTCTimeUNIV,
	et_UniversalTimeUNIV,
	et_GraphicStringUNIV,
	et_VisibleStringUNIV,
	et_ISO646StringUNIV,
	et_GeneralStringUNIV,
	et_CharacterStringUNIV,
	et_EXTERNALUNIV,
	et_ObjectDescriptorUNIV,
	};

static ptpe *dtabl[] = {
	dt_IA5StringUNIV,
	dt_NumericStringUNIV,
	dt_PrintableStringUNIV,
	dt_T61StringUNIV,
	dt_TeletexStringUNIV,
	dt_VideotexStringUNIV,
	dt_GeneralizedTimeUNIV,
	dt_GeneralisedTimeUNIV,
	dt_UTCTimeUNIV,
	dt_UniversalTimeUNIV,
	dt_GraphicStringUNIV,
	dt_VisibleStringUNIV,
	dt_ISO646StringUNIV,
	dt_GeneralStringUNIV,
	dt_CharacterStringUNIV,
	dt_EXTERNALUNIV,
	dt_ObjectDescriptorUNIV,
	};

static ptpe *ptabl[] = {
	pt_IA5StringUNIV,
	pt_NumericStringUNIV,
	pt_PrintableStringUNIV,
	pt_T61StringUNIV,
	pt_TeletexStringUNIV,
	pt_VideotexStringUNIV,
	pt_GeneralizedTimeUNIV,
	pt_GeneralisedTimeUNIV,
	pt_UTCTimeUNIV,
	pt_UniversalTimeUNIV,
	pt_GraphicStringUNIV,
	pt_VisibleStringUNIV,
	pt_ISO646StringUNIV,
	pt_GeneralStringUNIV,
	pt_CharacterStringUNIV,
	pt_EXTERNALUNIV,
	pt_ObjectDescriptorUNIV,
	};


/* Pointer table 24 entries */
caddr_t _ZptrtabUNIV[] = {
    (caddr_t ) "IA5String",
    (caddr_t ) "NumericString",
    (caddr_t ) "PrintableString",
    (caddr_t ) "T61String",
    (caddr_t ) "TeletexString",
    (caddr_t ) "VideotexString",
    (caddr_t ) "GeneralizedTime",
    (caddr_t ) "GeneralisedTime",
    (caddr_t ) "UTCTime",
    (caddr_t ) "UniversalTime",
    (caddr_t ) "GraphicString",
    (caddr_t ) "VisibleString",
    (caddr_t ) "ISO646String",
    (caddr_t ) "GeneralString",
    (caddr_t ) "CharacterString",
    (caddr_t ) "EXTERNAL",
    (caddr_t ) "direct-reference",
    (caddr_t ) "indirect-reference",
    (caddr_t ) "data-value-descriptor",
    (caddr_t ) "encoding",
    (caddr_t ) "single-ASN1-type",
    (caddr_t ) "octet-aligned",
    (caddr_t ) "arbitrary",
    (caddr_t ) "ObjectDescriptor",
};

modtyp _ZUNIV_mod = {
	"UNIV",
	17,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabUNIV,
	};


# line 137 "UNIV.py"


#ifndef PEPSY_VERSION

PEPYPARM NullParm = (PEPYPARM) 0;

#endif


#if	defined(lint) || defined(PEPSY_LINKABLE_FUNCS)

#undef decode_UNIV_IA5String
int	decode_UNIV_IA5String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_IA5String **parm;
{
  return (dec_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_NumericString
int	decode_UNIV_NumericString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_NumericString **parm;
{
  return (dec_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_NumericString
void free_UNIV_NumericString(val)
struct type_UNIV_NumericString *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZNumericStringUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_PrintableString
int	decode_UNIV_PrintableString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_PrintableString **parm;
{
  return (dec_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_PrintableString
void free_UNIV_PrintableString(val)
struct type_UNIV_PrintableString *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZPrintableStringUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_T61String
int	decode_UNIV_T61String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_T61String **parm;
{
  return (dec_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_TeletexString
int	decode_UNIV_TeletexString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_TeletexString **parm;
{
  return (dec_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_TeletexString
void free_UNIV_TeletexString(val)
struct type_UNIV_TeletexString *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZTeletexStringUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_VideotexString
int	decode_UNIV_VideotexString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_VideotexString **parm;
{
  return (dec_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_GeneralizedTime
int	decode_UNIV_GeneralizedTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralizedTime **parm;
{
  return (dec_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_GeneralizedTime
void free_UNIV_GeneralizedTime(val)
struct type_UNIV_GeneralizedTime *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZGeneralizedTimeUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_GeneralisedTime
int	decode_UNIV_GeneralisedTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralisedTime **parm;
{
  return (dec_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_GeneralisedTime
void free_UNIV_GeneralisedTime(val)
struct type_UNIV_GeneralisedTime *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZGeneralisedTimeUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_UTCTime
int	decode_UNIV_UTCTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_UTCTime **parm;
{
  return (dec_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_UTCTime
void free_UNIV_UTCTime(val)
struct type_UNIV_UTCTime *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZUTCTimeUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_UniversalTime
int	decode_UNIV_UniversalTime(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_UniversalTime **parm;
{
  return (dec_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_UniversalTime
void free_UNIV_UniversalTime(val)
struct type_UNIV_UniversalTime *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZUniversalTimeUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_GraphicString
int	decode_UNIV_GraphicString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GraphicString **parm;
{
  return (dec_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_VisibleString
int	decode_UNIV_VisibleString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_VisibleString **parm;
{
  return (dec_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_ISO646String
int	decode_UNIV_ISO646String(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_ISO646String **parm;
{
  return (dec_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_ISO646String
void free_UNIV_ISO646String(val)
struct type_UNIV_ISO646String *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZISO646StringUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_GeneralString
int	decode_UNIV_GeneralString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_GeneralString **parm;
{
  return (dec_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_CharacterString
int	decode_UNIV_CharacterString(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_CharacterString **parm;
{
  return (dec_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef decode_UNIV_EXTERNAL
int	decode_UNIV_EXTERNAL(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_EXTERNAL **parm;
{
  return (dec_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_EXTERNAL
void free_UNIV_EXTERNAL(val)
struct type_UNIV_EXTERNAL *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZEXTERNALUNIV], &_ZUNIV_mod, 1);
}

#undef decode_UNIV_ObjectDescriptor
int	decode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_UNIV_ObjectDescriptor **parm;
{
  return (dec_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef free_UNIV_ObjectDescriptor
void free_UNIV_ObjectDescriptor(val)
struct type_UNIV_ObjectDescriptor *val;
{
(void) fre_obj((char *) val, _ZUNIV_mod.md_dtab[_ZObjectDescriptorUNIV], &_ZUNIV_mod, 1);
}

#endif	/* lint||PEPSY_LINKABLE_FUNCS */
