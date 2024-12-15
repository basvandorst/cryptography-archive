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
 *      psecreate
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include "cadb.h"

#define SIGNKEY "signature"
#define ENCKEY "encryption"

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
	PSESel	        *pse;
        int             rc, i;
	int	        keysize[2];
	char	        line[256];
	AlgId           *algorithm = DEF_SUBJECT_SIGNALGID, *sig_alg = DEF_ISSUER_ALGID;
	AlgId           *s_algorithm = DEF_SUBJECT_SIGNALGID, *e_algorithm = DEF_SUBJECT_SIGNALGID;
        ObjId           *oid = NULLOBJID, *sig_oid = NULLOBJID;
        PSESel          *pse_sel;
	Name            *afname;
	DName		*af_dname;
	PSEToc          *psetoc;
	Key             key;
	KeyInfo         keyinfo;
	Certificate     *cert, *signcert;
	OctetString     *newcert, *ostr;
	extern char	*optarg;
	extern int	optind, opterr;
	KeyType         ktype = SIGNATURE;
	char	        *cmd = *parm, opt, *keytype;
	char	        *psename = CNULL;
        char            *pkrootfile;
        PKRoot          *pkroot;
	Boolean         onekeypaironly = TRUE;
	PSELocation     pse_location;


	char 		*proc = "main (psecreate)";

	psename	 = getenv("PSE");
	afname	 = getenv("MYDNAME");
	keysize[0] = keysize[1] = DEF_ASYM_KEYSIZE;

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

