SEC      

DEFINITIONS ::=


%{      /* surrounding global definitions */
#include "secure.h"

	static OctetString   * tmp_ostr1, * tmp_ostr2, * tmp_ostr3, * tmp_ostr4, * ostr;
	static OctetString     save_part1, save_part2, save_part3, save_part4;
	static rsa_parm_type * rsa_parm;
	static int             i, j, rc;
	static KeyBits       * tmp_keybits;
	static OctetString   * nullostr = NULLOCTETSTRING;
%}


PREFIXES build parse print

BEGIN


-- Temp labels to aid Pepsy parameter passing
TOURINTEGER [[P OctetString *]]
	::=
	[UNIVERSAL 2] IMPLICIT OCTET STRING [[o parm->octets $ parm->noctets]]


TBitString [[P Signature *]]
	::= 
	BIT STRING [[x parm->signature.bits $ parm->signature.nbits]]


TAlgorithmIdentifier [[P Signature *]]
	::= 
	AlgorithmIdentifier [[p parm->signAI]]


-- **********************************************


OctetString [[P OctetString *]]
   	::=
        %D{
        if (((*parm) = (OctetString *) calloc (1, sizeof (OctetString))) == ((OctetString *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	OCTET STRING [[o parm->octets $ parm->noctets]]


SETOFOctetString [[P SET_OF_OctetString *]]
	::=
        SET OF [[T SET_OF_Certificate * $ *]] <<next>>
        	OctetString [[p element]]


SETOFObjId [[P SET_OF_ObjId *]]
	::=
        SET OF [[T SET_OF_ObjId * $ *]] <<next>>
        	OBJECT IDENTIFIER [[O element]]


BitString [[P BitString *]]
   	::=
        %D{
        if (((*parm) = (BitString *) calloc (1, sizeof (BitString))) == ((BitString *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	BIT STRING [[x parm->bits $ parm->nbits]]


OURINTEGER [[P OctetString *]]
	::=
        %D{
        if (((*parm) = (OctetString *) calloc (1, sizeof (OctetString))) == ((OctetString *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	[UNIVERSAL 2] IMPLICIT OCTET STRING [[o parm->octets $ parm->noctets]]


KeyBits [[P KeyBits *]]
        ::= 
	%E{

	/* valid constellations:

	   (parm->part2.noctets = 0 && parm->part3.noctets = 0 && parm->part4.noctets = 0) ||
	   (parm->part2.noctets > 0 && parm->part3.noctets = 0 && parm->part4.noctets = 0) ||
	   (parm->part2.noctets > 0 && parm->part3.noctets > 0 && parm->part4.noctets = 0) ||
	   (parm->part2.noctets > 0 && parm->part3.noctets > 0 && parm->part4.noctets > 0)

	*/
	
	if(! nullostr){
		nullostr = aux_new_OctetString(1);
		nullostr->octets[0] = 0x00;
	}

	tmp_ostr1 = tmp_ostr2 = tmp_ostr3 = tmp_ostr4 = NULLOCTETSTRING;
	parm->choice = 1;

	if ((parm->part1.noctets == 0) ||
	    ((parm->part2.noctets > 0 || parm->part3.noctets > 0 || parm->part4.noctets > 0) &&
	     (parm->part2.noctets == 0 || parm->part3.noctets > 0 || parm->part4.noctets > 0) &&
	     (parm->part2.noctets == 0 || parm->part3.noctets == 0 || parm->part4.noctets > 0) &&
	     (parm->part2.noctets == 0 || parm->part3.noctets == 0 || parm->part4.noctets == 0)))
		return(NOTOK);

	if(parm->part1.octets[0] & MSBITINBYTE){
	        /* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr1 = aux_concatenate_OctetString(nullostr, &parm->part1);

		/* Save values of parm->part1 */
		save_part1.noctets = parm->part1.noctets;
		save_part1.octets = parm->part1.octets;
		parm->part1.noctets = tmp_ostr1->noctets;
		parm->part1.octets = tmp_ostr1->octets;
	}

	if (parm->part2.noctets > 0 && (parm->part2.octets[0] & MSBITINBYTE)){
		/* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr2 = aux_concatenate_OctetString(nullostr, &parm->part2);

		/* Save values of parm->part2 */
		save_part2.noctets = parm->part2.noctets;
		save_part2.octets = parm->part2.octets;
		parm->part2.noctets = tmp_ostr2->noctets;
		parm->part2.octets = tmp_ostr2->octets;
	}

	if (parm->part3.noctets > 0 && (parm->part3.octets[0] & MSBITINBYTE)){
		/* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr3 = aux_concatenate_OctetString(nullostr, &parm->part3);

		/* Save values of parm->part3 */
		save_part3.noctets = parm->part3.noctets;
		save_part3.octets = parm->part3.octets;
		parm->part3.noctets = tmp_ostr3->noctets;
		parm->part3.octets = tmp_ostr3->octets;
	}

	if (parm->part4.noctets > 0 && (parm->part4.octets[0] & MSBITINBYTE)){
		/* Most significant bit in most significant byte is 1. */
		/* Therefore, leading 0 byte is required to conform to ASN-1 integer encoding */
		tmp_ostr4 = aux_concatenate_OctetString(nullostr, &parm->part4);

		/* Save values of parm->part4 */
		save_part4.noctets = parm->part4.noctets;
		save_part4.octets = parm->part4.octets;
		parm->part4.noctets = tmp_ostr4->noctets;
		parm->part4.octets = tmp_ostr4->octets;
	}

	if (parm->part4.noctets > 0) 
		parm->choice = 1; /* DSA */
	else {
		parm->choice = 2; /* RSA or DH or DSA (without SEQUENCE) */
	}
	%} 
	%D{
        if (((*parm) = (KeyBits *) calloc (1, sizeof (KeyBits))) == ((KeyBits *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
	(*parm)->choice = 0;
        %} 
      
	SEQUENCE
	{
	part1       
		TOURINTEGER [[p &parm->part1]],

		CHOICE [[T KeyBits * $ *]] <E<parm->choice>><D<0>>
		{
			part234
				SEQUENCE [[T KeyBits * $ *]]
				{
				prime1      
					[0] TOURINTEGER [[p &parm->part2]],
				prime2      
					[1] TOURINTEGER [[p &parm->part3]],
				base        
					[2] TOURINTEGER [[p &parm->part4]]
				}
				%D{
				(*parm)->choice = 1;
				%},

			part2       
				TOURINTEGER [[p &parm->part2]]
		}
		OPTIONAL <E<parm->part2.noctets > 0>><D<0>>,

	part3       
		TOURINTEGER [[p &parm->part3]]
		OPTIONAL <E<parm->choice == 2 && parm->part3.noctets > 0>><D<(*parm)->choice != 1>>
	}

	%E{
	if(tmp_ostr1) {
		aux_free_OctetString(&tmp_ostr1);
		parm->part1.noctets = save_part1.noctets;
		parm->part1.octets = save_part1.octets;
	}
	if(tmp_ostr2) {
		aux_free_OctetString(&tmp_ostr2);
		parm->part2.noctets = save_part2.noctets;
		parm->part2.octets = save_part2.octets;
	}
	if(tmp_ostr3) {
		aux_free_OctetString(&tmp_ostr3);
		parm->part3.noctets = save_part3.noctets;
		parm->part3.octets = save_part3.octets;
	}
	if(tmp_ostr4) {
		aux_free_OctetString(&tmp_ostr4);
		parm->part4.noctets = save_part4.noctets;
		parm->part4.octets = save_part4.octets;
	}
	%}
	%D{
	if(! ((*parm)->part1.octets[0] | 0x00) ){    

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part1.noctets - 1; i++)
			(*parm)->part1.octets[i] = (*parm)->part1.octets[i + 1];
		(*parm)->part1.noctets -= 1;
		(*parm)->part1.octets = (char *)realloc((*parm)->part1.octets, (*parm)->part1.noctets);
	}

	if ((*parm)->part2.noctets > 0 && !((*parm)->part2.octets[0] | 0x00)){

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part2.noctets - 1; i++)
			(*parm)->part2.octets[i] = (*parm)->part2.octets[i + 1];
		(*parm)->part2.noctets -= 1;
		(*parm)->part2.octets = (char *)realloc((*parm)->part2.octets, (*parm)->part2.noctets);
	}

	if ((*parm)->part3.noctets > 0 && !((*parm)->part3.octets[0] | 0x00)){

		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part3.noctets - 1; i++)
			(*parm)->part3.octets[i] = (*parm)->part3.octets[i + 1];
		(*parm)->part3.noctets -= 1;
		(*parm)->part3.octets = (char *)realloc((*parm)->part3.octets, (*parm)->part3.noctets);
	}

	if ((*parm)->part4.noctets > 0 && !((*parm)->part4.octets[0] | 0x00)){
	
		/* The bits of the most significant byte are all 0, so delete them */
		for ( i = 0; i < (*parm)->part4.noctets - 1; i++)
			(*parm)->part4.octets[i] = (*parm)->part4.octets[i + 1];
		(*parm)->part4.noctets -= 1;
		(*parm)->part4.octets = (char *)realloc((*parm)->part4.octets, (*parm)->part4.noctets);
	}
	%}


KeyInfo [[P KeyInfo *]]
    	::=
        %D{
        if (((*parm) = (KeyInfo *) calloc (1, sizeof (KeyInfo))) == ((KeyInfo *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	SEQUENCE 
	{
	algorithm   
		AlgorithmIdentifier [[p subjectAI]],

	key         
		BIT STRING [[x subjectkey.bits $ subjectkey.nbits]]
	}


DigestInfo [[P DigestInfo *]]
    	::=
        %D{
        if (((*parm) = (DigestInfo *) calloc (1, sizeof (DigestInfo))) == ((DigestInfo *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	SEQUENCE 
	{
	digestai    
		AlgorithmIdentifier [[p digestAI]],

	digest      
		OCTET STRING [[o digest.octets $ digest.noctets]]
	}


EncryptedKey [[P EncryptedKey *]]
    	::=
        %D{
        if (((*parm) = (EncryptedKey *) calloc (1, sizeof (EncryptedKey))) == ((EncryptedKey *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %} 
	SEQUENCE 
	{
	encalg   
		AlgorithmIdentifier [[p encryptionAI]],

	subalg   
		AlgorithmIdentifier [[p subjectAI]],

	key         
		BIT STRING [[x subjectkey.bits $ subjectkey.nbits]]
	}


AlgorithmIdentifier [[P AlgId *]]
    	::=
        %D{
        if (((*parm) = (AlgId *) calloc (1, sizeof (AlgId))) == ((AlgId *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
        }
        %}
	SEQUENCE
	{
	objectid    
		OBJECT IDENTIFIER [[O objid]]
		-- for all algorithms true --
		%E{
	    	parm->paramchoice = aux_ObjId2ParmType(parm->objid);
	    	if (parm->paramchoice < 0){
            		advise (NULLCP, "parm->paramchoice has undefined value");
            		return NOTOK;
       		} 
		if (parm->paramchoice == PARM_INTEGER)
			parm->un.keyorblocksize = *((rsa_parm_type *)(parm->param));
		else if (parm->paramchoice == PARM_OctetString)
			parm->un.des_parm = (desCBC_parm_type *)(parm->param);
		else if (parm->paramchoice == PARM_KeyBits)
			parm->un.diffie_hellmann_parm = (KeyBits *)(parm->param);
		%}
		%D{
		(*parm)->param = CNULL;
		(*parm)->un.des_parm = (desCBC_parm_type *)0;
		(*parm)->un.diffie_hellmann_parm = (KeyBits *)0;
	    	(*parm)->paramchoice = aux_ObjId2ParmType((*parm)->objid);
	    	if ((*parm)->paramchoice == NoParmType){
            		advise (NULLCP, "parm->paramchoice has undefined value");
            		return NOTOK;
       		}
		%}
	,
	parameters  
		CHOICE [[T AlgId * $ *]] <E<parm->paramchoice>><D<(*parm)->paramchoice>>
		{
		keyorblocksize   
			INTEGER [[i un.keyorblocksize]]
			--- RSA or Hash Algorithms
     			%D{
		  	rsa_parm = (rsa_parm_type *)malloc(sizeof(rsa_parm_type));
		  	*rsa_parm = (*parm)->un.keyorblocksize;
		  	(*parm)->param = (char *)rsa_parm;
     			%},
		desIv     
			OctetString [[p un.des_parm]]
			--- DES Algorithms
     			%D{
		  	(*parm)->param = (char *)aux_cpy_OctetString((*parm)->un.des_parm);
		  	if((*parm)->un.des_parm)
				aux_free_OctetString(&((*parm)->un.des_parm));
    			%},

		diffhell
			KeyBits [[p un.diffie_hellmann_parm]]
			--- DIFFIE_HELLMAN Algorithm
     			%D{
		  	(*parm)->param = (char *)aux_cpy_KeyBits((*parm)->un.diffie_hellmann_parm);
		  	if((*parm)->un.diffie_hellmann_parm)
				aux_free_KeyBits(&((*parm)->un.diffie_hellmann_parm));
     			%},

		nullparm
		  	NULL     			

		} OPTIONAL <E<parm->paramchoice > 0>><D<(*parm)->paramchoice > 0>>
	}


SETOFAlgorithmIdentifier [[P SET_OF_AlgId *]]
	::=
        SET OF [[T SET_OF_AlgId * $ *]] <<next>>
		AlgorithmIdentifier [[p element]]


PSEObjects [[P struct PSE_Objects *]]
    	::=
	-- No DER rules applied, as never signed 
	SET OF [[T struct PSE_Objects * $ *]] <<next>>
		SEQUENCE [[ T struct PSE_Objects * $ *]] 
		{
		name       
			PrintableString [[s name]],
		create     
			UTCTime [[s create]],
		update     
			UTCTime [[s update]],
		noOctets   
			[0] INTEGER [[i noOctets]]
			OPTIONAL <E<parm->noOctets != 0>><D<0>>,
		status     
			[1] INTEGER [[i status]]
		        OPTIONAL <E<parm->status != 0>><D<0>>
		}


PSEToc [[P PSEToc *]]
    	::=
	SEQUENCE 
	{
	owner      
		PrintableString [[s owner]],
	create     
		UTCTime [[s create]],
	update     
		UTCTime [[s update]],
	status 
		INTEGER [[i status]]
		OPTIONAL <E<parm->status != 0>><D<0>>,
	sCObjects  
		PSEObjects [[p obj]]
		OPTIONAL <E<parm->obj != (struct PSE_Objects *)0>><D<0>>
	}


PSEObject [[P PSEObject *]]
    	::=
        %D{
        if (((*parm) = (PSEObject *) calloc (1, sizeof (PSEObject))) == ((PSEObject *) 0)) {
            	advise (NULLCP, "out of memory");
            	return NOTOK;
	}
        %}
	SEQUENCE 
	{
	type        
		OBJECT IDENTIFIER [[O objectType]]  
		-- for all algorithms true --
     		%E{
		parm->asn = aux_OctetString2PE(parm->objectValue);
     		%}
	    	,		
	value       
		ANY [[a asn]]
		%E{
		pe_free(parm->asn);
		%}
		%D{
		(*parm)->objectValue = aux_PE2OctetString((*parm)->asn);
		pe_free((*parm)->asn);
		%}
	}


END
