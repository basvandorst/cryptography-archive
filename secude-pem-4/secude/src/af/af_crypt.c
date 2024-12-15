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
#include <fcntl.h>
#include <stdio.h>
#include "af.h"

#define MAXCERT 125

/* stack element definition */
struct PathSTK {
	FCPath         *forwardpath;
	CrossCertificates * crosscertificates;
};


#ifdef __STDC__
static int	get_path	(Certificates *orig_cert, PKRoot *rootinfo, UTCTime *time, Certificate **cSTKp[], KeyInfo *topkey, OctetString **topkey_serial, Boolean usercert);
static int	certs_at_one_level	(FCPath *path);
static int	verify_CertificateWithPkroot	(Certificate *cert, PKRoot *pkroot);
static void	get_path_finish	(struct PathSTK *fstk, Certificate **cstk, PKList *pklist);
static UTCTime	*check_black_list	(Certificate *cert, UTCTime *time);
static Certificate	*check_CrossCertificates	(DName *own_root, DName *foreign_root);
static RC	complete_FCPath_from_Directory	(Certificate *tobeverified_cert, SET_OF_Certificate *certpath, int *level, KeyInfo *topkey, OctetString **topkey_serial, PKList *pklist, PKRoot *rootinfo, UTCTime *time);
static KeyInfo	*LookupPK	(DName *name, KeyInfo *namePK, PKList *pklist, UTCTime *time);
static KeyInfo	*LookupPK_in_FCPath	(DName *name, KeyInfo *namePK, FCPath *fcpath, UTCTime *time);
static RC	 examine_VerificationResult	(VerificationResult *verres);
static VerificationStep	 *create_verifstep	(Certificate *cert, int crlcheck, UTCTime *date, Validity *valid, int supplied, Boolean policy_CA, Boolean dn_subordination_violation);

#else

static int             get_path();
static int             certs_at_one_level();
static int	       verify_CertificateWithPkroot();
static void 	       get_path_finish();
static UTCTime       * check_black_list();
static Certificate   * check_CrossCertificates(); 
static RC              complete_FCPath_from_Directory();
static KeyInfo 	     * LookupPK();
static KeyInfo       * LookupPK_in_FCPath();
static RC	          examine_VerificationResult();
static VerificationStep	* create_verifstep();

#endif

static Boolean	       call_af_verify = FALSE;
static Boolean         pkroot_provided_global = FALSE;

static Boolean	       crosscert_appended_to_certs = FALSE;
static Boolean	       crosscert_from_Directory = FALSE;
static Boolean	       crosscert_from_PSE = FALSE;

static Boolean 	       certs_from_directory = FALSE;

static PKRoot        * own_pkroot;
static FCPath	     * own_fcpath, * reduced_fcpath;
static int	       usercert_crlcheck;

/***************************************************************
 *
 * Procedure af_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC af_encrypt(
	OctetString	 *inoctets,
	BitString	 *outbits,
	More		  more,
	Key		 *key,
	DName		 *dname
)

#else

RC af_encrypt(
	inoctets,
	outbits,
	more,
	key,
	dname
)
OctetString	 *inoctets;
BitString	 *outbits;
More		  more;
Key		 *key;
DName		 *dname;

#endif

{
	static Certificate  * encrCert = (Certificate * ) 0;
	static Key            encrkey = { (KeyInfo * ) 0, 0, (PSESel * ) 0, NULLALGID };
	int	              ret;
	char	            * proc = "af_encrypt";

	if (encrkey.key == (KeyInfo * ) 0)
		if (!dname) {
                        if(!key || (!key->key && !key->pse_sel && !key->keyref)) {
				/* use own key */
				encrCert = af_pse_get_Certificate(ENCRYPTION, NULLDNAME, 0);
				if (!encrCert)	 {
					AUX_ADD_ERROR;
					return - 1;
				}
				key = &encrkey;
				encrkey.key = encrCert->tbs->subjectPK;
			}
		} 
		else {
			/* use EKList */
			key = &encrkey;
			encrkey.key = af_pse_get_PK(ENCRYPTION, dname, NULLDNAME, 0);
			if (!encrkey.key)	 {
				aux_add_error(LASTERROR, LASTTEXT, (char *)dname, DName_n, proc);
				return - 1;
			}

		}
	/* key is defined */

	ret = sec_encrypt(inoctets, outbits, more, key);
	if (more == SEC_END) {
		if (encrCert) {
			aux_free_Certificate(&encrCert);
			encrkey.key = (KeyInfo * ) 0;
		} else if (encrkey.key)
			aux_free_KeyInfo(&encrkey.key);
		else 
			encrkey.key = (KeyInfo * ) 0;
	}
	if (ret < 0) {
		AUX_ADD_ERROR;
	}
	return ret;

}	/* af_encrypt() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure af_decrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC af_decrypt(
	BitString	 *inbits,
	OctetString	 *outoctets,
	More		  more,
	Key		 *key
)

#else

RC af_decrypt(
	inbits,
	outoctets,
	more,
	key
)
BitString	 *inbits;
OctetString	 *outoctets;
More		  more;
Key		 *key;

#endif

{
	static Key	deckey;
	static PSESel	pse;
	RC		ret;
	Boolean         onekeypaironly = FALSE;

	char	      * proc = "af_decrypt";

	if (!key || (!key->key && !key->pse_sel && !key->keyref)) {
		/* use PSE decrypt key */
		if(!key){
			key = &deckey;
			deckey.key = (KeyInfo * ) 0;
			deckey.keyref = 0;
			deckey.pse_sel = &pse;
			deckey.alg = NULLALGID;
		}
		else{
			key->pse_sel = &pse;
		}
		pse.app_name = AF_pse.app_name;
		pse.pin      = AF_pse.pin;

		if(af_check_if_onekeypaironly(&onekeypaironly)){
			AUX_ADD_ERROR;
			return(- 1);
		}

		if(onekeypaironly == TRUE){
			pse.object.name = SKnew_name;
			pse.object.pin = getobjectpin(SKnew_name);
		}
		else{
			pse.object.name = DecSKnew_name;
			pse.object.pin = getobjectpin(DecSKnew_name);
		}
	}

	if ( (ret = sec_decrypt(inbits, outoctets, more, key)) < 0) {
		AUX_ADD_ERROR;
		return(- 1);
	}
	return (ret);

}	/* af_decrypt() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure af_sign
 *
 ***************************************************************/
#ifdef __STDC__

RC af_sign(
	OctetString	 *inoctets,
	Signature	 *signature,
	More		  more
)

#else

RC af_sign(
	inoctets,
	signature,
	more
)
OctetString	 *inoctets;
Signature	 *signature;
More		  more;

#endif

{
	int	             ret;
	static PSESel	     pse;
	static Key           skey;                                    /* signkey refers to PSE Signature_Key */
	static HashInput   * hashin;	                        /* hashinput pointer */
	static Certificate * signCert = (Certificate *) 0;	/* signkey Certificate */
	static int	     first = 0;
	Boolean  	     onekeypaironly = FALSE;
	UTCTime		   * time;

	char		   * proc = "af_sign";


	if(!inoctets) {
		aux_add_error(EINVALID, "in_octets parameter missing", CNULL, 0, proc);
		return(-1);
	}

	if (!first)  {
		first = 1;
		skey.key = (KeyInfo * ) 0;
		skey.keyref = 0;
		skey.pse_sel = &pse;
		/* sign key resides on PSE */
		pse.app_name = AF_pse.app_name;
		pse.pin = AF_pse.pin;
		skey.alg = signature->signAI;

		if(af_check_if_onekeypaironly(&onekeypaironly)){
			AUX_ADD_ERROR;
			return (- 1);
		}

		if(onekeypaironly == TRUE){
			pse.object.name = SKnew_name;
			pse.object.pin = getobjectpin(SKnew_name);
		}
		else{
			pse.object.name = SignSK_name;
			pse.object.pin = getobjectpin(SignSK_name);
		}
		pse.pin = AF_pse.pin; /* if PSE was closed before call of getobjectpin */
		if (inoctets->noctets && signature->signAI){
			if (aux_ObjId2AlgHash(signature->signAI->objid) == SQMODN) {
				signCert = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
				if(! signCert) {
					AUX_ADD_ERROR;
					return - 1;
				}

				hashin = (HashInput * ) & signCert->tbs->subjectPK->subjectkey;
			}
			else 
				hashin = (HashInput * ) 0;
		}
	}

	ret = sec_sign(inoctets, signature, more, &skey, hashin);

	if (more == SEC_END) {
		if(af_sign_check_Validity){
			time = aux_current_UTCTime();
			if(! signCert) signCert = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
			if(! signCert) {
				AUX_ADD_ERROR;
				return(- 1);
			}
			if (af_check_validity_of_Certificate(time, signCert)) {
				/* User Certificate has expired ! */
				AUX_ADD_ERROR;
				return(- 1);
			}
			if(time) free(time);
		}
		if (signCert) 
			aux_free_Certificate(&signCert);
		first = 0;
	}

	if (ret < 0) {
		AUX_ADD_ERROR;
	}
	return(ret);

}	/* af_sign() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure af_verify
 *
 ***************************************************************/
#ifdef __STDC__

RC af_verify(
	OctetString	 *inocts,
	Signature	 *sign,
	More		  more,
	Certificates	 *or_cert,
	UTCTime		 *time,
	PKRoot		 *pkroot
)

#else

RC af_verify(
	inocts,
	sign,
	more,
	or_cert,
	time,
	pkroot
)
OctetString	 *inocts;
Signature	 *sign;
More		  more;
Certificates	 *or_cert;
UTCTime		 *time;
PKRoot		 *pkroot;

#endif

