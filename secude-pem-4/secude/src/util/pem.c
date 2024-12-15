/*
 *  SecuDE Release 4.3 (GMD)
 */

/********************************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                                *
 *                                                                              *
 *                                                                              *
 *                         NOTICE                                               *
 *                                                                              *
 *    Acquisition, use, and distribution of this module                         *
 *    and related materials are subject to restrictions                         *
 *    mentioned in each volume of the documentation.                            *
 *                                                                              *
 *******************************************************************************/


/*-----------------------pem.c-------------------------------------------------*/
/* GMD Darmstadt Institut fuer TeleKooperationsTechnik (I2)                    */
/* Rheinstr. 75 / Dolivostr. 15                                                */
/* 6100 Darmstadt                                                              */
/* Project ``SecuDE'' 1990 / "SecuDe" 1991,92,93                               */
/*      Grimm/Luehe/Nausester/Schneider/Viebeg/                                */
/*      Surkau/Reichelt/Kolletzki                     et alii                  */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/* PACKAGE   util            VERSION   4.0                                     */
/*                              DATE   01.04.1992                              */
/*                                BY   Surkau                                  */
/*                                                                             */
/*                            REVIEW                                           */
/*                              DATE                                           */
/*                                BY                                           */
/* DESCRIPTION                                                                 */
/*   This is the main program of the RFC 1421 - 1424  PEM filter               */
/*                                                                             */
/* CALLS TO                                                                    */
/*                                                                             */
/*-----------------------------------------------------------------------------*/


#include "pem.h"
#include "cadb.h"



#ifdef __STDC__
/***************************************************************
 *
 * Headers from file pem.c
 *
 ***************************************************************/ 
       int      main    (int cnt, char *parms[]);
static int      pse_open        ();
static PemOperation     evaluate_args   (int cnt, char *parms[]);
static RC       check_names     (SET_OF_NAME *names);
static RC       store_crl       (SET_OF_CRLWithCertificates *set_of_pemcrlwithcerts);
static RC       ask_for_update_certificate      (KeyType type, Certificate *cert);
static RC       check_for_new_certs     (PemMessageLocal *local);
static void     usage   (int help);
#else
/***************************************************************
 *
 * Headers from file pem.c
 *
 ***************************************************************/ 
       int      main    ();
static int      pse_open        ();
static PemOperation     evaluate_args   ();
static RC       check_names     ();
static RC       store_crl       ();
static RC       ask_for_update_certificate      ();
static RC       check_for_new_certs     ();
static void     usage   ();
#endif

Boolean	pem_enter_issuer_certificate_into_pklist = FALSE,
	pem_ask_for_wrong_names = TRUE,
	pem_forwarding = FALSE,
	pem_option_r = FALSE,
	pem_option_y = FALSE,
	pem_update_for_failed_verification = FALSE,
	pem_insert_boundaries = TRUE,
	pem_originator_keyinfo = TRUE,
	PEM_Conformance_Requested = TRUE,
	pem_mail_header = FALSE;
int	verbose = 0,
	pem_cert_num = 0,
	pem_scan_depth_begin = 0,
	pem_scan_depth_number = 200;
char	*input_file = CNULL,
	*output_file = CNULL,
	*psename = CNULL,
	*cadir = CNULL;
UPDATE_Mode 	update_mode = UPDATE_ASK;
SET_OF_NAME 	*names = (SET_OF_NAME *)0, *names2;
PemOperation 	 pem_op = PEM_scan;
static int 	 pse = 0;
static char 	*pin = CNULL;


#define DOUBLE_OPTION(text) {aux_add_error(EINVALID, "Option is set twice", text, char_n, proc); return(PEM_error);}
#define NEEDS_PARM(text) {aux_add_error(EINVALID, "Option needs parameter", text, char_n, proc); return(PEM_error);}
#define ERROR {AUX_ADD_ERROR;aux_fprint_error(stderr, verbose);return(-1);}
#define MALLOC_ERROR {fprintf(stderr, "Can't allocate memory");return(-1);}











/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
#ifdef __STDC__

int main(
	int	  cnt,
	char	 *parms[]
)

#else

int main(
	cnt,
	parms
)
int	  cnt;
char	 *parms[];

#endif

