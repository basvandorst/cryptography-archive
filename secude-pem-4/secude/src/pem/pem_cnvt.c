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

/*-----------------------pem_cnvt.c---------------------------------*/
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
/*   This modul presents functions to convert                       */
/*   PEM message parameters between                   	            */
/*   canonical and local forms                                      */
/*                                                                  */
/* EXPORT                                                           */
/*                                                                  */
/*  pem_Local2Canon()                                               */
/*  pem_Canon2Local()                                               */
/*  pem_Local2CanonSet()                                            */
/*  pem_Canon2LocalSet()                                            */
/*  pem_proctypes()                                                 */
/*                                                                  */
/* CALLS TO                                                         */
/*                                                                  */
/*  aux_ functions                                                  */
/*  e_ functions                                                    */
/*  d_ functions                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
#include "pem.h"



/*------------------------------------------------------------------
  pem_proctypes() counts the number of Proc-Type's in a local
  message set. The result is returned in an integer array.
  As index the enumeration PEM_Proc_Types is used.
  ...[PEM_MCC] is the number of MIC-Clear messages,
  ...[NO_PEM] the number of non PEM messages.
  If ...[0] == -1, no PEM mcc, mco or encr message is found.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_proctypes
 *
 ***************************************************************/
#ifdef __STDC__

int *pem_proctypes(
	SET_OF_PemMessageLocal	 *local
)

#else

int *pem_proctypes(
	local
)
SET_OF_PemMessageLocal	 *local;

#endif

{
	char	*proc = "pem_proctypes";
	int 	*numbers = (int *)calloc(NO_PEM + 1, sizeof(int)),
		 pem = 0;

	if(!numbers) AUX_MALLOC_ERROR((int *)0, ;)

	for(;local; local = local->next) 
		if(local->element)
			if(local->element->header) {
				if(local->element->header->proctype < 0 || local->element->header->proctype >= NO_PEM) {
					aux_add_error(EINVALID, "Invalid Proc-Type number", CNULL, 0, proc);
					free(numbers);
					return((int *)0);
				}
				if(local->element->header->proctype == PEM_ENC ||
				   local->element->header->proctype == PEM_MCC ||
				   local->element->header->proctype == PEM_MCO) pem++;
				numbers[local->element->header->proctype]++;
			}
			else numbers[NO_PEM]++;

	if(!pem) numbers[0] = -1;

	return(numbers);

}

/*------------------------------------------------------------------
  rfc64toOctetString() strips ind chars at the beginning of
  each line, concats the lines and decodes it to an OctetString
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure rfc64toOctetString
 *
 ***************************************************************/
#ifdef __STDC__

static OctetString *rfc64toOctetString(
	char	 *string,
	int	  ind
)

#else

static OctetString *rfc64toOctetString(
	string,
	ind
)
char	 *string;
int	  ind;

#endif

{
	char		*proc = "rfc64toOctetString";
	OctetString 	 tmp_ostr1,
			*tmp_ostr2,
			*res_ostr;
		
	tmp_ostr1.noctets = strlen(string);
	tmp_ostr1.octets = string;

	if(!(tmp_ostr2 = aux_de64(&tmp_ostr1, ind))) AUX_ERROR_RETURN((OctetString *) 0, ;)
	if(!(res_ostr = aux_decrfc(tmp_ostr2))) AUX_ERROR_RETURN((OctetString *) 0, 
								aux_free_OctetString(&tmp_ostr2))
	aux_free_OctetString(&tmp_ostr2);

	return(res_ostr);


}

/*------------------------------------------------------------------
  rfc64toBitString() strips ind chars at the beginning of
  each line, concats the lines and decodes it to a BitString
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure rfc64toBitString
 *
 ***************************************************************/
#ifdef __STDC__

static BitString *rfc64toBitString(
	char	 *string,
	int	  ind
)

#else

static BitString *rfc64toBitString(
	string,
	ind
)
char	 *string;
int	  ind;

#endif

{
	char		*proc = "rfc64toBitString";
	OctetString 	*tmp_ostr = rfc64toOctetString(string, ind);
	BitString 	*res_bstr;
		
	if(!tmp_ostr) AUX_ERROR_RETURN((BitString *)0, ;);

	res_bstr = (BitString *)tmp_ostr;

	res_bstr->nbits *= 8;

	return(res_bstr);


}
/*------------------------------------------------------------------
  OctetStringtorfc64() encodes the OctetString and splits into
  64 character lines with a leading space character
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure OctetStringtorfc64
 *
 ***************************************************************/
#ifdef __STDC__

static char *OctetStringtorfc64(
	OctetString	 *ostr,
	int		  ind
)

#else

static char *OctetStringtorfc64(
	ostr,
	ind
)
OctetString	 *ostr;
int		  ind;

#endif

{
	static OctetString 	 blank_ostr = { 1, " " },
				 empty_ostr = { 0, "" };
	OctetString 		*tmp_ostr2,
				*res_ostr;
	char 			*string,
				*proc = "OctetStringtorfc64";


	if(!(tmp_ostr2 = aux_encrfc(ostr))) AUX_ERROR_RETURN(CNULL, ;)
	if(!(res_ostr = aux_64(tmp_ostr2, ind ? &blank_ostr : &empty_ostr))) AUX_ERROR_RETURN(CNULL, 
								aux_free_OctetString(&tmp_ostr2))
	aux_free_OctetString(&tmp_ostr2);
	if(!(string = CATNSTR(CNULL, res_ostr->octets, res_ostr->noctets))) AUX_ERROR_RETURN(CNULL, 
								aux_free_OctetString(&res_ostr))
	aux_free_OctetString(&res_ostr);

	return(string);


}
/*------------------------------------------------------------------
  BitStringtorfc64() encodes the BitString and splits into
  64 character lines with a leading space character
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure BitStringtorfc64
 *
 ***************************************************************/
