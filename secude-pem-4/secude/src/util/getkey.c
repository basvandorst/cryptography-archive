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

/*
 *
 *      getkey
 *
 */

#define SIGNKEY "signature"
#define ENCKEY  "encryption"

#include <fcntl.h>
#include <stdio.h>
#include "cadb.h"

extern  UTCTime *aux_current_UTCTime(), *aux_delta_UTCTime();

char *getenv();

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
        int             rc, i;
        ObjId           *oid;
        PSESel          *pse;
        KeyType         type = SIGNATURE;
	AlgId           *sig_alg = (AlgId * )0;
	Certificate     *cert, *protocert;
        char            *filename = CNULL;
	OctetString     *encoded_cert;
	OctetString	*serial;
	extern char	*optarg;
	extern int	optind, opterr;
	char	        *cmd = *parm, opt, *pin;
	char	        *psename = CNULL, *cadir = CNULL;
	Boolean 	onekeypaironly = FALSE;

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "a:c:p:ehsvzVW")) != -1 ) {
		switch (opt) {
		case 'a':
                        oid = aux_Name2ObjId(optarg);
                        if (aux_ObjId2AlgType(oid) != SIG) usage(SHORT_HELP);
			sig_alg = aux_ObjId2AlgId(oid);
			break;
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
			break;
		case 'e':
			type = ENCRYPTION;
			break;
		case 's':
			type = SIGNATURE;
			break;
                case 'z':
                        MF_check = TRUE;
                        break;
		case 'v':
			verbose = 1;
			continue;
		case 'V':
			verbose = 2;
			continue;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			continue;
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}

	if (optind < cnt) filename = parm[optind++];

	aux_set_pse(psename, cadir);

	if(!(cert = af_pse_get_Certificate(type, NULLDNAME, 0))) {
                fprintf(stderr, "%s: ", cmd);
		if(type == SIGNATURE) fprintf(stderr, "can't read SignCert");
		else fprintf(stderr, "can't read EncCert");
		exit(-1);
	}

	if(!sig_alg){
		if(aux_ObjId2AlgEnc(cert->tbs->subjectPK->subjectAI->objid) == RSA) 
			sig_alg = aux_cpy_AlgId(md2WithRsaEncryption);
		if(aux_ObjId2AlgEnc(cert->tbs->subjectPK->subjectAI->objid) == DSA) 
			sig_alg = aux_cpy_AlgId(dsaWithSHA);
	}

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		aux_add_error(LASTERROR, "af_check_if_onekeypaironly failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}


	/* The prototype certificate of a CA has an ordinary serial number,
	   whereas the serial number of a user's prototype certificate is set to 0 */

	if(cadir){
		serial = af_pse_incr_serial();
		if(!serial){
			fprintf(stderr, "%s: ",cmd);
			fprintf(stderr, "Problems with serial number\n");
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
		if(onekeypaironly == TRUE)
			protocert = af_create_Certificate(cert->tbs->subjectPK, sig_alg, SKnew_name, NULLDNAME, serial);
		else {
			if ( type == SIGNATURE )
				protocert = af_create_Certificate(cert->tbs->subjectPK, sig_alg, SignSK_name, NULLDNAME, serial);
			else
				protocert = af_create_Certificate(cert->tbs->subjectPK, sig_alg, DecSKnew_name, NULLDNAME, serial);
		}
	}
	else{
		if(onekeypaironly == TRUE)
			protocert = af_create_Certificate(cert->tbs->subjectPK, sig_alg, SKnew_name, NULLDNAME, NULLOCTETSTRING);
		else {
			if ( type == SIGNATURE )
				protocert = af_create_Certificate(cert->tbs->subjectPK, sig_alg, SignSK_name, NULLDNAME, NULLOCTETSTRING);
			else
				protocert = af_create_Certificate(cert->tbs->subjectPK, sig_alg, DecSKnew_name, NULLDNAME, NULLOCTETSTRING);
		}
	}


	if (!protocert)       { 
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Can't create prototype certificate\n");
		aux_fprint_error(stderr, verbose);
		exit(-1); 
	}

        if(verbose) {
                fprintf(stderr, "%s: The following prototype certificate was created:\n", cmd);
        	aux_fprint_Certificate(stderr, protocert);
        }

	/* write it to filename or stdout */

	encoded_cert = e_Certificate(protocert);

	if (aux_OctetString2file(encoded_cert, filename, 2)) {
		fprintf(stderr, "%s: Can't create or write %s\n", cmd, filename);
		exit(-1);
	}

	exit(0);
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

        fprintf(stderr, "getkey: Build Prototype Certificate\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'getkey' creates a prototype certificate from a public key previously\n");
	fprintf(stderr, "stored on the PSE, and writes its content to file <proto> or stdout,\n");
	fprintf(stderr, "if <proto> is omitted.\n\n\n");
	
        fprintf(stderr, "usage:\n\n");
	fprintf(stderr, "getkey [-ehsvzVW] [-p <pse>] [-c <cadir>] [proto]\n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <pse>           PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
	fprintf(stderr, "-c <cadir>         Name of CA-directory (default: Environment variable CADIR or %s)\n", DEF_CADIR);
	fprintf(stderr, "-s                 build prototype certificate from public signature key (default)\n");
	fprintf(stderr, "-e                 build prototype certificate from public encryption key\n");
        fprintf(stderr, "-h                 write this help text\n");
	fprintf(stderr, "-z                 control malloc/free behaviour\n");
        fprintf(stderr, "-v                 verbose\n");
        fprintf(stderr, "-V                 Verbose\n");
        fprintf(stderr, "-W                 Grand Verbose (for testing only)\n");
	fprintf(stderr, "<proto>            File containing the resulting prototype certificate (or stdout, if omitted)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM GETKEY */
}