{
	OctetString 		*ostr,
				*input_text,
				*output_text,
				 crls,
				*new_crls;
	SET_OF_PemMessageCanon 	*canonset,
				*canonset_new;
	PemMessageCanon 	*canon;
	char 			*psepath,
				*ppin = CNULL,
 				*originator_name,
 				*originator_alias,
 				*originator_mailadr,
 				*alias,
 				*pca,
				 ask_mess[512],
				 tmp_name[512],
				*subject, 
				*rfc822_header,
				*proc = "main (pem)";
	SET_OF_NAME		*hdr_names;
	Boolean  		 opened = FALSE;
	SET_OF_PemMessageLocal 	*localset,
				*localset2,
				*forward,
				*validateset,
				*certifyset = 0,
				*certifyset2;
	PemMessageLocal 	*local;
	SET_OF_PemRecLocal 	*recips;
	int 			 i,
				 empty_line,
	 			*proc_types,
				 rc;


	pem_op = evaluate_args(cnt, parms);
	if(pem_op == PEM_error) {
		aux_fprint_error(stderr, 1);
		usage(SHORT_HELP);
	}
	if(pem_op == PEM_help) {
                usage(LONG_HELP);
	}
	if(pem_forwarding)  {
		if(!(input_text = aux_file2OctetString(input_file))) ERROR

		if(!(canonset = pem_parse(input_text))) ERROR
		aux_free_OctetString(&input_text);

		if(!(localset = pem_CanonSet2LocalSet(canonset))) ERROR	
		aux_free_SET_OF_PemMessageCanon(&canonset);

		if(!(proc_types = pem_proctypes(localset))) ERROR
		if(proc_types[PEM_ENC] > 0 || pem_op == PEM_encrypted) if(pse_open() < 0) ERROR

		switch(pem_op) {
			case PEM_mic_clear:
				if(!(forward = pem_forwardSet(0, localset, PEM_MCC, (SET_OF_NAME *)0))) ERROR
				break;
			case PEM_mic_only:
				if(!(forward = pem_forwardSet(0, localset, PEM_MCO, (SET_OF_NAME *)0))) ERROR
				break;
			case PEM_encrypted:
				check_names(names);

				if(!(forward = pem_forwardSet(0, localset, PEM_ENC, names))) ERROR

				for(localset2 = forward; localset2; localset2 = localset2->next)
				if(localset2->element && localset2->element->header && localset2->element->header->dek_fields)
					for(recips = localset2->element->header->dek_fields->recipients; recips; recips = recips->next) {
						if(recips->element && recips->element->certificate) {
							switch(pem_look_for_Certificate(pse, ENCRYPTION, recips->element->certificate)) {
								case -1:
									ERROR
					
								case 0: /* not found */
	
									if(ask_for_update_certificate(ENCRYPTION, recips->element->certificate) < 0) ERROR
									break;
							}
						}
	
					}
				break;
		}
		aux_free_SET_OF_PemMessageLocal(&localset);

		if(!(canonset = pem_LocalSet2CanonSet(forward))) ERROR
		aux_free_SET_OF_PemMessageLocal(&forward);

		if(!(output_text = pem_build(canonset))) ERROR
		aux_free_SET_OF_PemMessageCanon(&canonset);

 		if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
		aux_free_OctetString(&output_text);


	}
	else switch(pem_op) {
		case PEM_scan:
			if(!(input_text = aux_file2OctetString(input_file))) ERROR
			
			if(!(canonset = pem_parse(input_text))) ERROR

			if(!(localset = pem_CanonSet2LocalSet(canonset))) ERROR	
			aux_free_SET_OF_PemMessageCanon(&canonset);

			for(i = pem_scan_depth_begin; i; i--) {

				aux_free_OctetString(&input_text);
				if(!(input_text = pem_mic_clear_bodys(localset))) ERROR
				aux_free_SET_OF_PemMessageLocal(&localset);

				if(!(canonset = pem_parse(input_text))) ERROR

				if(!(localset = pem_CanonSet2LocalSet(canonset))) ERROR
				aux_free_SET_OF_PemMessageCanon(&canonset);
			}
			if(!(proc_types = pem_proctypes(localset))) ERROR

			output_text = input_text;
			crls.noctets = 0;
			crls.octets = CNULL;

			for(i = pem_scan_depth_number; i && proc_types[0] >= 0; i--) {

				if(pse_open() < 0) ERROR

				if(proc_types[PEM_CRL_]) {
					for(localset2 = localset; localset2; localset2 = localset2->next)
					    if(localset2->element && localset2->element->header)
						if(store_crl(localset2->element->header->crl_fields) < 0 )  ERROR
	
				}
				if(proc_types[PEM_CRL_RETRIEVAL_REQUEST]) {
	
					if(!(new_crls = pem_reply_crl_requestSet(pse, localset))) ERROR
					if(aux_append_OctetString(&crls, new_crls) < 0) ERROR
				}

				if(!(validateset = pem_validateSet(pse, localset))) ERROR
				aux_free_SET_OF_PemMessageLocal(&localset);
				for(localset2 = validateset; localset2; localset2 = localset2->next) {
					if(localset2->element){
						if(localset2->element->header && localset2->element->header->certificates
								 && localset2->element->header->certificates->usercertificate
								&& localset2->element->header->certificates->usercertificate->tbs){

							fprintf(stderr, "\n\n");
							if(pem_verbose_2 && localset2->element->comment) fprintf(stderr, "%s\n", localset2->element->comment);
							originator_name = aux_DName2Name(localset2->element->header->certificates->usercertificate->tbs->subject);
							originator_alias = aux_DName2alias(localset2->element->header->certificates->usercertificate->tbs->subject, LOCALNAME);
							originator_mailadr = aux_DName2alias(localset2->element->header->certificates->usercertificate->tbs->subject, RFCMAIL);
							if(originator_alias) alias = originator_alias;
							else if(originator_mailadr) alias = originator_mailadr;
							else alias = originator_name;
						
							if(localset2->element->validation_result) {
								if(localset2->element->validation_result->success) fprintf(stderr, "MIC OK. Message signed by <%s>\n", alias);
								else fprintf(stderr, "MIC NOT OK. Message signed by <%s>\n", alias);
								if(localset2->element->validation_result->trustedKey == 2) fprintf(stderr, "Validated with provided RootKey-information\n");
							} else fprintf(stderr, "No validation result. Message signed by <%s>\n", alias);

						/*
						 *	print verifresult from pem verification
						 */ 
							if(originator_name) free(originator_name);
							if(originator_alias) free(originator_alias);
							if(originator_mailadr) free(originator_mailadr);
						}
						if(localset2->element->validation_result) {
							if(pem_verbose_0) aux_fprint_VerificationResult(stderr, localset2->element->validation_result);
							else {
								pca = aux_PCA(localset2->element->validation_result);
								if(pca) {
									fprintf(stderr, "Validated through POLICY CA <%s>\n", pca);
									if (pca) free (pca);
						
								}	
							}
						}
						if(localset2->element->error) fprintf(stderr, localset2->element->error);
						if(pem_update_for_failed_verification || (localset2->element->validation_result && localset2->element->validation_result->success))
							if(check_for_new_certs(localset2->element) < 0) ERROR
					}
				}
				
				aux_free_OctetString(&output_text);

				if(!(output_text = pem_LocalSet2Clear(validateset, pem_insert_boundaries))) ERROR
				aux_free_SET_OF_PemMessageLocal(&validateset);

				
				if(!(canonset = pem_parse(output_text))) ERROR

				if(!(localset = pem_CanonSet2LocalSet(canonset))) ERROR	
				aux_free_SET_OF_PemMessageCanon(&canonset);

				free(proc_types);
				if(!(proc_types = pem_proctypes(localset))) ERROR

			}	

			if(proc_types[PEM_CRL_]) {
				for(localset2 = localset; localset2; localset2 = localset2->next)
					if(store_crl(localset2->element->header->crl_fields) < 0 )  ERROR

			}
			if(proc_types[PEM_CRL_RETRIEVAL_REQUEST]) {

				if(!(new_crls = pem_reply_crl_requestSet(pse, localset))) ERROR
				if(aux_append_OctetString(&crls, new_crls) < 0) ERROR

			}
			if(crls.noctets) {
				if(aux_input_from_device("There are CRL retrieval requests.\nCreate CRL's (y/n) ?", 
					tmp_name, TRUE) < 0 )  ERROR
				if(!strcasecmp(tmp_name, "yes") || !strcasecmp(tmp_name, "y")) {

					if(aux_append_OctetString(output_text, &crls) < 0) ERROR

				}
			}


			aux_free_SET_OF_PemMessageLocal(&localset);

			free(proc_types);
			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);
			break;
		case PEM_mic_clear:
			if(!(input_text = aux_file2OctetString(input_file))) ERROR
			if(pse_open() < 0) ERROR

			if(!(local = pem_enhance(pse, PEM_MCC, input_text, pem_cert_num, (SET_OF_NAME *)0))) ERROR
			aux_free_OctetString(&input_text);

			if(!(canon = pem_Local2Canon(local))) ERROR
			aux_free_PemMessageLocal(&local);

			if(!(output_text = pem_build_one(canon))) ERROR
			aux_free_PemMessageCanon(&canon);

 			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);

			break;
		case PEM_mic_only:
			if(!(input_text = aux_file2OctetString(input_file))) ERROR
			if(pse_open() < 0) ERROR

			if(!(local = pem_enhance(pse, PEM_MCO, input_text, pem_cert_num, (SET_OF_NAME *)0))) ERROR
			aux_free_OctetString(&input_text);

			if(!(canon = pem_Local2Canon(local))) ERROR
			aux_free_PemMessageLocal(&local);

			if(!(output_text = pem_build_one(canon))) ERROR
			aux_free_PemMessageCanon(&canon);

 			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);

			break;
		case PEM_encrypted:
			if(!(input_text = aux_file2OctetString(input_file))) ERROR
			if(pse_open() < 0) ERROR

			if(pem_originator_keyinfo) {
				if(!(names2 = (SET_OF_NAME *) calloc(1, sizeof(SET_OF_NAME)))) MALLOC_ERROR
				names2->next = names;
				names2->element = (NAME *)0;
				names = names2;
			}
			if(pem_mail_header) {
				if(input_text->noctets < 2) empty_line = input_text->noctets;
				else for(empty_line = 2; empty_line < input_text->noctets && (
							input_text->octets[empty_line-1] != '\n' ||
							input_text->octets[empty_line-2] != '\n'); empty_line++);

				if(!(rfc822_header = CATNSTR(CNULL, input_text->octets, empty_line))) MALLOC_ERROR  
  
				if(!(hdr_names = pem_scan822hdr(rfc822_header))) ERROR
				if(names) {
					for(names2 = names; names2->next; names2 = names2->next);
					names2->next = hdr_names;
				}
				else names = hdr_names;
			}
			else empty_line = 0;

			check_names(names);

			if(!(localset = (SET_OF_PemMessageLocal *) calloc(1, sizeof(SET_OF_PemMessageLocal)))) MALLOC_ERROR

			if(!(localset->element = local = pem_enhance(pse, PEM_ENC, input_text + empty_line, pem_cert_num, names))) ERROR
			aux_free_OctetString(&input_text);

			if(local->header && local->header->dek_fields)
				for(recips = local->header->dek_fields->recipients; recips; recips = recips->next) {
					if(recips->element && recips->element->certificate) {
						switch(pem_look_for_Certificate(pse, ENCRYPTION, recips->element->certificate)) {
							case -1:
								ERROR
				
							case 0: /* not found */

								if(ask_for_update_certificate(ENCRYPTION, recips->element->certificate) < 0) ERROR
								break;
						}
					}

				}
			if(pem_mail_header) {
				if(!(localset2 = (SET_OF_PemMessageLocal *) calloc(1, sizeof(SET_OF_PemMessageLocal)))) MALLOC_ERROR
				if(!(localset2->element = (PemMessageLocal *) calloc(1, sizeof(PemMessageLocal)))) MALLOC_ERROR
				if(!(localset2->element->body = (OctetString *) calloc(1, sizeof(OctetString)))) MALLOC_ERROR

				localset2->element->body->octets = rfc822_header;
				localset2->element->body->noctets = empty_line;

				localset2->next = localset;
				localset = localset2;

			}

			if(!(canonset = pem_LocalSet2CanonSet(localset))) ERROR
			aux_free_SET_OF_PemMessageLocal(&localset);

			if(!(output_text = pem_build(canonset))) ERROR
			aux_free_SET_OF_PemMessageCanon(&canonset);

 			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);

			break;
		case PEM_certify:
			if(!(input_text = aux_file2OctetString(input_file))) ERROR
			
			canonset = pem_parse(input_text);
			aux_free_OctetString(&input_text);

			localset = pem_CanonSet2LocalSet(canonset);	
			aux_free_SET_OF_PemMessageCanon(&canonset);

			localset2 = localset;

			certifyset = (SET_OF_PemMessageLocal *)0;

			while(localset2) {
				if(certifyset) {
					if(!(certifyset2->next = (SET_OF_PemMessageLocal *) calloc(1, sizeof(SET_OF_PemMessageLocal)))) MALLOC_ERROR
					certifyset2 = certifyset2->next;
				}
				else {
					if(!(certifyset2 = (SET_OF_PemMessageLocal *) calloc(1, sizeof(SET_OF_PemMessageLocal)))) MALLOC_ERROR
					certifyset = certifyset2;
				}
				certifyset2->element = (PemMessageLocal *)0;
				certifyset2->next = (SET_OF_PemMessageLocal *)0;
				if(localset2->element && localset2->element->header && 
							localset2->element->header->certificates &&
							localset2->element->header->certificates->usercertificate &&
							localset2->element->header->certificates->usercertificate->tbs &&
							!localset2->element->header->certificates->forwardpath) {
					if(pse_open() < 0) ERROR
					subject = aux_DName2NameOrAlias( 
						localset2->element->header->certificates->usercertificate->tbs->subject);
					sprintf(ask_mess, "Sign certificate of <%s> (y/n) ?", subject);
					free(subject);
					if(aux_input_from_device(ask_mess, tmp_name, TRUE) < 0 )  ERROR
					if(!strcasecmp(tmp_name, "yes") || !strcasecmp(tmp_name, "y")) 
						certifyset2->element = pem_certify(pse, localset2->element);

					
				}
				if(!certifyset2->element) certifyset2->element = aux_cpy_PemMessageLocal(localset2->element);
				if(localset2->element && !certifyset2->element) ERROR
				localset2 = localset2->next;
			}
			aux_free_SET_OF_PemMessageLocal(&localset);

			if(!(canonset = pem_LocalSet2CanonSet(certifyset))) ERROR
			aux_free_SET_OF_PemMessageLocal(&certifyset);

			if(!(output_text = pem_build(canonset))) ERROR
			aux_free_SET_OF_PemMessageCanon(&canonset);

 			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);

			break;
		case PEM_crl:
			if(pse_open() < 0) ERROR
			check_names(names);
			if(!(local = pem_crl(pse, names))) ERROR

			if(!(canon = pem_Local2Canon(local))) ERROR
			aux_free_PemMessageLocal(&local);

			if(!(output_text = pem_build_one(canon))) ERROR
			aux_free_PemMessageCanon(&canon);

 			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);

			break;
		case PEM_crl_rr:
			if(pse_open() < 0) ERROR
			check_names(names);
			if(!(local = pem_crl_rr(pse, names))) ERROR

			if(!(canon = pem_Local2Canon(local))) ERROR
			aux_free_PemMessageLocal(&local);

			if(!(output_text = pem_build_one(canon))) ERROR
			aux_free_PemMessageCanon(&canon);

 			if(aux_OctetString2file(output_text, output_file, 2) < 0) ERROR
			aux_free_OctetString(&output_text);

			break;
	}
	pem_pse_close(pse);
	return(0);
}

