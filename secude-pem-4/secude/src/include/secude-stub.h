#ifndef _STRONG_
#define _STRONG_

#ifndef _AF_
#include "af.h"
#endif

/* Declaration of build- and parse-routines */
#include "STRONG-types.h"

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif


typedef struct TokenTBS          	TokenTBS;
typedef struct Token             	Token;

typedef struct AddArgumentTBS   	AddArgumentTBS;
typedef struct AddArgument      	AddArgument;

typedef struct CompareArgumentTBS   	CompareArgumentTBS;
typedef struct CompareArgument      	CompareArgument;
typedef struct CompareResultTBS		CompareResultTBS;
typedef struct CompareResult		CompareResult;

typedef struct ListArgumentTBS   	ListArgumentTBS;
typedef struct ListArgument      	ListArgument;
typedef struct ListResultTBS		ListResultTBS;
typedef struct ListResult		ListResult;
typedef SET_OF(ListResult)		SET_OF_ListResult;
typedef struct SubordEntry		SubordEntry;
typedef SET_OF(SubordEntry)         	SET_OF_SubordEntry;
typedef struct ListInfo			ListInfo;

typedef struct ModifyEntryArgumentTBS   ModifyEntryArgumentTBS;
typedef struct ModifyEntryArgument      ModifyEntryArgument;
typedef struct EntryModification	EntryModification;
typedef SEQUENCE_OF(EntryModification)	SEQUENCE_OF_EntryModification;

typedef struct ModifyRDNArgumentTBS   	ModifyRDNArgumentTBS;
typedef struct ModifyRDNArgument      	ModifyRDNArgument;

typedef struct ReadArgumentTBS   	ReadArgumentTBS;
typedef struct ReadArgument      	ReadArgument;
typedef struct ReadResultTBS   		ReadResultTBS;
typedef struct ReadResult      		ReadResult;

typedef struct RemoveArgumentTBS   	RemoveArgumentTBS;
typedef struct RemoveArgument      	RemoveArgument;

typedef struct SearchArgumentTBS   	SearchArgumentTBS;
typedef struct SearchArgument      	SearchArgument;
typedef struct SearchResultTBS   	SearchResultTBS;
typedef struct SearchResult      	SearchResult;
typedef SET_OF(SearchResult)            SET_OF_SearchResult;
typedef struct SearchInfo		SearchInfo;
typedef struct OperationProgress	OperationProgress;
typedef struct ContReference		ContReference;
typedef SET_OF(ContReference)	        SET_OF_ContReference;
typedef struct PartialOutQual		PartialOutQual;
typedef struct AccessPoint		AccessPoint;
typedef SET_OF(AccessPoint)		SET_OF_AccessPoint;

typedef struct SFilter			SFilter;
typedef SET_OF(SFilter)           	SET_OF_SFilter;
typedef struct SFilterItem              SFilterItem;
typedef struct SFilterSubstrings        SFilterSubstrings;
typedef struct StringsCHOICE		StringsCHOICE;
typedef SEQUENCE_OF(StringsCHOICE)	SEQUENCE_OF_StringsCHOICE;

typedef struct EntryInfoSEL		EntryInfoSEL;
typedef struct EntryINFO		EntryINFO;
typedef SET_OF(EntryINFO)		SET_OF_EntryINFO;

typedef struct CommonArguments   	CommonArguments;
typedef struct CommonRes		CommonRes;

typedef struct SECExtension		SECExtension;
typedef struct ServiceControls   	ServiceControls;
typedef struct PartOutQual		PartOutQual;
typedef struct SecurityParameters       SecurityParameters;
typedef SET_OF(SECExtension)       	SET_OF_SECExtension;

typedef struct typeDSE_PSAPaddr		typeDSE_PSAPaddr;

typedef struct AttrAttrTypeCHOICE       AttrAttrTypeCHOICE;
typedef SET_OF(AttrAttrTypeCHOICE)      SET_OF_AttrAttrTypeCHOICE;

typedef struct set_of_PE {
	PE	           element;
	struct set_of_PE * next;
} SET_OF_PE;

extern struct signature        * secudesigned();
extern int      		 secudeverify();
extern struct certificate_list * secude_mkpath();
extern int			 secude_ckpath();
extern struct encrypted	       * secudeencrypted(); 
extern int      		 secudedecrypted();
extern struct Nonce            * secudemknonce();
extern int			 secudecknonce();


#define LISTRESULTSET 1
#define SEARCHRESULTSET 2
#define SECEXTENSIONSET 3
#define ATTRTYPESET 4
#define ATTRSET 5
#define ATTRATTRTYPECHOICESET 6
#define ENTRYINFOSET 7
#define FILTERSET 8
#define SUBORDENTRYSET 9
#define DNAMESET 10
#define OBJIDSET 11
#define ACLINFOSET 12
#define ATTRACLSET 13
#define CONTREFSET 14
#define OSTRSET 15
#define ACCESSPOINTSET 16


