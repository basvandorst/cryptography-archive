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

#ifdef X500
#include "psap.h"
#include "af.h"
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#define BUFLEN 4096


static char	buf[BUFLEN];

#else
#include "af.h"
#include <fcntl.h>
#include <stdio.h>
#endif



/***************************************************************
 *
 * Procedure af_create_CRL
 *
 ***************************************************************/
#ifdef __STDC__

CRL *af_create_CRL(
	UTCTime	 *lastUpdate,
	UTCTime	 *nextUpdate
)

#else

CRL *af_create_CRL(
	lastUpdate,
	nextUpdate
)
UTCTime	 *lastUpdate;
UTCTime	 *nextUpdate;

#endif

{

	CRL  *new_crl;
	ObjId	    *oid;
	char	    *proc = "af_create_CRL";

	new_crl = (CRL * )malloc(sizeof(CRL));
	if (!new_crl) {
		aux_add_error(EMALLOC, "new_crl", CNULL, 0, proc);
		return ((CRL * ) 0);
	}

	new_crl->tbs = (CRLTBS * )malloc(sizeof(CRLTBS));
	if (!new_crl->tbs) {
		aux_add_error(EMALLOC, "new_crl->tbs", CNULL, 0, proc);
		return ((CRL * ) 0);
	}

	if (!(new_crl->tbs->issuer = af_pse_get_Name())) {
		aux_add_error(EREADPSE, "af_pse_get_Name failed", CNULL, 0, proc);
		return ((CRL * ) 0);
	}

	if (!lastUpdate)
		new_crl->tbs->lastUpdate = aux_current_UTCTime();
	else {
		new_crl->tbs->lastUpdate = (char *) malloc (18);   /* TX_MAXLEN = 17 */
		strcpy(new_crl->tbs->lastUpdate, lastUpdate);
	}
	if (!nextUpdate) 
		new_crl->tbs->nextUpdate = aux_delta_UTCTime(new_crl->tbs->lastUpdate);
	else {
		new_crl->tbs->nextUpdate = (char *) malloc (18);   /* TX_MAXLEN = 17 */
		strcpy(new_crl->tbs->nextUpdate, nextUpdate);
	}
	
	new_crl->tbs->revokedCertificates = (SEQUENCE_OF_CRLEntry * )0;

	new_crl->sig = (Signature * )malloc(sizeof(Signature));
	if (!new_crl->sig) {
		aux_add_error(EMALLOC, "new_crl->sig", CNULL, 0, proc);
		return ((CRL * ) 0);
	}
	new_crl->sig->signature.nbits = 0;
	new_crl->sig->signature.bits = CNULL;

	new_crl->sig->signAI = af_pse_get_signAI();
	if ( aux_ObjId2AlgType(new_crl->sig->signAI->objid) == ASYM_ENC )
		new_crl->sig->signAI = aux_cpy_AlgId(md5WithRsa);

	new_crl->tbs->signatureAI = aux_cpy_AlgId(new_crl->sig->signAI);

	if ((new_crl->tbs_DERcode = e_CRLTBS(new_crl->tbs)) == NULLOCTETSTRING) {
		aux_add_error(EENCODE, "e_CRLTBS failed", CNULL, 0, proc);
		return( (CRL * )0);
	}

	fprintf(stderr, "\nThe following empty CRL is to be signed. Please check it:\n\n");
	aux_fprint_CRLTBS(stderr, new_crl->tbs);
	fprintf(stderr, "\nDo you want to sign the displayed CRL?\n");
	fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

	/*oid = af_get_objoid(SignSK_name);
	af_pse_close (oid);*/
	af_pse_close (NULLOBJID);
	if (af_sign(new_crl->tbs_DERcode, new_crl->sig, SEC_END) < 0 ) {
		aux_add_error(ESIGN, "af_sign failed", CNULL, 0, proc);
		return( (CRL * )0 );
	}
	return (new_crl);

}


/***************************************************************
 *
 * Procedure af_create_CRLEntry
 *
 ***************************************************************/
#ifdef __STDC__

CRLEntry *af_create_CRLEntry(
	OctetString * serial
)

#else

CRLEntry *af_create_CRLEntry(
	serial
)
OctetString * serial;

#endif

{

	CRLEntry *ret;
	char	   *proc = "af_create_CRLEntry";

	ret = (CRLEntry * )malloc(sizeof(CRLEntry));
	if (!ret) {
		aux_add_error(EMALLOC, "ret", CNULL, 0, proc);
		return ((CRLEntry * ) 0);
	}

	ret->serialnumber = aux_cpy_OctetString(serial);
	ret->revocationDate = aux_current_UTCTime();

	return(ret);
}


