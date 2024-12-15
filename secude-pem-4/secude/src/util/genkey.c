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
 *      genkey
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include "cadb.h"

#define SIGNKEY "signature"
#define ENCKEY  "encryption"


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
	int	        keysize = DEF_ASYM_KEYSIZE;
	AlgId         * algorithm = DEF_SUBJECT_SIGNALGID, * sig_alg = DEF_ISSUER_ALGID;
        ObjId         * oid = NULLOBJID, * sig_oid = NULLOBJID;
        PSESel        * pse_sel;
	Key             key;
	KeyInfo         keyinfo;
	Certificate   * cert;
        char          * filename = CNULL;
	OctetString   * newcert;
	extern char	*optarg;
	extern int	optind, opterr;
	KeyType         ktype = SIGNATURE;
	char	        *cmd = *parm, opt, *pin, *keytype = SIGNKEY;
	char	        *psename = CNULL, *cadir = CNULL;
        char            replace = FALSE;
	Boolean         onekeypaironly = FALSE;
	DName	      * owner;
	OctetString   * serial;

	char 		*proc = "main (genkey)";

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "a:s:e:k:c:p:hrvzVWXY")) != -1 ) {
		switch (opt) {
		case 'a':
			if (sig_oid) usage(SHORT_HELP);
                        sig_oid = aux_Name2ObjId(optarg);
                        if (aux_ObjId2AlgType(sig_oid) != SIG) usage(SHORT_HELP);
			sig_alg = aux_ObjId2AlgId(sig_oid);
			break;
		case 's':
			if (oid) usage(SHORT_HELP);
                        oid = aux_Name2ObjId(optarg);
                        if(aux_ObjId2AlgType(oid) != ASYM_ENC && aux_ObjId2AlgType(oid) != SIG)
				usage(SHORT_HELP);
			algorithm = aux_ObjId2AlgId(oid);
			break;
		case 'e':
			if (oid) usage(SHORT_HELP);
			oid = aux_Name2ObjId(optarg);
                        if (aux_ObjId2AlgType(oid) != ASYM_ENC) usage(SHORT_HELP);
			algorithm = aux_ObjId2AlgId(oid);
			ktype = ENCRYPTION;
			keytype = ENCKEY;
			break;
                case 'k':
			keysize = atoi(optarg);
			if ( (keysize < MIN_ASYM_KEYSIZE) || (keysize > MAX_ASYM_KEYSIZE)) usage(SHORT_HELP);
			break;
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
			break;
                case 'r':
                        replace = TRUE;
                        break;
                case 'z':
                        MF_check = TRUE;
                        break;
		case 'v':
			verbose = 1;
			sec_gen_verbose = TRUE;
			break;
		case 'V':
			verbose = 2;
			sec_gen_verbose = TRUE;
			break;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			break;
		case 'X':
			 random_from_pse = TRUE;
			break;
		case 'Y':
			 random_init_from_tty = TRUE;
			break;
		case 'h':
			usage(LONG_HELP);
			break;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}

	if (optind < cnt) filename = parm[optind++];

	aux_set_pse(psename, cadir);

	if(af_check_if_onekeypaironly(&onekeypaironly)){
		aux_add_error(LASTERROR, "af_check_if_onekeypaironly failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}
	
	owner = af_pse_get_Name();
	if(!owner){
		fprintf(stderr, "%s: ", cmd);
		fprintf(stderr, "Cannot determine owner of PSE\n"); 
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	if((aux_ObjId2AlgType(algorithm->objid) != ASYM_ENC) && (onekeypaironly == TRUE)){
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Wrong AlgType for a PSE which shall hold one keypair only\n");
		aux_add_error(EINVALID, "oid", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	key.keyref = 0;
	key.pse_sel = (PSESel * ) 0;
	key.key = &keyinfo;
	keyinfo.subjectAI = aux_cpy_AlgId(algorithm);
        if(aux_ObjId2ParmType(algorithm->objid) != PARM_NULL)
	             *(int *)(keyinfo.subjectAI->param) = keysize;

        if(verbose) fprintf(stderr, "%s: Generating %s key pair (Algorithm %s)\n        for <%s> with PSE %s ...\n", cmd, keytype, aux_ObjId2Name(algorithm->objid), aux_DName2Name(af_pse_get_Name()), AF_pse.app_name);

	rc = af_gen_key(&key, ktype, replace);

	if (rc < 0)       {            
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Can't generate new keys");
		aux_add_error(EINVALID, "Can't generate new keys", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1); 
	}
	else if(verbose) fprintf(stderr, "%s: Key generation (%s) O.K.\n", cmd, keytype);

	if(cadir){
		serial = af_pse_incr_serial();
		if(!serial){
			fprintf(stderr, "%s: ",cmd);
			fprintf(stderr, "Problems with serial number\n");
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
		if(onekeypaironly == TRUE)
			cert = af_create_Certificate(&keyinfo, sig_alg, SKnew_name, owner, serial);
		else{
			if ( ktype == SIGNATURE )
				cert = af_create_Certificate(&keyinfo, sig_alg, SignSK_name, owner, serial);
			else
				cert = af_create_Certificate(&keyinfo, sig_alg, DecSKnew_name, owner, serial);
		}
	}
	else{
		if(onekeypaironly == TRUE)
			cert = af_create_Certificate(&keyinfo, sig_alg, SKnew_name, owner, NULLOCTETSTRING);
		else{
			if ( ktype == SIGNATURE )
				cert = af_create_Certificate(&keyinfo, sig_alg, SignSK_name, owner, NULLOCTETSTRING);
			else
				cert = af_create_Certificate(&keyinfo, sig_alg, DecSKnew_name, owner, NULLOCTETSTRING);
		}
	}

	if (!cert)       { 
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Can't create prototype certificate");
		aux_add_error(EINVALID, "Can't create prototype certificate", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1); 
	}

        if(verbose) {
                fprintf(stderr, "%s: The following prototype certificate was created:\n", cmd);
        	aux_fprint_Certificate(stderr, cert);
        }

	if (af_pse_update_Certificate(ktype, cert, TRUE) < 0) {
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "unable to store prototype certificate on PSE");
		aux_add_error(EINVALID, "unable to store prototype certificate on PSE", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1); 
	}

	/* write it to filename or stdout */

	newcert = e_Certificate(cert);

	if (aux_OctetString2file(newcert, filename, 2)) {
		aux_add_error(EINVALID, "Can't create or write", filename, char_n, proc);
		aux_fprint_error(stderr, verbose);
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

        fprintf(stderr, "genkey: Generate Key and Prototype Certificate\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'genkey' generates an asymmetric keypair and installs the secret component on the PSE.\n");
	fprintf(stderr, "The public component of the keypair is wrapped into a self-signed prototype certificate\n");
	fprintf(stderr, "which is stored on the PSE and written to the file <proto> or stdout, if <proto> is omitted.\n\n\n");

        fprintf(stderr, "usage:\n\n");
        fprintf(stderr, "genkey [-hrvzVWXY] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-s <signalg>] [-k <keysize>]\n");
	fprintf(stderr, "       [-e <encalg>] [-k <keysize>] [proto]\n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <pse>           PSE name (default: environment variable PSE or %s)\n", DEF_PSE);
	fprintf(stderr, "-c <cadir>         Name of CA-directory (default: environment variable CADIR or %s)\n", DEF_CADIR);
	fprintf(stderr, "-a <issueralg>     Issuer algorithm associated with the signature of the prototype certificate\n");
	fprintf(stderr, "                   (default: md2WithRsaEncryption)\n");
	fprintf(stderr, "-s <signalg>       Signature algorithm (default: rsa)\n");
	fprintf(stderr, "-k <keysize>       Keysize of RSA signature key\n");
	fprintf(stderr, "-e <encalg>        Encryption algorithm (default: rsa)\n");
	fprintf(stderr, "-k <keysize>       Keysize of RSA encryption key\n");
	fprintf(stderr, "-r                 replace a previously generated secret key\n");
        fprintf(stderr, "-h                 write this help text\n");
	fprintf(stderr, "-z                 control malloc/free behaviour\n");
        fprintf(stderr, "-v                 verbose\n");
        fprintf(stderr, "-V                 Verbose\n");
        fprintf(stderr, "-W                 Grand Verbose (for testing only)\n");
        fprintf(stderr, "-X                 Read random number generator seed from PSE-object Random\n");
        fprintf(stderr, "-Y                 Init random number generator seed through keyboard input\n");
	fprintf(stderr, "<proto>            File containing the resulting prototype certificate (or stdout, if omitted)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM GENKEY */
}