#define SECUDESERVICES { \
	"secudesecurityservicesforquipu", \
	secudesigned, \
	secudeverify, \
	secude_ckpath, \
	secude_ckpath, \
	secude_mkpath, \
	secudeencrypted, \
	secudedecrypted, \
	secudemknonce, \
	secudecknonce \
}


extern struct SecurityServices	* use_serv_secude();



/*
 *      Object Identifiers of Attribute Types (extern: strong-init.c)
 */

extern ObjId *Acl_OID;


/********************** A D D  Section **********************/


struct AddArgumentTBS {
        DName           * ada_object;
	SET_OF_Attr     * ada_entry;   /* SET_OF_Attr is analogous to Attr_Sequence, used by QUIPU (see modify.h) */
	CommonArguments * ada_common;
};

struct AddArgument {
        OctetString    * tbs_DERcode;  /* Return-Parameter of e_AddArgumentTBS */
        AddArgumentTBS * tbs;
        Signature      * sig;         
};


/********************** B I N D  Section **********************/


struct TokenTBS {
        AlgId          * signatureAI;  
        DName          * dname;
        UTCTime        * time;
        BitString      * random;
};

struct Token {
        OctetString    * tbs_DERcode;  /* Return-Parameter of e_TokenTBS */
        TokenTBS       * tbs;
        Signature      * sig;         
};


/********************** C O M P A R E   Section *********************/


struct CompareArgumentTBS {
        DName                                   * cma_object;
        AttrValueAssertion  * cma_purported;
        CommonArguments    			* cma_common;
};

struct CompareArgument {
        OctetString         * tbs_DERcode;  /* Return-Parameter of e_CompareArgumentTBS */
        CompareArgumentTBS  * tbs;
        Signature           * sig;         
};

struct CompareResultTBS {
	DName            * cmr_object;
        Boolean            cmr_matched;           /* set to TRUE or FALSE */
        Boolean            cmr_fromEntry;           
        CommonRes        * cmr_common;
};

struct CompareResult {
        OctetString      * tbs_DERcode;  /* Return-Parameter of e_CompareResultTBS */
        CompareResultTBS * tbs;
        Signature        * sig;         
};


/********************** L I S T  Section **********************/


struct ListArgumentTBS {
	DName		   * object;
	CommonArguments    * lsa_common;
};

struct ListArgument {
        OctetString      * tbs_DERcode;  /* Return-Parameter of e_ListArgumentTBS */
        ListArgumentTBS  * tbs;
        Signature        * sig;         
};

struct SubordEntry {
	RDName        * sub_rdn;
	Boolean		sub_aliasentry;
	Boolean		sub_copy;
};	

struct ListInfo {
	DName    	       * lsr_object;
    	SET_OF_SubordEntry     * lsr_subordinates;
        PartialOutQual	       * lsr_poq;
	CommonRes	       * lsr_common;
};

struct ListResultTBS {
	int	lsr_type;
#define LSR_INFO                 1
#define LSR_UNCORRELINFO         2
	union {
		ListInfo	      * listinfo;
		SET_OF_ListResult     * uncorrel_listinfo;
	} lsrtbs_un;
};

struct ListResult {
        OctetString      * tbs_DERcode;  /* Return-Parameter of e_ListResultTBS */
        ListResultTBS    * tbs;
        Signature        * sig;         
};


/******************** M O D I F Y E N T R Y  Section ********************/


struct EntryModification {
        int                 em_type;
#define EM_ADDATTRIBUTE         1
#define EM_REMOVEATTRIBUTE      2
#define EM_ADDVALUES            3
#define EM_REMOVEVALUES         4
	union {
		Attr      * em_un_attr;
		ObjId	  * em_un_attrtype;
	} em_un;
};

struct ModifyEntryArgumentTBS {
        CommonArguments                  * mea_common;
        DName                            * mea_object;
        SEQUENCE_OF_EntryModification    * mea_changes;
};

struct ModifyEntryArgument {
        OctetString             * tbs_DERcode;  /* Return-Parameter of e_ModifyEntryArgumentTBS */
        ModifyEntryArgumentTBS  * tbs;
        Signature               * sig;         
};


/******************** M O D I F Y R D N  Section ********************/


struct ModifyRDNArgumentTBS {
        DName                   * mra_object;
	RDName		        * mra_newrdn;
	Boolean 		  deleterdn;     /* set to TRUE or FLASE */
        CommonArguments         * mra_common;
};

struct ModifyRDNArgument {
        OctetString             * tbs_DERcode;  /* Return-Parameter of e_ModifyRDNArgumentTBS */
        ModifyRDNArgumentTBS    * tbs;
        Signature               * sig;         
};


