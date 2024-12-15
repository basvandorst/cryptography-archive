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
#ifdef STRONG

#include "psap.h"
#include "secude-stub.h"



/***************************************************************
 *
 * Procedure e_TokenTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_TokenTBS(
	TokenTBS	 *token_tbs
)

#else

OctetString *e_TokenTBS(
	token_tbs
)
TokenTBS	 *token_tbs;

#endif

{
	PE             P_TBSToken, P_TBSToken_DER;
	OctetString  * ret;

	if(! token_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_TokenTBS(&P_TBSToken, 1, 0, CNULL, token_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSToken_DER = aux_DER_SETOF(P_TBSToken);
	ret = aux_PE2OctetString(P_TBSToken_DER);

	pe_free(P_TBSToken);
	pe_free(P_TBSToken_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_AddArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_AddArgumentTBS(
	AddArgumentTBS	 *addarg_tbs
)

#else

OctetString *e_AddArgumentTBS(
	addarg_tbs
)
AddArgumentTBS	 *addarg_tbs;

#endif

{
	PE             P_TBSAddArgument, P_TBSAddArgument_DER;
	OctetString  * ret;

	if(! addarg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_AddArgumentTBS(&P_TBSAddArgument, 1, 0, CNULL, addarg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSAddArgument_DER = aux_DER_SETOF(P_TBSAddArgument);
	ret = aux_PE2OctetString(P_TBSAddArgument_DER);

	pe_free(P_TBSAddArgument);
	pe_free(P_TBSAddArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_CompareArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CompareArgumentTBS(
	CompareArgumentTBS	 *cmparg_tbs
)

#else

OctetString *e_CompareArgumentTBS(
	cmparg_tbs
)
CompareArgumentTBS	 *cmparg_tbs;

#endif

{
	PE             P_TBSCompareArgument, P_TBSCompareArgument_DER;
	OctetString  * ret;

	if(! cmparg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_CompareArgumentTBS(&P_TBSCompareArgument, 1, 0, CNULL, cmparg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSCompareArgument_DER = aux_DER_SETOF(P_TBSCompareArgument);
	ret = aux_PE2OctetString(P_TBSCompareArgument_DER);

	pe_free(P_TBSCompareArgument);
	pe_free(P_TBSCompareArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_CompareResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_CompareResultTBS(
	CompareResultTBS	 *cmpres_tbs
)

#else

OctetString *e_CompareResultTBS(
	cmpres_tbs
)
CompareResultTBS	 *cmpres_tbs;

#endif

{
	PE             P_TBSCompareResult, P_TBSCompareResult_DER;
	OctetString  * ret;

	if(! cmpres_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_CompareResultTBS(&P_TBSCompareResult, 1, 0, CNULL, cmpres_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSCompareResult_DER = aux_DER_SETOF(P_TBSCompareResult);
	ret = aux_PE2OctetString(P_TBSCompareResult_DER);

	pe_free(P_TBSCompareResult);
	pe_free(P_TBSCompareResult_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ListArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ListArgumentTBS(
	ListArgumentTBS	 *listarg_tbs
)

#else

OctetString *e_ListArgumentTBS(
	listarg_tbs
)
ListArgumentTBS	 *listarg_tbs;

#endif

{
	PE             P_TBSListArgument, P_TBSListArgument_DER;
	OctetString  * ret;

	if(! listarg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_ListArgumentTBS(&P_TBSListArgument, 1, 0, CNULL, listarg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSListArgument_DER = aux_DER_SETOF(P_TBSListArgument);
	ret = aux_PE2OctetString(P_TBSListArgument_DER);

	pe_free(P_TBSListArgument);
	pe_free(P_TBSListArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ListResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ListResultTBS(
	ListResultTBS	 *listres_tbs
)

#else

OctetString *e_ListResultTBS(
	listres_tbs
)
ListResultTBS	 *listres_tbs;

#endif

{
	PE             P_TBSListResult, P_TBSListResult_DER;
	OctetString  * ret;

	if(! listres_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_ListResultTBS(&P_TBSListResult, 1, 0, CNULL, listres_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSListResult_DER = aux_DER_SETOF(P_TBSListResult);
	ret = aux_PE2OctetString(P_TBSListResult_DER);

	pe_free(P_TBSListResult);
	pe_free(P_TBSListResult_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ListResult
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ListResult(
	ListResult	 *listres
)

#else

OctetString *e_ListResult(
	listres
)
ListResult	 *listres;

#endif

{
	PE             P_ListResult, P_ListResult_DER;
	OctetString  * ret;

	if(! listres)
		return(NULLOCTETSTRING);

	if(build_STRONG_ListResult(&P_ListResult, 1, 0, CNULL, listres) == NOTOK)
		return(NULLOCTETSTRING);

	P_ListResult_DER = aux_DER_SETOF(P_ListResult);
	ret = aux_PE2OctetString(P_ListResult_DER);

	pe_free(P_ListResult);
	pe_free(P_ListResult_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ModifyEntryArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ModifyEntryArgumentTBS(
	ModifyEntryArgumentTBS	 *modarg_tbs
)

#else

OctetString *e_ModifyEntryArgumentTBS(
	modarg_tbs
)
ModifyEntryArgumentTBS	 *modarg_tbs;

#endif

{
	PE             P_TBSModifyEntryArgument, P_TBSModifyEntryArgument_DER;
	OctetString  * ret;

	if(! modarg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_ModifyEntryArgumentTBS(&P_TBSModifyEntryArgument, 1, 0, CNULL, modarg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSModifyEntryArgument_DER = aux_DER_SETOF(P_TBSModifyEntryArgument);
	ret = aux_PE2OctetString(P_TBSModifyEntryArgument_DER);

	pe_free(P_TBSModifyEntryArgument);
	pe_free(P_TBSModifyEntryArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ModifyRDNArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ModifyRDNArgumentTBS(
	ModifyRDNArgumentTBS	 *modrdnarg_tbs
)

#else

OctetString *e_ModifyRDNArgumentTBS(
	modrdnarg_tbs
)
ModifyRDNArgumentTBS	 *modrdnarg_tbs;

#endif

{
	PE             P_TBSModifyRDNArgument, P_TBSModifyRDNArgument_DER;
	OctetString  * ret;

	if(! modrdnarg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_ModifyRDNArgumentTBS(&P_TBSModifyRDNArgument, 1, 0, CNULL, modrdnarg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSModifyRDNArgument_DER = aux_DER_SETOF(P_TBSModifyRDNArgument);
	ret = aux_PE2OctetString(P_TBSModifyRDNArgument_DER);

	pe_free(P_TBSModifyRDNArgument);
	pe_free(P_TBSModifyRDNArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ReadArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ReadArgumentTBS(
	ReadArgumentTBS	 *readarg_tbs
)

#else

OctetString *e_ReadArgumentTBS(
	readarg_tbs
)
ReadArgumentTBS	 *readarg_tbs;

#endif

{
	PE             P_TBSReadArgument, P_TBSReadArgument_DER;
	OctetString  * ret;

	if(! readarg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_ReadArgumentTBS(&P_TBSReadArgument, 1, 0, CNULL, readarg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSReadArgument_DER = aux_DER_SETOF(P_TBSReadArgument);
	ret = aux_PE2OctetString(P_TBSReadArgument_DER);

	pe_free(P_TBSReadArgument);
	pe_free(P_TBSReadArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_ReadResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_ReadResultTBS(
	ReadResultTBS	 *readres_tbs
)

#else

OctetString *e_ReadResultTBS(
	readres_tbs
)
ReadResultTBS	 *readres_tbs;

#endif

{
	PE             P_TBSReadResult, P_TBSReadResult_DER;
	OctetString  * ret;

	if(! readres_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_ReadResultTBS(&P_TBSReadResult, 1, 0, CNULL, readres_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSReadResult_DER = aux_DER_SETOF(P_TBSReadResult);
	ret = aux_PE2OctetString(P_TBSReadResult_DER);

	pe_free(P_TBSReadResult);
	pe_free(P_TBSReadResult_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_RemoveArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_RemoveArgumentTBS(
	RemoveArgumentTBS	 *remarg_tbs
)

#else

OctetString *e_RemoveArgumentTBS(
	remarg_tbs
)
RemoveArgumentTBS	 *remarg_tbs;

#endif

{
	PE             P_TBSRemoveArgument, P_TBSRemoveArgument_DER;
	OctetString  * ret;

	if(! remarg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_RemoveArgumentTBS(&P_TBSRemoveArgument, 1, 0, CNULL, remarg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSRemoveArgument_DER = aux_DER_SETOF(P_TBSRemoveArgument);
	ret = aux_PE2OctetString(P_TBSRemoveArgument_DER);

	pe_free(P_TBSRemoveArgument);
	pe_free(P_TBSRemoveArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_SearchArgumentTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_SearchArgumentTBS(
	SearchArgumentTBS	 *searcharg_tbs
)

#else

OctetString *e_SearchArgumentTBS(
	searcharg_tbs
)
SearchArgumentTBS	 *searcharg_tbs;

#endif

{
	PE             P_TBSSearchArgument, P_TBSSearchArgument_DER;
	OctetString  * ret;

	if(! searcharg_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_SearchArgumentTBS(&P_TBSSearchArgument, 1, 0, CNULL, searcharg_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSSearchArgument_DER = aux_DER_SETOF(P_TBSSearchArgument);
	ret = aux_PE2OctetString(P_TBSSearchArgument_DER);

	pe_free(P_TBSSearchArgument);
	pe_free(P_TBSSearchArgument_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_SearchResultTBS
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_SearchResultTBS(
	SearchResultTBS	 *searchres_tbs
)

#else

OctetString *e_SearchResultTBS(
	searchres_tbs
)
SearchResultTBS	 *searchres_tbs;

#endif

{
	PE             P_TBSSearchResult, P_TBSSearchResult_DER;
	OctetString  * ret;

	if(! searchres_tbs)
		return(NULLOCTETSTRING);

	if(build_STRONG_SearchResultTBS(&P_TBSSearchResult, 1, 0, CNULL, searchres_tbs) == NOTOK)
		return(NULLOCTETSTRING);

	P_TBSSearchResult_DER = aux_DER_SETOF(P_TBSSearchResult);
	ret = aux_PE2OctetString(P_TBSSearchResult_DER);

	pe_free(P_TBSSearchResult);
	pe_free(P_TBSSearchResult_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure e_SearchResult
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *e_SearchResult(
	SearchResult	 *searchres
)

#else

OctetString *e_SearchResult(
	searchres
)
SearchResult	 *searchres;

#endif

{
	PE             P_SearchResult, P_SearchResult_DER;
	OctetString  * ret;

	if(! searchres)
		return(NULLOCTETSTRING);

	if(build_STRONG_SearchResult(&P_SearchResult, 1, 0, CNULL, searchres) == NOTOK)
		return(NULLOCTETSTRING);

	P_SearchResult_DER = aux_DER_SETOF(P_SearchResult);
	ret = aux_PE2OctetString(P_SearchResult_DER);

	pe_free(P_SearchResult);
	pe_free(P_SearchResult_DER);

	return(ret);
}


/***************************************************************
 *
 * Procedure PSAPaddr_dec
 *
 ***************************************************************/
