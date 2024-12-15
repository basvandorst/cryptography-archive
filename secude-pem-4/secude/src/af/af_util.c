/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include "af.h"



/****************************************************************************/

/***************************************************************
 *
 * Procedure af_get_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

RC af_get_EncryptedKey(
	EncryptedKey	 *encrypted_key,
	Key		 *plain_key,
	Key		 *encryption_key,
	DName		 *name,
	AlgId		 *encryption_key_algid
)

#else

RC af_get_EncryptedKey(
	encrypted_key,
	plain_key,
	encryption_key,
	name,
	encryption_key_algid
)
EncryptedKey	 *encrypted_key;
Key		 *plain_key;
Key		 *encryption_key;
DName		 *name;
AlgId		 *encryption_key_algid;

#endif

{
	Certificate * cert;
	Key           key;
	char	    * proc = "af_get_EncryptedKey";

	key.key = (KeyInfo * )0; 
	key.keyref = 0; 
	key.pse_sel = (PSESel * )0; 
	key.alg = (AlgId * )0;

	if ( encryption_key_algid && encryption_key_algid->objid && 
		(aux_ObjId2AlgType(encryption_key_algid->objid) != ASYM_ENC) ) {
		aux_add_error(EINVALID, "Invalid algid", (char *)encryption_key_algid, AlgId_n, proc);
		return(-1);
	}

	if ( !encryption_key && !name) {
		if ( !(cert = af_pse_get_Certificate(ENCRYPTION, NULLDNAME, 0)) ) {
			AUX_ADD_ERROR;
			return(-1);
		}
		if ( !(key.key = aux_cpy_KeyInfo(cert->tbs->subjectPK) )) {
			AUX_ADD_ERROR;
			return(-1);
		}

		if ( encryption_key_algid ) {
			if( aux_cmp_AlgId(key.key->subjectAI, encryption_key_algid) ) {
				aux_free_AlgId (&key.key->subjectAI);
				key.key->subjectAI = aux_cpy_AlgId (encryption_key_algid);
			}
		}

		if ( sec_get_EncryptedKey(encrypted_key, plain_key, &key) < 0 ) {
			AUX_ADD_ERROR;
			return(-1);
		}
	} else if ( !encryption_key && name ) {
		if ( !(key.key = af_pse_get_PK(ENCRYPTION, name, NULLDNAME, 0)) ) {
			if(aux_last_error() != ENONAME) AUX_ADD_ERROR;
			else aux_add_error(ENONAME, "Can't get PK for specified name", (char *) name, DName_n, proc);
			return(-1);
		}

		if ( encryption_key_algid ) {
			if( aux_cmp_AlgId(key.key->subjectAI, encryption_key_algid) ) {
				aux_free_AlgId (&key.key->subjectAI);
				key.key->subjectAI = aux_cpy_AlgId (encryption_key_algid);
			}
		}

		if ( sec_get_EncryptedKey(encrypted_key, plain_key, &key) < 0 ) {
			AUX_ADD_ERROR;
			return(-1);
		}
	} else if ( encryption_key ) {
		
		if ( encryption_key_algid ) {
			if( aux_cmp_AlgId(encryption_key->key->subjectAI, encryption_key_algid) ) {
				aux_free_AlgId (&(encryption_key->key->subjectAI));
				encryption_key->key->subjectAI = aux_cpy_AlgId (encryption_key_algid);
			}
		}

		if ( sec_get_EncryptedKey(encrypted_key, plain_key, encryption_key) < 0 ) {
			AUX_ADD_ERROR;
			return(-1);
		}
	}

	return(0);

}   /*af_get_EncryptedKey()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_put_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

RC af_put_EncryptedKey(
	EncryptedKey	 *encrypted_key,
	Key		 *plain_key,
	AlgId		 *decryption_key_algid
)

#else

RC af_put_EncryptedKey(
	encrypted_key,
	plain_key,
	decryption_key_algid
)
EncryptedKey	 *encrypted_key;
Key		 *plain_key;
AlgId		 *decryption_key_algid;

#endif

{
	PSESel 	             pse_sel;
	OctetString 	     content;
	ObjId 		     obj_type;
	Key 		     decryption_key;
	Boolean		     onekeypaironly = FALSE;

	char	           * proc = "af_put_EncryptedKey";

	if ( (!encrypted_key) || (!plain_key) ) {
		aux_add_error(EINVALID, "one parameter empty", CNULL, 0, proc);
		return(-1);
	}

	if ( decryption_key_algid && decryption_key_algid->objid &&
		(aux_ObjId2AlgType(decryption_key_algid->objid) != ASYM_ENC) ) {
		aux_add_error(EINVALID, "Invalid algid", (char *)decryption_key_algid, AlgId_n, proc);
		return(-1);
	}

	pse_sel.app_name = AF_pse.app_name;
	pse_sel.pin = AF_pse.pin;
	pse_sel.app_id = AF_pse.app_id;

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		AUX_ADD_ERROR;
		return (- 1);
	}

	if(onekeypaironly == TRUE){
		pse_sel.object.name = SKnew_name;
		pse_sel.object.pin = getobjectpin(SKnew_name);
		if(pse_sel.object.pin) setobjectpin(SKnew_name, pse_sel.object.pin);
	}
	else{
		pse_sel.object.name = DecSKnew_name;
		pse_sel.object.pin = getobjectpin(DecSKnew_name);
		if(pse_sel.object.pin) setobjectpin(DecSKnew_name, pse_sel.object.pin);
	}

	decryption_key.keyref = 0;
	decryption_key.key = (KeyInfo *)0;
	decryption_key.pse_sel = &pse_sel;
	decryption_key.alg = decryption_key_algid;

	
	if ( sec_put_EncryptedKey(encrypted_key, plain_key, &decryption_key, TRUE) < 0 ) {
		AUX_ADD_ERROR;
		return(-1);
	}

	return(0);

}   /*af_put_EncryptedKey()*/