/********************** R E A D   Section *********************/


struct ReadArgumentTBS {
        CommonArguments    * rda_common;
        DName              * object;
        EntryInfoSEL       * rda_eis;
};

struct ReadArgument {
        OctetString      * tbs_DERcode;  /* Return-Parameter of e_ReadArgumentTBS */
        ReadArgumentTBS  * tbs;
        Signature        * sig;         
};

struct ReadResultTBS {
	EntryINFO        * rdr_entry;
        CommonRes        * rdr_common;
};

struct ReadResult {
        OctetString      * tbs_DERcode;  /* Return-Parameter of e_ReadResultTBS */
        ReadResultTBS    * tbs;
        Signature        * sig;         
};


/********************** R E M O V E   Section *********************/


struct RemoveArgumentTBS {
        DName              * rma_object;
        CommonArguments    * rma_common;
};

struct RemoveArgument {
        OctetString        * tbs_DERcode;  /* Return-Parameter of e_RemoveArgumentTBS */
        RemoveArgumentTBS  * tbs;
        Signature          * sig;         
};


/******************** S E A R C H  Section ********************/

struct StringsCHOICE {
	int	     		   strings_type;
#define STRINGS_INITIAL	1
#define STRINGS_ANY	2
#define STRINGS_FINAL	3
	union {
		PE	           initial;
		PE	           any;
		PE	           final;
	} strings_un;
};

struct SFilterSubstrings {
	AttrType                  * type;
	SEQUENCE_OF_StringsCHOICE * seq;
};

struct SFilterItem {
	int      fi_type;
#define FILTERITEM_EQUALITY 1
#define FILTERITEM_SUBSTRINGS 2
#define FILTERITEM_GREATEROREQUAL 3
#define FILTERITEM_LESSOREQUAL 4
#define FILTERITEM_PRESENT 5
#define FILTERITEM_APPROX 6
	union {
		AttrType	    * fi_un_type;
		AttrValueAssertion  * fi_un_ava;
		SFilterSubstrings   * fi_un_substrings;
	} fi_un;
};

struct SFilter {
	int	flt_type;
#define FILTER_ITEM 1
#define FILTER_AND 2
#define FILTER_OR 3
#define FILTER_NOT 4
	union {
		SFilterItem         * flt_un_item;
		struct SFilter      * flt_un_filter;
		SET_OF_SFilter      * flt_un_filterset;
	} flt_un;
};

struct SearchArgumentTBS {   
	CommonArguments    * sra_common;
	DName		   * baseobject;
	int 		     subset;
#define SRA_BASEOBJECT          0
#define SRA_ONELEVEL            1
#define SRA_WHOLESUBTREE        2
        SFilter            * filter; 
        Boolean              searchaliases;
        EntryInfoSEL 	   * sra_eis;
};

struct SearchArgument {
        OctetString        * tbs_DERcode;  /* Return-Parameter of e_SearchArgumentTBS */
        SearchArgumentTBS  * tbs;
        Signature          * sig;         
};

struct SearchInfo {
	DName    	       * srr_object;
	SET_OF_EntryINFO       * srr_entries;
        PartialOutQual	       * srr_poq;
	CommonRes	       * srr_common;
};

struct SearchResultTBS {
	char			        srr_correlated;
	union {
		SearchInfo	        * searchinfo;
		SET_OF_SearchResult     * uncorrel_searchinfo;
	} srrtbs_un;
};

struct SearchResult {
        OctetString        * tbs_DERcode;  /* Return-Parameter of e_SearchResultTBS */
        SearchResultTBS    * tbs;
        Signature          * sig;         
};


/**************************************************************/

struct SECExtension {
	int	ext_id;
	char	ext_critical;
	PE      ext_item;
};

struct CommonArguments {     /* Common arguments for operations */
    	ServiceControls          * svc;
    	SecurityParameters       * sec_parm;
	DName	                 * requestor;
    	OperationProgress        * progress;
	int                        aliasedRDNs;
#define CA_NO_ALIASDEREFERENCED -1
    	SET_OF_SECExtension      * ext;
};

struct CommonRes {
    	SecurityParameters    * sec_parm;
	DName		      * performer;
        Boolean        		aliasDereferenced;
};

struct SecurityParameters {
	CertificationPath * certPath;
	DName             * name;
	UTCTime		  * time;
	BitString 	  * random;
	int	            target;
};