/***************************************************************
 *
 * Procedure pse_open
 *
 ***************************************************************/
#ifdef __STDC__

static int pse_open(
)

#else

static int pse_open(
)

#endif

{
	char 	 wrong_pins_allowed = 3;
	char	*psepath,
		 prompt[100];
	int	 reenter = 0;
	PSELocation      SCapp_available;
	char    *proc = "pse_open";

	if(pse > 0) return(0);
	
	if (!psename) {
		if(cadir) {
			psename = getenv("CAPSE");
			if(!psename) psename = DEF_CAPSE;
		}
		else {
			psename = getenv("PSE");
			if(!psename) psename = DEF_PSE;
		}
	}

        if(cadir) {
		if(!(psepath = CATSPRINTF(CNULL, "%s%s%s", cadir, PATH_SEPARATION_STRING, psename))) MALLOC_ERROR    
 	        if(!pin) pin = getenv("CAPIN");
        }
        else {
                if(!(psepath = aux_cpy_String(psename))) MALLOC_ERROR
 	        if(!pin) pin = getenv("USERPIN");
        }

	SCapp_available = sec_psetest(psepath);
#ifdef SCA
	if (SCapp_available == ERR_in_psetest) {
		if (aux_last_error() == EDEVLOCK) 
			aux_add_error(EDEVLOCK, "Cannot open device for SCT (No such device or device busy).", CNULL, 0, proc);
		else	aux_add_error(ECONFIG, "Error during SC configuration.", CNULL, 0, proc);
		return (-1);
	}
#endif

	if(SCapp_available == SCpse) {
		if(pse = pem_pse_open(cadir, psepath, CNULL) < 0) {
			aux_add_error(EDEVLOCK, "Cannot open SC PSE.", CNULL, 0, proc);
			return(-1);
		}

		return(pse);
	}

	if(pin) {
		if((pse = pem_pse_open(cadir, psepath, pin) >= 0)) {
			free(pin);
			return(pse);
		}
		else free(pin);
	}

	while(wrong_pins_allowed) {

		sprintf(prompt, "%d: Enter PIN for %s: ", wrong_pins_allowed, psepath);

		pin = aux_getpass(prompt);

		if(!strlen(pin)) {
			free(pin);
			aux_add_error(EPIN, "No PIN", CNULL, 0, proc);
			return(-1);
		}
		if((pse = pem_pse_open(cadir, psepath, pin) >= 0)) {
			free(pin);
			return(pse);
		}
		else free(pin);

		if(aux_last_error() != EPIN) {
			return(-1);
		} else aux_free_error();

		wrong_pins_allowed--;
	}
	AUX_ADD_ERROR;
	return(-1);
}




