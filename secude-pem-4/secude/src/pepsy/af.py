-- af.py
--
--                Certificates and Related Types from:
--            X.509 The Directory - Authentication Framework
--                  Subset of Definitions in Annex G
--
--                PKRoot and PKList from SecureDFN Project

-- AuthenticationFramework


SECAF
	{
	joint-iso-ccitt
	ds(5)
	modules(1)
	authenticationFramework(7)
	}

DEFINITIONS ::=

%{	/* surrounding global definitions */
#include        "af.h"
	
	static OctetString * tmp_ostr, * save_serialnumber;
	static OctetString * nullostr = NULLOCTETSTRING;
	static int i,j;
%}

PREFIXES build parse print

BEGIN


-- Temp label to aid Pepsy parameter passing

TCertificatePair [[P SEQUENCE_OF_CertificatePair *]]
	::= 
	CertificatePair [[p parm->element]]


-- *************************************************************


Validity [[P Validity *]]
	::=
        SEQUENCE 
	{
	notBefore
        	UTCTime [[s notbefore]],

	notAfter
                UTCTime [[s notafter]]
        }


PKRoot [[P PKRoot *]] 
	::=
        SEQUENCE 
	{
        cA
        	SECIF.Name [[p ca]],

        newkey
                RootKey [[p newkey]],

        oldkey
                RootKey [[p oldkey]]
                OPTIONAL <E<parm->oldkey != (struct Serial *)0>><D<0>>
        }


RootKey [[P struct Serial *]]
	::=
	%E{
	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serial && (parm->serial->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serial);
		save_serialnumber = parm->serial;
		parm->serial = tmp_ostr;
	}
	%}
        %D{
        if (((*parm) = (struct Serial *) malloc (sizeof (struct Serial))) == ((struct Serial *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) malloc (sizeof (Signature))) == ((Signature *) 0)) {
		    advise (NULLCP, "out of memory");
		    return NOTOK;
	}
        %}
        SEQUENCE
	{
        version
		[0] INTEGER [[i version]]
                {
                v1988(0),
		v1992(1)
                }
                DEFAULT v1988,
		
        serial
                SEC.OURINTEGER [[p serial]],

        key
                SEC.KeyInfo [[p key]],

        validity
                [1] Validity [[p valid]]
		OPTIONAL <E<parm->valid && parm->valid->notbefore != (UTCTime *)0>><D<0>>,

	signAlgID
                [2] SEC.TAlgorithmIdentifier [[p sig]]
		OPTIONAL <E<parm->sig != (Signature *)0>><D<0>>,

	signBitString
		-- ENCRYPTED OCTET STRING --
		SEC.TBitString [[p sig]]
		OPTIONAL <E<parm->sig != (Signature *)0>><D<0>>
        }
	%E{
	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serial = save_serialnumber;
	}
	%}
        %D{
	if(! (*parm)->sig->signAI){
		aux_free_KeyInfo( (KeyInfo **) & ((*parm)->sig) );
		(*parm)->sig = (Signature * ) 0;
	}
	if((*parm)->serial->noctets > 1 && !((*parm)->serial->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serial->noctets - 1; i++)
			(*parm)->serial->octets[i] = (*parm)->serial->octets[i + 1];
		(*parm)->serial->noctets -= 1;
		(*parm)->serial->octets = (char *)realloc((*parm)->serial->octets, (*parm)->serial->noctets);
	}
	%}


TBSCertificate [[P ToBeSigned *]]  
	::=
	%E{
	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serialnumber && (parm->serialnumber->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serialnumber);
		save_serialnumber = parm->serialnumber;
		parm->serialnumber = tmp_ostr;
	}
	%}
        SEQUENCE 
	{
        version
		[0] INTEGER [[i version]]
                {
                v1988(0),
		v1992(1)
                }
                -- DEFAULT v1988
		OPTIONAL <E<parm->version != 0>><D<0>>,

        serialNumber
                SEC.OURINTEGER [[p serialnumber]],

        signature
                SEC.AlgorithmIdentifier [[p signatureAI]],

        issuer
                SECIF.Name [[p issuer]],

        validity
		Validity [[p valid]],

        subject
                SECIF.Name [[p subject]],

        subjectPublickeyInfo
                SEC.KeyInfo [[p subjectPK]]
        }
	%E{
	if(tmp_ostr){
		aux_free_OctetString(&tmp_ostr);
		parm->serialnumber = save_serialnumber;
	}
	%}
	%D{
	if((*parm)->serialnumber->noctets > 1 && !((*parm)->serialnumber->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serialnumber->noctets - 1; i++)
			(*parm)->serialnumber->octets[i] = (*parm)->serialnumber->octets[i + 1];
		(*parm)->serialnumber->noctets -= 1;
		(*parm)->serialnumber->octets = (char *)realloc((*parm)->serialnumber->octets, (*parm)->serialnumber->noctets);
	}
	%}