struct ServiceControls {        	/* represents ServiceControls */
    	int	svc_options;
#define SVC_OPT_PREFERCHAIN             0X001    /*  0000 0000 0000 0001  */
#define SVC_OPT_CHAININGPROHIBIT        0X002    /*  0000 0000 0000 0010  */
#define SVC_OPT_LOCALSCOPE              0X004    /*  0000 0000 0000 0100  */
#define SVC_OPT_DONTUSECOPY             0X008    /*  0000 0000 0000 1000  */
#define SVC_OPT_DONTDEREFERENCEALIAS    0X010    /*  0000 0000 0001 0000  */
    	int	svc_prio;
#define SVC_PRIO_LOW    0
#define SVC_PRIO_MED    1
#define SVC_PRIO_HIGH   2
    	int	svc_timelimit;
#define SVC_NOTIMELIMIT -1
    	int	svc_sizelimit;
#define SVC_NOSIZELIMIT -1
    	int	svc_scopeofreferral;
#define SVC_REFSCOPE_NONE       -1
#define SVC_REFSCOPE_DMD        0
#define SVC_REFSCOPE_COUNTRY    1
	char  * svc_tmp;	/* pepy */
	int	svc_len;	/* pepy */
};

struct OperationProgress {	/* represents OperationProgress */
    	int     opResolutionPhase;
#define OP_PHASE_NOTDEFINED     -1
#define OP_PHASE_NOTSTARTED     1
#define OP_PHASE_PROCEEDING     2
#define OP_PHASE_COMPLETED      3
    	int     opNextRDNToBeResolved;
};

struct AttrAttrTypeCHOICE {
	int     offset;
	union { 
		ObjId           * choice_un_attrtype;
		SET_OF_Attr     * choice_un_attr;  /* only first element within SET is needed */
	} choice_un;
};
		
struct EntryINFO {   	/* Represents EntryInformation           */
    DName                         * ent_dn;
    Boolean            	            ent_fromentry;
    SET_OF_AttrAttrTypeCHOICE     * ent_attr;
};


struct EntryInfoSEL {	  /* EntryInformationSelection */
			  /* Represents EntryInformationSelection */
    char                eis_allattributes;
	                  /* if set to TRUE, all attributes       */
	                  /* returned, if not as per next arg     */
    SET_OF_ObjId      * eis_select;
	                  /* Sequence of attributes used to show  */
	                  /* which TYPES are wanted               */
    int                 eis_infotypes;
#define EIS_ATTRIBUTETYPESONLY 0
#define EIS_ATTRIBUTESANDVALUES 1
};

struct typeDSE_PSAPaddr {
	OctetString 	         pSelector;
	OctetString 	         sSelector;
	OctetString 	         tSelector;
	SET_OF_OctetString     * nAddress;
};

struct AccessPoint {           /* represents AccessPoint               */
    DName                     * ap_name;
    struct typeDSE_PSAPaddr   * ap_address;
};

struct ContReference {
    DName                   * cr_name;
    OperationProgress       * cr_progress;
    int         	      cr_rdn_resolved;
#define CR_RDNRESOLVED_NOTDEFINED       -1
    int         	      cr_aliasedRDNs;
#define CR_NOALIASEDRDNS -1
    int         	      cr_reftype;
#define RT_UNDEFINED -1
#define RT_SUPERIOR 1
#define RT_SUBORDINATE 2
#define RT_CROSS 3
#define RT_NONSPECIFICSUBORDINATE 4
    SET_OF_AccessPoint      * cr_accesspoints;	
 /* result of PEPSY-encoding routine, C-structure too complex ! */
};

#define LSR_NOLIMITPROBLEM      -1
#define LSR_TIMELIMITEXCEEDED   0
#define LSR_SIZELIMITEXCEEDED   1
#define LSR_ADMINSIZEEXCEEDED   2

struct PartialOutQual {
	int                        poq_limitproblem;
	SET_OF_ContReference     * poq_cref;
	Boolean			   poq_no_ext;
};







#ifdef __STDC__