{
	static HashInput  * hashin = (HashInput * ) 0;         /* hash input in case of sqmodn */
	static Key          key;                  	       /* the public key */
        static PKRoot     * lpkroot;
        static char         first = TRUE;
	static Boolean      freetime = FALSE, pkroot_provided = FALSE;      
	int		    pkroot_mall = 0;
	Certificate	  * cert;
	Validity          * valid;
	int	            rc, ret;
#ifdef TIMEMEASURE
	long                hash_secs, hash_microsecs, rsa_secs, rsa_microsecs;
#endif
	AlgHash             alghash;
	UTCTime           * timedate;
	char	          * proc = "af_verify";

	if (!(inocts && sign && sign->signAI)) {
		aux_add_error(EINVALID, "one parameter empty", CNULL, 0, proc);
		return - 1;
	}
	
	if (or_cert && !or_cert->usercertificate) {
		aux_add_error(EINVALID, "no user certificate provided", CNULL, 0, proc);
		return - 1;
	}

        if(first) {
		if(pkroot) pkroot_provided = TRUE;
		if(own_pkroot) aux_free_PKRoot(&own_pkroot);
		if (!time) {
			time = aux_current_UTCTime();
			freetime = TRUE;
		}

		call_af_verify = TRUE;

                lpkroot = pkroot;
                if(! or_cert) {
                        if(!lpkroot) {
                                lpkroot = af_pse_get_PKRoot();
                                pkroot_mall = 1;
                        }
                        if(!lpkroot) {
                		AUX_ADD_ERROR;
				if(freetime) {
					free(time);
					freetime = FALSE;
				}
                                return(-1);
                        }
			own_pkroot = aux_cpy_PKRoot(lpkroot);
                        key.key = lpkroot->newkey->key;
                }
                else {
			key.key = or_cert->usercertificate->tbs->subjectPK;
		}

        	key.keyref = 0;
        	key.pse_sel = (PSESel *) 0;

        	/* needs HashInput parameter set */
        	alghash = aux_ObjId2AlgHash(sign->signAI->objid);
        	if (alghash == SQMODN) hashin = (HashInput * ) & key.key->subjectkey;

                first = FALSE;
        }

	rc = sec_verify(inocts, sign, more, &key, hashin);

	/* Validity time frame checks must be performed after the call of sec_verify, in order to 
	   prevent that the user certificate or root info turns out to be expired (af_verify 
           exits with -1) after sec_verify has already been called (in case of more = SEC_MORE) at
           least once, but before being called with more = SEC_END. The consequence would be
	   that the static variables in sec_verify are not reset correctly, and the next
           verification would use "remains" of the previous verification.
	 */

	if (more == SEC_END) {
		first = TRUE;

		/* Free VerificationResult-structure filled by previous verification process */
		if(verifresult){
			aux_free_VerificationResult(&verifresult);
			verifresult = (VerificationResult * )0;
		}
	
		verifresult = (VerificationResult *)calloc(1, sizeof(VerificationResult) );
		if (! verifresult) {
			aux_add_error(EMALLOC, "verifresult", CNULL, 0, proc);
			return (- 1);
		}
		verifresult->verifstep = (VerificationStep ** )0;
		verifresult->top_name = CNULL;
		if(!or_cert) verifresult->originator = aux_DName2Name(lpkroot->ca);
		else verifresult->originator = aux_DName2Name(or_cert->usercertificate->tbs->subject);
		verifresult->top_serial = NULLOCTETSTRING;
		verifresult->trustedKey = - 1;
		verifresult->valid = (Validity * )0;
		verifresult->textverified = TRUE;
		verifresult->success = TRUE;

		if(pkroot_mall == 1) verifresult->trustedKey = 0;
		else if(pkroot_provided) verifresult->trustedKey = 2;

		/* Check Validity of Root-Info or Usercertificate */
		if(af_check_Validity){
			if (lpkroot){
				if (af_check_validity_of_PKRoot(time, lpkroot)) {
					/* Root Info has expired ! */
					AUX_ADD_ERROR;
					verifresult->top_serial = aux_cpy_OctetString(lpkroot->newkey->serial);
					verifresult->top_name = aux_DName2Name(lpkroot->ca);
					verifresult->valid = aux_cpy_Validity(lpkroot->newkey->valid);
					verifresult->trustedKey = - 2;
					verifresult->success = FALSE;
					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					if(pkroot_mall) aux_free_PKRoot(&lpkroot);
					call_af_verify = FALSE;
					pkroot_provided = FALSE;
					return(- 1);
				}
			}
	
			if(or_cert){
				/* Check Validity of usercertificate */
				if (af_check_validity_of_Certificate(time, or_cert->usercertificate)) {
					/* User Certificate has expired ! */
					AUX_ADD_ERROR;
					verifresult->verifstep = (VerificationStep **)calloc(2, sizeof(VerificationStep * ));
					if (! verifresult->verifstep) {
						aux_add_error(EMALLOC, "verifresult->verifstep", CNULL, 0, proc);
						return (- 1);
					}
			
					verifresult->verifstep[1] = (VerificationStep * )0;
					/* required for while loop in aux_fprint_VerificationResult */
		
					valid = aux_cpy_Validity(or_cert->usercertificate->tbs->valid);
					verifresult->verifstep[0] = create_verifstep(or_cert->usercertificate, 0, (UTCTime *) 0, valid, 0, FALSE, FALSE);
					aux_free_Validity(&valid);
					verifresult->trustedKey = - 2;
					verifresult->success = FALSE;
					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					call_af_verify = FALSE;
					pkroot_provided = FALSE;
					return (- 1);
				}
			}
		}
	}

	if(rc < 0){  /* rc contains result of sec_verify */
		if (more == SEC_END) {
			verifresult->success = FALSE;
	
			if (or_cert){
				verifresult->top_serial = aux_cpy_OctetString(or_cert->usercertificate->tbs->serialnumber);
				verifresult->top_name = aux_DName2Name(or_cert->usercertificate->tbs->subject);
	
				verifresult->verifstep = (VerificationStep **)calloc(2, sizeof(VerificationStep * ));
				if (! verifresult->verifstep) {
					aux_add_error(EMALLOC, "verifresult->verifstep", CNULL, 0, proc);
					return (- 1);
				}
		
				verifresult->verifstep[1] = (VerificationStep * )0;
				/* required for while loop in aux_fprint_VerificationResult */
	
				verifresult->verifstep[0] = create_verifstep(or_cert->usercertificate, 0, (UTCTime * )0, (Validity *) 0, 0, FALSE, FALSE);
				aux_add_error(EVERIFICATION, "Verification of text signature failed", (char *) or_cert->usercertificate, Certificate_n, proc);
			}
			else {
				verifresult->top_serial = aux_cpy_OctetString(lpkroot->newkey->serial);
				verifresult->top_name = aux_DName2Name(lpkroot->ca);
				aux_add_error(EVERIFICATION, "Verification of text signature failed", (char *) lpkroot->newkey->key, KeyInfo_n, proc);
			}
		}
		
                if(pkroot_mall) aux_free_PKRoot(&lpkroot);
                lpkroot = (PKRoot *)0;
		call_af_verify = FALSE;
		pkroot_provided = FALSE;

		if(freetime) {
			free(time);
			freetime = FALSE;
		}
		return (- 1);
	}
#ifdef TIMEMEASURE
	rsa_secs = rsa_sec;
	rsa_microsecs = rsa_usec;
	hash_secs = hash_sec;
	hash_microsecs = hash_usec;
#endif

	/* text has been verified, so verify or_cert */

	timedate = sec_SignatureTimeDate;

	if (more == SEC_END) {
                if(or_cert) {
			if(!or_cert->forwardpath){
				cert = af_pse_get_Certificate(SIGNATURE, NULLDNAME, 0);
				if(! cert){
					AUX_ADD_ERROR;
					call_af_verify = FALSE;
					pkroot_provided = FALSE;
					return (- 1);
				}
				if(!aux_cmp_Certificate(or_cert->usercertificate, cert)) {
					verifresult->trustedKey = 5;
					verifresult->top_serial = aux_cpy_OctetString(or_cert->usercertificate->tbs->serialnumber);
					verifresult->top_name = aux_DName2Name(or_cert->usercertificate->tbs->subject);
		
					verifresult->verifstep = (VerificationStep **)calloc(2, sizeof(VerificationStep * ));
					if (! verifresult->verifstep) {
						aux_add_error(EMALLOC, "verifresult->verifstep", CNULL, 0, proc);
						call_af_verify = FALSE;
						pkroot_provided = FALSE;
						return (- 1);
					}
			
					verifresult->verifstep[1] = (VerificationStep * )0;
					/* required for while loop in aux_fprint_VerificationResult */
		
					verifresult->verifstep[0] = create_verifstep(or_cert->usercertificate, 0, (UTCTime * )0, (Validity *) 0, 0, FALSE, FALSE);
					if(!verifresult->verifstep[0]){
						AUX_ADD_ERROR;
						call_af_verify = FALSE;
						pkroot_provided = FALSE;
						return(- 1);
					}

					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					call_af_verify = FALSE;
					pkroot_provided = FALSE;
					return(0);
				}
			}
			if(pkroot_provided) pkroot_provided_global = TRUE;
        		rc = af_verify_Certificates(or_cert, time, lpkroot);
			pkroot_provided_global = FALSE;
			if(verifresult && (verifresult->trustedKey == 0 || verifresult->trustedKey == 1) && pkroot_provided)
				verifresult->trustedKey = 2;
			call_af_verify = FALSE;
			pkroot_provided = FALSE;
        		if (rc < 0) 
				AUX_ADD_ERROR;
                }
                else {
			if(pkroot_mall) 
				aux_free_PKRoot(&lpkroot);
			verifresult->top_serial = aux_cpy_OctetString(lpkroot->newkey->serial);
			verifresult->top_name = aux_DName2Name(lpkroot->ca);
		}
                lpkroot = (PKRoot *)0;
	}  /* if (more == SEC_END) */

	sec_SignatureTimeDate = timedate;
	call_af_verify = FALSE;
	pkroot_provided = FALSE;

#ifdef TIMEMEASURE
	rsa_sec = rsa_secs;
	rsa_usec = rsa_microsecs;
	hash_sec = hash_secs;
	hash_usec = hash_microsecs;
#endif

	if(freetime) {
		free(time);
		freetime = FALSE;
	}

	if(more == SEC_END && verifresult->success == FALSE) return(- 1);

	return(rc);

}	/* af_verify() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure af_verify_Certificates
 *
 ***************************************************************/
#ifdef __STDC__

RC af_verify_Certificates(
	Certificates	 *or_cert,
	UTCTime		 *time,
	PKRoot		 *pkroot
)

#else

RC af_verify_Certificates(
	or_cert,
	time,
	pkroot
)
Certificates	 *or_cert;
UTCTime		 *time;
PKRoot		 *pkroot;

#endif