/***************************************************************
 *
 * Procedure evaluate_args
 *
 ***************************************************************/
#ifdef __STDC__

static PemOperation evaluate_args(
	int	  cnt,
	char	 *parms[]
)

#else

static PemOperation evaluate_args(
	cnt,
	parms
)
int	  cnt;
char	 *parms[];

#endif

{
        char 		*proc = "evaluate_args";
        int              nparm,
			 iparm,
			 i;


        /* defaults: */

        MF_check = FALSE;
	
	chk_PEM_subordination = TRUE;

        for(nparm = 1, iparm = 0; nparm < cnt; nparm++, iparm = 0) switch(parms[nparm][iparm]) {
                case '-':
                        while(parms[nparm][++iparm]) switch(parms[nparm][iparm]) {
                                case 'h':
                                        return(PEM_help);
                                        break;
				case 'D':
					af_access_directory = TRUE;
					break;
				case 'a':
                                        if(pem_op != PEM_scan) {
						aux_add_error(EINVALID, "-a is possible for pem scan only", CNULL, 0, proc);
						return(PEM_error);
					}

					pem_insert_boundaries = FALSE;
					break;
				case 'U':
                                        if(pem_op != PEM_scan) {
						aux_add_error(EINVALID, "-a is possible for pem scan only", CNULL, 0, proc);
						return(PEM_error);
					}

					pem_update_for_failed_verification = TRUE;
					break;
				case 'I':
                                        if(pem_op != PEM_scan) {
						aux_add_error(EINVALID, "-a is possible for pem scan only", CNULL, 0, proc);
						return(PEM_error);
					}

					pem_enter_issuer_certificate_into_pklist = TRUE;
					break;
#ifdef X500
				case 'd':
                                        if(af_dir_dsaname) DOUBLE_OPTION("-d");
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-d");
					af_dir_dsaname = parms[++nparm];
                                        goto next_parm;
				case 't':
                                        if(af_dir_tailor) DOUBLE_OPTION("-t");
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-t");
					af_dir_tailor = parms[++nparm];
                                        goto next_parm;
				case 'A':
					nparm++;
					if (! strcasecmp(parms[nparm], "STRONG"))
						af_dir_authlevel = DBA_AUTH_STRONG;
					else if (! strcasecmp(parms[nparm], "SIMPLE"))
						af_dir_authlevel = DBA_AUTH_SIMPLE;
					break;
#endif
				case 'F':
					af_FCPath_is_trusted = TRUE;
					break;
				case 'f':
					af_check_Validity = FALSE;
					break;
				case 'R':
					af_chk_crl = TRUE;
					break;
                                case 'v':
                                        verbose = 1;
                                        break;
                                case 'V':
                                        verbose = 2;
                                        break;
                                case 'W':
                                        verbose = 3;
                                        break;
                                case 'Z':
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-Z");
                                        input_device = parms[++nparm];
                                        goto next_parm;
                                case 'z':
                                        MF_check = TRUE;
                                        break;

                                case 'm':
                                        if(pem_op != PEM_scan) {
						aux_add_error(EINVALID, "-m is possible for pem scan only", CNULL, 0, proc);
						return(PEM_error);
					}

                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-m");
                                        sscanf (parms[++nparm],"%d", &pem_scan_depth_begin);
                                        goto next_parm;
                                case 'M':
                                        if(pem_op != PEM_scan) {
						aux_add_error(EINVALID, "-M is possible for pem scan only", CNULL, 0, proc);
						return(PEM_error);
					}

                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-M");
                                        sscanf (parms[++nparm],"%d", &pem_scan_depth_number);
                                        goto next_parm;
                                case 'r':
                                        if(pem_op != PEM_encrypted) {
						aux_add_error(EINVALID, "-r is possible for pem encrypted only", CNULL, 0, proc);
						return(PEM_error);
					}

                                        pem_option_r = TRUE;
                                        break;
                                case 'y':
                                        if(pem_op != PEM_crl && pem_op != PEM_crl_rr) {
						aux_add_error(EINVALID, "-y is possible for pem crl or pem crl-retrieval-request", CNULL, 0, proc);
						return(PEM_error);
					}

                                        pem_option_y = TRUE;
                                        break;
#ifdef SCA
                                case 'T':
                                        SC_verify = TRUE;
                                        break;
#endif
                                case 'n':
                                        if(pem_op != PEM_encrypted) {
						aux_add_error(EINVALID, "-n is possible for pem encrypted only", CNULL, 0, proc);
						return(PEM_error);
					}

                                        pem_originator_keyinfo = FALSE;
                                        break;
                                case 'i':
                                        if(input_file) DOUBLE_OPTION("-i");
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-i");
                                        input_file = aux_cpy_String(parms[++nparm]);
                                        goto next_parm;
                                case 'o':
                                        if(output_file) DOUBLE_OPTION("-o");
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-o");
                                        output_file = aux_cpy_String(parms[++nparm]);
                                        goto next_parm;
                                case 'p':
                                        if(psename) DOUBLE_OPTION("-p");
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-p");
                                        psename = aux_cpy_String(parms[++nparm]);
                                        goto next_parm;
                                case 'u':
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-u");
					for(update_mode = UPDATE_ASK;
 					    update_modes [ update_mode ].name &&
						strncmp( update_modes [ update_mode ].name, 
						parms[nparm], strlen(parms[nparm]) );
					    update_mode++) ;
					if(!update_modes [ update_mode ].name) {
						aux_add_error(EINVALID, "Wrong key word for -u", parms[nparm], char_n, proc); 
						return(PEM_error);
					}
					++nparm;
                                        goto next_parm;
                                case 'j':
					pem_ask_for_wrong_names = FALSE;
                                        break;
                                case 'c':
                                        if(psename) DOUBLE_OPTION("-c");
                                       	if(nparm >= (cnt - 1)) NEEDS_PARM("-c");
                                        cadir = aux_cpy_String(parms[++nparm]);
					isCA = TRUE;
                                        goto next_parm;
                                case 'C':
                                        if(nparm >= (cnt - 1) || parms[nparm + 1][0] < '0' || parms[nparm + 1][0] > '9') {
						pem_cert_num = 127;
						break;
					}
                                        sscanf (parms[++nparm],"%d", &pem_cert_num);
					if(!pem_cert_num)  pem_cert_num = 127;
                                        goto next_parm;
                                case 'l':
                                        if(pem_op != PEM_encrypted) {
						aux_add_error(EINVALID, "-l is only possible for encrypted", CNULL, 0, proc);
						return(PEM_error);
					}

 					pem_mail_header = TRUE;
                                        break;
                                case 'L':
 					pem_forwarding = TRUE;
                                        break;
                                case 'X':
 					random_from_pse = TRUE;
                                        break;
                                case 'Y':
 					random_init_from_tty = TRUE;
                                        break;
				case 'N':
					PEM_Conformance_Requested = FALSE;
					break;
				case 'O':
					chk_PEM_subordination = FALSE;
					break;
                                case 'E':
                                        if(pem_op != PEM_encrypted) {
						aux_add_error(EINVALID, "-E is possible for pem encrypted only", CNULL, 0, proc);
						return(PEM_error);
					}

                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-E");
                                        MSG_ENC_ALG = parms[++nparm];
                                        if(aux_Name2AlgType(MSG_ENC_ALG) != SYM_ENC) {
						aux_add_error(EINVALID, "Message encryption algorithm must be symmetric", CNULL, 0, proc);
						return(PEM_error);
					}

					if(PEM_Conformance_Requested) {
						if(aux_Name2AlgEnc(MSG_ENC_ALG) != DES || aux_Name2AlgMode(MSG_ENC_ALG) != CBC) {
							aux_add_error(EINVALID, "RFC 1423 requires desCBC as message encryption algorithm", CNULL, 0, proc);
							return(PEM_error);
						}

					}
                                        goto next_parm;
                                case 'S':
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-S");
                                        MIC_ENC_ALG = parms[++nparm];
                                        if(aux_Name2AlgType(MIC_ENC_ALG) != ASYM_ENC) {
						aux_add_error(EINVALID, "MIC encryption algorithm must be asymmetric", CNULL, 0, proc);
						return(PEM_error);
					}

					if(aux_Name2AlgEnc(MIC_ENC_ALG) == DSA) MIC_ALG = "NIST-SHA";
					if(PEM_Conformance_Requested) {
						if(strcmp(MIC_ENC_ALG, "RSA")){
							aux_add_error(EINVALID, "RFC 1423 requires RSA as MIC encryption algorithm", CNULL, 0, proc);
							return(PEM_error);
						}

					}
                                        goto next_parm;
                                case 'K':
                                        if(pem_op != PEM_encrypted) {
						aux_add_error(EINVALID, "-K is possible for pem encrypted only", CNULL, 0, proc);
						return(PEM_error);
					}
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-K");
                                        DEK_ENC_ALG = parms[++nparm];
                                        if(aux_Name2AlgEnc(DEK_ENC_ALG) != RSA || aux_Name2AlgType(DEK_ENC_ALG) != ASYM_ENC) {
						aux_add_error(EINVALID, "DEK encryption algorithm must be RSA", CNULL, 0, proc);
						return(PEM_error);
					}
                                        goto next_parm;
                                case 'H':
                                        if(nparm >= (cnt - 1)) NEEDS_PARM("-H");
                                        MIC_ALG = parms[++nparm];
                                        if(aux_Name2AlgType(MIC_ALG) != HASH) {
						aux_add_error(EINVALID, "Message digest algorithm must be a hash algorithm", CNULL, 0, proc);
						return(PEM_error);
					}

					if(aux_Name2AlgHash(MIC_ALG) == SHA) MIC_ENC_ALG = "NIST-DSA";
					if(PEM_Conformance_Requested) {
						if(strcmp(MIC_ALG, "RSA-MD2") && strcmp(MIC_ALG, "RSA-MD5")) {
							aux_add_error(EINVALID, "RFC 1423 requires RSA-MD2 or RSA-MD5 as message digest algorithm", CNULL, 0, proc);
							return(PEM_error);
						}

					}
                                        goto next_parm;
                                default:
					aux_add_error(EINVALID, "No such option", sprintf(malloc(2), "%c", parms[nparm][iparm]), char_n, proc);
                                        return(PEM_error);
                        }
next_parm:              break;

                default:
                        if(strcasecmp(parms[nparm], "scan") == 0) {
                                pem_op = PEM_scan;
                                break;
                        }
                        if(strcasecmp(parms[nparm], "mic-only") == 0) {
                                pem_op = PEM_mic_only;
                                break;
                        }
                        if(strcasecmp(parms[nparm], "mic-clear") == 0) {
                                pem_op = PEM_mic_clear;
                                break;
                        }
                        if(strcasecmp(parms[nparm], "encrypted") == 0) {
                                pem_op = PEM_encrypted;
                                break;
                        }
                        if(strcasecmp(parms[nparm], "certify") == 0) {
                                pem_op = PEM_certify;
                                break;
                        }
                        if(strcasecmp(parms[nparm], "crl") == 0) {
                                pem_op = PEM_crl;
                                break;
                        }
                        if(strcasecmp(parms[nparm], "crl-rr") == 0) {
                                pem_op = PEM_crl_rr;
                                break;
                        }
                        if(pem_option_r || pem_option_y) {
				NEWSTRUCT(names2, SET_OF_NAME, PEM_error, )
				NEWSTRUCT(names2->element, NAME, PEM_error, )

				if(!(names2->element->alias = aux_cpy_String(parms[nparm]))) AUX_ERROR_RETURN(PEM_error, )

				names2->element->name = CNULL;
				names2->element->dname = (DName *) 0;
                                names2->next = names;
                                names = names2;

                                break;
                        }
	 		aux_add_error(EINVALID, "Unknown key word", parms[nparm], char_n, proc);
                       	return(PEM_error);
        }

        pem_verbose_0 =  (verbose >= 1);
        pem_verbose_1 =  (verbose >= 2);
        pem_verbose_2 =  (verbose >= 3);
	pem_verbose_level = verbose;



		
	if(pem_option_y && !names) {
		NEWSTRUCT(names, SET_OF_NAME, PEM_error, )
		NEWSTRUCT(names->element, NAME, PEM_error, )
		names->element->alias = CNULL;
		names->element->name = CNULL;
		names->element->dname = (DName *) 0;

		names->next = (SET_OF_NAME *)NULL;

	}

	return(pem_op);
}
/***************************************************************
 *
 * Procedure get_update_mode
 *
 ***************************************************************/