/***************************************************************
 *
 * Headers from file af/strong_ed.c
 *
 ***************************************************************/ 
       OctetString	*e_TokenTBS	(TokenTBS *token_tbs);
       OctetString	*e_AddArgumentTBS	(AddArgumentTBS *addarg_tbs);
       OctetString	*e_CompareArgumentTBS	(CompareArgumentTBS *cmparg_tbs);
       OctetString	*e_CompareResultTBS	(CompareResultTBS *cmpres_tbs);
       OctetString	*e_ListArgumentTBS	(ListArgumentTBS *listarg_tbs);
       OctetString	*e_ListResultTBS	(ListResultTBS *listres_tbs);
       OctetString	*e_ListResult	(ListResult *listres);
       OctetString	*e_ModifyEntryArgumentTBS	(ModifyEntryArgumentTBS *modarg_tbs);
       OctetString	*e_ModifyRDNArgumentTBS	(ModifyRDNArgumentTBS *modrdnarg_tbs);
       OctetString	*e_ReadArgumentTBS	(ReadArgumentTBS *readarg_tbs);
       OctetString	*e_ReadResultTBS	(ReadResultTBS *readres_tbs);
       OctetString	*e_RemoveArgumentTBS	(RemoveArgumentTBS *remarg_tbs);
       OctetString	*e_SearchArgumentTBS	(SearchArgumentTBS *searcharg_tbs);
       OctetString	*e_SearchResultTBS	(SearchResultTBS *searchres_tbs);
       OctetString	*e_SearchResult	(SearchResult *searchres);
       OctetString	*e_SECExtension	(SECExtension *secext);
       OctetString	*e_AttrAttrTypeCHOICE	(AttrAttrTypeCHOICE *choice);
       OctetString	*e_EntryINFO	(EntryINFO *einfo);
       OctetString	*e_Filter	(SFilter *sfilter);
       OctetString	*e_SubordEntry	(SubordEntry *subord);
       OctetString	*e_ContReference	(ContReference *cref);
       OctetString	*e_AccessPoint	(AccessPoint *cref);
       typeDSE_PSAPaddr	*PSAPaddr_dec	(PE pe);
       int	         strong_ed_dummy	();