{


/*
      - verifresult->success = TRUE

	Verification of "Certificates" succeeded ...

		... with PKRoot(new) 
                    (verifresult->trustedKey = 0)

		... with PKRoot(old) 
                    (verifresult->trustedKey = 1)

		... with provided PKRoot
                    (verifresult->trustedKey = 2)

		... with PKList
                    (verifresult->trustedKey = 3)

		... with FCPath      
                    (verifresult->trustedKey = 4)

		... with own SIGNATURE certificate      
                    (verifresult->trustedKey = 5)

		... with PCAList
                    (verifresult->trustedKey = 6)

      - verifresult->success = FALSE

	Verification of "Certificates" failed ...

		... with PKRoot(new) 				
                    (verifresult->trustedKey = 0  ->  cryptographical verification failure)

		    (verifresult->trustedKey = 0 + verifresult->date is set  
					          -> Signature creation date does not lie 
						     within validity time frame of PKRoot(new) )

		... with PKRoot(old) 				
		    (verifresult->trustedKey = 1  ->  cryptographical verification failure)

		    (verifresult->trustedKey = 1 + verifresult->date is set  
					          -> Signature creation date does not lie 
						     within validity time frame of PKRoot(old) )

		... with provided PKRoot
                    (verifresult->trustedKey = 2)

		... with PKList      				
		    (verifresult->trustedKey = 3)

		... with FCPath      				
                    (verifresult->trustedKey = 4)

		... with certificate at next level above (verification failure)	
		    (verifresult->trustedKey = - 1, crlcheck = 0)

		... as certificate at next level above was revoked	
		    (verifresult->trustedKey = - 1, crlcheck = REVOKED)

		... as certificate provided at next level was expired
		    (verifresult->trustedKey = - 2)

		... as there was no certificate available at next level
		    (verifresult->trustedKey = - 3)	



		A certificate forming part of the VerificationResult structure may be 
		provided as follows: 

		supplied = 0 -> certificate provided within received certification path
		supplied = 1 -> certificate retrieved from Directory (.af-db or X.500)
		supplied = 2 -> certificate retrieved from PSE (PSE-object CrossCSet)

*/

	
	static Boolean  freetime = FALSE;
	Boolean		pkroot_provided = FALSE;      
	HashInput     * hashin = (HashInput * ) 0;         /* hash input in case of sqmodn */
	Key    	        key;                               /* the public key */
	KeyInfo	        topkey;
        int             pkroot_mall = 0;
	OctetString   * topkey_serial, * key_serial;
	int	        rc = 0;
	AlgHash         alghash;
	Certificate * * certList;                          /* certificate array from get path */
	Certificate   * C, * CC, * cert;
	FCPath	      * path;
	PKList	      * pklist = (PKList * )0, *pcalist = (PKList *)0;
	int 		count, index, ret;
	UTCTime	      * date;
	Validity      * valid;
	Certificates  * reduced_or_cert = (Certificates * )0;
	int	        c, cc;
	Boolean	        policy_CA = FALSE, dn_subordination_violation = FALSE, superior_CA_is_policyCA = FALSE;
	int		supplied, crlcheck;

	char	      * proc = "af_verify_Certificates";

	if(call_af_verify == FALSE){
		if(pkroot) pkroot_provided = TRUE;
		if(own_pkroot) aux_free_PKRoot(&own_pkroot);

		/* Free VerificationResult-structure filled by previous verification process */
		if(verifresult){
			aux_free_VerificationResult(&verifresult);
			verifresult = (VerificationResult * )0;
		}
	}
	else pkroot_provided = pkroot_provided_global;

	if(own_fcpath) aux_free_FCPath(&own_fcpath);
	if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);

	crosscert_appended_to_certs = crosscert_from_Directory = crosscert_from_PSE = FALSE;
	certs_from_directory = FALSE;

	if (! or_cert || ! or_cert->usercertificate) {
		aux_add_error(EINVALID, "No user certificate provided", CNULL, 0, proc);
		return (- 1);
	}

        if(! pkroot) {
                pkroot = af_pse_get_PKRoot();
                if(! pkroot) {
        		AUX_ADD_ERROR;
                        return(-1);
                }
                pkroot_mall = 1;
		own_pkroot = aux_cpy_PKRoot(pkroot);
                if(! own_pkroot) {
        		AUX_ADD_ERROR;
                        return(-1);
                }
        }

	key.keyref = 0;
	key.pse_sel = (PSESel *) 0;

	cert = or_cert->usercertificate;

	if(call_af_verify == FALSE){
		/* Allocate memory for VerificationResult structure */
		verifresult = (VerificationResult *)malloc(sizeof(VerificationResult) );
		if (! verifresult) {
			aux_add_error(EMALLOC, "verifresult", CNULL, 0, proc);
			return (- 1);
		}
		verifresult->verifstep = (VerificationStep ** )0;
		verifresult->top_name = CNULL;
		verifresult->top_serial = NULLOCTETSTRING;
		verifresult->trustedKey = - 1;
		verifresult->valid = (Validity * )0;
		verifresult->textverified = FALSE;
		verifresult->success = TRUE;
	}

	if (! time) {
		time = aux_current_UTCTime();
		freetime = TRUE;
	}

	verifresult->verifstep = (VerificationStep **)calloc(MAXCERT, sizeof(VerificationStep * ));
	if (! verifresult->verifstep) {
		aux_add_error(EMALLOC, "verifresult->verifstep", CNULL, 0, proc);
		return (- 1);
	}
	for(index = 0; index < MAXCERT; index++)
		verifresult->verifstep[index] = (VerificationStep * )0;

	if(af_check_Validity && call_af_verify == TRUE){
		/* Check Validity of usercertificate */
		if (af_check_validity_of_Certificate(time, cert)) {
			/* User Certificate has expired ! */
			AUX_ADD_ERROR;
			valid = aux_cpy_Validity(cert->tbs->valid);
			verifresult->verifstep[0] = create_verifstep(cert, 0, (UTCTime *) 0, valid, 0, FALSE, FALSE);
			aux_free_Validity(&valid);
			verifresult->trustedKey = - 2;
			verifresult->success = FALSE;
	
			if(freetime) {
				free(time);
				freetime = FALSE;
			}
			if(pkroot_mall) aux_free_PKRoot(&pkroot);
			return (- 1);
		}
	}

	pcalist = af_pse_get_PCAList();
	if(!pcalist) {
		if(aux_last_error() == EOBJNAME) aux_free_error();
		else {
			AUX_ADD_ERROR;
			return(-1);
		}
	}
	else {
		/* Check user certificate against PCAList */
		if (LookupPK(cert->tbs->subject, cert->tbs->subjectPK, pcalist, time) != 0) {
			verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
			verifresult->top_serial = aux_cpy_OctetString(cert->tbs->serialnumber);
			verifresult->trustedKey = 6;
	
			if(pcalist) aux_free_PKList(&pcalist);
			if(freetime) {
				free(time);
				freetime = FALSE;
			}
			/* verification successfully completed */
			if(examine_VerificationResult(verifresult) < 0) {
				AUX_ADD_ERROR;
				return(-1);
			}
			return(0);
		}
	}
	if (af_chk_crl == FALSE) {
		pklist = af_pse_get_PKList(SIGNATURE);
		if(!pklist) {
			if(aux_last_error() == EOBJNAME) aux_free_error();
			else {
				AUX_ADD_ERROR;
				if(pcalist) aux_free_PKList(&pcalist);
				return(-1);
			}
		}
		if (af_FCPath_is_trusted == TRUE) {	
			own_fcpath = af_pse_get_FCPath(NULLDNAME);
			if(!own_fcpath) {
				if(aux_last_error() == EOBJNAME) aux_free_error();
				else {
					AUX_ADD_ERROR;
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					return(-1);
				}
			}
		
			if(own_fcpath){
				reduced_fcpath = reduce_FCPath_to_HierarchyPath(own_fcpath);
				aux_free_FCPath(&own_fcpath);
				if(! reduced_fcpath){
					AUX_ADD_ERROR;
					aux_free_VerificationResult(&verifresult);
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					return(- 1);
				}
			}
		}

		if(call_af_verify == TRUE){

			/* Check user certificate against PKList */
			if(pklist){
				if (LookupPK(cert->tbs->subject, cert->tbs->subjectPK, pklist, time) != 0){
	
					verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
					verifresult->top_serial = aux_cpy_OctetString(cert->tbs->serialnumber);
					verifresult->trustedKey = 3;
	
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					/* verification successfully completed */
/*
					if(examine_VerificationResult(verifresult) < 0) {
						AUX_ADD_ERROR;
						return(-1);
					}
*/
					return(0);
				}
			}
		
			/* Check user certificate against FCPath */
			if (reduced_fcpath && af_FCPath_is_trusted == TRUE) {
				if (LookupPK_in_FCPath(cert->tbs->subject, cert->tbs->subjectPK, reduced_fcpath, time) != 0){
	
					verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
					verifresult->top_serial = aux_cpy_OctetString(cert->tbs->serialnumber);
					verifresult->trustedKey = 4;
	
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					/* verification successfully completed */
					if(examine_VerificationResult(verifresult) < 0) {
						AUX_ADD_ERROR;
						return(-1);
					}
					return(0);
				}
			}
		}
	}


	/* Check against revocation list */
	if (af_chk_crl){
		date = check_black_list(cert, time);
		if (! date) {
			if (aux_last_error() == EAVAILABLE)
				verifresult->verifstep[0] = create_verifstep(cert, CRL_NOT_AVAILABLE, (UTCTime * )0, (Validity *) 0, 0, FALSE, FALSE);
			else 
				verifresult->verifstep[0] = create_verifstep(cert, NOT_REVOKED, (UTCTime * )0, (Validity *) 0, 0, FALSE, FALSE);
		}
		else{
			if (aux_last_error() == EREVOKE) {
				verifresult->verifstep[0] = create_verifstep(cert, REVOKED, date, (Validity *) 0, 0, FALSE, FALSE);
				verifresult->success = FALSE;
				verifresult->trustedKey = - 1;
				if(freetime) {
					free(time);
					freetime = FALSE;
				}
				if(pcalist) aux_free_PKList(&pcalist);
				AUX_ADD_ERROR;
				return(- 1);
			}
			else if (aux_last_error() == EVALIDITY)
				verifresult->verifstep[0] = create_verifstep(cert, CRL_OUT_OF_DATE, date, (Validity *) 0, 0, FALSE, FALSE);
		}
		aux_free_error();
	}
	else 
		verifresult->verifstep[0] = create_verifstep(cert, NOT_REQUESTED, (UTCTime * )0, (Validity *) 0, 0, policy_CA, FALSE);


	path = or_cert->forwardpath; 
	count = certs_at_one_level(path);
	index = 0;

	while (count == 1) {

		index++;

		/* Verify with certificate of next level above */
		cert = path->liste->element;

		if(af_check_Validity){
			/* Check validity of certificate at next level above */
			if (af_check_validity_of_Certificate(time, cert)) {
				AUX_ADD_ERROR;
				valid = aux_cpy_Validity(cert->tbs->valid);
				verifresult->verifstep[index] = create_verifstep(cert, 0, (UTCTime *) 0, valid, 0, FALSE, FALSE);
				aux_free_Validity(&valid);
				verifresult->trustedKey = - 2;
				verifresult->success = FALSE;
	
				if(pklist) aux_free_PKList(&pklist);
				if(pcalist) aux_free_PKList(&pcalist);
				if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
	
				if(freetime) {
					free(time);
					freetime = FALSE;
				}
				return (- 1);
			}
		}

		/* Try to verify ... */

        	key.key = cert->tbs->subjectPK;  /* verification key of next level */
                key_serial = aux_cpy_OctetString(cert->tbs->serialnumber);

		/* needs HashInput parameter set */
		alghash = aux_ObjId2AlgHash(verifresult->verifstep[index - 1]->cert->sig->signAI->objid);
		if (alghash == SQMODN) 
			hashin = (HashInput * ) & key.key->subjectkey;
		else 
			hashin = (HashInput * ) 0;

		if (sec_verify(verifresult->verifstep[index - 1]->cert->tbs_DERcode, verifresult->verifstep[index - 1]->cert->sig, SEC_END, &key, hashin) < 0) {
			if(aux_last_error() == EVERIFICATION) aux_add_error(EVERIFICATION, "Verification of one certificate of path failed", (char *) verifresult->verifstep[index - 1]->cert, Certificate_n, proc);
			else AUX_ADD_ERROR;

			verifresult->verifstep[index] = create_verifstep(cert, 0, (UTCTime * )0, (Validity *) 0, 0, FALSE, FALSE);

			verifresult->success = FALSE;

			if(pklist) aux_free_PKList(&pklist);
			if(pcalist) aux_free_PKList(&pcalist);
			if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
			if(freetime) {
				free(time);
				freetime = FALSE;
			}
			return (- 1);
		}


		/* Check certificate against PCAList */
		if (LookupPK(cert->tbs->subject, cert->tbs->subjectPK, pcalist, time) != 0) {
			verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
			verifresult->top_serial = aux_cpy_OctetString(cert->tbs->serialnumber);
			verifresult->trustedKey = 6;
			if(pklist) aux_free_PKList(&pklist);
			if(pcalist) aux_free_PKList(&pcalist);
			if(freetime) {
				free(time);
				freetime = FALSE;
			}
			/* verification successfully completed */
			if(examine_VerificationResult(verifresult) < 0) {
				AUX_ADD_ERROR;
				return(-1);
			}
			return(0);
		}

		/* Check for PEM subordination, if required */

		if (chk_PEM_subordination == TRUE) {
			ret = aux_checkPemDNameSubordination(verifresult->verifstep[index - 1]->cert->tbs->issuer,
							    verifresult->verifstep[index - 1]->cert->tbs->subject);

			if (ret == - 1) {
				if(pklist) aux_free_PKList(&pklist);
				if(pcalist) aux_free_PKList(&pcalist);
				if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
				if(freetime) {
					free(time);
					freetime = FALSE;
				}
				AUX_ADD_ERROR;
				return (- 1);
			}
			if (ret == FALSE) {
				/* 'subject' NOT subordinate to 'issuer'                   */
				/* Check whether issuer's cert can be verified by 'pkroot' */
				ret = verify_CertificateWithPkroot(cert, pkroot);
				if (ret < 0) {
					verifresult->verifstep[index - 1]->dn_subordination_violation = TRUE;
					verifresult->trustedKey = - 4;		
					if(aux_last_error() == EVERIFICATION || aux_last_error() == EROOTKEY) aux_add_error(EINVALID, "Verification of certification path failed because the DName subordination is violated and Certificate can't be verified with PKRoot", (char *)cert, Certificate_n, proc);
					else AUX_ADD_ERROR;
				}
				else {
					/* 'subject' is a Policy CA. Therefore, naming subordination */
					/* may be violated.					     */
					policy_CA = TRUE;
				}
			}
		}  /* chk_PEM_subordination */					    


		/* Check certificate at next level above against revocation list */

		date = (UTCTime *)0;
		usercert_crlcheck = 0;
		if (af_chk_crl == TRUE){
			date = check_black_list(cert, time);
			if (! date) {
				if (aux_last_error() == EAVAILABLE)
					usercert_crlcheck = CRL_NOT_AVAILABLE;
				else 
					usercert_crlcheck = NOT_REVOKED;
			}
			else{
				if (aux_last_error() == EREVOKE) {
					verifresult->verifstep[index] = create_verifstep(cert, REVOKED, date, (Validity *) 0, 0, policy_CA, FALSE);
					verifresult->success = FALSE;
					verifresult->trustedKey = - 1;
		
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);

					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					AUX_ADD_ERROR;
					return(- 1);
				}
				else if (aux_last_error() == EVALIDITY)
					usercert_crlcheck = CRL_OUT_OF_DATE; 
			}
			aux_free_error();
		}
		else {
			usercert_crlcheck = NOT_REQUESTED;

			/* Check against PKList */
			if(pklist){
				if (LookupPK(cert->tbs->subject,
					      cert->tbs->subjectPK, 
					      pklist, time) != 0) {
	
					verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
					verifresult->top_serial = aux_cpy_OctetString(cert->tbs->serialnumber);
					verifresult->trustedKey = 3;
	
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					/* verification successfully completed */
					if(examine_VerificationResult(verifresult) < 0) {
						AUX_ADD_ERROR;
						return(-1);
					}
					return(0);
				}
			}
	
			/* Check against own FCPath */
			if (reduced_fcpath && af_FCPath_is_trusted == TRUE) {
				if (LookupPK_in_FCPath(cert->tbs->subject,
					      cert->tbs->subjectPK, 
					      reduced_fcpath, time) != 0) {
	
					verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
					verifresult->top_serial = aux_cpy_OctetString(cert->tbs->serialnumber);
					verifresult->trustedKey = 4;
	
					if(pklist) aux_free_PKList(&pklist);
					if(pcalist) aux_free_PKList(&pcalist);
					if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
					if(freetime) {
						free(time);
						freetime = FALSE;
					}
					/* verification successfully completed */
					if(examine_VerificationResult(verifresult) < 0) {
						AUX_ADD_ERROR;
						return(-1);
					}
					return(0);
				}
			}
		}

		verifresult->verifstep[index] = create_verifstep(cert, usercert_crlcheck, (UTCTime * )0, (Validity *) 0, 0, policy_CA, FALSE);

		path = path->next_forwardpath;
		count = certs_at_one_level(path);
		policy_CA = dn_subordination_violation = FALSE;

	}  /* while */


	/* verifresult->verifstep[index]->cert is:	        				              */
	/* either top-level certificate								              */
	/* or the one and only certificate at the highest hierarchy level which contains one certificate only.*/
	/* It will be checked as usercertificate in get_path()				 	              */

	if (index > 0){
		/* originator certificate has been reduced */
		reduced_or_cert = (Certificates *)malloc(sizeof(Certificates) );
		if (! reduced_or_cert) {
			aux_add_error(EMALLOC, "reduced_or_cert", CNULL, 0, proc);
			if(pklist) aux_free_PKList(&pklist);
			if(pcalist) aux_free_PKList(&pcalist);
			if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);
			if(freetime) {
				free(time);
				freetime = FALSE;
			}
			return (- 1);
		}
	
		reduced_or_cert->usercertificate = verifresult->verifstep[index]->cert;
		reduced_or_cert->forwardpath = path;

		c = get_path(reduced_or_cert, pkroot, time, &certList, &topkey, &topkey_serial, FALSE);
	}
	else
		c = get_path(or_cert, pkroot, time, &certList, &topkey, &topkey_serial, TRUE);


	if (pkroot_provided == TRUE && (verifresult->trustedKey == 0 || verifresult->trustedKey == 1))
		verifresult->trustedKey = 2;

	if (c < 0) {
		verifresult->success = FALSE;

                if(pkroot_mall) aux_free_PKRoot(&pkroot);

		if (aux_last_error() == EPATH){
			verifresult->trustedKey = - 3;
			verifresult->top_name = aux_DName2Name(cert->tbs->issuer);
		}

		if(freetime) {
			free(time);
			freetime = FALSE;
		}
		AUX_ADD_ERROR;
		return(- 1);
	}


        /* verify the chain of certificates in certList */

	/* N O  V A L I D I T Y  checks to be performed at this stage */ 

	/* The validity time frames of the certificates in certList have already been checked by the */
	/* routines certselect() and complete_FCPath_from_Directory().				     */
	/* If certselect() or complete_FCPath_from_Directory() find a certificate to have expired,   */
	/* they consider the "next" certificate available. If there is no "next" certificate 	     */
	/* available, they return error code EPATH.						     */

        for (cc = 1; cc <= c; ++cc, index++   ) {

		if(superior_CA_is_policyCA == TRUE) {
			policy_CA = TRUE;
			superior_CA_is_policyCA = FALSE;
		}
		supplied = 0;
		crlcheck = 0;
		date = (UTCTime *)0;

		C = certList[cc-1];   /* certificate to be verified */

		if (cc > 1){	
			if (certs_from_directory == FALSE) 
				supplied = 0;
			else if (cc == c && crosscert_appended_to_certs == TRUE) {
				if (crosscert_from_Directory == TRUE) supplied = 1;
				else if (crosscert_from_PSE == TRUE) supplied = 2;
			}
			else supplied = 1;
	
			if (af_chk_crl){
				date = check_black_list(C, time);
				if (! date) {
					if (aux_last_error() == EAVAILABLE)
						crlcheck = CRL_NOT_AVAILABLE;
					else 
						crlcheck = NOT_REVOKED;
				}
				else{
					if (aux_last_error() == EREVOKE) {
						verifresult->success = FALSE;
						verifresult->trustedKey = - 1;
						verifresult->verifstep[index] =
							create_verifstep(C, REVOKED, date, (Validity *) 0, supplied, policy_CA, FALSE);
						rc = - 1;
						break;
					}
					else if (aux_last_error() == EVALIDITY)
						crlcheck = CRL_OUT_OF_DATE;
				}
			}
			else crlcheck = NOT_REQUESTED;

			verifresult->verifstep[index] = create_verifstep(C, crlcheck, date, (Validity *) 0, supplied, policy_CA, FALSE);
		} /* if */

		CC = certList[cc];    /* certificate containing the verification key for C */
                if(cc == c) {
                	key.key = &topkey;               /* top verification key */
                        key_serial = aux_cpy_OctetString(topkey_serial);
			verifresult->top_serial = aux_cpy_OctetString(topkey_serial);
                }
                else {
        		key.key = CC->tbs->subjectPK;    /* verification key of next level */
                        key_serial = aux_cpy_OctetString(CC->tbs->serialnumber);
                }

		/* needs HashInput parameter set */
		alghash = aux_ObjId2AlgHash(C->sig->signAI->objid);
		if (alghash == SQMODN) 
			hashin = (HashInput * ) & key.key->subjectkey;
		else 
			hashin = (HashInput * ) 0;

		if (sec_verify(C->tbs_DERcode, C->sig, SEC_END, &key, hashin) < 0) {
			aux_add_error(EVERIFICATION, 
				      "Verification of one certificate of path failed", 
				      (char *) C, Certificate_n, proc);

			verifresult->success = FALSE;
			verifresult->top_serial = aux_cpy_OctetString(key_serial);

			if(cc < c) {
				verifresult->top_name = aux_DName2Name(CC->tbs->subject);

				/* Add further VerificationStep */
				index++;

				if(certs_from_directory == FALSE) supplied = 0;
				else if (cc == c - 1 && crosscert_appended_to_certs == TRUE) {
					if (crosscert_from_Directory == TRUE) supplied = 1;
					else if (crosscert_from_PSE == TRUE) supplied = 2;
				}
				else supplied = 1;
	
				verifresult->verifstep[index] = create_verifstep(CC, 0, (UTCTime * )0, (Validity *) 0, supplied, FALSE, FALSE);

				verifresult->trustedKey = - 1;
			}
			else { /* c == cc */
				verifresult->top_name = aux_DName2Name(C->tbs->issuer);
			}
			rc = - 1;
                        break;
		}  /* sec_verify < 0 */

		/* Check for PEM subordination, if required */

		if (chk_PEM_subordination == TRUE && cc < c) {
			/* We are aware that condition (cc < c) reduces the validity of the name subordination check. */
			ret = aux_checkPemDNameSubordination(C->tbs->issuer, C->tbs->subject);
			if (ret == - 1) {
				AUX_ADD_ERROR;
				aux_free_VerificationResult(&verifresult);
				rc = - 1;
				break;
			}
			if (ret == FALSE) {
				/* 'subject' of certificate C NOT subordinate to 'issuer' of C        */
				/* Check whether issuer's cert (i.e., CC) can be verified by 'pkroot' */
				ret = verify_CertificateWithPkroot(CC, pkroot);
				if (ret < 0) {
					dn_subordination_violation = TRUE;
					verifresult->trustedKey = - 4;
					if(aux_last_error() == EVERIFICATION || aux_last_error() == EROOTKEY) aux_add_error(EINVALID, "Verification of certification path failed because the DName subordination is violated and Certificate can't be verified with PKRoot", (char *)CC, Certificate_n, proc);
				}
				else{
					/* 'subject' of certificate CC is a Policy CA. Therefore, naming  */
					/* subordination may be violated.			          */
					superior_CA_is_policyCA = TRUE;
				}
			}
		}  /* chk_PEM_subordination */

		if(cc > 1 && dn_subordination_violation == TRUE){
			verifresult->verifstep[index]->dn_subordination_violation = dn_subordination_violation;
		}

		dn_subordination_violation = FALSE;
		policy_CA = FALSE;

	}  /* for */


	aux_free2_KeyInfo(&topkey);
	if (certList) free(certList);
        if(pkroot_mall) aux_free_PKRoot(&pkroot);
	if(pklist) aux_free_PKList(&pklist);
	if(pcalist) aux_free_PKList(&pcalist);
	if(reduced_fcpath) aux_free_FCPath(&reduced_fcpath);

	if(freetime) {
		free(time);
		freetime = FALSE;
	}
	if(rc < 0) return(-1);

	if(examine_VerificationResult(verifresult) < 0) {
		AUX_ADD_ERROR;
		return(-1);
	}
	return(0);

}	/* af_verify_Certificates() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure af_check_validity_of_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC af_check_validity_of_Certificate(
	UTCTime		 *time,
	Certificate	 *cert
)

#else

RC af_check_validity_of_Certificate(
	time,
	cert
)
UTCTime		 *time;
Certificate	 *cert;

#endif

{
	char	*proc = "af_check_validity_of_Certificate";

	if(! cert || ! cert->tbs || ! cert->tbs->valid) {
		aux_add_error(EINVALID, "Invalid Parameter", CNULL, 0, proc);
		return (- 1);
	}
	if (aux_interval_UTCTime(time, cert->tbs->valid->notbefore, cert->tbs->valid->notafter)) {
		aux_add_error(EVALIDITY, "Certificate has expired", (char *) cert, Certificate_n, proc);
		return (- 1);
	}
	return 0;
}



/***************************************************************
 *
 * Procedure af_check_validity_of_PKRoot
 *
 ***************************************************************/
