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

/*-----------------------pem_parse.c--------------------------------*/
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
/*   This modul presents functions to parse a                       */
/*   text for PEM message parts                  	            */
/*                                                                  */
/* EXPORT                                                           */
/*                                                                  */
/*  pem_parse()                                                     */
/*                                                                  */
/* CALLS TO                                                         */
/*                                                                  */
/*  aux_ functions                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
#include "pem.h"

/*------------------------------------------------------------------*/
/*  getparm() returns a copy of the                                 */
/*  next header field parameter after position *pos                 */
/*  of the PEM text text                                            */
/*  *pos is set to the position after this parameter                */
/*------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure getparm
 *
 ***************************************************************/
#ifdef __STDC__

static char *getparm(
	char	 *text,
	int	 *pos,
	int	  length
)

#else

static char *getparm(
	text,
	pos,
	length
)
char	 *text;
int	 *pos;
int	  length;

#endif

{
	int  	 pos2;
	char 	*parm = CNULL,
		*proc = "getparm";

	while(text[*pos] == ' ') (*pos)++;
	while(text[*pos] == 0x0d || text[*pos] == '\n') (*pos)++;
	pos2  = *pos;

	while(1) {
		while(text[*pos] != ',' && text[*pos] != '\n' && text[*pos] != 0x0d && *pos < length) (*pos)++;
		if(*pos >= length) {
			if(!(parm = CATNSTR(parm, text + pos2, *pos - pos2))) AUX_MALLOC_ERROR(CNULL, ;)
			return(parm);
		}
		switch(text[*pos]) {
			case ',' :
				if(!(parm = CATNSTR(parm, text + pos2, *pos - pos2))) AUX_MALLOC_ERROR(CNULL, ;)
				(*pos)++;
				return(parm);
			case '\n' :
				if(*pos + 1 == length || text[(*pos) + 1] != ' ') {
					if(!(parm = CATNSTR(parm, text + pos2, *pos - pos2))) AUX_MALLOC_ERROR(CNULL, ;)
					(*pos)++;
					return(parm);
				}
				else (*pos)++;
				break;
			case 0x0d :
				if(*pos + 1 == length || (text[(*pos) + 1] != ' ' && text[(*pos) + 1] != '\n')) {
					if(!(parm = CATNSTR(parm, text + pos2, *pos - pos2))) AUX_MALLOC_ERROR(CNULL, ;)
					(*pos)++;
					return(parm);
				}
				else if(*pos + 2 == length || (text[(*pos) + 1] == '\n' && text[(*pos) + 2] != ' ')) {
					if(!(parm = CATNSTR(parm, text + pos2, *pos - pos2))) AUX_MALLOC_ERROR(CNULL, ;)
					(*pos)+=2;
					return(parm);
				}
				else (*pos)++;
				break;
		}
	}
}