/***************************************************************
 *
 * Headers from file af/strong_free.c
 *
 ***************************************************************/ 
       void	aux_free2_Attr	(register Attr *attr);
       void	aux_free_Attr	(Attr **attr);
       void	aux_free_AttrValues	(AttrValues **set);
       void	aux_free_SET_OF_Attr	(SET_OF_Attr **set);
       void	aux_free2_AttrAttrTypeCHOICE	(register AttrAttrTypeCHOICE *choice);
       void	aux_free_AttrAttrTypeCHOICE	(AttrAttrTypeCHOICE **choice);
       void	aux_free_SET_OF_AttrAttrTypeCHOICE	(SET_OF_AttrAttrTypeCHOICE **set);
       void	aux_free2_Ava	(register AttrValueAssertion *ava);
       void	aux_free_Ava	(AttrValueAssertion **ava);
       void	aux_free2_EntryINFO	(register EntryINFO *ent);
       void	aux_free_EntryINFO	(EntryINFO **ent);
       void	aux_free_SET_OF_EntryINFO	(SET_OF_EntryINFO **set);
       void	aux_free_SET_OF_ObjId	(SET_OF_ObjId **set);
       void	aux_free2_EntryInfoSEL	(register EntryInfoSEL *eis);
       void	aux_free_EntryInfoSEL	(EntryInfoSEL **eis);
       void	aux_free2_SecurityParameters	(register SecurityParameters *sp);
       void	aux_free_SecurityParameters	(SecurityParameters **sp);
       void	aux_free2_ServiceControls	(register ServiceControls *sc);
       void	aux_free_ServiceControls	(ServiceControls **sc);
       void	aux_free2_CommonArguments	(register CommonArguments *ca);
       void	aux_free_CommonArguments	(CommonArguments **ca);
       void	aux_free2_CommonRes	(register CommonRes *cr);
       void	aux_free_CommonRes	(CommonRes **cr);
       void	aux_free2_SECExtension	(register SECExtension *secext);
       void	aux_free_SECExtension	(SECExtension **secext);
       void	aux_free_SET_OF_SECExtension	(SET_OF_SECExtension **set);
       void	aux_free2_TokenTBS	(register TokenTBS *tbs);
       void	aux_free_TokenTBS	(TokenTBS **tbs);
       void	aux_free2_AddArgumentTBS	(register AddArgumentTBS *tbs);
       void	aux_free_AddArgumentTBS	(AddArgumentTBS **tbs);
       void	aux_free2_CompareArgumentTBS	(register CompareArgumentTBS *tbs);
       void	aux_free_CompareArgumentTBS	(CompareArgumentTBS **tbs);
       void	aux_free2_CompareResultTBS	(register CompareResultTBS *tbs);
       void	aux_free_CompareResultTBS	(CompareResultTBS **tbs);
       void	aux_free2_ListArgumentTBS	(register ListArgumentTBS *tbs);
       void	aux_free_ListArgumentTBS	(ListArgumentTBS **tbs);
       void	aux_free2_SubordEntry	(register SubordEntry *sub);
       void	aux_free_SubordEntry	(SubordEntry **sub);
       void	aux_free_SET_OF_SubordEntry	(SET_OF_SubordEntry **set);
       void	aux_free2_ListInfo	(register ListInfo *info);
       void	aux_free_ListInfo	(ListInfo **info);
       void	aux_free2_ListResultTBS	(register ListResultTBS *tbs);
       void	aux_free_ListResultTBS	(ListResultTBS **tbs);
       void	aux_free2_EntryModification	(register EntryModification *em);
       void	aux_free_EntryModification	(EntryModification **em);
       void	aux_free_SEQUENCE_OF_EntryModification	(SEQUENCE_OF_EntryModification **set);
       void	aux_free2_ModifyEntryArgumentTBS	(register ModifyEntryArgumentTBS *tbs);
       void	aux_free_ModifyEntryArgumentTBS	(ModifyEntryArgumentTBS **tbs);
       void	aux_free2_ModifyRDNArgumentTBS	(register ModifyRDNArgumentTBS *tbs);
       void	aux_free_ModifyRDNArgumentTBS	(ModifyRDNArgumentTBS **tbs);
       void	aux_free2_ReadArgumentTBS	(register ReadArgumentTBS *tbs);
       void	aux_free_ReadArgumentTBS	(ReadArgumentTBS **tbs);
       void	aux_free2_ReadResultTBS	(register ReadResultTBS *tbs);
       void	aux_free_ReadResultTBS	(ReadResultTBS **tbs);
       void	aux_free2_RemoveArgumentTBS	(register RemoveArgumentTBS *tbs);
       void	aux_free_RemoveArgumentTBS	(RemoveArgumentTBS **tbs);
       void	aux_free_SET_OF_PE	(SET_OF_PE **set);
       void	aux_free_OperationProgress	(OperationProgress **op);
       void	aux_free_SET_OF_OctetString	(SET_OF_OctetString **set);
       void	aux_free2_PSAPaddr	(register typeDSE_PSAPaddr *psap);
       void	aux_free_PSAPaddr	(typeDSE_PSAPaddr **psap);
       void	aux_free2_AccessPoint	(register AccessPoint *accpoint);
       void	aux_free_AccessPoint	(AccessPoint **accpoint);
       void	aux_free_SET_OF_AccessPoint	(SET_OF_AccessPoint **set);
       void	aux_free2_ContReference	(register ContReference *cref);
       void	aux_free_ContReference	(ContReference **cref);
       void	aux_free_SET_OF_ContReference	(SET_OF_ContReference **set);
       void	aux_free2_PartialOutQual	(register PartialOutQual *poq);
       void	aux_free_PartialOutQual	(PartialOutQual **poq);
       void	aux_free2_SearchInfo	(register SearchInfo *info);
       void	aux_free_SearchInfo	(SearchInfo **info);
       void	aux_free2_SFilterSubstrings	(register SFilterSubstrings *substrgs);
       void	aux_free_SFilterSubstrings	(SFilterSubstrings **substrgs);
       void	aux_free2_SFilterItem	(register SFilterItem *fi);
       void	aux_free_SFilterItem	(SFilterItem **fi);
       void	aux_free2_SFilter	(register SFilter *flt);
       void	aux_free_SFilter	(SFilter **flt);
       void	aux_free_SET_OF_SFilter	(SET_OF_SFilter **set);
       void	aux_free2_StringsCHOICE	(register StringsCHOICE *choice);
       void	aux_free_StringsCHOICE	(StringsCHOICE **choice);
       void	aux_free_SEQUENCE_OF_StringsCHOICE	(SEQUENCE_OF_StringsCHOICE **seq);
       void	aux_free2_SearchArgumentTBS	(register SearchArgumentTBS *tbs);
       void	aux_free_SearchArgumentTBS	(SearchArgumentTBS **tbs);
       void	aux_free2_SearchResultTBS	(register SearchResultTBS *tbs);
       void	aux_free_SearchResultTBS	(SearchResultTBS **tbs);
       void	aux_free2_Token	(register Token *tok);
       void	aux_free_Token	(Token **tok);
       void	aux_free2_AddArgument	(register AddArgument *addarg);
       void	aux_free_AddArgument	(AddArgument **addarg);
       void	aux_free2_CompareArgument	(register CompareArgument *comparearg);
       void	aux_free_CompareArgument	(CompareArgument **comparearg);
       void	aux_free2_CompareResult	(register CompareResult *compareres);
       void	aux_free_CompareResult	(CompareResult **compareres);
       void	aux_free2_ListArgument	(register ListArgument *listarg);
       void	aux_free_ListArgument	(ListArgument **listarg);
       void	aux_free2_ListResult	(register ListResult *listres);
       void	aux_free_ListResult	(ListResult **listres);
       void	aux_free_SET_OF_ListResult	(SET_OF_ListResult **set);
       void	aux_free2_ModifyEntryArgument	(register ModifyEntryArgument *modifyentryarg);
       void	aux_free_ModifyEntryArgument	(ModifyEntryArgument **modifyentryarg);
       void	aux_free2_ModifyRDNArgument	(register ModifyRDNArgument *modifyrdnarg);
       void	aux_free_ModifyRDNArgument	(ModifyRDNArgument **modifyrdnarg);
       void	aux_free2_ReadArgument	(register ReadArgument *readarg);
       void	aux_free_ReadArgument	(ReadArgument **readarg);
       void	aux_free2_ReadResult	(register ReadResult *readres);
       void	aux_free_ReadResult	(ReadResult **readres);
       void	aux_free2_RemoveArgument	(register RemoveArgument *removearg);
       void	aux_free_RemoveArgument	(RemoveArgument **removearg);
       void	aux_free2_SearchArgument	(register SearchArgument *searcharg);
       void	aux_free_SearchArgument	(SearchArgument **searcharg);
       void	aux_free2_SearchResult	(register SearchResult *searchres);
       void	aux_free_SearchResult	(SearchResult **searchres);
       void	aux_free_SET_OF_SearchResult	(SET_OF_SearchResult **set);
       int	strong_free_dummy	();
