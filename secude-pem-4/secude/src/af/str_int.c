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
/*------------------------------------------------------------------*/
/*-----------Functions for Strong Authentication support------------*/

/* GMD Darmstadt Institute for System Technic (F2.G3)               */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``PASSWORD'' 1993                                        */
/*------------------------------------------------------------------*/


#if defined(X500) && defined(STRONG)

#include "psap.h"
#include "osisec-stub.h"
#include "secude-stub.h"
#include "cadb.h"   /* definition of DEF_ISSUER_ALGID */

#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include "aux_time.h"

#include "AF-types.h"
#include "common.h"
#include "DAS-types.h"  /* for definition of encode_DAS_PartialOutcomeQualifier() */
#include "attrvalue.h"
#include "authen.h"

#include "af_quipuif.h"

extern PE		    AlgId_enc();
extern AlgId              * AlgId_dec();
extern PE 		    cert_enc();
extern struct certificate * cert_dec();
extern Certificate        * certificate_dec();
extern PE 		    dn_enc ();
extern DN 	            dn_dec();
extern PE	            pe_cpy ();	   /* from isode/psap.h */
extern void  		    aux_xdump2();
extern PE 	            grab_pe();
extern UTCTime  	  * get_date_of_expiry();


static struct certificate_list * certlist = (struct certificate_list * )0;


/******************************/

/*static struct SecurityServices serv_secude = SECUDESERVICES;*/
struct SecurityServices serv_secude = SECUDESERVICES;





#ifdef __STDC__
	
	static        Signature	*aux_QUIPUsign2SECUDEsign	(struct signature *quipusign);
	static        struct signature	*aux_SECUDEsign2QUIPUsign	(Signature *secudesign);
	static        struct certificate_list	*aux_SECUDEocert2QUIPUcertlist	(Certificates *certs);
	static        struct certificate_list	*aux_SECUDEfcpath2QUIPUcertlist	(FCPath *fcpath);
	static        Certificates	*aux_QUIPUcertlist2SECUDEocert	(struct certificate_list *certlist);
	static        FCPath	*aux_QUIPUcertlist2SECUDEfcpath	(struct certificate_list *certlist);
	static        CertificationPath	*aux_QUIPUcertlist2SECUDEcertpath	(struct certificate_list *certlist);
	static        CertificatePairs	*aux_QUIPUcertlist2SECUDEcertificatepairs	(struct certificate_list *certlist);
	static        ObjId	*aux_QUIPUAttributeType2SECUDEObjId	(oid_table_attr *parm);
	static        SET_OF_Attr	*aux_QUIPUAttrSequence2SECUDESETOFAttr	(Attr_Sequence parm);
	static        Attr	*aux_QUIPUAttrSequence2SECUDEAttr	(Attr_Sequence parm);
	static        AttrValueAssertion	*aux_QUIPUAVA2SECUDEAttrValAssert	(AVA *parm);
	static        SET_OF_ObjId	*aux_QUIPUAttrSequence2SECUDESETOFObjId	(Attr_Sequence parm);
	static        ObjId	*aux_QUIPUAttrSequence2SECUDEObjId	(Attr_Sequence parm);
	static        BitString	*aux_cpy_random	(struct random_number *random);
	static        TokenTBS	*aux_extract_TokenTBS_from_BindArg	(struct ds_bind_arg *ds_bindarg);
	static        AddArgumentTBS	*aux_extract_AddArgumentTBS_from_AddArg	(struct ds_addentry_arg *ds_addarg);
	static        CompareArgumentTBS	*aux_extract_CompareArgumentTBS_from_CompareArg	(struct ds_compare_arg *ds_comparearg);
	static        CompareResultTBS	*aux_extract_CompareResultTBS_from_CompareRes	(struct ds_compare_result *ds_compareres);
	static        ListArgumentTBS	*aux_extract_ListArgumentTBS_from_ListArg	(struct ds_list_arg *ds_listarg);
	static        ListResultTBS	*aux_extract_ListResultTBS_from_ListRes	(struct ds_list_result *ds_listres);
	static        ModifyEntryArgumentTBS	*aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg	(struct ds_modifyentry_arg *ds_modifyentryarg);
	static        PE	rdn_enc	(RDN rdn);
	static        ModifyRDNArgumentTBS	*aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg	(struct ds_modifyrdn_arg *ds_modifyrdnarg);
	static        ReadArgumentTBS	*aux_extract_ReadArgumentTBS_from_ReadArg	(struct ds_read_arg *ds_readarg);
	static        ReadResultTBS	*aux_extract_ReadResultTBS_from_ReadRes	(struct ds_read_result *ds_readres);
	static        RemoveArgumentTBS	*aux_extract_RemoveArgumentTBS_from_RemoveArg	(struct ds_removeentry_arg *ds_removearg);
	static        SearchArgumentTBS	*aux_extract_SearchArgumentTBS_from_SearchArg	(struct ds_search_arg *ds_searcharg);
	static        SearchResultTBS	*aux_extract_SearchResultTBS_from_SearchRes	(struct ds_search_result *ds_searchres);
	static        SFilter	*aux_cpy_SFilter	(Filter QUIPUfilter);
	static        SET_OF_SFilter	*aux_cpy_SET_OF_SFilter	(Filter QUIPUfilter);
	static        SEQUENCE_OF_StringsCHOICE	*aux_cpy_SEQUENCE_OF_StringsCHOICE	(Filter_Substrings *QUIPUfilsubstrgs);
	static        SFilterSubstrings	*aux_cpy_SFilterSubstrings	(Filter_Substrings *QUIPUfilsubstrgs);
	static        SFilterItem	*aux_cpy_SFilterItem	(struct filter_item *QUIPUitem);
	static        CommonArguments	*aux_cpy_CommonArguments	(CommonArgs *QUIPUca);
	static        SecurityParameters	*aux_cpy_SecurityParameters	(struct security_parms *QUIPUsp);
	static        CommonRes	*aux_cpy_CommonRes	(CommonResults *QUIPUcr);
	static        SubordEntry	*aux_cpy_SubordEntry	(struct subordinate *QUIPUsubordinates);
	static        SET_OF_SubordEntry	*aux_cpy_SET_OF_SubordEntry	(struct subordinate *QUIPUsubordinates);
	static        EntryModification	*aux_cpy_EntryModification	(struct entrymod *QUIPUem);
	static        SEQUENCE_OF_EntryModification	*aux_cpy_SEQUENCE_OF_EntryModification	(struct entrymod *parm);
	static        ListInfo	*aux_cpy_ListInfo	(struct ds_list_result *QUIPUlsr);
	static        SearchInfo	*aux_cpy_SearchInfo	(struct ds_search_result *QUIPUsrr);
	static        EntryInfoSEL	*aux_cpy_EntryInfoSelection	(EntryInfoSelection *QUIPUeis);
	static        EntryINFO	*aux_cpy_EntryINFO	(EntryInfo *QUIPUei);
	static        SET_OF_EntryINFO	*aux_cpy_SET_OF_EntryINFO	(EntryInfo *QUIPUei);
	static        AttrAttrTypeCHOICE	*aux_cpy_AttrAttrTypeCHOICE	(Attr_Sequence parm);
	static        SET_OF_AttrAttrTypeCHOICE	*aux_cpy_SET_OF_AttrAttrTypeCHOICE	(Attr_Sequence parm);
	static        SET_OF_DName	*aux_QUIPUdnseq2SECUDESETOFDName	(struct dn_seq *parm);
	static        SET_OF_ObjId	*aux_QUIPUoidseq2SECUDEsetofobjid	(struct oid_seq *parm);
	static        AccessPoint	*aux_QUIPUaccpoint2SECUDEaccpoint	(struct access_point *parm);
	static        SET_OF_AccessPoint	*aux_QUIPUaccpoint2SECUDESETOFaccpoint	(struct access_point *parm);
	static        OperationProgress	*aux_QUIPUop2SECUDEop	(struct op_progress *parm);
	static        ContReference	*aux_QUIPUcref2SECUDEcref	(ContinuationRef parm);
	static        SET_OF_ContReference	*aux_QUIPUcref2SECUDESETOFcref	(ContinuationRef parm);
	static        PartialOutQual	*aux_QUIPUpoq2SECUDEpoq	(POQ *parm);

#else

	static        Signature	*aux_QUIPUsign2SECUDEsign	();
	static        struct signature	*aux_SECUDEsign2QUIPUsign	();
	static        struct certificate_list	*aux_SECUDEocert2QUIPUcertlist	();
	static        struct certificate_list	*aux_SECUDEfcpath2QUIPUcertlist	();
	static        Certificates	*aux_QUIPUcertlist2SECUDEocert	();
	static        FCPath	*aux_QUIPUcertlist2SECUDEfcpath	();
	static        CertificationPath	*aux_QUIPUcertlist2SECUDEcertpath	();
	static        CertificatePairs	*aux_QUIPUcertlist2SECUDEcertificatepairs	();
	static        ObjId	*aux_QUIPUAttributeType2SECUDEObjId	();
	static        SET_OF_Attr	*aux_QUIPUAttrSequence2SECUDESETOFAttr	();
	static        Attr	*aux_QUIPUAttrSequence2SECUDEAttr	();
	static        AttrValueAssertion	*aux_QUIPUAVA2SECUDEAttrValAssert	();
	static        SET_OF_ObjId	*aux_QUIPUAttrSequence2SECUDESETOFObjId	();
	static        ObjId	*aux_QUIPUAttrSequence2SECUDEObjId	();
	static        BitString	*aux_cpy_random	();
	static        TokenTBS	*aux_extract_TokenTBS_from_BindArg	();
	static        AddArgumentTBS	*aux_extract_AddArgumentTBS_from_AddArg	();
	static        CompareArgumentTBS	*aux_extract_CompareArgumentTBS_from_CompareArg	();
	static        CompareResultTBS	*aux_extract_CompareResultTBS_from_CompareRes	();
	static        ListArgumentTBS	*aux_extract_ListArgumentTBS_from_ListArg	();
	static        ListResultTBS	*aux_extract_ListResultTBS_from_ListRes	();
	static        ModifyEntryArgumentTBS	*aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg	();
	static        PE	rdn_enc	();
	static        ModifyRDNArgumentTBS	*aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg	();
	static        ReadArgumentTBS	*aux_extract_ReadArgumentTBS_from_ReadArg	();
	static        ReadResultTBS	*aux_extract_ReadResultTBS_from_ReadRes	();
	static        RemoveArgumentTBS	*aux_extract_RemoveArgumentTBS_from_RemoveArg	();
	static        SearchArgumentTBS	*aux_extract_SearchArgumentTBS_from_SearchArg	();
	static        SearchResultTBS	*aux_extract_SearchResultTBS_from_SearchRes	();
	static        SFilter	*aux_cpy_SFilter	();
	static        SET_OF_SFilter	*aux_cpy_SET_OF_SFilter	();
	static        SEQUENCE_OF_StringsCHOICE	*aux_cpy_SEQUENCE_OF_StringsCHOICE	();
	static        SFilterSubstrings	*aux_cpy_SFilterSubstrings	();
	static        SFilterItem	*aux_cpy_SFilterItem	();
	static        CommonArguments	*aux_cpy_CommonArguments	();
	static        SecurityParameters	*aux_cpy_SecurityParameters	();
	static        CommonRes	*aux_cpy_CommonRes	();
	static        SubordEntry	*aux_cpy_SubordEntry	();
	static        SET_OF_SubordEntry	*aux_cpy_SET_OF_SubordEntry	();
	static        EntryModification	*aux_cpy_EntryModification	();
	static        SEQUENCE_OF_EntryModification	*aux_cpy_SEQUENCE_OF_EntryModification	();
	static        ListInfo	*aux_cpy_ListInfo	();
	static        SearchInfo	*aux_cpy_SearchInfo	();
	static        EntryInfoSEL	*aux_cpy_EntryInfoSelection	();
	static        EntryINFO	*aux_cpy_EntryINFO	();
	static        SET_OF_EntryINFO	*aux_cpy_SET_OF_EntryINFO	();
	static        AttrAttrTypeCHOICE	*aux_cpy_AttrAttrTypeCHOICE	();
	static        SET_OF_AttrAttrTypeCHOICE	*aux_cpy_SET_OF_AttrAttrTypeCHOICE	();
	static        SET_OF_DName	*aux_QUIPUdnseq2SECUDESETOFDName	();
	static        SET_OF_ObjId	*aux_QUIPUoidseq2SECUDEsetofobjid	();
	static        AccessPoint	*aux_QUIPUaccpoint2SECUDEaccpoint	();
	static        SET_OF_AccessPoint	*aux_QUIPUaccpoint2SECUDESETOFaccpoint	();
	static        OperationProgress	*aux_QUIPUop2SECUDEop	();
	static        ContReference	*aux_QUIPUcref2SECUDEcref	();
	static        SET_OF_ContReference	*aux_QUIPUcref2SECUDESETOFcref	();
	static        PartialOutQual	*aux_QUIPUpoq2SECUDEpoq	();

#endif






















/***************************************************************
 *
 * Procedure aux_QUIPUsign2SECUDEsign
 *
 ***************************************************************/
#ifdef __STDC__

static Signature *aux_QUIPUsign2SECUDEsign(
	struct signature	 *quipusign
)

#else

static Signature *aux_QUIPUsign2SECUDEsign(
	quipusign
)
struct signature	 *quipusign;

#endif

{

	Signature        * sig;
	PE 	           pe;
	int	           i, nob;
	char	         * proc = "aux_QUIPUsign2SECUDEsign";

	if(! quipusign){
		aux_add_error(EINVALID, "No parameter provided (quipusign)", CNULL, 0, proc);
		return((Signature * )0);
	}

/*
	quipusign->alg.asn = NULLPE;
*/

	sig = (Signature * )malloc(sizeof(Signature));

	encode_AF_AlgorithmIdentifier(&pe, 0, 0, NULLCP, &(quipusign->alg));
	sig->signAI = AlgId_dec(pe);
	if(pe) pe_free (pe);
	if(! sig->signAI){
		aux_add_error(EDECODE, "AlgId_dec failed", CNULL, 0, proc);
		return((Signature * )0);
	}

	sig->signature.nbits = quipusign->n_bits;
	nob = sig->signature.nbits / 8;
	if(sig->signature.nbits % 8 )
		nob++;
	sig->signature.bits = (char * )malloc(nob);

	for ( i = 0; i < nob; i++) {
		sig->signature.bits[i] = quipusign->encrypted[i];
	}

	return(sig);
 
}


/***************************************************************
 *
 * Procedure aux_SECUDEsign2QUIPUsign
 *
 ***************************************************************/
#ifdef __STDC__

static struct signature *aux_SECUDEsign2QUIPUsign(
	Signature	 *secudesign
)

#else

static struct signature *aux_SECUDEsign2QUIPUsign(
	secudesign
)
Signature	 *secudesign;

#endif

{
	
	struct signature          * quipusign;
	PE	                    pe;
	struct alg_id             * quipu_alg;
	int		            result, i, nob;
	char	                  * proc = "aux_SECUDEsign2QUIPUsign";

	if(! secudesign){
		aux_add_error(EINVALID, "No parameter provided (secudesign)", CNULL, 0, proc);
		return((struct signature * )0);
	}

	quipusign = (struct signature * )malloc(sizeof(struct signature));

	pe = AlgId_enc(secudesign->signAI);
	result = decode_AF_AlgorithmIdentifier (pe, 0, NULLIP, NULLVP, &quipu_alg);
	pe_free(pe);
	if(result == NOTOK) {
		aux_add_error(EDECODE, "ret", CNULL, 0, proc);
		return((struct signature * )0);
	}
	alg_cpy(&(quipusign->alg), quipu_alg);

	quipusign->n_bits = secudesign->signature.nbits;
	nob = quipusign->n_bits / 8;
	if(quipusign->n_bits % 8 )
		nob++;
	quipusign->encrypted = (char * )malloc(nob);

	for ( i = 0; i < nob; i++)
		quipusign->encrypted[i] = secudesign->signature.bits[i];

	quipusign->encoded = NULLPE;

	return(quipusign);

}