#ifdef __STDC__

RC af_check_validity_of_PKRoot(
	UTCTime	 *time,
	PKRoot	 *pkroot
)

#else

RC af_check_validity_of_PKRoot(
	time,
	pkroot
)
UTCTime	 *time;
PKRoot	 *pkroot;

#endif

{
	char	*proc = "af_check_validity_of_PKRoot";


	if(! pkroot || ! pkroot->newkey || ! pkroot->newkey->valid) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (- 1);
	}

	if (pkroot->newkey->valid->notbefore && pkroot->newkey->valid->notafter) {
		if (aux_interval_UTCTime(time, pkroot->newkey->valid->notbefore, pkroot->newkey->valid->notafter)) {
			aux_add_error(EVALIDITY, "PKRoot has expired", (char *) pkroot, PKRoot_n, proc);
			return (- 1);
		}
	}
	return 0;
}



/***************************************************************
 *
 * Procedure af_check_validity_of_ToBeSigned
 *
 ***************************************************************/
#ifdef __STDC__

RC af_check_validity_of_ToBeSigned(
	UTCTime		 *time,
	ToBeSigned	 *tbs
)

#else

RC af_check_validity_of_ToBeSigned(
	time,
	tbs
)
UTCTime		 *time;
ToBeSigned	 *tbs;

#endif

{
	char	*proc = "af_check_validity_of_ToBeSigned";

	if(! tbs || ! tbs->valid) {
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return (- 1);
	}
	if (tbs->valid->notbefore && tbs->valid->notafter) {
		if (aux_interval_UTCTime(time, tbs->valid->notbefore, tbs->valid->notafter)) {
			aux_add_error(EVALIDITY, "TBS has expired", (char *) tbs, ToBeSigned_n, proc);
			return (- 1);
		}
	}
	return 0;
}

/*********************************************************************************************/


/***************************************************************
 *
 * Procedure reduce_FCPath_to_HierarchyPath
 *
 ***************************************************************/
#ifdef __STDC__

FCPath *reduce_FCPath_to_HierarchyPath(
	FCPath	 *fpath
)

#else

FCPath *reduce_FCPath_to_HierarchyPath(
	fpath
)
FCPath	 *fpath;

#endif

{
	SET_OF_Certificate * certset;
	FCPath		   * ret = (FCPath * )0, * tmp_fpath;

	char	           * proc = "reduce_FCPath_to_HierarchyPath";


	if (! fpath){
		aux_add_error(EINVALID, "No fcpath provided", CNULL, 0, proc);
		return((FCPath * )0);
	}

	certset = fpath->liste;
	if(! certset || ! certset->element){
		aux_add_error(EINVALID, "No certificates provided at very first level", CNULL, 0, proc);
		return((FCPath * ) 0);
	}

	if(! own_pkroot){
		own_pkroot = af_pse_get_PKRoot();
		if(! own_pkroot) {
			AUX_ADD_ERROR;
			return((FCPath * )0);
		}
	}

	for (fpath = fpath->next_forwardpath; fpath; fpath = fpath->next_forwardpath){
		if(! fpath->liste || ! fpath->liste->element){
			aux_add_error(EINVALID, "FCPath has level without any certificates", CNULL, 0, proc);
			if(ret) aux_free_FCPath(&ret);
			return ((FCPath * ) 0);
		}
		while (certset && 
	               aux_cmp_DName(certset->element->tbs->issuer, fpath->liste->element->tbs->subject))
		        /* different */
			certset = certset->next;

		if( ! certset){
			aux_add_error(EINVALID, "No hierarchy certificate in FCPath", CNULL, 0, proc);
			if(ret)
				aux_free_FCPath(&ret);
			return ((FCPath * ) 0);
		}

		if(! ret){
			ret = tmp_fpath = (FCPath * )malloc(sizeof(FCPath));
			if(! ret) {
				aux_add_error(EMALLOC, "ret", CNULL, 0, proc);
				return ((FCPath * ) 0);
			}
			ret->liste = (SET_OF_Certificate * )0;
			ret->next_forwardpath = (FCPath * )0;
		}
		else{
			tmp_fpath->next_forwardpath = (FCPath * )malloc(sizeof(FCPath));
			if(! tmp_fpath->next_forwardpath) {
				aux_add_error(EMALLOC, "tmp_fpath->next_forwardpath", CNULL, 0, proc);
				aux_free_FCPath(&ret);
				return ((FCPath * ) 0);
			}
			tmp_fpath = tmp_fpath->next_forwardpath;
		}

		tmp_fpath->liste = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate));
		if(! tmp_fpath->liste) {
			aux_add_error(EMALLOC, "tmp_fpath->liste", CNULL, 0, proc);
			aux_free_FCPath(&ret);
			return ((FCPath * ) 0);
		}
		tmp_fpath->next_forwardpath = (FCPath * )0;
		tmp_fpath->liste->element = aux_cpy_Certificate(certset->element);
		tmp_fpath->liste->next = (SET_OF_Certificate * )0;
 
		certset = fpath->liste;
	} /* for */
	
	
	/* certset points to the top level of fpath */
	/* check whether fpath terminates at pkroot */

	while (certset && aux_cmp_DName(certset->element->tbs->issuer, own_pkroot->ca))
		certset = certset->next;

	if( ! certset){
		aux_add_error(EINVALID, "No hierarchy certificate in FCPath", CNULL, 0, proc);
		aux_free_FCPath(&ret);
		return ((FCPath * ) 0);
	}

	if(! ret){  /* fpath contains one level only */
		ret = (FCPath * )malloc(sizeof(FCPath));
		if(! ret) {
			aux_add_error(EMALLOC, "ret", CNULL, 0, proc);
			return ((FCPath * ) 0);
		}
		ret->liste = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate));
		if(! ret->liste) {
			aux_add_error(EMALLOC, "ret->liste", CNULL, 0, proc);
			aux_free_FCPath(&ret);
			return ((FCPath * ) 0);
		}
		ret->next_forwardpath = (FCPath * )0;
		ret->liste->element = aux_cpy_Certificate(certset->element);
		ret->liste->next = (SET_OF_Certificate * )0;

		return(ret);
	}


	tmp_fpath->next_forwardpath = (FCPath * )malloc(sizeof(FCPath));
	if(! tmp_fpath->next_forwardpath) {
		aux_add_error(EMALLOC, "tmp_fpath->next_forwardpath", CNULL, 0, proc);
		aux_free_FCPath(&ret);
		return ((FCPath * ) 0);
	}
	tmp_fpath = tmp_fpath->next_forwardpath;

	tmp_fpath->liste = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate));
	if(! tmp_fpath->liste) {
		aux_add_error(EMALLOC, "tmp_fpath->liste", CNULL, 0, proc);
		aux_free_FCPath(&ret);
		return ((FCPath * ) 0);
	}
	tmp_fpath->next_forwardpath = (FCPath * )0;
	tmp_fpath->liste->element = aux_cpy_Certificate(certset->element);
	tmp_fpath->liste->next = (SET_OF_Certificate * )0;


	return(ret);

}	/* reduce_FCPath_to_HierarchyPath() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure transform_reducedFCPath_into_Certificates
 *
 ***************************************************************/