/***************************************************************
 *
 * Headers from file af/strong_print.c
 *
 ***************************************************************/ 
       int	strong_fprint_TokenTBS	(FILE *ff, TokenTBS *tok_tbs);
       int	strong_fprint_Token	(FILE *ff, Token *tok);
       int	strong_fprint_AddArgument	(FILE *ff, AddArgument *addarg);
       int	strong_fprint_CompareArgument	(FILE *ff, CompareArgument *comparearg);
       int	strong_fprint_CompareResult	(FILE *ff, CompareResult *compareres);
       int	strong_fprint_ListArgument	(FILE *ff, ListArgument *listarg);
       int	strong_fprint_ListResult	(FILE *ff, ListResult *listres);
       int	strong_fprint_ModifyEntryArgument	(FILE *ff, ModifyEntryArgument *modifyentryarg);
       int	strong_fprint_ModifyRDNArgument	(FILE *ff, ModifyRDNArgument *modifyrdnarg);
       int	strong_fprint_ReadArgument	(FILE *ff, ReadArgument *readarg);
       int	strong_fprint_ReadResult	(FILE *ff, ReadResult *readres);
       int	strong_fprint_RemoveArgument	(FILE *ff, RemoveArgument *removearg);
       int	strong_fprint_SearchArgument	(FILE *ff, SearchArgument *searcharg);
       int	strong_fprint_SearchResult	(FILE *ff, SearchResult *searchres);
       int	strong_fprint_CommonArguments	(FILE *ff, CommonArguments *ca);
       int	strong_fprint_CommonRes	(FILE *ff, CommonRes *cr);
       int	strong_fprint_SecurityParameters	(FILE *ff, SecurityParameters *sec_parm);
       int	aux_fprint_CertificationPath	(FILE *ff, CertificationPath *certpath);
       int	aux_fprint_CertificatePairs	(FILE *ff, CertificatePairs *certpairs);
       int	aux_fprint_CertificatePair	(FILE *ff, CertificatePair *cpair);
       int	aux_fprint_random	(FILE *ff, BitString *random);
       int	strong_fprint_dummy	();

/***************************************************************
 *
 * Headers from file af/strong_int.c
 *
 ***************************************************************/ 

#else

OctetString              * e_Token                  (/* Token *                   */);
Token                    * d_Token                  (/* OctetString *             */);
OctetString              * e_TokenTBS               (/* TokenTBS *                */);
OctetString		 * e_AddArgumentTBS         (/* AddArgumentTBS *          */);
OctetString		 * e_CompareArgumentTBS     (/* CompareArgumentTBS *      */);
OctetString		 * e_CompareResultTBS       (/* CompareResultTBS *        */);
OctetString  	         * e_ListArgumentTBS        (/* ListArgumentTBS *         */);
OctetString  	         * e_ListResultTBS          (/* ListResultTBS *           */);
OctetString  	         * e_ListResult             (/* ListResult *              */);
OctetString		 * e_ModifyEntryArgumentTBS (/* ModifyEntryArgumentTBS *  */);
OctetString		 * e_ModifyRDNArgumentTBS   (/* ModifyRDNArgumentTBS *    */);
OctetString  	         * e_ReadArgumentTBS        (/* ReadArgumentTBS *         */);
OctetString  	         * e_ReadResultTBS          (/* ReadResultTBS *           */);
OctetString		 * e_RemoveArgumentTBS      (/* RemoveArgumentTBS *       */);
OctetString		 * e_SearchArgumentTBS      (/* SearchArgumentTBS *       */);
OctetString		 * e_SearchResultTBS        (/* SearchResultTBS *         */);
OctetString		 * e_SearchResult           (/* SearchResult *            */);
OctetString  		 * e_SECExtension	    (/* SECExtension *            */);
OctetString  		 * e_AttrAttrTypeCHOICE     (/* AttrAttrTypeCHOICE *      */);
OctetString  		 * e_EntryINFO		    (/* EntryINFO *               */);
OctetString  		 * e_Filter		    (/* SFilter *                 */);
OctetString  		 * e_SubordEntry	    (/* SubordEntry *             */);
OctetString  		 * e_ContReference	    (/*	ContReference *	 	  */);
OctetString  		 * e_AccessPoint	    (/*	AccessPoint *	 	  */);

