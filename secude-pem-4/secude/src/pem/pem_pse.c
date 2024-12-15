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

/*-----------------------pem_pse.c---------------------------------*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institut fuer TeleKooperationsTechnik (I2)         */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990 / "SecuDe" 1991,92,93                */
/* 	Grimm/Nausester/Schneider/Viebeg/Vollmer/                   */
/* 	Surkau/Reichelt/Kolletzki                     et alii       */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PACKAGE   pem             VERSION   3.0                          */
/*                              DATE   01.04.1994                   */
/*                                BY   Surkau                       */
/*                                                                  */
/*                            REVIEW                                */
/*                              DATE                                */
/*                                BY                                */
/* DESCRIPTION                                                      */
/*   This modul presents all pem_ functions using                   */
/*   the PSE, the CA database, the alias list or                    */
/*   the directory                                                  */
/*                                                                  */
/* EXPORT                                                           */
/*                                                                  */
/*  pem_pse_open()                                                  */
/*  pem_get_Certificate()                                           */
/*  pem_fill_Certificate()                                          */
/*  pem_look_for_Certificate()                                      */
/*  pem_accept_certification_reply()                                */
/*  pem_pse_store_crlset()                                          */
/*  pem_cadb_store_crlset()                                         */
/*  pem_reply_crl_request()                                         */
/*  pem_certify()                                                   */
/*  pem_validate()                                                  */
/*  pem_validateSet()                                               */
/*  pem_reply_crl_requestSet()                                      */
/*  pem_crl()                                                       */
/*  pem_crl_rr()                                                    */
/*  pem_enhance()                                                   */
/*  pem_check_name()                                                */
/*                                                                  */
/* CALLS TO                                                         */
/*                                                                  */
/*  aux_ functions                                                  */
/*  e_ functions                                                    */
/*  d_ functions                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
#include "pem.h"
#include "cadb.h"

static char *cadir = CNULL;

#define get_cadir(pse) cadir

/***************************************************************
 *
 * Procedure pem_new_encryption_key
 *
 ***************************************************************/
#ifdef __STDC__

static Key *pem_new_encryption_key(
)

#else

static Key *pem_new_encryption_key(
)

#endif
{
	char 			*proc = "pem_new_encryption_key";
	Key 			*encryptKEY;

	NEWSTRUCT(encryptKEY, Key, (Key *)0, ;)

	NEWSTRUCT(encryptKEY->key, KeyInfo, (Key *)0, free(encryptKEY))

	if(!(encryptKEY->key->subjectAI = aux_Name2AlgId(MSG_ENC_ALG)))
			AUX_ERROR_RETURN((Key *)0, aux_free_Key(&encryptKEY))

	/* place generated DES-key into encryptKEY->key: */
	if (sec_gen_key(encryptKEY, TRUE) < 0) AUX_ERROR_RETURN((Key *)0, aux_free_Key(&encryptKEY))

        /* random initial vector for the DEK */
        encryptKEY->key->subjectAI->param = (char *)sec_random_ostr(8);

	return(encryptKEY);
}
/***************************************************************
 *
 * Procedure pem_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

static RC pem_encrypt(
	PemMessageLocal  *local,
	Key		 *key
)

#else

static RC pem_encrypt(
	local,
	key
)
PemMessageLocal	 *local;
Key		 *key;

#endif

{
	char * proc = "pem_encrypt";

	OctetString sig_to_encrypt;
	BitString encrypted_body;

	if(local->header->mic_fields->mic->nbits){

		sig_to_encrypt.noctets = ( local->header->mic_fields->mic->nbits + 7 ) / 8;
		sig_to_encrypt.octets = local->header->mic_fields->mic->bits;
	
		NEWSTRING(local->header->mic_fields->mic->bits, sig_to_encrypt.noctets + 8, -1, ;);
		local->header->mic_fields->mic->nbits = 0;
	
		if ( af_encrypt(&sig_to_encrypt, local->header->mic_fields->mic, SEC_END, key, (DName *)0 ) < 0 ) 
			AUX_ERROR_RETURN(-1,  free(sig_to_encrypt.octets))
	
		if(sig_to_encrypt.noctets) free(sig_to_encrypt.octets);
	
	}

	if(local->body->noctets){
	
	
	
		NEWSTRING(encrypted_body.bits, local->body->noctets + 8, -1,  )
		encrypted_body.nbits = 0;
	
		if ( af_encrypt(local->body, &encrypted_body, SEC_END, key, (DName *)0 ) < 0 ) 
			AUX_ERROR_RETURN(-1, free(encrypted_body.bits))
	
		if(local->body->noctets) free(local->body->octets);
	
		local->body->noctets = (encrypted_body.nbits + 7) / 8;
		local->body->octets = encrypted_body.bits;
	
	}

	return(0);
}

/***************************************************************
 *
 * Procedure pem_decrypt
 *
 ***************************************************************/
#ifdef __STDC__

static RC pem_decrypt(
	PemMessageLocal  *local,
	Key		 *key
)

#else

static RC pem_decrypt(
	local,
	key
)
PemMessageLocal	 *local;
Key		 *key;

#endif

{
	char * proc = "pem_decrypt";

	OctetString decrypted_sig;
	BitString body_to_decrypt;

	if(local->header->mic_fields->mic->nbits){

		NEWSTRING(decrypted_sig.octets, local->header->mic_fields->mic->nbits/8 + 8, -1,  )
		decrypted_sig.noctets = 0;
	
		if ( af_decrypt(local->header->mic_fields->mic, &decrypted_sig, SEC_END, key ) < 0 ) 
			AUX_ERROR_RETURN(-1,  free(decrypted_sig.octets))
	
		if(local->header->mic_fields->mic->nbits) free(local->header->mic_fields->mic->bits);
	
		local->header->mic_fields->mic->bits = decrypted_sig.octets;
		local->header->mic_fields->mic->nbits = decrypted_sig.noctets * 8;
	}

	if(local->body->noctets){
	
		body_to_decrypt.nbits= local->body->noctets * 8;
		body_to_decrypt.bits = local->body->octets;
	
		NEWSTRING(local->body->octets, local->body->noctets + 8, -1, ;);
		local->body->noctets = 0;
	
		if ( af_decrypt(&body_to_decrypt, local->body, SEC_END, key ) < 0 ) 
			AUX_ERROR_RETURN(-1, free(body_to_decrypt.bits))
	
		if(body_to_decrypt.nbits) free(body_to_decrypt.bits);
	}
	
	return(0);
}


/***************************************************************
 *
 * Procedure pem_get_decryption_key
 *
 ***************************************************************/
#ifdef __STDC__

static Key *pem_get_decryption_key(
	int		  pse,
	PemMessageLocal  *local
)

#else

static Key *pem_get_decryption_key(
	pse,
	local
)
int		 *pse;
PemMessageLocal	 *local;

#endif

{
	char 			*proc = "pem_get_decryption_key";

	Certificate		*mycert;
	EncryptedKey		 encrypted_key;
	Key 			*encryptKEY;
	SET_OF_PemRecLocal 	*local_recs;

	mycert = af_pse_get_Certificate(ENCRYPTION, 0, 0);

	if(!(encrypted_key.subjectAI = aux_cpy_AlgId(local->header->dek_fields->dekinfo_enc_alg))) AUX_ERROR_RETURN((Key *)0, ;);

	if(!aux_cmp_DName(mycert->tbs->issuer, local->header->certificates->usercertificate->tbs->issuer) &&
		!aux_cmp_OctetString(mycert->tbs->serialnumber, local->header->certificates->usercertificate->tbs->serialnumber)) {


		/* Was signed by you */
		


		if(!(encrypted_key.encryptionAI = aux_cpy_AlgId(local->header->dek_fields->keyinfo_enc_alg))) {
		}

		aux_cpy2_BitString(&encrypted_key.subjectkey, local->header->dek_fields->keyinfo_dek);

	} 
	else {
		for(local_recs = local->header->dek_fields->recipients; local_recs; local_recs = local_recs->next)
			if(!aux_cmp_DName(mycert->tbs->issuer, local_recs->element->certificate->tbs->issuer) &&
			    !aux_cmp_OctetString(mycert->tbs->serialnumber, 
				local_recs->element->certificate->tbs->serialnumber)) break;
		if(local_recs) {

			if(!(encrypted_key.encryptionAI = aux_cpy_AlgId(local_recs->element->enc_alg))) {
			}

			aux_cpy2_BitString(&encrypted_key.subjectkey, local_recs->element->dek);

		}
		else {
			aux_add_error(ENONAME, "You are not on the recipients list", CNULL, 0, proc);
			return((Key *)0);

		}
		
	}
	NEWSTRUCT(encryptKEY, Key, (Key *)0, ;);

	if(af_put_EncryptedKey(&encrypted_key, encryptKEY, encrypted_key.encryptionAI))
			AUX_ERROR_RETURN((Key *)0, ;);

	return(encryptKEY);
}
/*------------------------------------------------------------------
  pem_pse_open() opens the PSE with the given path. In case of a CA
  the ca directory must be set for database operations. If the PSE 
  is no SC PSE, pin must be the correct PIN.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_pse_open
 *
 ***************************************************************/