Certificate [[P Certificate *]]
    	::=
        %D{
        if (((*parm) = (Certificate *) malloc (sizeof (Certificate))) == ((Certificate *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) malloc (sizeof (Signature))) == ((Signature *) 0)) {
		advise (NULLCP, "out of memory");
		return NOTOK;
	}
        %}
	SEQUENCE
	-- SIGNED
	{
		TBSCertificate [[p tbs]]
		%D{
		(*parm)->tbs_DERcode = e_ToBeSigned((*parm)->tbs);
		%},

                SEC.TAlgorithmIdentifier [[p sig]],

		-- ENCRYPTED OCTET STRING
		SEC.TBitString [[p sig]]
        }


Certificates [[P Certificates *]] 
	::=
        SEQUENCE 
	{
        certificate
                Certificate [[p usercertificate]],

        certificationPath
                ForwardCertificationPath [[p forwardpath]]
                OPTIONAL <E<parm->forwardpath != (FCPath *)0>><D<0>>
        }


ForwardCertificationPath [[P FCPath *]] 
	::=
        SEQUENCE OF [[T FCPath * $ *]] <<next_forwardpath>>
        	CertificateSet [[p liste]]


CertificateSet [[P SET_OF_Certificate *]] 
	::=
        SET OF [[T SET_OF_Certificate * $ *]] <<next>>
        	Certificate [[p element]]