/***************************************************************
 *
 * Procedure aux_SECUDEocert2QUIPUcertlist
 *
 ***************************************************************/
#ifdef __STDC__

static struct certificate_list *aux_SECUDEocert2QUIPUcertlist(
	Certificates	 *certs
)

#else

static struct certificate_list *aux_SECUDEocert2QUIPUcertlist(
	certs
)
Certificates	 *certs;

#endif

{

	struct certificate_list        * ret;
	SET_OF_Certificate             * certset;
	char	                       * proc = "aux_SECUDEocert2QUIPUcertlist";

	if(! certs){
		aux_add_error(EINVALID, "No parameter provided (certs)", CNULL, 0, proc);
		return((struct certificate_list * )0);
	}

	ret = (struct certificate_list * )malloc(sizeof(struct certificate_list));

	ret->cert = secudeCert2quipuCert(certs->usercertificate);
	if(! ret->cert){
		aux_add_error(EINVALID, "Cannot transform SecuDE-certificate representation into Quipu representation", CNULL, 0, proc);
		return((struct certificate_list * )0);
	}

	ret->reverse = (struct certificate *)0;
	ret->next = ret->prev = (struct certificate_list *)0;
	ret->superior = aux_SECUDEfcpath2QUIPUcertlist(certs->forwardpath);

	return(ret);

}



/***************************************************************
 *
 * Procedure aux_SECUDEfcpath2QUIPUcertlist
 *
 ***************************************************************/
#ifdef __STDC__

static struct certificate_list *aux_SECUDEfcpath2QUIPUcertlist(
	FCPath	 *fcpath
)

#else

static struct certificate_list *aux_SECUDEfcpath2QUIPUcertlist(
	fcpath
)
FCPath	 *fcpath;

#endif

{

	struct certificate_list        * ret, * same_level;
	SET_OF_Certificate             * certset;
	char	                       * proc = "aux_SECUDEfcpath2QUIPUcertlist";

	if(! fcpath){
		aux_add_error(EINVALID, "No parameter provided (fcpath)", CNULL, 0, proc);
		return((struct certificate_list * )0);
	}

	ret = (struct certificate_list * )malloc(sizeof(struct certificate_list));

	certset = fcpath->liste;

	ret->cert = secudeCert2quipuCert(certset->element);
	if(! ret->cert){
		aux_add_error(EINVALID, "Cannot transform SecuDE-certificate representation into Quipu representation", CNULL, 0, proc);
		return((struct certificate_list * )0);
	}

	ret->reverse = (struct certificate *)0;
	ret->next = ret->prev = (struct certificate_list *)0;

	same_level = ret;
	certset = certset->next;

	while (certset) {
		same_level->next = (struct certificate_list * )malloc(sizeof(struct certificate_list));
			
		same_level = same_level->next;

		same_level->cert = secudeCert2quipuCert(certset->element);
		if(! same_level->cert){
			aux_add_error(EINVALID, "Cannot transform SecuDE-certificate representation into Quipu representation", CNULL, 0, proc);
			return((struct certificate_list * )0);
		}

		same_level->reverse = (struct certificate *)0;
		same_level->next = same_level->prev = same_level->superior = (struct certificate_list *)0;

		certset = certset->next;
	} /*while*/

	ret->superior = aux_SECUDEfcpath2QUIPUcertlist(fcpath->next_forwardpath);

	return(ret);

}



/***************************************************************
 *
 * Procedure aux_QUIPUcertlist2SECUDEocert
 *
 ***************************************************************/
#ifdef __STDC__

static Certificates *aux_QUIPUcertlist2SECUDEocert(
	struct certificate_list	 *certlist
)

#else

static Certificates *aux_QUIPUcertlist2SECUDEocert(
	certlist
)
struct certificate_list	 *certlist;

#endif

{
	Certificates        * ret;
	char	            * proc = "aux_QUIPUcertlist2SECUDEocert";

	if(! certlist){
		aux_add_error(EINVALID, "No parameter provided (certlist)", CNULL, 0, proc);
		return((Certificates * )0);
	}

	if((ret = (Certificates * )malloc(sizeof(Certificates))) == (Certificates * )0) 
		return((Certificates * )0);

	ret->usercertificate = quipuCert2secudeCert(certlist->cert);
	if(! ret->usercertificate){
		aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
		return((Certificates * )0);
	}

	ret->forwardpath = aux_QUIPUcertlist2SECUDEfcpath(certlist->superior);

	return(ret);
}



/***************************************************************
 *
 * Procedure aux_QUIPUcertlist2SECUDEfcpath
 *
 ***************************************************************/
#ifdef __STDC__

static FCPath *aux_QUIPUcertlist2SECUDEfcpath(
	struct certificate_list	 *certlist
)

#else

static FCPath *aux_QUIPUcertlist2SECUDEfcpath(
	certlist
)
struct certificate_list	 *certlist;

#endif

