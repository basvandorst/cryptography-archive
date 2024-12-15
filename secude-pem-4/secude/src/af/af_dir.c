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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <sys/stat.h>

#include "config.h"
#include "isoaddrs.h"
#include "psap.h"

#include "af.h"

#include "common.h"
#include "attr.h"
#include "sequence.h"
#include "dua.h"
#include "name.h"
#include "config.h"
#include "nrs_info.h"
#include "oid.h"
#include "AF-types.h"
#include "DAS-types.h"   /*for specifying the S T R O N G argument type*/
#include "af-cdefs.h"
#include "if-cdefs.h"
#include "nrs-cdefs.h"
#include "qu-cdefs.h"
#include "syntaxes.h"

#ifdef STRONG
#include "osisec-stub.h"
#include "secude-stub.h"
#endif

#include "af_quipuif.h"

#define EUNKNOWN       114
#define BUFLEN 4096

static char	stream_contents[LINESIZE];
static char	home_dir[LINESIZE];
static char	quipurc_file[100];
static char	prompt[64];
static char   * result;

static CommonArgs ca = default_common_args;

static DN directory_user_dn = NULLDN;
static char * directory_user_name = CNULL;

#ifdef STRONG
static struct security_parms * ca_security = (struct security_parms *)0;
static struct SecurityServices * dsap_security = (struct SecurityServices * ) 0;
#endif

/* Arguments used by secure_ds_bind(): */
static struct ds_bind_arg       bindarg;
static struct ds_bind_arg       bindresult;
static struct ds_bind_error     binderr;

static 	DN      real_name;
static 	char	Password[LINESIZE] = {'\0'};

static Boolean  store_password_on_PSE = FALSE;

static int        af_x500_count;
static char    ** af_x500_vecptr;

static DN first_intended_recipient;

#ifdef __STDC__

	static int ask_for_username();
	static int sign_bindarg();
	static int cmp_quipu_cert(struct certificate *a, struct certificate *b);
	static int cmp_quipu_cpair(struct certificate_list *a, struct certificate_list *b);
	static struct entrymod *ems_append_local(struct entrymod *a, struct entrymod *b);
	static void ems_part_free_local(struct entrymod	 *emp);
	static int set_error(struct DSError	 error);
	static int set_bind_error(struct ds_bind_error	 error);
	static RC get_credentials();
	static RC set_bindarg();
	static RC set_SecurityParameter();
	static RC verify_bindres();
	static void load_security_functions();
	static RC af_access_Directory(DapOperation dap_op, DName * target, AttributeTYPE attr_type, OctetString * serial, DName * issuer, caddr_t dap_arg);
	static DN secudeDName2quipuDN(DName * dname);

#else

	static int ask_for_username();
	static int sign_bindarg();
	static int cmp_quipu_cert();
	static int cmp_quipu_cpair();
	static struct entrymod *ems_append_local();
	static void ems_part_free_local();
	static int set_error();
	static int set_bind_error();
	static RC get_credentials();
	static RC set_bindarg();
	static RC set_SecurityParameter();
	static RC verify_bindres();
	static void load_security_functions();
	static RC af_access_Directory();
	static DN secudeDName2quipuDN();

#endif


extern AttributeValue 	    AttrV_cpy();
extern Attr_Sequence        as_cpy();
extern AttributeType  	    AttrT_new();
extern Attr_Sequence  	    as_comp_new();
extern AV_Sequence	    avs_comp_new();


/* from dsap/common/certificate.c: */
extern PE 		      cert_enc();
extern struct certificate   * cert_dec();
extern struct certificate   * cert_cpy();

#ifdef STRONG
/* from dsap/common/service.c: */
extern struct random_number * get_quipu_random();
#endif

/* from dsap/common/dn_str.c: */
extern DN dn_dec();
extern DN str2dn();

/* from dsap/common/dn_cpy.c: */
extern DN dn_cpy();

/* from dsap/common/dn_free.c: */
extern void dn_free ();

/* from dsap/common/cpair.c: */
extern struct certificate_list * cpair_cpy();


extern char * myname;  		      /* name of the DSA which is to be accessed */
				      /* "myname" is set in tai_args() in dsap_init() */




/************************** local functions: ******************************/



/***************************************************************
 *
 * Procedure cmp_quipu_cert
 *
 ***************************************************************/
#ifdef __STDC__

static int cmp_quipu_cert(
	struct certificate	 *a,
	struct certificate	 *b
)

#else

static int cmp_quipu_cert(
	a,
	b
)
struct certificate	 *a;
struct certificate	 *b;

#endif

{
	char	* proc = "cmp_quipu_cert";

	if (!a || !b)
		return(1);

	if (a->serial != b->serial)
		return(1);

	return(dn_cmp(a->issuer, b->issuer));
}



/***************************************************************
 *
 * Procedure cmp_quipu_cpair
 *
 ***************************************************************/
#ifdef __STDC__

static int cmp_quipu_cpair(
	struct certificate_list	 *a,
	struct certificate_list	 *b
)

#else

static int cmp_quipu_cpair(
	a,
	b
)
struct certificate_list	 *a;
struct certificate_list	 *b;

#endif

{
	RC	  rcode;
	char	* proc = "cmp_quipu_cpair";

	if (a->cert == (struct certificate *) 0) {
		if (b->cert == (struct certificate *) 0) 
			rcode = 0;
		else 
			rcode = 1;
	} else {
		if (b->cert == (struct certificate *) 0) 
			rcode = 1;
		else 
			rcode = cmp_quipu_cert(a->cert, b->cert);
	}

	if (rcode != 0)
		return (rcode);

	if (a->reverse == (struct certificate *) 0) {
		if (b->reverse == (struct certificate *) 0) 
			rcode = 0;
		else 
			rcode = 1;
	} else {
		if (b->reverse == (struct certificate *) 0) 
			rcode = 1;
		else 
			rcode = cmp_quipu_cert(a->reverse, b->reverse);
	}

	return (rcode);
}


/***************************************************************
 *
 * Procedure secudeDName2quipuDN
 *
 ***************************************************************/
#ifdef __STDC__

static DN secudeDName2quipuDN(
	DName	 *dname
)

#else

static DN secudeDName2quipuDN(
	dname
)
DName	 *dname;

#endif

{
	PE        pe;
	DN        quipu_dn;
	char	* proc = "secudeDName2quipuDN";

	if(! dname){
		aux_add_error(EINVALID, "parameter missing", CNULL, 0, proc);
		return(NULLDN);
	}

	build_SECIF_Name(&pe, 1, 0, NULLCP, dname);
	if (pe == NULLPE) {
		aux_add_error(EENCODE, "Encoding name", CNULL, 0, proc);
		return(NULLDN);
	}

	if ( (quipu_dn = dn_dec(pe)) == NULLDN ) {
		pe_free(pe);
		aux_add_error(EDECODE, "Decoding name", CNULL, 0, proc);
		return(NULLDN);
	}

	pe_free(pe);

	return(quipu_dn);
}


/***************************************************************
 *
 * Procedure af_access_Directory
 *
 ***************************************************************/
#ifdef __STDC__

static RC af_access_Directory(
	DapOperation	  dap_op,
	DName		 *target,
	AttributeTYPE     attr_type,
	OctetString	 *serial,
	DName		 *issuer,
	caddr_t           dap_arg
)

#else

static RC af_access_Directory(
	dap_op,
	target,
	attr_type,
	serial,
	issuer,
	dap_arg
)
DapOperation	  dap_op;
DName		 *target;
AttributeTYPE     attr_type;
OctetString      *serial;  /* used for specifying the cert that is to be removed from Directory */
DName	         *issuer;  /* used for specifying the cert that is to be removed from Directory */
caddr_t           dap_arg;

#endif

{
	/* Arguments used by ds_read(): */
	struct ds_read_arg        read_arg;
	struct DSError            read_error;
	struct ds_read_result     read_result;

	/* Arguments used by ds_modifyentry(): */
	struct ds_modifyentry_arg mod_arg;
	struct DSError            mod_error;

	AV_Sequence               avst_arg = NULLAV;       /*pointer*/
	AV_Sequence    		  avst_result = NULLAV;    /*pointer*/
	AV_Sequence    		  avseq = NULLAV;	   /*pointer*/
	AttributeType  		  at;             	   /*pointer*/
	Attr_Sequence  		  as;                      /*pointer*/
	AttributeValue 		  av;	                   /*pointer*/

	int		          found = 0, int_serial, index;
	DN 			  quipu_issuer;

	struct acl_info 	* acl;
	struct entrymod 	* emnew;

	static Boolean            first = TRUE;

	/* Certificate section */
	SET_OF_Certificate      * certset, * certset_tmp;
	struct certificate      * quipu_cert, * quipu_cert_tmp, * quipu_cert_found;

	/* CrossCertificatePair section */
	CertificatePair         * cpair;
	SET_OF_CertificatePair  * cpairset, * cpairset_tmp;
	struct certificate_list * quipu_cpair, * quipu_cpair_tmp;

	OCList                  * oclist;
	CRL                     * crl;
	RC			  rcode;

	static PS                 rps = NULLPS;

	char                      err_msg[256], attr_name[32], *target_name;
	
	char			* proc = "af_access_Directory";


	if(rps == NULLPS){
		rps = ps_alloc(std_open);
		std_setup(rps, stdout);
	}

	if(first){
		if(af_dir_tailor && af_dir_dsaname) af_x500_count = 5;
		else if(af_dir_tailor || af_dir_dsaname) af_x500_count = 3;
		else af_x500_count = 1; /* default, binding to local DSA, using default dsaptailor */
	
		af_x500_vecptr = (char **)calloc(af_x500_count + 1, sizeof(char *));
		if(! af_x500_vecptr) {
			aux_add_error(EMALLOC, "af_x500_vecptr", CNULL, 0, proc);
			return(- 1);
		}

		af_x500_vecptr[0] = aux_cpy_String("program");
		if(af_dir_dsaname && af_dir_tailor){
			af_x500_vecptr[1] = aux_cpy_String("-call");
			af_x500_vecptr[2] = aux_cpy_String(af_dir_dsaname);
			af_x500_vecptr[3] = aux_cpy_String("-tailor");
			af_x500_vecptr[4] = aux_cpy_String(af_dir_tailor);
			af_x500_vecptr[5] = (char *)0;
		}
		else if(af_dir_tailor){
			af_x500_vecptr[1] = aux_cpy_String("-tailor");
			af_x500_vecptr[2] = aux_cpy_String(af_dir_tailor);
			af_x500_vecptr[3] = (char *)0;
		}
		else if(af_dir_dsaname){
			af_x500_vecptr[1] = aux_cpy_String("-call");
			af_x500_vecptr[2] = aux_cpy_String(af_dir_dsaname);
			af_x500_vecptr[3] = (char *)0;
		}
		first = FALSE;
	}

	if(dap_op != DAP_BIND && dap_op != DAP_UNBIND && dap_op != DAP_ENTER && dap_op != DAP_DELETE && dap_op != DAP_RETRIEVE){
		aux_add_error(EINVALID, "Invalid DAP operation type", CNULL, 0, proc);
		return(- 1);
	}

	if(serial){
		/* Transform "OctetString" representation of serialnumber into "int" representation */
		int_serial = aux_OctetString2int(serial);
		if(int_serial < 0){
			aux_add_error(EINVALID, "Can't transform serialnumber to int", CNULL, 0, proc);
			return(- 1);
		}
	}

	if (issuer) {
		quipu_issuer = secudeDName2quipuDN(issuer);
		if(quipu_issuer == NULLDN){
			aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
			return(- 1);
		}
	}



	/*    B I N D     S E C T I O N    */

	if(dap_op == DAP_BIND && !af_dir_dsabound){
		/* set up the needed function pointers: */
		quipu_syntaxes();
		security_syntaxes();
		dsap_init(&af_x500_count, &af_x500_vecptr);
	
		if(set_bindarg()){
			aux_add_error(EINVALID, "set_bindarg failed", CNULL, 0, proc);
			return(- 1);
		};
	
		if ( secure_ds_bind(&bindarg, &binderr, &bindresult) != OK ) {
			if(af_verbose) {
				fprintf(stderr, "\n");
				ds_bind_error(rps, &binderr);
			}
			dn_free (bindarg.dba_dn);
			bindarg.dba_dn = NULLDN;
			dn_free(directory_user_dn);
			directory_user_dn = NULLDN;
			free(directory_user_name);
			if(af_dir_dsaname) sprintf(err_msg, "Can't contact DSA %s", af_dir_dsaname);
			else sprintf(err_msg, "Can't contact default DSA");
			aux_add_error(set_bind_error(binderr), err_msg, CNULL, 0, proc);
			if(af_dir_authlevel == DBA_AUTH_SIMPLE){
				bindarg.dba_passwd[0] = 0;
				Password[0] = '\0';
			}
			return(- 1);
		}		
	
		if (af_dir_authlevel == DBA_AUTH_SIMPLE && store_password_on_PSE == TRUE) {
			store_password_on_PSE = FALSE;			
			rcode = af_pse_update_QuipuPWD(Password);
			if (rcode < 0) {
				aux_add_error(EWRITEPSE, "Can't write Quipu PWD to PSE", CNULL, 0, proc);
				return(- 1);
			}
		}
	
#ifdef STRONG
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			if(verify_bindres()){
				aux_add_error(EVERIFICATION, "Can't verify bind result", CNULL, 0, proc);
				if(af_verbose) fprintf(stderr, "\nS T R O N G  authentication  f a i l e d !\n\n");
				return(- 1); 
			}
			if(af_verbose) fprintf(stderr, "\nS T R O N G  authentication  s u c c e e d e d !\n\n");
		}
#endif
		af_dir_dsabound = TRUE;

		return(0);
	}
	else if(dap_op == DAP_BIND) return(0);


	if(dap_op == DAP_UNBIND) {
		ds_unbind();
		af_dir_dsabound = FALSE;
		return(0);
	}



	/*   D E L E T E ,    E N T E R,    R E T R I E V E   */

	if(target){
		read_arg.rda_object = secudeDName2quipuDN(target);
		if(read_arg.rda_object == NULLDN){
			aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
			return(- 1);
		}
	}
	else read_arg.rda_object = dn_cpy(directory_user_dn);

	read_arg.rda_eis.eis_infotypes = EIS_ATTRIBUTESANDVALUES;
	read_arg.rda_eis.eis_allattributes = FALSE;


	/* Set up the desired attribute type to be read */

	switch(attr_type){

	case USERCERTIFICATE:
		strcpy(attr_name, "userCertificate");
		at = AttrT_new(attr_name);
		break;

	case CACERTIFICATE:
		strcpy(attr_name, "cACertificate");
		at = AttrT_new(attr_name);
		break;

	case REVLIST:
		strcpy(attr_name, "pemCRL");
		at = AttrT_new(attr_name);
		break;

	case CERTIFICATEPAIR:
		strcpy(attr_name, "crossCertificatePair");
		at = AttrT_new(attr_name);
		break;

	case OCLIST:
		strcpy(attr_name, "oldCertificateList");
		at = AttrT_new(attr_name);
		break;

	}

	if (at == NULLAttrT) {
		dn_free (read_arg.rda_object);
		aux_add_error(EINVALID, "Unknown attribute type", CNULL, 0, proc);
		return (- 1);
	}


	/*  Only the attribute types of the Attr_Sequence structure need to be set
	 *  (see Volume5, p.213)
	 */

	as = as_comp_new(AttrT_cpy(at), NULLAV, NULLACL_INFO);
	read_arg.rda_eis.eis_select = as_cpy(as);
	read_result.rdr_entry.ent_attr = NULLATTR;

	read_arg.rda_common = ca;
	read_arg.rda_common.ca_requestor = directory_user_dn;


