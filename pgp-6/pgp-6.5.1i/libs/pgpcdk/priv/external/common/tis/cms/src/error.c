/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"
#include "cms.h"

const char *TC_errlist[] = {
/* TC_E_INVARGS */ "invalid arguments",
/* TC_E_PARSE */  "ASN.1 parse error",
/* TC_E_NOMEMORY */  "out of memory",
/* TC_E_NOISSUER */  "issuer certificate is missing",
/* TC_E_INVSIG */ "invalid signature",
/* TC_E_INVDATE */  "invalid/expired data",
/* TC_E_EXTENSION */ "error while processing extension",
/* TC_E_FILE */  "error reading/writing to a file",
/* TC_E_CRITICAL */  "unhandled critical extension",
/* TC_E_DNAMEPARSE */ "error parsing distinguished name",
/* TC_E_NOTFOUND */  "certificate not found",
/* TC_E_INVSUBJ */  "alt-subject not critical, and no subject",
/* TC_E_INVISSU */  "alt-issuer not critical, and no issuer",
/* TC_E_CERTREVOKED */  "certificate revoked",
/* TC_E_NOCRL */  "issuer CRL is not present",
/* TC_E_PQG */  "unable to find PQG values for key",
/* TC_E_NOSERIAL */ "unable to find valid serial number in certificate",
/* TC_E_SIGNFAIL */ "unable to sign data",
/* TC_E_NOTASCII */ "processed Name data is not ascii",
/* TC_E_INVDATECRL */ "CRL's nextDate has expired",
/* TC_E_OTHER */ "unknown error, contact vendor",
/* TC_E_EXTNOTSUPPORTED */ "extension OID supplied not supported",
/* TC_E_NAMETOOLONG */ "provided string name has too many components",
/* TC_E_BADNUMERICSTRING */ "numeric string has disallowed chars",
/* TC_E_BADPRINTABLESTRING */ "printable string has disallowed chars",
/* TC_E_BADDNAMESTRINGTYPE */ "provided string type is not allowed",
/* TC_E_INVALIDPERIOD */ "one of notBefore or notAfter must be defined in privateKeyUsagePeriod",
/* TC_E_COPYNOTSUPPORTED */ "general name contains unsupported data type",
/* TC_E_COMPARENOTSUPPORTED */ "can not compare general names due to unsupported type",
/* TC_E_INVALIDCRL */ "CRL is invalid",
/* TC_E_NOTV2CRL */ "CRL is not version 2",
/* TC_E_MISSINGNEXTUPDATE */ "CRL is missing the required nextUpdate field",
/* TC_E_MISSINGAUTHKEYIDEXT */ "CRL is missing the AuthorityKeyIdentifier extension",
/* TC_E_MISSINGCRLNUMBEREXT */ "CRL is missing the CRLNumber extension",
/* TC_E_WRONGCRL */ "CRL does not match Certificate",
/* TC_E_INVALIDURI */ "GeneralName contains an inavalid URI value",
/* TC_E_INVALIDIPCONSTRAINT */ "GeneralName contains a malformed iPAddress constraint",
/* TC_E_WRONGNAMETYPE */ "GeneralName type does not match GeneralSubtree base",
/* TC_E_NAMETYPEUNSUPPORTED */ "GeneralName contains an unsupported type",
/* TC_E_CONSTRAINTFAIL */ "name constraints on certificate were not satisfied",
/* TC_E_MISSINGPOLICY */ "certificate is missing required policy extension",
/* TC_E_INVALIDPOLICY */ "certicicate policy does not match acceptable use",
/* TC_E_NOTCA */ "certificate is not a CA certificate",
/* TC_E_NOTSIGNINGKEY */ "CA certificate does not have the keyCertSign bit set",
/* TC_E_PATHCONSTRAINT */ "certificate path length constraint exceeded",
/* TC_E_POLICYMAPPED */ "policy mapping not allowed by prior constraint",
/* TC_E_POLICYMISMATCH */ "initial policy and acceptable policy do not overlap"
};

struct _asn2tc {
	int asnError;
	int tcError;
} ASN2TC_errors[] = {
 { PKIErrPackUnderrun, TC_E_PARSE },
 { PKIErrPackOverrun, TC_E_PARSE },
 { PKIErrPackSEQOFArrayTooLong, TC_E_PARSE },
 { PKIErrPackNoBlock, TC_E_PARSE },
 { PKIErrUnpackNoStructure, TC_E_INVARGS },
 { PKIErrUnpackNoBlockPtr, TC_E_INVARGS },
 { PKIErrUnpackNoBlock, TC_E_INVARGS },
 { PKIErrUnpackOverrun, TC_E_PARSE },
 { PKIErrUnpackUnderrun, TC_E_PARSE },
 { PKIErrUnpackBooleanLth, TC_E_PARSE },
 { PKIErrUnpackTaggedLth, TC_E_PARSE },
 { PKIErrUnpackReqMissing, TC_E_PARSE },
 { PKIErrUnpackNullLth, TC_E_PARSE },
 { PKIErrPackSETOFArrayTooLong, TC_E_PARSE },
 { PKIErrPackSETOFUnsortable, TC_E_PARSE },
 { PKIErrOutOfMemory, TC_E_NOMEMORY },
 { PKIErrPackBufferTooShort, TC_E_PARSE },
 { PKIErrUnpackInvalidEncoding, TC_E_PARSE },
 { PKIErrBadContext, TC_E_OTHER },
 { PKIErrChoiceBadType, TC_E_PARSE },
 { PKIErrBadNumericString, TC_E_BADNUMERICSTRING },
 { PKIErrBadPrintableString, TC_E_BADPRINTABLESTRING },
 { PKIErrNotFoundInDict, TC_E_OTHER },
 { 0, 0 }
};

int compiler2tc_error(int x)
{
    int i;

    if (x==0) return 0;

    for (i = 0; ASN2TC_errors[i].asnError != 0; i++)
        if (ASN2TC_errors[i].asnError == x)
	    return ASN2TC_errors[i].tcError;

    return TC_E_OTHER;
}
