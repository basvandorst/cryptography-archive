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


#if defined(X500) && defined(STRONG)

#include "secude-stub.h"

/********************** IF Section **********************/

/***************************************************************
 *
 * Procedure aux_free2_Attr
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_Attr(
	register Attr	 *attr
)

#else

void aux_free2_Attr(
	attr
)
register Attr	 *attr;

#endif

{
	char	 * proc = "aux_free2_Attr";

	if(attr){
		aux_free_ObjId(((ObjId **)& (attr->type)) );
		if(attr->values)
			aux_free_AttrValues(&(attr->values));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_Attr
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_Attr(
	Attr	**attr
)

#else

void aux_free_Attr(
	attr
)
Attr	**attr;

#endif

{
	char	 * proc = "aux_free_Attr";

	if(attr && * attr){
		aux_free2_Attr(* attr);
		free(* attr);
		* attr = (Attr * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_AttrValues
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_AttrValues(
	AttrValues	**set
)

#else

void aux_free_AttrValues(
	set
)
AttrValues	**set;

#endif

{
	register AttrValues * eptr;
	register AttrValues * next;

	char * proc = "aux_free_AttrValues";

	if(set && * set){
		for(eptr = * set; eptr != (AttrValues * )0; eptr = next){
			next = eptr->next;
			if(eptr->member_IF_1) pe_free(eptr->member_IF_1);
			free((char * )eptr);
			eptr = (AttrValues * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_Attr
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_Attr(
	SET_OF_Attr	**set
)

#else

void aux_free_SET_OF_Attr(
	set
)
SET_OF_Attr	**set;

#endif

{
	register SET_OF_Attr * eptr;
	register SET_OF_Attr * next;

	char * proc = "aux_free_SET_OF_Attr";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_Attr * )0; eptr = next){
			next = eptr->next;
			aux_free_Attr(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_Attr * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_AttrAttrTypeCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_AttrAttrTypeCHOICE(
	register AttrAttrTypeCHOICE	 *choice
)

#else

void aux_free2_AttrAttrTypeCHOICE(
	choice
)
register AttrAttrTypeCHOICE	 *choice;

#endif

{
	char	 * proc = "aux_free2_AttrAttrTypeCHOICE";

	if(choice){
		if(choice->offset == 1)
			aux_free_ObjId(&(choice->choice_un.choice_un_attrtype));
		else aux_free_SET_OF_Attr(&(choice->choice_un.choice_un_attr));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_AttrAttrTypeCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_AttrAttrTypeCHOICE(
	AttrAttrTypeCHOICE	**choice
)

#else

void aux_free_AttrAttrTypeCHOICE(
	choice
)
AttrAttrTypeCHOICE	**choice;

#endif

{
	char	 * proc = "aux_free_AttrAttrTypeCHOICE";

	if(choice && * choice){
		aux_free2_AttrAttrTypeCHOICE(* choice);
		free(* choice);
		* choice = (AttrAttrTypeCHOICE * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_AttrAttrTypeCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_AttrAttrTypeCHOICE(
	SET_OF_AttrAttrTypeCHOICE	**set
)

#else

void aux_free_SET_OF_AttrAttrTypeCHOICE(
	set
)
SET_OF_AttrAttrTypeCHOICE	**set;

#endif

{
	register SET_OF_AttrAttrTypeCHOICE * eptr;
	register SET_OF_AttrAttrTypeCHOICE * next;

	char * proc = "aux_free_SET_OF_AttrAttrTypeCHOICE";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_AttrAttrTypeCHOICE * )0; eptr = next){
			next = eptr->next;
			aux_free_AttrAttrTypeCHOICE(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_AttrAttrTypeCHOICE * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_Ava
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_Ava(
	register AttrValueAssertion	 *ava
)

#else

void aux_free2_Ava(
	ava
)
register AttrValueAssertion	 *ava;

#endif

{
	char	 * proc = "aux_free2_Ava";

	if(ava){
		if(ava->element_IF_0)
			aux_free_ObjId(((ObjId ** )&(ava->element_IF_0)));
		if(ava->element_IF_1)
			pe_free(ava->element_IF_1);
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_Ava
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_Ava(
	AttrValueAssertion	**ava
)

#else

void aux_free_Ava(
	ava
)
AttrValueAssertion	**ava;

#endif

{
	char	 * proc = "aux_free_Ava";

	if(ava && * ava){
		aux_free2_Ava(* ava);
		free(* ava);
		* ava = (AttrValueAssertion * )0;
	}
	return;
}


/******************* END of IF Section ******************/