#ifdef STRONG

	/****  S I G N  ReadArgument  ****/

	if(af_dir_authlevel == DBA_AUTH_STRONG){
		if(set_SecurityParameter() == NOTOK){
			ds_unbind();
			af_dir_dsabound = FALSE;
			dn_free(read_arg.rda_object);
			aux_add_error(EINVALID, "set_SecurityParameter failed", CNULL, 0, proc);
			return(- 1);
		}
		read_arg.rda_common.ca_security = ca_security;
		if (af_verbose) fprintf(stderr, "\n\n\n\n\n");
		read_arg.rda_common.ca_sig = (dsap_security->serv_sign)((caddr_t)&read_arg, _ZReadArgumentDataDAS, NULLALGID);
		if(! read_arg.rda_common.ca_sig){
			dn_free(read_arg.rda_object);
			aux_add_error(EINVALID, "Cannot sign read argument", CNULL, 0, proc);
			return(- 1);
		}
	}
	read_result.rdr_common.cr_sig = (struct signature * )0;
	read_result.rdr_common.cr_security == (struct security_parms *) 0;
#endif
	
	if(dap_op == DAP_ENTER && (attr_type == CERTIFICATEPAIR || attr_type == REVLIST || attr_type == OCLIST)){
		/* READ operation in order to find out whether an attribute of type
		   "crossCertificatePair" already exists in the specified entry */
	
#ifdef ICR1
		if ( ds_read(&read_arg, &read_error, &read_result) != DS_OK ) {
			if ( (read_error.dse_type != DSE_ATTRIBUTEERROR) || 
			    (read_error.dse_un.dse_un_attribute.DSE_at_plist && read_error.dse_un.dse_un_attribute.DSE_at_plist->DSE_at_what != DSE_AT_NOSUCHATTRIBUTE) ) {
#else
		if ( ds_read(&read_arg, &read_error, &read_result) != DS_OK ) {
			if ( (read_error.dse_type != DSE_ATTRIBUTEERROR) || 
		    	    (read_error.dse_un.dse_un_attribute.DSE_at_plist.DSE_at_what != DSE_AT_NOSUCHATTRIBUTE) ) {
#endif
				if(af_verbose) {
					fprintf(stderr, "\n");
					ds_error(rps, &read_error);
				}
				dn_free (read_arg.rda_object);
				target_name = aux_DName2Name(target);
				sprintf(err_msg, "Can't read attribute %s from entry <%s>", attr_name, target_name);
				aux_add_error(set_error(read_error), err_msg, CNULL, 0, proc);
				free(target_name);
				return(- 1);
			}
		}
	}
	else{
		if ( ds_read(&read_arg, &read_error, &read_result) != DS_OK ) {
			if(af_verbose) {
				fprintf(stderr, "\n");
				ds_error(rps, &read_error);
			}
			dn_free (read_arg.rda_object);
			target_name = aux_DName2Name(target);
			sprintf(err_msg, "Can't read attribute %s from entry <%s>", attr_name, target_name);
			aux_add_error(set_error(read_error), err_msg, CNULL, 0, proc);
			free(target_name);
			return(- 1);
		}
	}

#ifdef STRONG

	/****  V E R I F Y  ReadResult  ****/

	if(read_result.rdr_common.cr_sig){  /*read_result is SIGNED and must be evaluated*/
		/* Policy : signed messages must have security parameters present. */
		  if (read_result.rdr_common.cr_security == (struct security_parms *) 0){
			dn_free(read_arg.rda_object);
			aux_add_error(EINVALID, "Security Policy Violation: No security parameters present", CNULL, 0, proc);
			return(- 1);
		}
		if (dsap_security && dsap_security->serv_ckpath && dsap_security->serv_cknonce){
			if (af_verbose) fprintf(stderr, "\n\n\n\n\n");
			rcode = (dsap_security->serv_ckpath) 
				((caddr_t) &read_result, read_result.rdr_common.cr_security->sp_path, read_result.rdr_common.cr_sig, &real_name, _ZReadResultDataDAS);
			if(af_verbose) fprintf(stderr, "\n\n\n\n\n");
			/* CHECK whether real_name is needed within "serv_ckpath" !!!*/
			if (rcode != OK){
				dn_free(read_arg.rda_object);
				aux_add_error(EVERIFICATION, "Cannot verify signature applied to read_result", CNULL, 0, proc);
				return(- 1);
			}
		}
	}
#endif

	switch(dap_op){

	case DAP_RETRIEVE:

		dn_free (read_arg.rda_object);


		/* Evaluate ReadResult */

		switch (attr_type){

		case USERCERTIFICATE:
		case CACERTIFICATE:

			/*  The cACertificate attribute is MANDATORY within the directory entry of
			 *  a certificationAuthority, and the userCertificate attribute MANDATORY within
			 *  the directory entry of a strongAuthenticationUser; it has multiple value, namely
			 *  one or more EncrCertificates and one or more SignCertificates.
			 */

			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return (- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
		
			if ( avst_result == NULLAV ) {
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return (- 1);
			}
		
			quipu_cert = (struct certificate *)avst_result->avseq_av.av_struct;
		
			if ( !(certset = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate))) ) {
				aux_add_error(EMALLOC, "certset", CNULL, 0, proc);
				return(- 1);
			}

			certset->element = quipuCert2secudeCert(quipu_cert);
			if(! certset->element){
				aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
				return(- 1);
			}

			certset->next = (SET_OF_Certificate * )0;
			certset_tmp = certset;
		
		
			/*  The requested attribute has multiple value; the values are stored within a set 
			 *  of certificates:
			 */
			for ( avseq = avst_result->avseq_next; avseq ; avseq = avseq->avseq_next ) {
		
				if ( !(certset_tmp->next = (SET_OF_Certificate * )malloc(sizeof(SET_OF_Certificate))) ) {
					aux_add_error(EMALLOC, "certset_tmp->next", CNULL, 0, proc);
					return(- 1);
				}
		
				certset_tmp = certset_tmp->next;
				certset_tmp->next = (SET_OF_Certificate * )0;

				quipu_cert = (struct certificate *)avseq->avseq_av.av_struct;
		
				certset_tmp->element = quipuCert2secudeCert(quipu_cert);
				if(! certset_tmp->element){
					aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
					return(- 1);
				}		
			}    /*for*/

			result = (caddr_t) certset;
			break;

		
		case REVLIST:

			/*  The "pemCRL" attribute is OPTIONAL within the directory entry of a 
			 *  certification authority.
			 */
		
			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return (- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
		
			if ( avst_result == NULLAV ) {
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return (- 1);
			}

			result = avst_result->avseq_av.av_struct;
			break;


		case CERTIFICATEPAIR:
			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return(- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
		
			if ( avst_result == NULLAV ) {
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return(- 1);
			}
		
			if ((quipu_cpair = (struct certificate_list *)avst_result->avseq_av.av_struct)
			     == (struct certificate_list *)0) {
				aux_add_error(EINVALID, "quipu_cpair == 0", CNULL, 0, proc);
				return(- 1);
			}
		
			if ( !(cpairset = (SET_OF_CertificatePair * )malloc(sizeof(SET_OF_CertificatePair))) ) {
				aux_add_error(EMALLOC, "cpairset", CNULL, 0, proc);
				return(- 1);
			}
		
			if ( !(cpairset->element = (CertificatePair * )malloc(sizeof(CertificatePair))) ) {
				aux_add_error(EMALLOC, "cpairset->element", CNULL, 0, proc);
				return(- 1);
			}
		
			/* quipu_cpair->cert may be empty */
			if (!quipu_cpair->cert)
				cpairset->element->forward = (Certificate * )0;
			else {
				cpairset->element->forward = quipuCert2secudeCert(quipu_cpair->cert);
				if(! cpairset->element->forward){
					aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
					return(- 1);
				}
			}
		
			/* quipu_cpair->reverse may be empty */
			if (!quipu_cpair->reverse)
				cpairset->element->reverse = (Certificate * )0;
			else {
				cpairset->element->reverse = quipuCert2secudeCert(quipu_cpair->reverse);
				if(! cpairset->element->reverse){
					aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
					return(- 1);
				}
			}
		
			cpairset->next = (SET_OF_CertificatePair * )0;
			cpairset_tmp = cpairset;
		
			/*  The requested attribute has multiple value; the values are stored within a set 
			 *  of certificate pairs:
			 */
			for (avseq = avst_result->avseq_next; avseq; avseq = avseq->avseq_next ) {
		
				if ((quipu_cpair = (struct certificate_list *)avseq->avseq_av.av_struct)
				     == (struct certificate_list *)0) {
					return (- 1);
				}
		
				if ( !(cpairset_tmp->next = (SET_OF_CertificatePair * )malloc(sizeof(SET_OF_CertificatePair))) ) {
					aux_add_error(EMALLOC, "cpairset_tmp->next", CNULL, 0, proc);
					return(- 1);
				}
		
				cpairset_tmp = cpairset_tmp->next;
				cpairset_tmp->next = (SET_OF_CertificatePair * )0;
		
				if ( !(cpairset_tmp->element = (CertificatePair * )malloc(sizeof(CertificatePair))) ) {
					aux_add_error(EMALLOC, "cpairset_tmp->element", CNULL, 0, proc);
					return(- 1);
				}
		
				/* quipu_cpair->cert may be empty */
				if (!quipu_cpair->cert)
					cpairset_tmp->element->forward = (Certificate * )0;
				else {
					cpairset_tmp->element->forward = quipuCert2secudeCert(quipu_cpair->cert);
					if(! cpairset_tmp->element->forward){
						aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
						return(- 1);
					}
				}
		
				/* quipu_cpair->reverse may be empty */
				if (!quipu_cpair->reverse)
					cpairset_tmp->element->reverse = (Certificate * )0;
				else {
					cpairset_tmp->element->reverse = quipuCert2secudeCert(quipu_cpair->reverse);
					if(! cpairset_tmp->element->reverse){
						aux_add_error(EINVALID, "Cannot transform Quipu-certificate representation into SecuDE representation", CNULL, 0, proc);
						return(- 1);
					}
				}
		
			}    /*for*/

			result = (caddr_t) cpairset;
			break;


		case OCLIST:
			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return (- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
		
			if ( avst_result == NULLAV ) {
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return (- 1);
			}

			result = avst_result->avseq_av.av_struct;
			break;

		} /* switch attr_type */

		break;


	case DAP_DELETE:

		mod_arg.mea_changes = NULLMOD;

		switch (attr_type){

		case USERCERTIFICATE:
		case CACERTIFICATE:

			/*  The cACertificate attribute is MANDATORY within the directory entry of
			 *  a certificationAuthority, and the userCertificate attribute MANDATORY within
			 *  the directory entry of a strongAuthenticationUser; it has multiple value, namely
			 *  one or more EncrCertificates and one or more SignCertificates.
			 */
			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return(- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
			acl = read_result.rdr_entry.ent_attr->attr_acl;
		
			if ( avst_result == NULLAV ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return(- 1);
			}
		
			if (issuer) {
				for ( avseq = avst_result, found = 0 ; avseq ; avseq = avseq->avseq_next ) {
					quipu_cert_tmp = (struct certificate *)avseq->avseq_av.av_struct;
					if ( !dn_cmp(quipu_cert_tmp->issuer, quipu_issuer) && 
					    (int_serial == quipu_cert_tmp->serial) ) {    /*equal*/
						quipu_cert_found = quipu_cert_tmp;
						found = 1;
						break;
					}
				}  /*for*/
			} 
			else {
				for ( avseq = avst_result, found = 0 ; avseq ; avseq = avseq->avseq_next ) {
					quipu_cert_tmp = (struct certificate *)avseq->avseq_av.av_struct;
					if (int_serial == quipu_cert_tmp->serial) {
						if (!found) {
							quipu_cert_found = quipu_cert_tmp;
							found = 1;
						}
						else {
							dn_free (read_arg.rda_object);
							aux_add_error(EOBJ, "More than one certificate with specified serial number", CNULL,
							    0, proc);
							return(- 1);
						}
					}
				}  /*for*/
			}
		
			if ( !found ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EOBJNAME, "Specified certificate does not exist in Your directory entry", CNULL, 0, proc);
				return(- 1);
			}
		
			emnew = em_alloc();
			if (!emnew) {
				dn_free (read_arg.rda_object);
				aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
				return(- 1);
			}
			emnew->em_type = EM_REMOVEVALUES;
			av = AttrV_alloc();
			if (!av) {
				dn_free (read_arg.rda_object);
				aux_add_error(EMALLOC, "av", CNULL, 0, proc);
				return(- 1);
			}
			av->av_struct = (caddr_t) cert_cpy(quipu_cert_found);
			av->av_syntax = avst_result->avseq_av.av_syntax;
			break;


		case CERTIFICATEPAIR:

			cpair = (CertificatePair *) dap_arg;

			/* Transform SecuDE-type "CertificatePair" into Quipu-type "certificate_list" */
			/* by only considering the "issuer" and "serial number" components	      */
		
			if ( !(quipu_cpair = (struct certificate_list *)malloc(sizeof(struct certificate_list ))) ) {
				aux_add_error(EMALLOC, "quipu_cpair", CNULL, 0, proc);
				return(- 1);
			}
		
			if (cpair->forward) {
				if ( !(quipu_cpair->cert = (struct certificate *) calloc (1, sizeof(struct certificate ))) ) {
					aux_add_error(EMALLOC, "quipu_cpair->cert", CNULL, 0, proc);
					return(- 1);
				}
				quipu_cpair->cert->serial = aux_OctetString2int(cpair->forward->tbs->serialnumber);
				if(quipu_cpair->cert->serial < 0){
					aux_add_error(EINVALID, "aux_OctetString2int failed", CNULL, 0, proc);
					return(- 1);
				}
		
				quipu_cpair->cert->issuer = secudeDName2quipuDN(cpair->forward->tbs->issuer);
				if(quipu_cpair->cert->issuer == NULLDN){
					aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
					return(- 1);
				}
		
			} else /* cpair->forward may be the NULL pointer */
				quipu_cpair->cert = (struct certificate *)0;
		
			if (cpair->reverse) {
				if ( !(quipu_cpair->reverse = (struct certificate *) calloc (1, sizeof(struct certificate ))) ) {
					aux_add_error(EMALLOC, "quipu_cpair->reverse", CNULL, 0, proc);
					return(- 1);
				}
				quipu_cpair->reverse->serial = aux_OctetString2int(cpair->reverse->tbs->serialnumber);
				if(quipu_cpair->reverse->serial < 0){
					aux_add_error(EINVALID, "aux_OctetString2int failed", CNULL, 0, proc);
					return(- 1);
				}
		
				quipu_cpair->reverse->issuer = secudeDName2quipuDN(cpair->reverse->tbs->issuer);
				if(quipu_cpair->reverse->issuer == NULLDN){
					aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
					return(- 1);
				}
		
			} else /* cpair->reverse may be the NULL pointer */
				quipu_cpair->reverse = (struct certificate *)0;
		
			quipu_cpair->next = (struct certificate_list * )0;
			quipu_cpair->prev = (struct certificate_list * )0;
			quipu_cpair->superior = (struct certificate_list * )0;

			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
/*				ds_unbind(); */
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return(- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
			acl = read_result.rdr_entry.ent_attr->attr_acl;
		
			if ( avst_result == NULLAV ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return(- 1);
			}
		
			for ( avseq = avst_result, found = 0 ; avseq ; avseq = avseq->avseq_next ) {
				quipu_cpair_tmp = (struct certificate_list *)avseq->avseq_av.av_struct;
				if ( !cmp_quipu_cpair(quipu_cpair, quipu_cpair_tmp)) {    /*equal*/
					found = 1;
					break;
				}
			}  /*for*/
		
			if ( !found ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EOBJNAME, "Specified CertificatePair does not exist in your directory entry", CNULL, 0, proc);
				return(- 1);
			}
		
			emnew = em_alloc();
			if (!emnew) {
				dn_free (read_arg.rda_object);
				aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
				return(- 1);
			}
			if (avst_result->avseq_next) 
				emnew->em_type = EM_REMOVEVALUES;
			else 
				emnew->em_type = EM_REMOVEATTRIBUTE;
			av = AttrV_alloc();
			if (!av) {
				dn_free (read_arg.rda_object);
				aux_add_error(EMALLOC, "av", CNULL, 0, proc);
				return(- 1);
			}
			av->av_struct = (caddr_t) cpair_cpy(quipu_cpair_tmp);
			av->av_syntax = avst_result->avseq_av.av_syntax;
			break;

		}  /* switch attr_type */

		break;


	case DAP_ENTER:

		mod_arg.mea_changes = NULLMOD;

		switch (attr_type){

		case USERCERTIFICATE:
		case CACERTIFICATE:

			quipu_cert = secudeCert2quipuCert((Certificate *) dap_arg);
			if(! quipu_cert){
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "Cannot transform SecuDE-certificate representation into Quipu representation", CNULL, 0, proc);
				return(- 1);
			}

			/*  The cACertificate attribute is MANDATORY within the directory entry of
			 *  a certificationAuthority, and the userCertificate attribute MANDATORY within
			 *  the directory entry of a strongAuthenticationUser; it has multiple value, namely
			 *  one or more EncrCertificates and one or more SignCertificates.
			 */
			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "read_result.rdr_entry.ent_attr == NULLATTR", CNULL, 0, proc);
				return(- 1);
			}
		
			avst_result = read_result.rdr_entry.ent_attr->attr_value;
			acl = read_result.rdr_entry.ent_attr->attr_acl;
		
			if ( avst_result == NULLAV ) {
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "avst_result == NULLAV", CNULL, 0, proc);
				return(- 1);
			}

			for ( avseq = avst_result ; avseq ; avseq = avseq->avseq_next ) {
				quipu_cert_tmp = (struct certificate *)avseq->avseq_av.av_struct;
		
				if ( !cert_cmp(quipu_cert_tmp, quipu_cert) ) {    /*equal*/
					dn_free (read_arg.rda_object);
					aux_add_error(ECREATEOBJ, "Specified certificate already exists in Your directory entry", CNULL, 0, proc);
					return(- 1);
				}
			}  /*for*/
		
			emnew = em_alloc();
			if (!emnew) {
				dn_free (read_arg.rda_object);
				aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
				return(- 1);
			}
			emnew->em_type = EM_ADDVALUES;
			av = AttrV_alloc();
			if (!av) {
				dn_free (read_arg.rda_object);
				aux_add_error(EMALLOC, "av", CNULL, 0, proc);
				return(- 1);
			}
			av->av_struct = (caddr_t) cert_cpy(quipu_cert);
			av->av_syntax = avst_result->avseq_av.av_syntax;
			break;


		case REVLIST:

			/*  The "pemCRL" attribute is OPTIONAL within the directory entry of a 
			 *  certification authority.
			 */
				
			crl = (CRL *) dap_arg;

			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				/* create new attribute */
				emnew = em_alloc();
				if (!emnew) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
					return(- 1);
				}
				emnew->em_type = EM_ADDATTRIBUTE;
				av = AttrV_alloc();
				if (!av) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "av", CNULL, 0, proc);
					return(- 1);
				}
				av->av_struct = (caddr_t) aux_cpy_CRL(crl);
				av->av_syntax = at->oa_syntax;
				acl = NULLACL_INFO;
			}    /*if*/ 
			else {
				avst_result = read_result.rdr_entry.ent_attr->attr_value;
				acl = read_result.rdr_entry.ent_attr->attr_acl;
		
				if ( avst_result == NULLAV ) {
					/* add value */
					emnew = em_alloc();
					if (!emnew) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
						return(- 1);
					}
					emnew->em_type = EM_ADDVALUES;
					av = AttrV_alloc();
					if (!av) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "av", CNULL, 0, proc);
						return(- 1);
					}
					av->av_struct = (caddr_t) aux_cpy_CRL(crl);
					av->av_syntax = avst_result->avseq_av.av_syntax;
				} 
				else {
					/* replace old by new value */
					emnew = em_alloc();
					if (!emnew) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
						return(- 1);
					}
					emnew->em_type = EM_REMOVEVALUES;
					avst_arg = avs_comp_new(AttrV_cpy(&avst_result->avseq_av));
					emnew->em_what = as_comp_new(AttrT_cpy(at), avst_arg, acl);
					emnew->em_next = NULLMOD;
					if ( emnew != NULLMOD )
						mod_arg.mea_changes = ems_append_local (mod_arg.mea_changes, emnew);
		
					emnew = em_alloc();
					if (!emnew) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
						return(- 1);
					}
					emnew->em_type = EM_ADDVALUES;
					av = AttrV_alloc();
					if (!av) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "av", CNULL, 0, proc);
						return(- 1);
					}
					av->av_struct = (caddr_t) aux_cpy_CRL(crl);
					av->av_syntax = avst_result->avseq_av.av_syntax;
				}   /*else*/
			}    /*else*/
			break;


		case CERTIFICATEPAIR:

			/*  The crossCertificatePair attribute is OPTIONAL within the directory entry of
			 *  a CA; if it is not present, it is to be created as a new attribute in the
			 *  directory entry of the named CA:
			 */
		
			quipu_cpair = secudeCPair2quipuCPair((CertificatePair *) dap_arg);
			if(! quipu_cpair){
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "Cannot transform SecuDE-CertificatePair representation into Quipu representation", CNULL, 0, proc);
				return(- 1);
			}

			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {     /* create new attribute */
				emnew = em_alloc();
				if (!emnew) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
					return(- 1);
				}
				emnew->em_type = EM_ADDATTRIBUTE;
				av = AttrV_alloc();
				if (!av) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "av", CNULL, 0, proc);
					return(- 1);
				}
				av->av_struct = (caddr_t) cpair_cpy(quipu_cpair);
				av->av_syntax = at->oa_syntax;
				acl = NULLACL_INFO;
			}    /*if*/ 
			else {	/* add value */
				acl = read_result.rdr_entry.ent_attr->attr_acl;
				emnew = em_alloc();
				if (!emnew) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
					return(- 1);
				}
				emnew->em_type = EM_ADDVALUES;
				av = AttrV_alloc();
				if (!av) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "av", CNULL, 0, proc);
					return(- 1);
				}
				av->av_struct = (caddr_t) cpair_cpy(quipu_cpair);
				av->av_syntax = at->oa_syntax;
			}    /*else*/
			break;


		case OCLIST:

			/*  The oldCertificateList attribute is OPTIONAL within the directory entry of
			 *  a CA; if it is not present, it is to be created as a new attribute in the
			 *  directory entry of the named CA:
			 */

			oclist = (OCList *) dap_arg;
		
			if ( read_result.rdr_entry.ent_attr == NULLATTR ) {
				/* create new attribute */
				emnew = em_alloc();
				if (!emnew) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
					return(- 1);
				}
				emnew->em_type = EM_ADDATTRIBUTE;
				av = AttrV_alloc();
				if (!av) {
					dn_free (read_arg.rda_object);
					aux_add_error(EMALLOC, "av", CNULL, 0, proc);
					return(- 1);
				}
				av->av_struct = (caddr_t) aux_cpy_OCList(oclist);
				av->av_syntax = at->oa_syntax;
				acl = NULLACL_INFO;
			}    /*if*/ 
			else {
				avst_result = read_result.rdr_entry.ent_attr->attr_value;
				acl = read_result.rdr_entry.ent_attr->attr_acl;
		
				if ( avst_result == NULLAV ) {
					/* add value */
					emnew = em_alloc();
					if (!emnew) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
						return(- 1);
					}
					emnew->em_type = EM_ADDVALUES;
					av = AttrV_alloc();
					if (!av) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "av", CNULL, 0, proc);
						return(- 1);
					}
					av->av_struct = (caddr_t) aux_cpy_OCList(oclist);
					av->av_syntax = avst_result->avseq_av.av_syntax;
				}
				else {
					/* replace old by new value */
					emnew = em_alloc();
					if (!emnew) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
						return(- 1);
					}
					emnew->em_type = EM_REMOVEVALUES;
					avst_arg = avs_comp_new(AttrV_cpy(&avst_result->avseq_av));
					emnew->em_what = as_comp_new(AttrT_cpy(at), avst_arg, acl);
					emnew->em_next = NULLMOD;
					if ( emnew != NULLMOD )
						mod_arg.mea_changes = ems_append_local (mod_arg.mea_changes, emnew);
		
					emnew = em_alloc();
					if (!emnew) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "emnew", CNULL, 0, proc);
						return(- 1);
					}
					emnew->em_type = EM_ADDVALUES;
					av = AttrV_alloc();
					if (!av) {
						dn_free (read_arg.rda_object);
						aux_add_error(EMALLOC, "av", CNULL, 0, proc);
						return(- 1);
					}
					av->av_struct = (caddr_t) aux_cpy_OCList(oclist);
					av->av_syntax = avst_result->avseq_av.av_syntax;
				}   /*else*/
		
			}    /*else*/
			break;

		}  /* switch attr_type */

		break;

	}   /* switch dap_op */



	/*   M O D I F Y   is applicable to    E N T E R   and    D E L E T E   */

	switch (dap_op){

	case DAP_DELETE:
	case DAP_ENTER:

		avst_arg = avs_comp_new(av);
		emnew->em_what = as_comp_new(AttrT_cpy(at), avst_arg, acl);
		emnew->em_next = NULLMOD;
		if ( emnew != NULLMOD )
			mod_arg.mea_changes = ems_append_local (mod_arg.mea_changes, emnew);
		mod_arg.mea_object = read_arg.rda_object;
	
		mod_arg.mea_common = ca;
		mod_arg.mea_common.ca_requestor = directory_user_dn;

#ifdef STRONG

		/****  S I G N  ModifyentryArgument  ****/
	
		if(af_dir_authlevel == DBA_AUTH_STRONG){
			if(set_SecurityParameter() == NOTOK){
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "set_SecurityParameter failed", CNULL, 0, proc);
				return(- 1);
			}
			mod_arg.mea_common.ca_security = ca_security;
			if(af_verbose) fprintf(stderr, "\n\n\n\n\n");
			mod_arg.mea_common.ca_sig = (dsap_security->serv_sign)((caddr_t)&mod_arg, _ZModifyEntryArgumentDataDAS, NULLALGID);
			if(! mod_arg.mea_common.ca_sig){
				dn_free (read_arg.rda_object);
				aux_add_error(EINVALID, "Cannot sign mod_arg", CNULL, 0, proc);
				return(- 1);
			}
		}