#ifdef __STDC__

static char *BitStringtorfc64(
	BitString	 *bstr,
	int		  ind
)

#else

static char *BitStringtorfc64(
	bstr,
	ind
)
BitString	 *bstr;
int		  ind;

#endif

{
	OctetString 	 ostr;
	char 		*string,
			*proc = "BitStringtorfc64";

	ostr.noctets = (bstr->nbits + 7 ) / 8;
	NEWSTRING(ostr.octets, ostr.noctets, CNULL, ;)

	bcopy(bstr->bits, ostr.octets, ostr.noctets);

	if(bstr->nbits%8) ostr.octets[ostr.noctets-1] &= (1 << (bstr->nbits%8)) - 1; 

	if(!(string = OctetStringtorfc64(&ostr, ind))) AUX_ERROR_RETURN(CNULL, 
								free(ostr.octets))
	free(ostr.octets);



	return(string);

}

/*------------------------------------------------------------------
  pem_Local2Canon() converts a message from a local to
  a canonical structure. It uses ASN.1 and RFC 8 to 6 bit
  encoding and transforms enumerations or AlgId's to the
  representing names.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_Local2Canon
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageCanon *pem_Local2Canon(
	PemMessageLocal	 *local
)

#else

PemMessageCanon *pem_Local2Canon(
	local
)
PemMessageLocal	 *local;

#endif

{
	
	PemMessageCanon 		*canon;
	OctetString 			 tmp_ostr1,
					*tmp_ostr2,
					*tmp_ostr3,
					*tmp_body;
	PemHeaderCanon 			*chd;
	PemHeaderLocal 			*lhd;
	SET_OF_PemRecLocal		*lrecips;
	SET_OF_PemRecCanon		*crecips;
	SET_OF_Certificate		*lcerts,
					*cross;
	SET_OF_Name			*ccerts,
					*issuer_certs,
					*crl_issuer_certs,
					*ccrlrrs;
	SET_OF_CRLWithCertificates	*lcrls;
	SET_OF_PemCrlCanon		*ccrls;
	SET_OF_DName			*lcrlrrs;
	FCPath 				*path;
	char				*proc = "pem_Local2Canon";

	if(!local) return((PemMessageCanon *) 0);

	NEWSTRUCT(canon, PemMessageCanon, (PemMessageCanon *) 0, ;)

	lhd = local->header;
	if(lhd) {

		NEWSTRUCT(canon->header, PemHeaderCanon, (PemMessageCanon *) 0, aux_free_PemMessageCanon(&canon))

		chd = canon->header;



		if(lhd->rfc_version) if(!(chd->rfc_version = CATSPRINTF(CNULL, "%d",lhd->rfc_version))) 
						AUX_MALLOC_ERROR((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))

		if(lhd->proctype >= PEM_ENC && lhd->proctype <=PEM_MCC)
			if(!(chd->proctype = aux_cpy_String(proc_type_t[lhd->proctype].name))) 
						AUX_MALLOC_ERROR((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))

		if(lhd->content_domain == PEM_RFC822 || lhd->content_domain == PEM_MIME)
			if(!(chd->content_domain = aux_cpy_String(content_domain[lhd->content_domain].name))) 
						AUX_MALLOC_ERROR((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))

		if(lhd->dek_fields) {
			NEWSTRUCT(chd->dek_fields, PemDekCanon, (PemMessageCanon *) 0, aux_free_PemMessageCanon(&canon))

			if(lhd->dek_fields->dekinfo_enc_alg) {

				if(!(chd->dek_fields->dekinfo_enc_alg = aux_ObjId2Name(lhd->dek_fields->dekinfo_enc_alg->objid))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0, aux_free_PemMessageCanon(&canon))

				if(!(tmp_ostr2 = aux_enchex((OctetString *)lhd->dek_fields->dekinfo_enc_alg->param))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0, aux_free_PemMessageCanon(&canon))

				if(!(chd->dek_fields->dekinfo_param = CATNSTR(CNULL, tmp_ostr2->octets, tmp_ostr2->noctets)))
					AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_OctetString(&tmp_ostr2); aux_free_PemMessageCanon(&canon))

				aux_free_OctetString(&tmp_ostr2);
			}
			if(lhd->dek_fields->keyinfo_enc_alg)
				if(!(chd->dek_fields->keyinfo_enc_alg = aux_ObjId2Name(lhd->dek_fields->keyinfo_enc_alg->objid))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0, aux_free_PemMessageCanon(&canon))

			if(lhd->dek_fields->keyinfo_dek)
				if(!(chd->dek_fields->keyinfo_dek = BitStringtorfc64(lhd->dek_fields->keyinfo_dek, 1))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0, aux_free_PemMessageCanon(&canon))

			for(lrecips = lhd->dek_fields->recipients; lrecips; lrecips = lrecips->next ) {
				if(chd->dek_fields->recipients) {
					NEWSTRUCT(crecips->next, SET_OF_PemRecCanon, (PemMessageCanon *) 0, 
										aux_free_PemMessageCanon(&canon))
					crecips = crecips->next;
				}
				else {
					NEWSTRUCT(crecips, SET_OF_PemRecCanon, (PemMessageCanon *) 0, 
										aux_free_PemMessageCanon(&canon))
					chd->dek_fields->recipients = crecips;
				}
				crecips->next = (SET_OF_PemRecCanon *)0;

				if(lrecips->element) {
					NEWSTRUCT(crecips->element, PemRecCanon, (PemMessageCanon *) 0, 
										aux_free_PemMessageCanon(&canon))

					if(lrecips->element->enc_alg)
						if(!(crecips->element->enc_alg = aux_ObjId2Name(lrecips->element->enc_alg->objid))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))
					if(lrecips->element->dek)
						if(!(crecips->element->dek = BitStringtorfc64(lrecips->element->dek, 1))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))
					if(lrecips->element->certificate && lrecips->element->certificate->tbs) {

						if(!(tmp_ostr2 = e_DName(lrecips->element->certificate->tbs->issuer))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))
						if(!(crecips->element->issuer = OctetStringtorfc64(tmp_ostr2, 1))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_OctetString(&tmp_ostr2);
							aux_free_PemMessageCanon(&canon))
						aux_free_OctetString(&tmp_ostr2);

						if(!(tmp_ostr2 = aux_enchex(lrecips->element->certificate->tbs->serialnumber))) 
									AUX_ERROR_RETURN((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
						if(!(crecips->element->serialnumber = CATNSTR(CNULL, tmp_ostr2->octets, tmp_ostr2->noctets)))
									AUX_ERROR_RETURN((PemMessageCanon *) 0,
									aux_free_OctetString(&tmp_ostr2);
									aux_free_PemMessageCanon(&canon))
						aux_free_OctetString(&tmp_ostr2);
					}
				}

			}




		}
		if(lhd->certificates) {
			if(lhd->certificates->usercertificate && lhd->certificates->usercertificate->tbs) {
				NEWSTRUCT(chd->orig_fields, PemOriginatorCanon, (PemMessageCanon *) 0, 
										aux_free_PemMessageCanon(&canon))

				if(lhd->certificates->usercertificate->tbs->subject) {
					if(!(tmp_ostr2 = e_Certificate(lhd->certificates->usercertificate))) 
						AUX_ERROR_RETURN((PemMessageCanon *) 0,
						aux_free_PemMessageCanon(&canon))
					if(!(chd->orig_fields->certificate = OctetStringtorfc64(tmp_ostr2, 1))) 
						AUX_ERROR_RETURN((PemMessageCanon *) 0,
						aux_free_OctetString(&tmp_ostr2);
						aux_free_PemMessageCanon(&canon))
					aux_free_OctetString(&tmp_ostr2);
				}
				else {
					if(!(tmp_ostr2 = e_DName(lhd->certificates->usercertificate->tbs->issuer))) 
						AUX_ERROR_RETURN((PemMessageCanon *) 0,
						aux_free_PemMessageCanon(&canon))
					if(!(chd->orig_fields->issuer = OctetStringtorfc64(tmp_ostr2, 1))) 
						AUX_ERROR_RETURN((PemMessageCanon *) 0,
						aux_free_OctetString(&tmp_ostr2);
						aux_free_PemMessageCanon(&canon))
					aux_free_OctetString(&tmp_ostr2);

					if(!(tmp_ostr2 = aux_enchex(lhd->certificates->usercertificate->tbs->serialnumber))) 
								AUX_ERROR_RETURN((PemMessageCanon *) 0,
								aux_free_PemMessageCanon(&canon))
					if(!(chd->orig_fields->serialnumber = CATNSTR(CNULL, tmp_ostr2->octets, tmp_ostr2->noctets)))
								AUX_ERROR_RETURN((PemMessageCanon *) 0,
								aux_free_OctetString(&tmp_ostr2);
								aux_free_PemMessageCanon(&canon))
					aux_free_OctetString(&tmp_ostr2);
				}
			}



				
			for(path = lhd->certificates->forwardpath; path; path = path->next_forwardpath)
				for(cross = path->liste; cross; cross = cross->next) 
					if(cross->element) {
						if(chd->issuer_fields) {
							NEWSTRUCT(issuer_certs->next, SET_OF_Name, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))
							issuer_certs = issuer_certs->next;
						}
						else {
							NEWSTRUCT(issuer_certs, SET_OF_Name, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))

							chd->issuer_fields = issuer_certs;
						}
						issuer_certs->next = (SET_OF_Name *)0;

						if(!(tmp_ostr2 = e_Certificate(cross->element))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))
						if(!(issuer_certs->element = OctetStringtorfc64(tmp_ostr2, 1))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_OctetString(&tmp_ostr2);
							aux_free_PemMessageCanon(&canon))
						aux_free_OctetString(&tmp_ostr2);
					}
			if(lhd->root_certificate) {
				if(chd->issuer_fields) {
					NEWSTRUCT(issuer_certs->next, SET_OF_Name, (PemMessageCanon *) 0, 
							aux_free_PemMessageCanon(&canon))
					issuer_certs = issuer_certs->next;
				}
				else {
					NEWSTRUCT(issuer_certs, SET_OF_Name, (PemMessageCanon *) 0, 
							aux_free_PemMessageCanon(&canon))
	
					chd->issuer_fields = issuer_certs;
				}
				issuer_certs->next = (SET_OF_Name *)0;
	
				if(!(tmp_ostr2 = e_Certificate(lhd->root_certificate))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0,
					aux_free_PemMessageCanon(&canon))
				if(!(issuer_certs->element = OctetStringtorfc64(tmp_ostr2, 1))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0,
					aux_free_OctetString(&tmp_ostr2);
					aux_free_PemMessageCanon(&canon))
				aux_free_OctetString(&tmp_ostr2);
			}

		}
		if(lhd->mic_fields) {
			NEWSTRUCT(chd->mic_fields, PemMicCanon, (PemMessageCanon *) 0, 
						aux_free_PemMessageCanon(&canon))


			if(!aux_cmp_AlgId(lhd->mic_fields->signAI, md2WithRsaEncryption)) {
				if(!(chd->mic_fields->mic_alg = aux_cpy_String("RSA-MD2"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
				if(!(chd->mic_fields->micenc_alg = aux_cpy_String("RSA"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
			}
			else if(!aux_cmp_AlgId(lhd->mic_fields->signAI, md4WithRsaEncryption)) {
				if(!(chd->mic_fields->mic_alg = aux_cpy_String("RSA-MD4"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
				if(!(chd->mic_fields->micenc_alg = aux_cpy_String("RSA"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
			}
			else if(!aux_cmp_AlgId(lhd->mic_fields->signAI, md5WithRsaEncryption)) {
				if(!(chd->mic_fields->mic_alg = aux_cpy_String("RSA-MD5"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
				if(!(chd->mic_fields->micenc_alg = aux_cpy_String("RSA"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
			}
			else if(!aux_cmp_AlgId(lhd->mic_fields->signAI, dsaWithSHA)) {
				if(!(chd->mic_fields->mic_alg = aux_cpy_String("SHA"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
				if(!(chd->mic_fields->micenc_alg = aux_cpy_String("NIST-DSA"))) AUX_MALLOC_ERROR((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
			}
			if(lhd->mic_fields->mic) 
				if(!(chd->mic_fields->mic = BitStringtorfc64(lhd->mic_fields->mic, 1))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0,
						aux_free_PemMessageCanon(&canon))
		}
		for(lcrls = lhd->crl_fields; lcrls; lcrls = lcrls->next ) {
			if(chd->crl_fields) {
				NEWSTRUCT(ccrls->next, SET_OF_PemCrlCanon, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))
				ccrls = ccrls->next;
			}
			else {
				NEWSTRUCT(ccrls, SET_OF_PemCrlCanon, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))
				chd->crl_fields = ccrls;
			}
			ccrls->next = (SET_OF_PemCrlCanon *)0;

			if(lcrls->element) {
				NEWSTRUCT(ccrls->element, PemCrlCanon, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))

				if(lcrls->element->crl) {
						if(!(tmp_ostr2 = e_CRL(lcrls->element->crl))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))
						if(!(ccrls->element->crl = OctetStringtorfc64(tmp_ostr2, 1))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_OctetString(&tmp_ostr2);
							aux_free_PemMessageCanon(&canon))
						aux_free_OctetString(&tmp_ostr2);
				}
				if(lcrls->element->certificates) {
					if(lcrls->element->certificates->usercertificate) {
						if(!(tmp_ostr2 = e_Certificate(lcrls->element->certificates->usercertificate))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_PemMessageCanon(&canon))
						if(!(ccrls->element->certificate = OctetStringtorfc64(tmp_ostr2, 1))) 
							AUX_ERROR_RETURN((PemMessageCanon *) 0,
							aux_free_OctetString(&tmp_ostr2);
							aux_free_PemMessageCanon(&canon))
						aux_free_OctetString(&tmp_ostr2);

					}
					for(path = lcrls->element->certificates->forwardpath; path; path = path->next_forwardpath)
						for(cross = path->liste; cross; cross = cross->next) 
							if(cross->element) {
								if(ccrls->element->issuer_certificate) {
									NEWSTRUCT(crl_issuer_certs->next, SET_OF_Name, (PemMessageCanon *) 0, 
											aux_free_PemMessageCanon(&canon))
									crl_issuer_certs = crl_issuer_certs->next;
								}
								else {
									NEWSTRUCT(crl_issuer_certs, SET_OF_Name, (PemMessageCanon *) 0, 
											aux_free_PemMessageCanon(&canon))
		
									ccrls->element->issuer_certificate = crl_issuer_certs;
								}
								crl_issuer_certs->next = (SET_OF_Name *)0;
		
								if(!(tmp_ostr2 = e_Certificate(cross->element))) 
									AUX_ERROR_RETURN((PemMessageCanon *) 0,
									aux_free_PemMessageCanon(&canon))
								if(!(crl_issuer_certs->element = OctetStringtorfc64(tmp_ostr2, 1))) 
									AUX_ERROR_RETURN((PemMessageCanon *) 0,
									aux_free_OctetString(&tmp_ostr2);
									aux_free_PemMessageCanon(&canon))
								aux_free_OctetString(&tmp_ostr2);
							}
				}
			}

		}
		for(lcrlrrs = lhd->crl_rr_fields; lcrlrrs; lcrlrrs = lcrlrrs->next ) {
			if(chd->crl_fields) {
				NEWSTRUCT(ccrlrrs->next, SET_OF_Name, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))
				ccrlrrs = ccrlrrs->next;
			}
			else {
				NEWSTRUCT(ccrlrrs, SET_OF_Name, (PemMessageCanon *) 0, 
									aux_free_PemMessageCanon(&canon))
				chd->crl_rr_fields = ccrlrrs;
			}
			ccrlrrs->next = (SET_OF_Name *)0;

			if(lcrlrrs->element) {
				if(!(tmp_ostr2 = e_DName(lcrlrrs->element))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0,
					aux_free_PemMessageCanon(&canon))
				if(!(ccrlrrs->element = OctetStringtorfc64(tmp_ostr2, 1))) 
					AUX_ERROR_RETURN((PemMessageCanon *) 0,
					aux_free_OctetString(&tmp_ostr2);
					aux_free_PemMessageCanon(&canon))
				aux_free_OctetString(&tmp_ostr2);
			}

		}



	}

	if(local->body) {
		if(lhd && (lhd->proctype == PEM_MCO || lhd->proctype == PEM_ENC)) {
			if(!(canon->body = OctetStringtorfc64(local->body, 0))) AUX_ERROR_RETURN((PemMessageCanon *) 0, 
								aux_free_PemMessageCanon(&canon))
		}
		else {
			if(lhd && lhd->proctype == PEM_MCC) {
				if(!(tmp_body = aux_cpy_OctetString(local->body))) 
							AUX_ERROR_RETURN((PemMessageCanon *)0, aux_free_PemMessageCanon(&canon))

				if(aux_prepend_dash_(tmp_body, 1)) 
							AUX_ERROR_RETURN((PemMessageCanon *)0, aux_free_PemMessageCanon(&canon))

				if(!(canon->body = CATNSTR(CNULL, tmp_body->octets, tmp_body->noctets))) AUX_ERROR_RETURN((PemMessageCanon *) 0, 
								aux_free_PemMessageCanon(&canon))

				aux_free_OctetString(&tmp_body);

			}
			else if(!(canon->body = CATNSTR(CNULL, local->body->octets, local->body->noctets))) AUX_ERROR_RETURN((PemMessageCanon *) 0, 
								aux_free_PemMessageCanon(&canon))
		}
	}
	return(canon);



}
/*------------------------------------------------------------------
  pem_Canon2Local() converts a message from a canonical to
  a local structure. It uses ASN.1 and RFC 8 to 6 bit
  decoding and transforms names to the
  representing AlgId's or elements of enumerations.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_Canon2Local
 *
 ***************************************************************/