/***************************************************************
 *
 * Procedure aux_free2_EntryINFO
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_EntryINFO(
	register EntryINFO	 *ent
)

#else

void aux_free2_EntryINFO(
	ent
)
register EntryINFO	 *ent;

#endif

{
	char	 * proc = "aux_free2_EntryINFO";

	if(ent){
		if(ent->ent_dn)
			aux_free_DName(&(ent->ent_dn));
		if(ent->ent_attr)
			aux_free_SET_OF_AttrAttrTypeCHOICE(&(ent->ent_attr));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_EntryINFO
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_EntryINFO(
	EntryINFO	**ent
)

#else

void aux_free_EntryINFO(
	ent
)
EntryINFO	**ent;

#endif

{
	char	 * proc = "aux_free_EntryINFO";

	if(ent && * ent){
		aux_free2_EntryINFO(* ent);
		free(* ent);
		* ent = (EntryINFO * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_EntryINFO
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_EntryINFO(
	SET_OF_EntryINFO	**set
)

#else

void aux_free_SET_OF_EntryINFO(
	set
)
SET_OF_EntryINFO	**set;

#endif

{
	register SET_OF_EntryINFO * eptr;
	register SET_OF_EntryINFO * next;

	char * proc = "aux_free_SET_OF_EntryINFO";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_EntryINFO * )0; eptr = next){
			next = eptr->next;
			aux_free_EntryINFO(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_EntryINFO * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_ObjId
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_ObjId(
	SET_OF_ObjId	**set
)

#else

void aux_free_SET_OF_ObjId(
	set
)
SET_OF_ObjId	**set;

#endif

{
	register SET_OF_ObjId * eptr;
	register SET_OF_ObjId * next;

	char * proc = "aux_free_SET_OF_ObjId";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_ObjId * )0; eptr = next){
			next = eptr->next;
			aux_free_ObjId(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_ObjId * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_EntryInfoSEL
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_EntryInfoSEL(
	register EntryInfoSEL	 *eis
)

#else

void aux_free2_EntryInfoSEL(
	eis
)
register EntryInfoSEL	 *eis;

#endif

{
	char	 * proc = "aux_free2_EntryInfoSEL";

	if(eis){
		if(eis->eis_select)
			aux_free_SET_OF_ObjId(&(eis->eis_select));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_EntryInfoSEL
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_EntryInfoSEL(
	EntryInfoSEL	**eis
)

#else

void aux_free_EntryInfoSEL(
	eis
)
EntryInfoSEL	**eis;

#endif

{
	char	 * proc = "aux_free_EntryInfoSEL";

	if(eis && * eis){
		aux_free2_EntryInfoSEL(* eis);
		free(* eis);
		* eis = (EntryInfoSEL * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SecurityParameters
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SecurityParameters(
	register SecurityParameters	 *sp
)

#else

void aux_free2_SecurityParameters(
	sp
)
register SecurityParameters	 *sp;

#endif

{
	char	 * proc = "aux_free2_SecurityParameters";

	if(sp){
		if(sp->certPath)
			aux_free_CertificationPath(&(sp->certPath));
		if(sp->name)
			aux_free_DName(&(sp->name));
		if(sp->time) 
			free(sp->time);
		if(sp->random) 
			aux_free_BitString(&(sp->random));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SecurityParameters
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SecurityParameters(
	SecurityParameters	**sp
)

#else

void aux_free_SecurityParameters(
	sp
)
SecurityParameters	**sp;

#endif

{
	char	 * proc = "aux_free_SecurityParameters";

	if(sp && * sp){
		aux_free2_SecurityParameters(* sp);
		free(* sp);
		* sp = (SecurityParameters * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ServiceControls
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ServiceControls(
	register ServiceControls	 *sc
)

#else

void aux_free2_ServiceControls(
	sc
)
register ServiceControls	 *sc;

#endif

{
	char	 * proc = "aux_free2_ServiceControls";

	if(sc){
		if(sc->svc_tmp){
			free(sc->svc_tmp);
			sc->svc_tmp = CNULL;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ServiceControls
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ServiceControls(
	ServiceControls	**sc
)

#else

void aux_free_ServiceControls(
	sc
)
ServiceControls	**sc;

#endif

{
	char	 * proc = "aux_free_ServiceControls";

	if(sc && * sc){
		aux_free2_ServiceControls(* sc);
		free(* sc);
		* sc = (ServiceControls * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_CommonArguments
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_CommonArguments(
	register CommonArguments	 *ca
)

#else

void aux_free2_CommonArguments(
	ca
)
register CommonArguments	 *ca;

#endif

{
	char	 * proc = "aux_free2_CommonArguments";

	if(ca){
		if(ca->svc)
			aux_free_ServiceControls(&(ca->svc));
		if(ca->sec_parm)
			aux_free_SecurityParameters(&(ca->sec_parm));
		if(ca->requestor) 
			aux_free_DName(&(ca->requestor));
		if(ca->progress){
			free(ca->progress);
			ca->progress = (OperationProgress * )0;
		}
		if(ca->ext) 
			aux_free_SET_OF_SECExtension(&(ca->ext));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_CommonArguments
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_CommonArguments(
	CommonArguments	**ca
)

#else

void aux_free_CommonArguments(
	ca
)
CommonArguments	**ca;

#endif

{
	char	 * proc = "aux_free_CommonArguments";

	if(ca && * ca){
		aux_free2_CommonArguments(* ca);
		free(* ca);
		* ca = (CommonArguments * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_CommonRes
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_CommonRes(
	register CommonRes	 *cr
)

#else

void aux_free2_CommonRes(
	cr
)
register CommonRes	 *cr;

#endif

{
	char	 * proc = "aux_free2_CommonRes";

	if(cr){
		if(cr->sec_parm)
			aux_free_SecurityParameters(&(cr->sec_parm));
		if(cr->performer) 
			aux_free_DName(&(cr->performer));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_CommonRes
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_CommonRes(
	CommonRes	**cr
)

#else

void aux_free_CommonRes(
	cr
)
CommonRes	**cr;

#endif

{
	char	 * proc = "aux_free_CommonRes";

	if(cr && * cr){
		aux_free2_CommonRes(* cr);
		free(* cr);
		* cr = (CommonRes * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SECExtension
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SECExtension(
	register SECExtension	 *secext
)

#else

void aux_free2_SECExtension(
	secext
)
register SECExtension	 *secext;

#endif

{
	char	 * proc = "aux_free2_SECExtension";

	if(secext){
		if(secext->ext_item) 
			pe_free(secext->ext_item);
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SECExtension
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SECExtension(
	SECExtension	**secext
)

#else

void aux_free_SECExtension(
	secext
)
SECExtension	**secext;

#endif

{
	char	 * proc = "aux_free_SECExtension";

	if(secext && * secext){
		aux_free2_SECExtension(* secext);
		free(* secext);
		* secext = (SECExtension * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_SECExtension
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_SECExtension(
	SET_OF_SECExtension	**set
)

#else

void aux_free_SET_OF_SECExtension(
	set
)
SET_OF_SECExtension	**set;

#endif

{
	register SET_OF_SECExtension * eptr;
	register SET_OF_SECExtension * next;
	char	                     * proc = "aux_free_SET_OF_SECExtension";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_SECExtension * )0; eptr = next){
			next = eptr->next;
			aux_free_SECExtension(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_SECExtension * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_TokenTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_TokenTBS(
	register TokenTBS	 *tbs
)

#else

void aux_free2_TokenTBS(
	tbs
)
register TokenTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_TokenTBS";

	if(tbs){
		aux_free_AlgId(&(tbs->signatureAI));
		if(tbs->dname) 
			aux_free_DName(&(tbs->dname));
		if(tbs->time) 
			free(tbs->time);
		if(tbs->random)
			aux_free_BitString(&(tbs->random));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_TokenTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_TokenTBS(
	TokenTBS	**tbs
)

#else

void aux_free_TokenTBS(
	tbs
)
TokenTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_TokenTBS";

	if(tbs && * tbs){
		aux_free2_TokenTBS(* tbs);
		free(* tbs);
		* tbs = (TokenTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_AddArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_AddArgumentTBS(
	register AddArgumentTBS	 *tbs
)

#else

void aux_free2_AddArgumentTBS(
	tbs
)
register AddArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_AddArgumentTBS";

	if(tbs){
		if(tbs->ada_object) 
			aux_free_DName(&(tbs->ada_object));
		if(tbs->ada_entry) 
			aux_free_SET_OF_Attr(&(tbs->ada_entry));
		if(tbs->ada_common) 
			aux_free_CommonArguments(&(tbs->ada_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_AddArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_AddArgumentTBS(
	AddArgumentTBS	**tbs
)

#else

void aux_free_AddArgumentTBS(
	tbs
)
AddArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_AddArgumentTBS";

	if(tbs && * tbs){
		aux_free2_AddArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (AddArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_CompareArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_CompareArgumentTBS(
	register CompareArgumentTBS	 *tbs
)

#else

void aux_free2_CompareArgumentTBS(
	tbs
)
register CompareArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_CompareArgumentTBS";

	if(tbs){
		if(tbs->cma_object) 
			aux_free_DName(&(tbs->cma_object));
		if(tbs->cma_purported)

		if(tbs->cma_common) 
			aux_free_CommonArguments(&(tbs->cma_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_CompareArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_CompareArgumentTBS(
	CompareArgumentTBS	**tbs
)

#else

void aux_free_CompareArgumentTBS(
	tbs
)
CompareArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_CompareArgumentTBS";

	if(tbs && * tbs){
		aux_free2_CompareArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (CompareArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_CompareResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_CompareResultTBS(
	register CompareResultTBS	 *tbs
)

#else

void aux_free2_CompareResultTBS(
	tbs
)
register CompareResultTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_CompareResultTBS";

	if(tbs){
		if(tbs->cmr_object) 
			aux_free_DName(&(tbs->cmr_object));
		if(tbs->cmr_common)
			aux_free_CommonRes(&(tbs->cmr_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_CompareResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_CompareResultTBS(
	CompareResultTBS	**tbs
)

#else

void aux_free_CompareResultTBS(
	tbs
)
CompareResultTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_CompareResultTBS";

	if(tbs && * tbs){
		aux_free2_CompareResultTBS(* tbs);
		free(* tbs);
		* tbs = (CompareResultTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ListArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ListArgumentTBS(
	register ListArgumentTBS	 *tbs
)

#else

void aux_free2_ListArgumentTBS(
	tbs
)
register ListArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_ListArgumentTBS";

	if(tbs){
		if(tbs->object) 
			aux_free_DName(&(tbs->object));
		if(tbs->lsa_common) 
			aux_free_CommonArguments(&(tbs->lsa_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ListArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ListArgumentTBS(
	ListArgumentTBS	**tbs
)

#else

void aux_free_ListArgumentTBS(
	tbs
)
ListArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_ListArgumentTBS";

	if(tbs && * tbs){
		aux_free2_ListArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (ListArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SubordEntry
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SubordEntry(
	register SubordEntry	 *sub
)

#else

void aux_free2_SubordEntry(
	sub
)
register SubordEntry	 *sub;

#endif

{
	char	 * proc = "aux_free2_SubordEntry";

	if(sub){
		if(sub->sub_rdn) 
			aux_free_RDName(&(sub->sub_rdn));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SubordEntry
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SubordEntry(
	SubordEntry	**sub
)

#else

void aux_free_SubordEntry(
	sub
)
SubordEntry	**sub;

#endif

{
	char	 * proc = "aux_free_SubordEntry";

	if(sub && * sub){
		aux_free2_SubordEntry(* sub);
		free(* sub);
		* sub = (SubordEntry * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_SubordEntry
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_SubordEntry(
	SET_OF_SubordEntry	**set
)

#else

void aux_free_SET_OF_SubordEntry(
	set
)
SET_OF_SubordEntry	**set;

#endif

{
	register SET_OF_SubordEntry * eptr;
	register SET_OF_SubordEntry * next;

	char * proc = "aux_free_SET_OF_SubordEntry";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_SubordEntry * )0; eptr = next){
			next = eptr->next;
			aux_free_SubordEntry(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_SubordEntry * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ListInfo
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ListInfo(
	register ListInfo	 *info
)

#else

void aux_free2_ListInfo(
	info
)
register ListInfo	 *info;

#endif

{
	char	 * proc = "aux_free2_ListInfo";

	if(info){
		if(info->lsr_object) 
			aux_free_DName(&(info->lsr_object));
		if(info->lsr_subordinates) 
			aux_free_SET_OF_SubordEntry(&(info->lsr_subordinates));
		if(info->lsr_poq) 
			aux_free_PartialOutQual(&info->lsr_poq);
		if(info->lsr_common) 
			aux_free_CommonRes(&(info->lsr_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ListInfo
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ListInfo(
	ListInfo	**info
)

#else

void aux_free_ListInfo(
	info
)
ListInfo	**info;

#endif

{
	char	 * proc = "aux_free_ListInfo";

	if(info && * info){
		aux_free2_ListInfo(* info);
		free(* info);
		* info = (ListInfo * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ListResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ListResultTBS(
	register ListResultTBS	 *tbs
)

#else

void aux_free2_ListResultTBS(
	tbs
)
register ListResultTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_ListResultTBS";

	if(tbs){
		if(tbs->lsr_type == 1)
			aux_free_ListInfo(&(tbs->lsrtbs_un.listinfo));
		else aux_free_SET_OF_ListResult(&(tbs->lsrtbs_un.uncorrel_listinfo));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ListResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ListResultTBS(
	ListResultTBS	**tbs
)

#else

void aux_free_ListResultTBS(
	tbs
)
ListResultTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_ListResultTBS";

	if(tbs && * tbs){
		aux_free2_ListResultTBS(* tbs);
		free(* tbs);
		* tbs = (ListResultTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_EntryModification
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_EntryModification(
	register EntryModification	 *em
)

#else

void aux_free2_EntryModification(
	em
)
register EntryModification	 *em;

#endif

{
	char	 * proc = "aux_free2_EntryModification";

	if(em){
		switch(em->em_type){
		case EM_ADDATTRIBUTE:
			aux_free_Attr(&(em->em_un.em_un_attr));
			break;
		case EM_REMOVEATTRIBUTE:
			aux_free_ObjId(&(em->em_un.em_un_attrtype));
			break;
		case EM_ADDVALUES:
			aux_free_Attr(&(em->em_un.em_un_attr));
			break;
		case EM_REMOVEVALUES:
			aux_free_Attr(&(em->em_un.em_un_attr));
			break;
		default:
			aux_add_error(EINVALID, "em->em_type has bad value", CNULL, 0, proc);
			return;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_EntryModification
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_EntryModification(
	EntryModification	**em
)

#else

void aux_free_EntryModification(
	em
)
EntryModification	**em;

#endif

{
	char	 * proc = "aux_free_EntryModification";

	if(em && * em){
		aux_free2_EntryModification(* em);
		free(* em);
		* em = (EntryModification * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SEQUENCE_OF_EntryModification
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SEQUENCE_OF_EntryModification(
	SEQUENCE_OF_EntryModification	**set
)

#else

void aux_free_SEQUENCE_OF_EntryModification(
	set
)
SEQUENCE_OF_EntryModification	**set;

#endif

{
	register SEQUENCE_OF_EntryModification * eptr;
	register SEQUENCE_OF_EntryModification * next;

	char * proc = "aux_free_SEQUENCE_OF_EntryModification";

	if(set && * set){
		for(eptr = * set; eptr != (SEQUENCE_OF_EntryModification * )0; eptr = next){
			next = eptr->next;
			aux_free_EntryModification(&(eptr->element));
			free((char * )eptr);
			eptr = (SEQUENCE_OF_EntryModification * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ModifyEntryArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ModifyEntryArgumentTBS(
	register ModifyEntryArgumentTBS	 *tbs
)

#else

void aux_free2_ModifyEntryArgumentTBS(
	tbs
)
register ModifyEntryArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_ModifyEntryArgumentTBS";

	if(tbs){
		if(tbs->mea_common) 
			aux_free_CommonArguments(&(tbs->mea_common));
		if(tbs->mea_object) 
			aux_free_DName(&(tbs->mea_object));
		if(tbs->mea_changes) 
			aux_free_SEQUENCE_OF_EntryModification(&(tbs->mea_changes));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ModifyEntryArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ModifyEntryArgumentTBS(
	ModifyEntryArgumentTBS	**tbs
)

#else

void aux_free_ModifyEntryArgumentTBS(
	tbs
)
ModifyEntryArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_ModifyEntryArgumentTBS";

	if(tbs && * tbs){
		aux_free2_ModifyEntryArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (ModifyEntryArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ModifyRDNArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ModifyRDNArgumentTBS(
	register ModifyRDNArgumentTBS	 *tbs
)

#else

void aux_free2_ModifyRDNArgumentTBS(
	tbs
)
register ModifyRDNArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_ModifyRDNArgumentTBS";

	if(tbs){
		if(tbs->mra_object) 
			aux_free_DName(&(tbs->mra_object));
		if(tbs->mra_newrdn) 
			aux_free_RDName(&(tbs->mra_newrdn));
		if(tbs->mra_common) 
			aux_free_CommonArguments(&(tbs->mra_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ModifyRDNArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ModifyRDNArgumentTBS(
	ModifyRDNArgumentTBS	**tbs
)

#else

void aux_free_ModifyRDNArgumentTBS(
	tbs
)
ModifyRDNArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_ModifyRDNArgumentTBS";

	if(tbs && * tbs){
		aux_free2_ModifyRDNArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (ModifyRDNArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ReadArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ReadArgumentTBS(
	register ReadArgumentTBS	 *tbs
)

#else

void aux_free2_ReadArgumentTBS(
	tbs
)
register ReadArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_ReadArgumentTBS";

	if(tbs){
		if(tbs->rda_common) 
			aux_free_CommonArguments(&(tbs->rda_common));
		if(tbs->object) 
			aux_free_DName(&(tbs->object));
		if(tbs->rda_eis) 
			aux_free_EntryInfoSEL(&(tbs->rda_eis));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ReadArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ReadArgumentTBS(
	ReadArgumentTBS	**tbs
)

#else

void aux_free_ReadArgumentTBS(
	tbs
)
ReadArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_ReadArgumentTBS";

	if(tbs && * tbs){
		aux_free2_ReadArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (ReadArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ReadResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ReadResultTBS(
	register ReadResultTBS	 *tbs
)

#else

void aux_free2_ReadResultTBS(
	tbs
)
register ReadResultTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_ReadResultTBS";

	if(tbs){
		if(tbs->rdr_entry) 
			aux_free_EntryINFO(&(tbs->rdr_entry));
		if(tbs->rdr_common) 
			aux_free_CommonRes(&(tbs->rdr_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ReadResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ReadResultTBS(
	ReadResultTBS	**tbs
)

#else

void aux_free_ReadResultTBS(
	tbs
)
ReadResultTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_ReadResultTBS";

	if(tbs && * tbs){
		aux_free2_ReadResultTBS(* tbs);
		free(* tbs);
		* tbs = (ReadResultTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_RemoveArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_RemoveArgumentTBS(
	register RemoveArgumentTBS	 *tbs
)

#else

void aux_free2_RemoveArgumentTBS(
	tbs
)
register RemoveArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_RemoveArgumentTBS";

	if(tbs){
		if(tbs->rma_object) 
			aux_free_DName(&(tbs->rma_object));
		if(tbs->rma_common) 
			aux_free_CommonArguments(&(tbs->rma_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_RemoveArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_RemoveArgumentTBS(
	RemoveArgumentTBS	**tbs
)

#else

void aux_free_RemoveArgumentTBS(
	tbs
)
RemoveArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_RemoveArgumentTBS";

	if(tbs && * tbs){
		aux_free2_RemoveArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (RemoveArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_PE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_PE(
	SET_OF_PE	**set
)

#else

void aux_free_SET_OF_PE(
	set
)
SET_OF_PE	**set;

#endif

{
	register SET_OF_PE * eptr;
	register SET_OF_PE * next;

	char * proc = "aux_free_SET_OF_PE";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_PE * )0; eptr = next){
			next = eptr->next;
			pe_free(eptr->element);
			free((char * )eptr);
			eptr = (SET_OF_PE * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_OperationProgress
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_OperationProgress(
	OperationProgress	**op
)

#else

void aux_free_OperationProgress(
	op
)
OperationProgress	**op;

#endif

{
	char	 * proc = "aux_free_OperationProgress";

	if(op && * op){
		free(* op);
		* op = (OperationProgress * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_OctetString
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_OctetString(
	SET_OF_OctetString	**set
)

#else

void aux_free_SET_OF_OctetString(
	set
)
SET_OF_OctetString	**set;

#endif

{
	register SET_OF_OctetString * eptr;
	register SET_OF_OctetString * next;

	char * proc = "aux_free_SET_OF_OctetString";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_OctetString * )0; eptr = next){
			next = eptr->next;
			aux_free_OctetString(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_OctetString * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_PSAPaddr
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_PSAPaddr(
	register typeDSE_PSAPaddr	 *psap
)

#else

void aux_free2_PSAPaddr(
	psap
)
register typeDSE_PSAPaddr	 *psap;

#endif

{
	char	 * proc = "aux_free2_PSAPaddr";

	if(psap){
		if (psap->pSelector.octets) 
			free(psap->pSelector.octets);
		if (psap->sSelector.octets) 
			free(psap->sSelector.octets);
		if (psap->tSelector.octets) 
			free(psap->tSelector.octets);
		if(psap->nAddress) 
			aux_free_SET_OF_OctetString(&(psap->nAddress));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_PSAPaddr
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_PSAPaddr(
	typeDSE_PSAPaddr	**psap
)

#else

void aux_free_PSAPaddr(
	psap
)
typeDSE_PSAPaddr	**psap;

#endif

{
	char	 * proc = "aux_free_PSAPaddr";

	if(psap && * psap){
		aux_free2_PSAPaddr(* psap);
		free(* psap);
		* psap = (typeDSE_PSAPaddr * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_AccessPoint
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_AccessPoint(
	register AccessPoint	 *accpoint
)

#else

void aux_free2_AccessPoint(
	accpoint
)
register AccessPoint	 *accpoint;

#endif

{
	char	 * proc = "aux_free2_AccessPoint";

	if(accpoint){
		if(accpoint->ap_name) 
			aux_free_DName(&(accpoint->ap_name));
		if(accpoint->ap_address) 
			aux_free_PSAPaddr(&(accpoint->ap_address));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_AccessPoint
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_AccessPoint(
	AccessPoint	**accpoint
)

#else

void aux_free_AccessPoint(
	accpoint
)
AccessPoint	**accpoint;

#endif

{
	char	 * proc = "aux_free_AccessPoint";

	if(accpoint && * accpoint){
		aux_free2_AccessPoint(* accpoint);
		free(* accpoint);
		* accpoint = (AccessPoint * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_AccessPoint
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_AccessPoint(
	SET_OF_AccessPoint	**set
)

#else

void aux_free_SET_OF_AccessPoint(
	set
)
SET_OF_AccessPoint	**set;

#endif

{
	register SET_OF_AccessPoint * eptr;
	register SET_OF_AccessPoint * next;

	char * proc = "aux_free_SET_OF_AccessPoint";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_AccessPoint * )0; eptr = next){
			next = eptr->next;
			aux_free_AccessPoint(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_AccessPoint * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ContReference
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ContReference(
	register ContReference	 *cref
)

#else

void aux_free2_ContReference(
	cref
)
register ContReference	 *cref;

#endif

{
	char	 * proc = "aux_free2_ContReference";

	if(cref){
		if(cref->cr_name) 
			aux_free_DName(&(cref->cr_name));
		if(cref->cr_progress) 
			aux_free_OperationProgress(&(cref->cr_progress));
		if(cref->cr_accesspoints) 
			aux_free_SET_OF_AccessPoint(&cref->cr_accesspoints);
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ContReference
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ContReference(
	ContReference	**cref
)

#else

void aux_free_ContReference(
	cref
)
ContReference	**cref;

#endif

{
	char	 * proc = "aux_free_ContReference";

	if(cref && * cref){
		aux_free2_ContReference(* cref);
		free(* cref);
		* cref = (ContReference * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_ContReference
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_ContReference(
	SET_OF_ContReference	**set
)

#else

void aux_free_SET_OF_ContReference(
	set
)
SET_OF_ContReference	**set;

#endif

{
	register SET_OF_ContReference * eptr;
	register SET_OF_ContReference * next;

	char * proc = "aux_free_SET_OF_ContReference";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_ContReference * )0; eptr = next){
			next = eptr->next;
			aux_free_ContReference(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_ContReference * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_PartialOutQual
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_PartialOutQual(
	register PartialOutQual	 *poq
)

#else

void aux_free2_PartialOutQual(
	poq
)
register PartialOutQual	 *poq;

#endif

{
	char	 * proc = "aux_free2_PartialOutQual";

	if(poq){
		if(poq->poq_cref) 
			aux_free_SET_OF_ContReference(&(poq->poq_cref));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_PartialOutQual
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_PartialOutQual(
	PartialOutQual	**poq
)

#else

void aux_free_PartialOutQual(
	poq
)
PartialOutQual	**poq;

#endif

{
	char	 * proc = "aux_free_PartialOutQual";

	if(poq && * poq){
		aux_free2_PartialOutQual(* poq);
		free(* poq);
		* poq = (PartialOutQual * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SearchInfo
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SearchInfo(
	register SearchInfo	 *info
)

#else

void aux_free2_SearchInfo(
	info
)
register SearchInfo	 *info;

#endif

{
	char	 * proc = "aux_free2_SearchInfo";

	if(info){
		if(info->srr_object) 
			aux_free_DName(&(info->srr_object));
		if(info->srr_entries) 
			aux_free_SET_OF_EntryINFO(&(info->srr_entries));
		if(info->srr_poq) 
			aux_free_PartialOutQual(&info->srr_poq);
		if(info->srr_common) 
			aux_free_CommonRes(&(info->srr_common));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SearchInfo
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SearchInfo(
	SearchInfo	**info
)

#else

void aux_free_SearchInfo(
	info
)
SearchInfo	**info;

#endif

{
	char	 * proc = "aux_free_SearchInfo";

	if(info && * info){
		aux_free2_SearchInfo(* info);
		free(* info);
		* info = (SearchInfo * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SFilterSubstrings
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SFilterSubstrings(
	register SFilterSubstrings	 *substrgs
)

#else

void aux_free2_SFilterSubstrings(
	substrgs
)
register SFilterSubstrings	 *substrgs;

#endif

{
	char	 * proc = "aux_free2_SFilterSubstrings";

	if(substrgs){
		if(substrgs->type) 
			aux_free_ObjId(((ObjId ** )&(substrgs->type)));
		if(substrgs->seq) 
			aux_free_SEQUENCE_OF_StringsCHOICE(&(substrgs->seq));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SFilterSubstrings
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SFilterSubstrings(
	SFilterSubstrings	**substrgs
)

#else

void aux_free_SFilterSubstrings(
	substrgs
)
SFilterSubstrings	**substrgs;

#endif

{
	char	 * proc = "aux_free_SFilterSubstrings";

	if(substrgs && * substrgs){
		aux_free2_SFilterSubstrings(* substrgs);
		free(* substrgs);
		* substrgs = (SFilterSubstrings * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SFilterItem
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SFilterItem(
	register SFilterItem	 *fi
)

#else

void aux_free2_SFilterItem(
	fi
)
register SFilterItem	 *fi;

#endif

{
	char	 * proc = "aux_free2_SFilterItem";

	if(fi){
		switch(fi->fi_type){
		case FILTERITEM_EQUALITY:
			aux_free_Ava(&(fi->fi_un.fi_un_ava));
			break;
		case FILTERITEM_SUBSTRINGS:
			aux_free_SFilterSubstrings(&(fi->fi_un.fi_un_substrings));
			break;
		case FILTERITEM_GREATEROREQUAL:
			aux_free_Ava(&(fi->fi_un.fi_un_ava));
			break;
		case FILTERITEM_LESSOREQUAL:
			aux_free_Ava(&(fi->fi_un.fi_un_ava));
			break;
		case FILTERITEM_PRESENT:
			aux_free_ObjId(((ObjId ** )&(fi->fi_un.fi_un_type)));
			break;
		case FILTERITEM_APPROX:
			aux_free_Ava(&(fi->fi_un.fi_un_ava));
			break;
		default:
			aux_add_error(EINVALID, "fi->fi_type has bad value", CNULL, 0, proc);
			return;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SFilterItem
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SFilterItem(
	SFilterItem	**fi
)

#else

void aux_free_SFilterItem(
	fi
)
SFilterItem	**fi;

#endif

{
	char	 * proc = "aux_free_SFilterItem";

	if(fi && * fi){
		aux_free2_SFilterItem(* fi);
		free(* fi);
		* fi = (SFilterItem * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SFilter
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SFilter(
	register SFilter	 *flt
)

#else

void aux_free2_SFilter(
	flt
)
register SFilter	 *flt;

#endif

{
	char	 * proc = "aux_free2_SFilter";

	if(flt){
		switch(flt->flt_type){
		case FILTER_ITEM:
			aux_free_SFilterItem(&(flt->flt_un.flt_un_item));
			break;
		case FILTER_AND:
			aux_free_SET_OF_SFilter(&(flt->flt_un.flt_un_filterset));
			break;
		case FILTER_OR:
			aux_free_SET_OF_SFilter(&(flt->flt_un.flt_un_filterset));
			break;
		case FILTER_NOT:
			aux_free_SFilter(&(flt->flt_un.flt_un_filter));
			break;
		default:
			aux_add_error(EINVALID, "flt->flt_type has bad value", CNULL, 0, proc);
			return;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SFilter
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SFilter(
	SFilter	**flt
)

#else

void aux_free_SFilter(
	flt
)
SFilter	**flt;

#endif

{
	char	 * proc = "aux_free_SFilter";

	if(flt && * flt){
		aux_free2_SFilter(* flt);
		free(* flt);
		* flt = (SFilter * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_SFilter
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_SFilter(
	SET_OF_SFilter	**set
)

#else

void aux_free_SET_OF_SFilter(
	set
)
SET_OF_SFilter	**set;

#endif

{
	register SET_OF_SFilter * eptr;
	register SET_OF_SFilter * next;

	char * proc = "aux_free_SET_OF_SFilter";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_SFilter * )0; eptr = next){
			next = eptr->next;
			aux_free_SFilter(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_SFilter * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_StringsCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_StringsCHOICE(
	register StringsCHOICE	 *choice
)

#else

void aux_free2_StringsCHOICE(
	choice
)
register StringsCHOICE	 *choice;

#endif

{
	char	 * proc = "aux_free2_StringsCHOICE";

	if(choice){
		switch(choice->strings_type){
		case STRINGS_INITIAL:
			pe_free(choice->strings_un.initial);
			break;
		case STRINGS_ANY:
			pe_free(choice->strings_un.any);
			break;
		case STRINGS_FINAL:
			pe_free(choice->strings_un.final);
			break;
		default:
			aux_add_error(EINVALID, "choice->strings_type has bad value", CNULL, 0, proc);
			return;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_StringsCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_StringsCHOICE(
	StringsCHOICE	**choice
)

#else

void aux_free_StringsCHOICE(
	choice
)
StringsCHOICE	**choice;

#endif

{
	char	 * proc = "aux_free_StringsCHOICE";

	if(choice && * choice){
		aux_free2_StringsCHOICE(* choice);
		free(* choice);
		* choice = (StringsCHOICE * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SEQUENCE_OF_StringsCHOICE
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SEQUENCE_OF_StringsCHOICE(
	SEQUENCE_OF_StringsCHOICE	**seq
)

#else

void aux_free_SEQUENCE_OF_StringsCHOICE(
	seq
)
SEQUENCE_OF_StringsCHOICE	**seq;

#endif

{
	register SEQUENCE_OF_StringsCHOICE * eptr;
	register SEQUENCE_OF_StringsCHOICE * next;

	char * proc = "aux_free_SEQUENCE_OF_StringsCHOICE";

	if(seq && * seq){
		for(eptr = * seq; eptr != (SEQUENCE_OF_StringsCHOICE * )0; eptr = next){
			next = eptr->next;
			aux_free_StringsCHOICE(&(eptr->element));
			free((char * )eptr);
			eptr = (SEQUENCE_OF_StringsCHOICE * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SearchArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SearchArgumentTBS(
	register SearchArgumentTBS	 *tbs
)

#else

void aux_free2_SearchArgumentTBS(
	tbs
)
register SearchArgumentTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_SearchArgumentTBS";

	if(tbs){
		if(tbs->sra_common) 
			aux_free_CommonArguments(&(tbs->sra_common));
		if(tbs->baseobject) 
			aux_free_DName(&(tbs->baseobject));
		if(tbs->filter) 
			aux_free_SFilter(&(tbs->filter));
		if(tbs->sra_eis) 
			aux_free_EntryInfoSEL(&(tbs->sra_eis));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SearchArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SearchArgumentTBS(
	SearchArgumentTBS	**tbs
)

#else

void aux_free_SearchArgumentTBS(
	tbs
)
SearchArgumentTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_SearchArgumentTBS";

	if(tbs && * tbs){
		aux_free2_SearchArgumentTBS(* tbs);
		free(* tbs);
		* tbs = (SearchArgumentTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SearchResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SearchResultTBS(
	register SearchResultTBS	 *tbs
)

#else

void aux_free2_SearchResultTBS(
	tbs
)
register SearchResultTBS	 *tbs;

#endif

{
	char	 * proc = "aux_free2_SearchResultTBS";

	if(tbs){
		if (tbs->srr_correlated == FALSE)
			aux_free_SET_OF_SearchResult(&(tbs->srrtbs_un.uncorrel_searchinfo));
		else
			aux_free_SearchInfo(&(tbs->srrtbs_un.searchinfo));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SearchResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SearchResultTBS(
	SearchResultTBS	**tbs
)

#else

void aux_free_SearchResultTBS(
	tbs
)
SearchResultTBS	**tbs;

#endif

{
	char	 * proc = "aux_free_SearchResultTBS";

	if(tbs && * tbs){
		aux_free2_SearchResultTBS(* tbs);
		free(* tbs);
		* tbs = (SearchResultTBS * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_Token
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_Token(
	register Token	 *tok
)

#else

void aux_free2_Token(
	tok
)
register Token	 *tok;

#endif

{
	char	 * proc = "aux_free2_Token";

	if(tok){
		if(tok->tbs_DERcode) 
			aux_free_OctetString(&(tok->tbs_DERcode));
		aux_free_TokenTBS(&(tok->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(tok->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_Token
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_Token(
	Token	**tok
)

#else

void aux_free_Token(
	tok
)
Token	**tok;

#endif

{
	char	 * proc = "aux_free_Token";

	if(tok && * tok){
		aux_free2_Token(* tok);
		free(* tok);
		* tok = (Token * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_AddArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_AddArgument(
	register AddArgument	 *addarg
)

#else

void aux_free2_AddArgument(
	addarg
)
register AddArgument	 *addarg;

#endif

{
	char	 * proc = "aux_free2_AddArgument";

	if(addarg){
		if(addarg->tbs_DERcode) 
			aux_free_OctetString(&(addarg->tbs_DERcode));
		aux_free_AddArgumentTBS(&(addarg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(addarg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_AddArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_AddArgument(
	AddArgument	**addarg
)

#else

void aux_free_AddArgument(
	addarg
)
AddArgument	**addarg;

#endif

{
	char	 * proc = "aux_free_AddArgument";

	if(addarg && * addarg){
		aux_free2_AddArgument(* addarg);
		free(* addarg);
		* addarg = (AddArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_CompareArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_CompareArgument(
	register CompareArgument	 *comparearg
)

#else

void aux_free2_CompareArgument(
	comparearg
)
register CompareArgument	 *comparearg;

#endif

{
	char	 * proc = "aux_free2_CompareArgument";

	if(comparearg){
		if(comparearg->tbs_DERcode) 
			aux_free_OctetString(&(comparearg->tbs_DERcode));
		aux_free_CompareArgumentTBS(&(comparearg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(comparearg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_CompareArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_CompareArgument(
	CompareArgument	**comparearg
)

#else

void aux_free_CompareArgument(
	comparearg
)
CompareArgument	**comparearg;

#endif

{
	char	 * proc = "aux_free_CompareArgument";

	if(comparearg && * comparearg){
		aux_free2_CompareArgument(* comparearg);
		free(* comparearg);
		* comparearg = (CompareArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_CompareResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_CompareResult(
	register CompareResult	 *compareres
)

#else

void aux_free2_CompareResult(
	compareres
)
register CompareResult	 *compareres;

#endif

{
	char	 * proc = "aux_free2_CompareResult";

	if(compareres){
		if(compareres->tbs_DERcode) 
			aux_free_OctetString(&(compareres->tbs_DERcode));
		aux_free_CompareResultTBS(&(compareres->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(compareres->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_CompareResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_CompareResult(
	CompareResult	**compareres
)

#else

void aux_free_CompareResult(
	compareres
)
CompareResult	**compareres;

#endif

{
	char	 * proc = "aux_free_CompareResult";

	if(compareres && * compareres){
		aux_free2_CompareResult(* compareres);
		free(* compareres);
		* compareres = (CompareResult * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ListArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ListArgument(
	register ListArgument	 *listarg
)

#else

void aux_free2_ListArgument(
	listarg
)
register ListArgument	 *listarg;

#endif

{
	char	 * proc = "aux_free2_ListArgument";

	if(listarg){
		if(listarg->tbs_DERcode) 
			aux_free_OctetString(&(listarg->tbs_DERcode));
		aux_free_ListArgumentTBS(&(listarg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(listarg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ListArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ListArgument(
	ListArgument	**listarg
)

#else

void aux_free_ListArgument(
	listarg
)
ListArgument	**listarg;

#endif

{
	char	 * proc = "aux_free_ListArgument";

	if(listarg && * listarg){
		aux_free2_ListArgument(* listarg);
		free(* listarg);
		* listarg = (ListArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ListResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ListResult(
	register ListResult	 *listres
)

#else

void aux_free2_ListResult(
	listres
)
register ListResult	 *listres;

#endif

{
	char	 * proc = "aux_free2_ListResult";

	if(listres){
		if(listres->tbs_DERcode) 
			aux_free_OctetString(&(listres->tbs_DERcode));
		aux_free_ListResultTBS(&(listres->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(listres->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ListResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ListResult(
	ListResult	**listres
)

#else

void aux_free_ListResult(
	listres
)
ListResult	**listres;

#endif

{
	char	 * proc = "aux_free_ListResult";

	if(listres && * listres){
		aux_free2_ListResult(* listres);
		free(* listres);
		* listres = (ListResult * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_ListResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_ListResult(
	SET_OF_ListResult	**set
)

#else

void aux_free_SET_OF_ListResult(
	set
)
SET_OF_ListResult	**set;

#endif

{
	register SET_OF_ListResult * eptr;
	register SET_OF_ListResult * next;

	char * proc = "aux_free_SET_OF_ListResult";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_ListResult * )0; eptr = next){
			next = eptr->next;
			aux_free_ListResult(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_ListResult * )0;
		}
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ModifyEntryArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ModifyEntryArgument(
	register ModifyEntryArgument	 *modifyentryarg
)

#else

void aux_free2_ModifyEntryArgument(
	modifyentryarg
)
register ModifyEntryArgument	 *modifyentryarg;

#endif

{
	char	 * proc = "aux_free2_ModifyEntryArgument";

	if(modifyentryarg){
		if(modifyentryarg->tbs_DERcode) 
			aux_free_OctetString(&(modifyentryarg->tbs_DERcode));
		aux_free_ModifyEntryArgumentTBS(&(modifyentryarg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(modifyentryarg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ModifyEntryArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ModifyEntryArgument(
	ModifyEntryArgument	**modifyentryarg
)

#else

void aux_free_ModifyEntryArgument(
	modifyentryarg
)
ModifyEntryArgument	**modifyentryarg;

#endif

{
	char	 * proc = "aux_free_ModifyEntryArgument";

	if(modifyentryarg && * modifyentryarg){
		aux_free2_ModifyEntryArgument(* modifyentryarg);
		free(* modifyentryarg);
		* modifyentryarg = (ModifyEntryArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ModifyRDNArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ModifyRDNArgument(
	register ModifyRDNArgument	 *modifyrdnarg
)

#else

void aux_free2_ModifyRDNArgument(
	modifyrdnarg
)
register ModifyRDNArgument	 *modifyrdnarg;

#endif

{
	char	 * proc = "aux_free2_ModifyRDNArgument";

	if(modifyrdnarg){
		if(modifyrdnarg->tbs_DERcode) 
			aux_free_OctetString(&(modifyrdnarg->tbs_DERcode));
		aux_free_ModifyRDNArgumentTBS(&(modifyrdnarg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(modifyrdnarg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ModifyRDNArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ModifyRDNArgument(
	ModifyRDNArgument	**modifyrdnarg
)

#else

void aux_free_ModifyRDNArgument(
	modifyrdnarg
)
ModifyRDNArgument	**modifyrdnarg;

#endif

{
	char	 * proc = "aux_free_ModifyRDNArgument";

	if(modifyrdnarg && * modifyrdnarg){
		aux_free2_ModifyRDNArgument(* modifyrdnarg);
		free(* modifyrdnarg);
		* modifyrdnarg = (ModifyRDNArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ReadArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ReadArgument(
	register ReadArgument	 *readarg
)

#else

void aux_free2_ReadArgument(
	readarg
)
register ReadArgument	 *readarg;

#endif

{
	char	 * proc = "aux_free2_ReadArgument";

	if(readarg){
		if(readarg->tbs_DERcode) 
			aux_free_OctetString(&(readarg->tbs_DERcode));
		aux_free_ReadArgumentTBS(&(readarg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(readarg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ReadArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ReadArgument(
	ReadArgument	**readarg
)

#else

void aux_free_ReadArgument(
	readarg
)
ReadArgument	**readarg;

#endif

{
	char	 * proc = "aux_free_ReadArgument";

	if(readarg && * readarg){
		aux_free2_ReadArgument(* readarg);
		free(* readarg);
		* readarg = (ReadArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_ReadResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_ReadResult(
	register ReadResult	 *readres
)

#else

void aux_free2_ReadResult(
	readres
)
register ReadResult	 *readres;

#endif

{
	char	 * proc = "aux_free2_ReadResult";

	if(readres){
		if(readres->tbs_DERcode) 
			aux_free_OctetString(&(readres->tbs_DERcode));
		aux_free_ReadResultTBS(&(readres->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(readres->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_ReadResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_ReadResult(
	ReadResult	**readres
)

#else

void aux_free_ReadResult(
	readres
)
ReadResult	**readres;

#endif

{
	char	 * proc = "aux_free_ReadResult";

	if(readres && * readres){
		aux_free2_ReadResult(* readres);
		free(* readres);
		* readres = (ReadResult * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_RemoveArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_RemoveArgument(
	register RemoveArgument	 *removearg
)

#else

void aux_free2_RemoveArgument(
	removearg
)
register RemoveArgument	 *removearg;

#endif

{
	char	 * proc = "aux_free2_RemoveArgument";

	if(removearg){
		if(removearg->tbs_DERcode) 
			aux_free_OctetString(&(removearg->tbs_DERcode));
		aux_free_RemoveArgumentTBS(&(removearg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(removearg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_RemoveArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_RemoveArgument(
	RemoveArgument	**removearg
)

#else

void aux_free_RemoveArgument(
	removearg
)
RemoveArgument	**removearg;

#endif

{
	char	 * proc = "aux_free_RemoveArgument";

	if(removearg && * removearg){
		aux_free2_RemoveArgument(* removearg);
		free(* removearg);
		* removearg = (RemoveArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SearchArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SearchArgument(
	register SearchArgument	 *searcharg
)

#else

void aux_free2_SearchArgument(
	searcharg
)
register SearchArgument	 *searcharg;

#endif

{
	char	 * proc = "aux_free2_SearchArgument";

	if(searcharg){
		if(searcharg->tbs_DERcode) 
			aux_free_OctetString(&(searcharg->tbs_DERcode));
		aux_free_SearchArgumentTBS(&(searcharg->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(searcharg->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SearchArgument
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SearchArgument(
	SearchArgument	**searcharg
)

#else

void aux_free_SearchArgument(
	searcharg
)
SearchArgument	**searcharg;

#endif

{
	char	 * proc = "aux_free_SearchArgument";

	if(searcharg && * searcharg){
		aux_free2_SearchArgument(* searcharg);
		free(* searcharg);
		* searcharg = (SearchArgument * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free2_SearchResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free2_SearchResult(
	register SearchResult	 *searchres
)

#else

void aux_free2_SearchResult(
	searchres
)
register SearchResult	 *searchres;

#endif

{
	char	 * proc = "aux_free2_SearchResult";

	if(searchres){
		if(searchres->tbs_DERcode) 
			aux_free_OctetString(&(searchres->tbs_DERcode));
		aux_free_SearchResultTBS(&(searchres->tbs));
		aux_free_KeyInfo(((KeyInfo ** )&(searchres->sig)));
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SearchResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SearchResult(
	SearchResult	**searchres
)

#else

void aux_free_SearchResult(
	searchres
)
SearchResult	**searchres;

#endif

{
	char	 * proc = "aux_free_SearchResult";

	if(searchres && * searchres){
		aux_free2_SearchResult(* searchres);
		free(* searchres);
		* searchres = (SearchResult * )0;
	}
	return;
}


/***************************************************************
 *
 * Procedure aux_free_SET_OF_SearchResult
 *
 ***************************************************************/
#ifdef __STDC__

void aux_free_SET_OF_SearchResult(
	SET_OF_SearchResult	**set
)

#else

void aux_free_SET_OF_SearchResult(
	set
)
SET_OF_SearchResult	**set;

#endif

{
	register SET_OF_SearchResult * eptr;
	register SET_OF_SearchResult * next;

	char * proc = "aux_free_SET_OF_SearchResult";

	if(set && * set){
		for(eptr = * set; eptr != (SET_OF_SearchResult * )0; eptr = next){
			next = eptr->next;
			aux_free_SearchResult(&(eptr->element));
			free((char * )eptr);
			eptr = (SET_OF_SearchResult * )0;
		}
	}
	return;
}







#else

/* dummy */
/***************************************************************
 *
 * Procedure strong_free_dummy
 *
 ***************************************************************/

int strong_free_dummy()

{
	return(0);
}

#endif
