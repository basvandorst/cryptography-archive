/* automatically generated by pepsy 8.0 #2 (pilsen), do not edit! */

#ifndef	_module_UNIV_defined_
#define	_module_UNIV_defined_

#ifndef	PEPSY_VERSION
#define	PEPSY_VERSION		2
#endif

#ifndef	PEPYPATH
#include <isode/psap.h>
#include <isode/pepsy.h>
#else
#include "psap.h"
#include "pepsy.h"
#endif



extern modtyp	_ZUNIV_mod;
#define _ZGraphicStringUNIV	10
#define _ZIA5StringUNIV	0
#define _ZGeneralStringUNIV	13
#define _ZUTCTimeUNIV	8
#define _ZT61StringUNIV	3
#define _ZTeletexStringUNIV	4
#define _ZEXTERNALUNIV	15
#define _ZVideotexStringUNIV	5
#define _ZGeneralisedTimeUNIV	7
#define _ZObjectDescriptorUNIV	16
#define _ZPrintableStringUNIV	2
#define _ZGeneralizedTimeUNIV	6
#define _ZUniversalTimeUNIV	9
#define _ZCharacterStringUNIV	14
#define _ZVisibleStringUNIV	11
#define _ZISO646StringUNIV	12
#define _ZNumericStringUNIV	1

#ifndef	lint
#define encode_UNIV_IA5String(pe, top, len, buffer, parm) \
    enc_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_IA5String(pe, top, len, buffer, parm) \
    dec_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_IA5String(pe, top, len, buffer, parm) \
    prnt_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_IA5String_P    _ZIA5StringUNIV, &_ZUNIV_mod

#define encode_UNIV_NumericString(pe, top, len, buffer, parm) \
    enc_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_NumericString(pe, top, len, buffer, parm) \
    dec_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_NumericString(pe, top, len, buffer, parm) \
    prnt_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_NumericString_P    _ZNumericStringUNIV, &_ZUNIV_mod

#define encode_UNIV_PrintableString(pe, top, len, buffer, parm) \
    enc_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_PrintableString(pe, top, len, buffer, parm) \
    dec_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_PrintableString(pe, top, len, buffer, parm) \
    prnt_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_PrintableString_P    _ZPrintableStringUNIV, &_ZUNIV_mod

#define encode_UNIV_T61String(pe, top, len, buffer, parm) \
    enc_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_T61String(pe, top, len, buffer, parm) \
    dec_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_T61String(pe, top, len, buffer, parm) \
    prnt_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_T61String_P    _ZT61StringUNIV, &_ZUNIV_mod

#define encode_UNIV_TeletexString(pe, top, len, buffer, parm) \
    enc_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_TeletexString(pe, top, len, buffer, parm) \
    dec_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_TeletexString(pe, top, len, buffer, parm) \
    prnt_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_TeletexString_P    _ZTeletexStringUNIV, &_ZUNIV_mod

#define encode_UNIV_VideotexString(pe, top, len, buffer, parm) \
    enc_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_VideotexString(pe, top, len, buffer, parm) \
    dec_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_VideotexString(pe, top, len, buffer, parm) \
    prnt_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_VideotexString_P    _ZVideotexStringUNIV, &_ZUNIV_mod

#define encode_UNIV_GeneralizedTime(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GeneralizedTime(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GeneralizedTime(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GeneralizedTime_P    _ZGeneralizedTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_GeneralisedTime(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GeneralisedTime(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GeneralisedTime(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GeneralisedTime_P    _ZGeneralisedTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_UTCTime(pe, top, len, buffer, parm) \
    enc_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_UTCTime(pe, top, len, buffer, parm) \
    dec_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_UTCTime(pe, top, len, buffer, parm) \
    prnt_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_UTCTime_P    _ZUTCTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_UniversalTime(pe, top, len, buffer, parm) \
    enc_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_UniversalTime(pe, top, len, buffer, parm) \
    dec_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_UniversalTime(pe, top, len, buffer, parm) \
    prnt_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_UniversalTime_P    _ZUniversalTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_GraphicString(pe, top, len, buffer, parm) \
    enc_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GraphicString(pe, top, len, buffer, parm) \
    dec_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GraphicString(pe, top, len, buffer, parm) \
    prnt_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GraphicString_P    _ZGraphicStringUNIV, &_ZUNIV_mod