CertificatePair [[P CertificatePair *]] 
	::=
        %D{
        if (((*parm) = (CertificatePair *) malloc (sizeof (CertificatePair))) == ((CertificatePair *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
        SEQUENCE 
	{
        forward
                [0] Certificate [[p forward]]
		OPTIONAL <E<parm->forward != (Certificate *)0>><D<0>>,
		
        reverse
                [1] Certificate [[p reverse]]
		OPTIONAL <E<parm->reverse != (Certificate *)0>><D<0>>
        }
        %D{
        if (!(*parm)->forward && !(*parm)->reverse) {
            	advise (NULLCP, "at least one certificate must be present");
            	return NOTOK;
        }
        %}


CrossCertificatePair [[P SET_OF_CertificatePair *]] 
	::=
	SET OF [[T SET_OF_CertificatePair * $ *]] <<next>>
		CertificatePair [[p element]]


PKList [[P PKList *]] 
	::=
        SET OF [[T PKList * $ *]] <<next>>
        	TBSCertificate [[p element]]


TBSCRL [[P CRLTBS *]]
    	::=	
        SEQUENCE
        {
        signature
                SEC.AlgorithmIdentifier [[p signatureAI]],

        issuer
		SECIF.Name [[p issuer]],

        lastUpdate
                UTCTime [[s lastUpdate]],

	nextUpdate
                UTCTime [[s nextUpdate]],

        revokedCertificates
		CRLEntrySequence [[p revokedCertificates]]
		OPTIONAL <E<parm->revokedCertificates != (SEQUENCE_OF_CRLEntry *)0>><D<0>>
        }


CRL [[P CRL *]]
    	::=
        %D{
        if (((*parm) = (CRL *) malloc (sizeof (CRL))) == ((CRL *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	if (((*parm)->sig = (Signature *) malloc (sizeof (Signature))) == ((Signature *) 0)) {
		advise (NULLCP, "out of memory");
		return NOTOK;
	}
        %}
	SEQUENCE   
	-- SIGNED
	{       
		TBSCRL [[p tbs]]
		%D{
		(*parm)->tbs_DERcode = e_CRLTBS((*parm)->tbs);
		%},

                SEC.TAlgorithmIdentifier [[p sig]],

	   	-- ENCRYPTED OCTET STRING
		SEC.TBitString [[p sig]]
	}  
	-- end of SIGNED



SETOFCRL [[P SET_OF_CRL *]] 
	::=
        SET OF [[T SET_OF_CRL * $ *]] <<next>>
        	CRL [[p element]]


CRLEntry [[P CRLEntry *]] 
	::=
	%E{
	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serialnumber && (parm->serialnumber->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serialnumber);
		save_serialnumber = parm->serialnumber;
		parm->serialnumber = tmp_ostr;
	}
	%}
	SEQUENCE
        {
        serialNumber
                SEC.OURINTEGER [[p serialnumber]],

	revocationDate
                UTCTime [[s revocationDate]]
        }
	%E{
	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serialnumber = save_serialnumber;
	}
	%}
	%D{
	if((*parm)->serialnumber->noctets > 1 && !((*parm)->serialnumber->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serialnumber->noctets - 1; i++)
			(*parm)->serialnumber->octets[i] = (*parm)->serialnumber->octets[i + 1];
		(*parm)->serialnumber->noctets -= 1;
		(*parm)->serialnumber->octets = (char *)realloc((*parm)->serialnumber->octets, (*parm)->serialnumber->noctets);
	}
	%}


CRLEntrySequence [[P SEQUENCE_OF_CRLEntry *]]
    	::=
	SEQUENCE OF [[T SEQUENCE_OF_CRLEntry * $ *]] <<next>>
		CRLEntry [[p element]]


Crl [[P Crl *]] 
	::=
	SEQUENCE
        {
        issuer
		SECIF.Name [[p issuer]],

        nextUpdate
                UTCTime [[s nextUpdate]],

	revcerts
		CRLEntrySequence [[p revcerts]]
		OPTIONAL <E<parm->revcerts != (SEQUENCE_OF_CRLEntry *)0>><D<0>>
        }


CrlSet [[P CrlSet *]] 
	::=
        SET OF [[T CrlSet * $ *]] <<next>>
        	Crl [[p element]]


CertificationPath [[P CertificationPath *]] 
	::=
	SEQUENCE 
	{
        userCertificate
                Certificate [[p userCertificate]],

        theCACertificates
                CertificatePairs[[p theCACertificates]]
		OPTIONAL <E<parm->theCACertificates != (CertificatePairs *)0>><D<0>>
        }


CRLWithCertificates [[P CRLWithCertificates *]] 
	::=
	SEQUENCE 
	{
        pemCRL
                CRL [[p crl]],

        certificates
                Certificates [[p certificates]]
		OPTIONAL <E<parm->certificates != (Certificates *)0>><D<0>>
        }


AliasesMember [[P Aliases *]] 
	::=
	SEQUENCE 
	{
	aname
		IA5String [[s aname]],
		
	aliasfile
		ENUMERATED [[i aliasfile]]
		{
		useralias (0),
		sytemalias (1)
		}
	}


Aliases [[P Aliases *]] 
	::=
	SEQUENCE OF [[T Aliases * $ *]] <<next>>
		AliasesMember [[p *]]


AliasListMember [[P AliasList *]]
	::=
	SEQUENCE 
	{
	aliases
		Aliases [[p a]],

	dname
		IA5String [[s dname]]
	}


AliasList [[P AliasList *]] 
	::=
	SEQUENCE OF [[T AliasList * $ *]] <<next>>
		AliasListMember [[p *]]


OldCertificate [[P OCList *]]
	::=
	%E{
	tmp_ostr = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if(parm->serialnumber && (parm->serialnumber->octets[0] & MSBITINBYTE)){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr = aux_concatenate_OctetString(nullostr, parm->serialnumber);
		save_serialnumber = parm->serialnumber;
		parm->serialnumber = tmp_ostr;
	}
	%}
	SEQUENCE
        {
        serialNumber
        	SEC.OURINTEGER [[p serialnumber]],

        crossCertificate
        	Certificate [[p ccert]]
        }
	%E{
	if(tmp_ostr) {
		aux_free_OctetString(&tmp_ostr);
		parm->serialnumber = save_serialnumber;
	}
	%}
	%D{
	if((*parm)->serialnumber->noctets > 1 && !((*parm)->serialnumber->octets[0] | 0x00)){    

		/* The bits of the most significant byte are all 0, so delete them */

		for ( i = 0; i < (*parm)->serialnumber->noctets - 1; i++)
			(*parm)->serialnumber->octets[i] = (*parm)->serialnumber->octets[i + 1];
		(*parm)->serialnumber->noctets -= 1;
		(*parm)->serialnumber->octets = (char *)realloc((*parm)->serialnumber->octets, (*parm)->serialnumber->noctets);
	}
	%}


OldCertificateList [[ P OCList *]]
    	::=
        SEQUENCE OF [[T OCList * $ *]] <<next>>
		OldCertificate [[p *]]


CertificatePairs [[P CertificatePairs *]] 
	::=
	SEQUENCE OF [[T CertificatePairs * $ *]] <<superior>>
		TCertificatePair [[p liste]]


END