#ifdef __STDC__

static UPDATE_Mode get_update_mode(
	char		 *text,
	UPDATE_Mode	  std,
	char		  no,
	char		  yes,
	char		  pse,
	char		  cadb
)

#else

static UPDATE_Mode get_update_mode(
	text,
	std,
	no,
	yes,
	pse,
	cadb
)
char		 *text;
UPDATE_Mode	  std;
char		  no;
char		  yes;
char		  pse;
char		  cadb;

#endif

{
	char		puff[128];

	if(update_mode == UPDATE_ASK) {

again1:
		if(aux_input_from_device(text, puff, TRUE) < 0 )  {
			AUX_ADD_ERROR;
			return(UPDATE_NO);
		}



		if(strlen(puff) != 1) return(std);
		str_low(puff);
		if (puff[0] == no) return(UPDATE_NO);
		else if (puff[0] == yes) return(UPDATE_YES);
		else if (puff[0] == pse) return(UPDATE_PSE);
		else if (puff[0] == cadb) return(UPDATE_CADB);
		else goto again1;
		
	} else return(update_mode);

}
/***************************************************************
 *
 * Procedure check_names
 *
 ***************************************************************/
#ifdef __STDC__

static RC check_names(
	SET_OF_NAME	 *names
)

#else

static RC check_names(
	names
)
SET_OF_NAME	 *names;