/***************************************************************
 *
 * Procedure af_search_CRLEntry
 *
 ***************************************************************/
#ifdef __STDC__

RC af_search_CRLEntry(
	CRL		 *crl,
	CRLEntry	 *crlentry
)

#else

RC af_search_CRLEntry(
	crl,
	crlentry
)
CRL		 *crl;
CRLEntry	 *crlentry;

#endif

{
	SEQUENCE_OF_CRLEntry *seq;
	int		        found;
	char	               *proc = "af_search_CRLEntry";

	if ( !crl || !crlentry ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return (-1);
	}

	seq = crl->tbs->revokedCertificates;
	found = 0;
	while ( seq && !found ) {
		if (!aux_cmp_OctetString(seq->element->serialnumber, crlentry->serialnumber)) {
			found = 1;
			break;
		}
		seq = seq->next;
	}

	return(found);
}


#ifdef X500


/***************************************************************
 *
 * Procedure af_create_OCList
 *
 ***************************************************************/
#ifdef __STDC__

OCList *af_create_OCList(
	KeyInfo	 *new_pubkey
)

#else

OCList *af_create_OCList(
	new_pubkey
)
KeyInfo	 *new_pubkey;

#endif

{

	OCList      * ret;		 /*return value, newly created first line of "Old Certificates"*/
	Certificate * newcert;    /*cross certificate in first line of "Old Certificates"*/
	OctetString   nullocts;
	char	    * proc = "af_create_OCList";

	if (!new_pubkey) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return ((OCList * ) 0);
	}

	fprintf(stderr, "\nCreating (new) first line of OldCertificates table...\n");

	if ( !(ret = (OCList * )malloc(sizeof(OCList))) ) {
		aux_add_error(EMALLOC, "ret", CNULL, 0, proc);
		return ((OCList * ) 0);
	}

	/*create cross certificate which will be recorded in the right column of        */
	/*the new first line within the Root CA's directory table "Old Certificates":	*/
	newcert = (Certificate * )malloc(sizeof(Certificate));
	if (!newcert) {
		aux_add_error(EMALLOC, "newcert", CNULL, 0, proc);
		return ((OCList * ) 0);
	}

	newcert->tbs = (ToBeSigned * )malloc(sizeof(ToBeSigned));
	if (!newcert->tbs) {
		aux_add_error(EMALLOC, "newcert->tbs", CNULL, 0, proc);
		return ((OCList * ) 0);
	}

	newcert->tbs->version = 0;    /* default version */
	newcert->tbs->serialnumber = af_pse_incr_serial();
	newcert->tbs->issuer = af_pse_get_Name();

	newcert->tbs->valid = (Validity * )malloc(sizeof(Validity));
	if (!newcert->tbs->valid) {
		aux_add_error(EMALLOC, "newcert->tbs->valid", CNULL, 0, proc);
		return ((OCList * ) 0);
	}
	newcert->tbs->valid->notbefore = aux_current_UTCTime();   /*????*/
	newcert->tbs->valid->notafter = aux_delta_UTCTime(newcert->tbs->valid->notbefore);  /*????*/

	newcert->tbs->subject = aux_cpy_DName(newcert->tbs->issuer);
	newcert->tbs->subjectPK = aux_cpy_KeyInfo(new_pubkey);

	newcert->sig = (Signature * )malloc(sizeof(Signature));
	if (!newcert->sig) {
		aux_add_error(EMALLOC, "newcert->sig", CNULL, 0, proc);
		return ((OCList * ) 0);
	}

	newcert->sig->signAI = NULLALGID;
	nullocts.noctets = 0;
	nullocts.octets = CNULL;
	af_pse_close (NULLOBJID);
	if ( af_sign(&nullocts, newcert->sig, SEC_END) < 0) {
		aux_add_error(ESIGN, "af_sign failed", CNULL, 0, proc);
		return ( (OCList * )0 );
	}
	newcert->tbs->signatureAI = aux_cpy_AlgId(newcert->sig->signAI);
	aux_free2_Signature(newcert->sig);

	if ((newcert->tbs_DERcode = e_ToBeSigned(newcert->tbs)) == NULLOCTETSTRING) {
		aux_add_error(EENCODE, "e_ToBeSigned failed", CNULL, 0, proc);
		return ( (OCList * )0 );
	}

	fprintf(stderr, "\nThe following Cross Certificate is to be signed.\n");
	fprintf(stderr, "It will be published in the newly created first line ");
	fprintf(stderr, "of the OldCertificates table.\n");
	fprintf(stderr, "Please check it:\n\n");
	aux_fprint_ToBeSigned(stderr, newcert->tbs);
	fprintf(stderr, "\nDo you want to sign the displayed Cross Certificate?\n");
	fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

	af_pse_close (NULLOBJID);
	if ( af_sign(newcert->tbs_DERcode, newcert->sig, SEC_END) < 0 )  /*sign with OWN signature key*/ {
		aux_add_error(ESIGN, "af_sign failed", (char *) newcert, Certificate_n, proc);
		return( (OCList * )0 );
	}


	/*left column of newly created first line, assuming that the certificate	*/
	/*number is a number smaller than 1 million:					*/