#ifdef __STDC__

int pem_pse_open(
	char	 *ca_dir,
	char	 *psepath,
	char	 *pin
)

#else

int pem_pse_open(
	ca_dir,
	psepath,
	pin
)
char	 *ca_dir;
char	 *psepath;
char	 *pin;

#endif

{
	PSESel 	*std_pse;
	int 	 i;
	char	*proc = "pem_pse_open",
		*afdb,
		*af_dir_env_authlevel;
	Boolean  x500;


	cadir = aux_cpy_String(ca_dir);

	AF_pse.app_name = psepath;

        if ( aux_create_AFPSESel(psepath, pin) < 0 ) AUX_ERROR_RETURN(-1, ;)


	if (!(std_pse = af_pse_open((ObjId *)0, FALSE))) AUX_ERROR_RETURN(-1, ;)

	aux_free_PSESel(&std_pse);
	for (i = 0; i < PSE_MAXOBJ; i++) AF_pse.object[i].pin = aux_cpy_String(AF_pse.pin);





#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	if(!(afdb = CATSPRINTF(CNULL, "%s%s", AFDBFILE, X500_name))) AUX_MALLOC_ERROR(-1, ;)           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif

#ifdef X500
	if (af_dir_authlevel == DBA_AUTH_NONE) {
		af_dir_env_authlevel = getenv("AUTHLEVEL");
		if (af_dir_env_authlevel) {
			if (! strcasecmp(af_dir_env_authlevel, "STRONG"))
				af_dir_authlevel = DBA_AUTH_STRONG;
			else if (! strcasecmp(af_dir_env_authlevel, "SIMPLE"))
				af_dir_authlevel = DBA_AUTH_SIMPLE;
		}
	}
	if (x500 && af_access_directory == TRUE) directory_user_dname = af_pse_get_Name();

#endif




	return(1);
}
/***************************************************************
 *
 * Procedure pem_pse_close
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_pse_close(
	int	  pse
)

#else

RC pem_pse_close(
	pse
)
int	  pse;

#endif

{
	af_pse_close((ObjId *)0);
}
/*------------------------------------------------------------------
  pem_get_Certificate() searches for a certificate of the
  specified type. The subject name or an issuer - serial number
  combination must be given in TBS components of the given
  certificate. the found certificate is returned.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_get_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

Certificate *pem_get_Certificate(
	int		  pse,
	KeyType		  type,
	Certificate	 *certificate
)

#else

Certificate *pem_get_Certificate(
	pse,
	type,
	certificate
)
int		  pse;
KeyType		  type;
Certificate	 *certificate;

#endif

{
	ToBeSigned 	*tbs;
	Certificate 	*cert;
	char		*proc = "pem_get_Certificate";

	if(!certificate || !certificate->tbs) {
		aux_add_error(EINVALID, "No TBS given", CNULL, 0, proc);
		return((Certificate   *)0);
	}
	if(certificate->sig) {
		return(aux_cpy_Certificate(certificate));
	}
	if(certificate->tbs->issuer && certificate->tbs->serialnumber) {
		if((tbs = af_pse_get_TBS(type, 0, certificate->tbs->issuer, certificate->tbs->serialnumber))) {
			NEWSTRUCT(cert, Certificate, (Certificate   *)0, aux_free_ToBeSigned(&tbs))
			cert->tbs = tbs;
			return(cert);
		}
		if((cert = af_pse_get_Certificate(type, NULLDNAME, 0))) {
			if(!cert->tbs || aux_cmp_DName(certificate->tbs->issuer, cert->tbs->issuer)
					|| aux_cmp_OctetString(certificate->tbs->serialnumber, cert->tbs->serialnumber)) {
				aux_free_Certificate(&cert);
				return((Certificate   *)0);
			}
			return(cert);
		}
		return((Certificate   *)0);
	}
	if(certificate->tbs->subject) {
		if((cert = af_search_Certificate(type, certificate->tbs->subject))) return(cert);
		return((Certificate   *)0);
	}

}

/*------------------------------------------------------------------
  pem_fill_Certificate() searches for a certificate of the
  specified type. The subject name or an issuer - serial number
  combination must be given in TBS components of the given
  certificate which is replaced by the found one.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_fill_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_fill_Certificate(
	int		  pse,
	KeyType		  type,
	Certificate	 *certificate
)

#else

RC pem_fill_Certificate(
	pse,
	type,
	certificate
)
int		  pse;
KeyType		  type;
Certificate	 *certificate;

#endif

{
	ToBeSigned 	*tbs;
	Certificate 	*cert;
	char		*proc = "pem_fill_Certificate";

	if(!certificate || !certificate->tbs) {
		aux_add_error(EINVALID, "No TBS given", CNULL, 0, proc);
		return(-1);
	}
	if(certificate->sig) {
		return(1);
	}
	if(certificate->tbs->issuer && certificate->tbs->serialnumber) {
		if((tbs = af_pse_get_TBS(type, 0, certificate->tbs->issuer, certificate->tbs->serialnumber))) {
			aux_free_ToBeSigned(&certificate->tbs);
			certificate->tbs = tbs;
			return(1);
		}
		if((cert = af_pse_get_Certificate(type, NULLDNAME, 0))) {
			if(cert->tbs && !aux_cmp_DName(certificate->tbs->issuer, cert->tbs->issuer)
					&& !aux_cmp_OctetString(certificate->tbs->serialnumber, cert->tbs->serialnumber)) {

				aux_free_OctetString(&certificate->tbs_DERcode);
				aux_free_ToBeSigned(&certificate->tbs);
				certificate->tbs_DERcode = cert->tbs_DERcode;
				certificate->tbs = cert->tbs;
				certificate->sig = cert->sig;
				free(cert);

				return(1);
			}
			else aux_free_Certificate(&cert);
		}
	}
	if(certificate->tbs->subject) {
		if((cert = af_search_Certificate(type, certificate->tbs->subject))){
			aux_free_OctetString(&certificate->tbs_DERcode);
			aux_free_ToBeSigned(&certificate->tbs);
			certificate->tbs_DERcode = cert->tbs_DERcode;
			certificate->tbs = cert->tbs;
			certificate->sig = cert->sig;
			free(cert);

			return(1);
		}
	}
	return(0);

}
/*------------------------------------------------------------------
  pem_look_for_Certificate() searches on your PSE for a certificate of the
  specified type. The subject name or an issuer - serial number
  combination must be given in TBS components of the given
  certificate. The return value gives information, where
  the certificate was found.
    -1 : ERROR
     0 : Not found
     1 : A TBS to this certificate found in PKList
     2 : The given certificate is your own certificate
     3 : The given certificate is a signed version of your own prototype certificate
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_look_for_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_look_for_Certificate(
	int		  pse,
	KeyType		  type,
	Certificate	 *certificate
)

#else

RC pem_look_for_Certificate(
	pse,
	type,
	certificate
)
int		  pse;
KeyType		  type;
Certificate	 *certificate;

#endif

{
	ToBeSigned 	*tbs;
	Certificate 	*cert;
	RC		 rc = 0;
	char		*proc = "pem_look_for_Certificate";

	if(!certificate || !certificate->tbs) {
		aux_add_error(EINVALID, "No TBS given", CNULL, 0, proc);
		return(-1);
	}
	if(!certificate->tbs->issuer || !certificate->tbs->issuer || !certificate->tbs->serialnumber) {
		aux_add_error(EINVALID, "No subject, issuer or serialnumber given", CNULL, 0, proc);
		return(-1);
	}
	if((cert = af_pse_get_Certificate(type, NULLDNAME, 0))) {
		if(!aux_cmp_DName(certificate->tbs->subject, cert->tbs->subject)) {

			if(!aux_cmp_DName(certificate->tbs->issuer, cert->tbs->issuer)
					&& !aux_cmp_OctetString(certificate->tbs->serialnumber, cert->tbs->serialnumber)) 
				rc = 2;
			
			else if(!aux_cmp_DName(cert->tbs->subject, cert->tbs->issuer) && 
				aux_cmp_BitString(&(certificate->sig->signature), &(cert->sig->signature)) &&
				!sec_checkSK(PSEobj(SKnew_name), certificate->tbs->subjectPK)) 
					rc = 3;
			
		}
		aux_free_Certificate(&cert);
		if(rc) return(rc);

	}
	if((tbs = af_pse_get_TBS(type, 0, certificate->tbs->issuer, certificate->tbs->serialnumber))) {
		aux_free_ToBeSigned(&tbs);
		return(1);
	}
	return(0);

}
/*------------------------------------------------------------------
  pem_accept_certification_reply() checks if the certificates of the given
  Message represents a certification reply for the own prototype
  certificate and updates the PSE objects
  Cert, FCPath and PKRoot.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_accept_certification_reply
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_accept_certification_reply(
	int		  pse,
	PemMessageLocal	 *local
)

#else

RC pem_accept_certification_reply(
	pse,
	local
)
int		  pse;
PemMessageLocal	 *local;

#endif

{

	Certificate 	*cert;
	FCPath 		*path;
	PKRoot 		*pkroot;
	char		*proc = "pem_accept_certification_reply";

	if(!local || !local->header || !local->header->certificates)  {
		aux_add_error(EINVALID, "No Certificates given", CNULL, 0, proc);
		return(-1);
	}
	
	cert = local->header->certificates->usercertificate;


	if(!cert || !cert->sig || !cert->tbs)  {
		aux_add_error(EINVALID, "No user certificate given", CNULL, 0, proc);
		return(-1);
	}


	if (af_pse_update_Certificate(SIGNATURE, cert, TRUE) < 0) AUX_ERROR_RETURN(-1, ;) 
		
	path = local->header->certificates->forwardpath;
	if(path) 	if (af_pse_update_FCPath(path) < 0) AUX_ERROR_RETURN(-1, ;) 

	if(!(pkroot = aux_create_PKRoot(local->header->root_certificate, (Certificate *)0))) AUX_ERROR_RETURN(-1, ;) 
	if (af_pse_update_PKRoot(pkroot)< 0) AUX_ERROR_RETURN(-1, ;) 
	

	return(0);
}

/*------------------------------------------------------------------
  pem_pse_store_crlset() stores the CRL components in the
  PSE object CrlSet.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_pse_store_crlset
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_pse_store_crlset(
	int				  pse,
	SET_OF_CRLWithCertificates	 *set_of_pemcrlwithcerts
)

#else

RC pem_pse_store_crlset(
	pse,
	set_of_pemcrlwithcerts
)
int				  pse;
SET_OF_CRLWithCertificates	 *set_of_pemcrlwithcerts;

#endif

{
	Crl			 crlpse;
        SEQUENCE_OF_CRLEntry   	*revokedCertificates,
				*revcerts;

	char 			*proc = "pem_pse_store_crlset";

	for(; set_of_pemcrlwithcerts; set_of_pemcrlwithcerts = set_of_pemcrlwithcerts->next){

		if(!set_of_pemcrlwithcerts->element || !set_of_pemcrlwithcerts->element->crl ||
			!set_of_pemcrlwithcerts->element->crl->tbs) AUX_ERROR_RETURN(-1, ;)

		crlpse.issuer = set_of_pemcrlwithcerts->element->crl->tbs->issuer;
		crlpse.nextUpdate = set_of_pemcrlwithcerts->element->crl->tbs->nextUpdate;
		crlpse.revcerts = (SEQUENCE_OF_CRLEntry * )0;
		for (revokedCertificates = set_of_pemcrlwithcerts->element->crl->tbs->revokedCertificates; 
				revokedCertificates; revokedCertificates = revokedCertificates->next) {

			revcerts = (SEQUENCE_OF_CRLEntry *)calloc(1, sizeof(SEQUENCE_OF_CRLEntry));
			revcerts->next = crlpse.revcerts;
			revcerts->element = aux_cpy_CRLEntry(revokedCertificates->element);
			crlpse.revcerts = revcerts;
		}
		if(af_pse_add_CRL(&crlpse) < 0){
			if(pem_verbose_1) 
				fprintf(stderr, "Cannot add PEM CRL to PSE object CrlSet\n");
			AUX_ADD_ERROR;
			return(-1); 
		}
		if(pem_verbose_1) fprintf(stderr, "PEM CRL added to PSE object CrlSet\n");
		while (crlpse.revcerts) {
			revcerts = crlpse.revcerts;
			crlpse.revcerts = crlpse.revcerts->next;
			free(revcerts);
		}
	} 
	return(0);

}

/*------------------------------------------------------------------
  pem_cadb_store_crlset() stores the signed CRLs  in the
  CA database.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_cadb_store_crlset
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_cadb_store_crlset(
	int				  pse,
	SET_OF_CRLWithCertificates	 *set_of_pemcrlwithcerts
)

#else

RC pem_cadb_store_crlset(
	pse,
	set_of_pemcrlwithcerts
)
int				  pse;
SET_OF_CRLWithCertificates	 *set_of_pemcrlwithcerts;

#endif

{

	char 			*proc = "pem_cadb_store_crlset",
				*logpath;
	if(!get_cadir(pse)){
		aux_add_error(EINVALID, "No CA directory specified", CNULL, 0, proc);
		return (-1);
	}

	for(; set_of_pemcrlwithcerts; set_of_pemcrlwithcerts = set_of_pemcrlwithcerts->next){


		if(!(logpath = aux_get_abs_path(get_cadir(pse), CALOG))) AUX_ERROR_RETURN(-1, ;);

		if ((logfile = fopen(logpath, LOGFLAGS)) == (FILE *) 0) {
			aux_add_error(ELOGFILE, "Can't open log file", (char *) CALOG, char_n, proc);
			free(logpath);
			return (-1);
		}
		free(logpath);

		for(; set_of_pemcrlwithcerts; set_of_pemcrlwithcerts = set_of_pemcrlwithcerts->next){
			if(af_cadb_add_CRLWithCertificates(set_of_pemcrlwithcerts->element, get_cadir(pse)) < 0){
				LOGERR("can't access CA database");
				AUX_ADD_ERROR;
				return (-1);
			}
		}
		fclose(logfile);
	} 
	return(0);

}

/*------------------------------------------------------------------
  pem_reply_crl_request() creates a CRL message according to
  the given CRL-RETRIEVAL-REQUEST message.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_reply_crl_request
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_reply_crl_request(
	int		  pse,
	PemMessageLocal	 *local_mess
)

#else

OctetString *pem_reply_crl_request(
	pse,
	local_mess
)
int		  pse;
PemMessageLocal	 *local_mess;

#endif

{
	PemHeaderLocal 	*hd;
	PemMessageLocal *local,
			*local_crl;
	PemMessageCanon *canon_crl;
	SET_OF_NAME 	*names = 0,
			*names2;
	SET_OF_DName 	*issuers;
	OctetString	*clear_crl = 0;
	char		*proc = "pem_reply_crl_request";


	if(!local_mess) {
		aux_add_error(EINVALID, "no CRL-RR message", CNULL, 0, proc);
		return((OctetString *)0);
	}

	hd = local_mess->header;

	if(hd && hd->proctype == PEM_CRL_RETRIEVAL_REQUEST) {

		for(issuers = hd->crl_rr_fields; issuers; issuers = issuers->next) if(issuers->element) {
		
			NEWSTRUCT(names2, SET_OF_NAME, (OctetString *)0, ;)
			NEWSTRUCT(names2->element, NAME, (OctetString *)0, ;)
			names2->next = names;
			if(!(names2->element->dname = aux_cpy_DName(issuers->element))) AUX_ERROR_RETURN((OctetString *)0, ;)
			names = names2;

		}
		if(!(local_crl = pem_crl(pse, names))) AUX_ERROR_RETURN((OctetString *)0, ;)
		aux_free_SET_OF_NAME(&names);

		if(!(canon_crl = pem_Local2Canon(local_crl))) AUX_ERROR_RETURN((OctetString *)0, ;)
		aux_free_PemMessageLocal(&local_crl);

		if(!(clear_crl = pem_build_one(canon_crl))) AUX_ERROR_RETURN((OctetString *)0, ;)
		aux_free_PemMessageCanon(&canon_crl);


	}
	else {
		aux_add_error(EINVALID, "no CRL-RR message", CNULL, 0, proc);
		return((OctetString *)0);
	}

	return(clear_crl);

}
/*------------------------------------------------------------------
  pem_certify() makes a copy of the given message,
  signs the user certificate and adds FCPath and PKRoot.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_certify
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_certify(
	int		  pse,
	PemMessageLocal	 *local_mess
)

#else

PemMessageLocal *pem_certify(
	pse,
	local_mess
)
int		  pse;
PemMessageLocal	 *local_mess;

#endif

{
	PemHeaderLocal 		*hd;
	FCPath 			*received_fcpath,
				*ordered_fcpath,
				*actual;
	Certificate 		*mycert = (Certificate *)0,
				*cert,
				*protocert,
				*top_cert;
	SET_OF_Certificate 	*soc;
	FCPath 			*fcp;
	EncryptedKey		 encrypted_key;
	Key 			*encryptKEY,
				 top_key,
				 signkey;
	SET_OF_PemRecLocal 	*local_recs;
	BitString 	 	 decr_bstr;
	OctetString 		 ostr,
				 body,
				*dc_body,
				*c_body;
	Signature 		 signature; 
	int 			 index;
	PKRoot			*pkroot = (PKRoot *)0;
	PemMessageLocal 	*local;
	AlgId           	*algorithm = DEF_ISSUER_ALGID;
	char			*proc = "pem_certify",
				*logpath,
 				*printrepr;

	if(!get_cadir(pse)){
		aux_add_error(EINVALID, "No CA directory specified", CNULL, 0, proc);
		return ((PemMessageLocal *)0);
	}
	if(!local_mess) return((PemMessageLocal *)0);

	if(!(local = aux_cpy_PemMessageLocal(local_mess))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);

	hd = local->header;

	if(!hd || hd->proctype == PEM_CRL_RETRIEVAL_REQUEST || hd->proctype == PEM_CRL_RETRIEVAL_REQUEST) {
		aux_free_PemHeaderLocal(&local->header);
		return(local);
	}

	if(hd->proctype == PEM_ENC) {

		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Getting encrypted key\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)


		if(!(encryptKEY = pem_get_decryption_key(pse, local))) AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local););
		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Decrypting body and MIC info\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)


		if(pem_decrypt(local, encryptKEY) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local););
		aux_free_Key(&encryptKEY);
	}

	aux_cpy2_BitString(&signature.signature, hd->mic_fields->mic);
	signature.signAI = aux_cpy_AlgId(hd->mic_fields->signAI);


	if(pem_verbose_1) 
	if(!(local->comment = CATSPRINTF(local->comment, "Canonicalize body\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

	if(!(dc_body = aux_decanon(local->body)))  AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local););
	aux_free_OctetString(&local->body);
	local->body = dc_body;


	if(!(c_body = aux_canon(local->body))) AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local););




	if(hd->root_certificate) top_cert = hd->root_certificate;
	else if(!hd->certificates->forwardpath && hd->certificates->usercertificate && hd->certificates->usercertificate->tbs &&
				hd->certificates->usercertificate->sig &&
			!aux_cmp_DName(hd->certificates->usercertificate->tbs->subject, 
					hd->certificates->usercertificate->tbs->issuer))
				top_cert = hd->certificates->usercertificate;
	else top_cert = 0;

	if(pem_verbose_1) 
	if(!(local->comment = CATSPRINTF(local->comment, "Verifiing certificates\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

	if(hd->certificates->usercertificate && !hd->certificates->usercertificate->sig) {
			aux_add_error(ENONAME, "Originator Certificate not found", CNULL, 0, proc);
			return((PemMessageLocal *)0);
	}
	else if(af_verify(c_body, &signature, SEC_END, hd->certificates, 0, pkroot)) {
		if(verifresult)
		switch(verifresult->trustedKey){
			case 0:
			case 1:
			case 2:
			case 3:
				index = 0;
				while(verifresult->verifstep[index]) index++;
				index--;
				switch(verifresult->verifstep[index]->crlcheck) {
					case REVOKED:
						aux_add_error(EMIC, "MIC not validated (REVOKED certificate encountered)", 0, 0, proc);
						break;
					case CRL_NOT_AVAILABLE:
						aux_add_error(EMIC, "MIC not validated (Couldn't check all CRLs)", 0, 0, proc);
						break;
					case CRL_OUT_OF_DATE:
						aux_add_error(EMIC, "MIC not validated (Expired CRL encountered)", 0, 0, proc);
						break;
					default:
						AUX_ADD_ERROR;
				}
/*				aux_add_error(EMIC, "MIC not validated (Verification failure)", 0, 0, proc); */
				break;
			case -2:
				aux_add_error(EMIC, "MIC not validated (EXPIRED certificate encountered)", 0, 0, proc);
				break;
			case -3:
				aux_add_error(EMIC, "MIC not validated (no certificate available)", 0, 0, proc);
				break;
			case -4:
				aux_add_error(EMIC, "MIC not validated (violation of PEM name subordination rule)", 0, 0, proc);
				break;
			case -1:
				index = 0;
				while(verifresult->verifstep[index]) index++;
				index--;
				switch(verifresult->verifstep[index]->crlcheck) {
					case REVOKED:
						aux_add_error(EMIC, "MIC not validated (REVOKED certificate encountered)", 0, 0, proc);
						break;
					case CRL_NOT_AVAILABLE:
						aux_add_error(EMIC, "MIC not validated (Couldn't check all CRLs)", 0, 0, proc);
						break;
					case CRL_OUT_OF_DATE:
						aux_add_error(EMIC, "MIC not validated (Expired CRL encountered)", 0, 0, proc);
						break;
					default:
						AUX_ADD_ERROR;
				}
				break;
			default:
				break;
		} /* switch */
	}
	else aux_free_error();
	local->validation_result = verifresult;
	verifresult = (VerificationResult *)0;
	local->body = aux_cpy_OctetString(local_mess->body);


	if(!(logpath = aux_get_abs_path(get_cadir(pse), CALOG))) AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local););

	if ((logfile = fopen(logpath, LOGFLAGS)) == (FILE *) 0) {
		aux_add_error(ELOGFILE, "Can't open log file", (char *) CALOG, char_n, proc);
		free(logpath);
		aux_free_PemMessageLocal(&local);
		return ((PemMessageLocal *)0);
	}
	free(logpath);


	cert = hd->certificates->usercertificate;

	/* include tests of prototype certificate before signing */
	if(top_cert) {
		top_key.key = top_cert->tbs->subjectPK;
		top_key.keyref = 0;
		top_key.pse_sel = (PSESel *)0;
	
		/*
		  *    Verify prototype certificate
		  */
	
		if(sec_verify(top_cert->tbs_DERcode, top_cert->sig, SEC_END, &top_key, (HashInput * ) 0)) {
			LOGAFERR;
			aux_free_PemMessageLocal(&local);

			return((PemMessageLocal *)0);
		}


		pkroot = aux_create_PKRoot(top_cert, (Certificate *)0);
		if(!isCA) aux_free_Signature(&(pkroot->newkey->sig));
	}


	printrepr = aux_DName2Name(cert->tbs->subject);
        if(af_cadb_add_user(printrepr, get_cadir(pse)) < 0) {
                LOGERR("can't access user db");
		AUX_ADD_ERROR;
		aux_free_PemMessageLocal(&local);
		return ((PemMessageLocal *)0);
        }
	free(printrepr);

	/* get issuer as name of PSE */
	if (cert->tbs->issuer) {
		aux_free_DName (&cert->tbs->issuer);
	}
	cert->tbs->issuer = af_pse_get_Name();

	if (cert->tbs->valid->notbefore) {
		free (cert->tbs->valid->notbefore);
		cert->tbs->valid->notbefore = CNULL;
	}
	if (cert->tbs->valid->notafter) {
		free (cert->tbs->valid->notafter);
		cert->tbs->valid->notafter = CNULL;
	}

	cert->tbs->valid->notbefore = aux_current_UTCTime();
	cert->tbs->valid->notafter = aux_delta_UTCTime(cert->tbs->valid->notbefore);

	cert->tbs->serialnumber = af_pse_incr_serial();
	if(!cert->tbs->serialnumber) AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local);)
	cert->tbs->version = 0;           /* default version */

	aux_free_OctetString(&cert->tbs_DERcode);
	if (cert->sig) aux_free_Signature( &cert->sig );
	cert->sig = (Signature * )malloc(sizeof(Signature));
	cert->sig->signAI = af_pse_get_signAI();
	if ( ! cert->sig->signAI ) AUX_ERROR_RETURN((PemMessageLocal *)0, 
									aux_free_PemMessageLocal(&local);)

	if (aux_ObjId2AlgType(cert->sig->signAI->objid) == ASYM_ENC )
		cert->sig->signAI = aux_cpy_AlgId(algorithm);

	cert->tbs->signatureAI = aux_cpy_AlgId(cert->sig->signAI);
	cert->tbs_DERcode = e_ToBeSigned(cert->tbs);
           
	if (!cert->tbs_DERcode) {
		aux_add_error(EENCODE, "Can't encode TBS", CNULL, 0, proc);
		LOGAFERR;
		aux_free_PemMessageLocal(&local);
		return ((PemMessageLocal *)0);
	}
	if (af_sign(cert->tbs_DERcode, cert->sig, SEC_END) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, LOGAFERR;
									aux_free_PemMessageLocal(&local))

	if (af_cadb_add_Certificate(0, cert, get_cadir(pse))) AUX_ERROR_RETURN((PemMessageLocal *)0, LOGERR("Can't access cert db");
									aux_free_PemMessageLocal(&local))