{
	FCPath                     * ret;
	SET_OF_Certificate         * tmp_certset;
	struct certificate_list    * same_level;
	char                       * proc = "aux_QUIPUcertlist2SECUDEfcpath";

	if(! certlist){
		aux_add_error(EINVALID, "No parameter provided (certlist)", CNULL, 0, proc);
		return((FCPath * )0);
	}

	if((ret = (FCPath * )malloc(sizeof(FCPath))) == (FCPath * )0) {
		return((FCPath * )0);
	}

	if((ret->liste = (SET_OF_Certificate * )
				malloc(sizeof(SET_OF_Certificate))) == (SET_OF_Certificate * )0) {
		return((FCPath * )0);
	}
	
	tmp_certset = ret->liste;

	ret->liste->element = quipuCert2secudeCert(certlist->cert);
	if(! ret->liste->element){
		aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
		return((FCPath * )0);
	}

	ret->liste->next = (SET_OF_Certificate * )0;

	same_level = certlist->next;

	while (same_level) {
		if((tmp_certset->next = (SET_OF_Certificate * )
				malloc(sizeof(SET_OF_Certificate))) == (SET_OF_Certificate * )0) {
			return((FCPath * )0);
		}
		tmp_certset = tmp_certset->next;

		tmp_certset->element = quipuCert2secudeCert(same_level->cert);
		if(! tmp_certset->element){
			aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
			return((FCPath * )0);
		}

		tmp_certset->next = (SET_OF_Certificate *)0;
		same_level = same_level->next;
	}
	
	ret->next_forwardpath = aux_QUIPUcertlist2SECUDEfcpath(certlist->superior);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUcertlist2SECUDEcertpath
 *
 ***************************************************************/
#ifdef __STDC__

static CertificationPath *aux_QUIPUcertlist2SECUDEcertpath(
	struct certificate_list	 *certlist
)

#else

static CertificationPath *aux_QUIPUcertlist2SECUDEcertpath(
	certlist
)
struct certificate_list	 *certlist;

#endif

{
	CertificationPath        * ret;
	char	                 * proc = "aux_QUIPUcertlist2SECUDEcertpath";

	if(! certlist){
		aux_add_error(EINVALID, "No parameter provided (certlist)", CNULL, 0, proc);
		return((CertificationPath * )0);
	}

	if((ret = (CertificationPath * )
				malloc(sizeof(CertificationPath))) == (CertificationPath * )0) 
		return((CertificationPath * )0);

	ret->userCertificate = quipuCert2secudeCert(certlist->cert);
	if(! ret->userCertificate){
		aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
		return((CertificationPath * )0);
	}

	ret->theCACertificates = aux_QUIPUcertlist2SECUDEcertificatepairs(certlist->superior);

	return(ret);
}



/***************************************************************
 *
 * Procedure aux_QUIPUcertlist2SECUDEcertificatepairs
 *
 ***************************************************************/
#ifdef __STDC__

static CertificatePairs *aux_QUIPUcertlist2SECUDEcertificatepairs(
	struct certificate_list	 *certlist
)

#else

static CertificatePairs *aux_QUIPUcertlist2SECUDEcertificatepairs(
	certlist
)
struct certificate_list	 *certlist;

#endif

{
	CertificatePairs               * ret;
	SEQUENCE_OF_CertificatePair    * tmp;
	struct certificate_list        * same_level;
	char                           * proc = "aux_QUIPUcertlist2SECUDEcertificatepairs";

	if(! certlist){
		aux_add_error(EINVALID, "No parameter provided (certlist)", CNULL, 0, proc);
		return((CertificatePairs * )0);
	}

	if((ret = (CertificatePairs * ) malloc(sizeof(CertificatePairs))) == (CertificatePairs * )0) {
		return((CertificatePairs * )0);
	}

	if((ret->liste = (SEQUENCE_OF_CertificatePair * )
			malloc(sizeof(SEQUENCE_OF_CertificatePair))) == (SEQUENCE_OF_CertificatePair * )0) {
		return((CertificatePairs * )0);
	}

	tmp = ret->liste;

	if((tmp->element = (CertificatePair * )
				malloc(sizeof(CertificatePair))) == (CertificatePair * )0) {
		return((CertificatePairs * )0);
	}

	tmp->element->forward = (Certificate *)0;
	tmp->element->reverse = (Certificate *)0;
	tmp->next = (SEQUENCE_OF_CertificatePair *)0;

	if(certlist->cert){
		tmp->element->forward = quipuCert2secudeCert(certlist->cert);
		if(! tmp->element->forward){
			aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
			return((CertificatePairs * )0);
		}
	}

	if(certlist->reverse){
		tmp->element->reverse = quipuCert2secudeCert(certlist->reverse);
		if(! tmp->element->reverse){
			aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
			return((CertificatePairs * )0);
		}
	}

	same_level = certlist->next;

	while (same_level) {
		if((tmp->next = (SEQUENCE_OF_CertificatePair * )
				malloc(sizeof(SEQUENCE_OF_CertificatePair))) == (SEQUENCE_OF_CertificatePair * )0) {
			return((CertificatePairs * )0);
		}

		tmp = tmp->next;

		if((tmp->element = (CertificatePair * )
					malloc(sizeof(CertificatePair))) == (CertificatePair * )0) {
			return((CertificatePairs * )0);
		}

		tmp->element->forward = (Certificate *)0;
		tmp->element->reverse = (Certificate *)0;
		tmp->next = (SEQUENCE_OF_CertificatePair *)0;

		if(same_level->cert){
			tmp->element->forward = quipuCert2secudeCert(same_level->cert);
			if(! tmp->element->forward){
				aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
				return((CertificatePairs * )0);
			}
		}

		if(same_level->reverse){
			tmp->element->reverse = quipuCert2secudeCert(same_level->reverse);
			if(! tmp->element->reverse){
				aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
				return((CertificatePairs * )0);
			}
		}

		same_level = same_level->next;
	}
	
	ret->superior = aux_QUIPUcertlist2SECUDEcertificatepairs(certlist->superior);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUAttributeType2SECUDEObjId
 *
 ***************************************************************/
#ifdef __STDC__

static ObjId *aux_QUIPUAttributeType2SECUDEObjId(
	oid_table_attr	 *parm
)

#else

static ObjId *aux_QUIPUAttributeType2SECUDEObjId(
	parm
)
oid_table_attr	 *parm;

#endif

{
	ObjId * oid;
	int     i;
	char  * proc = "aux_QUIPUAttributeType2SECUDEObjId";


	if(! parm || (parm->oa_ot.ot_oid == (OIDentifier * )0) || 
	    ((parm->oa_ot.ot_oid->oid_nelem > 0) && (parm->oa_ot.ot_oid->oid_elements == (unsigned int * )0))){
		aux_add_error(EINVALID, "Invalid parameter", CNULL, 0, proc);
		return(NULLOBJID);
	}

	oid = (ObjId * )malloc(sizeof(ObjId));

	oid->oid_nelem = parm->oa_ot.ot_oid->oid_nelem;
	if(! oid->oid_nelem){
		oid->oid_nelem = 0;
		oid->oid_elements = (unsigned * )0;
	} 
	else {
		oid->oid_elements = (unsigned int * )malloc(parm->oa_ot.ot_oid->oid_nelem * sizeof(unsigned int));
		for ( i = 0; i < parm->oa_ot.ot_oid->oid_nelem; i++)
			oid->oid_elements[i] = parm->oa_ot.ot_oid->oid_elements[i];
	}
	return(oid);
}


/***************************************************************
 *
 * Procedure aux_QUIPUAttrSequence2SECUDESETOFAttr
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_Attr *aux_QUIPUAttrSequence2SECUDESETOFAttr(
	Attr_Sequence	  parm
)

#else

static SET_OF_Attr *aux_QUIPUAttrSequence2SECUDESETOFAttr(
	parm
)
Attr_Sequence	  parm;

#endif

{
	SET_OF_Attr                    * ret, * ret_tmp;
	Attr_Sequence  	                 parm_tmp;
	PE			         pe;
	AV_Sequence                      avseq = NULLAV;
	AttrValues * attrvalues;
	OctetString		       * ostr;
	int			         rc;
	char         	               * proc = "aux_QUIPUAttrSequence2SECUDESETOFAttr";


	if(! parm || ! parm->attr_type){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_Attr * )0);
	}

	ret = (SET_OF_Attr * )malloc(sizeof(SET_OF_Attr));

	ret->element = (Attr * )malloc(sizeof(Attr));

	ret->element->type = (OIDentifier * ) aux_QUIPUAttributeType2SECUDEObjId (parm->attr_type);
	if(! ret->element->type){
		aux_add_error(EINVALID, "parm->attr_type", CNULL, 0, proc);
		return((SET_OF_Attr * )0);
	}
	if(parm->attr_value){
		avseq = parm->attr_value;

		ret->element->values = (AttrValues * )malloc(sizeof(AttrValues));
		ret->element->values->member_IF_1 = grab_pe(avseq->avseq_av);

		ret->element->values->next = (AttrValues * )0;
		attrvalues = ret->element->values;

		avseq = avseq->avseq_next;
		while(avseq){
			attrvalues->next = (AttrValues * )malloc(sizeof(AttrValues));

			attrvalues = attrvalues->next;
			attrvalues->member_IF_1 = grab_pe(avseq->avseq_av);
			attrvalues->next = (AttrValues * )0;

			avseq = avseq->avseq_next;
		}
	}
	else ret->element->values = (AttrValues * )0;

	ret->next = (SET_OF_Attr *)0;

	for (ret_tmp = ret, parm_tmp = parm->attr_link; parm_tmp; parm_tmp = parm_tmp->attr_link) {
		ret_tmp->next = (SET_OF_Attr * )malloc(sizeof(SET_OF_Attr));

		ret_tmp = ret_tmp->next;

		ret_tmp->element = (Attr * )malloc(sizeof(Attr));

		ret_tmp->element->type = (OIDentifier * ) aux_QUIPUAttributeType2SECUDEObjId (parm_tmp->attr_type);
		if(! ret_tmp->element->type){
			aux_add_error(EINVALID, "parm_tmp->attr_type", CNULL, 0, proc);
			return((SET_OF_Attr * )0);
		}

		if(parm_tmp->attr_value){
			avseq = parm_tmp->attr_value;

			ret_tmp->element->values = (AttrValues * )malloc(sizeof(AttrValues));
			ret_tmp->element->values->member_IF_1 = grab_pe(avseq->avseq_av);

			ret_tmp->element->values->next = (AttrValues * )0;
			attrvalues = ret_tmp->element->values;

			avseq = avseq->avseq_next;
			while(avseq){
				attrvalues->next = (AttrValues * )malloc(sizeof(AttrValues));

				attrvalues = attrvalues->next;
				attrvalues->member_IF_1 = grab_pe(avseq->avseq_av);
				attrvalues->next = (AttrValues * )0;

				avseq = avseq->avseq_next;
			}
		}
		else ret_tmp->element->values = (AttrValues * )0;

		ret_tmp->next = (SET_OF_Attr *)0;
	} 

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUAttrSequence2SECUDEAttr
 *
 ***************************************************************/
#ifdef __STDC__

static Attr *aux_QUIPUAttrSequence2SECUDEAttr(
	Attr_Sequence	  parm
)

#else

static Attr *aux_QUIPUAttrSequence2SECUDEAttr(
	parm
)
Attr_Sequence	  parm;

#endif

{
	Attr                           * ret;
	AV_Sequence         	         avseq = NULLAV;
	AttrValues * attrvalues;
	char         	               * proc = "aux_QUIPUAttrSequence2SECUDEAttr";


	if(! parm || ! parm->attr_type){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((Attr * )0);
	}

	ret = (Attr * )malloc(sizeof(Attr));

	ret->type = (OIDentifier * ) aux_QUIPUAttributeType2SECUDEObjId (parm->attr_type);
	if(! ret->type){
		aux_add_error(EINVALID, "parm->attr_type", CNULL, 0, proc);
		return((Attr * )0);
	}
	if(parm->attr_value){
		avseq = parm->attr_value;
		ret->values = (AttrValues * )malloc(sizeof(AttrValues));
		ret->values->member_IF_1 = grab_pe(avseq->avseq_av);
		ret->values->next = (AttrValues * )0;
		attrvalues = ret->values;

		avseq = avseq->avseq_next;
		while(avseq){
			attrvalues->next = (AttrValues * )malloc(sizeof(AttrValues));
			attrvalues = attrvalues->next;
			attrvalues->member_IF_1 = grab_pe(avseq->avseq_av);
			attrvalues->next = (AttrValues * )0;

			avseq = avseq->avseq_next;
		}
	}
	else ret->values = (AttrValues * )0; 

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUAVA2SECUDEAttrValAssert
 *
 ***************************************************************/
#ifdef __STDC__

static AttrValueAssertion *aux_QUIPUAVA2SECUDEAttrValAssert(
	AVA	 *parm
)

#else

static AttrValueAssertion *aux_QUIPUAVA2SECUDEAttrValAssert(
	parm
)
AVA	 *parm;

#endif

{
	AttrValueAssertion  * ret;
	char      			        * proc = "aux_QUIPUAVA2SECUDEAttrValAssert";

	if(! parm || ! parm->ava_type || ! parm->ava_value){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((AttrValueAssertion * )0);
	}

	ret = (AttrValueAssertion * )malloc(sizeof(AttrValueAssertion));	
	ret->element_IF_0 = (OIDentifier * ) aux_QUIPUAttributeType2SECUDEObjId (parm->ava_type);
	ret->element_IF_1 = grab_pe(parm->ava_value);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUAttrSequence2SECUDESETOFObjId
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_ObjId *aux_QUIPUAttrSequence2SECUDESETOFObjId(
	Attr_Sequence	  parm
)

#else

static SET_OF_ObjId *aux_QUIPUAttrSequence2SECUDESETOFObjId(
	parm
)
Attr_Sequence	  parm;

#endif

{
	SET_OF_ObjId          * ret, * ret_tmp;
	Attr_Sequence  	        parm_tmp;
	char         	      * proc = "aux_QUIPUAttrSequence2SECUDESETOFObjId";


	if(! parm || ! parm->attr_type){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_ObjId * )0);
	}

	ret = (SET_OF_ObjId * )malloc(sizeof(SET_OF_ObjId));

	ret->element = aux_QUIPUAttributeType2SECUDEObjId (parm->attr_type);
	if(! ret->element){
		aux_add_error(EINVALID, "parm->attr_type", CNULL, 0, proc);
		return((SET_OF_ObjId * )0);
	}
	ret->next = (SET_OF_ObjId *)0;

	for (ret_tmp = ret, parm_tmp = parm->attr_link; parm_tmp; parm_tmp = parm_tmp->attr_link) {
		ret_tmp->next = (SET_OF_ObjId * )malloc(sizeof(SET_OF_ObjId));

		ret_tmp = ret_tmp->next;

		ret_tmp->element = aux_QUIPUAttributeType2SECUDEObjId (parm_tmp->attr_type);
		if(! ret_tmp->element){
			aux_add_error(EINVALID, "parm_tmp->attr_type", CNULL, 0, proc);
			return((SET_OF_ObjId * )0);
		}

		ret_tmp->next = (SET_OF_ObjId *)0;
	} 

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUAttrSequence2SECUDEObjId
 *
 ***************************************************************/
#ifdef __STDC__

static ObjId *aux_QUIPUAttrSequence2SECUDEObjId(
	Attr_Sequence	  parm
)

#else

static ObjId *aux_QUIPUAttrSequence2SECUDEObjId(
	parm
)
Attr_Sequence	  parm;

#endif

{
	ObjId          * ret;
	char           * proc = "aux_QUIPUAttrSequence2SECUDEObjId";


	if(! parm || ! parm->attr_type){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return(NULLOBJID);
	}

	ret = aux_QUIPUAttributeType2SECUDEObjId (parm->attr_type);
	if(! ret){
		aux_add_error(EINVALID, "parm->attr_type", CNULL, 0, proc);
		return(NULLOBJID);
	}

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_cpy_random
 *
 ***************************************************************/
#ifdef __STDC__

static BitString *aux_cpy_random(
	struct random_number	 *random
)

#else

static BitString *aux_cpy_random(
	random
)
struct random_number	 *random;

#endif

{
	BitString      bstr;
	char	     * proc = "aux_cpy_random";


	if(! random){
		aux_add_error(EINVALID, "No parameter provided (random)", CNULL, 0, proc);
		return(NULLBITSTRING);
	}

	bstr.nbits = random->n_bits;
	bstr.bits = random->value;

	return(aux_cpy_BitString(&bstr));
}


/***************************************************************
 *
 * Procedure aux_free2_random
 *
 ***************************************************************/
#ifdef __STDC__

static void aux_free2_random(
	register struct random_number	 *random
)

#else

static void aux_free2_random(
	random
)
register struct random_number	 *random;

#endif

{
	char * proc = "aux_free2_random";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);	
#endif

	if (random){
		if (random->value) free(random->value);
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_random
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_random(
	struct random_number	 **random
)

#else

void aux_free_random(
	random
)
struct random_number	 **random;

#endif

{
	char	     * proc = "aux_free_random";


	if (random && *random) {
		aux_free2_random(*random);
		free(*random);
		*random = (struct random_number * ) 0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_extract_TokenTBS_from_BindArg
 *
 ***************************************************************/
#ifdef __STDC__

static TokenTBS *aux_extract_TokenTBS_from_BindArg(
	struct ds_bind_arg	 *ds_bindarg
)

#else

static TokenTBS *aux_extract_TokenTBS_from_BindArg(
	ds_bindarg
)
struct ds_bind_arg	 *ds_bindarg;

#endif

{
	TokenTBS       * tok_tbs;
	PE    	         pe;
	char	       * proc = "aux_extract_TokenTBS_from_BindArg";


	if(! ds_bindarg){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((TokenTBS * )0);
	}

	tok_tbs = (TokenTBS * )malloc(sizeof(TokenTBS));

	encode_AF_AlgorithmIdentifier(&pe, 0, 0, NULLCP, &(ds_bindarg->dba_alg));
	tok_tbs->signatureAI = AlgId_dec(pe);
	if(pe)
		pe_free(pe);
	if(! tok_tbs->signatureAI){
		aux_add_error(EDECODE, "AlgId_dec failed", CNULL, 0, proc);
		return((TokenTBS * )0);
	}

	pe = dn_enc(ds_bindarg->dba_dn);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &tok_tbs->dname);
	pe_free(pe);

	tok_tbs->time = aux_cpy_Name(ds_bindarg->dba_time1);
	tok_tbs->random = aux_cpy_random(&ds_bindarg->dba_r1);

	return(tok_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_AddArgumentTBS_from_AddArg
 *
 ***************************************************************/
#ifdef __STDC__

static AddArgumentTBS *aux_extract_AddArgumentTBS_from_AddArg(
	struct ds_addentry_arg	 *ds_addarg
)

#else

static AddArgumentTBS *aux_extract_AddArgumentTBS_from_AddArg(
	ds_addarg
)
struct ds_addentry_arg	 *ds_addarg;

#endif

{
	AddArgumentTBS  * addarg_tbs;
	PE    	          pe;
	char	        * proc = "aux_extract_AddArgumentTBS_from_AddArg";

	/* ada_object and ada_entry are MANDATORY */
	if(! ds_addarg || ! ds_addarg->ada_object || ! ds_addarg->ada_entry){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((AddArgumentTBS * )0);
	}

	addarg_tbs = (AddArgumentTBS * )malloc(sizeof(AddArgumentTBS));

	pe = dn_enc(ds_addarg->ada_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &addarg_tbs->ada_object);
	pe_free(pe);

	addarg_tbs->ada_entry = aux_QUIPUAttrSequence2SECUDESETOFAttr(ds_addarg->ada_entry);
	addarg_tbs->ada_common = aux_cpy_CommonArguments (& ds_addarg->ada_common);

	return(addarg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_CompareArgumentTBS_from_CompareArg
 *
 ***************************************************************/
#ifdef __STDC__

static CompareArgumentTBS *aux_extract_CompareArgumentTBS_from_CompareArg(
	struct ds_compare_arg	 *ds_comparearg
)

#else

static CompareArgumentTBS *aux_extract_CompareArgumentTBS_from_CompareArg(
	ds_comparearg
)
struct ds_compare_arg	 *ds_comparearg;

#endif

{
	CompareArgumentTBS * comparearg_tbs;
	PE    	             pe;
	char	           * proc = "aux_extract_CompareArgumentTBS_from_CompareArg";


	/* cma_object and cma_purported are MANDATORY */
	if(! ds_comparearg || ! ds_comparearg->cma_object){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((CompareArgumentTBS * )0);
	}

	comparearg_tbs = (CompareArgumentTBS * )malloc(sizeof(CompareArgumentTBS));

	pe = dn_enc(ds_comparearg->cma_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &comparearg_tbs->cma_object);
	pe_free(pe);

	comparearg_tbs->cma_purported = aux_QUIPUAVA2SECUDEAttrValAssert(& ds_comparearg->cma_purported);
	comparearg_tbs->cma_common = aux_cpy_CommonArguments (& ds_comparearg->cma_common);

	return(comparearg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_CompareResultTBS_from_CompareRes
 *
 ***************************************************************/
#ifdef __STDC__

static CompareResultTBS *aux_extract_CompareResultTBS_from_CompareRes(
	struct ds_compare_result	 *ds_compareres
)

#else

static CompareResultTBS *aux_extract_CompareResultTBS_from_CompareRes(
	ds_compareres
)
struct ds_compare_result	 *ds_compareres;

#endif

{
	CompareResultTBS * compareres_tbs;
	PE    	           pe;
	char	         * proc = "aux_extract_CompareResultTBS_from_CompareRes";


	if(! ds_compareres){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((CompareResultTBS * )0);
	}

	compareres_tbs = (CompareResultTBS * )malloc(sizeof(CompareResultTBS));

	if(! ds_compareres->cmr_object)   /* cmr_object is OPTIONAL */
		compareres_tbs->cmr_object = NULLDNAME;
	else {
		pe = dn_enc(ds_compareres->cmr_object);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &compareres_tbs->cmr_object);
		pe_free(pe);
	}

	compareres_tbs->cmr_matched = ds_compareres->cmr_matched;

	/* from das.py */
	if(ds_compareres->cmr_iscopy == INFO_MASTER)
		ds_compareres->cmr_pepsycopy = TRUE;
	else
		ds_compareres->cmr_pepsycopy = FALSE;
	compareres_tbs->cmr_fromEntry = ds_compareres->cmr_pepsycopy;

	compareres_tbs->cmr_common = aux_cpy_CommonRes (& ds_compareres->cmr_common);

	return(compareres_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_ListArgumentTBS_from_ListArg
 *
 ***************************************************************/
#ifdef __STDC__

static ListArgumentTBS *aux_extract_ListArgumentTBS_from_ListArg(
	struct ds_list_arg	 *ds_listarg
)

#else

static ListArgumentTBS *aux_extract_ListArgumentTBS_from_ListArg(
	ds_listarg
)
struct ds_list_arg	 *ds_listarg;

#endif

{
	ListArgumentTBS * listarg_tbs;
	PE    	          pe;
	char	        * proc = "aux_extract_ListArgumentTBS_from_ListArg";


	if(! ds_listarg){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ListArgumentTBS * )0);
	}

	listarg_tbs = (ListArgumentTBS * )malloc(sizeof(ListArgumentTBS));

	pe = dn_enc(ds_listarg->lsa_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &listarg_tbs->object);
	pe_free(pe);

	listarg_tbs->lsa_common = aux_cpy_CommonArguments (& ds_listarg->lsa_common);

	return(listarg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_ListResultTBS_from_ListRes
 *
 ***************************************************************/
#ifdef __STDC__

static ListResultTBS *aux_extract_ListResultTBS_from_ListRes(
	struct ds_list_result	 *ds_listres
)

#else

static ListResultTBS *aux_extract_ListResultTBS_from_ListRes(
	ds_listres
)
struct ds_list_result	 *ds_listres;

#endif

{
	ListResultTBS * listres_tbs;
	PE    	          pe;
	char	        * proc = "aux_extract_ListResultTBS_from_ListRes";


	if(! ds_listres){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ListResultTBS * )0);
	}

	listres_tbs = (ListResultTBS * )malloc(sizeof(ListResultTBS));

	/* Note uncorrelated will need to be    */
	/* added in to do the secure stuff      */
	/* in a distributed manner              */
	/* this also applies to search          */

	listres_tbs->lsr_type = LSR_INFO;
	listres_tbs->lsrtbs_un.listinfo = aux_cpy_ListInfo (ds_listres);

	return(listres_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg
 *
 ***************************************************************/
#ifdef __STDC__

static ModifyEntryArgumentTBS *aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg(
	struct ds_modifyentry_arg	 *ds_modifyentryarg
)

#else

static ModifyEntryArgumentTBS *aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg(
	ds_modifyentryarg
)
struct ds_modifyentry_arg	 *ds_modifyentryarg;

#endif

{
	ModifyEntryArgumentTBS * modifyentryarg_tbs;
	PE    	                 pe;
	char	               * proc = "aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg";


	/* mea_object is MANDATORY */
	if(! ds_modifyentryarg || ! ds_modifyentryarg->mea_object){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ModifyEntryArgumentTBS * )0);
	}

	modifyentryarg_tbs = (ModifyEntryArgumentTBS * )malloc(sizeof(ModifyEntryArgumentTBS));

	pe = dn_enc(ds_modifyentryarg->mea_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &modifyentryarg_tbs->mea_object);
	pe_free(pe);

	modifyentryarg_tbs->mea_changes = aux_cpy_SEQUENCE_OF_EntryModification(ds_modifyentryarg->mea_changes);
	modifyentryarg_tbs->mea_common = aux_cpy_CommonArguments (& ds_modifyentryarg->mea_common);

	return(modifyentryarg_tbs);
}


/***************************************************************
 *
 * Procedure rdn_enc
 *
 ***************************************************************/
#ifdef __STDC__

static PE rdn_enc(
	RDN	  rdn
)

#else

static PE rdn_enc(
	rdn
)
RDN	  rdn;

#endif

{
	PE ret_pe;

	(void) encode_IF_RelativeDistinguishedName (&ret_pe, 0, 0, NULLCP, rdn);
	return(ret_pe);
}


/***************************************************************
 *
 * Procedure aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg
 *
 ***************************************************************/
#ifdef __STDC__

static ModifyRDNArgumentTBS *aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg(
	struct ds_modifyrdn_arg	 *ds_modifyrdnarg
)

#else

static ModifyRDNArgumentTBS *aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg(
	ds_modifyrdnarg
)
struct ds_modifyrdn_arg	 *ds_modifyrdnarg;

#endif

{
	ModifyRDNArgumentTBS * modifyrdnarg_tbs;
	PE    	               pe;
	char	             * proc = "aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg";


	/* mra_object and mra_newrdn are MANDATORY */
	if(! ds_modifyrdnarg || ! ds_modifyrdnarg->mra_object || ! ds_modifyrdnarg->mra_newrdn){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ModifyRDNArgumentTBS * )0);
	}

	modifyrdnarg_tbs = (ModifyRDNArgumentTBS * )malloc(sizeof(ModifyRDNArgumentTBS));

	pe = dn_enc(ds_modifyrdnarg->mra_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &modifyrdnarg_tbs->mra_object);
	pe_free(pe);

	pe = rdn_enc(ds_modifyrdnarg->mra_newrdn);
	parse_SECIF_RelativeDistinguishedName(pe, 1, NULLIP, NULLVP, &modifyrdnarg_tbs->mra_newrdn);
	pe_free(pe);

	modifyrdnarg_tbs->deleterdn = ds_modifyrdnarg->deleterdn;
	modifyrdnarg_tbs->mra_common = aux_cpy_CommonArguments (& ds_modifyrdnarg->mra_common);

	return(modifyrdnarg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_ReadArgumentTBS_from_ReadArg
 *
 ***************************************************************/
#ifdef __STDC__

static ReadArgumentTBS *aux_extract_ReadArgumentTBS_from_ReadArg(
	struct ds_read_arg	 *ds_readarg
)

#else

static ReadArgumentTBS *aux_extract_ReadArgumentTBS_from_ReadArg(
	ds_readarg
)
struct ds_read_arg	 *ds_readarg;

#endif

{
	ReadArgumentTBS * readarg_tbs;
	PE    	          pe;
	char	        * proc = "aux_extract_ReadArgumentTBS_from_ReadArg";


	if(! ds_readarg){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ReadArgumentTBS * )0);
	}

	readarg_tbs = (ReadArgumentTBS * )malloc(sizeof(ReadArgumentTBS));

	pe = dn_enc(ds_readarg->rda_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &readarg_tbs->object);
	pe_free(pe);

	readarg_tbs->rda_common = aux_cpy_CommonArguments (&ds_readarg->rda_common);
	readarg_tbs->rda_eis = aux_cpy_EntryInfoSelection(&ds_readarg->rda_eis);

	return(readarg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_ReadResultTBS_from_ReadRes
 *
 ***************************************************************/
#ifdef __STDC__

static ReadResultTBS *aux_extract_ReadResultTBS_from_ReadRes(
	struct ds_read_result	 *ds_readres
)

#else

static ReadResultTBS *aux_extract_ReadResultTBS_from_ReadRes(
	ds_readres
)
struct ds_read_result	 *ds_readres;

#endif

{
	ReadResultTBS   * readres_tbs;
	PE    	          pe;
	char	        * proc = "aux_extract_ReadResultTBS_from_ReadRes";


	if(! ds_readres){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ReadResultTBS * )0);
	}

	readres_tbs = (ReadResultTBS * )malloc(sizeof(ReadResultTBS));

	readres_tbs->rdr_common = aux_cpy_CommonRes(&ds_readres->rdr_common);
	readres_tbs->rdr_entry = aux_cpy_EntryINFO(&ds_readres->rdr_entry);
	
	return(readres_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_RemoveArgumentTBS_from_RemoveArg
 *
 ***************************************************************/
#ifdef __STDC__

static RemoveArgumentTBS *aux_extract_RemoveArgumentTBS_from_RemoveArg(
	struct ds_removeentry_arg	 *ds_removearg
)

#else

static RemoveArgumentTBS *aux_extract_RemoveArgumentTBS_from_RemoveArg(
	ds_removearg
)
struct ds_removeentry_arg	 *ds_removearg;

#endif

{
	RemoveArgumentTBS * removearg_tbs;
	PE    	            pe;
	char	          * proc = "aux_extract_RemoveArgumentTBS_from_RemoveArg";


	/* rma_object is MANDATORY */
	if(! ds_removearg || ! ds_removearg->rma_object){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((RemoveArgumentTBS * )0);
	}

	removearg_tbs = (RemoveArgumentTBS * )malloc(sizeof(RemoveArgumentTBS));

	pe = dn_enc(ds_removearg->rma_object);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &removearg_tbs->rma_object);
	pe_free(pe);

	removearg_tbs->rma_common = aux_cpy_CommonArguments (&ds_removearg->rma_common);

	return(removearg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_SearchArgumentTBS_from_SearchArg
 *
 ***************************************************************/
#ifdef __STDC__

static SearchArgumentTBS *aux_extract_SearchArgumentTBS_from_SearchArg(
	struct ds_search_arg	 *ds_searcharg
)

#else

static SearchArgumentTBS *aux_extract_SearchArgumentTBS_from_SearchArg(
	ds_searcharg
)
struct ds_search_arg	 *ds_searcharg;

#endif

{
	SearchArgumentTBS * searcharg_tbs;
	PE    	            pe;
	char	          * proc = "aux_extract_SearchArgumentTBS_from_SearchArg";


	if(! ds_searcharg){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SearchArgumentTBS * )0);
	}

	searcharg_tbs = (SearchArgumentTBS * )malloc(sizeof(SearchArgumentTBS));

	pe = dn_enc(ds_searcharg->sra_baseobject);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &searcharg_tbs->baseobject);
	pe_free(pe);

	searcharg_tbs->subset = ds_searcharg->sra_subset;
	searcharg_tbs->sra_common = aux_cpy_CommonArguments (&ds_searcharg->sra_common);
	searcharg_tbs->sra_eis = aux_cpy_EntryInfoSelection(&ds_searcharg->sra_eis);
	searcharg_tbs->filter = aux_cpy_SFilter (ds_searcharg->sra_filter);
	searcharg_tbs->searchaliases = ds_searcharg->sra_searchaliases;

	return(searcharg_tbs);
}


/***************************************************************
 *
 * Procedure aux_extract_SearchResultTBS_from_SearchRes
 *
 ***************************************************************/
#ifdef __STDC__

static SearchResultTBS *aux_extract_SearchResultTBS_from_SearchRes(
	struct ds_search_result	 *ds_searchres
)

#else

static SearchResultTBS *aux_extract_SearchResultTBS_from_SearchRes(
	ds_searchres
)
struct ds_search_result	 *ds_searchres;

#endif

{
	SearchResultTBS   * searchres_tbs;
	PE    	            pe;
	char	          * proc = "aux_extract_SearchResultTBS_from_SearchArg";


	if(! ds_searchres){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SearchResultTBS * )0);
	}

	searchres_tbs = (SearchResultTBS * )malloc(sizeof(SearchResultTBS));

	/* Note uncorrelated will need to be    */
	/* added in to do the secure stuff      */
	/* in a distributed manner              */
	/* this also applies to search          */

	searchres_tbs->srr_correlated = ds_searchres->srr_correlated;

	if (searchres_tbs->srr_correlated == FALSE) {
		searchres_tbs->srrtbs_un.uncorrel_searchinfo = (SET_OF_SearchResult * )malloc(sizeof(SET_OF_SearchResult));
		/* uncorrelated stuff needs to be added here */
	}
	else
		searchres_tbs->srrtbs_un.searchinfo = aux_cpy_SearchInfo (ds_searchres);

	return(searchres_tbs);
}


/***************************************************************
 *
 * Procedure aux_cpy_SFilter
 *
 ***************************************************************/
#ifdef __STDC__

static SFilter *aux_cpy_SFilter(
	Filter	  QUIPUfilter
)

#else

static SFilter *aux_cpy_SFilter(
	QUIPUfilter
)
Filter	  QUIPUfilter;

#endif

{
	SFilter  * SECUDEfilter;
	char	 * proc = "aux_cpy_SFilter";

	if(! QUIPUfilter){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SFilter * )0);
	}

	SECUDEfilter = (SFilter * )malloc(sizeof(SFilter));

	SECUDEfilter->flt_type = QUIPUfilter->flt_type;

	switch(SECUDEfilter->flt_type){
	case FILTER_ITEM:
		SECUDEfilter->flt_un.flt_un_item = aux_cpy_SFilterItem(&QUIPUfilter->flt_un.flt_un_item);
		break;
	case FILTER_AND:
		SECUDEfilter->flt_un.flt_un_filterset = aux_cpy_SET_OF_SFilter(QUIPUfilter->flt_un.flt_un_filter);
		break;
	case FILTER_OR:
		SECUDEfilter->flt_un.flt_un_filterset = aux_cpy_SET_OF_SFilter(QUIPUfilter->flt_un.flt_un_filter);
		break;
	case FILTER_NOT:
		SECUDEfilter->flt_un.flt_un_filter = aux_cpy_SFilter(QUIPUfilter->flt_un.flt_un_filter);
		break;
	default:
		aux_add_error(EINVALID, "QUIPUfilter->flt_type has bad value", CNULL, 0, proc);
		return((SFilter * )0);
	}

	return(SECUDEfilter);
}


/***************************************************************
 *
 * Procedure aux_cpy_SET_OF_SFilter
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_SFilter *aux_cpy_SET_OF_SFilter(
	Filter	  QUIPUfilter
)

#else

static SET_OF_SFilter *aux_cpy_SET_OF_SFilter(
	QUIPUfilter
)
Filter	  QUIPUfilter;

#endif

{
	SET_OF_SFilter     * filterset;
	char		   * proc = "aux_cpy_SET_OF_SFilter";

	if(! QUIPUfilter){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_SFilter * )0);
	}

	filterset = (SET_OF_SFilter * )malloc(sizeof(SET_OF_SFilter));

	filterset->element = aux_cpy_SFilter(QUIPUfilter);
	filterset->next = aux_cpy_SET_OF_SFilter(QUIPUfilter->flt_next);

	return(filterset);
}


/***************************************************************
 *
 * Procedure aux_cpy_SEQUENCE_OF_StringsCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

static SEQUENCE_OF_StringsCHOICE *aux_cpy_SEQUENCE_OF_StringsCHOICE(
	Filter_Substrings	 *QUIPUfilsubstrgs
)

#else

static SEQUENCE_OF_StringsCHOICE *aux_cpy_SEQUENCE_OF_StringsCHOICE(
	QUIPUfilsubstrgs
)
Filter_Substrings	 *QUIPUfilsubstrgs;

#endif

{
	SEQUENCE_OF_StringsCHOICE  * SECUDEfilsubstrgs;
	char	                   * proc = "aux_cpy_SEQUENCE_OF_StringsCHOICE";


	if(! QUIPUfilsubstrgs){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SEQUENCE_OF_StringsCHOICE * )0);
	}

	SECUDEfilsubstrgs = (SEQUENCE_OF_StringsCHOICE * )malloc(sizeof(SEQUENCE_OF_StringsCHOICE));

	SECUDEfilsubstrgs->element = (StringsCHOICE * )malloc(sizeof(StringsCHOICE));

	if(QUIPUfilsubstrgs->fi_sub_initial){ 
		SECUDEfilsubstrgs->element->strings_type = STRINGS_INITIAL;
		SECUDEfilsubstrgs->element->strings_un.initial = grab_pe(QUIPUfilsubstrgs->fi_sub_initial->avseq_av);
	}
	else if(QUIPUfilsubstrgs->fi_sub_any){
		SECUDEfilsubstrgs->element->strings_type = STRINGS_ANY;
		SECUDEfilsubstrgs->element->strings_un.any = grab_pe(QUIPUfilsubstrgs->fi_sub_any->avseq_av);
	}
	else if(QUIPUfilsubstrgs->fi_sub_final){
		SECUDEfilsubstrgs->element->strings_type = STRINGS_FINAL;
		SECUDEfilsubstrgs->element->strings_un.final = grab_pe(QUIPUfilsubstrgs->fi_sub_final->avseq_av);
	}
	else {
		aux_add_error(EINVALID, "strings_type", CNULL, 0, proc);
		return((SEQUENCE_OF_StringsCHOICE * )0);
	}

	SECUDEfilsubstrgs->next = (SEQUENCE_OF_StringsCHOICE * )0;
	/* because of QUIPU simplification */

	return(SECUDEfilsubstrgs);
}


/***************************************************************
 *
 * Procedure aux_cpy_SFilterSubstrings
 *
 ***************************************************************/
#ifdef __STDC__

static SFilterSubstrings *aux_cpy_SFilterSubstrings(
	Filter_Substrings	 *QUIPUfilsubstrgs
)

#else

static SFilterSubstrings *aux_cpy_SFilterSubstrings(
	QUIPUfilsubstrgs
)
Filter_Substrings	 *QUIPUfilsubstrgs;

#endif

{
	SFilterSubstrings  * SECUDEfilsubstrgs;
	char	           * proc = "aux_cpy_SFilterSubstrings";

	if(! QUIPUfilsubstrgs){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SFilterSubstrings * )0);
	}

	SECUDEfilsubstrgs = (SFilterSubstrings * )malloc(sizeof(SFilterSubstrings));

	SECUDEfilsubstrgs->type = (OIDentifier * ) aux_QUIPUAttributeType2SECUDEObjId (QUIPUfilsubstrgs->fi_sub_type);
	SECUDEfilsubstrgs->seq = aux_cpy_SEQUENCE_OF_StringsCHOICE (QUIPUfilsubstrgs);

	return(SECUDEfilsubstrgs);
}


/***************************************************************
 *
 * Procedure aux_cpy_SFilterItem
 *
 ***************************************************************/
#ifdef __STDC__

static SFilterItem *aux_cpy_SFilterItem(
	struct filter_item	 *QUIPUitem
)

#else

static SFilterItem *aux_cpy_SFilterItem(
	QUIPUitem
)
struct filter_item	 *QUIPUitem;

#endif

{
	SFilterItem        * SECUDEitem;
	char	           * proc = "aux_cpy_SFilterItem";

	if(! QUIPUitem){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SFilterItem * )0);
	}

	SECUDEitem = (SFilterItem * )malloc(sizeof(SFilterItem));
	
	SECUDEitem->fi_type = QUIPUitem->fi_type;

	switch(SECUDEitem->fi_type){
	case FILTERITEM_EQUALITY:
		SECUDEitem->fi_un.fi_un_ava = aux_QUIPUAVA2SECUDEAttrValAssert(&QUIPUitem->fi_un.fi_un_ava);
		break;
	case FILTERITEM_SUBSTRINGS:
		SECUDEitem->fi_un.fi_un_substrings = aux_cpy_SFilterSubstrings(&QUIPUitem->fi_un.fi_un_substrings);
		break;
	case FILTERITEM_GREATEROREQUAL:
		SECUDEitem->fi_un.fi_un_ava = aux_QUIPUAVA2SECUDEAttrValAssert(&QUIPUitem->fi_un.fi_un_ava);
		break;
	case FILTERITEM_LESSOREQUAL:
		SECUDEitem->fi_un.fi_un_ava = aux_QUIPUAVA2SECUDEAttrValAssert(&QUIPUitem->fi_un.fi_un_ava);
		break;
	case FILTERITEM_PRESENT:
		SECUDEitem->fi_un.fi_un_type = (OIDentifier * )aux_QUIPUAttributeType2SECUDEObjId (QUIPUitem->fi_un.fi_un_type);
		break;
	case FILTERITEM_APPROX:
		SECUDEitem->fi_un.fi_un_ava = aux_QUIPUAVA2SECUDEAttrValAssert(&QUIPUitem->fi_un.fi_un_ava);
		break;
	default:
		aux_add_error(EINVALID, "QUIPUitem->fi_type has bad value", CNULL, 0, proc);
		return((SFilterItem * )0);
	}

	return(SECUDEitem);
}