#ifdef __STDC__

PemMessageLocal *pem_Canon2Local(
	PemMessageCanon	 *canon
)

#else

PemMessageLocal *pem_Canon2Local(
	canon
)
PemMessageCanon	 *canon;

#endif

{
	PemMessageLocal 		*local;
	OctetString 			 tmp_ostr1,
					*tmp_ostr2,
					*tmp_ostr3;
	PemHeaderLocal 			*lhd;
	PemHeaderCanon 			*chd;
	SET_OF_PemRecLocal		*lrecips;
	SET_OF_PemRecCanon		*crecips;
	SET_OF_Certificate		*lcerts,
					*cross;
	SET_OF_Name			*ccerts,
					*issuer_certs,
					*crl_issuer_certs,
					*ccrlrrs;
	SET_OF_DName			*lcrlrrs;
	SET_OF_CRLWithCertificates	*lcrls;
	SET_OF_PemCrlCanon		*ccrls;
	DName 				*tmp_dname,
					*subj = (DName *)0;
	FCPath 				*path;
	Certificate 			*cert;
	int 				 tmp_int1, n;
	Boolean 			 wrong_hash;
	char				*proc = "pem_Canon2Local";
	
	if(!canon) return((PemMessageLocal *) 0);

	NEWSTRUCT(local, PemMessageLocal, (PemMessageLocal *) 0, ;)

	chd = canon->header;
	if(chd) {
		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "PEM message found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

		NEWSTRUCT(local->header, PemHeaderLocal, (PemMessageLocal *) 0, aux_free_PemMessageLocal(&local))

		lhd = local->header;



		if(chd->rfc_version) lhd->rfc_version = atoi(chd->rfc_version);

		if(chd->proctype) {
			if(pem_verbose_1) 
			if(!(local->comment = CATSPRINTF(local->comment, "PROC-Type: %s\n", chd->proctype))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

			for(tmp_int1 = 0; proc_type_t[tmp_int1].name; tmp_int1++)
				if(!strcmp(proc_type_t[tmp_int1].name, chd->proctype)) break;
			if(!proc_type_t[tmp_int1].name) {
				aux_add_error(EINVALID, "No valid Proc-Type", chd->proctype, char_n, proc);
				aux_free_PemMessageLocal(&local);
				return((PemMessageLocal *) 0);
			}
			lhd->proctype = proc_type_t[tmp_int1].value;
		}

		if(chd->content_domain) {
			if(pem_verbose_1) 
			if(!(local->comment = CATSPRINTF(local->comment, "Content-Domain: %s\n", chd->content_domain))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

			for(tmp_int1 = 0; content_domain[tmp_int1].name; tmp_int1++)
				if(!strcmp(content_domain[tmp_int1].name, chd->content_domain)) break;
			if(!content_domain[tmp_int1].name) {
				aux_add_error(EINVALID, "No valid Content-Domain", chd->content_domain, char_n, proc);
				aux_free_PemMessageLocal(&local);
				return((PemMessageLocal *) 0);
			}
			lhd->content_domain = content_domain[tmp_int1].value;
		}



		if(chd->dek_fields) {
			if(pem_verbose_1) 
			if(!(local->comment = CATSPRINTF(local->comment, "DEK-Info found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

			NEWSTRUCT(lhd->dek_fields, PemDekLocal, (PemMessageLocal *) 0, aux_free_PemMessageLocal(&local))

			if(chd->dek_fields->dekinfo_enc_alg && chd->dek_fields->dekinfo_param) {

				tmp_ostr1.octets = chd->dek_fields->dekinfo_param;
				tmp_ostr1.noctets = strlen(tmp_ostr1.octets);

				NEWSTRUCT(lhd->dek_fields->dekinfo_enc_alg, AlgId, (PemMessageLocal *) 0, aux_free_PemMessageLocal(&local))

				if(!(lhd->dek_fields->dekinfo_enc_alg->objid = aux_Name2ObjId(chd->dek_fields->dekinfo_enc_alg))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
				if(!(lhd->dek_fields->dekinfo_enc_alg->param = (char *)aux_dechex(&tmp_ostr1))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
			}
			if(chd->dek_fields->keyinfo_enc_alg)
				if(!(lhd->dek_fields->keyinfo_enc_alg = aux_Name2AlgId(chd->dek_fields->keyinfo_enc_alg))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
			if(chd->dek_fields->keyinfo_dek)
				if(!(lhd->dek_fields->keyinfo_dek = rfc64toBitString(chd->dek_fields->keyinfo_dek, 1))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))

			if(pem_verbose_1) 
			if(!(local->comment = CATSPRINTF(local->comment, "Originator-Key-Info found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

			for(crecips = chd->dek_fields->recipients, n = 0; crecips; crecips = crecips->next, n++ ) {
				if(lhd->dek_fields->recipients) {
					NEWSTRUCT(lrecips->next, SET_OF_PemRecLocal, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))
					lrecips = lrecips->next;
				}
				else {
					NEWSTRUCT(lrecips, SET_OF_PemRecLocal, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))
					lhd->dek_fields->recipients = lrecips;
				}
				lrecips->next = (SET_OF_PemRecLocal *)0;

				if(crecips->element) {
					NEWSTRUCT(lrecips->element, PemRecLocal, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))

					if(crecips->element->enc_alg)
						if(!(lrecips->element->enc_alg = aux_Name2AlgId(crecips->element->enc_alg))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
					if(crecips->element->dek)
						if(!(lrecips->element->dek = rfc64toBitString(crecips->element->dek, 1))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
					if(crecips->element->issuer && crecips->element->serialnumber) {
						NEWSTRUCT(lrecips->element->certificate, Certificate, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))
						NEWSTRUCT(lrecips->element->certificate->tbs, ToBeSigned, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))

						if(!(tmp_ostr2 = rfc64toOctetString(crecips->element->issuer, 1))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
						if(!(lrecips->element->certificate->tbs->issuer = d_DName(tmp_ostr2))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_OctetString(&tmp_ostr2);
							aux_free_PemMessageLocal(&local))
						aux_free_OctetString(&tmp_ostr2);

						tmp_ostr1.octets = crecips->element->serialnumber;
						tmp_ostr1.noctets = strlen(tmp_ostr1.octets);

						if(!(lrecips->element->certificate->tbs->serialnumber = aux_dechex(&tmp_ostr1))) 
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
	
					}
					
				}

			}

			if(pem_verbose_1 && n) 
			if(!(local->comment = CATSPRINTF(local->comment, "%d recipient keys found\n", n))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)





		}
		if(chd->orig_fields) {
			NEWSTRUCT(lhd->certificates, Certificates, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))

			if(chd->orig_fields->certificate) {
				if(pem_verbose_1) 
				if(!(local->comment = CATSPRINTF(local->comment, "Originator-Certificate found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

				if(!(tmp_ostr2 = rfc64toOctetString(chd->orig_fields->certificate, 1))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_PemMessageLocal(&local))
				if(!(lhd->certificates->usercertificate = d_Certificate(tmp_ostr2))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_OctetString(&tmp_ostr2);
					aux_free_PemMessageLocal(&local))
				aux_free_OctetString(&tmp_ostr2);

			}

			if(chd->orig_fields->issuer && chd->orig_fields->serialnumber) {
				if(pem_verbose_1) 
				if(!(local->comment = CATSPRINTF(local->comment, "Originator-ID-Asymmetric found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

				NEWSTRUCT(lhd->certificates->usercertificate, Certificate, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))
				NEWSTRUCT(lhd->certificates->usercertificate->tbs, ToBeSigned, (PemMessageLocal *) 0, 
										aux_free_PemMessageLocal(&local))
				if(!(tmp_ostr2 = rfc64toOctetString(chd->orig_fields->issuer, 1))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_PemMessageLocal(&local))
				if(!(lhd->certificates->usercertificate->tbs->issuer = d_DName(tmp_ostr2))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_OctetString(&tmp_ostr2);
					aux_free_PemMessageLocal(&local))
				aux_free_OctetString(&tmp_ostr2);

				tmp_ostr1.octets = chd->orig_fields->serialnumber;
				tmp_ostr1.noctets = strlen(tmp_ostr1.octets);

				if(!(lhd->certificates->usercertificate->tbs->serialnumber = aux_dechex(&tmp_ostr1))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_PemMessageLocal(&local))

			}


			if((issuer_certs = chd->issuer_fields))
				NEWSTRUCT(lhd->certificates->forwardpath, FCPath, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
			else lhd->certificates->forwardpath = (FCPath *) 0;
				
			path = lhd->certificates->forwardpath;
			subj = (DName *)0;

			n = 0;
			while(issuer_certs) {
				n++;
				if(issuer_certs->element) {
					if(!(tmp_ostr2 = rfc64toOctetString(issuer_certs->element, 1))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_PemMessageLocal(&local))
					if(!(cert = d_Certificate(tmp_ostr2))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_OctetString(&tmp_ostr2);
						aux_free_PemMessageLocal(&local))
					aux_free_OctetString(&tmp_ostr2);

					if(!aux_cmp_DName(cert->tbs->issuer, cert->tbs->subject)) {
	
						/* This is a prototype certificate which is supposed to be the top-level certificate */
						lhd->root_certificate = cert;
					} 
					else if(!subj || aux_cmp_DName(subj, cert->tbs->subject)) {
						if(subj) {
							NEWSTRUCT(path->next_forwardpath, FCPath, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
							path = path->next_forwardpath;
						}
	
						NEWSTRUCT(cross = path->liste, SET_OF_Certificate, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
						path->liste->element = cert;
						if(!(subj = aux_cpy_DName(cert->tbs->subject)))  
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
							
					} 
					else {	
						NEWSTRUCT(cross->next, CrossCertificates, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
						cross = cross->next;
						cross->element = cert;
					}
				}
				issuer_certs = issuer_certs->next;
			}
			if(!subj) {
				free(lhd->certificates->forwardpath);
				lhd->certificates->forwardpath = (FCPath *) 0;
			}
			aux_free_DName(&subj);

			if(pem_verbose_1 && n) 
			if(!(local->comment = CATSPRINTF(local->comment, "%d issuer certificates found\n", n))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

		}

		if(chd->mic_fields) {
			if(pem_verbose_1) 
			if(!(local->comment = CATSPRINTF(local->comment, "MIC-Info found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

			NEWSTRUCT(lhd->mic_fields, PemMicLocal, (PemMessageLocal *) 0, 
						aux_free_PemMessageLocal(&local))
						
			wrong_hash = FALSE;
			lhd->mic_fields->PEM_conformant = FALSE;

			switch(aux_Name2AlgHash(chd->mic_fields->mic_alg)) {

				case MD2:	
					if(aux_Name2AlgEnc(chd->mic_fields->micenc_alg) == RSA) switch(aux_Name2AlgSpecial(chd->mic_fields->micenc_alg)) {

						case NoAlgSpecial :
							lhd->mic_fields->signAI = aux_cpy_AlgId(md2WithRsa);
							break;

						case PKCS_BT_02:
							lhd->mic_fields->signAI = aux_cpy_AlgId(md2WithRsaEncryption);
							lhd->mic_fields->PEM_conformant = TRUE;
							break;
					}
					break;

				case MD4:	
					if(aux_Name2AlgEnc(chd->mic_fields->micenc_alg) == RSA) switch(aux_Name2AlgSpecial(chd->mic_fields->micenc_alg)) {

						case NoAlgSpecial:
							lhd->mic_fields->signAI = aux_cpy_AlgId(md4WithRsa);
							break;

						case PKCS_BT_02:
							lhd->mic_fields->signAI = aux_cpy_AlgId(md4WithRsaEncryption);
							break;
					}
					break;

				case MD5:	
					if(aux_Name2AlgEnc(chd->mic_fields->micenc_alg) == RSA) switch(aux_Name2AlgSpecial(chd->mic_fields->micenc_alg)) {

						case NoAlgSpecial:
							lhd->mic_fields->signAI = aux_cpy_AlgId(md5WithRsa);
							break;

						case PKCS_BT_02:
							lhd->mic_fields->signAI = aux_cpy_AlgId(md5WithRsaEncryption);
							lhd->mic_fields->PEM_conformant = TRUE;
							break;
					}
					break;
				case SHA:
					if(aux_Name2AlgEnc(chd->mic_fields->micenc_alg) == DSA) lhd->mic_fields->signAI = aux_cpy_AlgId(dsaWithSHA);
					break;
				default:
					wrong_hash = TRUE;
			}
			if(lhd->mic_fields->PEM_conformant) {
				if(strcmp(chd->mic_fields->mic_alg, "RSA-MD2") && strcmp(chd->mic_fields->mic_alg, "RSA-MD5")) 
					lhd->mic_fields->PEM_conformant = FALSE;
			}


			if(!lhd->mic_fields->signAI) {
				if(wrong_hash) {
					aux_add_error(EMSGBUF, "wrong MIC algorithm", (char *) chd->mic_fields->mic_alg, char_n, proc);
				}
				else {
					aux_add_error(EMSGBUF, "wrong MIC-ENC algorithm", (char *) chd->mic_fields->micenc_alg, char_n, proc);
				}
				aux_free_PemMessageLocal(&local);
				return((PemMessageLocal *) 0);
			}

			if(chd->mic_fields->mic) {
				if(!(lhd->mic_fields->mic = rfc64toBitString(chd->mic_fields->mic, 1))) AUX_ERROR_RETURN((PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))
			}
		}
		for(ccrls = chd->crl_fields, n = 0; ccrls; ccrls = ccrls->next, n++ ) {
			if(lhd->crl_fields) {
				NEWSTRUCT(lcrls->next, SET_OF_CRLWithCertificates, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
				lcrls = lcrls->next;
			}
			else {
				NEWSTRUCT(lcrls, SET_OF_CRLWithCertificates, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
				lhd->crl_fields = lcrls;
			}
			lcrls->next = (SET_OF_CRLWithCertificates *)0;

			if(ccrls->element) {
				NEWSTRUCT(lcrls->element, CRLWithCertificates, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))

				if(ccrls->element->crl) {
					if(!(tmp_ostr2 = rfc64toOctetString(ccrls->element->crl, 1))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_PemMessageLocal(&local))
					if(!(lcrls->element->crl = d_CRL(tmp_ostr2))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_OctetString(&tmp_ostr2);
						aux_free_PemMessageLocal(&local))
					aux_free_OctetString(&tmp_ostr2);
				}
				NEWSTRUCT(lcrls->element->certificates, Certificates, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
				if(ccrls->element->certificate) {
					if(!(tmp_ostr2 = rfc64toOctetString(ccrls->element->certificate, 1))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_PemMessageLocal(&local))
					if(!(lcrls->element->certificates->usercertificate = d_Certificate(tmp_ostr2))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_OctetString(&tmp_ostr2);
						aux_free_PemMessageLocal(&local))
					aux_free_OctetString(&tmp_ostr2);

				}
				if((crl_issuer_certs = ccrls->element->issuer_certificate))
					NEWSTRUCT(lcrls->element->certificates->forwardpath, FCPath, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
				else lcrls->element->certificates->forwardpath = (FCPath *) 0;

				path = lcrls->element->certificates->forwardpath;
				subj = (DName *)0;
				n = 0;
				while(crl_issuer_certs) {
					n++;
					if(!(tmp_ostr2 = rfc64toOctetString(crl_issuer_certs->element, 1))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_PemMessageLocal(&local))
					if(!(cert = d_Certificate(tmp_ostr2))) 
						AUX_ERROR_RETURN((PemMessageLocal *) 0,
						aux_free_OctetString(&tmp_ostr2);
						aux_free_PemMessageLocal(&local))
					aux_free_OctetString(&tmp_ostr2);

					if(!subj || aux_cmp_DName(subj, cert->tbs->subject)) {
						if(subj) {
							NEWSTRUCT(path->next_forwardpath, FCPath, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
							path = path->next_forwardpath;
						}
	
						NEWSTRUCT(cross = path->liste, SET_OF_Certificate, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))

						path->liste->element = cert;
						if(!(subj = aux_cpy_DName(cert->tbs->subject)))  
							AUX_ERROR_RETURN((PemMessageLocal *) 0,
							aux_free_PemMessageLocal(&local))
							
					} 
					else {	
						NEWSTRUCT(cross->next, CrossCertificates, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
						cross = cross->next;
						cross->element = cert;
					}
					crl_issuer_certs = crl_issuer_certs->next;
				}
				if(!subj) {
					free(lcrls->element->certificates->forwardpath);
					lcrls->element->certificates->forwardpath = (FCPath *) 0;
				}
				aux_free_DName(&subj);

				if(pem_verbose_1 && n) 
				if(!(local->comment = CATSPRINTF(local->comment, "%d certificates to CRL found\n", n))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

				
			}

		}
		if(pem_verbose_1 && n) 
		if(!(local->comment = CATSPRINTF(local->comment, "%d CRLs found\n", n))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

		for(ccrlrrs = chd->crl_rr_fields, n = 0; ccrlrrs; ccrlrrs = ccrlrrs->next, n++ ) {
			if(lhd->crl_rr_fields) {
				NEWSTRUCT(lcrlrrs->next, SET_OF_DName, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
				lcrlrrs = lcrlrrs->next;
			}
			else {
				NEWSTRUCT(lcrlrrs, SET_OF_DName, (PemMessageLocal *) 0, 
									aux_free_PemMessageLocal(&local))
				lhd->crl_rr_fields = lcrlrrs;
			}
			lcrlrrs->next = (SET_OF_DName *)0;

			if(ccrlrrs->element) {
				if(!(tmp_ostr2 = rfc64toOctetString(ccrlrrs->element, 1))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_PemMessageLocal(&local))
				if(!(lcrlrrs->element = d_DName(tmp_ostr2))) 
					AUX_ERROR_RETURN((PemMessageLocal *) 0,
					aux_free_OctetString(&tmp_ostr2);
					aux_free_PemMessageLocal(&local))
				aux_free_OctetString(&tmp_ostr2);
			}

		}
		if(pem_verbose_1 && n) 
		if(!(local->comment = CATSPRINTF(local->comment, "%d CRL-RRs found\n", n))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)




	}
	else {
		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "Clear text found\n"))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

	}
	if(canon->body) {
		if(canon->header && (lhd->proctype == PEM_MCO || lhd->proctype == PEM_ENC)) {
			if(!(local->body = rfc64toOctetString(canon->body, 0))) AUX_ERROR_RETURN((PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))
		}
		else {
			if(!(local->body = aux_create_OctetString(canon->body))) AUX_ERROR_RETURN((PemMessageLocal *) 0, 
								aux_free_PemMessageLocal(&local))

			if(canon->header && lhd->proctype == PEM_MCC) 	if (aux_strip_dash_(local->body, 1) < 0)
				AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local))

		}
		if(pem_verbose_1) 
		if(!(local->comment = CATSPRINTF(local->comment, "%d octets body found\n", local->body->noctets))) AUX_ERROR_RETURN((PemMessageLocal *)0, aux_free_PemMessageLocal(&local);)

	}
	return(local);
}
/*------------------------------------------------------------------
  pem_CanonSet2LocalSet() converts each element of a message
  set from a canonical to
  a local structure. It uses ASN.1 and RFC 8 to 6 bit
  decoding and transforms names to the
  representing AlgId's or elements of enumerations.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_CanonSet2LocalSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_PemMessageLocal *pem_CanonSet2LocalSet(
	SET_OF_PemMessageCanon	 *canon
)

#else

SET_OF_PemMessageLocal *pem_CanonSet2LocalSet(
	canon
)
SET_OF_PemMessageCanon	 *canon;

#endif

{
	SET_OF_PemMessageLocal 	*local = (SET_OF_PemMessageLocal *)0,
				*local2;
	char			*proc = "pem_CanonSet2LocalSet";
	
	while(canon) {
		if(local) {
			NEWSTRUCT(local2->next, SET_OF_PemMessageLocal, (SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&local))
			local2 = local2->next;
		}
		else {
			NEWSTRUCT(local2, SET_OF_PemMessageLocal, (SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&local))
			local = local2;
		}
		local2->next = (SET_OF_PemMessageLocal *)0;
		local2->element = pem_Canon2Local(canon->element);
		if(canon->element && !local2->element) AUX_ERROR_RETURN((SET_OF_PemMessageLocal *) 0, 
										aux_free_SET_OF_PemMessageLocal(&local))
		canon = canon->next;
	}
	return(local);
	
}
/*------------------------------------------------------------------
  pem_Local2Canon() converts each element of a
  message set from a local to
  a canonical structure. It uses ASN.1 and RFC 8 to 6 bit
  encoding and transforms enumerations or AlgId's to the
  representing names.
------------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure pem_LocalSet2CanonSet
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_PemMessageCanon *pem_LocalSet2CanonSet(
	SET_OF_PemMessageLocal	 *local
)

#else

SET_OF_PemMessageCanon *pem_LocalSet2CanonSet(
	local
)
SET_OF_PemMessageLocal	 *local;

#endif

{
	SET_OF_PemMessageCanon 	*canon = (SET_OF_PemMessageCanon *)0,
				*canon2;
	char			*proc = "pem_LocalSet2CanonSet";
	
	while(local) {
		if(canon) {
			NEWSTRUCT(canon2->next, SET_OF_PemMessageCanon, (SET_OF_PemMessageCanon *) 0, 
										aux_free_SET_OF_PemMessageCanon(&canon))
			canon2 = canon2->next;
		}
		else {
			NEWSTRUCT(canon2, SET_OF_PemMessageCanon, (SET_OF_PemMessageCanon *) 0, 
										aux_free_SET_OF_PemMessageCanon(&canon))
			canon = canon2;
		}
		canon2->next = (SET_OF_PemMessageCanon *)0;
		canon2->element = pem_Local2Canon(local->element);
		if(local->element && !canon2->element) AUX_ERROR_RETURN((SET_OF_PemMessageCanon *) 0, 
										aux_free_SET_OF_PemMessageCanon(&canon))
		local = local->next;
	}
	return(canon);
	
}