/* create message to reply */


        soc = (SET_OF_Certificate *)malloc(sizeof(SET_OF_Certificate));
        soc->element = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);



        if(aux_cmp_DName(soc->element->tbs->issuer, soc->element->tbs->subject)) {

	    soc->next = af_pse_get_CertificateSet(SIGNATURE);


 	    pkroot = af_pse_get_PKRoot();
	    hd->root_certificate = af_PKRoot2Protocert(pkroot);
	    if(pkroot) aux_free_PKRoot(&pkroot);

	}
        else {
	    hd->root_certificate = soc->element;
	    free(soc);
	    soc = af_pse_get_CertificateSet(SIGNATURE);

	}
	fcp = (FCPath * )malloc(sizeof(FCPath));

	
	hd->certificates->forwardpath = fcp;
	fcp->liste = soc;
	fcp->next_forwardpath = af_pse_get_FCPath(NULLDNAME);


	return(local);
}

/*------------------------------------------------------------------
  pem_certify() validates the given message and,
  if neccessary, decrypts the body.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_validate
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_validate(
	int		  pse,
	PemMessageLocal	 *local_mess
)

#else

PemMessageLocal *pem_validate(
	pse,
	local_mess
)
int		  pse;
PemMessageLocal	 *local_mess;

#endif

{
	PemHeaderLocal 		*hd;
	FCPath 			*received_fcpath,
				*ordered_fcpath,
				*actual;
	Certificate 		*mycert = (Certificate *)0;
	EncryptedKey		 encrypted_key;
	Key 			*encryptKEY,
				 top_key;
	SET_OF_PemRecLocal 	*local_recs;
	BitString 		 decr_bstr;
	OctetString		 ostr,
				 body,
				*dc_body,
				*c_body;
	Signature		 signature; 
	int			 index, 
				 cert_found;
	PKRoot			*pkroot = (PKRoot *)0;
	PemMessageLocal 	*local;
	char			*proc = "pem_validate";

	if(!local_mess) return((PemMessageLocal *)0);

	if(!(local = aux_cpy_PemMessageLocal(local_mess))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);

	hd = local->header;

	if(!hd || hd->proctype == PEM_CRL || hd->proctype == PEM_CRL_RETRIEVAL_REQUEST) {
		aux_free_PemHeaderLocal(&local->header);	
		return(local);
	}

	
	/* Sort received IssuerCertificates in ascending order */

	if(hd->certificates && hd->certificates->forwardpath && hd->certificates->forwardpath->next_forwardpath) {

		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Sorting Issuer Certificates\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

		/* Select certificate from received_fcpath whose subject corresponds to the issuer */
		/* of the usercertificate						           */
		for(received_fcpath = hd->certificates->forwardpath;
	   	    received_fcpath && aux_cmp_DName(hd->certificates->usercertificate->tbs->issuer, received_fcpath->liste->element->tbs->subject);
		    received_fcpath = received_fcpath->next_forwardpath)
			;

		if(received_fcpath){
			ordered_fcpath = actual = (FCPath * )malloc(sizeof(FCPath));
			if(!ordered_fcpath){
				aux_add_error(EMALLOC, "ordered_fcpath", CNULL, 0, proc);
				return((PemMessageLocal *)0);
			}
			ordered_fcpath->liste = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate));
			if(!ordered_fcpath->liste){
				aux_add_error(EMALLOC, "ordered_fcpath->liste", CNULL, 0, proc);
				return((PemMessageLocal *)0);
			}
			ordered_fcpath->liste->element = aux_cpy_Certificate(received_fcpath->liste->element);
			ordered_fcpath->liste->next = (SET_OF_Certificate * )0;
			ordered_fcpath->next_forwardpath = (FCPath *)0;


			received_fcpath = hd->certificates->forwardpath;
	
			while(received_fcpath){
				for(received_fcpath = hd->certificates->forwardpath;
				       received_fcpath && aux_cmp_DName(actual->liste->element->tbs->issuer, received_fcpath->liste->element->tbs->subject);
				    received_fcpath = received_fcpath->next_forwardpath)
					;
		
				if(received_fcpath){
					actual->next_forwardpath = (FCPath * )malloc(sizeof(FCPath));
					if(!actual->next_forwardpath){
						aux_add_error(EMALLOC, "actual->next_forwardpath", CNULL, 0, proc);
						return((PemMessageLocal *)0);
					}
					actual = actual->next_forwardpath;
					actual->liste = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate));
					if(!actual->liste){
						aux_add_error(EMALLOC, "actual->liste", CNULL, 0, proc);
						return((PemMessageLocal *)0);
					}
					actual->liste->element = aux_cpy_Certificate(received_fcpath->liste->element);
					actual->liste->next = (SET_OF_Certificate * )0;
					actual->next_forwardpath = (FCPath *)0;
				}
			}
		}
		else ordered_fcpath = (FCPath *)0;

		aux_free_FCPath(&hd->certificates->forwardpath);
		hd->certificates->forwardpath = ordered_fcpath;
	}
	if(hd->certificates && hd->certificates->usercertificate) {

		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Getting Originator Certificate\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

		if((cert_found = pem_fill_Certificate(pse, SIGNATURE, hd->certificates->usercertificate)) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)
	}
	if(hd->proctype == PEM_ENC) {

		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Getting encrypted key\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

		if(!(encryptKEY = pem_get_decryption_key(pse, local))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);

		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Decrypting body and MIC info\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

		if(pem_decrypt(local, encryptKEY) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);

		aux_free_Key(&encryptKEY);
	}

	aux_cpy2_BitString(&signature.signature, hd->mic_fields->mic);
	signature.signAI = aux_cpy_AlgId(hd->mic_fields->signAI);


	if(pem_verbose_1) 
	if(!(local->comment = CATSPRINTF(local->comment, "Canonicalize body\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

	if(!(dc_body = aux_decanon(local->body)))  AUX_ERROR_RETURN((PemMessageLocal *)0, ;);
	aux_free_OctetString(&local->body);
	local->body = dc_body;


	if(!(c_body = aux_canon(local->body))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);




	if(hd->root_certificate) {
		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Verifiing root certificate\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

		top_key.key = hd->root_certificate->tbs->subjectPK;
		top_key.keyref = 0;
		top_key.pse_sel = (PSESel *)0;
	
		/*
		  *    Verify prototype certificate
		  */
	
		if(sec_verify(hd->root_certificate->tbs_DERcode, hd->root_certificate->sig, SEC_END, &top_key, (HashInput * ) 0)) {

			return((PemMessageLocal *)0);
		}


		pkroot = aux_create_PKRoot(hd->root_certificate, (Certificate *)0);
		if(!isCA) aux_free_Signature(&(pkroot->newkey->sig));
	}
	if(pem_verbose_1) 
	if(!(local->comment = CATSPRINTF(local->comment, "Verifiing certificates\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

	if(!cert_found) {
			aux_add_error(ENONAME, "Originator Certificate not found", CNULL, 0, proc);
			return((PemMessageLocal *)0);
	}
	else if(af_verify(c_body, &signature, SEC_END, hd->certificates, 0, pkroot)) {
		if(verifresult)
		switch(verifresult->trustedKey){
			case 0:
			case 1:
			case 2:
			case 3:
				index = 0;
				while(verifresult->verifstep[index]) index++;
				index--;
				switch(verifresult->verifstep[index]->crlcheck) {
					case REVOKED:
						aux_add_error(EMIC, "MIC not validated (REVOKED certificate encountered)", 0, 0, proc);
						break;
					case CRL_NOT_AVAILABLE:
						aux_add_error(EMIC, "MIC not validated (Couldn't check all CRLs)", 0, 0, proc);
						break;
					case CRL_OUT_OF_DATE:
						aux_add_error(EMIC, "MIC not validated (Expired CRL encountered)", 0, 0, proc);
						break;
					default:
						AUX_ADD_ERROR;
				}
/*				aux_add_error(EMIC, "MIC not validated (Verification failure)", 0, 0, proc); */
				break;
			case -2:
				aux_add_error(EMIC, "MIC not validated (EXPIRED certificate encountered)", 0, 0, proc);
				break;
			case -3:
				aux_add_error(EMIC, "MIC not validated (no certificate available)", 0, 0, proc);
				break;
			case -4:
				aux_add_error(EMIC, "MIC not validated (violation of PEM name subordination rule)", 0, 0, proc);
				break;
			case -1:
				index = 0;
				while(verifresult->verifstep[index]) index++;
				index--;
				switch(verifresult->verifstep[index]->crlcheck) {
					case REVOKED:
						aux_add_error(EMIC, "MIC not validated (REVOKED certificate encountered)", 0, 0, proc);
						break;
					case CRL_NOT_AVAILABLE:
						aux_add_error(EMIC, "MIC not validated (Couldn't check all CRLs)", 0, 0, proc);
						break;
					case CRL_OUT_OF_DATE:
						aux_add_error(EMIC, "MIC not validated (Expired CRL encountered)", 0, 0, proc);
						break;
					default:
						AUX_ADD_ERROR;
				}
				break;
			default:
				break;
		} /* switch */
	}
	else aux_free_error();

	local->validation_result = verifresult;
	verifresult = (VerificationResult *)0;
	return(local);
}



/*------------------------------------------------------------------
  pem_certify() validates each element of the given message set.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_validateSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_PemMessageLocal *pem_validateSet(
	int			  pse,
	SET_OF_PemMessageLocal	 *message
)

#else

SET_OF_PemMessageLocal *pem_validateSet(
	pse,
	message
)
int			  pse;
SET_OF_PemMessageLocal	 *message;

#endif

{
	SET_OF_PemMessageLocal 	*val_message = (SET_OF_PemMessageLocal *)0,
				*val_message2;
	char			*proc = "pem_validateSet";

	while(message) {
		if(val_message) {
			NEWSTRUCT(val_message2->next, SET_OF_PemMessageLocal, (SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&val_message))
			val_message2 = val_message2->next;
		}
		else {
			NEWSTRUCT(val_message2, SET_OF_PemMessageLocal, (SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&val_message))
			val_message = val_message2;
		}
		val_message2->next = (SET_OF_PemMessageLocal *)0;
		val_message2->element = pem_validate(pse, message->element);
		if(message->element && !val_message2->element) {
			if(!(val_message2->element = aux_cpy_PemMessageLocal(message->element))) 
								AUX_ERROR_RETURN((SET_OF_PemMessageLocal *) 0, 
								aux_free_SET_OF_PemMessageLocal(&val_message))

		}
		val_message2->element->error = aux_sprint_error(val_message2->element->error, pem_verbose_level);
		aux_free_error();
		message = message->next;
	}
	return(val_message);
	
}
/*------------------------------------------------------------------
  pem_reply_crl_requestSet() creates a CRL message according to
  all CRL-RETRIEVAL-REQUEST messages of the given set.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_reply_crl_requestSet
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_reply_crl_requestSet(
	int			  pse,
	SET_OF_PemMessageLocal	 *local_message
)

#else

OctetString *pem_reply_crl_requestSet(
	pse,
	local_message
)
int			  pse;
SET_OF_PemMessageLocal	 *local_message;

#endif

{
	OctetString	*message,
			*new;
	char		*proc = "pem_reply_crl_requestSet";

	NEWSTRUCT(message, OctetString, (OctetString *)0, ;)
	message->octets = CNULL;
	message->noctets = 0;

	for(; local_message; local_message = local_message->next) 
	    if(local_message->element && local_message->element->header &&
			local_message->element->header->proctype == PEM_CRL_RETRIEVAL_REQUEST) {
		new = pem_reply_crl_request(pse, local_message->element);
		if(new)	{
			if(aux_append_OctetString(message, new) < 0) AUX_ERROR_RETURN((OctetString *)0, ;)
			aux_free_OctetString(&new);
		}
		else AUX_ERROR_RETURN((OctetString *)0, aux_free_OctetString(&message))
	}
	return(message);
	
}


/*------------------------------------------------------------------
  pem_crl() creates a CRL message.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_crl
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_crl(
	int		  pse,
	SET_OF_NAME	 *issuers
)

#else

PemMessageLocal *pem_crl(
	pse,
	issuers
)
int		  pse;
SET_OF_NAME	 *issuers;

#endif

{
	PemMessageLocal 		*local;
	PemHeaderLocal  		*hd;
	CRLWithCertificates 		*pemcrlwithcerts;
	SET_OF_CRLWithCertificates 	*setofpemcrlwithcerts2,
					*setofpemcrlwithcerts = 0;
	char				*proc = "pem_crl";

	if(!get_cadir(pse)){
		aux_add_error(EINVALID, "No CA directory specified", CNULL, 0, proc);
		return ((PemMessageLocal *)0);
	}
	NEWSTRUCT(local, PemMessageLocal, (PemMessageLocal *)0, ;)
	NEWSTRUCT(local->header, PemHeaderLocal, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

	local->body = 0;

	hd = local->header;
	hd->rfc_version = 4;
	hd->proctype = PEM_CRL;

	if(!issuers) setofpemcrlwithcerts = af_cadb_list_CRLWithCertificates(get_cadir(pse));
	else
	while (issuers) {
		if(issuers->element) {
			if(pem_check_name(pse, issuers->element)) 
				aux_add_warning(EINVALID, "Invalid issuer name",
					issuers->element->alias ? issuers->element->alias :issuers->element->name,
					char_n, proc);
			else {

				if((pemcrlwithcerts = af_cadb_get_CRLWithCertificates(issuers->element->name, get_cadir(pse)))) {
					setofpemcrlwithcerts2 = (SET_OF_CRLWithCertificates * ) 
							malloc(sizeof(SET_OF_CRLWithCertificates));
					setofpemcrlwithcerts2->element = pemcrlwithcerts;
					setofpemcrlwithcerts2->next = setofpemcrlwithcerts;
					setofpemcrlwithcerts = setofpemcrlwithcerts2;

				}
			}
		}
		issuers = issuers->next;
	}

	hd->crl_fields = setofpemcrlwithcerts;


	return(local);

}
/*------------------------------------------------------------------
  pem_crl_rr() creates a CRL-RETRIEVAL-REQUEST message.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_crl_rr
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_crl_rr(
	int		  pse,
	SET_OF_NAME	 *issuers
)

#else

PemMessageLocal *pem_crl_rr(
	pse,
	issuers
)
int		  pse;
SET_OF_NAME	 *issuers;

#endif

{
	PemMessageLocal 	*local;
	PemHeaderLocal  	*hd;
	SET_OF_DName 		*names2,
				*names = 0;
	char			*proc = "pem_crl_rr";

	NEWSTRUCT(local, PemMessageLocal, (PemMessageLocal *)0, ;)
	NEWSTRUCT(local->header, PemHeaderLocal, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

	local->body = 0;

	hd = local->header;
	hd->rfc_version = 4;
	hd->proctype = PEM_CRL_RETRIEVAL_REQUEST;

	while (issuers) {
		if(issuers->element) {
			if(pem_check_name(pse, issuers->element)) 
				aux_add_warning(EINVALID, "Invalid issuer name",
					issuers->element->alias ? issuers->element->alias :issuers->element->name,
					char_n, proc);
			else {

				NEWSTRUCT(names2, SET_OF_DName, (PemMessageLocal * ) 0, aux_free_PemMessageLocal(&local))
				names2->element = aux_cpy_DName(issuers->element->dname);
				names2->next = names;
				names = names2;

			}
		}
		issuers = issuers->next;
	}

	hd->crl_rr_fields = names;


	return(local);

}
/*------------------------------------------------------------------
  pem_enhance() creates a MIC-Clear, MIC-Only or Encrypted message.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_enhance
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_enhance(
	int		  pse,
	PEM_Proc_Types	  proc_type,
	OctetString	 *text,
	int		  no_of_certs,
	SET_OF_NAME	 *recipients
)

#else

PemMessageLocal *pem_enhance(
	pse,
	proc_type,
	text,
	no_of_certs,
	recipients
)
int		  pse;
PEM_Proc_Types	  proc_type;
OctetString	 *text;
int		  no_of_certs;
SET_OF_NAME	 *recipients;

#endif

{
	PemMessageLocal *local;
	PemHeaderLocal  *hd;
	Certificates	*mycerts = 0;
	Certificate	*mycert = 0;
	FCPath		*fcpath;
	int 		 i;
	Signature     	 signature;
	BitString	 enc_c_body;
	Key 		*encryptKEY;
	Key              encryption_key;
	SET_OF_PemRecLocal *recips_tmp;
	Certificate	*recip_cert;
	EncryptedKey     encrypted_key;
	OctetString 	*c_body, body, sign;
	Boolean		 no_key_info = TRUE;
	char		*proc = "pem_enhance";

	NEWSTRUCT(local, PemMessageLocal, (PemMessageLocal *)0, ;)
	NEWSTRUCT(local->header, PemHeaderLocal, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
	hd = local->header;
	hd->rfc_version = 4;
	hd->content_domain = PEM_RFC822;
	hd->proctype = proc_type;


	if(!no_of_certs) {
		if(!(mycert = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0)))
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

		NEWSTRUCT(hd->certificates, Certificates, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
		NEWSTRUCT(hd->certificates->usercertificate, Certificate, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
		NEWSTRUCT(hd->certificates->usercertificate->tbs, ToBeSigned, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

		if(!(hd->certificates->usercertificate->tbs->issuer = aux_cpy_DName(mycert->tbs->issuer))) 
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
		if(!(hd->certificates->usercertificate->tbs->serialnumber = aux_cpy_OctetString(mycert->tbs->serialnumber))) 
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

	}
	else if(no_of_certs == 1) {
		NEWSTRUCT(hd->certificates, Certificates, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
		if(!(mycert = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0)))
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))


		if(!(hd->certificates->usercertificate = aux_cpy_Certificate(mycert))) 
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))



	}
	else {
		if(!(mycerts = af_pse_get_Certificates(SIGNATURE, NULL)))
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

		if(!(hd->certificates = aux_cpy_Certificates(mycerts))) 
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

		for(fcpath = hd->certificates->forwardpath, i = no_of_certs - 2; fcpath && i; fcpath = fcpath->next_forwardpath, i--);
		if(fcpath) aux_free_FCPath(&fcpath->next_forwardpath);
	}

	if(!(local->body = aux_canon(text))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

	/* MIC_ALG and MIC_ENC_ALG from pem.h */
	if(aux_Name2AlgHash(MIC_ALG) == MD2) {
		if(strcmp(MIC_ENC_ALG, "RSA") == 0) signature.signAI = md2WithRsaEncryption;
		else signature.signAI = md2WithRsa;
	} 

	else if(aux_Name2AlgHash(MIC_ALG) == MD4) {
		if(strcmp(MIC_ENC_ALG, "RSA") == 0) signature.signAI = md4WithRsaEncryption;
		else signature.signAI = md4WithRsa;
	}

	else if(aux_Name2AlgHash(MIC_ALG) == MD5) {
		if(strcmp(MIC_ENC_ALG, "RSA") == 0) signature.signAI = md5WithRsaEncryption;
		else signature.signAI = md5WithRsa;
	} 

	else if(aux_Name2AlgHash(MIC_ALG) == SHA) {
		if(strcmp(MIC_ENC_ALG, "NIST-DSA") == 0) signature.signAI = dsaWithSHA;
		else {
			aux_add_error(EINVALID, "Invalid signature enc algorithm", MIC_ALG, char_n, proc);
			return((PemMessageLocal *)0 );
		}
	}
	else {
		aux_add_error(EINVALID, "Invalid signature algorithm", MIC_ALG, char_n, proc);
		return((PemMessageLocal *)0 );
	}
	if(af_sign(local->body, &signature, SEC_END) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

	NEWSTRUCT(hd->mic_fields, PemMicLocal, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
	if(!(hd->mic_fields->signAI = aux_cpy_AlgId(signature.signAI)))
		AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
	if(!(hd->mic_fields->mic = aux_cpy_BitString(&signature.signature)))
		AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))




	if(proc_type == PEM_ENC) {
		NEWSTRUCT(hd->dek_fields, PemDekLocal, (PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

		if(!(encryptKEY = pem_new_encryption_key()))
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))


		if(pem_encrypt(local, encryptKEY) < 0)
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))





		if(!(hd->dek_fields->dekinfo_enc_alg = aux_cpy_AlgId(encryptKEY->key->subjectAI))) 
				AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
		encryption_key.keyref = 0;
		encryption_key.alg = aux_Name2AlgId(DEK_ENC_ALG);
		encryption_key.pse_sel = (PSESel * )0;

		while (recipients || no_key_info) {
			if(recipients && recipients->element && ( recipients->element->alias ||
								recipients->element->name || recipients->element->dname)) {

				if(pem_check_name(pse, recipients->element)) 
					aux_add_warning(EINVALID, "Invalid recipient name",
						recipients->element->alias ? recipients->element->alias :recipients->element->name,
						char_n, proc);
				else
				if((recip_cert = af_search_Certificate(ENCRYPTION, recipients->element->dname))) {
					no_key_info = FALSE;

					if(hd->dek_fields->recipients) {
						NEWSTRUCT(recips_tmp->next, SET_OF_PemRecLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))
						recips_tmp = recips_tmp->next;
					}
					else {
						NEWSTRUCT(recips_tmp, SET_OF_PemRecLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))

						hd->dek_fields->recipients = recips_tmp;
					}
					recips_tmp->next = (SET_OF_PemRecLocal *)0;
					NEWSTRUCT(recips_tmp->element, PemRecLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))
					recips_tmp->element->certificate = recip_cert;				


					aux_free_AlgId(&recip_cert->tbs->subjectPK->subjectAI);
					recip_cert->tbs->subjectPK->subjectAI = aux_Name2AlgId(DEK_ENC_ALG);
					encryption_key.key = recip_cert->tbs->subjectPK;

					if (af_get_EncryptedKey(&encrypted_key, encryptKEY, &encryption_key, (DName *)0, 
						recip_cert->tbs->subjectPK->subjectAI) < 0)
							AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
	
					recips_tmp->element->enc_alg = recip_cert->tbs->subjectPK->subjectAI;

					NEWSTRUCT(recips_tmp->element->dek, BitString, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))
					recips_tmp->element->dek->nbits = encrypted_key.subjectkey.nbits;
					recips_tmp->element->dek->bits = encrypted_key.subjectkey.bits;
				}
			}
			else if(!hd->dek_fields->keyinfo_enc_alg) {
				no_key_info = FALSE;
				
				if(!(recip_cert = af_pse_get_Certificate(ENCRYPTION, NULL, 0)))
					AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))
				aux_free_AlgId(&recip_cert->tbs->subjectPK->subjectAI);
				recip_cert->tbs->subjectPK->subjectAI = aux_Name2AlgId(DEK_ENC_ALG);
				encryption_key.key = recip_cert->tbs->subjectPK;

				if (af_get_EncryptedKey(&encrypted_key, encryptKEY, &encryption_key, (DName *)0, 
					recip_cert->tbs->subjectPK->subjectAI) < 0)
						AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

				hd->dek_fields->keyinfo_enc_alg = recip_cert->tbs->subjectPK->subjectAI;

				NEWSTRUCT(hd->dek_fields->keyinfo_dek, BitString, (PemMessageLocal *) 0, 
							aux_free_PemMessageLocal(&local))
				hd->dek_fields->keyinfo_dek->nbits = encrypted_key.subjectkey.nbits;
				hd->dek_fields->keyinfo_dek->bits = encrypted_key.subjectkey.bits;
			}
			if(recipients) recipients = recipients->next;
		} /*end while over recp*/
		aux_free_AlgId(&encryption_key.alg);

	}

	return(local);
}