#endif


		if ( ds_modifyentry(&mod_arg, &mod_error) != DS_OK ) {
			if(af_verbose) {
				fprintf(stderr, "\n");
				ds_error(rps, &mod_error);
			}
			dn_free (read_arg.rda_object);
			target_name = aux_DName2Name(target);
			sprintf(err_msg, "Can't modify attribute %s of entry <%s>", attr_name, target_name);
			aux_add_error(set_error(mod_error), err_msg, CNULL, 0, proc);
			free(target_name);
			return(- 1);
		}
	
		ems_part_free_local(mod_arg.mea_changes);
		dn_free (read_arg.rda_object);

		break;

	default:
		break;

	}  /* switch dap_op */


	return(OK);
}



/***************************************************************
 *
 * Procedure ems_append_local
 *
 ***************************************************************/
#ifdef __STDC__

static struct entrymod *ems_append_local(
	struct entrymod	 *a,
	struct entrymod	 *b
)

#else

static struct entrymod *ems_append_local(
	a,
	b
)
struct entrymod	 *a;
struct entrymod	 *b;

#endif

{
	struct entrymod * ptr;
	char	        * proc = "ems_append_local";

	if ((ptr = a) == NULLMOD)
		return b;

	for ( ; ptr->em_next != NULLMOD; ptr = ptr->em_next)
		;

	ptr->em_next = b;
	return a;
}