#ifdef __STDC__

typeDSE_PSAPaddr *PSAPaddr_dec(
	PE	  pe
)

#else

typeDSE_PSAPaddr *PSAPaddr_dec(
	pe
)
PE	  pe;

#endif

{
	typeDSE_PSAPaddr * ret;
	int	           result;
	char	         * proc = "PSAPaddr_dec";

#ifdef TEST
	fprintf(stderr, "%s\n", proc);	
#endif

	if ( pe == NULLPE )
		return( (typeDSE_PSAPaddr * )0 );

	/*NOTE: Space for PSAPaddr is allocated by parse_STRONG_PSAPaddr,*/
	/*parameter is of type typeDSE_PSAPaddr ** (not *!) */

	result = parse_STRONG_PSAPaddr (pe, 1, NULLIP, NULLVP, &ret);

	return (result ? (typeDSE_PSAPaddr * )0 : ret);
}


/***************************************************************
 *
 * Procedure handle_seq
 *
 ***************************************************************/
#ifdef __STDC__

static PE handle_seq(
	PE	  	seq
)

#else

static PE handle_seq(
	seq
)
PE	     seq;

#endif

{
	PE     new_seq, tmp_pe, tmp2_pe, last = NULLPE;
	char * proc = "handle_seq";


	if(! seq){
		aux_add_error(EINVALID, "Parameter missing", CNULL, 0, proc);
		return(NULLPE);
	}

	/* build new SEQUENCE */
	new_seq = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SEQ);
	if(new_seq == NULLPE){
		aux_add_error(EMALLOC, "new_seq", CNULL, 0, proc);
		return(NULLPE);
	}

	/* step through SEQUENCE */
	for (tmp_pe = first_member(seq); tmp_pe; tmp_pe = next_member(seq, tmp_pe)) {
		/* DER encode each component of SEQUENCE */
		tmp2_pe = aux_DER_SETOF(tmp_pe);

		/* Add DER-coded component to new SEQUENCE */
		seq_addon(new_seq, last, tmp2_pe);
		last = tmp2_pe;
	}

	return(new_seq);
}