/*------------------------------------------------------------------
  pem_check_name() fills the empty compontents of a name structure.
  if an alias or name cannot transformed into a DName -1 is returned
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_check_name
 *
 ***************************************************************/
#ifdef __STDC__

RC pem_check_name(
	int	  pse,
	NAME	 *name
)

#else

RC pem_check_name(
	pse,
	name
)
int	  pse;
NAME	 *name;

#endif

{
	char	*proc = "pem_check_name";

	if(!name) return(0);

	if(!name->dname) {
		if(!name->name) 
			if(!(name->name = aux_alias2Name(name->alias))) 
				return(-1);
		if(!(name->dname = aux_Name2DName(name->name))) 
			return(-1);
	}

	if(!name->name) {
		if(!(name->name = aux_DName2Name(name->dname))) 
			return(-1);
	}
	if(!name->alias) {
		if(!(name->alias = aux_Name2NameOrAlias(name->name))) 
			return(-1);
	}

	return(0);
}


/***************************************************************
 *
 * Procedure pem_forward
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_forward(
	int			  pse,
	PemMessageLocal	 	 *message,
	PEM_Proc_Types	 	  proc_type,
	SET_OF_NAME		 *recipients
)

#else

PemMessageLocal *pem_forward(
	pse,
	message,
	proc_type,
	recipients
)
int			  pse;
PemMessageLocal	 	 *message;
PEM_Proc_Types	 	  proc_type;
SET_OF_NAME		 *recipients;
#endif

{
	char			*proc = "pem_forward";
	PemHeaderLocal  	*hd;
	PemMessageLocal 	*forward_message;
	Certificate		*mycert = 0;
	Certificate		*recip_cert;
	EncryptedKey    	 encrypted_key;
	SET_OF_PemRecLocal 	*local_recs;
	SET_OF_PemRecLocal 	*recips_tmp;
	Key             	*encryptKEY = 0;
	Key             	 encryption_key;
	Signature		 signature; 
	OctetString		 ostr,
				*c_body,
				 sign,
				 body;
	BitString 		 decr_bstr;
	BitString		 enc_c_body;

	if(!message) return((PemMessageLocal *)0);

	if(!(forward_message = aux_cpy_PemMessageLocal(message))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;)

	if(!(hd = forward_message->header) 	 || proc_type == PEM_CRL
					 || proc_type == PEM_CRL_RETRIEVAL_REQUEST
					 || proc_type == NO_PEM
					 || hd->proctype == PEM_CRL
					 || hd->proctype == PEM_CRL_RETRIEVAL_REQUEST
					 || (hd->proctype == proc_type && proc_type != PEM_ENC))
				return(forward_message);







	if(hd->proctype == PEM_ENC) {
		if(!(encryptKEY = pem_get_decryption_key(pse, forward_message))) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);

	}
	else if(proc_type == PEM_ENC) {

		if(!(encryptKEY = pem_new_encryption_key()))
			AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&forward_message))
	}


	if(hd->proctype == PEM_ENC && proc_type != PEM_ENC) {

		aux_free_PemDekLocal(&hd->dek_fields);

		if(pem_decrypt(forward_message, encryptKEY) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);


	}
	else 	if(hd->proctype != PEM_ENC && proc_type == PEM_ENC) {

		if(pem_encrypt(forward_message, encryptKEY) < 0) AUX_ERROR_RETURN((PemMessageLocal *)0, ;);

	}

	if(proc_type == PEM_ENC) {
		if(!hd->dek_fields) {
			NEWSTRUCT(hd->dek_fields, PemDekLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&forward_message))

			if(!(hd->dek_fields->dekinfo_enc_alg = aux_cpy_AlgId(encryptKEY->key->subjectAI))) 
				AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&forward_message))
		}
		aux_free_SET_OF_PemRecLocal(&hd->dek_fields->recipients);

		encryption_key.keyref = 0;
		encryption_key.alg = aux_Name2AlgId(DEK_ENC_ALG);
		encryption_key.pse_sel = (PSESel * )0;

		while (recipients) {
			if(recipients && recipients->element && ( recipients->element->alias ||
								recipients->element->name || recipients->element->dname)) {

				if(pem_check_name(pse, recipients->element)) 
					aux_add_warning(EINVALID, "Invalid recipient name",
						recipients->element->alias ? recipients->element->alias :recipients->element->name,
						char_n, proc);
				else
				if((recip_cert = af_search_Certificate(ENCRYPTION, recipients->element->dname))) {

					if(hd->dek_fields->recipients) {
						NEWSTRUCT(recips_tmp->next, SET_OF_PemRecLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&forward_message))
						recips_tmp = recips_tmp->next;
					}
					else {
						NEWSTRUCT(recips_tmp, SET_OF_PemRecLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&forward_message))

						hd->dek_fields->recipients = recips_tmp;
					}
					recips_tmp->next = (SET_OF_PemRecLocal *)0;
					NEWSTRUCT(recips_tmp->element, PemRecLocal, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&forward_message))
					recips_tmp->element->certificate = recip_cert;				


					aux_free_AlgId(&recip_cert->tbs->subjectPK->subjectAI);
					recip_cert->tbs->subjectPK->subjectAI = aux_Name2AlgId(DEK_ENC_ALG);
					encryption_key.key = recip_cert->tbs->subjectPK;

					if (af_get_EncryptedKey(&encrypted_key, encryptKEY, &encryption_key, (DName *)0, 
						recip_cert->tbs->subjectPK->subjectAI) < 0)
							AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&forward_message))
	
					recips_tmp->element->enc_alg = recip_cert->tbs->subjectPK->subjectAI;

					NEWSTRUCT(recips_tmp->element->dek, BitString, (PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&forward_message))
					recips_tmp->element->dek->nbits = encrypted_key.subjectkey.nbits;
					recips_tmp->element->dek->bits = encrypted_key.subjectkey.bits;
				}
			}

			recipients = recipients->next;
		}

	}

	aux_free_Key(&encryptKEY);

	hd->proctype = proc_type;

	return(forward_message);

}

/***************************************************************
 *
 * Procedure pem_forwardSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_PemMessageLocal *pem_forwardSet(
	int			  pse,
	SET_OF_PemMessageLocal	 *message,
	PEM_Proc_Types	 	  proc_type,
	SET_OF_NAME		 *recipients
)

#else

SET_OF_PemMessageLocal *pem_forwardSet(
	pse,
	message,
	proc_type,
	recipients
)
int			  pse;
SET_OF_PemMessageLocal	 *message;
PEM_Proc_Types	 	  proc_type;
SET_OF_NAME		 *recipients;
#endif

{
	SET_OF_PemMessageLocal 	*forward_message = (SET_OF_PemMessageLocal *)0,
				*forward_message2;
	char			*proc = "pem_forwardSet";

	while(message) {
		if(forward_message) {
			NEWSTRUCT(forward_message2->next, SET_OF_PemMessageLocal, (SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&forward_message))
			forward_message2 = forward_message2->next;
		}
		else {
			NEWSTRUCT(forward_message2, SET_OF_PemMessageLocal, (SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&forward_message))
			forward_message = forward_message2;
		}
		forward_message2->next = (SET_OF_PemMessageLocal *)0;
		forward_message2->element = pem_forward(pse, message->element, proc_type, recipients);
		if(message->element && !forward_message2->element) AUX_ERROR_RETURN((SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&forward_message))
		message = message->next;
	}


	
	return(forward_message);
	
}