#ifdef __STDC__

Certificates *transform_reducedFCPath_into_Certificates(
	FCPath	 *fpath
)

#else

Certificates *transform_reducedFCPath_into_Certificates(
	fpath
)
FCPath	 *fpath;

#endif

{
	/* Each level of fpath contains one certificate only */

	Certificates 	   * certs;
	SET_OF_Certificate * certset;
	FCPath		   * certpath = (FCPath * )0;
	char	           * proc = "transform_FCPath_into_Certificates";
	

	if (! fpath){
		aux_add_error(EINVALID, "no parameter", CNULL, 0, proc);
		return((Certificates * )0);
	}

	certset = fpath->liste;
	if(! certset || ! certset->element){
		aux_add_error(EINVALID, "No certificates provided at very first level", CNULL, 0, proc);
		return((Certificates * ) 0);
	}

	if(certset->next){
		aux_add_error(EINVALID, "More than one certificate in FCPath at very first level", CNULL, 0, proc);
		return((Certificates * ) 0);
	}

	certs = (Certificates * )malloc(sizeof(Certificates));
	if(! certs) {
		aux_add_error(EMALLOC, "certs", CNULL, 0, proc);
		return ((Certificates * ) 0);
	}

	certs->usercertificate = aux_cpy_Certificate(fpath->liste->element);
	certs->forwardpath = (FCPath * )0;


	for (fpath = fpath->next_forwardpath; fpath; fpath = fpath->next_forwardpath){
		certset = fpath->liste;
		if(! certset || ! certset->element){
			aux_add_error(EINVALID, "FCPath has level without any certificates", CNULL, 0, proc);
			aux_free_Certificates(&certs);
			return((Certificates * ) 0);
		}
	
		if(certset->next){
			aux_add_error(EINVALID, "More than one certificate in FCPath provided at one level", CNULL, 0, proc);
			aux_free_Certificates(&certs);
			return((Certificates * ) 0);
		}

		if(! certs->forwardpath){
			certs->forwardpath = certpath = (FCPath * )malloc(sizeof(FCPath));
			if(! certs->forwardpath) {
				aux_add_error(EMALLOC, "certs->forwardpath", CNULL, 0, proc);
				aux_free_Certificates(&certs);
				return ((Certificates * ) 0);
			}
			certpath->liste = (SET_OF_Certificate * )0;
			certpath->next_forwardpath = (FCPath * )0;
		}
		else{
			certpath->next_forwardpath = (FCPath * )malloc(sizeof(FCPath));
			if(! certpath->next_forwardpath) {
				aux_add_error(EMALLOC, "certpath->next_forwardpath", CNULL, 0, proc);
				aux_free_Certificates(&certs);
				return ((Certificates * ) 0);
			}
			certpath = certpath->next_forwardpath;
		}

		certpath->liste = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate));
		if(! certpath->liste) {
			aux_add_error(EMALLOC, "certpath->liste", CNULL, 0, proc);
			aux_free_Certificates(&certs);
			return ((Certificates * ) 0);
		}
		certpath->next_forwardpath = (FCPath * )0;
		certpath->liste->element = aux_cpy_Certificate(certset->element);
		certpath->liste->next = (SET_OF_Certificate * )0;
 	} /* for */

	return(certs);

}	/* transform_reducedFCPath_into_Certificates() */



/***************************************************************
 *
 * Procedure CRL2Crl
 *
 ***************************************************************/
#ifdef __STDC__

Crl *CRL2Crl(
	CRL	 *crl
)

#else

Crl *CRL2Crl(
	crl
)
CRL	 *crl;

#endif

{
	Crl                  * crlpse;
	SEQUENCE_OF_CRLEntry  * revokedCertificates, * seq;
	char	                * proc = "CRL2Crl";

	if(! crl){
		aux_add_error(EINVALID, "no parameter", CNULL, 0, proc);
		return ((Crl *)0);
	}

	crlpse = (Crl *)malloc(sizeof(Crl));
	if( !crlpse ) {
		aux_add_error(EMALLOC, "crlpse", CNULL, 0, proc);
		return ((Crl *)0);
	}
	crlpse->issuer = aux_cpy_DName(crl->tbs->issuer);
	crlpse->nextUpdate = aux_cpy_Name(crl->tbs->nextUpdate);
	revokedCertificates = crl->tbs->revokedCertificates;
	if (!revokedCertificates) {
		crlpse->revcerts = (SEQUENCE_OF_CRLEntry *)0;
		return (crlpse);
	}

	crlpse->revcerts = seq = (SEQUENCE_OF_CRLEntry *)malloc(sizeof(SEQUENCE_OF_CRLEntry));
	if( !crlpse->revcerts ) {
		aux_add_error(EMALLOC, "crlpse->revcerts", CNULL, 0, proc);
		aux_free_Crl (&crlpse);
		return ((Crl *)0);
	}

	/* copy first element: */
	crlpse->revcerts->element = aux_cpy_CRLEntry(revokedCertificates->element);
	crlpse->revcerts->next = (SEQUENCE_OF_CRLEntry *)0;
	revokedCertificates = revokedCertificates->next;
	while (revokedCertificates) {
		seq->next = (SEQUENCE_OF_CRLEntry * )malloc(sizeof(SEQUENCE_OF_CRLEntry));
		if(! seq->next) {
			aux_add_error(EMALLOC, "seq->next", CNULL, 0, proc);
			aux_free_Crl (&crlpse);
			return ((Crl *)0);
		}
		seq = seq->next;
		seq->next = (SEQUENCE_OF_CRLEntry * )0;
		seq->element = aux_cpy_CRLEntry(revokedCertificates->element);
		revokedCertificates = revokedCertificates->next;
	}

	return (crlpse);

} 	 /* CRL2Crl() */











/*--------------------------------------------------------------------------------------------*/
/*--------------------------- I N T E R N A L    F U N C T I O N S ---------------------------*/
/*--------------------------------------------------------------------------------------------*/




/*********************************************************************************************/


/***************************************************************
 *
 * Procedure certselect
 *
 ***************************************************************/
#ifdef __STDC__

static int certselect(
	Certificate	 *cSTK[],
	int		  cx,
	struct PathSTK	 *pathelem,
	UTCTime		 *time
)

#else

static int certselect(
	cSTK,
	cx,
	pathelem,
	time
)
Certificate	 *cSTK[];
int		  cx;
struct PathSTK	 *pathelem;
UTCTime		 *time;

#endif

{
	/* select a certificate from crosscertificates in pathelem
	 * and put it at the end of the stack cSTK.
	 */
	DName             * issuer;
	CrossCertificates * cross;
	char	          * proc = "certselect";

	issuer = cSTK[cx]->tbs->issuer;
	for (cross = pathelem->crosscertificates; cross; cross = cross->next) 
		{ /* check subject and validity */
		if (!aux_cmp_DName(issuer, cross->element->tbs->subject) && 
		    (af_check_Validity == FALSE || !af_check_validity_of_Certificate(time, cross->element)))
			break;
	}
	if (cross) {
		pathelem->crosscertificates = cross->next;
		cSTK[++cx] = cross->element;
		return 1;
	}

	pathelem->crosscertificates = (CrossCertificates *) 0;
	return 0;
}


/*********************************************************************************************/


/* -finish cleans up temporary data */
/***************************************************************
 *
 * Procedure get_path_finish
 *
 ***************************************************************/
#ifdef __STDC__

static void get_path_finish(
	struct PathSTK	 *fstk,
	Certificate	**cstk,
	PKList		 *pklist
)

#else

static void get_path_finish(
	fstk,
	cstk,
	pklist
)
struct PathSTK	 *fstk;
Certificate	**cstk;
PKList		 *pklist;

#endif

{
	char	* proc = "get_path_finish";

	if (fstk) free(fstk);
	if (cstk) free(cstk);
	if (pklist) aux_free_PKList(&pklist);
	return;
}


/*********************************************************************************************/


    /* get_path returns in certlist the chain of certificates to be verified. Return parameter c 
       indicates the number of certificates in the chain. 

       certlist[0]   is the user certificate.
       certlist[c-1] is the certificate which is to be verified by the top level verification key
		     returned in topkey. 
       topkey        is either pkroot (which in turn is either given to af_verify via parameter pkroot
				       or is taken from PSE object PKRoot) 
		     or a key found in PSE object PKList.
    */



/***************************************************************
 *
 * Procedure get_path
 *
 ***************************************************************/
#ifdef __STDC__

static int get_path(
	Certificates	 *orig_cert,
	PKRoot		 *rootinfo,
	UTCTime		 *time,
	Certificate	**cSTKp[],
	KeyInfo		 *topkey,
	OctetString	**topkey_serial,
	Boolean		  usercert
)

#else

static int get_path(
	orig_cert,
	rootinfo,
	time,
	cSTKp,
	topkey,
	topkey_serial,
	usercert
)
Certificates	 *orig_cert;
PKRoot		 *rootinfo;
UTCTime		 *time;
Certificate	**cSTKp[];
KeyInfo		 *topkey;
OctetString	**topkey_serial;
Boolean		  usercert;

#endif