/***************************************************************
 *
 * Procedure aux_cpy_CommonArguments
 *
 ***************************************************************/
#ifdef __STDC__

static CommonArguments *aux_cpy_CommonArguments(
	CommonArgs	 *QUIPUca
)

#else

static CommonArguments *aux_cpy_CommonArguments(
	QUIPUca
)
CommonArgs	 *QUIPUca;

#endif

{
	CommonArguments * SECUDEca;
	PE 		  pe;
	char	        * proc = "aux_cpy_CommonArguments";


	if(! QUIPUca){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((CommonArguments * )0);
	}

	SECUDEca = (CommonArguments * )malloc(sizeof(CommonArguments));

/* The fields ca_ext, ca_progress, ca_requestor and ca_aliased_rdns are provided
   as they are defined within X.500. Neither the QUIPU DSA or DUA use these
   fields (see Volume 5, section 17.2.1)
*/

	SECUDEca->ext = (SET_OF_SECExtension *)0;
	SECUDEca->aliasedRDNs = QUIPUca->ca_aliased_rdns;

	if(! QUIPUca->ca_requestor) 
		SECUDEca->requestor = NULLDNAME;
	else {
		pe = dn_enc(QUIPUca->ca_requestor);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &SECUDEca->requestor);
		pe_free(pe);
	}

	
	/* OperationProgress section */

	if(QUIPUca->ca_progress.op_resolution_phase > 1){
		SECUDEca->progress = (OperationProgress * )malloc(sizeof(OperationProgress));
		SECUDEca->progress->opResolutionPhase = QUIPUca->ca_progress.op_resolution_phase;
		SECUDEca->progress->opNextRDNToBeResolved = QUIPUca->ca_progress.op_nextrdntoberesolved;
	}
	else
		SECUDEca->progress = (OperationProgress * )0;


	/* ServiceControls section */

	if((QUIPUca->ca_servicecontrol.svc_options != 0) ||
            (QUIPUca->ca_servicecontrol.svc_prio != SVC_PRIO_MED) ||
            (QUIPUca->ca_servicecontrol.svc_timelimit != SVC_NOTIMELIMIT) ||
	    (QUIPUca->ca_servicecontrol.svc_sizelimit != SVC_NOSIZELIMIT) ||
	    (QUIPUca->ca_servicecontrol.svc_scopeofreferral != SVC_REFSCOPE_NONE)){
		SECUDEca->svc = (ServiceControls * )malloc(sizeof(ServiceControls));
		SECUDEca->svc->svc_options = QUIPUca->ca_servicecontrol.svc_options;
		SECUDEca->svc->svc_prio = QUIPUca->ca_servicecontrol.svc_prio;
		SECUDEca->svc->svc_timelimit = QUIPUca->ca_servicecontrol.svc_timelimit;
		SECUDEca->svc->svc_sizelimit = QUIPUca->ca_servicecontrol.svc_sizelimit;
		SECUDEca->svc->svc_scopeofreferral = QUIPUca->ca_servicecontrol.svc_scopeofreferral;
		SECUDEca->svc->svc_tmp = CNULL;
		SECUDEca->svc->svc_len = 0;
	}
	else
		SECUDEca->svc = (ServiceControls * )0;


	/* SecurityParameters section */

	SECUDEca->sec_parm = aux_cpy_SecurityParameters(QUIPUca->ca_security);

	return(SECUDEca);
}