typeDSE_PSAPaddr 	 * PSAPaddr_dec();
			





void aux_free2_SecurityParameters(), aux_free_SecurityParameters();
void aux_free2_ServiceControls(), aux_free_ServiceControls();
void aux_free2_CommonArguments(), aux_free_CommonArguments();
void aux_free2_CommonRes(), aux_free_CommonRes();
void aux_free2_TokenTBS(), aux_free_TokenTBS();
void aux_free2_AddArgumentTBS(), aux_free_AddArgumentTBS();
void aux_free2_CompareArgumentTBS(), aux_free_CompareArgumentTBS();
void aux_free2_CompareResultTBS(), aux_free_CompareResultTBS();
void aux_free2_ListArgumentTBS(), aux_free_ListArgumentTBS();
void aux_free2_ListResultTBS(), aux_free_ListResultTBS();
void aux_free2_ListInfo(), aux_free_ListInfo();
void aux_free2_SubordEntry(), aux_free_SubordEntry(), aux_free_SET_OF_SubordEntry();
void aux_free2_ModifyEntryArgumentTBS(), aux_free_ModifyEntryArgumentTBS();
void aux_free2_EntryModification(), aux_free_EntryModification(), aux_free_SEQUENCE_OF_EntryModification();
void aux_free2_ModifyRDNArgumentTBS(), aux_free_ModifyRDNArgumentTBS();
void aux_free2_ReadArgumentTBS(), aux_free_ReadArgumentTBS();
void aux_free2_ReadResultTBS(), aux_free_ReadResultTBS();
void aux_free2_RemoveArgumentTBS(), aux_free_RemoveArgumentTBS();
void aux_free2_SearchArgumentTBS(), aux_free_SearchArgumentTBS();
void aux_free2_SearchResultTBS(), aux_free_SearchResultTBS();
void aux_free2_SearchInfo(), aux_free_SearchInfo();
void aux_free2_SFilterSubstrings(), aux_free_SFilterSubstrings();
void aux_free2_SFilter(), aux_free_SFilter(), aux_free_SET_OF_SFilter();
void aux_free2_SearchInfo(), aux_free_SearchInfo();
void aux_free2_StringsCHOICE(), aux_free_StringsCHOICE(), aux_free_SEQUENCE_OF_StringsCHOICE();
void aux_free2_SFilterSubstrings(), aux_free_SFilterSubstrings();
void aux_free2_Token(), aux_free_Token();
void aux_free2_AddArgument(), aux_free_AddArgument();
void aux_free2_CompareArgument(), aux_free_CompareArgument();
void aux_free2_CompareResult(), aux_free_CompareResult();
void aux_free2_ListArgument(), aux_free_ListArgument();
void aux_free2_ListResult(), aux_free_ListResult(), aux_free_SET_OF_ListResult();
void aux_free2_ModifyEntryArgument(), aux_free_ModifyEntryArgument();
void aux_free2_ModifyRDNArgument(), aux_free_ModifyRDNArgument();
void aux_free2_ReadArgument(), aux_free_ReadArgument();
void aux_free2_ReadResult(), aux_free_ReadResult();
void aux_free2_RemoveArgument(), aux_free_RemoveArgument();
void aux_free2_SearchArgument(), aux_free_SearchArgument();
void aux_free2_SearchResult(), aux_free_SearchResult(), aux_free_SET_OF_SearchResult();

void aux_free2_SECExtension(), aux_free_SECExtension(), aux_free_SET_OF_SECExtension();
void aux_free2_EntryInfoSEL(), aux_free_EntryInfoSEL();
void aux_free_SET_OF_ObjId();
void aux_free2_EntryINFO(), aux_free_EntryINFO(), aux_free_SET_OF_EntryINFO();
void aux_free_SET_OF_PE(), aux_free_OperationProgress(), aux_free2_ContReference(), aux_free_ContReference();
void aux_free_SET_OF_ContReference(), aux_free2_PartialOutQual(), aux_free_PartialOutQual();

/* IF */
void aux_free2_Attr(), aux_free_Attr(), aux_free_SET_OF_Attr();
void aux_free_AttrValues();
void aux_free2_AttrAttrTypeCHOICE(), aux_free_AttrAttrTypeCHOICE(), aux_free_SET_OF_AttrAttrTypeCHOICE();
void aux_free2_Ava(), aux_free_Ava();



#endif /* __STDC__ */




#endif