{
	/*
	 * get path fills the certificate stack with the useable cross
	 * certificates from 'forwardpath' in orig_cert. They are selected
	 * with information from the certificate in orig_cert. If a PKList
	 * exists this information may be used to cut off the path up to an
	 * public key found in that list (may be the public key of the
	 * certificate in orig_cert itself).
	 */

	int	                 i, cindex = 0, cnt;
	FCPath                 * fpath, * path;
	Certificate           ** cSTK;
	Certificate           ** dir_cSTK;  /* stack of certificates retrieved from directory */
	Certificate	       * cert;
	struct PathSTK         * FCstk, * p;
	PKList	               * pklist = (PKList * )0;
	SET_OF_Certificate     * certpath, * certpath_tmp;
	int 		       * level;
	int 			 cmp;

	char	               * proc = "get_path";

	if (cSTKp)  *cSTKp = (Certificate * *) 0;     /* init return value */
	if (!(orig_cert && cSTKp && topkey))  {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return - 1;
	}
	/* Validity time frame of orig_cert->usercertificate has already been checked */

	/* allocate array of cross certificates */
	for (cnt = 0, path = orig_cert->forwardpath; 
	    path; 
	    cnt++, path = path->next_forwardpath
	    ) /* count */;

	cSTK = (Certificate **) calloc(cnt + 2, sizeof(Certificate * )); 
	/* cSTK[0] shall hold usercertificate (+1), cSTK[cnt] shall hold potential cross-certificate (+1) */

	if (! cSTK) {
		aux_add_error(EMALLOC, "cSTK", CNULL, 0, proc);
		return - 1;
	}
	if (cnt > 0) {
		FCstk = (struct PathSTK *) calloc(cnt, sizeof(struct PathSTK ));
		if (!FCstk) {
			free(cSTK);
			aux_add_error(EMALLOC, "FCstk", CNULL, 0, proc);
			return - 1;
		}
	} 
	else FCstk = (struct PathSTK *) 0;

	*cSTKp = cSTK;
	cSTK[0] = orig_cert->usercertificate;
	fpath = orig_cert->forwardpath;

	pklist = af_pse_get_PKList(SIGNATURE);
	if(! pklist)
		aux_free_error();

	/* check first certificate */
	if(usercert == FALSE && af_chk_crl == FALSE){
		/* check if subjectkey may be found in PKList */
		if ( LookupPK(cSTK[0]->tbs->subject, cSTK[0]->tbs->subjectPK, pklist, time) != 0 ) {
			aux_cpy2_KeyInfo(topkey, cSTK[0]->tbs->subjectPK);
			*topkey_serial = aux_cpy_OctetString(cSTK[0]->tbs->serialnumber);
			verifresult->trustedKey = 3;
			verifresult->top_serial = aux_cpy_OctetString(cSTK[0]->tbs->serialnumber);
			verifresult->top_name = aux_DName2Name(cSTK[0]->tbs->issuer);
			get_path_finish(FCstk, cSTK, pklist);
			*cSTKp = (Certificate * *) 0;
			return  0;      /* no certificates to check */
		}
	
		/* check if subjectkey may be found in FCPath */
		if (reduced_fcpath && af_FCPath_is_trusted == TRUE) {
			if ( LookupPK_in_FCPath(cSTK[0]->tbs->subject, cSTK[0]->tbs->subjectPK, reduced_fcpath, time) != 0 ) {
				aux_cpy2_KeyInfo(topkey, cSTK[0]->tbs->subjectPK);
				*topkey_serial = aux_cpy_OctetString(cSTK[0]->tbs->serialnumber);
				verifresult->trustedKey = 4;
				verifresult->top_serial = aux_cpy_OctetString(cSTK[0]->tbs->serialnumber);
				verifresult->top_name = aux_DName2Name(cSTK[0]->tbs->issuer);
				get_path_finish(FCstk, cSTK, pklist);
				*cSTKp = (Certificate * *) 0;
				return  0;      /* no certificates to check */
			}
		}
	} /* if */

	/* check for issuer as Root CA */
	if (aux_cmp_DName(rootinfo->ca, cSTK[0]->tbs->issuer) == 0) { /* issuer is rootCA */
		int	rc = 1;
/* !! newkey/oldkey handling to be implemented */
		if (1 || ((cmp = aux_cmp_OctetString(cSTK[0]->tbs->serialnumber, rootinfo->newkey->serial)) == 1
		      || !cmp)) {
			/* newkey */
			aux_cpy2_KeyInfo(topkey, rootinfo->newkey->key);
                        *topkey_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
			verifresult->trustedKey = 0;
			verifresult->top_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
			verifresult->top_name = aux_DName2Name(rootinfo->ca);

			/* Check whether time (time and date of signature) lies within the validity */
			/* time frame of rootinfo						    */
			if(af_check_Validity){
				if (af_check_validity_of_PKRoot(time, rootinfo)) {
					/* Root Info has expired ! */
					AUX_ADD_ERROR;
					verifresult->valid = aux_cpy_Validity(rootinfo->newkey->valid);
					verifresult->success = FALSE;
				}
			}
		} 
		else if ((cmp = aux_cmp_OctetString(cSTK[0]->tbs->serialnumber, rootinfo->oldkey->serial)) == 1 || !cmp) {
			/* oldkey */
			aux_cpy2_KeyInfo(topkey, rootinfo->oldkey->key);
                        *topkey_serial = aux_cpy_OctetString(rootinfo->oldkey->serial);
			verifresult->trustedKey = 1;
			verifresult->top_serial = aux_cpy_OctetString(rootinfo->oldkey->serial);
			verifresult->top_name = aux_DName2Name(rootinfo->ca);

			/* Check whether time (time and date of signature) lies within the validity */
			/* time frame of rootinfo						    */
			if(af_check_Validity){
				if (af_check_validity_of_PKRoot(time, rootinfo)) {
					/* Root Info has expired ! */
					AUX_ADD_ERROR;
					verifresult->valid = aux_cpy_Validity(rootinfo->oldkey->valid);
					verifresult->success = FALSE;
				}
			}
		} 
		else {
			aux_add_error(EROOTKEY, "Can't verify certificate with PKRoot: needs lower serial number", (char *) cSTK[0], Certificate_n, proc);
			get_path_finish(FCstk, cSTK, pklist);
			verifresult->trustedKey = - 1;
			return(-1);
		}
		get_path_finish(FCstk, (Certificate **)0, pklist);
		return  rc;
	}

	/* check if issuer of "orig_cert->usercertificate" is cross-certified by own Root CA */
	cert = check_CrossCertificates (rootinfo->ca, cSTK[0]->tbs->issuer);
	if (cert){
		int	rc = 2;
		crosscert_appended_to_certs = TRUE;
		cSTK[1] = aux_cpy_Certificate(cert);
		aux_free_Certificate(&cert);
		aux_cpy2_KeyInfo(topkey, rootinfo->newkey->key);
		*topkey_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
		get_path_finish(FCstk, (Certificate **)0, pklist);
		verifresult->trustedKey = 0;
		verifresult->top_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
		verifresult->top_name = aux_DName2Name(rootinfo->ca);

		/* Check whether time (time and date of signature) lies within the validity */
		/* time frame of rootinfo						    */
		if(af_check_Validity){
			if (af_check_validity_of_PKRoot(time, rootinfo)) {
				/* Root Info has expired ! */
				AUX_ADD_ERROR;
				verifresult->valid = aux_cpy_Validity(rootinfo->newkey->valid);
				verifresult->success = FALSE;
			}
		}

		return(rc);
	}

	if((! fpath)){
		if (af_access_directory == TRUE)
			goto establish_certpath_from_directory;
		else {
			aux_add_error(EPATH, "Can't get path because the chain of certificates is incomplete", CNULL, 0, proc);
			get_path_finish(FCstk, cSTK, pklist);
			return(- 1);
		}
	}

	cindex = 0;
	p = &FCstk[cindex++];
	p->crosscertificates = fpath->liste;
	p->forwardpath = fpath;
	/* scan forwardpath list and set cSTK       */
	while (p->crosscertificates) {
		int	rcselect;

		fpath = fpath->next_forwardpath;

		while ((rcselect = certselect(cSTK, cindex - 1, p, time))) {
			/* check this for issuer root CA
			   or if found in pklist and return
			*/

			if (af_chk_crl == FALSE) {
				/* check if subjectkey is found in PKList */
				if ( LookupPK(cSTK[cindex]->tbs->subject, cSTK[cindex]->tbs->subjectPK, pklist, time) != 0 ) {
					get_path_finish(FCstk, (Certificate **)0, pklist);
					aux_cpy2_KeyInfo(topkey, cSTK[cindex]->tbs->subjectPK);
					*topkey_serial = aux_cpy_OctetString(cSTK[cindex]->tbs->serialnumber);
					verifresult->trustedKey = 3;
					verifresult->top_serial = aux_cpy_OctetString(cSTK[cindex]->tbs->serialnumber);
					verifresult->top_name = aux_DName2Name(cSTK[cindex]->tbs->issuer);
					return  cindex;
				}
	
				/* check if subjectkey is found in FCPath */
				if (reduced_fcpath && af_FCPath_is_trusted == TRUE) {
					if ( LookupPK_in_FCPath(cSTK[cindex]->tbs->subject, cSTK[cindex]->tbs->subjectPK, reduced_fcpath, time) != 0 ) {
						get_path_finish(FCstk, (Certificate **)0, pklist);
						aux_cpy2_KeyInfo(topkey, cSTK[cindex]->tbs->subjectPK);
						*topkey_serial = aux_cpy_OctetString(cSTK[cindex]->tbs->serialnumber);
						verifresult->trustedKey = 4;
						verifresult->top_serial = aux_cpy_OctetString(cSTK[cindex]->tbs->serialnumber);
						verifresult->top_name = aux_DName2Name(cSTK[cindex]->tbs->issuer);
						return  cindex;
					}
				}
			}

			/* check for issuer as rootCA */
			if (aux_cmp_DName(rootinfo->ca, cSTK[cindex]->tbs->issuer) == 0) {
				int	rc = cindex + 1;
				/* issuer is rootCA */
/* !! newkey/oldkey handling to be implemented */
				if (1 || ((cmp = aux_cmp_OctetString(cSTK[cindex]->tbs->serialnumber, rootinfo->newkey->serial)) == 1 || !cmp)) {
					/* newkey */
					aux_cpy2_KeyInfo(topkey, rootinfo->newkey->key);
                                        *topkey_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
					verifresult->trustedKey = 0;
					verifresult->top_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
					verifresult->top_name = aux_DName2Name(rootinfo->ca);

					/* Check whether time (time and date of signature) lies within the validity */
					/* time frame of rootinfo						    */
					if(af_check_Validity){
						if (af_check_validity_of_PKRoot(time, rootinfo)) {
							/* Root Info has expired ! */
							AUX_ADD_ERROR;
							verifresult->valid = aux_cpy_Validity(rootinfo->newkey->valid);
							verifresult->success = FALSE;
						}
					}
				} 
				else if ((cmp = aux_cmp_OctetString(cSTK[cindex]->tbs->serialnumber, rootinfo->oldkey->serial)) == 1 || !cmp) {
					/* oldkey */
					aux_cpy2_KeyInfo(topkey, rootinfo->oldkey->key);
                                        *topkey_serial = aux_cpy_OctetString(rootinfo->oldkey->serial);
					verifresult->trustedKey = 1;
					verifresult->top_serial = aux_cpy_OctetString(rootinfo->oldkey->serial);
					verifresult->top_name = aux_DName2Name(rootinfo->ca);

					/* Check whether time (time and date of signature) lies within the validity */
					/* time frame of rootinfo						    */
					if(af_check_Validity){
						if (af_check_validity_of_PKRoot(time, rootinfo)) {
							/* Root Info has expired ! */
							AUX_ADD_ERROR;
							verifresult->valid = aux_cpy_Validity(rootinfo->oldkey->valid);
							verifresult->success = FALSE;
						}
					}
				} 
				else {
					aux_add_error(EROOTKEY, "Can't verify certificate with PKRoot: needs lower serial number", (char *) cSTK[cindex], Certificate_n, proc);
					get_path_finish(FCstk, cSTK, pklist);
					verifresult->trustedKey = - 1;
					return(-1);
				}
				get_path_finish(FCstk, (Certificate **)0, pklist);
				return(rc);
			}

			/* check if issuer of cert is cross-certified by own Root CA */
			cert = check_CrossCertificates (rootinfo->ca, cSTK[cindex]->tbs->issuer);
			if (cert){
				int	rc = cindex + 2;
				crosscert_appended_to_certs = TRUE;
				cSTK[cindex + 1] = aux_cpy_Certificate(cert);
				aux_free_Certificate(&cert);
				aux_cpy2_KeyInfo(topkey, rootinfo->newkey->key);
				*topkey_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
				get_path_finish(FCstk, (Certificate **)0, pklist);
				verifresult->trustedKey = 0;
				verifresult->top_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
				verifresult->top_name = aux_DName2Name(rootinfo->ca);

				/* Check whether time (time and date of signature) lies within the validity */
				/* time frame of rootinfo						    */
				if(af_check_Validity){
					if (af_check_validity_of_PKRoot(time, rootinfo)) {
						/* Root Info has expired ! */
						AUX_ADD_ERROR;
						verifresult->valid = aux_cpy_Validity(rootinfo->newkey->valid);
						verifresult->success = FALSE;
					}
				}

				return(rc);
			}

			/* try next certificate of this level, if at top */
			if (fpath) break;
		}

		if (rcselect) {	/* got one certificate */
			p = &FCstk[cindex++];
			p->crosscertificates = fpath->liste;
			p->forwardpath = fpath;
		} 
		else if (cindex > 0) {	/* try backtrack */
			cSTK[cindex] = (Certificate *) 0;
			p = &FCstk[--cindex];
			fpath = p->forwardpath;	/* restore to next level below */
		}
		else break;
	}

	if(af_access_directory == FALSE){
		aux_add_error(EPATH, "Can't get path because the chain of certificates is incomplete", (char *) cSTK[cindex], Certificate_n, proc);
		get_path_finish(FCstk, cSTK, pklist);
		return - 1;
	}



establish_certpath_from_directory:

	/* Access a directory in order to complete originator's certification path ... */
	/* cindex = 0 */

	certs_from_directory = TRUE;

	certpath = (SET_OF_Certificate *)malloc(sizeof(SET_OF_Certificate ) );
	if (!certpath) {
		aux_add_error(EMALLOC, "certpath", CNULL, 0, proc);
		return (- 1);
	}
	certpath->element = aux_cpy_Certificate(orig_cert->usercertificate);
	certpath->next = (SET_OF_Certificate *)0;

	level = (int * )malloc(sizeof(int));
	if ( ! level ) {
		aux_add_error(EMALLOC, "level", CNULL, 0, proc);
		return (- 1);
	}
	*level = 0;

	if(complete_FCPath_from_Directory(orig_cert->usercertificate, certpath, level, topkey, topkey_serial, pklist, rootinfo, time)){
		/* Directory access failed */
		aux_add_error(EPATH, "Completion of certification path using X.500 or .af-db directory failed", CNULL, 0, proc);
		return(- 1);
	}

	/* The originator's certification path was successfully established from the directory */

	dir_cSTK = (Certificate **) calloc(*level, sizeof(Certificate * ));
	if (!dir_cSTK) {
		aux_add_error(EMALLOC, "dir_cSTK", CNULL, 0, proc);
		return(- 1);
	}

	*cSTKp = dir_cSTK;
	dir_cSTK[0] = aux_cpy_Certificate(orig_cert->usercertificate);
	for(i = 1, certpath_tmp = certpath->next; (i <= *level) && certpath_tmp; i ++, certpath_tmp = certpath_tmp->next)
		dir_cSTK[i] = aux_cpy_Certificate(certpath_tmp->element);
	aux_free_CertificateSet(&certpath);
	cindex = *level;
	free (level);

	get_path_finish(FCstk, cSTK, pklist);

	if(* level == 0) {
		aux_add_error(EPATH, "Creation of certification path using X.500 or .af-db directory failed", CNULL, 0, proc);
		return (- 1);
	}

	return(cindex);

}	/* get_path() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure LookupPK
 *
 ***************************************************************/
#ifdef __STDC__

static KeyInfo *LookupPK(
	DName	 *name,
	KeyInfo	 *namePK,
	PKList	 *pklist,
	UTCTime	 *time
)

#else

static KeyInfo *LookupPK(
	name,
	namePK,
	pklist,
	time
)
DName	 *name;
KeyInfo	 *namePK;
PKList	 *pklist;
UTCTime	 *time;

#endif

{
	char   * proc = "LookupPK";

	if (!name)
		return ( (KeyInfo * ) 0);

	for (; pklist; pklist = pklist->next) {
		if (aux_cmp_DName(name, pklist->element->subject) == 0 && 
		    !aux_cmp_KeyInfo(namePK, pklist->element->subjectPK)) {
			if (af_check_Validity == FALSE || !af_check_validity_of_ToBeSigned(time, pklist->element))
				return (pklist->element->subjectPK);
		}
	}

	return ( (KeyInfo * ) 0);

}	/* LookupPK() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure LookupPK_in_FCPath
 *
 ***************************************************************/
#ifdef __STDC__

static KeyInfo *LookupPK_in_FCPath(
	DName	 *name,
	KeyInfo	 *namePK,
	FCPath	 *fcpath,
	UTCTime	 *time
)

#else

static KeyInfo *LookupPK_in_FCPath(
	name,
	namePK,
	fcpath,
	time
)
DName	 *name;
KeyInfo	 *namePK;
FCPath	 *fcpath;
UTCTime	 *time;

#endif