/***************************************************************
 *
 * Procedure handle_set
 *
 ***************************************************************/
#ifdef __STDC__

static PE handle_set(
	PE	  	set
)

#else

static PE handle_set(
	set
)
PE	     set;

#endif

{
	PE 		   new_set; /* SET or SET OF type, whose members are each DER sorted */
	PE 		   sorted_set;
	PE 		   tmp_pe, tmp2_pe;
	PE 		   last = NULLPE, first;
	PElementID 	   pe_id;
	PElementClass 	   pe_class;
	OctetString 	** oSTK, * tmp_ostr;
	unsigned char      a, b;
	int 	           n, i, k, j, s, cnt = 0, index = 0;
	Boolean 	   equal = FALSE;
	char	         * proc = "handle_set";


	if(! set){
		aux_add_error(EINVALID, "Parameter missing", CNULL, 0, proc);
		return(NULLPE);
	}

	new_set = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SET);
	if(new_set == NULLPE){
		aux_add_error(EMALLOC, "new_set", CNULL, 0, proc);
		return(NULLPE);
	}

	/* prepare check for equal types */
	equal = TRUE;

	first = first_member(set);
	if (! first){
		/*aux_add_error(EINVALID, "first_member failed", CNULL, 0, proc);
		return(NULLPE);*/
		return(set);
	}

	pe_id = first->pe_id;
	pe_class = first->pe_class;

	for (tmp_pe = first; tmp_pe; tmp_pe = next_member(set, tmp_pe)){
		/* DER encode each component of SET */
		tmp2_pe = aux_DER_SETOF(tmp_pe);

		/* Add DER-coded component to new SET */
		(void) set_addon(new_set, last, tmp2_pe);
		last = tmp2_pe;

		/* Compare tags, i.e., check whether a SET or a SET OF type is present */
		if(equal == TRUE)
			equal = (tmp2_pe->pe_id == pe_id && tmp2_pe->pe_class == pe_class);

		cnt ++;
	} /* for */


	if(equal == TRUE && cnt > 1){
		/* SET OF type, whose members are to be sorted in ascending order */

		oSTK = (OctetString ** )calloc(cnt, sizeof(OctetString * ));
		if(! oSTK){
			aux_add_error(EMALLOC, "oSTK", CNULL, 0, proc);
			return(NULLPE);
		}

		for (tmp_pe = first_member(new_set); tmp_pe; tmp_pe = next_member(new_set, tmp_pe)){
			oSTK[index] = aux_PE2OctetString(tmp_pe);
			if(! oSTK[index++]){
				aux_add_error(LASTERROR, "aux_PE2OctetString failed", CNULL, 0, proc);
				return(NULLPE);
			}
		}

		/* sort elements of oSTK in ascending order */
	
		for (i = 0; i < cnt - 1; i++) {
			k = i;
			tmp_ostr = oSTK[i];
			for (j = i + 1; j < cnt; j++) {
	
				/* n = min(tmp_ostr->noctets, oSTK[j]->noctets) */
				n = tmp_ostr->noctets;
				if(oSTK[j]->noctets < tmp_ostr->noctets)
					n = oSTK[j]->noctets;
	
				s = 0;
				while (oSTK[j]->octets[s] == tmp_ostr->octets[s] && s < n)
					s++;
				if(s < n && (a = oSTK[j]->octets[s]) < (b = tmp_ostr->octets[s])){
					k = j;
					tmp_ostr = oSTK[j];
				} /* if */
	
			}  /* for */
			oSTK[k] = oSTK[i];
			oSTK[i] = tmp_ostr;
		}  /* for */
	
	
		if ((sorted_set = pe_alloc (PE_CLASS_UNIV, PE_FORM_CONS, PE_CONS_SET)) == NULLPE){
			aux_add_error(EMALLOC, "sorted_set", CNULL, 0, proc);
		    	return (NULLPE);
		}
    
		for (i = 0, last = NULLPE; i < cnt; i++) {
		    	tmp_pe = aux_OctetString2PE(oSTK[i]);
			if(! tmp_pe){	
				aux_add_error(LASTERROR, "aux_OctetString2PE failed", CNULL, 0, proc);
				return(NULLPE);
			}
		    	(void) seq_addon (sorted_set, last, tmp_pe);
		    	last = tmp_pe;
		}
	
		for (i = 0; i < cnt; i++) aux_free_OctetString(&oSTK[i]);
	
		free (oSTK);
	}
	else{
		/* SET type, whose members are already sorted in ascending order */
		sorted_set = pe_cpy(new_set);
	}

	pe_free(new_set);

	return(sorted_set);
}