/***************************************************************
 *
 * Procedure ems_part_free_local
 *
 ***************************************************************/
#ifdef __STDC__

static void ems_part_free_local(
	struct entrymod	 *emp
)

#else

static void ems_part_free_local(
	emp
)
struct entrymod	 *emp;

#endif

{
	char  * proc = "ems_part_free_local";

	if (emp == NULLMOD)
		return;
	ems_part_free_local(emp->em_next);
	free((char *)emp);
}


/***************************************************************
 *
 * Procedure set_error
 *
 ***************************************************************/
#ifdef __STDC__

static int set_error(
	struct DSError	  error
)

#else

static int set_error(
	error
)
struct DSError	  error;

#endif

{

	switch (error.dse_type) {

	case DSE_ATTRIBUTEERROR:
		return (EATTRDIR);

	case DSE_NAMEERROR:
		return (ENAMEDIR);

	case DSE_SERVICEERROR:
		return (ENODIR);

	case DSE_SECURITYERROR:
		return (EACCDIR);

	case DSE_UPDATEERROR:
		return (EUPDATE);

	}  /*switch*/

}


/***************************************************************
 *
 * Procedure set_bind_error
 *
 ***************************************************************/
#ifdef __STDC__

static int set_bind_error(
	struct ds_bind_error	  error
)

#else

static int set_bind_error(
	error
)
struct ds_bind_error	  error;

#endif

{
	switch (error.dbe_type) {
	case DBE_TYPE_SERVICE:
		return(ENODIR);
	case DBE_TYPE_SECURITY:
		return(EACCDIR);
	default:
		return(EUNKNOWN);
	}
}


/************************************************************************************************************ 
 *  get_credentials() returns:										    *
 *													    *
 *													    *
 * 	directory user's distinguished name and password in case of SIMPLE authentication (DBA_AUTH_SIMPLE);* 
 *													    *
 *	directory user's distinguished name only in case of STRONG authentication (DBA_AUTH_STRONG).        *
 *													    *
 ************************************************************************************************************/

/***************************************************************
 *
 * Procedure get_credentials
 *
 ***************************************************************/
#ifdef __STDC__

static RC get_credentials(
)

#else

static RC get_credentials(
)

#endif

