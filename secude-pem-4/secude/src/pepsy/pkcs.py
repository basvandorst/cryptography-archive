PKCS

DEFINITIONS ::=

%{	/* surrounding global definitions */
#include        "pkcs.h"

	extern OctetString   * e_ExtendedCertificateInfo();
	extern OctetString   * e_CertificationRequestInfo();

	static OctetString   * tmp_ostr1, * tmp_ostr2, * tmp_ostr3, * tmp_ostr4;
	static OctetString   * tmp_ostr5, * tmp_ostr6, * tmp_ostr7, * tmp_ostr8;
	static OctetString     save_modulus, save_pubex, save_privex, save_prime1;
	static OctetString     save_prime2, save_exp1, save_exp2, save_coeff;
	static OctetString   * nullostr = NULLOCTETSTRING;
	static int	       i;
%}

PREFIXES build parse print

BEGIN



RSAPrivateKey [[P RSAPrivateKey *]]
	::=
	%E{

	tmp_ostr1 = tmp_ostr2 = tmp_ostr3 = tmp_ostr4 = NULLOCTETSTRING;
	tmp_ostr5 = tmp_ostr6 = tmp_ostr7 = tmp_ostr8 = NULLOCTETSTRING;

	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	if ((parm->modulus.noctets == 0) || (parm->pubex.noctets == 0) ||
	    (parm->privex.noctets == 0) || (parm->prime1.noctets == 0) ||
	    (parm->prime2.noctets == 0) || (parm->exp1.noctets == 0) ||
	    (parm->exp2.noctets == 0) || (parm->coeff.noctets == 0))
		return(NOTOK);

	if(parm->modulus.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr1 = aux_concatenate_OctetString(nullostr, &parm->modulus);

		/* Save values of parm->modulus */
		save_modulus.noctets = parm->modulus.noctets;
		save_modulus.octets = parm->modulus.octets;
		parm->modulus.noctets = tmp_ostr1->noctets;
		parm->modulus.octets = tmp_ostr1->octets;
	}

	if(parm->pubex.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr2 = aux_concatenate_OctetString(nullostr, &parm->pubex);

		/* Save values of parm->pubex */
		save_pubex.noctets = parm->pubex.noctets;
		save_pubex.octets = parm->pubex.octets;
		parm->pubex.noctets = tmp_ostr2->noctets;
		parm->pubex.octets = tmp_ostr2->octets;
	}

	if(parm->privex.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr3 = aux_concatenate_OctetString(nullostr, &parm->privex);

		/* Save values of parm->privex */
		save_privex.noctets = parm->privex.noctets;
		save_privex.octets = parm->privex.octets;
		parm->privex.noctets = tmp_ostr3->noctets;
		parm->privex.octets = tmp_ostr3->octets;
	}

	if(parm->prime1.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr4 = aux_concatenate_OctetString(nullostr, &parm->prime1);

		/* Save values of parm->prime1 */
		save_prime1.noctets = parm->prime1.noctets;
		save_prime1.octets = parm->prime1.octets;
		parm->prime1.noctets = tmp_ostr4->noctets;
		parm->prime1.octets = tmp_ostr4->octets;
	}

	if(parm->prime2.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr5 = aux_concatenate_OctetString(nullostr, &parm->prime2);

		/* Save values of parm->prime2 */
		save_prime2.noctets = parm->prime2.noctets;
		save_prime2.octets = parm->prime2.octets;
		parm->prime2.noctets = tmp_ostr5->noctets;
		parm->prime2.octets = tmp_ostr5->octets;
	}

	if(parm->exp1.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr6 = aux_concatenate_OctetString(nullostr, &parm->exp1);

		/* Save values of parm->exp1 */
		save_exp1.noctets = parm->exp1.noctets;
		save_exp1.octets = parm->exp1.octets;
		parm->exp1.noctets = tmp_ostr6->noctets;
		parm->exp1.octets = tmp_ostr6->octets;
	}

	if(parm->exp2.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr7 = aux_concatenate_OctetString(nullostr, &parm->exp2);

		/* Save values of parm->exp2 */
		save_exp2.noctets = parm->exp2.noctets;
		save_exp2.octets = parm->exp2.octets;
		parm->exp2.noctets = tmp_ostr7->noctets;
		parm->exp2.octets = tmp_ostr7->octets;
	}

	if(parm->coeff.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr8 = aux_concatenate_OctetString(nullostr, &parm->coeff);

		/* Save values of parm->coeff */
		save_coeff.noctets = parm->coeff.noctets;
		save_coeff.octets = parm->coeff.octets;
		parm->coeff.noctets = tmp_ostr8->noctets;
		parm->coeff.octets = tmp_ostr8->octets;
	}
	%}

	SEQUENCE 
	{
	version 
		INTEGER [[i version]],

	modulus     
		SEC.TOURINTEGER [[p &parm->modulus]],

	publicExponent
		SEC.TOURINTEGER [[p &parm->pubex]],

	privateExponent
		SEC.TOURINTEGER [[p &parm->privex]],

	prime1
		SEC.TOURINTEGER [[p &parm->prime1]],

	prime2
		SEC.TOURINTEGER [[p &parm->prime2]],

	exponent1
		SEC.TOURINTEGER [[p &parm->exp1]],

	exponent2
		SEC.TOURINTEGER [[p &parm->exp2]],

	coefficient
		SEC.TOURINTEGER [[p &parm->coeff]]
	}

	%E{
	if(tmp_ostr1) {
		aux_free_OctetString(&tmp_ostr1);
		parm->modulus.noctets = save_modulus.noctets;
		parm->modulus.octets = save_modulus.octets;
	}
	if(tmp_ostr2) {
		aux_free_OctetString(&tmp_ostr2);
		parm->pubex.noctets = save_pubex.noctets;
		parm->pubex.octets = save_pubex.octets;
	}
	if(tmp_ostr3) {
		aux_free_OctetString(&tmp_ostr3);
		parm->privex.noctets = save_privex.noctets;
		parm->privex.octets = save_privex.octets;
	}
	if(tmp_ostr4) {
		aux_free_OctetString(&tmp_ostr4);
		parm->prime1.noctets = save_prime1.noctets;
		parm->prime1.octets = save_prime1.octets;
	}
	if(tmp_ostr5) {
		aux_free_OctetString(&tmp_ostr5);
		parm->prime2.noctets = save_prime2.noctets;
		parm->prime2.octets = save_prime2.octets;
	}
	if(tmp_ostr6) {
		aux_free_OctetString(&tmp_ostr6);
		parm->exp1.noctets = save_exp1.noctets;
		parm->exp1.octets = save_exp1.octets;
	}
	if(tmp_ostr7) {
		aux_free_OctetString(&tmp_ostr7);
		parm->exp2.noctets = save_exp2.noctets;
		parm->exp2.octets = save_exp2.octets;
	}
	if(tmp_ostr8) {
		aux_free_OctetString(&tmp_ostr8);
		parm->coeff.noctets = save_coeff.noctets;
		parm->coeff.octets = save_coeff.octets;
	}
	%}
	%D{
	if(! ((*parm)->modulus.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->modulus.noctets - 1; i++)
			(*parm)->modulus.octets[i] = (*parm)->modulus.octets[i + 1];
		(*parm)->modulus.noctets -= 1;
		(*parm)->modulus.octets = (char *)realloc((*parm)->modulus.octets, (*parm)->modulus.noctets);
	}

	if(! ((*parm)->pubex.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->pubex.noctets - 1; i++)
			(*parm)->pubex.octets[i] = (*parm)->pubex.octets[i + 1];
		(*parm)->pubex.noctets -= 1;
		(*parm)->pubex.octets = (char *)realloc((*parm)->pubex.octets, (*parm)->pubex.noctets);
	}

	if(! ((*parm)->privex.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->privex.noctets - 1; i++)
			(*parm)->privex.octets[i] = (*parm)->privex.octets[i + 1];
		(*parm)->privex.noctets -= 1;
		(*parm)->privex.octets = (char *)realloc((*parm)->privex.octets, (*parm)->privex.noctets);
	}

	if(! ((*parm)->prime1.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->prime1.noctets - 1; i++)
			(*parm)->prime1.octets[i] = (*parm)->prime1.octets[i + 1];
		(*parm)->prime1.noctets -= 1;
		(*parm)->prime1.octets = (char *)realloc((*parm)->prime1.octets, (*parm)->prime1.noctets);
	}

	if(! ((*parm)->prime2.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->prime2.noctets - 1; i++)
			(*parm)->prime2.octets[i] = (*parm)->prime2.octets[i + 1];
		(*parm)->prime2.noctets -= 1;
		(*parm)->prime2.octets = (char *)realloc((*parm)->prime2.octets, (*parm)->prime2.noctets);
	}

	if(! ((*parm)->exp1.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->exp1.noctets - 1; i++)
			(*parm)->exp1.octets[i] = (*parm)->exp1.octets[i + 1];
		(*parm)->exp1.noctets -= 1;
		(*parm)->exp1.octets = (char *)realloc((*parm)->exp1.octets, (*parm)->exp1.noctets);
	}

	if(! ((*parm)->exp2.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->exp2.noctets - 1; i++)
			(*parm)->exp2.octets[i] = (*parm)->exp2.octets[i + 1];
		(*parm)->exp2.noctets -= 1;
		(*parm)->exp2.octets = (char *)realloc((*parm)->exp2.octets, (*parm)->exp2.noctets);
	}

	if(! ((*parm)->coeff.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->coeff.noctets - 1; i++)
			(*parm)->coeff.octets[i] = (*parm)->coeff.octets[i + 1];
		(*parm)->coeff.noctets -= 1;
		(*parm)->coeff.octets = (char *)realloc((*parm)->coeff.octets, (*parm)->coeff.noctets);
	}
	%}