#endif

{

	SET_OF_NAME *names2;
	char	ask_mess[512],
		tmp_name[512];

			if(pem_ask_for_wrong_names) {
				for(names2 = names; names2; names2 = names2->next)
					while(pem_check_name(pse, names2->element)) {
						sprintf(ask_mess, "<%s> is no correct name or alias. Enter replacement: ", 
						names2->element->alias);

						if(aux_input_from_device(ask_mess, tmp_name, TRUE) < 0 )  AUX_ERROR_RETURN(-1,)
						if(names2->element->alias) {
							free(names2->element->alias);
						}
						if(names2->element->name) {
							free(names2->element->name);
							names2->element->name = CNULL;
						}
						if(strlen(tmp_name) == 0) {
							free(names2->element);
							names2->element = (NAME *)0;
						}
						else if(!(names2->element->alias = aux_cpy_String(tmp_name))) AUX_ERROR_RETURN(-1,)
					}

			}
	return(0);
}
/***************************************************************
 *
 * Procedure store_crl
 *
 ***************************************************************/
#ifdef __STDC__

static RC store_crl(
	SET_OF_CRLWithCertificates	 *set_of_pemcrlwithcerts
)

#else

static RC store_crl(
	set_of_pemcrlwithcerts
)
SET_OF_CRLWithCertificates	 *set_of_pemcrlwithcerts;

#endif

{
	UPDATE_Mode		 upd_mode;

	char 			*proc = "store_crl";

	if(!set_of_pemcrlwithcerts) return(0);

	upd_mode = get_update_mode("\nStore Certificate Revocation List in  CA-database ( c ), PSE ( p )  or both ( y ) ? ", UPDATE_NO, 'n', 'y', 'p', 'c');

	if(upd_mode == UPDATE_CADB || upd_mode == UPDATE_YES){

		if(pem_cadb_store_crlset(pse, set_of_pemcrlwithcerts) < 0) AUX_ERROR_RETURN(-1, )
	}

	if(upd_mode == UPDATE_PSE || upd_mode == UPDATE_YES){
		if(pem_pse_store_crlset(pse, set_of_pemcrlwithcerts) < 0) AUX_ERROR_RETURN(-1, )
	}

	return(0);
}