#define encode_UNIV_VisibleString(pe, top, len, buffer, parm) \
    enc_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_VisibleString(pe, top, len, buffer, parm) \
    dec_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_VisibleString(pe, top, len, buffer, parm) \
    prnt_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_VisibleString_P    _ZVisibleStringUNIV, &_ZUNIV_mod

#define encode_UNIV_ISO646String(pe, top, len, buffer, parm) \
    enc_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_ISO646String(pe, top, len, buffer, parm) \
    dec_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_ISO646String(pe, top, len, buffer, parm) \
    prnt_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_ISO646String_P    _ZISO646StringUNIV, &_ZUNIV_mod

#define encode_UNIV_GeneralString(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GeneralString(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GeneralString(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GeneralString_P    _ZGeneralStringUNIV, &_ZUNIV_mod

#define encode_UNIV_CharacterString(pe, top, len, buffer, parm) \
    enc_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_CharacterString(pe, top, len, buffer, parm) \
    dec_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_CharacterString(pe, top, len, buffer, parm) \
    prnt_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_CharacterString_P    _ZCharacterStringUNIV, &_ZUNIV_mod

#define encode_UNIV_EXTERNAL(pe, top, len, buffer, parm) \
    enc_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_EXTERNAL(pe, top, len, buffer, parm) \
    dec_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_EXTERNAL(pe, top, len, buffer, parm) \
    prnt_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_EXTERNAL_P    _ZEXTERNALUNIV, &_ZUNIV_mod

#define encode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm) \
    enc_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm) \
    dec_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_ObjectDescriptor(pe, top, len, buffer, parm) \
    prnt_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_ObjectDescriptor_P    _ZObjectDescriptorUNIV, &_ZUNIV_mod


#endif   /* lint */

#define	type_UNIV_IA5String	qbuf
#define	free_UNIV_IA5String	qb_free

#define	type_UNIV_NumericString	type_UNIV_IA5String
#define	free_UNIV_NumericString	free_UNIV_IA5String

#define	type_UNIV_PrintableString	type_UNIV_IA5String
#define	free_UNIV_PrintableString	free_UNIV_IA5String

#define	type_UNIV_T61String	qbuf
#define	free_UNIV_T61String	qb_free

#define	type_UNIV_TeletexString	type_UNIV_T61String
#define	free_UNIV_TeletexString	free_UNIV_T61String

#define	type_UNIV_VideotexString	qbuf
#define	free_UNIV_VideotexString	qb_free

#define	type_UNIV_GeneralizedTime	type_UNIV_VisibleString
#define	free_UNIV_GeneralizedTime	free_UNIV_VisibleString

#define	type_UNIV_GeneralisedTime	type_UNIV_GeneralizedTime
#define	free_UNIV_GeneralisedTime	free_UNIV_GeneralizedTime

#define	type_UNIV_UTCTime	type_UNIV_VisibleString
#define	free_UNIV_UTCTime	free_UNIV_VisibleString

#define	type_UNIV_UniversalTime	type_UNIV_UTCTime
#define	free_UNIV_UniversalTime	free_UNIV_UTCTime

#define	type_UNIV_GraphicString	qbuf
#define	free_UNIV_GraphicString	qb_free

#define	type_UNIV_VisibleString	qbuf
#define	free_UNIV_VisibleString	qb_free

#define	type_UNIV_ISO646String	type_UNIV_VisibleString
#define	free_UNIV_ISO646String	free_UNIV_VisibleString

#define	type_UNIV_GeneralString	qbuf
#define	free_UNIV_GeneralString	qb_free

#define	type_UNIV_CharacterString	qbuf
#define	free_UNIV_CharacterString	qb_free

#define	type_UNIV_ObjectDescriptor	type_UNIV_GraphicString
#define	free_UNIV_ObjectDescriptor	free_UNIV_GraphicString

struct type_UNIV_EXTERNAL {
    OID     direct__reference;

    integer     indirect__reference;

    struct type_UNIV_ObjectDescriptor *data__value__descriptor;

    struct choice_UNIV_0 {
        int         offset;
#define	choice_UNIV_0_single__ASN1__type	1
#define	choice_UNIV_0_octet__aligned	2
#define	choice_UNIV_0_arbitrary	3

        union {
            PE      single__ASN1__type;

            struct qbuf *octet__aligned;

            PE      arbitrary;
        }       un;
    } *encoding;
};
#define	free_UNIV_EXTERNAL(parm)\
	(void) fre_obj((char *) parm, _ZUNIV_mod.md_dtab[_ZEXTERNALUNIV], &_ZUNIV_mod, 1)
#endif