/****************************************************************************/

/***************************************************************
 *
 * Procedure af_create_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *af_create_Certificate(
	KeyInfo	    *keyinfo,
	AlgId	    *sig_alg,
	char	    *obj_name,
	DName	    *subject,
	OctetString *serial
)

#else

Certificate *af_create_Certificate(
	keyinfo,
	sig_alg,
	obj_name,
	subject,
	serial
)
KeyInfo	    *keyinfo;
AlgId	    *sig_alg;
char	    *obj_name;
DName	    *subject;
OctetString *serial;

#endif

{
	int	      ret;
	FCPath      * fcpath;
	PKRoot      * pkroot;
	PSESel	      pse;
	Key           skey;         		        /* signkey refers to PSE Signature_Key */
	HashInput   * hashin;				/* hashinput pointer */
	Certificate * newcert;
	RC	      rcode;
	static OctetString null = {
		0, CNULL 	
	};
	char	*proc = "af_create_Certificate";

	if (!obj_name) {
		aux_add_error(EINVALID, "no object name specified", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}

	if(sig_alg && aux_ObjId2AlgType(sig_alg->objid) != SIG) {
		aux_add_error(EINVALID, "sig_alg does not specify a SIGNATURE algorithm", CNULL, 0, proc);
		return ((Certificate * ) 0);
	} 

	/* create prototype certificate */
	newcert = (Certificate * )malloc(sizeof(Certificate));
	if (!newcert) {
		aux_add_error(EMALLOC, "newcert", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}
	newcert->tbs = (ToBeSigned * )malloc(sizeof(ToBeSigned));
	if (!newcert->tbs) {
		aux_add_error(EMALLOC, "newcert->tbs", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}
	newcert->tbs->version = 0;
	if(serial) newcert->tbs->serialnumber = aux_cpy_OctetString(serial);
	else{
		newcert->tbs->serialnumber = aux_new_OctetString(1);
		newcert->tbs->serialnumber->octets[0] = 0x00;
	}
	newcert->tbs->signatureAI = aux_cpy_AlgId(keyinfo->subjectAI);
	if(!subject)
		newcert->tbs->subject = af_pse_get_Name();
	else
		newcert->tbs->subject = aux_cpy_DName(subject);
	newcert->tbs->issuer = aux_cpy_DName(newcert->tbs->subject);

	newcert->tbs->valid = (Validity * ) malloc (sizeof(Validity));
	if (!newcert->tbs->valid) {
		aux_add_error(EMALLOC, "newcert->tbs->valid", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}
	newcert->tbs->valid->notbefore = aux_current_UTCTime();
	newcert->tbs->valid->notafter = aux_delta_UTCTime(newcert->tbs->valid->notbefore);

	newcert->tbs->subjectPK = aux_cpy_KeyInfo(keyinfo);

#ifdef COSINE
	newcert->tbs->authatts = (AuthorisationAttributes * )malloc(sizeof(AuthorisationAttributes));
	if (!newcert->tbs->authatts) {
		aux_add_error(EMALLOC, "newcert->tbs->authatts", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}
	newcert->tbs->authatts->country = "\0";
	newcert->tbs->authatts->group = "\0";
	newcert->tbs->authatts->class = Normal;
#endif

	newcert->sig = (Signature * )calloc(1, sizeof(Signature));
	if (!newcert->sig) {
		aux_add_error(EMALLOC, "newcert->sig", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}

	skey.key = (KeyInfo *) 0;
	skey.keyref = 0;
	skey.alg = (AlgId *)0;
	skey.pse_sel = &pse;
	/* sign key resides on PSE */
	pse.app_name = AF_pse.app_name;
	pse.pin      = AF_pse.pin;

	pse.object.name = obj_name;
	pse.object.pin = getobjectpin(obj_name);

	rcode = sec_checkSK(&skey, keyinfo);
	if(rcode < 0){
 		AUX_ADD_ERROR;
		return ((Certificate * ) 0);
	}

	skey.alg = sig_alg;
	if(skey.alg) {
		if(aux_ObjId2AlgEnc(skey.alg->objid) != aux_ObjId2AlgEnc(keyinfo->subjectAI->objid)) {
 			aux_add_error(EINVALID, "sig_alg does not fit to key", CNULL, 0, proc);
			return ((Certificate * ) 0);
		}
	}
	else{
		switch(aux_ObjId2AlgEnc(keyinfo->subjectAI->objid)) {
			case RSA:
				skey.alg = md2WithRsaEncryption;
				break;
			case DSA:
				skey.alg = dsaWithSHA;
				break;
		}
	}
	/* now we have the inrended signature algorithm in skey.alg */

	newcert->tbs->signatureAI = aux_cpy_AlgId(skey.alg);
	if ((newcert->tbs_DERcode = e_ToBeSigned(newcert->tbs)) == NULLOCTETSTRING) {
		aux_add_error(EENCODE, "Can't encode TBS", CNULL, 0, proc);
		return ((Certificate * ) 0);
	}

	if (aux_ObjId2AlgHash(skey.alg->objid) == SQMODN)
		hashin = (HashInput *) & (newcert->tbs->subjectPK->subjectkey);
	else hashin = (HashInput * ) 0;

	ret = sec_sign(newcert->tbs_DERcode, newcert->sig, SEC_END, &skey, hashin);

	if (ret < 0) {
		AUX_ADD_ERROR;
		return ((Certificate * ) 0);
	}

	return(newcert);

}



/***************************************************************
 *
 * Procedure af_get_objoid
 *
 ***************************************************************/
#ifdef __STDC__

ObjId *af_get_objoid(
	char	 *objname
)

#else

ObjId *af_get_objoid(
	objname
)
char	 *objname;

#endif

{
	register int	i;

	for (i = 0; i < PSE_MAXOBJ; i++) 
		if (strcmp(objname, AF_pse.object[i].name) == 0) {
			return(aux_cpy_ObjId (AF_pse.object[i].oid));
		}
	return(aux_cpy_ObjId(Uid_OID));

}

#ifdef X500
extern DName * directory_user_dname;    /* defined in af_init.c */
#endif

#ifndef NULLCERTIFICATE
#define NULLCERTIFICATE (Certificate *)0
#endif


/************************************************************************************************/
/*		af_search_Certificate()								*/
/* 												*/
/* 												*/
/************************************************************************************************/
/***************************************************************
 *
 * Procedure af_search_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *af_search_Certificate(
	KeyType	  type,
	DName	 *dname
)

#else

Certificate *af_search_Certificate(
	type,
	dname
)
KeyType	  type;
DName	 *dname;

#endif

{
	ToBeSigned		*tbs;
	SET_OF_Certificate	*found_soc, *soc, *tmp_soc;
	Certificate             *certificate;
	ObjId  		        *coid;
	Boolean		         onekeypaironly = FALSE;
	char			*proc = "af_search_Certificate";

	char			afdb[0x100];

#ifdef X500
	AlgType			algtype;
#endif

	if(!dname || ((type != SIGNATURE) && (type != ENCRYPTION))) {
		aux_add_error(EINVALID, "dname or type wrong", 0, 0, proc);
		return(NULLCERTIFICATE);
	}

	tbs = af_pse_get_TBS(type, dname, 0, 0);
	if(tbs) {
		if(!(certificate = (Certificate *)calloc(1, sizeof(Certificate)))) return(NULLCERTIFICATE);
		certificate->tbs = tbs;
		return(certificate);
	}

	if(!af_access_directory) return(NULLCERTIFICATE);

	if(af_verbose) fprintf(stderr, "Accessing the .af-db directory entry of <%s> ... ", aux_DName2Name(dname));
	found_soc = af_afdb_retrieve_Certificate(dname, type);
	if(af_verbose) fprintf(stderr, "done.\n");
	if(found_soc) {
/* certificate should be verified here */
		certificate = aux_cpy_Certificate(found_soc->element);
		aux_free_CertificateSet(&found_soc);	
		return(certificate);
	}

#ifdef X500
	if (! directory_user_dname) directory_user_dname = af_pse_get_Name();
	strcpy(afdb, AFDBFILE);
	strcat(afdb, "X500");           				/* file = .af-db/X500	*/

	if(af_access_directory && open(afdb, O_RDONLY) > 0) {
		if(af_verbose) 
			fprintf(stderr, "Accessing the X.500 directory entry of <%s> ... ", aux_DName2Name(dname));
		found_soc = af_dir_retrieve_Certificate(dname, userCertificate);
		if(af_verbose) fprintf(stderr, "done.\n");
		if (!found_soc) {
			if(af_verbose) fprintf(stderr, "No certificate with owner = \"%s\" stored in X.500 directory\n", aux_DName2Name(dname));
			aux_add_error(LASTERROR, "Can't retrieve certificate from directory", 0, 0, proc);
			return(NULLCERTIFICATE);
		}
	}

	if(found_soc) {
/* certificate should be verified here */
		certificate = aux_cpy_Certificate(found_soc->element);
		aux_free_CertificateSet(&found_soc);	
		return(certificate);
	}
#endif

	aux_add_error(LASTERROR, "No certificate found", 0, 0, proc);
	return(NULLCERTIFICATE);
}




/************************************************************************************************/
/*		af_PKRoot2Protocert()								*/
/* 												*/
/* 												*/
/************************************************************************************************/


/***************************************************************
 *
 * Procedure af_PKRoot2Protocert
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *af_PKRoot2Protocert(
	PKRoot	 *pkroot
)

#else

Certificate *af_PKRoot2Protocert(
	pkroot
)
PKRoot	 *pkroot;

#endif

{
	Certificate             * cert;
	char			* proc = "af_PKRoot2Protocert";


	if(! pkroot || ! pkroot->newkey) {
		aux_add_error(EINVALID, "pkroot or pkroot->newkey", 0, 0, proc);
		return(NULLCERTIFICATE);
	}

	if(! pkroot->newkey->sig || ! pkroot->newkey->valid || ! pkroot->newkey->valid->notbefore || ! pkroot->newkey->valid->notafter) {
		aux_add_error(EINVALID, "pkroot cannot be transformed into protocert, as the relevant components are missing", 0, 0, proc);
		return (NULLCERTIFICATE);
	}

	if (!(cert = (Certificate * )malloc(sizeof(Certificate)))) {
		aux_add_error(EMALLOC, "cert", CNULL, 0, proc);
		return (NULLCERTIFICATE);
	}

	if (!(cert->tbs = (ToBeSigned * )malloc(sizeof(ToBeSigned)))) {
		aux_add_error(EMALLOC, "cert->tbs", CNULL, 0, proc);
		return (NULLCERTIFICATE);
	}

	cert->tbs->issuer = aux_cpy_DName(pkroot->ca);
	cert->tbs->subject = aux_cpy_DName(pkroot->ca);

	if (!(cert->tbs->valid = (Validity * )malloc(sizeof(Validity)))) {
		aux_add_error(EMALLOC, "cert->tbs->valid", CNULL, 0, proc);
		return (NULLCERTIFICATE);
	}
	cert->tbs->valid->notbefore = aux_cpy_Name(pkroot->newkey->valid->notbefore);
	cert->tbs->valid->notafter = aux_cpy_Name(pkroot->newkey->valid->notafter);

	cert->tbs->serialnumber = aux_cpy_OctetString(pkroot->newkey->serial);
	cert->tbs->version = pkroot->newkey->version;
	cert->tbs->subjectPK = aux_cpy_KeyInfo(pkroot->newkey->key);

	cert->sig = aux_cpy_Signature(pkroot->newkey->sig);

	cert->tbs->signatureAI = aux_cpy_AlgId(cert->sig->signAI);

	if ((cert->tbs_DERcode = e_ToBeSigned(cert->tbs)) == NULLOCTETSTRING) {
		aux_add_error(EENCODE, "Can't encode TBS", CNULL, 0, proc);
		return (NULLCERTIFICATE);
	}

	return(cert);
}


/***************************************************************
 *
 * Procedure af_OctetString2SignedFile
 *
 ***************************************************************/
#ifdef __STDC__

RC af_OctetString2SignedFile(
	char		 *file,
	OctetString	 *ostr,
	Boolean		  withcerts,
	AlgId		 *signai
)

#else

RC af_OctetString2SignedFile(
	file,
	ostr,
	withcerts,
	signai
)
char		 *file;
OctetString	 *ostr;
Boolean		  withcerts;
AlgId		 *signai;

#endif

{
	OctetString *encoded_signature, *encoded_certs;
	Certificates *certs;
	Signature sign_signature;
	char *sig, *ctf;

	char *proc = "af_OctetString2SignedFile";


	if(signai) sign_signature.signAI = signai;
	else sign_signature.signAI = md5WithRsaTimeDate;	/* default signature
						        	 * algorithm */

	if(withcerts) {

		if (!(certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME))) {
			AUX_ADD_ERROR;
			return(-1);
		}
		if (!(encoded_certs = e_Certificates(certs))) {
			aux_add_error(EENCODE, "Can't ASN.1-encode Certificates", CNULL, 0, proc);
			aux_free_Certificates(&certs);
			return(-1);
		}
		aux_free_Certificates(&certs);
	}
	
	if (af_sign(ostr, &sign_signature, SEC_END) < 0) {
		AUX_ADD_ERROR;
		aux_free_OctetString(&encoded_certs);
		return(-1);
	}

	encoded_signature = e_Signature(&sign_signature);
	if (!encoded_signature) {
		aux_add_error(EENCODE, "Can't ASN.1-encode signature", CNULL, 0, proc);
		aux_free_OctetString(&encoded_certs);
		free(sign_signature.signature.bits);
		return(-1);
	}
	free(sign_signature.signature.bits);

	if (aux_OctetString2file(ostr, file, 2) < 0) {
		aux_add_error(EWRITEFILE, "Can't write signed file", file, char_n, proc);
		aux_free_OctetString(&encoded_signature);
		aux_free_OctetString(&encoded_certs);
		return(-1);
	}
	sig = (char *) malloc(strlen(file) + 8);
	strcpy(sig, file);
	strcat(sig, ".sig");
	if (aux_OctetString2file(encoded_signature, sig, 2) < 0) {
		aux_add_error(EWRITEFILE, "Can't write signature file", sig, char_n, proc);
		aux_free_OctetString(&encoded_signature);
		aux_free_OctetString(&encoded_certs);
		free(sig);
		return(-1);
	}
	aux_free_OctetString(&encoded_signature);
	free(sig);

	if(withcerts) {
		ctf = (char *) malloc(strlen(file) + 8);
		strcpy(ctf, file);
		strcat(ctf, ".ctf");
		if (aux_OctetString2file(encoded_certs, ctf, 2) < 0) {
			aux_add_error(EWRITEFILE, "Can't write ctf file", ctf, char_n, proc);
			aux_free_OctetString(&encoded_certs);
			free(ctf);
			return(-1);
		}
		aux_free_OctetString(&encoded_certs);
		free(ctf);
	}
	return(0);
}

/***************************************************************
 *
 * Procedure af_SignedFile2OctetString
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *af_SignedFile2OctetString(
	char	 *file
)

#else

OctetString *af_SignedFile2OctetString(
	file
)
char	 *file;

#endif

{
	OctetString  * ostr, * encoded_signature, * encoded_certs;
	Certificates * certs = (Certificates * )0;
	Signature    * verify_signature;
	PSESel       * pse_sel;
	char         * sig, * ctf;

	char	    * proc = "af_SignedFile2OctetString";

/*
 *      	Open PSE which shall be used for signature verification
 */

	pse_sel = af_pse_open(NULLOBJID, FALSE);
	if(!pse_sel) {
		AUX_ADD_ERROR;
		return((OctetString *)0);
	}

	aux_free_PSESel(&pse_sel);


/*
 *      	read input file, signature etc. and verify
 */

	
	ostr = aux_file2OctetString(file);
	if (!ostr) {
		aux_add_error(EREADFILE, "Can't read signed file", file, char_n, proc);
		return((OctetString *)0);
	}

	sig = (char *) malloc(strlen(file) + 8);
	strcpy(sig, file);
	strcat(sig, ".sig");

	encoded_signature = aux_file2OctetString(sig);
	free(sig);
	if (!encoded_signature) {
		aux_add_error(EREADFILE, "Can't read signature file", CNULL, 0, proc);
		aux_free_OctetString(&ostr);
		return((OctetString *)0);
	}
	verify_signature = d_Signature(encoded_signature);
	aux_free_OctetString(&encoded_signature);
	if (!verify_signature) {
		aux_add_error(EDECODE, "Can't ASN.1-decode signature", CNULL, 0, proc);
		aux_free_OctetString(&ostr);
		return((OctetString *)0);
	}

	ctf = (char *) malloc(strlen(file) + 8);
	strcpy(ctf, file);
	strcat(ctf, ".ctf");
	if ((encoded_certs = aux_file2OctetString(ctf))) {
		if (!(certs = d_Certificates(encoded_certs))) {
			aux_add_error(EDECODE, "Can't ASN.1-decode ctf", CNULL, 0, proc);
			free(ctf);
			aux_free_OctetString(&ostr);
			aux_free_OctetString(&encoded_certs);
			aux_free_Signature(&verify_signature);
			return((OctetString *)0);
		}
		aux_free_OctetString(&encoded_certs);
	}
	free(ctf);

	if(!certs) {

		/* no .ctf file, read Certificates from PSE */

		certs = (Certificates *)calloc(1, sizeof(Certificates));
		if(!certs) return(NULLOCTETSTRING);
		certs->usercertificate = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
	}
	
	if(af_verify(ostr, verify_signature, SEC_END, certs, (UTCTime *) 0, (PKRoot *)0) < 0) {
		if(aux_last_error() == EVERIFICATION) aux_add_error(EVERIFICATION, "Can't verify file", file, char_n, proc);
		else AUX_ADD_ERROR;
		if(certs) aux_free_Certificates(&certs);
		aux_free_OctetString(&ostr);
		aux_free_Signature(&verify_signature);
		return((OctetString *)0);
	}	
	if(certs) aux_free_Certificates(&certs);
	aux_free_Signature(&verify_signature);

	return(ostr);
}