{
	SET_OF_Certificate * set;
	char   		   * proc = "LookupPK_in_FCPath";

	if (! name)
		return ( (KeyInfo * ) 0);

	for (; fcpath; fcpath = fcpath->next_forwardpath) {
		set = fcpath->liste;
		for (; set; set = set->next) {
			if (aux_cmp_DName(name, set->element->tbs->subject) == 0 && 
			    !aux_cmp_KeyInfo(namePK, set->element->tbs->subjectPK)) {
				if (af_check_Validity == FALSE || !af_check_validity_of_Certificate(time, set->element))
					return (set->element->tbs->subjectPK);
			}
		} /* for */
	} /* for */

	return ( (KeyInfo * ) 0);

}	/* LookupPK_in_FCPath() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure check_black_list
 *
 ***************************************************************/
#ifdef __STDC__

static UTCTime *check_black_list(
	Certificate	 *cert,
	UTCTime		 *time
)

#else

static UTCTime *check_black_list(
	cert,
	time
)
Certificate	 *cert;
UTCTime		 *time;

#endif

{
        CrlSet                  * crlset;
	CRL                  * crl = (CRL * )0;
	Crl                  * crlpse;
	Boolean 	          update = FALSE, found = FALSE, tried = FALSE;
	SEQUENCE_OF_CRLEntry  * revokedCertificates;
	HashInput               * hashin = (HashInput * ) 0;         /* hash input in case of sqmodn */
	Key                       key;                               /* the public key */
	KeyType         	  ktype = SIGNATURE;
	AlgHash                   alghash;
	SEQUENCE_OF_CRLEntry	* revcerts;
	ToBeSigned 	        * tbs;
        SET_OF_Certificate      * certset;
        int		          algtype;
	VerificationResult      * store_verifresult;
	Boolean		          store_certs_from_directory;
	int 		          rcode = - 1, rc;
	char			  x500 = TRUE;
#ifdef AFDBFILE
	char			  afdb[256];
#endif
	char	                * proc = "check_black_list";


	
	if (! cert) {
		aux_add_error(EINVALID, "No certificate to check against black list", CNULL, 0, proc);
		return ((UTCTime * )0);
	}
 
#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, "X500");           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif

	crlset = af_pse_get_CrlSet();
	if(!crlset) {
		if(aux_last_error() == EOBJNAME) aux_free_error();
		else {
			AUX_ADD_ERROR;
			return((UTCTime * )0);
		}
	}

	while ( crlset && (found == FALSE) ) {
		if (aux_cmp_DName(cert->tbs->issuer, crlset->element->issuer) == 0) {
		        /* issuer's revocation list stored locally */
			if (aux_cmp_UTCTime(time, crlset->element->nextUpdate) != 2) {
				/* revocation list stored in PSE is obsolete */
				aux_add_error(EVALIDITY, "Locally stored revocation list is out-of-date", CNULL, 0, proc);
				update = TRUE;
				break;
			}
			else {
				revcerts = crlset->element->revcerts;
				while (revcerts) {
					if (revcerts->element &&
					    !aux_cmp_OctetString(cert->tbs->serialnumber, revcerts->element->serialnumber)) {
						found = TRUE;
						break;
					}
					revcerts = revcerts->next;
				}
				if (! revcerts)    /* cert NOT contained in revocation list */
					return ((UTCTime * )0);
			} 
		}
		if (found == TRUE)
			break;
		crlset = crlset->next;
	} /* while */

	if (found == TRUE) {
		aux_add_error(EREVOKE, "Certificate has been revoked", (char *) cert, Certificate_n, proc);
		return (revcerts->element->revocationDate);
	}

	if(af_access_directory == FALSE){
		if(aux_last_error() == EVALIDITY) 
			return(crlset->element->nextUpdate);
		aux_add_error(EAVAILABLE, "Revocation List is unavailable", CNULL, 0, proc);
		return ((UTCTime * )0);
	}


	/* Access Directory (X.500 or .af-db) */

	/* The following is valid: !crlset || (update == TRUE). In other words:
	 * Retrieve revocation list of cert->tbs->issuer from directory, as 
	 * 1. there is no revocation list of that issuer stored in the PSE, or 
	 * 2. the revocation list of that issuer stored in the PSE is out-of-date. */

	crl = (CRL * ) 0;

#ifdef X500
	if (x500 && af_access_directory == TRUE){
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			store_verifresult = verifresult;
			verifresult = (VerificationResult *)0;
			store_certs_from_directory = certs_from_directory;
		} 
		crl = af_dir_retrieve_CRL(cert->tbs->issuer);
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			verifresult = store_verifresult;
			certs_from_directory = store_certs_from_directory;
		}
	}
#endif
#ifdef AFDBFILE
	if ( (!x500 || !af_x500) && af_access_directory == TRUE) 
		crl = af_afdb_retrieve_CRL(cert->tbs->issuer);
#endif

	if (!crl) {
		aux_add_error(EAVAILABLE, "Revocation List is unavailable", CNULL, 0, proc);
		return ((UTCTime * )0);
	}

	/* Verifying the returned revocation list */
	if ( !(tbs = af_pse_get_TBS(SIGNATURE, cert->tbs->issuer, NULLDNAME, 0)) ) {

		certset = (SET_OF_Certificate * )0;

#ifdef X500
		if ( x500 && af_access_directory == TRUE){
			if(af_dir_authlevel == DBA_AUTH_STRONG){
				store_verifresult = verifresult;
				verifresult = (VerificationResult *)0;
				store_certs_from_directory = certs_from_directory;
			}
			certset = af_dir_retrieve_Certificate(cert->tbs->issuer, cACertificate);
			if(af_dir_authlevel == DBA_AUTH_STRONG){
				verifresult = store_verifresult;
				certs_from_directory = store_certs_from_directory;
			}
			if(!certset){
				aux_add_error(EVERIFICATION, 
				"Can't find public verification key of issuer", CNULL, 0, proc);
				aux_free_CRL (&crl);
				return ((UTCTime * )0);
			}
		}
#endif
#ifdef AFDBFILE
		if ( (!x500 || !af_x500) && af_access_directory == TRUE && 
		     !(certset = af_afdb_retrieve_Certificate(cert->tbs->issuer,ktype)) ) {
			aux_add_error(EVERIFICATION, 
			"Can't find public verification key of issuer", CNULL, 0, proc);
			aux_free_CRL (&crl);
			return ((UTCTime * )0);
		}
#endif
	}

        key.keyref = 0;
        key.pse_sel = (PSESel *) 0;

	if (tbs){
		key.key = tbs->subjectPK;
        	alghash = aux_ObjId2AlgHash(crl->sig->signAI->objid);
		if (alghash == SQMODN) hashin = (HashInput * ) & key.key->subjectkey;
		else hashin = (HashInput * ) 0;

		rcode = sec_verify(crl->tbs_DERcode, crl->sig, SEC_END, &key, hashin);
	}
	else{		
		while (certset) {
			/* compare, if ENCRYPTION or SIGNATURE object identifier: */
			algtype = aux_ObjId2AlgType(certset->element->tbs->subjectPK->subjectAI->objid);
			if ((algtype != SIG) && (algtype != ASYM_ENC)) certset = certset->next;
			else{
        			/* needs HashInput parameter set */
				tried = TRUE;
				key.key = certset->element->tbs->subjectPK;
        			alghash = aux_ObjId2AlgHash(crl->sig->signAI->objid);
				if (alghash == SQMODN) hashin = (HashInput * ) & key.key->subjectkey;
				else hashin = (HashInput * ) 0;

				rcode = sec_verify(crl->tbs_DERcode, crl->sig, SEC_END, &key, hashin);
				if(rcode != 0) 
					certset = certset->next; /* Try verification by applying next certificate in set */
				else break;
			}
		} /* while */
		if (! certset && ! tried) {
			aux_add_error(EVERIFICATION, 
			"No SIGNATURE certificate in directory entry of issuer", CNULL, 0, proc);
			aux_free_CRL (&crl);
			return ((UTCTime * )0);
		}
	}

	/* Verification of revocation list FAILED */

	if (rcode != 0) { 
		aux_add_error(EVERIFICATION, "Verification of revocation list failed", CNULL, 0, proc);
		aux_free_CRL (&crl);
		return((UTCTime * )0);
	}


	/* Verification of revocation list SUCCEEDED */

	/* Revocation list has been verified, check if it is out-of-date: */
	rc = aux_cmp_UTCTime(time, crl->tbs->nextUpdate);
	if(rc != 2){  /*obsolete*/
		aux_add_error(EVALIDITY, "Revocation list returned from directory is out-of-date", CNULL, 0, proc);
		aux_free_CRL (&crl);
		return (crl->tbs->nextUpdate);
	}

	/* Update of PSE object CrlSet */
	crlpse = CRL2Crl(crl);
	rcode = af_pse_add_CRL(crlpse);
	aux_free_Crl (&crlpse);
	if (rcode != 0)
		aux_add_error(ECREATEOBJ, "Cannot update PSE object CrlSet", CNULL, 0, proc);

	revokedCertificates = crl->tbs->revokedCertificates;
	while (revokedCertificates) {
		if (!aux_cmp_OctetString(cert->tbs->serialnumber, revokedCertificates->element->serialnumber)) {
			found = TRUE;
			break;
		}
		revokedCertificates = revokedCertificates->next;
	}

	if (found == TRUE) {   /* cert found in revocation list */
		aux_add_error(EREVOKE, "Certificate has been revoked", (char *) cert, Certificate_n, proc);
		aux_free_CRL(&crl);
		return (revokedCertificates->element->revocationDate);
	}

	aux_free_CRL(&crl);

	return ((UTCTime * )0);

} 	/* check_black_list() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure check_CrossCertificates
 *
 ***************************************************************/
#ifdef __STDC__

static Certificate *check_CrossCertificates(
	DName	 *own_root,
	DName	 *foreign_root
)

#else

static Certificate *check_CrossCertificates(
	own_root,
	foreign_root
)
DName	 *own_root;
DName	 *foreign_root;

#endif

{
	SET_OF_CertificatePair * cpairset, * local_cpairset, * tmp;
	Certificate	       * ret;
	char 			 x500 = TRUE;
	VerificationResult     * store_verifresult;
	Boolean		         store_certs_from_directory;
#ifdef AFDBFILE
	char			 afdb[256];
#endif
	char                   * proc = "check_CrossCertificates";

#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, "X500");           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif

	local_cpairset = tmp = af_pse_get_CertificatePairSet();

	if(local_cpairset){
		while (tmp) {
			if ( ! aux_cmp_DName (foreign_root, tmp->element->reverse->tbs->subject) ) {
				ret = aux_cpy_Certificate(tmp->element->reverse);
				aux_free_CertificatePairSet(& local_cpairset);
				crosscert_from_PSE = TRUE;
				return (ret);
			}
			tmp = tmp->next;
		}
		aux_free_CertificatePairSet(& local_cpairset);
	}

	if (af_access_directory == FALSE)
		return ( (Certificate *) 0);
   
	/* no set of cross certificate pairs locally stored, or required cross certificate pair not found */
	/* within set of locally stored cross certificate pairs						  */

	cpairset = (SET_OF_CertificatePair * )0;

#ifdef X500
	if (x500 && af_access_directory == TRUE){
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			store_verifresult = verifresult;
			verifresult = (VerificationResult *)0;
			store_certs_from_directory = certs_from_directory;
		} 
		cpairset = tmp = af_dir_retrieve_CertificatePair(own_root);
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			verifresult = store_verifresult;
			certs_from_directory = store_certs_from_directory;
		}
	}
#endif
#ifdef AFDBFILE
	if ( (!x500 || !af_x500) && af_access_directory == TRUE) 
		cpairset = tmp = af_afdb_retrieve_CertificatePair(own_root);
#endif

	if (! cpairset) {
		aux_add_error(LASTERROR, "No cross certificates returned from directory", CNULL, 0, proc);
		return ( (Certificate *)0 );
	}

	while (tmp) {
		if ( ! aux_cmp_DName (foreign_root, tmp->element->reverse->tbs->subject) ) {
			ret = aux_cpy_Certificate(tmp->element->reverse);
			crosscert_from_Directory = TRUE;
			if ( af_pse_update_CertificatePairSet(cpairset) < 0 ) {
				aux_add_error(EWRITEPSE, "Cannot update cross certificate set", (char *) cpairset, SET_OF_CertificatePair_n, proc);
				aux_free_CertificatePairSet(&cpairset);
				return ( (Certificate *) 0);
			}
			aux_free_CertificatePairSet(&cpairset);
			return (ret);
		}
		tmp = tmp->next;
	}

	if ( af_pse_update_CertificatePairSet(cpairset) < 0 ) {
		AUX_ADD_ERROR;
		aux_free_CertificatePairSet(&cpairset);
		return ( (Certificate *) 0);
	}

	aux_free_CertificatePairSet(&cpairset);
	return ( (Certificate *) 0);

}	/* check_CrossCertificates() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure complete_FCPath_from_Directory
 *
 ***************************************************************/
#ifdef __STDC__

static RC complete_FCPath_from_Directory(
	Certificate		 *tobeverified_cert,
	SET_OF_Certificate	 *certpath,
	int			 *level,
	KeyInfo			 *topkey,
	OctetString	         **topkey_serial,
	PKList			 *pklist,
	PKRoot			 *rootinfo,
	UTCTime			 *time
)

#else