/***************************************************************
 *
 * Procedure ask_for_update_certificate
 *
 ***************************************************************/
#ifdef __STDC__

static RC ask_for_update_certificate(
	KeyType		  type,
	Certificate	 *cert
)

#else

static RC ask_for_update_certificate(
	type,
	cert
)
KeyType		  type;
Certificate	 *cert;

#endif

{
	UPDATE_Mode upd_mode;
	Name *subject, *issuer, *top;
	char	buff[ 1000], ask_text[1000];

	if(!(issuer = aux_DName2NameOrAlias(cert->tbs->issuer)))
		AUX_ERROR_RETURN(-1,)
	if(!(subject = aux_DName2NameOrAlias(cert->tbs->subject)))
		AUX_ERROR_RETURN(-1,)

	sprintf(ask_text, "\nA new %s certificate of <%s>\nfound, signed by <%s>.\nSave into PKList (y/n) ? ", 
		type == SIGNATURE ? "signature" : "encryption ", subject, issuer);

	upd_mode = get_update_mode(ask_text, UPDATE_NO, 'n', 'y', 0, 0);

	if(upd_mode == UPDATE_YES || upd_mode == UPDATE_PSE) {
		if(update_mode == UPDATE_ASK) {
			do {
				sprintf(ask_text, "Enter alias name for <%s>: ", subject);

				if(aux_input_from_device(ask_text, buff, TRUE) < 0 ) 
					AUX_ERROR_RETURN(-1,)

			 } while(strlen(buff) && aux_add_alias(buff, cert->tbs->subject, useralias, TRUE, TRUE) < 0);

			do {
				sprintf(ask_text, "Enter mail address for <%s>: ", subject);

				if(aux_input_from_device(ask_text, buff, TRUE) < 0 ) 
					AUX_ERROR_RETURN(-1,)

			 } while(strlen(buff) && aux_add_alias(buff, cert->tbs->subject, useralias, TRUE, TRUE) < 0);
		}
		if(af_pse_add_PK(type, cert->tbs) < 0)  AUX_ERROR_RETURN(-1,) 
	} 

	free(issuer);
	free(subject);
	return(0);
}
/***************************************************************
 *
 * Procedure check_for_new_certs
 *
 ***************************************************************/
#ifdef __STDC__

static RC check_for_new_certs(
	PemMessageLocal	 *local
)

#else

static RC check_for_new_certs(
	local
)
PemMessageLocal	 *local;

#endif

{
	PemHeaderLocal *hd;
	UPDATE_Mode upd_mode;
	Name *subject, *issuer, *top;
	char	buff[ 1000], ask_text[1000];
	FCPath *path;
	SET_OF_Certificate *cross;

	if(update_mode == UPDATE_YES || update_mode == UPDATE_PSE || update_mode == UPDATE_ASK) {

		if(local && (hd = local->header) && hd->certificates) {
			if(hd->certificates->usercertificate && hd->certificates->usercertificate->tbs
						&& hd->certificates->usercertificate->sig) {

				switch(pem_look_for_Certificate(pse, SIGNATURE, hd->certificates->usercertificate)) {
					case -1:
						AUX_ERROR_RETURN(-1,)
				
					case 1: /* in PKList */
						break;
					case 2: /* own cert */
						break;
					case 3: /* signed reply of own prototype certificate */
			
						if(!(issuer = aux_DName2NameOrAlias(hd->certificates->usercertificate->tbs->issuer)))
							AUX_ERROR_RETURN(-1,)

						if(hd->root_certificate && hd->root_certificate->tbs) {

							if(!(top = aux_DName2NameOrAlias(hd->root_certificate->tbs->issuer)))
								AUX_ERROR_RETURN(-1,)

							sprintf(ask_text, "\nThis is a certification reply from <%s>,\nvalidated with root key of <%s>.\nUpdate PKRoot, FCPath and Cert (y/n) ? ", issuer, top);
						}
						else {
							top = 0;
							sprintf(ask_text, "\nThis is a certification reply from <%s>.\nUpdate FCPath and Cert (y/n) ? ", issuer);
						}

						free(issuer);
						if(top) free(top);
						upd_mode = get_update_mode(ask_text, UPDATE_NO, 'n', 'y', 0, 0);
	
						if(upd_mode == UPDATE_YES || upd_mode == UPDATE_PSE) 
						if(pem_accept_certification_reply(pse, local) < 0) AUX_ERROR_RETURN(-1,)
						break;
					case 0: /* not found */

						if(ask_for_update_certificate(SIGNATURE, hd->certificates->usercertificate) < 0) 
							AUX_ERROR_RETURN(-1,)
						break;


				}

			}
			if(pem_enter_issuer_certificate_into_pklist) {
				for(path = hd->certificates->forwardpath; path; path = path->next_forwardpath)
				    for(cross = path->liste; cross; cross = cross->next) {
	
					switch(pem_look_for_Certificate(pse, SIGNATURE, cross->element)) {
						case -1:
							AUX_ERROR_RETURN(-1,)
					
						case 0: /* not found */
	
							if(ask_for_update_certificate(SIGNATURE, cross->element) < 0) 
								AUX_ERROR_RETURN(-1,)
							break;
	
	
					}
	
				}
				if(hd->root_certificate) {
	
					switch(pem_look_for_Certificate(pse, SIGNATURE, hd->root_certificate)) {
						case -1:
							AUX_ERROR_RETURN(-1,)
					
						case 0: /* not found */
	
							if(ask_for_update_certificate(SIGNATURE, hd->root_certificate) < 0) 
								AUX_ERROR_RETURN(-1,)
							break;
	
	
					}
	
				}
			}
						

		}

	}

	return(0);
	
}




/***************************************************************
 *
 * Procedure usage
 *
 ***************************************************************/
#ifdef __STDC__

static void usage(
	int	  help
)

#else

static void usage(
	help
)
int	  help;

#endif