/***************************************************************
 *
 * Procedure aux_cpy_SecurityParameters
 *
 ***************************************************************/
#ifdef __STDC__

static SecurityParameters *aux_cpy_SecurityParameters(
	struct security_parms	 *QUIPUsp
)

#else

static SecurityParameters *aux_cpy_SecurityParameters(
	QUIPUsp
)
struct security_parms	 *QUIPUsp;

#endif

{
	SecurityParameters    * SECUDEsp;
	PE 		        pe;
	char	              * proc = "aux_cpy_SecurityParameters";


	if(! QUIPUsp){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SecurityParameters * )0);
	}

	SECUDEsp = (SecurityParameters * )malloc(sizeof(SecurityParameters));

	SECUDEsp->certPath = aux_QUIPUcertlist2SECUDEcertpath(QUIPUsp->sp_path);
	if(! QUIPUsp->sp_name )
		SECUDEsp->name = NULLDNAME;
	else {
		pe = dn_enc(QUIPUsp->sp_name);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &SECUDEsp->name);
		pe_free(pe);
	}
	SECUDEsp->time = aux_cpy_Name(QUIPUsp->sp_time);
	SECUDEsp->random = aux_cpy_random(QUIPUsp->sp_random);
	SECUDEsp->target = QUIPUsp->sp_target;

	return(SECUDEsp);
}


/***************************************************************
 *
 * Procedure aux_cpy_CommonRes
 *
 ***************************************************************/
#ifdef __STDC__

static CommonRes *aux_cpy_CommonRes(
	CommonResults	 *QUIPUcr
)

#else

static CommonRes *aux_cpy_CommonRes(
	QUIPUcr
)
CommonResults	 *QUIPUcr;

#endif

{
	CommonRes       * SECUDEcr;
	PE 		  pe;
	char	        * proc = "aux_cpy_CommonRes";


	if(! QUIPUcr){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((CommonRes * )0);
	}

	SECUDEcr = (CommonRes * )malloc(sizeof(CommonRes));

	if(! QUIPUcr->cr_requestor) 
		SECUDEcr->performer = NULLDNAME;
	else {
		pe = dn_enc(QUIPUcr->cr_requestor);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &SECUDEcr->performer);
		pe_free(pe);
	}

	SECUDEcr->aliasDereferenced = QUIPUcr->cr_aliasdereferenced;

	/* SecurityParameters section */
	SECUDEcr->sec_parm = aux_cpy_SecurityParameters(QUIPUcr->cr_security);

	return(SECUDEcr);
}


/***************************************************************
 *
 * Procedure aux_cpy_SubordEntry
 *
 ***************************************************************/
#ifdef __STDC__

static SubordEntry *aux_cpy_SubordEntry(
	struct subordinate	 *QUIPUsubordinates
)

#else

static SubordEntry *aux_cpy_SubordEntry(
	QUIPUsubordinates
)
struct subordinate	 *QUIPUsubordinates;

#endif

{
	SubordEntry * SECUDEsubordentry;
	PE 	      pe; 
	char	    * proc = "aux_cpy_SubordEntry";


	if(! QUIPUsubordinates || ! QUIPUsubordinates->sub_rdn){  /*sub_rdn is MANDATORY*/
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SubordEntry * )0);
	}

	SECUDEsubordentry = (SubordEntry * )malloc(sizeof(SubordEntry));

	SECUDEsubordentry->sub_aliasentry = QUIPUsubordinates->sub_aliasentry;
	SECUDEsubordentry->sub_copy = QUIPUsubordinates->sub_copy;

	pe = rdn_enc(QUIPUsubordinates->sub_rdn);
	parse_SECIF_RelativeDistinguishedName(pe, 1, NULLIP, NULLVP, &SECUDEsubordentry->sub_rdn);
	pe_free(pe);

	return(SECUDEsubordentry);
}


/***************************************************************
 *
 * Procedure aux_cpy_SET_OF_SubordEntry
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_SubordEntry *aux_cpy_SET_OF_SubordEntry(
	struct subordinate	 *QUIPUsubordinates
)

#else

static SET_OF_SubordEntry *aux_cpy_SET_OF_SubordEntry(
	QUIPUsubordinates
)
struct subordinate	 *QUIPUsubordinates;

#endif

{
	SET_OF_SubordEntry * SECUDEsubordinates; 
	char	           * proc = "aux_cpy_SET_OF_SubordEntry";


	if(! QUIPUsubordinates){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_SubordEntry * )0);
	}

	SECUDEsubordinates = (SET_OF_SubordEntry * )malloc(sizeof(SET_OF_SubordEntry));

	SECUDEsubordinates->element = aux_cpy_SubordEntry(QUIPUsubordinates);
	SECUDEsubordinates->next = aux_cpy_SET_OF_SubordEntry(QUIPUsubordinates->sub_next);

	return(SECUDEsubordinates);
}


/***************************************************************
 *
 * Procedure aux_cpy_EntryModification
 *
 ***************************************************************/
#ifdef __STDC__

static EntryModification *aux_cpy_EntryModification(
	struct entrymod	 *QUIPUem
)

#else

static EntryModification *aux_cpy_EntryModification(
	QUIPUem
)
struct entrymod	 *QUIPUem;

#endif

{
	EntryModification * SECUDEem;
	PE 	            pe; 
	char	          * proc = "aux_cpy_EntryModification";


	if(! QUIPUem){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((EntryModification * )0);
	}

	SECUDEem = (EntryModification * )malloc(sizeof(EntryModification));
	
	SECUDEem->em_type = QUIPUem->em_type;

	switch (SECUDEem->em_type) {
	case EM_ADDATTRIBUTE:
		SECUDEem->em_un.em_un_attr = aux_QUIPUAttrSequence2SECUDEAttr(QUIPUem->em_what);
		break;
	case EM_REMOVEATTRIBUTE:
		SECUDEem->em_un.em_un_attrtype = aux_QUIPUAttrSequence2SECUDEObjId(QUIPUem->em_what);
		break;
	case EM_ADDVALUES:
		SECUDEem->em_un.em_un_attr = aux_QUIPUAttrSequence2SECUDEAttr(QUIPUem->em_what);
		break;
	case EM_REMOVEVALUES:
		SECUDEem->em_un.em_un_attr = aux_QUIPUAttrSequence2SECUDEAttr(QUIPUem->em_what);
		break;
	default:
		aux_add_error(EINVALID, "QUIPUem->em_type has bad value", CNULL, 0, proc);
		return((EntryModification * )0);
	}  /* switch */

	return(SECUDEem);
}


/***************************************************************
 *
 * Procedure aux_cpy_SEQUENCE_OF_EntryModification
 *
 ***************************************************************/
#ifdef __STDC__

static SEQUENCE_OF_EntryModification *aux_cpy_SEQUENCE_OF_EntryModification(
	struct entrymod	 *parm
)

#else

static SEQUENCE_OF_EntryModification *aux_cpy_SEQUENCE_OF_EntryModification(
	parm
)
struct entrymod	 *parm;

#endif

{
	SEQUENCE_OF_EntryModification * ret; 
	char	                      * proc = "aux_cpy_SEQUENCE_OF_EntryModification";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SEQUENCE_OF_EntryModification * )0);
	}

	ret = (SEQUENCE_OF_EntryModification * )malloc(sizeof(SEQUENCE_OF_EntryModification));

	ret->element = aux_cpy_EntryModification(parm);
	ret->next = aux_cpy_SEQUENCE_OF_EntryModification(parm->em_next);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_cpy_ListInfo
 *
 ***************************************************************/
#ifdef __STDC__

static ListInfo *aux_cpy_ListInfo(
	struct ds_list_result	 *QUIPUlsr
)

#else

static ListInfo *aux_cpy_ListInfo(
	QUIPUlsr
)
struct ds_list_result	 *QUIPUlsr;

#endif

{
	ListInfo * SECUDElistinfo;
	PE	   pe; 
	char	 * proc = "aux_cpy_ListInfo";


	if(! QUIPUlsr){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ListInfo * )0);
	}

	SECUDElistinfo = (ListInfo * )malloc(sizeof(ListInfo));

	if(! QUIPUlsr->lsr_object)  /* OPTIONAL */
		SECUDElistinfo->lsr_object = NULLDNAME;
	else {
		pe = dn_enc(QUIPUlsr->lsr_object);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &SECUDElistinfo->lsr_object);
		pe_free(pe);
	}

	SECUDElistinfo->lsr_subordinates = aux_cpy_SET_OF_SubordEntry(QUIPUlsr->lsr_subordinates);
	SECUDElistinfo->lsr_common = aux_cpy_CommonRes(&QUIPUlsr->lsr_common);

	if((QUIPUlsr->lsr_poq.poq_limitproblem != LSR_NOLIMITPROBLEM) || (QUIPUlsr->lsr_poq.poq_cref != NULLCONTINUATIONREF))
		SECUDElistinfo->lsr_poq = aux_QUIPUpoq2SECUDEpoq(&QUIPUlsr->lsr_poq);
	else SECUDElistinfo->lsr_poq = (PartialOutQual * )0;

	return(SECUDElistinfo);
}


/***************************************************************
 *
 * Procedure aux_cpy_SearchInfo
 *
 ***************************************************************/
#ifdef __STDC__

static SearchInfo *aux_cpy_SearchInfo(
	struct ds_search_result	 *QUIPUsrr
)

#else

static SearchInfo *aux_cpy_SearchInfo(
	QUIPUsrr
)
struct ds_search_result	 *QUIPUsrr;

#endif

{
	SearchInfo * SECUDEsearchinfo; 
	PE           pe;
	char	   * proc = "aux_cpy_SearchInfo";


	if(! QUIPUsrr){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SearchInfo * )0);
	}

	SECUDEsearchinfo = (SearchInfo * )malloc(sizeof(SearchInfo));

	if(! QUIPUsrr->CSR_object)  /* OPTIONAL */
		SECUDEsearchinfo->srr_object = NULLDNAME;
	else {
		pe = dn_enc(QUIPUsrr->CSR_object);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &SECUDEsearchinfo->srr_object);
		pe_free(pe);
	}

	SECUDEsearchinfo->srr_common = aux_cpy_CommonRes(&QUIPUsrr->CSR_common);
	SECUDEsearchinfo->srr_entries = aux_cpy_SET_OF_EntryINFO (QUIPUsrr->CSR_entries);

	if((QUIPUsrr->CSR_limitproblem != LSR_NOLIMITPROBLEM) || (QUIPUsrr->CSR_cr != NULLCONTINUATIONREF))
		SECUDEsearchinfo->srr_poq = aux_QUIPUpoq2SECUDEpoq(&QUIPUsrr->srr_un.srr_unit->srr_poq);
	else SECUDEsearchinfo->srr_poq = (PartialOutQual * )0;

	return(SECUDEsearchinfo);
}


/***************************************************************
 *
 * Procedure aux_cpy_EntryInfoSelection
 *
 ***************************************************************/
#ifdef __STDC__

static EntryInfoSEL *aux_cpy_EntryInfoSelection(
	EntryInfoSelection	 *QUIPUeis
)

#else