static RC complete_FCPath_from_Directory(
	tobeverified_cert,
	certpath,
	level,
	topkey,
	topkey_serial,
	pklist,
	rootinfo,
	time
)
Certificate		 *tobeverified_cert;
SET_OF_Certificate	 *certpath;
int			 *level;
KeyInfo			 *topkey;
OctetString		 **topkey_serial;
PKList			 *pklist;
PKRoot			 *rootinfo;
UTCTime			 *time;

#endif

{
	SET_OF_Certificate * certset;
	SET_OF_Certificate * tmp_certpath;
	HashInput          * hashin = (HashInput * ) 0;         /* hash input in case of sqmodn */
	Key    	             key;                               /* the public key */
	AlgHash              alghash;
	Name               * printrepr;
	Certificate	   * cross_cert;
	VerificationResult * store_verifresult;
	Boolean		     store_certs_from_directory;
	char 	             x500 = TRUE;
	int 		     cmp;
#ifdef AFDBFILE
	char	             afdb[256];
#endif

	char	           * proc = "complete_FCPath_from_Directory";

#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, "X500");           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif

	if(! tobeverified_cert || ! certpath || ! level || ! topkey_serial){
		aux_add_error(EINVALID, "no parameter", CNULL, 0, proc);
		return(- 1);
	}

	certset = (SET_OF_Certificate * )0;

#ifdef X500
	if (x500 && af_access_directory == TRUE){
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			store_verifresult = verifresult;
			verifresult = (VerificationResult *)0;
			store_certs_from_directory = certs_from_directory;
		}
		certset = af_dir_retrieve_Certificate(tobeverified_cert->tbs->issuer, cACertificate);
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			verifresult = store_verifresult;
			certs_from_directory = store_certs_from_directory;
		}
	}
#endif
#ifdef AFDBFILE
	if ((!x500 || !af_x500) && af_access_directory == TRUE) 
		certset = af_afdb_retrieve_Certificate(tobeverified_cert->tbs->issuer, SIGNATURE);
#endif

	if(! certset){
		aux_add_error(LASTERROR, "No set of certificates returned from directory", CNULL, 0, proc);
		return(- 1);
	}

	while(certset){
		if(af_check_Validity){
			if(af_check_validity_of_Certificate(time, certset->element)) {
				certset = certset->next;
				continue;	
			}
		}

		key.key = certset->element->tbs->subjectPK;    /* verification key of next level */
		key.keyref = 0;
        	key.pse_sel = (PSESel *) 0;

		/* needs HashInput parameter set */
		alghash = aux_ObjId2AlgHash(tobeverified_cert->sig->signAI->objid);
		if (alghash == SQMODN) hashin = (HashInput * ) & key.key->subjectkey;
		else hashin = (HashInput * ) 0;

		if (sec_verify(tobeverified_cert->tbs_DERcode, tobeverified_cert->sig, SEC_END, &key, hashin) < 0) {
			aux_add_error(EVERIFICATION, "Verification of certificate failed", (char *) tobeverified_cert, Certificate_n, proc);
			certset = certset->next; /* Try to verify "tobeverified_cert" with the help of the next cert within set */
		}
		else{
			/* Verification  S U C C E S S F U L */

			if (af_chk_crl == FALSE) {
				/* check if subjectkey is contained in PKList */
				if ( LookupPK(certset->element->tbs->subject, certset->element->tbs->subjectPK, pklist, time) != 0 ) {
					aux_cpy2_KeyInfo(topkey, certset->element->tbs->subjectPK);
					*topkey_serial = aux_cpy_OctetString(certset->element->tbs->serialnumber);
					verifresult->trustedKey = 3;
					verifresult->top_serial = aux_cpy_OctetString(certset->element->tbs->serialnumber);
					verifresult->top_name = aux_DName2Name(certset->element->tbs->issuer);
					(*level)++;
					return(0);
				}
	
				/* check if subjectkey is found in own FCPath */
				if (reduced_fcpath && af_FCPath_is_trusted == TRUE) {
					if ( LookupPK_in_FCPath(certset->element->tbs->subject, certset->element->tbs->subjectPK, reduced_fcpath, time) != 0 ) {
						aux_cpy2_KeyInfo(topkey, certset->element->tbs->subjectPK);
						*topkey_serial = aux_cpy_OctetString(certset->element->tbs->serialnumber);
						verifresult->trustedKey = 4;
						verifresult->top_serial = aux_cpy_OctetString(certset->element->tbs->serialnumber);
						verifresult->top_name = aux_DName2Name(certset->element->tbs->issuer);
						(*level)++;
						return(0);
					}
				}
			} /* if */

			/* check for issuer as rootCA */
			if (aux_cmp_DName(rootinfo->ca, certset->element->tbs->issuer) == 0) {
				/* issuer is rootCA */
				certpath->next = tmp_certpath = (SET_OF_Certificate *)malloc(sizeof(SET_OF_Certificate ) );
				if (!certpath->next) {
					aux_add_error(EMALLOC, "certpath->next", CNULL, 0, proc);
					return (- 1);
				}
				tmp_certpath->element = aux_cpy_Certificate(certset->element);
				tmp_certpath->next = (SET_OF_Certificate *)0;
				(*level) ++;

/* !! newkey/oldkey handling to be implemented */
				if (1 || ((cmp = aux_cmp_OctetString(certset->element->tbs->serialnumber, rootinfo->newkey->serial)) == 1 || !cmp))
					/* ">=" comparison */
					/* newkey */ {
					aux_cpy2_KeyInfo(topkey, rootinfo->newkey->key);
                                        *topkey_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
					(*level)++;  /* for Root key */
					verifresult->trustedKey = 0;
					verifresult->top_serial = aux_cpy_OctetString(rootinfo->newkey->serial);
					verifresult->top_name = aux_DName2Name(rootinfo->ca);

					/* Check whether time (time and date of signature) lies within the validity */
					/* time frame of rootinfo						    */
					if(af_check_Validity){
						if (af_check_validity_of_PKRoot(time, rootinfo)) {
							/* Root Info has expired ! */
							AUX_ADD_ERROR;
							verifresult->valid = aux_cpy_Validity(rootinfo->newkey->valid);
							verifresult->success = FALSE;
						}
					}

					return(0);
				} 
				else if ((cmp = aux_cmp_OctetString(certset->element->tbs->serialnumber, rootinfo->oldkey->serial)) == 1 || !cmp) 
					/* ">=" comparison */
					/* oldkey */ {
					aux_cpy2_KeyInfo(topkey, rootinfo->oldkey->key);
                                        *topkey_serial = aux_cpy_OctetString(rootinfo->oldkey->serial);
					(*level)++;  /* for Root key */
					verifresult->trustedKey = 1;
					verifresult->top_serial = aux_cpy_OctetString(rootinfo->oldkey->serial);
					verifresult->top_name = aux_DName2Name(rootinfo->ca);

					/* Check whether time (time and date of signature) lies within the validity */
					/* time frame of rootinfo						    */
					if(af_check_Validity){
						if (af_check_validity_of_PKRoot(time, rootinfo)) {
							/* Root Info has expired ! */
							AUX_ADD_ERROR;
							verifresult->valid = aux_cpy_Validity(rootinfo->oldkey->valid);
							verifresult->success = FALSE;
						}
					}

					return(0);
				} 

				aux_add_error(EROOTKEY, "Can't verify Certificate from X500 directory with PKRoot: needs lower serial number", (char *) certset->element, Certificate_n, proc);
				verifresult->trustedKey = - 1;
				return(- 1);
			}

			/* Neither subjectkey was found in PKList nor does the issuer correspond to the own rootCA nor  */
			/* has the issuer been cross-certified by the own rootCA. 					*/
			/* Therefore, keep on accessing the directory ... */

			certpath->next = tmp_certpath = (SET_OF_Certificate *)malloc(sizeof(SET_OF_Certificate ) );
			if (!certpath->next) {
				aux_add_error(EMALLOC, "certpath->next", CNULL, 0, proc);
				return (- 1);
			}
			tmp_certpath->element = aux_cpy_Certificate(certset->element);
			tmp_certpath->next = (SET_OF_Certificate *)0;
			(*level) ++;

			if(! complete_FCPath_from_Directory(certset->element, tmp_certpath, level, topkey, topkey_serial, pklist, rootinfo, time))  /* ok */
				return(0);
			aux_free_CertificateSet(&certpath->next);
			certpath->next = (SET_OF_Certificate *)0;
			(*level) --;
		}

	}  /* while */

	return(- 1);

}	/* complete_FCPath_from_Directory() */


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure certs_at_one_level
 *
 ***************************************************************/
#ifdef __STDC__

static int certs_at_one_level(
	FCPath	 *path
)

#else

static int certs_at_one_level(
	path
)
FCPath	 *path;

#endif

{
	SET_OF_Certificate * certset;
	int		     count;

	if (! path || ! path->liste)
		return(0);

	for (certset = path->liste, count = 0; certset; certset = certset->next, count++)
		/* count */;
		
	return(count);
}


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure verify_CertificateWithPkroot
 *
 ***************************************************************/
#ifdef __STDC__

static int verify_CertificateWithPkroot(
	Certificate	 *cert,
	PKRoot		 *pkroot
)

#else

static int verify_CertificateWithPkroot(
	cert,
	pkroot
)
Certificate	 *cert;
PKRoot		 *pkroot;

#endif

{
	HashInput     * hashin = (HashInput * ) 0;         /* hash input in case of sqmodn */
	Key    	        key;                               /* the public key */
	AlgHash         alghash;
	int 		cmp;
	int		rc;

	char	      * proc = "verify_CertificateWithPkroot";


/* !! newkey/oldkey handling to be implemented */
	if (1 || ((cmp = aux_cmp_OctetString(cert->tbs->serialnumber, pkroot->newkey->serial)) == 1 || !cmp))
		/* ">=" comparison */
		/* newkey */
		key.key = pkroot->newkey->key;
	else if (pkroot->oldkey && 
		 ((cmp = aux_cmp_OctetString(cert->tbs->serialnumber, pkroot->oldkey->serial)) == 1 || !cmp))
		/* ">=" comparison */
		/* oldkey */
		key.key = pkroot->oldkey->key;
	else {
		aux_add_error(EROOTKEY, "Can't verify Certificate with PKRoot: needs lower serial number", (char *) cert, Certificate_n, proc);
		return(-1);
	}

	key.keyref = 0;
	key.pse_sel = (PSESel *) 0;

	/* needs HashInput parameter set */
	alghash = aux_ObjId2AlgHash(cert->sig->signAI->objid);
	if (alghash == SQMODN) 
		hashin = (HashInput * ) & key.key->subjectkey;
	else 
		hashin = (HashInput * ) 0;

	rc = sec_verify(cert->tbs_DERcode, cert->sig, SEC_END, &key, hashin);
	if(rc < 0) {
		if(aux_last_error() == EVERIFICATION) aux_add_error(EVERIFICATION, "Can't verify Certificate with PKRoot", (char *) cert, Certificate_n, proc);
		else AUX_ADD_ERROR;
	}
	return(rc);

}

/*********************************************************************************************/


/***************************************************************
 *
 * Procedure create_verifstep
 *
 ***************************************************************/
#ifdef __STDC__

static VerificationStep *create_verifstep(
	Certificate      *cert, 
	int 		  crlcheck, 
	UTCTime 	 *date,
	Validity         *valid,
	int 		  supplied, 
	Boolean 	  policy_CA, 
	Boolean 	  dn_subordination_violation
)

#else

static VerificationStep *create_verifstep(
	cert,
	crlcheck,
	date,
	valid,
	supplied,
	policy_CA,
	dn_subordination_violation
)
Certificate      *cert;
int 		  crlcheck;
UTCTime 	 *date;
Validity         *valid;
int 		  supplied;
Boolean 	  policy_CA;
Boolean 	  dn_subordination_violation;

#endif

{
	VerificationStep  * verifstep;
	char	          * proc = "create_verifstep";

	verifstep = (VerificationStep *)malloc(sizeof(VerificationStep));
	if(!verifstep) {
		aux_add_error(EMALLOC, "verifstep", CNULL, 0, proc);
		return((VerificationStep *)0);
	}

	verifstep->cert = aux_cpy_Certificate(cert);
	verifstep->crlcheck = crlcheck;
	verifstep->date = aux_cpy_Name(date);
	verifstep->valid = aux_cpy_Validity(valid);
	verifstep->supplied = supplied;
	verifstep->policy_CA = policy_CA;
	verifstep->dn_subordination_violation = dn_subordination_violation;

	return(verifstep);
}


/*********************************************************************************************/


/***************************************************************
 *
 * Procedure examine_VerificationResult
 *
 ***************************************************************/
#ifdef __STDC__

static RC examine_VerificationResult(
	VerificationResult *verres
)

#else

static RC examine_VerificationResult(
	verres
)
VerificationResult *verres;

#endif

{
	int	index = 0;
	char  * proc = "examine_VerificationResult";

	if(!verres) {
		aux_add_error(EINVALID, "verres parameter missing", CNULL, 0, proc);
		return(-1);
	}

	if (verres->verifstep){
		while(verres->verifstep[index]){
			if(verres->verifstep[index]->dn_subordination_violation == TRUE) {
				aux_add_error(EINVALID, "PEM 1422 Name subordination rule violated", CNULL, 0, proc);
				return(- 1);
			}
			if(verres->verifstep[index]->crlcheck == CRL_NOT_AVAILABLE) {
				aux_add_error(EAVAILABLE, "PEM CRL not available", CNULL, 0, proc);
				return(- 1);
			}
			index++;
		}
	}

	return(0);
}