{
	aux_fprint_version(stderr);

        fprintf(stderr, "pem  Privacy Enhancement for Internet Electronic Mail\n\n");
        fprintf(stderr, "usage:\n\n");
        fprintf(stderr, "pem [ SCAN | MIC-CLEAR | MIC-ONLY | ENCRYPTED | CRL | CLR-RR | CERTIFY ]\n");
        fprintf(stderr, "    [-i <inputfile>] [-o <outputfile>] [-p <psename>] [-c <cadir>] [-m|M 1..200] [-u <update>]\n");
#ifdef X500
	fprintf(stderr, "    [-r <name1 ...> ] [-y <name1 ...> ] [-CnfFNOhvzVWRDTIXYZlL] [-d <dsaname>] [-t <dsaptailor>]\n");
#else
	fprintf(stderr, "    [-r <name1 ...> ] [-y <name1 ...> ] [-CnfFNOhvzVWRDTIXYZlL]\n");
#endif
        fprintf(stderr, "    [-H <mic-alg>] [-S <micenc-alg>] [-E <msgenc-alg>] [-K <dekenc-alg>]");
        fprintf(stderr, "\n\n");

        if(help == LONG_HELP) {

        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "SCAN             read PEM any Proc-Type, write clear body and/or update\n");
        fprintf(stderr, "                 PSE and/or CA-database according to -u (default)\n");
        fprintf(stderr, "MIC-CLEAR        read text file, write PEM Proc-Type MIC-CLEAR\n");
        fprintf(stderr, "MIC-ONLY         read text file, write PEM Proc-Type MIC-ONLY\n");
        fprintf(stderr, "ENCRYPTED        read text file, write PEM Proc Type ENCRYPTED according to -r\n");
        fprintf(stderr, "CRL              write PEM Proc-Type CRL according to -y\n");
        fprintf(stderr, "CRL-RR           write PEM Proc-Type CRL-RETRIEVAL-REQUEST according to -y\n");
        fprintf(stderr, "CERTIFY          read PEM Proc-Type MIC-CLEAR or MIC-ONLY, check whether it is certification\n");
        fprintf(stderr, "                 request, sign Prototype-certificate, write certification reply\n");
        fprintf(stderr, "-i <inputfile>   inputfile (default: stdin)\n");
        fprintf(stderr, "-o <outputfile>  outputfile (default: stdout)\n");
        fprintf(stderr, "-p <psename>     PSE name (default: %s)\n", DEF_PSE);
        fprintf(stderr, "-c <cadir>       name of CA-directory (default: %s)\n", DEF_CADIR);
        fprintf(stderr, "-N               use of non-PEM conformant algorithms allowed\n");
        fprintf(stderr, "-O               RFC 1422 DName subordination not required\n");
	fprintf(stderr, "-F               consider own FCPath as trusted\n");
	fprintf(stderr, "-f               do not check validity time frames\n");
        fprintf(stderr, "-R               consult CRLs during validation process\n");
        fprintf(stderr, "-D               retrieve missing certificates or CRLs from the Directory (X.500 or .af-db)\n");
#ifdef X500
	fprintf(stderr, "-d <dsaname>     name of the DSA to be initially accessed (default: locally configured DSA)\n");
	fprintf(stderr, "-t <dsaptailor>  Location of dsaptailor file (default: dsaptailor in the ISODE ETCDIR directory)\n");
	fprintf(stderr, "-A <authlevel>   level of authentication used for binding to the X.500 Directory\n");
#endif
        fprintf(stderr, "-Z <device>      Input device ( default is /dev/tty )\n");
        fprintf(stderr, "-h               write this help text\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for tests only)\n");
        fprintf(stderr, "-z               enable memory checking\n");
#ifdef SCA
        fprintf(stderr, "-T               verification of signature is to be done by the smartcard terminal\n");
#endif
        fprintf(stderr, "-X               Read random number generator seed from PSE-object Random\n");
        fprintf(stderr, "-Y               Init random number generator seed through keyboard input\n");
        fprintf(stderr, "\nOptions/parameters for de-enhancement only:\n");
        fprintf(stderr, "-a               Surround the resulting clear bodies with boundaries and add validation results.\n");
        fprintf(stderr, "-m <level>       depth of multi PEM body, which is to be de-enhanced (only if pem scan)\n");
        fprintf(stderr, "-M <level>       depth of multi PEM body, up to which is to be de-enhanced (only if pem scan)\n");
        fprintf(stderr, "-u <update>      mode for updating the PSE or CA-database after scanning a PEM-msg\n");
        fprintf(stderr, "                 (ask, yes, no, cadb, pse (default: ask))\n");
        fprintf(stderr, "-U               Allows an update of the PKlist in case of a failed verification\n");
        fprintf(stderr, "-I               store scanned issuer certificates as trusted key in PKList (if update mode set)\n");
        fprintf(stderr, "-y <issuers>     DNames or alias-names of issuers of CRLs or CRL-RRs (only if pem crl or pem clr-rr)\n");
        fprintf(stderr, "\nOptions/parameters for enhancement only:\n");
        fprintf(stderr, "-j               Ask to replace wrongs recipient names\n");
        fprintf(stderr, "-C               generate PEM-header with Originator-Certificate and all Issuer-Certificates\n");
        fprintf(stderr, "-C <number>      generate PEM-header with number Certificates\n");
        fprintf(stderr, "-r <recipients>  DNames or alias-names of recipients (only if pem encrypted)\n");
        fprintf(stderr, "                 (default: generate PEM-header with Originator-ID-Asymmetric)\n");
        fprintf(stderr, "-n               don't insert Key-Info header field for originator (only if pem ENCRYPTED)\n");
        fprintf(stderr, "-l               assume <inputfile> as RFC822-formatted. Enhance only the body (after the first blank\n");
        fprintf(stderr, "                 line). In case of ENCRYPTED, extract recipient names from \"To:\" and \"Cc:\" header\n");
        fprintf(stderr, "                 fields.\n");
        fprintf(stderr, "-L               change the Proc-Type of of the given PEM message.\n");
        fprintf(stderr, "-H <mic-alg>     MIC algorithm (default: %s)\n", MIC_ALG);
        fprintf(stderr, "-S <micenc-alg>  MIC encryption algorithm (default: %s)\n", MIC_ENC_ALG);
        fprintf(stderr, "-E <msgenc-alg>  Message encryption algorithm (default: %s) (only if pem encrypted)\n", MSG_ENC_ALG);
        fprintf(stderr, "-K <dekenc-alg>  DEK encryption algorithm (default: %s) (only if pem encrypted)\n", DEK_ENC_ALG);
	}

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM PEM */
}