static EntryInfoSEL *aux_cpy_EntryInfoSelection(
	QUIPUeis
)
EntryInfoSelection	 *QUIPUeis;

#endif

{
	EntryInfoSEL * SECUDEeis;
	char	     * proc = "aux_cpy_EntryInfoSelection";


	if(! QUIPUeis){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((EntryInfoSEL * )0);
	}

	if((QUIPUeis->eis_allattributes != TRUE) || (QUIPUeis->eis_infotypes != EIS_ATTRIBUTESANDVALUES)){
		SECUDEeis = (EntryInfoSEL * )malloc(sizeof(EntryInfoSEL));
		if(QUIPUeis->eis_allattributes == TRUE){
			SECUDEeis->eis_allattributes = TRUE;
			SECUDEeis->eis_select = (SET_OF_ObjId *)0;
		}
		else {
			SECUDEeis->eis_allattributes = FALSE;
			SECUDEeis->eis_select = aux_QUIPUAttrSequence2SECUDESETOFObjId (QUIPUeis->eis_select);
		}
		SECUDEeis->eis_infotypes = QUIPUeis->eis_infotypes;
	
		return(SECUDEeis);
	}

	return((EntryInfoSEL * )0);
}


/***************************************************************
 *
 * Procedure aux_cpy_EntryINFO
 *
 ***************************************************************/
#ifdef __STDC__

static EntryINFO *aux_cpy_EntryINFO(
	EntryInfo	 *QUIPUei
)

#else

static EntryINFO *aux_cpy_EntryINFO(
	QUIPUei
)
EntryInfo	 *QUIPUei;

#endif

{
	EntryINFO    * SECUDEei;
	PE 	       pe;
	char	     * proc = "aux_cpy_EntryINFO";


	/* ent_dn is MANDATORY */
	if(! QUIPUei || ! QUIPUei->ent_dn){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((EntryINFO * )0);
	}

	SECUDEei = (EntryINFO * )malloc(sizeof(EntryINFO));

	pe = dn_enc(QUIPUei->ent_dn);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &SECUDEei->ent_dn);
	pe_free(pe);	

	/* from das.py */
	if(QUIPUei->ent_iscopy == INFO_MASTER)
		QUIPUei->ent_pepsycopy = TRUE;	
	else
		QUIPUei->ent_pepsycopy = FALSE;
	SECUDEei->ent_fromentry = QUIPUei->ent_pepsycopy;

	SECUDEei->ent_attr = aux_cpy_SET_OF_AttrAttrTypeCHOICE(QUIPUei->ent_attr);

	return(SECUDEei);
}


/***************************************************************
 *
 * Procedure aux_cpy_SET_OF_EntryINFO
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_EntryINFO *aux_cpy_SET_OF_EntryINFO(
	EntryInfo	 *QUIPUei
)

#else

static SET_OF_EntryINFO *aux_cpy_SET_OF_EntryINFO(
	QUIPUei
)
EntryInfo	 *QUIPUei;

#endif

{
	SET_OF_EntryINFO   * ret; 
	char	           * proc = "aux_cpy_SET_OF_EntryINFO";


	if(! QUIPUei){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_EntryINFO * )0);
	}

	ret = (SET_OF_EntryINFO * )malloc(sizeof(SET_OF_EntryINFO));

	ret->element = aux_cpy_EntryINFO(QUIPUei);
	ret->next = aux_cpy_SET_OF_EntryINFO(QUIPUei->ent_next);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_cpy_AttrAttrTypeCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

static AttrAttrTypeCHOICE *aux_cpy_AttrAttrTypeCHOICE(
	Attr_Sequence	  parm
)

#else

static AttrAttrTypeCHOICE *aux_cpy_AttrAttrTypeCHOICE(
	parm
)
Attr_Sequence	  parm;

#endif

{
	AttrAttrTypeCHOICE     * ret;
	PE 	       	         pe;
	char	    	       * proc = "aux_cpy_AttrAttrTypeCHOICE";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((AttrAttrTypeCHOICE * )0);
	}

	ret = (AttrAttrTypeCHOICE * )malloc(sizeof(AttrAttrTypeCHOICE));

	if(parm->attr_value == NULLAV){
		ret->offset = 1;
		ret->choice_un.choice_un_attrtype = aux_QUIPUAttributeType2SECUDEObjId (parm->attr_type);
	}
	else{
		ret->offset = 2;
		ret->choice_un.choice_un_attr = aux_QUIPUAttrSequence2SECUDESETOFAttr(parm);
	}

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_cpy_SET_OF_AttrAttrTypeCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_AttrAttrTypeCHOICE *aux_cpy_SET_OF_AttrAttrTypeCHOICE(
	Attr_Sequence	  parm
)

#else

static SET_OF_AttrAttrTypeCHOICE *aux_cpy_SET_OF_AttrAttrTypeCHOICE(
	parm
)
Attr_Sequence	  parm;

#endif

{
	SET_OF_AttrAttrTypeCHOICE     * ret;
	PE 	       			pe;
	char	    		      * proc = "aux_cpy_SET_OF_AttrAttrTypeCHOICE";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_AttrAttrTypeCHOICE * )0);
	}

	ret = (SET_OF_AttrAttrTypeCHOICE * )malloc(sizeof(SET_OF_AttrAttrTypeCHOICE));

	ret->element = aux_cpy_AttrAttrTypeCHOICE(parm);
	ret->next = aux_cpy_SET_OF_AttrAttrTypeCHOICE(parm->attr_link);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUdnseq2SECUDESETOFDName
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_DName *aux_QUIPUdnseq2SECUDESETOFDName(
	struct dn_seq	 *parm
)

#else

static SET_OF_DName *aux_QUIPUdnseq2SECUDESETOFDName(
	parm
)
struct dn_seq	 *parm;

#endif

{
	SET_OF_DName            * ret, * ret_tmp;
	struct dn_seq 		* parm_tmp;
	PE			  pe;
	char         	        * proc = "aux_QUIPUdnseq2SECUDESETOFDName";


	if(! parm || ! parm->dns_dn){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_DName * )0);
	}

	ret = (SET_OF_DName * )malloc(sizeof(SET_OF_DName));

	pe = dn_enc(parm->dns_dn);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &ret->element);
	pe_free(pe);

	ret->next = (SET_OF_DName *)0;

	for (ret_tmp = ret, parm_tmp = parm->dns_next; parm_tmp; parm_tmp = parm_tmp->dns_next) {
		ret_tmp->next = (SET_OF_DName * )malloc(sizeof(SET_OF_DName));
		ret_tmp = ret_tmp->next;
		pe = dn_enc(parm_tmp->dns_dn);
		parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &ret_tmp->element);
		pe_free(pe);
		ret_tmp->next = (SET_OF_DName *)0;
	} 

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUoidseq2SECUDEsetofobjid
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_ObjId *aux_QUIPUoidseq2SECUDEsetofobjid(
	struct oid_seq	 *parm
)

#else

static SET_OF_ObjId *aux_QUIPUoidseq2SECUDEsetofobjid(
	parm
)
struct oid_seq	 *parm;

#endif

{
	SET_OF_ObjId		* ret, * ret_tmp;
	struct oid_seq 		* parm_tmp;
	char         	        * proc = "aux_QUIPUoidseq2SECUDEsetofobjid";


	if(! parm || ! parm->oid_oid){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_ObjId * )0);
	}

	ret = (SET_OF_ObjId * )malloc(sizeof(SET_OF_ObjId));
	ret->element = aux_cpy_ObjId(parm->oid_oid);
	ret->next = (SET_OF_ObjId *)0;

	for (ret_tmp = ret, parm_tmp = parm->oid_next; parm_tmp; parm_tmp = parm_tmp->oid_next) {
		ret_tmp->next = (SET_OF_ObjId * )malloc(sizeof(SET_OF_ObjId));
		ret_tmp = ret_tmp->next;
		ret_tmp->element = aux_cpy_ObjId(parm_tmp->oid_oid);
		ret_tmp->next = (SET_OF_ObjId *)0;
	} 

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUaccpoint2SECUDEaccpoint
 *
 ***************************************************************/
#ifdef __STDC__

static AccessPoint *aux_QUIPUaccpoint2SECUDEaccpoint(
	struct access_point	 *parm
)

#else

static AccessPoint *aux_QUIPUaccpoint2SECUDEaccpoint(
	parm
)
struct access_point	 *parm;

#endif

{
	AccessPoint	* ret;
	PE	          pe;
	int	 	  rc;
	char		* proc = "aux_QUIPUaccpoint2SECUDEaccpoint";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((AccessPoint * )0);
	}

	ret = (AccessPoint * )malloc(sizeof(AccessPoint));

	pe = dn_enc(parm->ap_name);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &ret->ap_name);
	pe_free(pe);

	rc = enc_ipa(parm, &pe);
	ret->ap_address = PSAPaddr_dec(pe);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUaccpoint2SECUDESETOFaccpoint
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_AccessPoint *aux_QUIPUaccpoint2SECUDESETOFaccpoint(
	struct access_point	 *parm
)

#else

static SET_OF_AccessPoint *aux_QUIPUaccpoint2SECUDESETOFaccpoint(
	parm
)
struct access_point	 *parm;

#endif

{
	SET_OF_AccessPoint	* ret;
	char			* proc = "aux_QUIPUaccpoint2SECUDESETOFaccpoint";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_AccessPoint * )0);
	}

	ret = (SET_OF_AccessPoint * )malloc(sizeof(SET_OF_AccessPoint));

	ret->element = aux_QUIPUaccpoint2SECUDEaccpoint(parm);
	ret->next = aux_QUIPUaccpoint2SECUDESETOFaccpoint(parm->ap_next);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUop2SECUDEop
 *
 ***************************************************************/
#ifdef __STDC__

static OperationProgress *aux_QUIPUop2SECUDEop(
	struct op_progress	 *parm
)

#else

static OperationProgress *aux_QUIPUop2SECUDEop(
	parm
)
struct op_progress	 *parm;

#endif

{
	OperationProgress	* ret;
	char		  	* proc = "aux_QUIPUop2SECUDEop";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((OperationProgress * )0);
	}

	ret = (OperationProgress * )malloc(sizeof(OperationProgress));

	ret->opResolutionPhase = parm->op_resolution_phase;
	ret->opNextRDNToBeResolved = parm->op_nextrdntoberesolved;

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUcref2SECUDEcref
 *
 ***************************************************************/
#ifdef __STDC__

static ContReference *aux_QUIPUcref2SECUDEcref(
	ContinuationRef	  parm
)

#else

static ContReference *aux_QUIPUcref2SECUDEcref(
	parm
)
ContinuationRef	  parm;

#endif

{
	ContReference	* ret;
	PE		  pe;
	char            * proc = "aux_QUIPUcref2SECUDEcref";


	if(! parm || ! parm->cr_name){
		/* cr_name is MANDATORY */
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((ContReference * )0);
	}

	ret = (ContReference * )malloc(sizeof(ContReference));

	pe = dn_enc(parm->cr_name);
	parse_SECIF_Name(pe, 1, NULLIP, NULLVP, &ret->cr_name);
	pe_free(pe);

	ret->cr_progress = aux_QUIPUop2SECUDEop(&parm->cr_progress);
	ret->cr_rdn_resolved = parm->cr_rdn_resolved;
	ret->cr_aliasedRDNs = parm->cr_aliasedRDNs;
	ret->cr_reftype = parm->cr_reftype;
	ret->cr_accesspoints = aux_QUIPUaccpoint2SECUDESETOFaccpoint(parm->cr_accesspoints);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUcref2SECUDESETOFcref
 *
 ***************************************************************/
#ifdef __STDC__

static SET_OF_ContReference *aux_QUIPUcref2SECUDESETOFcref(
	ContinuationRef	  parm
)

#else

static SET_OF_ContReference *aux_QUIPUcref2SECUDESETOFcref(
	parm
)
ContinuationRef	  parm;

#endif

{
	SET_OF_ContReference	* ret;
	char         		* proc = "aux_QUIPUcref2SECUDESETOFcref";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((SET_OF_ContReference * )0);
	}

	ret = (SET_OF_ContReference * )malloc(sizeof(SET_OF_ContReference));

	ret->element = aux_QUIPUcref2SECUDEcref(parm);
	ret->next = aux_QUIPUcref2SECUDESETOFcref(parm->cr_next);

	return(ret);
}


/***************************************************************
 *
 * Procedure aux_QUIPUpoq2SECUDEpoq
 *
 ***************************************************************/
#ifdef __STDC__

static PartialOutQual *aux_QUIPUpoq2SECUDEpoq(
	POQ	 *parm
)

#else

static PartialOutQual *aux_QUIPUpoq2SECUDEpoq(
	parm
)
POQ	 *parm;

#endif

{
	PartialOutQual	* ret;
	char         	* proc = "aux_QUIPUpoq2SECUDEpoq";


	if(! parm){
		aux_add_error(EINVALID, "Parameters not present", CNULL, 0, proc);
		return((PartialOutQual * )0);
	}

	ret = (PartialOutQual * )malloc(sizeof(PartialOutQual));

	ret->poq_limitproblem = parm->poq_limitproblem;
	ret->poq_no_ext = parm->poq_no_ext;
	ret->poq_cref = aux_QUIPUcref2SECUDESETOFcref(parm->poq_cref);

	return(ret);
}




/****************************************************************************************/
/*											*/
/*											*/
/* 	I n t e r f a c e s   R o u t i n e s   t o   security-enhanced   Q U I P U 	*/
/*											*/
/*											*/
/****************************************************************************************/ 


/***************************************************************
 *
 * Procedure use_serv_secude
 *
 ***************************************************************/
#ifdef __STDC__

struct SecurityServices *use_serv_secude(
)

#else

struct SecurityServices *use_serv_secude(
)

#endif

{
	return (&serv_secude);
}

/******************************/


/***************************************************************
 *
 * Procedure secudesigned
 *
 ***************************************************************/
#ifdef __STDC__

struct signature *secudesigned(
	caddr_t	  arg,
	int	  type,
	AlgId    *algorithm
)

#else

struct signature *secudesigned(
	arg,
	type,
	algorithm
)
caddr_t	  arg;
int	  type;
AlgId    *algorithm;

#endif