/***************************************************************
 *
 * Procedure aux_DER_SETOF
 *
 ***************************************************************/
#ifdef __STDC__

PE aux_DER_SETOF(
	PE	  	pe
)

#else

PE aux_DER_SETOF(
	pe
)
PE	     pe;

#endif

{
	PE		sorted_pe, tmp_pe = NULLPE;
	char	      * proc = "aux_DER_SETOF";

	if(! pe){
		aux_add_error(EINVALID, "Parameter missing", CNULL, 0, proc);
		return(NULLPE);
	}

	/* check whether pe_form is primitive or constructed */
	switch(pe->pe_form){

	case PE_FORM_PRIM:
		/* do nothing, just copy */
		return(pe_cpy(pe));
		break;

	case PE_FORM_CONS:
		/* check classes */
		switch(pe->pe_class){

		case PE_CLASS_UNIV: /* Universal */

			/* check tag (pe_id) */
			switch(pe->pe_id){
	
			case PE_CONS_SEQ:
				tmp_pe = handle_seq(pe);
				break;

			case PE_CONS_SET:
				tmp_pe = handle_set(pe);
				break;
			} /* switch */

			break;

		case PE_CLASS_APPL: /* Application-wide */	
		case PE_CLASS_CONT: /* Context-specific */
		case PE_CLASS_PRIV: /* Private-use */
			/* Unter der Annahme, dass diesen Tagklassen lediglich
			   EXPLIZITES (und nicht IMPLIZITES) Taggen zugrundeliegt,
			   dann wird das zugrundeliegende Universal-Tag im Inhaltsfeld
			   mitkodiert. Man verzweigt also einfach in das Inhaltsfeld
			   (un_pe_cons) und DER-codiert es weiter.

			   s. ASN.1, S.126, 6.3.12: Bei der Kodierung eines Wertes, 
			   dessen Typ mit Hilfe von Tags definiert wurde, ist zu unterscheiden
			   zwischen der Kodierung des Tags und der Kodierung des
			   zugrundeliegenden Typs. Falls das Schluesselwort IMPLICIT
			   bei der Definition des TAGGED-Types nicht angegeben wurde,
			   ist die Kodierung zusammengesetzt [constructed] und das
		           Inhaltsfeld besteht aus der kompletten Kodierung des
			   zugrundeliegenden (Basis-)Typs [also "Tag + Value"].
			*/
			tmp_pe = pe_cpy(pe);

			tmp_pe->pe_un1.un_pe_cons = aux_DER_SETOF(pe->pe_un1.un_pe_cons);
			break;
		} /* switch */
		break;

	case PE_FORM_ICONS:
		break;

	} /* switch */

	return(tmp_pe);
}




#else

/* dummy */
strong_ed_dummy() 
{
	return(0);
}

#endif


#else
/* dummy */
strong_ed_dummy() 
{
	return(0);
}

#endif