{

	PE 	        pe;
	char	        Dirname[LINESIZE];
	struct passwd * pw_entry;
	int	        uid ;
	FILE          * fp_quipurc;
	char	      * p;
	char          * dd = CNULL;
	int	        ind = 0;
	RC		rcode;
	char	      * proc = "get_credentials";

	if(! directory_user_dn && directory_user_dname) { /* "directory_user_dname" may have been provided by a util-routine */
		directory_user_name = aux_DName2Name(directory_user_dname);
		if(af_verbose) fprintf(stderr, "\nBinding as \"%s\"\n", directory_user_name);

		directory_user_dn = secudeDName2quipuDN(directory_user_dname);
		if(directory_user_dn == NULLDN){
			aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
			return(- 1);
		}

		if(af_dir_authlevel == DBA_AUTH_STRONG) return(OK);
	}

	uid = getuid();

	if ((pw_entry = getpwuid(uid)) == (struct passwd *) 0) {
		aux_add_error(EINVALID, "Who are you? (no name for your uid number)", CNULL, 0, proc);
		return(NOTOK);
	}

#ifndef MAC
	if (getenv("HOME") == CNULL) 
#else
    if (MacGetEnv("HOME") == 0) 
#endif /* MAC */
	{
		if(af_verbose) fprintf(stderr, "No home directory?!!\n");
		strcpy(home_dir, pw_entry->pw_dir);
	}
	else strcpy(home_dir, getenv("HOME"));

	strcpy(quipurc_file, home_dir);
	strcat(quipurc_file, "/.quipurc");

	/*  If possible, read the Directory-name and -password used for binding from the 
	 *  invoker's .quipurc file; otherwise, Directory-name and -password have to be entered
	 *  interactively.
 	 */

	/* read from .quipurc file */

	if ( (fp_quipurc = fopen(quipurc_file, "r")) != (FILE *) 0 ) {
		while ( fgets(stream_contents, LINESIZE, fp_quipurc) != CNULL ) {

			/* read user's distinguished name */

			if(! directory_user_dn){
				if ( strstr(stream_contents, "username") != CNULL ) {
					p = strchr(strstr(stream_contents, "username"), ':');
					p += 2;
					Dirname[0] = 0;
					while ( (Dirname[ind++] = *p++) != '\n' ) {
					}
					ind--;
					Dirname[ind] = '\0';		/* lexequ?? */
					if ( (directory_user_dn = str2dn(TidyString(Dirname))) == NULLDN ) {
						aux_add_error(EPARSE, "Cannot fill DN structure", CNULL, 0, proc);
						return(NOTOK);
					}
					directory_user_name = (char *)malloc(strlen(Dirname) + 1);
					strcpy(directory_user_name, Dirname);
				}
			}
			if(directory_user_dn && af_dir_authlevel == DBA_AUTH_STRONG){
				fclose(fp_quipurc);
				return(OK);
			}


			/* read user's directory password */

			if(! Password[0]){
				if ( strstr(stream_contents, "password") != CNULL ) {
					p = strchr(strstr(stream_contents, "password"), ':');
					p += 2;
					ind = 0;
					while ( (Password[ind++] = *p++) != '\n' ) {
					}
					ind--;
					Password[ind] = '\0';
					strcpy(bindarg.dba_passwd, Password);
					break;
				}
			}
		} /*while*/

		fclose(fp_quipurc);
		if(Password[0]) return(OK);
	} 


	/* read from stdin */

	if(! directory_user_dn){
		if(ask_for_username() == NOTOK){
			aux_add_error(EINVALID, "ask_for_username failed", CNULL, 0, proc);
			return(NOTOK);
		}
		if(af_dir_authlevel == DBA_AUTH_STRONG) return(OK);
	}

	if(! Password[0]){
		/* Read user's X.500 password from user's PSE */
		dd = af_pse_get_QuipuPWD();
		if (! dd) {
			store_password_on_PSE = TRUE;
			strcpy(prompt, "\nEnter your directory password: ");
			dd = getpass(&prompt[1]);
			if(!dd || strlen(dd) == 0){
				aux_add_error(EINVALID, "getpass failed", CNULL, 0, proc);
				return(NOTOK);
			}
		}
		strcpy(bindarg.dba_passwd, dd);
		strcpy(Password, dd);
		free(dd);
		dd = CNULL;
	}

	return(OK);
}


/***************************************************************
 *
 * Procedure ask_for_username
 *
 ***************************************************************/
#ifdef __STDC__

static int ask_for_username(
)

#else

static int ask_for_username(
)

#endif

{
	char	        buf[BUFLEN];
	Name          * alias;
	PE		pe;
	char	      * proc = "ask_for_username";

	fprintf(stderr, "\nEnter your Directory name in printable representation\n");
	fprintf(stderr, "  (e.g. C=DE, O=GMD, OU=CA): ");
	while ( !gets(buf) || !buf[0] ) {
		fprintf(stderr, "Directory Name? ");
	}
	alias = malloc(strlen(buf) + 1);
	if (!alias) {
		aux_add_error(EMALLOC, "alias", CNULL, 0, proc);
		return(NOTOK);
	}
	strcpy(alias, buf);
	directory_user_dname = aux_alias2DName(alias);
	free(alias);

	directory_user_name = aux_DName2Name(directory_user_dname);
	if(af_verbose) fprintf(stderr, "\nBinding as \"%s\"\n", directory_user_name);

	directory_user_dn = secudeDName2quipuDN(directory_user_dname);
	if(directory_user_dn == NULLDN){
		aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
		return(- 1);
	}

	return(OK);
}


/***************************************************************
 *
 * Procedure set_bindarg
 *
 ***************************************************************/
#ifdef __STDC__

static RC set_bindarg(
)

#else

static RC set_bindarg(
)

#endif

{
	DName	    * dsa_dname;
	PE	      pe;
	char	    * proc = "set_bindarg";


	if (af_dir_authlevel == DBA_AUTH_SIMPLE){
		if(! directory_user_dn || ! Password[0]){
			/* read distinguished name and password */
			if(get_credentials() == NOTOK){
				AUX_ADD_ERROR;
				return(- 1);
			}
		}

		/* "bindarg.dba_dn" is the directory user's distinguished name; */
		/* therefore, copy it from "directory_user_dn"			*/

		bindarg.dba_dn = dn_cpy(directory_user_dn);
		bindarg.dba_auth_type = DBA_AUTH_SIMPLE;
		bindarg.dba_version = DBA_VERSION_V1988;
		bindarg.dba_passwd_len = strlen(bindarg.dba_passwd);
		return(0);
	}

#ifdef STRONG
	if(af_dir_authlevel == DBA_AUTH_STRONG){
		if(! directory_user_dn){
			if(get_credentials() == NOTOK){
				AUX_ADD_ERROR;  /* reads distinguished name */
				return(- 1);
			}
		}

		/* "bindarg.dba_dn" is the distinguished name of the first intended recipient, i.e. the DSA's name */

		dsa_dname = aux_alias2DName(myname);
		if(! dsa_dname){
			aux_add_error(EINVALID, "DSA (alias-)name cannot be transformed into DName-structure (Check PSE-object AliasList!)", CNULL, 0, proc);
			return(- 1);
		}

		bindarg.dba_dn = secudeDName2quipuDN(dsa_dname);
		if(bindarg.dba_dn == NULLDN){
			aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
			return(- 1);
		}

		first_intended_recipient = dn_cpy(bindarg.dba_dn);  /* Security Parameter 'sp_name' */

		if(sign_bindarg() == NOTOK){
			aux_add_error(EINVALID, "Creation of STRONG credentials failed", CNULL, 0, proc);
			return(- 1);
		}
		return(0);
	}
#endif

	if (af_dir_authlevel == DBA_AUTH_NONE){
		bindarg.dba_dn = NULLDN;
		bindarg.dba_auth_type = DBA_AUTH_NONE;
		bindarg.dba_version = DBA_VERSION_V1988;
		bindarg.dba_passwd[0]  = 0;
		bindarg.dba_passwd_len = 0;
		return(0);
	}

	return(0);
}	


#ifdef STRONG
/***************************************************************
 *
 * Procedure set_SecurityParameter
 *
 ***************************************************************/
#ifdef __STDC__

static RC set_SecurityParameter(
)

#else

static RC set_SecurityParameter(
)

#endif

{
	char  * proc = "set_SecurityParameter";

	if(! ca_security){
		ca_security = (struct security_parms *)calloc(1, sizeof(struct security_parms));
		if(! ca_security){
			aux_add_error(EMALLOC, "ca_security", CNULL, 0, proc);
			return(NOTOK);
		}
		/* The following are constant values which need be assigned once only */
		ca_security->sp_name = dn_cpy(first_intended_recipient);
		ca_security->sp_target = 1;  /* Result (if provided) shall be protected by a digital signature */
		ca_security->sp_random = (struct random_number *)0;
		ca_security->sp_time = CNULL;  
		if (dsap_security->serv_mkpath) 
			ca_security->sp_path = (dsap_security->serv_mkpath)();
		else ca_security->sp_path = (struct certificate_list *)0;
	}
	if(ca_security->sp_time) free(ca_security->sp_time);
	ca_security->sp_time = get_date_of_expiry();

	if(ca_security->sp_random) aux_free_random(&ca_security->sp_random);
	ca_security->sp_random = get_quipu_random();

	return(OK);
}
#endif


#ifdef STRONG
/***************************************************************
 *
 * Procedure verify_bindres
 *
 ***************************************************************/
#ifdef __STDC__

static RC verify_bindres(
)

#else

static RC verify_bindres(
)

#endif

{

/* As the data type of the bind argument does not provide a "ProtectionRequest" field,       */
/* PASSWORD DSAs will return a signed bind result to a requestor only if the bind argument   */ 
/* submitted by the requestor was signed, too. (see X.500 Interoperability Profile Document  */

	RC 	rcode;
	PS      rps = NULLPS;
	char  * proc = "verify_bindres";

/*
	rps = ps_alloc(std_open);
	std_setup(rps, stdout);
*/

	if(bindresult.dba_dn){  /*result is provided and must be evaluated*/

	        if (dsap_security && dsap_security->serv_ckpath && dsap_security->serv_cknonce){
			if (af_verbose) {
				fprintf(stderr, "\n\n\n\n\n\n\n\n\n\n\n\n *******************************************************************");
				fprintf(stderr, "\n ****************   B I N D   R  E  S  U  L  T   *******************");
				fprintf(stderr, "\n *******************************************************************\n\n\n");
			}
			rcode = (dsap_security->serv_ckpath) 
				((caddr_t) &bindresult, bindresult.dba_cpath, bindresult.dba_sig, &real_name, _ZTokenToSignDAS);
			if (rcode != OK){
				binderr.dbe_version = DBA_VERSION_V1988;
				binderr.dbe_type = DBE_TYPE_SECURITY;
				if ( err_stack->e_number == ESIGNATURE )
					binderr.dbe_value = DSE_SC_INVALIDSIGNATURE;
				else if ( err_stack->e_number == EVERIFICATION )
					binderr.dbe_value = DSE_SC_AUTHENTICATION;
				else  binderr.dbe_value = DSE_SC_NOINFORMATION;
				if(af_verbose) ds_bind_error(rps, &binderr);
				return (- 1);
			}
		}
		if(dn_cmp(directory_user_dn, bindresult.dba_dn) != OK){
			if(af_verbose) {
				fprintf(stderr, "User != Authenticated User, ie %s != %s\n", dn2str(bindresult.dba_dn),directory_user_name);
				binderr.dbe_version = DBA_VERSION_V1988;
				binderr.dbe_type = DBE_TYPE_SECURITY;
				binderr.dbe_value = DSE_SC_AUTHENTICATION;
				ds_bind_error(rps, &binderr);
			}
			return (- 1);
	    	}
	}

	return(0);
}
#endif


#ifdef STRONG
/***************************************************************
 *
 * Procedure load_security_functions
 *
 ***************************************************************/
#ifdef __STDC__

static void load_security_functions(
)

#else

static void load_security_functions(
)

#endif

{
	char    * proc = "load_security_functions";

	dsap_security = use_serv_secude();
}
#endif


#ifdef STRONG
/***************************************************************
 *
 * Procedure sign_bindarg
 *
 ***************************************************************/
#ifdef __STDC__

static int sign_bindarg(
)

#else

static int sign_bindarg(
)

#endif