/* For further study!!! */
	/*ret->serialnumber = ((newcert->tbs->serialnumber + 1000000) / 1000000) * 1000000;*/
	ret->serialnumber = aux_new_OctetString(1);
	ret->serialnumber->octets[0] = 0x00;

	ret->ccert = aux_cpy_Certificate(newcert);
	aux_free_Certificate(&newcert);
	ret->next = (OCList * )0;

	return(ret);
}




/* Assumptions made by af_dir_update_OCList():				                */
/* - The Root CA has changed her signature key.					       	*/
/* - The Root CA has already cross-certified her newly created public key with her 	*/
/*   expiring signature key by invoking af_create_OCList().				*/
/* - The new signature key has already been installed on the Root CA's chipcard.	*/
/* - The chipcard entry "Public Root-CA-Keys" has NOT YET been updated after the change */
/*   of the Root CA-key (see steps 1-4, 6.2.4 ("Change of a Root CA key"), p.25, Vol.1).*/

/***************************************************************
 *
 * Procedure af_dir_update_OCList
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_update_OCList(
	OCList	 *first_line
)

#else

RC af_dir_update_OCList(
	first_line
)
OCList	 *first_line;

#endif

{

	DName       * dname;
	OCList      * oclist;
	PKRoot      * pkroot;
	Certificate * newcert;
	Key	      vkey;		                /* verifykey, the public key */
	HashInput   * vhashin = (HashInput *) 0;	/* hashinput pointer used for verifying, hash input in case of sqmodn*/
	int	      rc;
	AlgHash       alghash;
	OctetString   nullocts;
	AlgId       * save_signAI;
	char	    * proc = "af_dir_update_OCList";

	if (!first_line) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	dname = af_pse_get_Name();

	fprintf(stderr, "\nSee if the directory entry associated with the Root CA already contains\n");
	fprintf(stderr, "an attribute of type OldCertificates...\n\n");

	if ( !(oclist = af_dir_retrieve_OCList(dname)) && (err_stack->e_number != EATTRDIR) ) {
		aux_add_error(EINVALID, "af_dir_retrieve_OCList failed", (char *) dname, DName_n, proc);
		return (-1);
	}
	if ( !(pkroot = af_pse_get_PKRoot()) ) {
		aux_add_error(EINVALID, "af_pse_get_PKRoot failed", CNULL, 0, proc);

		return (-1);
	}

	if ( !oclist ) {  	/*first change of Root CA-key*/

		fprintf(stderr, "\n\nThe Root CA is changing its signature key ");
		fprintf(stderr, "for the first time.\n\n");

		/* creating second line of OldCertificates table: */

		fprintf(stderr, "\nCreating second line of OldCertificates table...\n");

		oclist = (OCList * )malloc(sizeof(OCList));
		if (!oclist) {
			aux_add_error(EMALLOC, "oclist", CNULL, 0, proc);
			return (-1);
		}

		newcert = (Certificate * )malloc(sizeof(Certificate));
		if (!newcert) {
			aux_add_error(EMALLOC, "newcert", CNULL, 0, proc);
			return (-1);
		}

		newcert->tbs = (ToBeSigned * )malloc(sizeof(ToBeSigned));
		if (!newcert->tbs) {
			aux_add_error(EMALLOC, "newcert->tbs", CNULL, 0, proc);
			return (-1);
		}

		newcert->tbs->version = 0;
		newcert->tbs->serialnumber = af_pse_incr_serial();
		newcert->tbs->issuer = aux_cpy_DName(dname);

		newcert->tbs->valid = (Validity * )malloc(sizeof(Validity));
		if (!newcert->tbs->valid) {
			aux_add_error(EMALLOC, "newcert->tbs->valid", CNULL, 0, proc);
			return (- 1);
		}
		newcert->tbs->valid->notbefore = aux_current_UTCTime();
		newcert->tbs->valid->notafter = aux_delta_UTCTime(newcert->tbs->valid->notbefore);

		newcert->tbs->subject = aux_cpy_DName(dname);
		newcert->tbs->subjectPK = aux_cpy_KeyInfo(pkroot->newkey->key);  /*expired public key*/


		newcert->sig = (Signature * )malloc(sizeof(Signature));
		if (!newcert->sig) {
			aux_add_error(EMALLOC, "newcert->sig", CNULL, 0, proc);
			return (-1);
		}

		newcert->sig->signAI = NULLALGID;
		nullocts.noctets = 0;
		nullocts.octets = CNULL;
		af_pse_close (NULLOBJID);
		if ( af_sign(&nullocts, newcert->sig, SEC_END) < 0) {
			aux_add_error(ESIGN, "can't get AlgId if SignSK", CNULL, 0, proc);
			return (-1);
		}
		newcert->tbs->signatureAI = aux_cpy_AlgId(newcert->sig->signAI);
		aux_free2_Signature(newcert->sig);

		if ((newcert->tbs_DERcode = e_ToBeSigned(newcert->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ToBeSigned failed", CNULL, 0, proc);
			return (-1);
		}

		fprintf(stderr, "\nThe following Cross Certificate is to be signed.\n");
		fprintf(stderr, "It will be published in the second line ");
		fprintf(stderr, "of the OldCertificates table.\n");
		fprintf(stderr, "Please check it:\n\n");
		aux_fprint_ToBeSigned(stderr, newcert->tbs);
		fprintf(stderr, "\nDo you want to sign the displayed Cross Certificate?\n");
		fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

		af_pse_close (NULLOBJID);
		if ( af_sign(newcert->tbs_DERcode, newcert->sig, SEC_END) < 0 ) {
			aux_add_error(ESIGN, "af_sign failed", (char *) newcert, Certificate_n, proc);
			return(-1);
		}

		oclist->ccert = aux_cpy_Certificate(newcert);
		aux_free_Certificate(&newcert);
		oclist->serialnumber = 0;
		oclist->next = (OCList * )0;
		first_line->next = oclist;
	} 
	else {
		first_line->next = oclist;

		/* verify cross certificate in first line of obsolete directory table   */
		/* "Old Certificates":							*/

		fprintf(stderr, "\nVerifying cross certificate in first line of obsolete ");
		fprintf(stderr, "directory table\n `OldCertificates` by applying PKRoot.oldkey...\n\n");

		vkey.key = aux_cpy_KeyInfo(pkroot->oldkey->key);
		vkey.keyref = 0;
		vkey.pse_sel = (PSESel *) 0;

		alghash = aux_ObjId2AlgHash(oclist->ccert->sig->signAI->objid);
		/* needs HashInput parameter set */
		if (alghash == SQMODN) {
			vhashin = (HashInput * ) & vkey.key->subjectkey;
		}

		rc = sec_verify(oclist->ccert->tbs_DERcode, oclist->ccert->sig, SEC_END, &vkey, vhashin);
		if ( rc < 0 ) {
			aux_add_error(EVERIFY, "sec_verify failed for cross certificate", CNULL, 0, proc);
			return (-1);
		}

		/* Replace the certificates' signatures and serial numbers, whereby other	*/
		/* certificate attributes including certified keys and validity values		*/
		/* remain unchanged. Of course, the table's serial numbers of those lines 	*/
		/* remain the same, too.							*/

		oclist->ccert->tbs->serialnumber = af_pse_incr_serial();

		oclist->ccert->sig->signAI = NULLALGID;
		nullocts.noctets = 0;
		nullocts.octets = CNULL;
		af_pse_close (NULLOBJID);
		if ( af_sign(&nullocts, oclist->ccert->sig, SEC_END) < 0) {
			aux_add_error(ESIGN, "af_sign failed", CNULL, 0, proc);
			return (-1);
		}
		oclist->ccert->tbs->signatureAI = aux_cpy_AlgId(oclist->ccert->sig->signAI);
		save_signAI = aux_cpy_AlgId(oclist->ccert->sig->signAI);
		aux_free2_Signature(oclist->ccert->sig);

		if ((oclist->ccert->tbs_DERcode = e_ToBeSigned(oclist->ccert->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ToBeSigned failed", CNULL, 0, proc);
			return (-1);
		}

		fprintf(stderr, "\nThe following Cross Certificate is to be signed. Please check it:\n\n");
		aux_fprint_ToBeSigned(stderr, oclist->ccert->tbs);
		fprintf(stderr, "\nDo you want to sign the displayed Cross Certificate?\n");
		fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

		af_pse_close (NULLOBJID);
		if ( af_sign(oclist->ccert->tbs_DERcode, oclist->ccert->sig, SEC_END) < 0 ) {
			aux_add_error(ESIGN, "af_sign failed", (char *) oclist->ccert, Certificate_n, proc);
			return(-1);
		}

		if ( (oclist = oclist->next) == (OCList * )0 ) {
			aux_add_error(EINVALID, "oclist->next empty", CNULL, 0, proc);
			return (-1);
		}
		/*Directory table "Old Certificates" must be composed of at least two lines*/

		/* verify cross certificate in second line of obsolete directory table   */
		/* "Old Certificates":							 */

		vkey.key = aux_cpy_KeyInfo(pkroot->newkey->key);
		vkey.keyref = 0;
		vkey.pse_sel = (PSESel *) 0;

		/**vhashin = 0;*/
		alghash = aux_ObjId2AlgHash(oclist->ccert->sig->signAI->objid);
		/* needs HashInput parameter set */
		if (alghash == SQMODN) {
			vhashin = (HashInput * ) & vkey.key->subjectkey;
		}

		rc = sec_verify(oclist->ccert->tbs_DERcode, oclist->ccert->sig, SEC_END, &vkey, vhashin);
		if ( rc < 0 ) {
			aux_add_error(EVERIFY, "sec_verify failed for cross certificate", CNULL, 0, proc);
			return - 1;
		}

		oclist->ccert->tbs->serialnumber = af_pse_incr_serial();
		oclist->ccert->tbs->signatureAI = aux_cpy_AlgId(save_signAI);

		if ((oclist->ccert->tbs_DERcode = e_ToBeSigned(oclist->ccert->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ToBeSigned failed", (char *) oclist->ccert, Certificate_n, proc);
			return (-1);
		}

		fprintf(stderr, "\nThe following Cross Certificate is to be signed. Please check it:\n\n");
		aux_fprint_ToBeSigned(stderr, oclist->ccert->tbs);
		fprintf(stderr, "\nDo you want to sign the displayed Cross Certificate?\n");
		fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

		af_pse_close (NULLOBJID);
		if ( af_sign(oclist->ccert->tbs_DERcode, oclist->ccert->sig, SEC_END) < 0 ) {
			aux_add_error(ESIGN, "af_sign failed", (char *) oclist->ccert, Certificate_n, proc);
			return(-1);
		}

		oclist = oclist->next;

		while ( oclist ) {
			rc = sec_verify(oclist->ccert->tbs_DERcode, oclist->ccert->sig, SEC_END, &vkey, vhashin);
			if ( rc < 0 ) {
				aux_add_error(EVERIFY, "sec_verify failed for cross certificate", CNULL, 0, proc);
				return - 1;
			}

			oclist->ccert->tbs->serialnumber = af_pse_incr_serial();
			oclist->ccert->tbs->signatureAI = aux_cpy_AlgId(save_signAI);

			if ((oclist->ccert->tbs_DERcode = e_ToBeSigned(oclist->ccert->tbs)) == NULLOCTETSTRING) {
				aux_add_error(EENCODE, "e_ToBeSigned failed", CNULL, 0, proc);
				return (-1);
			}

			fprintf(stderr, "\nThe following Cross Certificate is to be signed. Please check it:\n\n");
			aux_fprint_ToBeSigned(stderr, oclist->ccert->tbs);
			fprintf(stderr, "\nDo you want to sign the displayed Cross Certificate?\n");
			fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

			af_pse_close (NULLOBJID);
			if ( af_sign(oclist->ccert->tbs_DERcode, oclist->ccert->sig, SEC_END) < 0 ) {
				aux_add_error(ESIGN, "af_sign failed", (char *) oclist->ccert, Certificate_n, proc);
				return(-1);
			}

			oclist = oclist->next;
		}  /*while*/

		aux_free_AlgId(&save_signAI);

	}  /*else*/

	if ( af_dir_enter_OCList(first_line) < 0 ) {
		aux_add_error(EINVALID, "af_dir_enter_OCList failed", CNULL, 0, proc);
		return(-1);
	}


	return(0);

}

#endif


