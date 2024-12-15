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

/*-----------------------pem_build.c---------------------------------*/
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
/*   This modul presents functions to build                         */
/*   a PEM message from a canonical structure                       */
/*   and functions to extract the clear text                        */
/*   of a local structure                                           */
/*                                                                  */
/* EXPORT                                                           */
/*                                                                  */
/*  pem_Local2Clear()                                               */
/*  pem_LocalSet2Clear()                                            */
/*  pem_mic_clear_bodys()                                           */
/*  pem_build_one()                                                 */
/*  pem_build()                                                     */
/*                                                                  */
/* CALLS TO                                                         */
/*                                                                  */
/*  aux_ functions                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
#include "pem.h"

/*------------------------------------------------------------------
  pem_Local2Clear() extracts the clear text of a message. In case
  of a validated PEM message it can be encapsulated with 
  boundary lines and validation information by setting
  insert_boundaries == TRUE
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_Local2Clear
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_Local2Clear(
	PemMessageLocal	 *local,
	Boolean		  insert_boundaries
)

#else

OctetString *pem_Local2Clear(
	local,
	insert_boundaries
)
PemMessageLocal	 *local;
Boolean		  insert_boundaries;

#endif

{
	char 		*proc = "pem_Local2Clear";
	OctetString	*message;
	Name 		*name, *pca;
	
	NEWSTRUCT(message, OctetString, (OctetString *)0, ;)
	message->octets = CNULL;
	message->noctets = 0;


	if( local) {

		if(insert_boundaries && local->header) {

			if(aux_append_char(message, "------Begin of PEM message body\n") < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

			if(aux_sappend_char(message, "------Proc-Type: %s\n", proc_type_t[local->header->proctype].name) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

			if(local->validation_result) {

				if(aux_sappend_char(message, "------MIC %s\n", 
						local->validation_result->success ? "OK" : "NOT OK") < 0) 
							AUX_ERROR_RETURN((OctetString *)0, ;)
				if(local->validation_result->trustedKey == 2)
						if(aux_append_char(message, "------Validated with provided RootKey-information\n") < 0) 
							AUX_ERROR_RETURN((OctetString *)0, ;)
				pca = aux_PCA(local->validation_result);
				if(pca) {
					if(aux_sappend_char(message, "Validated through POLICY CA <%s>\n", pca) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
					if (pca) free (pca);
		
				}	

			}
			else if(aux_append_char(message, "------No Validation result\n") < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

			if(local->header) {
				if(local->header->certificates &&
						local->header->certificates->usercertificate &&
						local->header->certificates->usercertificate->tbs) {
					if( local->header->certificates->usercertificate->tbs->subject &&
						(name = aux_DName2Name(local->header->certificates->usercertificate->tbs->subject))) {

						if(aux_sappend_char(message, "------Originator is %s\n", name) < 0) 
							AUX_ERROR_RETURN((OctetString *)0, free(name))

						free(name);
					}
					if( local->header->certificates->usercertificate->tbs->issuer &&
						(name = aux_DName2Name(local->header->certificates->usercertificate->tbs->issuer))) {

						if(aux_sappend_char(message, "------Issuer is %s\n", name) < 0) 
							AUX_ERROR_RETURN((OctetString *)0, free(name))

						free(name);
					}
				}

			}
			

		}

		if(local->body && (!local->header || local->validation_result)) if(aux_append_OctetString(message, local->body) < 0) AUX_ERROR_RETURN((OctetString *)0, ;)


		if(insert_boundaries && local->header) 
			if(aux_append_char(message, "------End of PEM message body\n") < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
	}


	return(message);
}

/*------------------------------------------------------------------
  pem_LocalSet2Clear() concats the clear texts of all message
  elements. Validated PEM message can be encapsulated with 
  boundary lines and validation information by setting
  insert_boundaries == TRUE
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_LocalSet2Clear
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_LocalSet2Clear(
	SET_OF_PemMessageLocal	 *local_mess,
	Boolean			  insert_boundaries
)

#else

OctetString *pem_LocalSet2Clear(
	local_mess,
	insert_boundaries
)
SET_OF_PemMessageLocal	 *local_mess;
Boolean			  insert_boundaries;

#endif

{
	char 		*proc = "pem_LocalSet2Clear";
	OctetString	*message,
			*new;
	

	NEWSTRUCT(message, OctetString, (OctetString *)0, ;)
	message->octets = CNULL;
	message->noctets = 0;

	for(;local_mess; local_mess = local_mess->next)	{

		new = pem_Local2Clear(local_mess->element, insert_boundaries);
		if(new)	{
			if(aux_append_OctetString(message, new) < 0) AUX_ERROR_RETURN((OctetString *)0, ;)
			aux_free_OctetString(&new);
		}
		else AUX_ERROR_RETURN((OctetString *)0, aux_free_OctetString(&message))
		

	}

	return(message);
}
/*------------------------------------------------------------------
  pem_mic_clear_bodys() concats the clear texts of all
  PEM MIC-clear nad MIC-only message elements.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_mic_clear_bodys
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_mic_clear_bodys(
	SET_OF_PemMessageLocal	 *local_mess
)

#else

OctetString *pem_mic_clear_bodys(
	local_mess
)
SET_OF_PemMessageLocal	 *local_mess;

#endif

{
	char 		*proc = "pem_mic_clear_bodys";
	OctetString	*message,
			*new;

	NEWSTRUCT(message, OctetString, (OctetString *)0, ;)
	message->octets = CNULL;
	message->noctets = 0;

	for(;local_mess; local_mess = local_mess->next)	
	    if(local_mess->element && local_mess->element->header && (local_mess->element->header->proctype == PEM_MCC ||
				local_mess->element->header->proctype == PEM_MCO)) {

		new = pem_Local2Clear(local_mess->element, 0);
		if(new)	{
			if(aux_append_OctetString(message, new) < 0) AUX_ERROR_RETURN((OctetString *)0, ;)
			aux_free_OctetString(&new);
		}
		else AUX_ERROR_RETURN((OctetString *)0, aux_free_OctetString(&message))
		

	}

	return(message);
}


/*------------------------------------------------------------------
  pem_build_one() builds a message from a canon form
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_build_one
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_build_one(
	PemMessageCanon	 *message
)

#else

OctetString *pem_build_one(
	message
)
PemMessageCanon	 *message;

#endif

{
	char 			*proc = "pem_build_one";	
	SET_OF_Name 		*issuer;
	SET_OF_PemRecCanon	*recips;
	SET_OF_PemCrlCanon	*crls;
	OctetString		*res_ostr;

	if(!message) return((OctetString *)0);

	if(!message->header) {
		if(!(res_ostr = aux_create_OctetString(message->body))) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
		return(res_ostr);
	}

	NEWSTRUCT(res_ostr, OctetString, (OctetString *)0, ;)
	res_ostr->octets = CNULL;
	res_ostr->noctets = 0;


	if(aux_sappend_char(res_ostr, "%s\n", PEM_Boundary_Begin) < 0) 
				AUX_ERROR_RETURN((OctetString *)0, ;)



	if(message->header->rfc_version && message->header->proctype) 
	if(aux_sappend_char(res_ostr, "%s: %s,%s\n", rXH_kwl[PEM_PROC_TYPE].name,
					message->header->rfc_version, message->header->proctype) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)


	if(message->header->content_domain)
	if(aux_sappend_char(res_ostr, "%s: %s\n", rXH_kwl[PEM_CONTENT_DOMAIN].name,
					message->header->content_domain) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)



	if(message->header->dek_fields) 
		if(message->header->dek_fields->dekinfo_enc_alg && message->header->dek_fields->dekinfo_param)
		if(aux_sappend_char(res_ostr, "%s: %s,%s\n", rXH_kwl[PEM_DEK_INFO].name,
					message->header->dek_fields->dekinfo_enc_alg,
					message->header->dek_fields->dekinfo_param) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
	if(message->header->orig_fields) 
		if(message->header->orig_fields->certificate) {
			if(aux_sappend_char(res_ostr, "%s: \n%s", rXH_kwl[PEM_CERTIFICATE].name,
					message->header->orig_fields->certificate) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
		}
		else if(message->header->orig_fields->issuer && message->header->orig_fields->serialnumber){
			if(aux_sappend_char(res_ostr, "%s: \n%s ,%s\n", rXH_kwl[PEM_SENDER_ID].name,
					message->header->orig_fields->issuer,
					message->header->orig_fields->serialnumber) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
		}

	if(message->header->dek_fields) 
		if(message->header->dek_fields->keyinfo_enc_alg && message->header->dek_fields->keyinfo_dek)
			if(aux_sappend_char(res_ostr, "%s: %s,\n%s", rXH_kwl[PEM_KEY_INFO].name,
					message->header->dek_fields->keyinfo_enc_alg,
					message->header->dek_fields->keyinfo_dek) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)


	for(issuer = message->header->issuer_fields; issuer; issuer = issuer->next)
		if(issuer->element) 
			if(aux_sappend_char(res_ostr, "%s: \n%s", rXH_kwl[PEM_ISSUER_CERTIFICATE].name,
					issuer->element) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)



	if(message->header->mic_fields) 
		if(	message->header->mic_fields->mic_alg &&
			message->header->mic_fields->micenc_alg &&
			message->header->mic_fields->mic)
			if(aux_sappend_char(res_ostr, "%s: %s,%s,\n%s", rXH_kwl[PEM_MIC_INFO].name,
					message->header->mic_fields->mic_alg,
					message->header->mic_fields->micenc_alg,
					message->header->mic_fields->mic) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

	if(message->header->dek_fields) 
	for(recips = message->header->dek_fields->recipients; recips; recips = recips->next)
		if(recips->element && 	recips->element->enc_alg &&
					recips->element->dek &&
					recips->element->issuer &&
					recips->element->serialnumber) 
			if(aux_sappend_char(res_ostr, "%s:\n%s ,%s\n%s: %s,\n%s", rXH_kwl[PEM_RECIPIENT_ID].name,
							recips->element->issuer,
							recips->element->serialnumber,
							rXH_kwl[PEM_KEY_INFO].name,
							recips->element->enc_alg,
							recips->element->dek) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)


	for(issuer = message->header->crl_rr_fields; issuer; issuer = issuer->next)
		if(issuer->element) 
			if(aux_sappend_char(res_ostr, "%s: \n%s", rXH_kwl[PEM_ISSUER].name,
					issuer->element) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

	for(crls = message->header->crl_fields; crls; crls = crls->next)
		if(crls->element) {
			if(crls->element->crl)
				if(aux_sappend_char(res_ostr, "%s: \n%s", rXH_kwl[PEM_CRL_].name,
					crls->element->crl) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

			if(crls->element->certificate)
				if(aux_sappend_char(res_ostr, "%s: \n%s", rXH_kwl[PEM_CERTIFICATE].name,
					crls->element->crl) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)
		}

	if(aux_append_char(res_ostr, "\n") < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

	if(aux_append_char(res_ostr, message->body) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)

	if(aux_sappend_char(res_ostr, "%s\n", PEM_Boundary_End) < 0) 
						AUX_ERROR_RETURN((OctetString *)0, ;)



	return(res_ostr);

}
/*------------------------------------------------------------------
  pem_build() builds a message from a set of canon forms
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_build
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *pem_build(
	SET_OF_PemMessageCanon	 *canon_mess
)

#else

OctetString *pem_build(
	canon_mess
)
SET_OF_PemMessageCanon	 *canon_mess;

#endif

{
	char 		*proc = "pem_build";
	OctetString	*message,
			*new;

	NEWSTRUCT(message, OctetString, (OctetString *)0, ;)
	message->octets = CNULL;
	message->noctets = 0;


	for(;canon_mess; canon_mess = canon_mess->next)	{

		new = pem_build_one(canon_mess->element);
		if(new)	{
			if(aux_append_OctetString(message, new) < 0) AUX_ERROR_RETURN((OctetString *)0, ;)
			aux_free_OctetString(&new);
		}
		else AUX_ERROR_RETURN((OctetString *)0, aux_free_OctetString(&message))
		

	}

	return(message);
}