{
	struct Nonce * nonce;
	char	     * proc = "sign_bindarg";

	if (dsap_security == (struct SecurityServices *) 0){
		load_security_functions();
		if (dsap_security == (struct SecurityServices *) 0)
			return (NOTOK);
	}
 
	if (! dsap_security->serv_mknonce)
		return (NOTOK);

	nonce = (dsap_security->serv_mknonce)((struct Nonce *) 0);
	if (nonce == (struct Nonce *) 0)
		return (NOTOK);

	bindarg.dba_auth_type = DBA_AUTH_STRONG;
	bindarg.dba_version = DBA_VERSION_V1988;
	bindarg.dba_time1 = nonce->non_time1;
	bindarg.dba_time2 = nonce->non_time2;
	bindarg.dba_r1.n_bits = nonce->non_r1.n_bits;
	bindarg.dba_r1.value = nonce->non_r1.value;
	bindarg.dba_r2.n_bits = nonce->non_r2.n_bits;
	bindarg.dba_alg.algorithm = nonce->non_alg.algorithm;
	bindarg.dba_alg.p_type = nonce->non_alg.p_type;
	bindarg.dba_alg.asn = nonce->non_alg.asn;
	free((char *) nonce);
	if (dsap_security->serv_sign)
		bindarg.dba_sig = (dsap_security->serv_sign)((char*)&bindarg, _ZTokenToSignDAS, NULLALGID);
	else
		return (NOTOK);

	if (bindarg.dba_sig == (struct signature *) 0)
		return (NOTOK);

	if (dsap_security->serv_mkpath)
 		bindarg.dba_cpath = (dsap_security->serv_mkpath)();
	else
 		bindarg.dba_cpath = (struct certificate_list *)0;

	return (OK);
}
#endif


/***************************************************************
 *
 * Procedure security_syntaxes
 *
 ***************************************************************/
#ifdef __STDC__

int security_syntaxes(
)

#else

int security_syntaxes(
)

#endif

{
	oclist_syntax();
	crl_syntax();
}


/***************************************************************
 *
 * Procedure quipuCert2secudeCert
 *
 ***************************************************************/
#ifdef __STDC__

Certificate * quipuCert2secudeCert(
	struct certificate      * quipu_cert
)

#else

Certificate * quipuCert2secudeCert(
	quipu_cert
)
struct certificate	 * quipu_cert;

#endif

{
	PE             pe;
	Certificate  * secude_cert;

	char	     * proc = "quipuCert2secudeCert";

	if(! quipu_cert){
		aux_add_error(EINVALID, "parameter missing", CNULL, 0, proc);
		return((Certificate *) 0);
	}

	if ( (pe = cert_enc(quipu_cert)) == NULLPE ) {
		aux_add_error(EENCODE, "cert_enc failed", CNULL, 0, proc);
		return((Certificate *) 0);
	}

	if ( (secude_cert = certificate_dec(pe)) == (Certificate * )0 ) {
		pe_free(pe);
		aux_add_error(EDECODE, "certificate_dec failed", CNULL, 0, proc);
		return((Certificate *) 0);
	}

	pe_free(pe);

	return(secude_cert);
}



/***************************************************************
 *
 * Procedure secudeCert2quipuCert
 *
 ***************************************************************/
#ifdef __STDC__

struct certificate * secudeCert2quipuCert(
	Certificate      * secude_cert
)

#else

struct certificate * secudeCert2quipuCert(
	secude_cert
)
Certificate	 * secude_cert;

#endif

{
	PE                    pe;
	struct certificate  * quipu_cert;

	char	            * proc = "secudeCert2quipuCert";


	if(! secude_cert){
		aux_add_error(EINVALID, "parameter missing", CNULL, 0, proc);
		return((struct certificate *) 0);
	}

	if ( (pe = certificate_enc(secude_cert)) == NULLPE ) {
		aux_add_error(EENCODE, "certificate_enc failed", CNULL, 0, proc);
		return((struct certificate *) 0);
	}

	if ( (quipu_cert = cert_dec(pe)) == (struct certificate *)0 ) {
		pe_free(pe);
		aux_add_error(EDECODE, "cert_dec failed", CNULL, 0, proc);
		return((struct certificate *) 0);
	}

	pe_free(pe);

	return(quipu_cert);
}


/***************************************************************
 *
 * Procedure secudeCPair2quipuCPair
 *
 ***************************************************************/
#ifdef __STDC__

struct certificate_list * secudeCPair2quipuCPair(
	CertificatePair      * secude_cpair
)

#else

struct certificate_list * secudeCPair2quipuCPair(
	secude_cpair
)
CertificatePair	 * secude_cpair;

#endif

{
	struct certificate_list  * quipu_cpair;

	char	                 * proc = "secudeCPair2quipuCPair";

	/* Transform SecuDE-type "CertificatePair" into Quipu-type "certificate_list" */

	if(! secude_cpair){
		aux_add_error(EINVALID, "parameter missing", CNULL, 0, proc);
		return((struct certificate_list *) 0);
	}

	if ( !(quipu_cpair = (struct certificate_list *)malloc(sizeof(struct certificate_list ))) ) {
		aux_add_error(EMALLOC, "quipu_cpair", CNULL, 0, proc);
		return((struct certificate_list *) 0);
	}

	if (secude_cpair->forward) {
		quipu_cpair->cert = secudeCert2quipuCert(secude_cpair->forward);
		if(! quipu_cpair->cert){
			aux_add_error(EINVALID, "Cannot transform SecuDE-certificate representation into Quipu representation", CNULL, 0, proc);
			return((struct certificate_list *) 0);
		}
	} 
	else 
		/* secude_cpair->forward may be the NULL pointer */
		quipu_cpair->cert = (struct certificate *)0;

	if (secude_cpair->reverse) {
		quipu_cpair->reverse = secudeCert2quipuCert(secude_cpair->reverse);
		if(! quipu_cpair->reverse){
			aux_add_error(EINVALID, "Cannot transform SecuDE-certificate representation into Quipu representation", CNULL, 0, proc);
			return((struct certificate_list *) 0);
		}
	} 
	else 
		/* secude_cpair->reverse may be the NULL pointer */
		quipu_cpair->reverse = (struct certificate *)0;

	return(quipu_cpair);
}


/***************************************************************
 *
 * Procedure af_dir_enter_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_enter_Certificate(
	Certificate	 *cert,
	CertificateType	  type
)

#else

RC af_dir_enter_Certificate(
	cert,
	type
)
Certificate	 *cert;
CertificateType	  type;

#endif

{
	RC	      rcode;
	char	    * proc = "af_dir_enter_Certificate";

	if ( !cert || ((type != userCertificate) && (type != cACertificate)) ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	if ( type == userCertificate )
		rcode = af_access_Directory(DAP_ENTER, cert->tbs->subject, USERCERTIFICATE, NULLOCTETSTRING, NULLDNAME, (caddr_t) cert);
	else
		rcode = af_access_Directory(DAP_ENTER, cert->tbs->subject, CACERTIFICATE, NULLOCTETSTRING, NULLDNAME, (caddr_t) cert);

	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}



/***************************************************************
 *
 * Procedure af_dir_retrieve_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_Certificate *af_dir_retrieve_Certificate(
	DName		 *dname,
	CertificateType	  type
)

#else

SET_OF_Certificate *af_dir_retrieve_Certificate(
	dname,
	type
)
DName		 *dname;
CertificateType	  type;

#endif

{
	RC		     rcode;
	SET_OF_Certificate * ret;   /* return value */

	char		   * proc = "af_dir_retrieve_Certificate";

	if ( !dname || ((type != userCertificate) && (type != cACertificate)) ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return((SET_OF_Certificate * )0);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return((SET_OF_Certificate * )0);
	}

	if ( type == userCertificate )
		rcode = af_access_Directory(DAP_RETRIEVE, dname, USERCERTIFICATE, NULLOCTETSTRING, NULLDNAME, CNULL);
	else
		rcode = af_access_Directory(DAP_RETRIEVE, dname, CACERTIFICATE, NULLOCTETSTRING, NULLDNAME, CNULL);

	if(rcode == NOTOK){
		AUX_ADD_ERROR;
		return((SET_OF_Certificate * )0);
	}

	ret = aux_cpy_SET_OF_Certificate((SET_OF_Certificate *)result);
	free(result);

	return(ret);
}



/***************************************************************
 *
 * Procedure af_dir_delete_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_delete_Certificate(
	OctetString	 *serial,
	DName		 *issuer,
	CertificateType	  type
)

#else

RC af_dir_delete_Certificate(
	serial,
	issuer,
	type
)
OctetString	 *serial;
DName		 *issuer;
CertificateType	  type;

#endif

{
	RC	  rcode;
	char	* proc = "af_dir_delete_Certificate";


	if (! serial || ((type != userCertificate) && (type != cACertificate)) ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	if ( type == userCertificate )
		rcode = af_access_Directory(DAP_DELETE, NULLDNAME, USERCERTIFICATE, serial, issuer, CNULL);
	else
		rcode = af_access_Directory(DAP_DELETE, NULLDNAME, CACERTIFICATE, serial, issuer, CNULL);

	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}



/***************************************************************
 *
 * Procedure af_dir_enter_CRL
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_enter_CRL(
	CRL	 *crl
)

#else

RC af_dir_enter_CRL(
	crl
)
CRL	 *crl;

#endif

{
	RC	  rcode;
	char	* proc = "af_dir_enter_CRL";

	if (! crl) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}


	bindarg.dba_dn = secudeDName2quipuDN(crl->tbs->issuer);
	if(bindarg.dba_dn == NULLDN){
		aux_add_error(EINVALID, "Cannot transform SecuDE-DName into Quipu-DN structure", CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	rcode = af_access_Directory(DAP_ENTER, NULLDNAME, REVLIST, NULLOCTETSTRING, NULLDNAME, (caddr_t) crl);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}



/***************************************************************
 *
 * Procedure af_dir_retrieve_CRL
 *
 ***************************************************************/
#ifdef __STDC__

CRL *af_dir_retrieve_CRL(
	DName	 *dname
)

#else

CRL *af_dir_retrieve_CRL(
	dname
)
DName	 *dname;

#endif

{
	RC        rcode;
	CRL  * ret;

	char	* proc = "af_dir_retrieve_CRL";

	if ( !dname ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return((CRL * )0);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return((CRL * )0);
	}

	rcode = af_access_Directory(DAP_RETRIEVE, dname, REVLIST, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode == NOTOK){
		AUX_ADD_ERROR;
		return((CRL * )0);
	}

	ret = aux_cpy_CRL((CRL *)result);
	free(result);

	return(ret);
}



/***************************************************************
 *
 * Procedure af_dir_enter_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_enter_CertificatePair(
	CertificatePair	 *cpair
)

#else

RC af_dir_enter_CertificatePair(
	cpair
)
CertificatePair	 *cpair;

#endif

{
	RC	  		  rcode;
	char			* proc = "af_dir_enter_CertificatePair";

	if ( !cpair) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	/* at least one (forward or reverse) must be present (X.509, 7.6) */
	if (! cpair->forward && ! cpair->reverse) {
		aux_add_error(EINVALID, "Invalid argument: At least one certificate (forward or reverse) must be present",
		     CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	rcode = af_access_Directory(DAP_ENTER, NULLDNAME, CERTIFICATEPAIR, NULLOCTETSTRING, NULLDNAME, (caddr_t) cpair);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}



/***************************************************************
 *
 * Procedure af_dir_retrieve_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_CertificatePair *af_dir_retrieve_CertificatePair(
	DName	 *dname
)

#else

SET_OF_CertificatePair *af_dir_retrieve_CertificatePair(
	dname
)
DName	 *dname;

#endif

{
	RC	   		 rcode;
	SET_OF_CertificatePair * ret;   /* return value */

	char		       * proc = "af_dir_retrieve_CertificatePair";

	if ( !dname ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return((SET_OF_CertificatePair * )0);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return((SET_OF_CertificatePair * )0);
	}

	rcode = af_access_Directory(DAP_RETRIEVE, dname, CERTIFICATEPAIR, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode == NOTOK){
		AUX_ADD_ERROR;
		return((SET_OF_CertificatePair * )0);
	}

	ret = aux_cpy_SET_OF_CertificatePair((SET_OF_CertificatePair *)result);
	free(result);

	return(ret);
}