ExtendedCertificateOrCertificate [[P ExtendedCertificateOrCertificate *]]
	::= 
	CHOICE <<choice>>
	{
  	certificate 
		SECAF.Certificate [[p un.cert]], -- X.509

  	extendedCertificate 
		[0] IMPLICIT ExtendedCertificate [[p un.extcert]]
	}


ExtendedCertificatesAndCertificates [[P ExtendedCertificatesAndCertificates *]]
	::=
  	SET OF [[T ExtendedCertificatesAndCertificates * $ *]] <<next>>
		ExtendedCertificateOrCertificate [[p element]]


ExtendedCertificateInfo [[P ExtendedCertificateInfo *]]
	::=
	SEQUENCE
	{
	version
		INTEGER [[i version]],

	certificate
		SECAF.Certificate [[p cert]],

	attributes
		SECIF.SETOFAttribute [[p attributes]]
	}


ExtendedCertificate [[P ExtendedCertificate *]]
	::=
        %D{
        if (((*parm) = (ExtendedCertificate *) calloc (1, sizeof (ExtendedCertificate))) == ((ExtendedCertificate *) 0)) {
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
	extendedCertificateInfo
		ExtendedCertificateInfo [[p extcertinfo]]
		%D{
		(*parm)->tbs_DERcode = e_ExtendedCertificateInfo((*parm)->extcertinfo);
		%},

	signatureAlgorithm
		SEC.TAlgorithmIdentifier [[p sig]],

	signature
		-- ENCRYPTED OCTET STRING
		SEC.TBitString [[p sig]]
	}
	-- end of SIGNED


IssuerAndSerialNumber [[P IssuerAndSerialNumber *]]
	::= 
	SEQUENCE 
	{
  	issuer 
		SECIF.Name [[p issuer]],

  	serialNumber 
		SEC.OURINTEGER [[p serialnumber]] 
	}


ContentInfo [[P ContentInfo *]]
	::= 
	SEQUENCE 
	{
  	contentType 
		OBJECT IDENTIFIER [[O contentType]]
		-- for all content types true --
		%E{
	    	parm->contentchoice = aux_ObjId2ContentType(parm->contentType);
	    	if (parm->contentchoice < 0){
            		advise (NULLCP, "parm->contentchoice has undefined value");
            		return NOTOK;
       		} 
		if (parm->contentchoice == 1)
			parm->un.data = (data_type *)(parm->content);
		else if (parm->contentchoice == 2)
			parm->un.signedData = (signedData_type *)(parm->content);
		else if (parm->contentchoice == 3)
			parm->un.envelopedData = (envelopedData_type *)(parm->content);
		else if (parm->contentchoice == 4)
			parm->un.signedAndEnvelopedData = (signedAndEnvelopedData_type *)(parm->content);
		else if (parm->contentchoice == 5)
			parm->un.digestedData = (digestedData_type *)(parm->content);
		else if (parm->contentchoice == 6)
			parm->un.encryptedData = (encryptedData_type *)(parm->content);
		%}
		%D{
		(*parm)->content = CNULL;
		(*parm)->un.data = (data_type *)0;
		(*parm)->un.signedData = (signedData_type *)0;
		(*parm)->un.envelopedData = (envelopedData_type *)0;
		(*parm)->un.signedAndEnvelopedData = (signedAndEnvelopedData_type *)0;
		(*parm)->un.digestedData = (digestedData_type *)0;
		(*parm)->un.encryptedData = (encryptedData_type *)0;
	    	(*parm)->contentchoice = aux_ObjId2ContentType((*parm)->contentType);
	    	if ((*parm)->contentchoice < 0){
            		advise (NULLCP, "parm->contentchoice has undefined value");
            		return NOTOK;
       		}
		%}
		,

  	content
		[0] EXPLICIT CHOICE [[T ContentInfo * $ *]] <E<parm->contentchoice>><D<(*parm)->contentchoice>>
		{
		data   
			SEC.OctetString [[p un.data]]
     			%D{
		  	(*parm)->content = (char *)aux_cpy_OctetString((*parm)->un.data);
		  	if((*parm)->un.data)
				aux_free_OctetString(&((*parm)->un.data));
     			%},

		signedData 
			SignedData [[p un.signedData]]
     			%D{
		  	(*parm)->content = (char *)aux_cpy_SignedData((*parm)->un.signedData);
		  	if((*parm)->un.signedData)
				aux_free_SignedData(&((*parm)->un.signedData));
    			%},

		envelopedData
			EnvelopedData [[p un.envelopedData]]
     			%D{
		  	(*parm)->content = (char *)aux_cpy_EnvelopedData((*parm)->un.envelopedData);
		  	if((*parm)->un.envelopedData)
				aux_free_EnvelopedData(&((*parm)->un.envelopedData));
     			%},

		signedAndEnvelopedData
			SignedAndEnvelopedData [[p un.signedAndEnvelopedData]]
     			%D{
		  	(*parm)->content = (char *)aux_cpy_SignedAndEnvelopedData((*parm)->un.signedAndEnvelopedData);
		  	if((*parm)->un.signedAndEnvelopedData)
				aux_free_SignedAndEnvelopedData(&((*parm)->un.signedAndEnvelopedData));
     			%},

		digestedData
			DigestedData [[p un.digestedData]]
     			%D{
		  	(*parm)->content = (char *)aux_cpy_DigestedData((*parm)->un.digestedData);
		  	if((*parm)->un.digestedData)
				aux_free_DigestedData(&((*parm)->un.digestedData));
     			%},

		encryptedData
			EncryptedData [[p un.encryptedData]]
     			%D{
		  	(*parm)->content = (char *)aux_cpy_EncryptedData((*parm)->un.encryptedData);
		  	if((*parm)->un.encryptedData)
				aux_free_EncryptedData(&((*parm)->un.encryptedData));
     			%}

		} OPTIONAL <E<parm->contentchoice > 0>><D<(*parm)->contentchoice > 0>>
	} 


SignerInfo [[P SignerInfo *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	issuerAndSerialNumber 
		IssuerAndSerialNumber [[p issuerandserial]],

  	digestAlgorithm 
		SEC.AlgorithmIdentifier [[p digestAI]],

  	authenticatedAttributes
    		[0] IMPLICIT SECIF.SETOFAttribute [[p authAttributes]] 
		OPTIONAL,

  	digestEncryptionAlgorithm
    		SEC.AlgorithmIdentifier [[p digencAI]],

  	encryptedDigest 
		SEC.OctetString [[p encryptedDigest]],

  	unauthenticatedAttributes
    		[1] IMPLICIT SECIF.SETOFAttribute [[p unauthAttributes]]
		OPTIONAL 
	}


SignerInfos [[P SignerInfos *]] 
	::=
        SET OF [[T SignerInfos * $ *]] <<next>>
        	SignerInfo [[p element]]


RecipientInfo [[P RecipientInfo *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	issuerAndSerialNumber 
		IssuerAndSerialNumber [[p issuerandserial]],

  	keyEncryptionAlgorithm
    		SEC.AlgorithmIdentifier [[p keyencAI]],

  	encryptedKey 
		SEC.OctetString [[p encryptedKey]] 
	}


RecipientInfos [[P RecipientInfos *]] 
	::=
        SET OF [[T RecipientInfos * $ *]] <<next>>
        	RecipientInfo [[p element]]


EncryptedContentInfo [[P EncryptedContentInfo *]]
	::= 
	SEQUENCE 
	{
  	contentType 
		OBJECT IDENTIFIER [[O contentType]],

  	contentEncryptionAlgorithm
    		SEC.AlgorithmIdentifier [[p contentencAI]],

  	encryptedContent
    		[0] IMPLICIT OCTET STRING [[o encryptedContent.octets $ encryptedContent.noctets]]
		OPTIONAL 
	}


SignedData [[P SignedData *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	digestAlgorithms 
		SEC.SETOFAlgorithmIdentifier [[p digestAlgorithms]],

  	contentInfo 
		ContentInfo [[p contentInfo]],

  	certificates
     		[0] IMPLICIT ExtendedCertificatesAndCertificates [[p certificates]]
       		OPTIONAL,

  	crls
    		[1] IMPLICIT SECAF.SETOFCRL [[p crls]]
		OPTIONAL,

  	signerInfos 
		SignerInfos [[p signerInfos]]
	}


EnvelopedData [[P EnvelopedData *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	recipientInfos 
		RecipientInfos [[p recipientInfos]],

  	encryptedContentInfo 
		EncryptedContentInfo [[p encryptedContInfo]]
	}


SignedAndEnvelopedData [[P SignedAndEnvelopedData *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	recipientInfos 
		RecipientInfos [[p recipientInfos]],

  	digestAlgorithms 
		SEC.SETOFAlgorithmIdentifier [[p digestAIs]],

  	encryptedContentInfo 
		EncryptedContentInfo [[p encryptedContInfo]],

  	certificates
     		[0] IMPLICIT ExtendedCertificatesAndCertificates [[p certificates]]
       		OPTIONAL,

  	crls
    		[1] IMPLICIT SECAF.SETOFCRL [[p crls]] 
		OPTIONAL,

  	signerInfos 
		SignerInfos [[p signerInfos]]
	}


DigestedData [[P DigestedData *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	digestAlgorithm 
		SEC.AlgorithmIdentifier [[p digestAI]],

  	contentInfo 
		ContentInfo [[p contentInfo]],

  	digest 
		SEC.OctetString [[p digest]] 
	}


EncryptedData [[P EncryptedData *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	encryptedContentInfo 
		EncryptedContentInfo [[p encryptedContInfo]] 
	}


CertificationRequestInfo [[P CertificationRequestInfo *]]
	::= 
	SEQUENCE 
	{
  	version 
		INTEGER [[i version]],

  	subject 
		SECIF.Name [[p subject]],

  	subjectPublicKeyInfo 
		SEC.KeyInfo [[p subjectPK]],

  	attributes [0] IMPLICIT SECIF.SETOFAttribute [[p attributes]]
	}


CertificationRequest [[P CertificationRequest *]]
	::=
        %D{
        if (((*parm) = (CertificationRequest *) calloc (1, sizeof (CertificationRequest))) == ((CertificationRequest *) 0)) {
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
	certificationRequestInfo
		CertificationRequestInfo [[p certreqinfo]]
		%D{
		(*parm)->tbs_DERcode = e_CertificationRequestInfo((*parm)->certreqinfo);
		%},

	signatureAlgorithm
		SEC.TAlgorithmIdentifier [[p sig]],

	signature
		-- ENCRYPTED OCTET STRING
		SEC.TBitString [[p sig]]
	}
	-- end of SIGNED

END