/*------------------------------------------------------------------
  pem_parse() parses a text for PEM message and clear message parts.
  It returns a list with an element for each part. An element has
  a pointer to a struct PemHeaderCanon which is nil in case of
  a clear message and contains the header parameters as character
  strings in case of a PEM message. The body part of the element is
  either the clear text or the body of the PEM message.

------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_parse
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_PemMessageCanon *pem_parse(
	OctetString	 *text
)

#else

SET_OF_PemMessageCanon *pem_parse(
	text
)
OctetString	 *text;

#endif

{
	SET_OF_PemMessageCanon 	*canon_mess_ret,
				*canon_mess,
				*canon_mess_tmp;
	PemMessageCanon 	*canon;
	SET_OF_PemCrlCanon 	*crl;
	SET_OF_Name 		*issuer_cert,
				*issuer_crl_rr,
				*issuer_crl_cert = (SET_OF_Name *)0;
	SET_OF_PemRecCanon 	*recips = ( SET_OF_PemRecCanon *)0;
	int 			 header_fields,
				 end_mode = 0,
				 end_of_body,
				 n = 0,
				 length,
				 rc,
				 next_boundary,
				 lbegin = strlen(PEM_Boundary_Begin),
				 lend = strlen(PEM_Boundary_End);
	char			*proc = "pem_parse";


	if(!text) return((SET_OF_PemMessageCanon *)0);
	length = text->noctets;


	NEWSTRUCT(canon_mess_ret, SET_OF_PemMessageCanon, (SET_OF_PemMessageCanon *)0, ;)
	NEWSTRUCT(canon_mess_ret->element, PemMessageCanon, (SET_OF_PemMessageCanon *)0, free(canon_mess_ret))
	canon_mess_ret->next = (SET_OF_PemMessageCanon *)0;

	canon_mess = canon_mess_ret;
	canon = canon_mess_ret->element;
	canon->header = (PemHeaderCanon *)0;




	do {
		next_boundary = n;
		if(!end_mode)
		rc = aux_searchitem(text, PEM_Boundary_Begin, &next_boundary);

		if(rc == -1) {
			NEWSTRING(canon->body, length - n, (SET_OF_PemMessageCanon *)0, 
				aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
			strncpy(canon->body, text->octets + n, length - n);
		}
		else {
			if(!end_mode && next_boundary - lbegin - n) {
				NEWSTRING(canon->body, next_boundary - lbegin - n, (SET_OF_PemMessageCanon *)0, 
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
				strncpy(canon->body, text->octets + n, next_boundary - lbegin - n);



				NEWSTRUCT(canon_mess->next, SET_OF_PemMessageCanon, (SET_OF_PemMessageCanon *)0, 
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
				canon_mess = canon_mess->next;
				canon_mess->next = (SET_OF_PemMessageCanon *)0;
				NEWSTRUCT(canon_mess->element, PemMessageCanon, (SET_OF_PemMessageCanon *)0, 
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
				canon = canon_mess->element;
				canon->body = CNULL;
			}
			NEWSTRUCT(canon->header, PemHeaderCanon, (SET_OF_PemMessageCanon *)0, aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
			canon->header->rfc_version = CNULL;
			canon->header->proctype = CNULL;
			canon->header->content_domain = CNULL;
			canon->header->dek_fields = (PemDekCanon *)0;
			canon->header->orig_fields = (PemOriginatorCanon *)0;
			canon->header->issuer_fields = (SET_OF_Name *)0;
			canon->header->mic_fields = (PemMicCanon *)0;
			canon->header->crl_fields = (SET_OF_PemCrlCanon *)0;
			canon->header->crl_rr_fields = (SET_OF_Name *)0;


			n = next_boundary;
			if(!end_mode) {
				while(n < length-1 && text->octets[n] != '\n') n++;
				n++;
			}
			while(text->octets[n] != '\n' && text->octets[n] != 0x0d) {
				for(header_fields = 0; rXH_kwl[header_fields].name; header_fields++)
				    if(strlen(rXH_kwl[header_fields].name) <= length-n)
				    if(!strncmp(rXH_kwl[header_fields].name, text->octets + n, strlen(rXH_kwl[header_fields].name))) break;
				if(!rXH_kwl[header_fields].name) {
					aux_add_error(EMSGBUF, "Header Field expected", CNULL, 0, proc);
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret);
					return((SET_OF_PemMessageCanon *)0);
				}
				n += strlen(rXH_kwl[header_fields].name) + 1;

				switch(rXH_kwl[header_fields].value) {
					case PEM_PROC_TYPE:
						canon->header->rfc_version = getparm(text->octets, &n, length);
						canon->header->proctype = getparm(text->octets, &n, length);
						break;
					case PEM_CRL_:
						if(canon->header->crl_fields) {
						
							NEWSTRUCT(crl->next, SET_OF_PemCrlCanon, (SET_OF_PemMessageCanon *)0,
 								aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
							crl = crl->next;
						}
						else {
							NEWSTRUCT(crl, SET_OF_PemCrlCanon, (SET_OF_PemMessageCanon *)0,
 								aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
							canon->header->crl_fields = crl;
						}
						crl->next = (SET_OF_PemCrlCanon *)0;


						NEWSTRUCT(crl->element, PemCrlCanon, (SET_OF_PemMessageCanon *)0,
 							aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
						crl->element->crl = getparm(text->octets, &n, length);
						crl->element->certificate = CNULL;
						crl->element->issuer_certificate = (SET_OF_Name *)0;
						break;
					case PEM_CONTENT_DOMAIN:
						canon->header->content_domain = getparm(text->octets, &n, length);
						break;
					case PEM_DEK_INFO:
						NEWSTRUCT(canon->header->dek_fields, PemDekCanon, (SET_OF_PemMessageCanon *)0,
 							aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
						canon->header->dek_fields->dekinfo_enc_alg = getparm(text->octets, &n, length);
						canon->header->dek_fields->dekinfo_param = getparm(text->octets, &n, length);
						canon->header->dek_fields->keyinfo_enc_alg = CNULL;
						canon->header->dek_fields->keyinfo_dek = CNULL;
						canon->header->dek_fields->recipients = (SET_OF_PemRecCanon *)0;
						break;
					case PEM_SENDER_ID:
						NEWSTRUCT(canon->header->orig_fields, PemOriginatorCanon, (SET_OF_PemMessageCanon *)0,
 							aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
						canon->header->orig_fields->certificate = CNULL;
						canon->header->orig_fields->issuer = getparm(text->octets, &n, length);
						canon->header->orig_fields->serialnumber = getparm(text->octets, &n, length);


						break;
					case PEM_SENDER_IDS:
						aux_add_error(EINVALID, "Symetric ID's not supported", CNULL, 0, proc);
						aux_free_SET_OF_PemMessageCanon(&canon_mess_ret);
						return((SET_OF_PemMessageCanon *)0);
						break;
					case PEM_CERTIFICATE:
						if(canon->header->crl_fields) {
							crl->element->certificate = getparm(text->octets, &n, length);
						}
						else {
							NEWSTRUCT(canon->header->orig_fields, PemOriginatorCanon, (SET_OF_PemMessageCanon *)0,
 								aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
							canon->header->orig_fields->certificate = getparm(text->octets, &n, length);
							canon->header->orig_fields->issuer = CNULL;
							canon->header->orig_fields->serialnumber = CNULL;


						}
						break;
					case PEM_ISSUER_CERTIFICATE:
						if(canon->header->crl_fields) {
							if(crl->element->issuer_certificate) {
							
								NEWSTRUCT(issuer_crl_cert->next, SET_OF_Name, (SET_OF_PemMessageCanon *)0,
									 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
								issuer_crl_cert = issuer_crl_cert->next;
							}
							else {
								NEWSTRUCT(issuer_crl_cert, SET_OF_Name, (SET_OF_PemMessageCanon *)0,
									 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
								crl->element->issuer_certificate = issuer_crl_cert;
							}
							issuer_crl_cert->next = (SET_OF_Name *)0;
							issuer_crl_cert->element = getparm(text->octets, &n, length);
						}
						else {
							if(canon->header->issuer_fields) {
							
								NEWSTRUCT(issuer_cert->next, SET_OF_Name, (SET_OF_PemMessageCanon *)0,
									 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
								issuer_cert = issuer_cert->next;
							}
							else {
								NEWSTRUCT(issuer_cert, SET_OF_Name, (SET_OF_PemMessageCanon *)0,
									 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
								canon->header->issuer_fields = issuer_cert;
							}
							issuer_cert->next = (SET_OF_Name *)0;
							issuer_cert->element = getparm(text->octets, &n, length);
						}

						break;
					case PEM_MIC_INFO:
						NEWSTRUCT(canon->header->mic_fields, PemMicCanon, (SET_OF_PemMessageCanon *)0,
 							aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
						canon->header->mic_fields->mic_alg = getparm(text->octets, &n, length);
						canon->header->mic_fields->micenc_alg = getparm(text->octets, &n, length);
						canon->header->mic_fields->mic = getparm(text->octets, &n, length);
						break;
					case PEM_RECIPIENT_ID:
						if(canon->header->dek_fields) {


							if(canon->header->dek_fields->recipients) {
							
								NEWSTRUCT(recips->next, SET_OF_PemRecCanon, (SET_OF_PemMessageCanon *)0,
									 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
								recips = recips->next;
							}
							else {
								NEWSTRUCT(recips, SET_OF_PemRecCanon, (SET_OF_PemMessageCanon *)0,
									 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
								canon->header->dek_fields->recipients = recips;
							}
							recips->next = (SET_OF_PemRecCanon *)0;


							NEWSTRUCT(recips->element, PemRecCanon, 
								(SET_OF_PemMessageCanon *)0,
								 aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
							recips->element->issuer = getparm(text->octets, &n, length);
							recips->element->serialnumber = getparm(text->octets, &n, length);
							recips->element->enc_alg = CNULL;
							recips->element->dek = CNULL;
						}
						break;
					case PEM_ID_SYMMETRIC:
						aux_add_error(EINVALID, "Symetric ID's not supported", CNULL, 0, proc);
						aux_free_SET_OF_PemMessageCanon(&canon_mess_ret);
						return((SET_OF_PemMessageCanon *)0);
						break;
					case PEM_KEY_INFO:
						if(canon->header->dek_fields) 
						if(recips && recips->element) {
							recips->element->enc_alg = getparm(text->octets, &n, length);
							recips->element->dek = getparm(text->octets, &n, length);
						}
						else {
							canon->header->dek_fields->keyinfo_enc_alg = getparm(text->octets, &n, length);
							canon->header->dek_fields->keyinfo_dek = getparm(text->octets, &n, length);

						}
						break;
					case PEM_ISSUER:
						if(canon->header->crl_rr_fields) {
						
							NEWSTRUCT(issuer_crl_rr->next, SET_OF_Name, (SET_OF_PemMessageCanon *)0,
 								aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
							issuer_crl_rr = issuer_crl_rr->next;
						}
						else {
							NEWSTRUCT(issuer_crl_rr, SET_OF_Name, (SET_OF_PemMessageCanon *)0,
 								aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
							canon->header->crl_rr_fields = issuer_crl_rr;
						}
						issuer_crl_rr->next = (SET_OF_Name *)0;
						issuer_crl_rr->element = getparm(text->octets, &n, length);


						break;
				}
	
			}
			if(text->octets[n] == 0x0d) n++;

			n++;
			next_boundary = n;
			do {
				if(aux_searchitem(text, PEM_Boundary_Com, &next_boundary) < 0) {
					aux_add_error(EMSGBUF, "No PEM boundary end line", CNULL, 0, proc);
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret);
					return((SET_OF_PemMessageCanon *)0);
				}
				if(!strncmp(PEM_Boundary_End, text->octets + (next_boundary - lend), lend) &&
					text->octets[next_boundary - lend - 1] == '\n') {
						end_mode = 0;
						end_of_body = next_boundary - lend;
				}
				else if(!strncmp(PEM_Boundary_Begin, text->octets + (next_boundary - lbegin), lbegin) &&
					text->octets[next_boundary - lbegin - 1] == '\n') {
						end_mode = 1;
						end_of_body = next_boundary - lbegin;
				}
				else end_mode = -1;
				
			} while (end_mode < 0);

			if(!(canon->body = CATNSTR(CNULL, text->octets + n, end_of_body - n)))
				AUX_MALLOC_ERROR((SET_OF_PemMessageCanon *)0, aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))

			n = next_boundary;
			while(n < length && text->octets[n] != '\n') n++;
			if(text->octets[n] == '\n') n++;

			if(n == length) rc = 1;
			else {
				NEWSTRUCT(canon_mess->next, SET_OF_PemMessageCanon, (SET_OF_PemMessageCanon *)0, 
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
				canon_mess = canon_mess->next;
				canon_mess->next = (SET_OF_PemMessageCanon *)0;
				NEWSTRUCT(canon_mess->element, PemMessageCanon, (SET_OF_PemMessageCanon *)0, 
					aux_free_SET_OF_PemMessageCanon(&canon_mess_ret))
				canon = canon_mess->element;
				canon->header = (PemHeaderCanon *)0;
				canon->body = CNULL;

			}
			
		}
	} while(!rc);


	return(canon_mess_ret);
}