/***************************************************************
 *
 * Procedure af_dir_delete_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_delete_CertificatePair(
	CertificatePair	 *cpair
)

#else

RC af_dir_delete_CertificatePair(
	cpair
)
CertificatePair	 *cpair;

#endif

{
	RC	  rcode;
	char	* proc = "af_dir_delete_CertificatePair";

	if (!cpair) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	/* at least one (forward or reverse) must be present (X.509, 7.6) */
	if (!cpair->forward && !cpair->reverse) {
		aux_add_error(EINVALID, "Invalid argument: At least one certificate (forward or reverse) must be present",
		     CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	rcode = af_access_Directory(DAP_DELETE, NULLDNAME, CERTIFICATEPAIR, NULLOCTETSTRING, NULLDNAME, (caddr_t) cpair);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}



/***************************************************************
 *
 * Procedure af_dir_enter_OCList
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_enter_OCList(
	OCList	 *oclist
)

#else

RC af_dir_enter_OCList(
	oclist
)
OCList	 *oclist;

#endif

{
	RC	  rcode;
	char	* proc = "af_dir_enter_OCList";

	if ( !oclist) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	rcode = af_access_Directory(DAP_ENTER, NULLDNAME, OCLIST, NULLOCTETSTRING, NULLDNAME, (caddr_t) oclist);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}



/***************************************************************
 *
 * Procedure af_dir_retrieve_OCList
 *
 ***************************************************************/
#ifdef __STDC__

OCList *af_dir_retrieve_OCList(
	DName	 *dname
)

#else

OCList *af_dir_retrieve_OCList(
	dname
)
DName	 *dname;

#endif

{
	RC 	  rcode;
	OCList  * ret;

	char	* proc = "af_dir_retrieve_OCList";

	if ( !dname ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return((OCList * )0);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return((OCList * )0);
	}

	rcode = af_access_Directory(DAP_RETRIEVE, dname, OCLIST, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode == NOTOK){
		AUX_ADD_ERROR;
		return((OCList * )0);
	}

	ret = aux_cpy_OCList((OCList *)result);
	free(result);

	return(ret);
}



/***************************************************************
 *
 * Procedure af_dir_delete_Certificate_from_targetObject
 *
 ***************************************************************/
#ifdef __STDC__

RC af_dir_delete_Certificate_from_targetObject(
	DName		 *target,
	DName		 *issuer,
	CertificateType	  type
)

#else

RC af_dir_delete_Certificate_from_targetObject(
	target,
	issuer,
	type
)
DName		 *target;
DName		 *issuer;
CertificateType	  type;

#endif

{
	RC      rcode;
	char  * proc = "af_dir_delete_Certificate_from_targetObject";

	if ( ! target || ((type != userCertificate) && (type != cACertificate)) ) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return(- 1);
	}

	/* Bind to X.500 Directory */
	rcode = af_access_Directory(DAP_BIND, NULLDNAME, NOTYPE, NULLOCTETSTRING, NULLDNAME, CNULL);
	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	if ( type == userCertificate )
		rcode = af_access_Directory(DAP_DELETE, target, USERCERTIFICATE, NULLOCTETSTRING, issuer, CNULL);
	else
		rcode = af_access_Directory(DAP_DELETE, target, CACERTIFICATE, NULLOCTETSTRING, issuer, CNULL);

	if(rcode < 0){
		AUX_ADD_ERROR;
		return(- 1);
	}

	return(0);
}

#endif



#ifdef AFDBFILE

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "af.h"

/* af_afdb_retrieve_Certificate(dname, ktype) : get 'dname's Certificate of type 'ktype' */

/***************************************************************
 *
 * Procedure af_afdb_retrieve_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_Certificate *af_afdb_retrieve_Certificate(
	DName	 *dname,
	KeyType	  ktype
)

#else

SET_OF_Certificate *af_afdb_retrieve_Certificate(
	dname,
	ktype
)
DName	 *dname;
KeyType	  ktype;

#endif

{
	SET_OF_Certificate  * ret;
	char	            * csetfile;
	OctetString         * loaded;

	char	            * proc = "af_afdb_retrieve_Certificate";

	if (!dname || (ktype != SIGNATURE && ktype != ENCRYPTION)){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);       
		return ( (SET_OF_Certificate *) 0);
	}

	csetfile = aux_DName2afdbname(dname);
	if (!csetfile){
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);	
		return ( (SET_OF_Certificate *) 0);
	}


	strcat(csetfile, CSet_name);

	loaded = aux_file2OctetString(csetfile);
	if (loaded) ret = d_CertificateSet(loaded);
	else {
		csetfile[strlen(csetfile) - 4] = '\0';
		if(ktype == SIGNATURE) strcat(csetfile, SignCSet_name);
		else strcat(csetfile, EncCSet_name);
		loaded = aux_file2OctetString(csetfile);
		if (loaded) ret = d_CertificateSet(loaded);
		else {
			free(csetfile);
			return ( (SET_OF_Certificate *) 0);
		}
	}
	aux_free_OctetString(&loaded);
	free(csetfile);

	if (! ret)  {
		aux_add_error(EDECODE, "d_CertificateSet failed", CNULL, 0, proc);
		return ( (SET_OF_Certificate *) 0);
	}

	return ret;
}


/***************************************************************
 *
 * Procedure af_afdb_enter_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC af_afdb_enter_Certificate(
	Certificate	 *cert,
	KeyType		  ktype
)

#else

RC af_afdb_enter_Certificate(
	cert,
	ktype
)
Certificate	 *cert;
KeyType		  ktype;

#endif

{
	char	      		* csetfile;
	RC	      		  rcode;
	Boolean       		  onekeypaironly = FALSE;
	SET_OF_Certificate  	* cset, * tmp_cset;
	OctetString             * encoded, * loaded;
	char	    		* proc = "af_afdb_enter_Certificate";

	if (! cert || (ktype != SIGNATURE && ktype != ENCRYPTION)) {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);       
		return(- 1);
	}

	csetfile = aux_DName2afdbname(cert->tbs->subject);
	if (!csetfile){
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);	
		return(- 1);
	}

	csetfile[strlen(csetfile) - 1] = '\0';
	if ((mkdir(csetfile, 0755) < 0) && (errno != EEXIST)) {
		aux_add_error(ESYSTEM, ".afdb Directory entry does not exist", CNULL, 0, proc);
		free(csetfile);
		return(- 1);
	}
	csetfile[strlen(csetfile)] = PATH_SEPARATION_CHAR;


	if(af_check_if_onekeypaironly(&onekeypaironly)){
		aux_add_error(LASTERROR, "af_check_if_onekeypaironly failed", CNULL, 0, proc);
		free(csetfile);
		return (- 1);
	}
	if(onekeypaironly == TRUE)
		strcat(csetfile, CSet_name);
	else{
		if (ktype == ENCRYPTION) 
			strcat(csetfile, EncCSet_name);
		else
			strcat(csetfile, SignCSet_name);
	}

	loaded = aux_file2OctetString(csetfile);
	if (!loaded) {
		if ( !(cset = (SET_OF_Certificate * )
				   malloc(sizeof(SET_OF_Certificate))) ) {
			aux_add_error(EMALLOC, "cset", CNULL, 0, proc);
			return(- 1);
		}
		cset->element = aux_cpy_Certificate(cert);
		cset->next = (SET_OF_Certificate *) 0;
	}
	else {
		/* got cset */
		cset = d_CertificateSet(loaded);
		aux_free_OctetString(&loaded);
		if (!cset)  {
			aux_add_error(EDECODE, "d_CertificateSet failed", CNULL, 0, proc);
			free(csetfile);
			return(- 1);
		}

		/* Check whether 'cert' already exists in .af-db entry */
		for (tmp_cset = cset; tmp_cset ; tmp_cset = tmp_cset->next ) {
			if (! aux_cmp_Certificate(cert, tmp_cset->element) ) {    /*equal*/
				if(onekeypaironly == TRUE)
					aux_add_error(ECREATEOBJ, "Specified certificate already exists in Your directory entry", CNULL, 0, proc);
				else{
					if (ktype == ENCRYPTION)
						aux_add_error(ECREATEOBJ, "Specified certificate already exists in the set of ENCRYPTION certificates in your Directory entry", CNULL, 0, proc);
					else
						aux_add_error(ECREATEOBJ, "Specified certificate already exists in the set of SIGNATURE certificates in your Directory entry", CNULL, 0, proc);
				}
				free(csetfile);
				aux_free_CertificateSet(&cset);
				return(- 1);
			}
		}

		if ( !(tmp_cset = (SET_OF_Certificate * )
					   malloc(sizeof(SET_OF_Certificate))) ) {
			aux_add_error(EMALLOC, "tmp_cset", CNULL, 0, proc);
			return(- 1);
		}
		tmp_cset->element = aux_cpy_Certificate(cert);
		tmp_cset->next = cset;
		cset = tmp_cset;
	}	

	encoded = e_CertificateSet(cset);
	if (!encoded)  {
		aux_add_error(EENCODE, "e_CertificateSet failed", (char *) cset, SET_OF_Certificate_n, proc);
		free(csetfile);
		aux_free_CertificateSet(&cset);
		return(- 1);
	}

	if ((rcode = aux_OctetString2file(encoded, csetfile, 2)) < 0) {
		aux_add_error(ESYSTEM, "can't write cset into .af-db", csetfile, char_n, proc);
	}
	aux_free_OctetString(&encoded);
	aux_free_CertificateSet(&cset);
	free(csetfile);

	return rcode;
}


/***************************************************************
 *
 * Procedure af_afdb_delete_Certificate
 *
 ***************************************************************/
#ifdef __STDC__

RC af_afdb_delete_Certificate(
	OctetString *serial,
	DName	    *issuer,
	KeyType	     ktype
)

#else

RC af_afdb_delete_Certificate(
	serial,
	issuer,
	ktype
)
OctetString *serial;
DName	    *issuer;
KeyType	     ktype;

#endif

{
	char	    	      * csetfile;
	SET_OF_Certificate    * cset, * np, * ahead_np;
	SET_OF_Certificate    * found_np = (SET_OF_Certificate * ) 0, * found_ahead_np = (SET_OF_Certificate * ) 0;
	OctetString           * encoded, * loaded;
	Boolean	                found;
	Boolean       		onekeypaironly = FALSE;
	RC			rcode;

	char	    	      * proc = "af_afdb_delete_Certificate";

	if (! serial || (ktype != SIGNATURE && ktype != ENCRYPTION) ){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);      
		return(- 1);
	}

	csetfile = aux_DName2afdbname(directory_user_dname);
	if (!csetfile){
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);	
		return(- 1);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		aux_add_error(LASTERROR, "af_check_if_onekeypaironly failed", CNULL, 0, proc);
		free(csetfile);
		return (- 1);
	}
	if(onekeypaironly == TRUE)
		strcat(csetfile, CSet_name);
	else{
		if (ktype == ENCRYPTION) 
			strcat(csetfile, EncCSet_name);
		else
			strcat(csetfile, SignCSet_name);
	}

	loaded = aux_file2OctetString(csetfile);
	if (!loaded) {
		free(csetfile);
		return(- 1);
	}

	/* got cset */
	cset = d_CertificateSet(loaded);
	aux_free_OctetString(&loaded);
	if (! cset)  {
		aux_add_error(EDECODE, "d_CertificateSet failed", CNULL, 0, proc);
		free(csetfile);
		return(- 1);
	}

	found = 0;
	for (np = cset, ahead_np = (SET_OF_Certificate *) 0; np; ahead_np = np, np = np->next) {
		if (issuer) {
			if (! aux_cmp_OctetString(np->element->tbs->serialnumber, serial) && ! aux_cmp_DName(np->element->tbs->issuer, issuer))
				break;
		} 
		else {
			if (! aux_cmp_OctetString(np->element->tbs->serialnumber, serial)){
				if (! found) {
					found_np = np;
					found_ahead_np = ahead_np;
					found = 1;
				}
				else {
					aux_add_error(EOBJ, "More than one certificate with specified serial number", CNULL, 0, proc);
					free(csetfile);
					aux_free_CertificateSet(&cset);
					return(- 1);
				}
			}
		}
	}

	if(found_np) {
		np = found_np;
		ahead_np = found_ahead_np;
	}

	if (np) {      /* Certificate (to be deleted) found */
		if (! ahead_np) 
			cset = np->next;     /* firstelement */
		else 
			ahead_np->next = np->next;    /* not first */
		np->next = (SET_OF_Certificate *) 0;
		aux_free_CertificateSet(&np);

		if (! cset )      /* last element deleted from cset */
			unlink(csetfile);
		else {
			encoded = e_CertificateSet(cset);
			aux_free_CertificateSet(&cset);
			if (!encoded)  {
				aux_add_error(EENCODE, "e_CertificateSet failed", (char *) cset, SET_OF_Certificate_n, proc);
				free(csetfile);
				return(- 1);
			}

			if ((rcode = aux_OctetString2file(encoded, csetfile, 2)) < 0) {
				aux_add_error(ESYSTEM, "can't write cset into .af-db", csetfile, char_n, proc);
			}
			aux_free_OctetString(&encoded);
			free(csetfile);
		}
	} 
	else {      /* Certificate (to be deleted) not found */
		aux_free_CertificateSet(&cset);
		free(csetfile);
		aux_add_error(EOBJNAME, "Certificate (to be deleted) not found", CNULL, 0, proc);
		return(- 1);
	}

	return (rcode);

}