{
	static AlgId		  * sig_alg = NULLALGID;
	Token	                  * tok;
	struct ds_addentry_arg    * ds_addarg;
	struct ds_bind_arg        * ds_bindarg;
	struct ds_compare_arg     * ds_comparearg;
	struct ds_compare_result  * ds_compareres;
	struct ds_list_arg        * ds_listarg;
	struct ds_list_result     * ds_listres;
	struct ds_modifyentry_arg * ds_modifyentryarg;
	struct ds_modifyrdn_arg   * ds_modifyrdnarg;
	struct ds_read_arg        * ds_readarg;
	struct ds_read_result     * ds_readres;
	struct ds_removeentry_arg * ds_removearg;
	struct ds_search_arg      * ds_searcharg;
	struct ds_search_result   * ds_searchres;
	AddArgument	          * addarg;
	CompareArgument		  * comparearg;
	CompareResult		  * compareres;
	ListArgument	          * listarg;
	ListResult		  * listres;
	ModifyEntryArgument       * modifyentryarg;
	ModifyRDNArgument	  * modifyrdnarg;
	ReadArgument	          * readarg;
	ReadResult		  * readres;
	RemoveArgument		  * removearg;
	SearchArgument            * searcharg;
	SearchResult              * searchres;
	struct signature          * ret;

	char	                  * proc = "secudesigned";


	if(! arg )
		return((struct signature * )0);

	if(! algorithm){
		if(! sig_alg){
			sig_alg = af_pse_get_signAI();
			if(! sig_alg){
				aux_add_error(EREADPSE, "af_pse_get_signAI failed", CNULL, 0, proc);
				return((struct signature * )0);
			}
			if(aux_ObjId2AlgType(sig_alg->objid) == ASYM_ENC )
				sig_alg = aux_cpy_AlgId(DEF_ISSUER_ALGID);
		}
	}
	else {
		if(! sig_alg || (sig_alg && aux_cmp_AlgId(sig_alg, algorithm))) {
			if(sig_alg) aux_free_AlgId(&sig_alg);
			sig_alg = aux_cpy_AlgId(algorithm);
		}
	}

	switch (type){

	case _ZTokenToSignDAS:
		ds_bindarg = (struct ds_bind_arg * )arg;

		tok = (Token * )malloc(sizeof(Token));
		if(! tok ){
			aux_add_error(EMALLOC, "tok", CNULL, 0, proc);
			return((struct signature * )0);
		}

		tok->tbs = aux_extract_TokenTBS_from_BindArg(ds_bindarg);
		if(! tok->tbs )
			return((struct signature * )0);

		if ((tok->tbs_DERcode = e_TokenTBS(tok->tbs))== NULLOCTETSTRING){
			aux_add_error(EENCODE, "e_TokenTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		tok->sig = (Signature * )malloc(sizeof(Signature));
		if (! tok->sig){
			aux_add_error(EMALLOC, "tok->sig", CNULL, 0, proc);
			return((struct signature * )0);
		}
		tok->sig->signAI = aux_cpy_AlgId(tok->tbs->signatureAI);
		if(af_sign(tok->tbs_DERcode, tok->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_Token (stderr, tok);

		ret = aux_SECUDEsign2QUIPUsign(tok->sig);
		aux_free_Token(&tok);

		break;

	case _ZAddEntryArgumentDataDAS:
		ds_addarg = (struct ds_addentry_arg * )arg;

		addarg = (AddArgument * )malloc(sizeof(AddArgument));
		if (! addarg) {
			aux_add_error(EMALLOC, "addarg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		addarg->tbs = aux_extract_AddArgumentTBS_from_AddArg(ds_addarg);
		if(! addarg->tbs )
			return((struct signature * )0);

		if ((addarg->tbs_DERcode = e_AddArgumentTBS(addarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_AddArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		addarg->sig = (Signature * )malloc(sizeof(Signature));
		if (! addarg->sig) {
			aux_add_error(EMALLOC, "addarg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		addarg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(addarg->tbs_DERcode, addarg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_AddArgument (stderr, addarg);

		ret = aux_SECUDEsign2QUIPUsign(addarg->sig);
		aux_free_AddArgument(&addarg);

		break;

	case _ZCompareArgumentDataDAS:
		ds_comparearg = (struct ds_compare_arg * )arg;

		comparearg = (CompareArgument * )malloc(sizeof(CompareArgument));
		if (! comparearg) {
			aux_add_error(EMALLOC, "comparearg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		comparearg->tbs = aux_extract_CompareArgumentTBS_from_CompareArg(ds_comparearg);
		if(! comparearg->tbs )
			return((struct signature * )0);

		if ((comparearg->tbs_DERcode = e_CompareArgumentTBS(comparearg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_CompareArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		comparearg->sig = (Signature * )malloc(sizeof(Signature));
		if (! comparearg->sig) {
			aux_add_error(EMALLOC, "comparearg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		comparearg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(comparearg->tbs_DERcode, comparearg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_CompareArgument (stderr, comparearg);

		ret = aux_SECUDEsign2QUIPUsign(comparearg->sig);
		aux_free_CompareArgument(&comparearg);

		break;

	case _ZCompareResultDataDAS:
		ds_compareres = (struct ds_compare_result * ) arg;

		compareres = (CompareResult * )malloc(sizeof(CompareResult));
		if (! compareres) {
			aux_add_error(EMALLOC, "compareres", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		compareres->tbs = aux_extract_CompareResultTBS_from_CompareRes(ds_compareres);
		if(! compareres->tbs )
			return((struct signature * )0);

		if ((compareres->tbs_DERcode = e_CompareResultTBS(compareres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_CompareResultTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		compareres->sig = (Signature * )malloc(sizeof(Signature));
		if (! compareres->sig) {
			aux_add_error(EMALLOC, "compareres->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		compareres->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(compareres->tbs_DERcode, compareres->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_CompareResult (stderr, compareres);

		ret = aux_SECUDEsign2QUIPUsign(compareres->sig);
		aux_free_CompareResult(&compareres);

		break;

	case _ZListArgumentDataDAS:
		ds_listarg = (struct ds_list_arg * )arg;

		listarg = (ListArgument * )malloc(sizeof(ListArgument));
		if (! listarg) {
			aux_add_error(EMALLOC, "listarg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		listarg->tbs = aux_extract_ListArgumentTBS_from_ListArg(ds_listarg);
		if(! listarg->tbs )
			return((struct signature * )0);

		if ((listarg->tbs_DERcode = e_ListArgumentTBS(listarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ListArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		listarg->sig = (Signature * )malloc(sizeof(Signature));
		if (!listarg->sig) {
			aux_add_error(EMALLOC, "listarg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		listarg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(listarg->tbs_DERcode, listarg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_ListArgument (stderr, listarg);

		ret = aux_SECUDEsign2QUIPUsign(listarg->sig);
		aux_free_ListArgument(&listarg);

		break;

	case _ZListResultDataDAS:
		ds_listres = (struct ds_list_result * ) arg;

		listres = (ListResult * )malloc(sizeof(ListResult));
		if (! listres) {
			aux_add_error(EMALLOC, "listres", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		listres->tbs = aux_extract_ListResultTBS_from_ListRes(ds_listres);
		if(! listres->tbs )
			return((struct signature * )0);

		if ((listres->tbs_DERcode = e_ListResultTBS(listres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ListResultTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		listres->sig = (Signature * )malloc(sizeof(Signature));
		if (!listres->sig) {
			aux_add_error(EMALLOC, "listres->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		listres->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(listres->tbs_DERcode, listres->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_ListResult (stderr, listres);

		ret = aux_SECUDEsign2QUIPUsign(listres->sig);
		aux_free_ListResult(&listres);

		break;

	case _ZModifyEntryArgumentDataDAS:
		ds_modifyentryarg = (struct ds_modifyentry_arg * ) arg;

		modifyentryarg = (ModifyEntryArgument * )malloc(sizeof(ModifyEntryArgument));
		if (! modifyentryarg) {
			aux_add_error(EMALLOC, "modifyentryarg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		modifyentryarg->tbs = aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg(ds_modifyentryarg);
		if(! modifyentryarg->tbs )
			return((struct signature * )0);

		if ((modifyentryarg->tbs_DERcode = e_ModifyEntryArgumentTBS(modifyentryarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ModifyEntryArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		modifyentryarg->sig = (Signature * )malloc(sizeof(Signature));
		if (! modifyentryarg->sig) {
			aux_add_error(EMALLOC, "modifyentryarg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		modifyentryarg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(modifyentryarg->tbs_DERcode, modifyentryarg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_ModifyEntryArgument (stderr, modifyentryarg);

		ret = aux_SECUDEsign2QUIPUsign(modifyentryarg->sig);
		aux_free_ModifyEntryArgument(&modifyentryarg);

		break;

	case _ZModifyRDNArgumentDataDAS:
		ds_modifyrdnarg = (struct ds_modifyrdn_arg * )arg;

		modifyrdnarg = (ModifyRDNArgument * )malloc(sizeof(ModifyRDNArgument));
		if (! modifyrdnarg) {
			aux_add_error(EMALLOC, "modifyrdnarg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		modifyrdnarg->tbs = aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg(ds_modifyrdnarg);
		if(! modifyrdnarg->tbs )
			return((struct signature * )0);

		if ((modifyrdnarg->tbs_DERcode = e_ModifyRDNArgumentTBS(modifyrdnarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ModifyRDNArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		modifyrdnarg->sig = (Signature * )malloc(sizeof(Signature));
		if (! modifyrdnarg->sig) {
			aux_add_error(EMALLOC, "modifyrdnarg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		modifyrdnarg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(modifyrdnarg->tbs_DERcode, modifyrdnarg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_ModifyRDNArgument (stderr, modifyrdnarg);

		ret = aux_SECUDEsign2QUIPUsign(modifyrdnarg->sig);
		aux_free_ModifyRDNArgument(&modifyrdnarg);

		break;

	case _ZReadArgumentDataDAS:
		ds_readarg = (struct ds_read_arg * )arg;

		readarg = (ReadArgument * )malloc(sizeof(ReadArgument));
		if (! readarg) {
			aux_add_error(EMALLOC, "readarg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		readarg->tbs = aux_extract_ReadArgumentTBS_from_ReadArg(ds_readarg);
		if(! readarg->tbs )
			return((struct signature * )0);

		if ((readarg->tbs_DERcode = e_ReadArgumentTBS(readarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ReadArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		readarg->sig = (Signature * )malloc(sizeof(Signature));
		if (! readarg->sig) {
			aux_add_error(EMALLOC, "readarg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		readarg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(readarg->tbs_DERcode, readarg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_ReadArgument (stderr, readarg);

		ret = aux_SECUDEsign2QUIPUsign(readarg->sig);
		aux_free_ReadArgument(&readarg);

		break;

	case _ZReadResultDataDAS:
		ds_readres = (struct ds_read_result * )arg;

		readres = (ReadResult * )malloc(sizeof(ReadResult));
		if (! readres) {
			aux_add_error(EMALLOC, "readres", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		readres->tbs = aux_extract_ReadResultTBS_from_ReadRes(ds_readres);
		if(! readres->tbs )
			return((struct signature * )0);

		if ((readres->tbs_DERcode = e_ReadResultTBS(readres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ReadResultTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		readres->sig = (Signature * )malloc(sizeof(Signature));
		if (! readres->sig) {
			aux_add_error(EMALLOC, "readres->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		readres->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(readres->tbs_DERcode, readres->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_ReadResult (stderr, readres);

		ret = aux_SECUDEsign2QUIPUsign(readres->sig);
		aux_free_ReadResult(&readres);

		break;

	case _ZRemoveEntryArgumentDataDAS:
		ds_removearg = (struct ds_removeentry_arg * )arg;

		removearg = (RemoveArgument * )malloc(sizeof(RemoveArgument));
		if (! removearg) {
			aux_add_error(EMALLOC, "removearg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		removearg->tbs = aux_extract_RemoveArgumentTBS_from_RemoveArg(ds_removearg);
		if(! removearg->tbs )
			return((struct signature * )0);

		if ((removearg->tbs_DERcode = e_RemoveArgumentTBS(removearg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_RemoveArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		removearg->sig = (Signature * )malloc(sizeof(Signature));
		if (! removearg->sig) {
			aux_add_error(EMALLOC, "removearg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		removearg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(removearg->tbs_DERcode, removearg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_RemoveArgument(stderr, removearg);

		ret = aux_SECUDEsign2QUIPUsign(removearg->sig);
		aux_free_RemoveArgument(&removearg);

		break;

	case _ZSearchArgumentDataDAS:
		ds_searcharg = (struct ds_search_arg * )arg;

		searcharg = (SearchArgument * )malloc(sizeof(SearchArgument));
		if (! searcharg) {
			aux_add_error(EMALLOC, "searcharg", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		searcharg->tbs = aux_extract_SearchArgumentTBS_from_SearchArg(ds_searcharg);
		if(! searcharg->tbs )
			return((struct signature * )0);

		if ((searcharg->tbs_DERcode = e_SearchArgumentTBS(searcharg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_SearchArgumentTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		searcharg->sig = (Signature * )malloc(sizeof(Signature));
		if (! searcharg->sig) {
			aux_add_error(EMALLOC, "searcharg->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		searcharg->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(searcharg->tbs_DERcode, searcharg->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_SearchArgument(stderr, searcharg);

		ret = aux_SECUDEsign2QUIPUsign(searcharg->sig);
		aux_free_SearchArgument(&searcharg);

		break;

	case _ZSearchResultDataDAS:
		ds_searchres = (struct ds_search_result * )arg;

		searchres = (SearchResult * )malloc(sizeof(SearchResult));
		if (! searchres) {
			aux_add_error(EMALLOC, "searchres", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		searchres->tbs = aux_extract_SearchResultTBS_from_SearchRes(ds_searchres);
		if(! searchres->tbs )
			return((struct signature * )0);

		if ((searchres->tbs_DERcode = e_SearchResultTBS(searchres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_SearchResultTBS failed", CNULL, 0, proc);
			return((struct signature * )0);
		}

		searchres->sig = (Signature * )malloc(sizeof(Signature));
		if (! searchres->sig) {
			aux_add_error(EMALLOC, "searchres->sig", CNULL, 0, proc);
			return((struct signature * ) 0);
		}

		searchres->sig->signAI = aux_cpy_AlgId(sig_alg);

		if(af_sign(searchres->tbs_DERcode, searchres->sig, SEC_END) < 0 ){
			aux_add_error(ESIGN, "Cannot sign", CNULL, 0, proc);
			return((struct signature * )0);
		}

		if(af_verbose) strong_fprint_SearchResult (stderr, searchres);

		ret = aux_SECUDEsign2QUIPUsign(searchres->sig);
		aux_free_SearchResult(&searchres);

		break;

	}  /* switch */


	return (ret);
}


/***************************************************************
 *
 * Procedure secudeverify
 *
 ***************************************************************/
#ifdef __STDC__

int secudeverify(
)

#else

int secudeverify(
)

#endif

{



}


/***************************************************************
 *
 * Procedure secude_ckpath
 *
 ***************************************************************/
#ifdef __STDC__

int secude_ckpath(
	caddr_t			  arg,
	struct certificate_list	 *quipu_cpath,
	struct signature	 *quipu_sig,
	DN			 *nameptr,
	int			  type
)

#else

int secude_ckpath(
	arg,
	quipu_cpath,
	quipu_sig,
	nameptr,
	type
)
caddr_t			  arg;
struct certificate_list	 *quipu_cpath;
struct signature	 *quipu_sig;
DN			 *nameptr;
int			  type;

#endif

{

	Certificates              * or_cert;
	CertificationPath         * SECUDEcpath = (CertificationPath * )0;
	Token 	                  * tok;
	PE                          pe;
	int			    rc;
	struct ds_addentry_arg    * ds_addarg;
	struct ds_bind_arg        * ds_bindarg;
	struct ds_compare_arg     * ds_comparearg;
	struct ds_compare_result  * ds_compareres;
	struct ds_list_arg        * ds_listarg;
	struct ds_list_result     * ds_listres;
	struct ds_modifyentry_arg * ds_modifyentryarg;
	struct ds_modifyrdn_arg   * ds_modifyrdnarg;
	struct ds_read_arg        * ds_readarg;
	struct ds_read_result     * ds_readres;
	struct ds_removeentry_arg * ds_removearg;
	struct ds_search_arg      * ds_searcharg;
	struct ds_search_result   * ds_searchres;
	AddArgument	          * addarg;
	CompareArgument		  * comparearg;
	CompareResult		  * compareres;
	ListArgument	          * listarg;
	ListResult		  * listres;
	ModifyEntryArgument       * modifyentryarg;
	ModifyRDNArgument	  * modifyrdnarg;
	ReadArgument	          * readarg;
	ReadResult		  * readres;
	RemoveArgument		  * removearg;
	SearchArgument            * searcharg;
	SearchResult              * searchres;

/***/
PS rps;
/***/
	char	           * proc = "secude_ckpath";

/***/
rps = ps_alloc(std_open);
std_setup(rps, stdout);
/***/


	if(! arg || ! quipu_cpath || ! quipu_sig )
		return(- 1);

	switch (type) {

	case _ZTokenToSignDAS:
		ds_bindarg = (struct ds_bind_arg * )arg;

		tok = (Token * )malloc(sizeof(Token));
		if(! tok ) {
			aux_add_error(EMALLOC, "tok", CNULL, 0, proc);
			return(- 1);
		}

		tok->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! tok->sig )
			return(- 1);

		tok->tbs = aux_extract_TokenTBS_from_BindArg(ds_bindarg);
		if(! tok->tbs )
			return(- 1);

		if ((tok->tbs_DERcode = e_TokenTBS(tok->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_TokenTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_Token(stderr, tok);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (tok->tbs_DERcode, tok->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);
		
		aux_free_Token(&tok);

		break;

	case _ZAddEntryArgumentDataDAS:
		ds_addarg = (struct ds_addentry_arg * )arg;

		addarg = (AddArgument * )malloc(sizeof(AddArgument));
		if(! addarg ) {
			aux_add_error(EMALLOC, "addarg", CNULL, 0, proc);
			return(- 1);
		}

		addarg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! addarg->sig )
			return(- 1);

		addarg->tbs = aux_extract_AddArgumentTBS_from_AddArg(ds_addarg);
		if(! addarg->tbs )
			return(- 1);

		if ((addarg->tbs_DERcode = e_AddArgumentTBS(addarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_AddArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_AddArgument(stderr, addarg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (addarg->tbs_DERcode, addarg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_AddArgument(&addarg);

		break;

	case _ZCompareArgumentDataDAS:
		ds_comparearg = (struct ds_compare_arg * )arg;

		comparearg = (CompareArgument * )malloc(sizeof(CompareArgument));
		if(! comparearg ) {
			aux_add_error(EMALLOC, "comparearg", CNULL, 0, proc);
			return(- 1);
		}

		comparearg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! comparearg->sig )
			return(- 1);

		comparearg->tbs = aux_extract_CompareArgumentTBS_from_CompareArg(ds_comparearg);
		if(! comparearg->tbs )
			return(- 1);

		if ((comparearg->tbs_DERcode = e_CompareArgumentTBS(comparearg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_CompareArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_CompareArgument(stderr, comparearg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (comparearg->tbs_DERcode, comparearg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_CompareArgument(&comparearg);

		break;

	case _ZCompareResultDataDAS:
		ds_compareres = (struct ds_compare_result * )arg;

		compareres = (CompareResult * )malloc(sizeof(CompareResult));
		if(! compareres ) {
			aux_add_error(EMALLOC, "compareres", CNULL, 0, proc);
			return(- 1);
		}

		compareres->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! compareres->sig )
			return(- 1);

		compareres->tbs = aux_extract_CompareResultTBS_from_CompareRes(ds_compareres);
		if(! compareres->tbs )
			return(- 1);

		if ((compareres->tbs_DERcode = e_CompareResultTBS(compareres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_CompareResultTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_CompareResult(stderr, compareres);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (compareres->tbs_DERcode, compareres->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_CompareResult(&compareres);

		break;

	case _ZListArgumentDataDAS:
		ds_listarg = (struct ds_list_arg * )arg;

		listarg = (ListArgument * )malloc(sizeof(ListArgument));
		if(! listarg ) {
			aux_add_error(EMALLOC, "listarg", CNULL, 0, proc);
			return(- 1);
		}

		listarg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! listarg->sig )
			return(- 1);

		listarg->tbs = aux_extract_ListArgumentTBS_from_ListArg(ds_listarg);
		if(!listarg->tbs )
			return(- 1);

		if ((listarg->tbs_DERcode = e_ListArgumentTBS(listarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ListArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_ListArgument(stderr, listarg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (listarg->tbs_DERcode, listarg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_ListArgument(&listarg);

		break;

	case _ZListResultDataDAS:
		ds_listres = (struct ds_list_result * )arg;

		listres = (ListResult * )malloc(sizeof(ListResult));
		if(!listres ) {
			aux_add_error(EMALLOC, "listres", CNULL, 0, proc);
			return(- 1);
		}

		listres->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! listres->sig )
			return(- 1);

		listres->tbs = aux_extract_ListResultTBS_from_ListRes(ds_listres);
		if(! listres->tbs )
			return(- 1);

		if ((listres->tbs_DERcode = e_ListResultTBS(listres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ListResultTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_ListResult(stderr, listres);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (listres->tbs_DERcode, listres->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_ListResult(&listres);

		break;

	case _ZModifyEntryArgumentDataDAS:
		ds_modifyentryarg = (struct ds_modifyentry_arg * )arg;

		modifyentryarg = (ModifyEntryArgument * )malloc(sizeof(ModifyEntryArgument));
		if(! modifyentryarg ) {
			aux_add_error(EMALLOC, "modifyentryarg", CNULL, 0, proc);
			return(- 1);
		}

		modifyentryarg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! modifyentryarg->sig )
			return(- 1);

		modifyentryarg->tbs = aux_extract_ModifyEntryArgumentTBS_from_ModifyEntryArg(ds_modifyentryarg);
		if(! modifyentryarg->tbs )
			return(- 1);

		if ((modifyentryarg->tbs_DERcode = e_ModifyEntryArgumentTBS(modifyentryarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ModifyEntryArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_ModifyEntryArgument(stderr, modifyentryarg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (modifyentryarg->tbs_DERcode, modifyentryarg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_ModifyEntryArgument(&modifyentryarg);

		break;

	case _ZModifyRDNArgumentDataDAS:
		ds_modifyrdnarg = (struct ds_modifyrdn_arg * )arg;

		modifyrdnarg = (ModifyRDNArgument * )malloc(sizeof(ModifyRDNArgument));
		if(! modifyrdnarg ) {
			aux_add_error(EMALLOC, "modifyrdnarg", CNULL, 0, proc);
			return(- 1);
		}

		modifyrdnarg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! modifyrdnarg->sig )
			return(- 1);

		modifyrdnarg->tbs = aux_extract_ModifyRDNArgumentTBS_from_ModifyRDNArg(ds_modifyrdnarg);
		if(! modifyrdnarg->tbs )
			return(- 1);

		if ((modifyrdnarg->tbs_DERcode = e_ModifyRDNArgumentTBS(modifyrdnarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ModifyRDNArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_ModifyRDNArgument(stderr, modifyrdnarg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (modifyrdnarg->tbs_DERcode, modifyrdnarg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_ModifyRDNArgument(&modifyrdnarg);

		break;

	case _ZReadArgumentDataDAS:
		ds_readarg = (struct ds_read_arg * )arg;

		readarg = (ReadArgument * )malloc(sizeof(ReadArgument));
		if(! readarg ) {
			aux_add_error(EMALLOC, "readarg", CNULL, 0, proc);
			return(- 1);
		}

		readarg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! readarg->sig )
			return(- 1);

		readarg->tbs = aux_extract_ReadArgumentTBS_from_ReadArg(ds_readarg);
		if(! readarg->tbs )
			return(- 1);

		if ((readarg->tbs_DERcode = e_ReadArgumentTBS(readarg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ReadArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_ReadArgument(stderr, readarg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify(readarg->tbs_DERcode, readarg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_ReadArgument(&readarg);

		break;

	case _ZReadResultDataDAS:
		ds_readres = (struct ds_read_result * )arg;

		readres = (ReadResult * )malloc(sizeof(ReadResult));
		if(! readres ) {
			aux_add_error(EMALLOC, "readres", CNULL, 0, proc);
			return(- 1);
		}

		readres->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! readres->sig )
			return(- 1);

		readres->tbs = aux_extract_ReadResultTBS_from_ReadRes(ds_readres);
		if(! readres->tbs )
			return(- 1);

		if ((readres->tbs_DERcode = e_ReadResultTBS(readres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_ReadResultTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_ReadResult(stderr, readres);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify(readres->tbs_DERcode, readres->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_ReadResult(&readres);

		break;

	case _ZRemoveEntryArgumentDataDAS:
		ds_removearg = (struct ds_removeentry_arg * )arg;

		removearg = (RemoveArgument * )malloc(sizeof(RemoveArgument));
		if(! removearg ) {
			aux_add_error(EMALLOC, "removearg", CNULL, 0, proc);
			return(- 1);
		}

		removearg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! removearg->sig )
			return(- 1);

		removearg->tbs = aux_extract_RemoveArgumentTBS_from_RemoveArg(ds_removearg);
		if(! removearg->tbs )
			return(- 1);

		if ((removearg->tbs_DERcode = e_RemoveArgumentTBS(removearg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_RemoveArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_RemoveArgument(stderr, removearg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify (removearg->tbs_DERcode, removearg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_RemoveArgument(&removearg);

		break;

	case _ZSearchArgumentDataDAS:
		ds_searcharg = (struct ds_search_arg * )arg;

		searcharg = (SearchArgument * )malloc(sizeof(SearchArgument));
		if(! searcharg ) {
			aux_add_error(EMALLOC, "searcharg", CNULL, 0, proc);
			return(- 1);
		}

		searcharg->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! searcharg->sig )
			return(- 1);

		searcharg->tbs = aux_extract_SearchArgumentTBS_from_SearchArg(ds_searcharg);
		if(! searcharg->tbs )
			return(- 1);

		if ((searcharg->tbs_DERcode = e_SearchArgumentTBS(searcharg->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_SearchArgumentTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_SearchArgument(stderr, searcharg);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify(searcharg->tbs_DERcode, searcharg->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_SearchArgument(&searcharg);

		break;

	case _ZSearchResultDataDAS:
		ds_searchres = (struct ds_search_result * )arg;

		searchres = (SearchResult * )malloc(sizeof(SearchResult));
		if(! searchres ) {
			aux_add_error(EMALLOC, "searchres", CNULL, 0, proc);
			return(- 1);
		}

		searchres->sig = aux_QUIPUsign2SECUDEsign(quipu_sig);
		if(! searchres->sig )
			return(- 1);

		searchres->tbs = aux_extract_SearchResultTBS_from_SearchRes(ds_searchres);
		if(! searchres->tbs )
			return(- 1);

		if ((searchres->tbs_DERcode = e_SearchResultTBS(searchres->tbs)) == NULLOCTETSTRING) {
			aux_add_error(EENCODE, "e_SearchResultTBS failed", CNULL, 0, proc);
			return(- 1);
		}

		if(af_verbose) strong_fprint_SearchResult(stderr, searchres);

		or_cert = aux_QUIPUcertlist2SECUDEocert(quipu_cpath);
		if(! or_cert )
			return(- 1);

		rc = af_verify(searchres->tbs_DERcode, searchres->sig, SEC_END, or_cert, CNULL, (PKRoot * )0);
		if(af_verbose || rc < 0) aux_fprint_VerificationResult(stderr, verifresult);
		aux_free_VerificationResult(&verifresult);

		aux_free_SearchResult(&searchres);

		break;

	}  /* switch */

	build_SECIF_Name(&pe, 1, 0, NULLCP, or_cert->usercertificate->tbs->subject);
	* nameptr = dn_dec(pe);
	if (pe)
		pe_free(pe);

	aux_free_Certificates(&or_cert);

	return(rc);
}



/***************************************************************
 *
 * Procedure secude_mkpath
 *
 ***************************************************************/
#ifdef __STDC__

struct certificate_list *secude_mkpath(
)

#else

struct certificate_list *secude_mkpath(
)

#endif

{
	Certificates   * or_cert;
	char	       * proc = "secude_mkpath";


	if(! certlist){
		or_cert = af_pse_get_Certificates(SIGNATURE, NULLDNAME);
		certlist = aux_SECUDEocert2QUIPUcertlist(or_cert);
	}

	return (certlist);
}


/***************************************************************
 *
 * Procedure secudeencrypted
 *
 ***************************************************************/
#ifdef __STDC__

struct encrypted *secudeencrypted(
)

#else

struct encrypted *secudeencrypted(
)

#endif

{

}



/***************************************************************
 *
 * Procedure secudedecrypted
 *
 ***************************************************************/
#ifdef __STDC__

int secudedecrypted(
)

#else

int secudedecrypted(
)

#endif

{

}



/***************************************************************
 *
 * Procedure secudemknonce
 *
 ***************************************************************/
#ifdef __STDC__

struct Nonce *secudemknonce(
	AlgId *algorithm
)

#else

struct Nonce *secudemknonce(
	algorithm
)
AlgId * algorithm;

#endif

{
	static AlgId   * sig_alg = NULLALGID;
	struct Nonce   * ret;
	BitString      * random_bstr;
	struct alg_id  * quipu_alg;
	PE	         pe;
	int              i, nob, result;
	
	char	       * proc = "secudemknonce";


	if(! algorithm){
		if(! sig_alg){
			sig_alg = af_pse_get_signAI();
			if(! sig_alg){
				aux_add_error(EREADPSE, "af_pse_get_signAI failed", CNULL, 0, proc);
				return((struct Nonce * )0);
			}
			if(aux_ObjId2AlgType(sig_alg->objid) == ASYM_ENC )
				sig_alg = aux_cpy_AlgId(DEF_ISSUER_ALGID);
		}
	}
	else {
		if(! sig_alg || (sig_alg && aux_cmp_AlgId(sig_alg, algorithm))) {
			if(sig_alg) aux_free_AlgId(&sig_alg);
			sig_alg = aux_cpy_AlgId(algorithm);
		}
	}

	if((ret = (struct Nonce * )malloc(sizeof(struct Nonce))) == (struct Nonce * )0) {
		aux_add_error(EMALLOC, "ret", CNULL, 0, proc);
		return((struct Nonce * )0);
	}

	random_bstr = sec_random_bstr(64);

	ret->non_r1.n_bits = random_bstr->nbits;
	nob = ret->non_r1.n_bits / 8;
	if(ret->non_r1.n_bits % 8 )
		nob++;
	if((ret->non_r1.value = (char *)malloc(nob)) == (char * )0 ) {
		aux_add_error(EMALLOC, "ret->non_r1.value", CNULL, 0, proc);
		return((struct Nonce * )0);
	}
	for(i = 0; i < nob; i++) {
		ret->non_r1.value[i] = random_bstr->bits[i];
	}
	aux_free_BitString(&random_bstr);

	ret->non_r2.n_bits = 0;
	ret->non_r2.value = CNULL;

	ret->non_time1 = get_date_of_expiry();
	ret->non_time2 = CNULL;

	pe = AlgId_enc(sig_alg);
	result = decode_AF_AlgorithmIdentifier (pe, 0, NULLIP, NULLVP, &quipu_alg);
	pe_free(pe);
	if (result == NOTOK) {
		aux_add_error(EDECODE, "ret", CNULL, 0, proc);
		return((struct Nonce * )0);
	}
	alg_cpy(&(ret->non_alg), quipu_alg);

	return (ret);

}


/***************************************************************
 *
 * Procedure secudecknonce
 *
 ***************************************************************/
#ifdef __STDC__

int secudecknonce(
	struct Nonce	 *nonce
)

#else

int secudecknonce(
	nonce
)
struct Nonce	 *nonce;

#endif

{
	char	     * proc = "secudecknonce";

	return 0;
}




#else

/* dummy */
/***************************************************************
 *
 * Procedure strong_int_dummy
 *
 ***************************************************************/

int strong_int_dummy()

{
	return(0);
}

#endif
