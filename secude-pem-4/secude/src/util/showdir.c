/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1992, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include "cadb.h"

static CERT();

int             verbose = 0;
static void     usage();

/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
#ifdef __STDC__

int main(
	int	  cnt,
	char	**parm
)

#else

int main(
	cnt,
	parm
)
int	  cnt;
char	**parm;

#endif

{
	Certificate 	        * cert;
	OctetString             * ostr;
	KeyType 	          keytype = SIGNATURE;
	SET_OF_Certificate      * certset;
	char	       	        * alias = CNULL;
	DName		        * owner;
	int		          i;
	char	                * psename = CNULL, * cadir = CNULL;
	char 		        * attrtype = CNULL, * name = CNULL;
	char	                * cmd = * parm, opt, *file = CNULL;
	AlgType		          algtype;
	SET_OF_CertificatePair  * cpairset;
	CRL	 		* crl;
	extern char	        * optarg;
	extern int	          optind, opterr;
	char			  x500 = TRUE;
#ifdef AFDBFILE
	char			  afdb[256];
#endif
#ifdef X500
	char	                * env_af_dir_authlevel;
	CertificateType           certtype = userCertificate; /* default */
#endif
	char                    * proc = "main (showdir)";


	MF_check = FALSE;

	optind = 1;
	opterr = 0;

nextopt:

#ifdef X500
	while ( (opt = getopt(cnt, parm, "c:p:o:d:t:A:a:ehvzVW")) != -1 ) {
#else
	while ( (opt = getopt(cnt, parm, "c:p:o:a:ehvzVW")) != -1 ) {
#endif
		switch (opt) {
#ifdef X500
		case 'd':
			af_dir_dsaname = aux_cpy_String(optarg);
			break;
		case 't':
			af_dir_tailor = aux_cpy_String(optarg);
			break;
		case 'A':
 			if (! strcasecmp(optarg, "STRONG"))
 				af_dir_authlevel = DBA_AUTH_STRONG;
 			else if (! strcasecmp(optarg, "SIMPLE"))
 				af_dir_authlevel = DBA_AUTH_SIMPLE;
 			break;
#endif
		case 'o':
			if (file) usage(SHORT_HELP);
			else file = optarg;
			break;
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
			break;
		case 'a':
			if (attrtype) usage(SHORT_HELP);
			else attrtype = optarg;
			break;
		case 'e':
			keytype = ENCRYPTION;
			break;
                case 'z':
                        MF_check = TRUE;
                        break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			verbose = 2;
			af_verbose = TRUE;
			break;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			break;
		case 'h':
			usage(LONG_HELP);
			break;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}

	if (optind < cnt) {
		alias = parm[optind++];
		goto nextopt;
	}

	if (optind < cnt) usage(SHORT_HELP);

	if ( ! attrtype )
		attrtype = "cert";
	else if (strcasecmp(attrtype, "cert") && strcasecmp(attrtype, "cacert") && strcasecmp(attrtype, "cross") && strcasecmp(attrtype, "crl")){
		fprintf(stderr, "Attribute must be 'cert' or 'cacert' or 'cross' or 'crl'\n");
		exit(-1);
	}

	aux_set_pse(psename, cadir);

	if(!alias) {
		owner = af_pse_get_Name();
		name = aux_DName2Name(owner);
		alias = aux_DName2alias(owner, LOCALNAME);
	}
	else {
		name = aux_alias2Name(alias);
		owner = aux_Name2DName(name);
		if(aux_alias(alias) == FALSE) alias = CNULL;
	} 

	if (!owner) {
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}


#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, X500_name);           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif


#ifdef X500
	if(x500) {
		if (af_dir_authlevel == DBA_AUTH_NONE) {
			env_af_dir_authlevel = getenv("AUTHLEVEL");
			if (env_af_dir_authlevel) {
				if (! strcasecmp(env_af_dir_authlevel, "STRONG"))
					af_dir_authlevel = DBA_AUTH_STRONG;
				else if (! strcasecmp(env_af_dir_authlevel, "SIMPLE"))
					af_dir_authlevel = DBA_AUTH_SIMPLE;
			}
		}

		directory_user_dname = af_pse_get_Name();

		if(verbose) {
			if (!alias)
				fprintf(stderr, "\nAccessing the X.500 Directory entry of <%s> ...\n", name);
			else
				fprintf(stderr, "\nAccessing the X.500 Directory entry of %s (%s) ...\n", alias, name);
			}
	}
#endif
#ifdef AFDBFILE
	if (!x500 || !af_x500){
		if(verbose) {
			if (!alias)
				if(verbose) fprintf(stderr, "\nAccessing the .afdb Directory entry of <%s> ...\n", name);
			else
				if(verbose) fprintf(stderr, "\nAccessing the .afdb Directory entry of %s (%s) ...\n", alias, name);
		}
	}
#endif

	if (!strcasecmp(attrtype, "cert") || !strcasecmp(attrtype, "cacert")) {
#ifdef X500
		if ( x500 ){
			if(!strcasecmp(attrtype, "cacert")) certtype = cACertificate;
			certset = af_dir_retrieve_Certificate(owner, certtype);
			if (!certset) { 
				aux_fprint_error(stderr, verbose);
				exit(-1); 
			}

			if(!file) for ( ; certset; certset = certset->next) {
				cert = certset->element;
				switch (certtype) {
					case userCertificate:
						CERT("user", cert);
						break;
					case cACertificate:
						CERT("cA", cert);
						break;
				}
			}
		}
#endif
#ifdef AFDBFILE
		if (!x500 || !af_x500){
			certset = af_afdb_retrieve_Certificate(owner, keytype);
			if (!certset) { 
				aux_fprint_error(stderr, verbose);
				if(keytype == SIGNATURE)
					fprintf(stderr, "No SIGNATURE certificate with owner <%s> in the .af-db Directory.\n", name);
				else fprintf(stderr, "No ENCRYPTION certificate with owner <%s> in the .af-db Directory.\n", name);
				exit(-1); 
			}
			if(!file) {
				if(keytype == SIGNATURE) CERT("Signature", certset->element);
				else CERT("Encryption", certset->element);
			}
		}
#endif

		if(file) {
			ostr = e_CertificateSet(certset);
			if(ostr) {
				aux_OctetString2file(ostr, file, 2);
				aux_free_OctetString(&ostr);
			}
			else fprintf(stderr, "Can't encode retrieved certificate\n");
		}
	}
	else if (!strcmp(attrtype,"cross")) {
#ifdef X500
		if ( x500 )
			cpairset = af_dir_retrieve_CertificatePair(owner);
#endif
#ifdef AFDBFILE
		if (!x500 || !af_x500)
			cpairset = af_afdb_retrieve_CertificatePair(owner);
#endif
		if (!cpairset) {
			aux_fprint_error(stderr, verbose);
			exit (-1);
                }
		if(!file) aux_fprint_CertificatePairSet(stderr, cpairset);
		else {
			ostr = e_CertificatePairSet(cpairset);
			if(ostr) {
				aux_OctetString2file(ostr, file, 2);
				aux_free_OctetString(&ostr);
			}
			else fprintf(stderr, "Can't encode retrieved certificate pair set\n");
		}
	}
	else {
#ifdef X500
		if  ( x500 )
			crl = af_dir_retrieve_CRL(owner);
#endif
#ifdef AFDBFILE
		if (!x500 || !af_x500)
			crl = af_afdb_retrieve_CRL(owner);
#endif
		if (!crl) {
			aux_fprint_error(stderr, verbose);
			exit (-1);
                }
		if(!file) aux_fprint_CRL(stderr, crl);
		else {
			ostr = e_CRL(crl);
			if(ostr) {
				aux_OctetString2file(ostr, file, 2);
				aux_free_OctetString(&ostr);
			}
			else fprintf(stderr, "Can't encode retrieved CRL\n");
		}
	}


	exit(0);
}


static CERT(s, c) 
Certificate*c; 
{ 
	fprintf(stderr, "\n---- %sCertificate ----\n\n", s); 
	aux_fprint_Certificate(stderr, c); 

	return(0);
}




static
void usage(help)
int     help;
{

	aux_fprint_version(stderr);

	fprintf(stderr, "showdir: Retrieve and Show Security Attributes from Directory\n\n\n");
	fprintf(stderr, "Description:\n\n"); 
	fprintf(stderr, "'showdir' reads a security attribute from the directory entry of 'Name' and\n");
	fprintf(stderr, "either prints its contents in an appropriate format to stderr,\n");
	fprintf(stderr, "or writes its ASN.1-encoded content to <file> (option -o).\n\n\n");

        fprintf(stderr, "usage:\n\n");
#ifdef X500
	fprintf(stderr, "showdir [-aehvzVW] [-p <pse>] [-c <cadir>] [-o <attributeType>] [-d <dsa name>] [-t <dsaptailor>] [-A <authlevel>] [Name].\n\n"); 
#else
	fprintf(stderr, "showdir [-ehvzVW] [-p <pse>] [-c <cadir>] [-o <attributeType>] [Name].\n\n");
#endif

        if(help == LONG_HELP) {

        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <psename>        PSE name (default: environment variable PSE or %s)\n", DEF_PSE);
        fprintf(stderr, "-c <cadir>          Name of CA-directory (default: environment variable CADIR or %s)\n", DEF_CADIR);
	fprintf(stderr, "-o <file>           write ASN.1-code of attribute value to <file>\n");
	fprintf(stderr, "-a <attributeType>  Attribute whose value is requested (default: certificate)\n");
	fprintf(stderr, "                    supported attribute types:\n"); 
	fprintf(stderr, "                    'cert' (userCertificate),\n");
	fprintf(stderr, "                    'cacert' (cACertificate),\n");
	fprintf(stderr, "                    'cross' (cross certificate pair), and\n");
	fprintf(stderr, "                    'crl' (certificate revocation list)\n");
	fprintf(stderr, "-e                  consider ENCRYPTION certificates only (.cadb Directory only)\n");
        fprintf(stderr, "-h                  write this help text\n");
	fprintf(stderr, "-z                  control malloc/free behaviour\n");
        fprintf(stderr, "-v                  verbose\n");
        fprintf(stderr, "-V                  Verbose\n");
        fprintf(stderr, "-W                  Grand Verbose (for testing only)\n");
#ifdef X500
	fprintf(stderr, "-d <dsa name>       Name of the DSA to be initially accessed (default: locally configured DSA)\n");
	fprintf(stderr, "-t <dsaptailor>     Location of dsaptailor file (default: dsaptailor in the ISODE ETCDIR directory)\n");
	fprintf(stderr, "-A <authlevel>      Level of authentication used for X.500 Directory access\n");
	fprintf(stderr, "                    <authlevel> may have one of the values 'SIMPLE' or 'STRONG'\n");
	fprintf(stderr, "                    (default: environment variable AUTHLEVEL or 'No authentication')\n");
	fprintf(stderr, "                    STRONG implies the use of signed DAP operations\n");
#endif
	fprintf(stderr, "<Name>              Name of Directory entry to be accessed (default: own entry)\n");         
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM AFPRINT */
}