/***************************************************************
 *
 * Procedure af_afdb_enter_CRL
 *
 ***************************************************************/
#ifdef __STDC__

RC af_afdb_enter_CRL(
	CRL	 *crl
)

#else

RC af_afdb_enter_CRL(
	crl
)
CRL	 *crl;

#endif

{
	char	    * crlfile;
	OctetString * encoded;
	RC	      rcode;
	char	    * proc = "af_afdb_enter_CRL";

	if (!crl){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);       
		return(- 1);
	}

	encoded = e_CRL(crl);
	if (!encoded)  {
		aux_add_error(EENCODE, "e_CRL failed", (char *) crl, CRL_n, proc);
		return(- 1);
	}

	crlfile = aux_DName2afdbname(crl->tbs->issuer);
	if (!crlfile){
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);	
		return(- 1);
	}

	crlfile[strlen(crlfile) - 1] = '\0';
	if ((mkdir(crlfile, 0755) < 0) && (errno != EEXIST)) {
		aux_add_error(ESYSTEM, ".afdb Directory entry does not exist", CNULL, 0, proc);
		free(crlfile);
		return(- 1);
	}
	crlfile[strlen(crlfile)] = PATH_SEPARATION_CHAR;

	strcat(crlfile, "CRL");

	if ((rcode = aux_OctetString2file(encoded, crlfile, 2)) < 0) {
		aux_add_error(ESYSTEM, "can't write crl into .af-db", crlfile, char_n, proc);
	}
	aux_free_OctetString(&encoded);
	free(crlfile);

	return rcode;
}


/***************************************************************
 *
 * Procedure af_afdb_retrieve_CRL
 *
 ***************************************************************/
#ifdef __STDC__

CRL *af_afdb_retrieve_CRL(
	DName	 *dname
)

#else

CRL *af_afdb_retrieve_CRL(
	dname
)
DName	 *dname;

#endif

{
	char	     * crlfile;
	OctetString  * loaded;
	CRL          * ret;
	char	     * proc = "af_afdb_retrieve_CRL";

	if (!dname){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);       
		return ( (CRL *) 0);
	}

	crlfile = aux_DName2afdbname(dname);
	if (!crlfile){
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);	
		return ( (CRL *) 0);
	}

	strcat(crlfile, "CRL");

	loaded = aux_file2OctetString(crlfile);
	if (!loaded) {
		free(crlfile);
		return ( (CRL *) 0);
	}

	/* got crl */
	ret = d_CRL(loaded);
	aux_free_OctetString(&loaded);
	if (!ret)  {
		aux_add_error(EDECODE, "d_CRL failed", CNULL, 0, proc);
	}
	free(crlfile);
	return ret;
}


/***************************************************************
 *
 * Procedure af_afdb_enter_OCList
 *
 ***************************************************************/
#ifdef __STDC__

RC af_afdb_enter_OCList(
)

#else

RC af_afdb_enter_OCList(
)

#endif

{
	return(0);
}


/***************************************************************
 *
 * Procedure af_afdb_retrieve_OCList
 *
 ***************************************************************/
#ifdef __STDC__

OCList *af_afdb_retrieve_OCList(
)

#else

OCList *af_afdb_retrieve_OCList(
)

#endif

{
	return((OCList *)0);
}


/***************************************************************
 *
 * Procedure af_afdb_enter_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

RC af_afdb_enter_CertificatePair(
	CertificatePair	 *cpair
)

#else

RC af_afdb_enter_CertificatePair(
	cpair
)
CertificatePair	 *cpair;

#endif

{
	char	                * cpairsetfile;
	SET_OF_CertificatePair  * cpairset, * cpairset_tmp;
	OctetString             * encoded, * loaded;
	RC	                  rcode;
	char	                * proc = "af_afdb_enter_CertificatePair";

	if (!cpair){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);      
		return(- 1);
	}

	cpairsetfile = aux_DName2afdbname(directory_user_dname);
	if (!cpairsetfile) {
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);
		return(- 1);
	}

	cpairsetfile[strlen(cpairsetfile) - 1] = '\0';
	if ((mkdir(cpairsetfile, 0755) < 0) && (errno != EEXIST)) {
		aux_add_error(ESYSTEM, ".afdb Directory entry does not exist", CNULL, 0, proc);
		free(cpairsetfile);
		return(- 1);
	}
	cpairsetfile[strlen(cpairsetfile)] = PATH_SEPARATION_CHAR;

	strcat(cpairsetfile, CrossCSet_name);

	loaded = aux_file2OctetString(cpairsetfile);
	if (!loaded) {
		if ( !(cpairset = (SET_OF_CertificatePair * )
			   malloc(sizeof(SET_OF_CertificatePair))) ) {
				aux_add_error(EMALLOC, "cpairset", CNULL, 0, proc);
				free(cpairsetfile);
				return(- 1);
		}
		cpairset->element = aux_cpy_CertificatePair(cpair);
		cpairset->next = (SET_OF_CertificatePair *) 0;
	}
	else {
		/* got cpairset */
		cpairset = d_CertificatePairSet(loaded);
		aux_free_OctetString(&loaded);
		if (!cpairset)  {
			aux_add_error(EDECODE, "d_CertificatePairSet failed", CNULL, 0, proc);
			free(cpairsetfile);
			return(- 1);
		}
		if ( !(cpairset_tmp = (SET_OF_CertificatePair * )
					   malloc(sizeof(SET_OF_CertificatePair))) ) {
			aux_add_error(EMALLOC, "cpairset_tmp", CNULL, 0, proc);
			free(cpairsetfile);
			return(- 1);
		}
		cpairset_tmp->element = aux_cpy_CertificatePair(cpair);
		cpairset_tmp->next = cpairset;
		cpairset = cpairset_tmp;
	}	

	encoded = e_CertificatePairSet(cpairset);
	aux_free_CertificatePairSet(&cpairset);
	if (!encoded)  {
		aux_add_error(EENCODE, "e_CertificatePairSet failed", (char *) cpairset, SET_OF_CertificatePair_n, proc);
		free(cpairsetfile);
		return(- 1);
	}

	if ((rcode = aux_OctetString2file(encoded, cpairsetfile, 2)) < 0) {
		aux_add_error(ESYSTEM, "can't write cpairset into .af-db", cpairsetfile, char_n, proc);
	}
	aux_free_OctetString(&encoded);
	free(cpairsetfile);

	return rcode;
}


/***************************************************************
 *
 * Procedure af_afdb_retrieve_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_CertificatePair *af_afdb_retrieve_CertificatePair(
	DName	 *dname
)

#else

SET_OF_CertificatePair *af_afdb_retrieve_CertificatePair(
	dname
)
DName	 *dname;

#endif

{
	char	               * cpairsetfile;
	OctetString            * loaded;
	SET_OF_CertificatePair * ret;
	char	               * proc = "af_afdb_retrieve_CertificatePair";

	if (!dname){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);       
		return ( (SET_OF_CertificatePair *) 0);
	}

	cpairsetfile = aux_DName2afdbname(dname);
	if (!cpairsetfile){
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);	
		return ( (SET_OF_CertificatePair *) 0);
	}

	strcat(cpairsetfile, CrossCSet_name);

	loaded = aux_file2OctetString(cpairsetfile);
	free(cpairsetfile);
	if (!loaded) {
		return ( (SET_OF_CertificatePair *) 0);
	}

	/* got cpairset */
	ret = d_CertificatePairSet(loaded);
	aux_free_OctetString(&loaded);
	if (!ret)  {
		aux_add_error(EDECODE, "d_CertificatePairSet failed", CNULL, 0, proc);
	}

	return ret;
}

/***************************************************************
 *
 * Procedure af_afdb_delete_CertificatePair
 *
 ***************************************************************/
#ifdef __STDC__

RC af_afdb_delete_CertificatePair(
	CertificatePair	 *cpair
)

#else

RC af_afdb_delete_CertificatePair(
	cpair
)
CertificatePair	 *cpair;

#endif

{
	char	                * cpairsetfile;
	SET_OF_CertificatePair  * cpairset, * np, * ahead_np;
	OctetString             * encoded, * loaded;
	RC	                  rcode;
	char	                * proc = "af_afdb_delete_CertificatePair";

	if (! cpair ){
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);       
		return(- 1);
	}

	cpairsetfile = aux_DName2afdbname(directory_user_dname);
	if (!cpairsetfile) {
		aux_add_error(EINVALID, "aux_DName2afdbname failed", CNULL, 0, proc);
		return(- 1);
	}

	strcat(cpairsetfile, CrossCSet_name);

	loaded = aux_file2OctetString(cpairsetfile);
	if (!loaded) {
		free(cpairsetfile);
		return(- 1);
	}

	/* got cpairset */
	cpairset = d_CertificatePairSet(loaded);
	aux_free_OctetString(&loaded);
	if (!cpairset)  {
		aux_add_error(EDECODE, "d_CertificatePairSet failed", CNULL, 0, proc);
		free(cpairsetfile);
		return(- 1);
	}

	for (np = cpairset, ahead_np = (SET_OF_CertificatePair *) 0; np; ahead_np = np, np = np->next) {
		if ( aux_cmp_CertificatePair(np->element, cpair) == 0 )
			break;
	}
	if (np) {      /* CertificatePair (to be deleted) found */
		if (!ahead_np) 
			cpairset = np->next;     /* firstelement */
		else 
			ahead_np->next = np->next;    /* not first */
		np->next = (SET_OF_CertificatePair *) 0;
		aux_free_CertificatePairSet(&np);

		if ( !cpairset )      /* last element deleted from cpairset */
			unlink(cpairsetfile);
		else {
			encoded = e_CertificatePairSet(cpairset);
			aux_free_CertificatePairSet(&cpairset);
			if (!encoded)  {
				aux_add_error(EENCODE, "e_CertificatePairSet failed", (char *) cpairset, SET_OF_CertificatePair_n, proc);
				free(cpairsetfile);
				return(- 1);
			}

			if ((rcode = aux_OctetString2file(encoded, cpairsetfile, 2)) < 0) {
				aux_add_error(ESYSTEM, "can't write cpairset into .af-db", cpairsetfile, char_n, proc);
			}
			aux_free_OctetString(&encoded);
		}
	} 
	else {      /* CertificatePair (to be deleted) not found */
		aux_free_CertificatePairSet(&cpairset);
		aux_add_error(EOBJNAME, "CertificatePair (to be deleted) not found", CNULL, 0, proc);
		free(cpairsetfile);
		return(- 1);
	}

	return (rcode);
}


#endif