nextopt:
	while ( (opt = getopt(cnt, parm, "s:e:k:p:hqvzVWXY")) != -1 ) {
		switch (opt) {
		case 's':
                        oid = aux_Name2ObjId(optarg);
                        if(aux_ObjId2AlgType(oid) != ASYM_ENC && aux_ObjId2AlgType(oid) != SIG)
				usage(SHORT_HELP);
			s_algorithm = aux_ObjId2AlgId(oid);
			ktype = SIGNATURE;
			break;
		case 'e':
			oid = aux_Name2ObjId(optarg);
                        if (aux_ObjId2AlgType(oid) != ASYM_ENC) usage(SHORT_HELP);
			e_algorithm = aux_ObjId2AlgId(oid);
			ktype = ENCRYPTION;
			break;
                case 'k':
			keysize[ktype] = atoi(optarg);
			if(ktype == SIGNATURE && aux_ObjId2AlgEnc(s_algorithm->objid) == DSA) sec_dsa_keysize = keysize[ktype];
			if ( (keysize[ktype] < MIN_ASYM_KEYSIZE) || (keysize[ktype] > MAX_ASYM_KEYSIZE)) usage(SHORT_HELP);
			break;
		case 'p':
			psename = optarg;
			break;
                case 'q':
                        onekeypaironly = FALSE;
			keytype = SIGNKEY;
                        break;
                case 'z':
                        MF_check = TRUE;
                        break;
		case 'v':
			verbose = 1;
			sec_gen_verbose = TRUE;
			continue;
		case 'V':
			verbose = 2;
			sec_gen_verbose = TRUE;
			continue;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			continue;
		case 'X':
			 random_from_pse = TRUE;
			 break;
		case 'Y':
			 random_init_from_tty = TRUE;
			 break;
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}

	if (optind < cnt) {
		afname = parm[optind++];
		goto nextopt;
	}

	if((aux_ObjId2AlgType(s_algorithm->objid) != ASYM_ENC) && (onekeypaironly == TRUE)){
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Wrong AlgType for a PSE which shall hold one keypair only\n");
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	aux_set_pse(psename, CNULL);

	if((pse_location = sec_psetest(AF_pse.app_name)) == ERR_in_psetest) {
		if (aux_last_error() == EDEVLOCK) 
			fprintf(stderr, "Cannot open device for SCT (No such device or device busy)\n");
		else	fprintf(stderr, "Error during SC configuration.\n");
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	if (pse_location == SCpse) {
/*		name = aux_DName2Attr(af_dname, "CN");*/
		fprintf(stderr, "    Please insert smartcard of %s\n", afname);
	}

	/*
	 *  Set global flag "sec_onekeypair" used in function "sec_create"
	 */

	sec_onekeypair = onekeypaironly;

	/* create new PSE */

	if ( !(pse = af_pse_create((ObjId *)0))) {
                fprintf(stderr, "%s: ", cmd);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}
	aux_free_PSESel(&pse);

	accept_alias_without_verification = TRUE;
        if(!afname) { /* read owner's DN from stdin */
again:
                fprintf(stderr, "%s: Distinguished name of PSE owner: ", cmd);
                line[0] = '\0';
                gets(line);
                afname = line;
		if(!(af_dname = aux_alias2DName(afname))) {
                        fprintf(stderr, "%s: Invalid distinguished name\n", cmd);
                        goto again;
                }
                if(!(ostr = e_DName(af_dname))) {
                        fprintf(stderr, "%s: Invalid distinguished name\n", cmd);
                        goto again;
                }
        }
        else {
		af_dname = aux_alias2DName(afname);
		ostr = e_DName(af_dname);
		if(! ostr){
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
	}
	if ((optind < cnt) || !afname || !ostr ) usage(SHORT_HELP);

	af_dname = d_DName(ostr);
	aux_free_OctetString(&ostr);

	algorithm = s_algorithm;
	keytype = SIGNKEY;

genkey:

	key.keyref = 0;
	key.pse_sel = (PSESel * ) 0;
	key.key = &keyinfo;

	keyinfo.subjectAI = aux_cpy_AlgId(algorithm);
        if(aux_ObjId2ParmType(algorithm->objid) != PARM_NULL)
	             *(int *)(keyinfo.subjectAI->param) = keysize[ktype];

        if(verbose) fprintf(stderr, "%s: Generating %s key pair (Algorithm %s)\n        for <%s> with PSE %s ...\n", cmd, keytype, aux_ObjId2Name(algorithm->objid), aux_DName2Name(af_dname), AF_pse.app_name);
	
        else sec_verbose = FALSE;

	rc = af_gen_key(&key, ktype, FALSE);

	if (rc < 0)       {            
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Can't generate new keys\n");
		aux_fprint_error(stderr, verbose);
		exit(-1); 
	}
	else if(verbose) fprintf(stderr, "%s: Key generation (%s) O.K.\n", cmd, keytype);

	if(aux_ObjId2AlgEnc(keyinfo.subjectAI->objid) == RSA) 
		sig_alg = aux_cpy_AlgId(md2WithRsaEncryption);
	if(aux_ObjId2AlgEnc(keyinfo.subjectAI->objid) == DSA) 
		sig_alg = aux_cpy_AlgId(dsaWithSHA);

	if(onekeypaironly == TRUE)
		cert = af_create_Certificate(&keyinfo, sig_alg, SKnew_name, af_dname, NULLOCTETSTRING);
	else {
		if ( ktype == SIGNATURE )
			cert = af_create_Certificate(&keyinfo, sig_alg, SignSK_name, af_dname, NULLOCTETSTRING);
		else
			cert = af_create_Certificate(&keyinfo, sig_alg, DecSKnew_name, af_dname, NULLOCTETSTRING);
	}

	if (!cert) { 
		fprintf(stderr, "%s: ",cmd);
	        fprintf(stderr, "Can't create prototype certificate\n");
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
	        fprintf(stderr, "Can't write prototype certificate to PSE\n");
		aux_fprint_error(stderr, verbose);
		exit(-1); 
	}

	if(onekeypaironly == FALSE && ktype == SIGNATURE) {
		signcert = cert;
		ktype = ENCRYPTION;
		algorithm = e_algorithm;
		keytype = ENCKEY;
		goto genkey;
	}
	if(onekeypaironly) signcert = cert;

	/* Create PKRoot from prototype certificate in Cert  */

	pkroot = aux_create_PKRoot(signcert, NULLCERTIFICATE);
	if(!pkroot || !pkroot->newkey) {
		fprintf(stderr, "Cannot create PKRoot from prototype certificate");
		exit(-1);
	}

	if(pkroot->newkey->sig) aux_free_Signature(&pkroot->newkey->sig);

	if(af_pse_update_PKRoot(pkroot) < 0) {
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "can't write PKRoot to PSE\n");
		aux_fprint_error(stderr, verbose);
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

        fprintf(stderr, "psecreate: Create User PSE\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'psecreate'  creates a User PSE with one or two asymmetric keypairs on it,\n");
	fprintf(stderr, "whose public keys are held within self-signed prototype certificates.\n\n\n");

        fprintf(stderr, "usage:\n\n");
        fprintf(stderr,"psecreate [-hqvzVWXY] [-p <pse>] [-s <signalg>] [-k <keysize>] [-e <encalg>] [-k <keysize>] [Name]\n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <pse>         PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
	fprintf(stderr, "-s <signalg>     Signature algorithm (default: rsa)\n");
	fprintf(stderr, "-k <keysize>     Keysize of RSA signature key\n");
	fprintf(stderr, "-e <encalg>      Encryption algorithm (default: rsa)\n");
	fprintf(stderr, "-k <keysize>     Keysize of RSA encryption key\n");
	fprintf(stderr, "-q               create PSE that contains two RSA keypairs (default: one RSA keypair only)\n");
        fprintf(stderr, "-h               write this help text\n");
	fprintf(stderr, "-z               control malloc/free behaviour\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
        fprintf(stderr, "-X               Read random number generator seed from PSE-object Random\n");
        fprintf(stderr, "-Y               Init random number generator seed through keyboard input\n");
	fprintf(stderr, "<Name>           Intended owner of the generated User PSE\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM PSECREATE */
}
