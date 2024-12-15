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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "af.h"

#define BUFSIZE 8192

char           *getenv();
char           *PROG;
Key             key;
PSESel          pse_sel;
PSESel          pse_sel_hashinput;
KeyInfo         keyinfo;
KeyInfo         key_hashinput;
int             mode;
Certificates   *certs;
OctetString    *encoded_certs;
HashInput       hashinput;
BitString       in_bits, out_bits;
OctetString     in_octets, out_octets, *ostr, *in_ostr;
AlgId           algid;
ObjId           object_oid;
More            more;
Signature       sign_signature;
long            a_sec, a_usec, a_hash_sec, a_hash_usec;

char            buf[BUFSIZE], outbuf[512];
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
	extern char    *optarg;
	extern int      optind, opterr;
	char           *cmd = *parm, opt;
	char           *strmtch();
	char           *buf1, *buf2, *bb, *file = CNULL, *sig = CNULL,
	               *ctf = CNULL, *fcp = CNULL, *pkr = CNULL;
	char           *originator_name, *originator_alias;
	char           *object = CNULL, *app = CNULL, *pin;
	char            certflag = TRUE;
	char           *psename = NULL, *cadir = NULL;
	char            k_option = FALSE, c_option = FALSE;
	char           *certname = NULL, *fcpathname = NULL, *pkrootname = NULL;
	int             i, j, in, fd_in = 0, fd_out = 1, out, rc, alg,
	                rest, nfiles = 0, ifiles = 0, nf, optfiles = 0;
	int             c, keyref = 0;
	PSESel         *std_pse;
	unsigned int    blocksize;
	char           *proc = "main (sign)";
	int             SCapp_available;

	PROG = cmd;

	sign_signature.signAI = md5WithRsaTimeDate;	/* default signature
						         * algorithm */

	key.keyref = 0;
	key.key = (KeyInfo *) 0;
	key.pse_sel = (PSESel *) 0;
	key.alg = (AlgId *)0;

	af_verbose = FALSE;
	sec_time = FALSE;


/*
 *      get args
 */

	optind = 1;
	opterr = 0;


nextopt:
	while ((opt = getopt(cnt, parm, "a:k:H:c:p:CfmvzVWUh")) != -1)
		switch (opt) {
		case 'a':
			sign_signature.signAI = aux_Name2AlgId(optarg);
			continue;
		case 'z':
			MF_check = TRUE;
			continue;
		case 'v':
			verbose = 1;
			af_verbose = FALSE;
			continue;
		case 'V':
			verbose = 2;
			af_verbose = TRUE;
			continue;
		case 'W':
			verbose = 2;
			af_verbose = TRUE;
			sec_verbose = TRUE;
			continue;
		case 'f':
			af_sign_check_Validity = FALSE;
			continue;
		case 'h':
			usage(LONG_HELP);
			continue;
		case 'U':
			sec_time = TRUE;
			continue;
		case 'k':
			if(c_option) {
				fprintf(stderr, "Only one of options -k or -C is possible\n");
				exit(-1);
			}
			k_option = TRUE;
			bb = optarg;
			while (*bb) {
				if (*bb < '0' || *bb > '9') {
					key.pse_sel = &pse_sel;
					pse_sel.object.name = optarg;
					break;
				}
				bb++;
			}
			if (!(*bb)) sscanf(optarg, "%d", &key.keyref);
			continue;
		case 'H':
			bb = optarg;
			while (*bb) {
				if (*bb < '0' || *bb > '9') {
					object = optarg;
					break;
				}
				bb++;
			}
			if (!(*bb)) sscanf(optarg, "%d", &keyref);
			build_hashinput(keyref, object);
			continue;
		case 'C':
			if(k_option) {
				fprintf(stderr, "Only one of options -k or -C is possible\n");
				exit(-1);
			}
			c_option = TRUE;
			continue;
		case 'c':
			cadir = optarg;
			continue;
		case 'p':
			psename = optarg;
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}


	if (optind < cnt) {
		file = parm[optind];
		ifiles++;
		if(!optfiles) optfiles = optind;
		if ((fd_in = open(file, O_RDONLY)) <= 0) {
			fprintf(stderr, "Can't open %s\n", file);
		}
		else {
			close(fd_in);
			nfiles++;
		}
		optind++;
		goto nextopt;
	}

	if (!ifiles) usage(SHORT_HELP); /* sign requires a file name */
	if (!nfiles) exit(-1);


	aux_set_pse(psename, cadir);	
	pse_sel.app_name = AF_pse.app_name;
	pse_sel_hashinput.app_name = AF_pse.app_name;


	if (!(std_pse = af_pse_open((ObjId *)0, FALSE))) {
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	aux_free_PSESel(&std_pse);

	pse_sel.app_name = AF_pse.app_name;
	pse_sel.object.pin = AF_pse.pin;
	pse_sel.pin = AF_pse.pin;
	pse_sel.app_id = AF_pse.app_id;

	pse_sel_hashinput.pin = pse_sel.pin;

	if (!(certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME))) {
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}
	if (!(encoded_certs = e_Certificates(certs))) {
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	for (nf = 0; nf < nfiles; nf++) {

/*
 *      	prepare signature file, certificate file, fcpath file, pkroot file
 */

		file = parm[optfiles + nf];

	/*
	 *      read input file and sign to output file
	 */
	
	
		more = SEC_MORE;
		fd_in = open(file, O_RDONLY);
		in_ostr = (OctetString *)0;
		while(more == SEC_MORE) {
			in = read(fd_in, buf, BUFSIZE);
			if(in == BUFSIZE) more = SEC_MORE;
			else more = SEC_END;
			in_octets.octets = buf;
			if(in > 0) in_octets.noctets = in;
			else in_octets.noctets = 0;
	/*
			in_ostr = aux_file2OctetString(file); 
			if (!in_ostr) {
				aux_fprint_error(stderr, verbose);
				continue;
			}
			more = SEC_END;
	*/ 
			if (key.keyref == 0 && !key.pse_sel) {
				if ((rc = af_sign(&in_octets, &sign_signature, more)) < 0) {
					aux_fprint_error(stderr, verbose);
					break;
				}
			} 
			else {
				if ((rc = sec_sign(&in_octets, &sign_signature, more, &key, &hashinput)) < 0) {
					aux_fprint_error(stderr, verbose);
					break;
				}
			}
	
		} 
		close(fd_in); 
	
	
		if(sec_time) {
			a_hash_usec = (a_hash_sec + hash_sec) * 1000000 + a_hash_usec + hash_usec;
			a_hash_sec = a_hash_usec/1000000;
			a_hash_usec = a_hash_usec % 1000000;
			if(rsa_sec || rsa_usec) {
				a_usec = (a_sec + rsa_sec) * 1000000 + a_usec + rsa_usec;
				a_sec = a_usec/1000000;
				a_usec = a_usec % 1000000;
			}
			if(dsa_sec || dsa_usec) {
				a_usec = (a_sec + dsa_sec) * 1000000 + a_usec + dsa_usec;
				a_sec = a_usec/1000000;
				a_usec = a_usec % 1000000;
			}
		}
	
		if(in_ostr) aux_free_OctetString(&in_ostr);
		if (rc < 0) continue;
	
		ostr = e_Signature(&sign_signature);
		if (!ostr) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "BER-Encoding of signature of %s failed\n", file);
			continue;
		}
		sig = (char *) malloc(strlen(file) + 8);
		strcpy(sig, file);
		strcat(sig, EXT_SIGNATURE);
		if (aux_OctetString2file(ostr, sig, 2) < 0) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "Can't create or write signature file %s\n", sig);
			aux_free_OctetString(&ostr);
			free(sig);
			continue;
		}
		aux_free_OctetString(&ostr);
		free(sig);
	
		if (key.keyref == 0 && !key.pse_sel && c_option) {
			ctf = (char *) malloc(strlen(file) + 8);
			strcpy(ctf, file);
			strcat(ctf, EXT_CERTIFICATE);
			if (aux_OctetString2file(encoded_certs, ctf, 2) < 0) {
				aux_fprint_error(stderr, verbose);
				fprintf(stderr, "Can't create or write certificates file %s\n", ctf);
				free(ctf);
				continue;
			}
			free(ctf);
		}
		fprintf(stderr, "File %s signed\n", file);
		if(MF_check) MF_fprint(stderr);
	}
	if(sec_time) {
		a_hash_usec = a_hash_usec/1000;
		a_usec = a_usec/1000;
		fprintf(stderr, "Time used for hash computation: %ld.%03ld sec\n", a_hash_sec, a_hash_usec);
		if(rsa_sec || rsa_usec) fprintf(stderr, "Time used for RSA computation:  %ld.%03ld sec\n", a_sec, a_usec);
		else fprintf(stderr, "Time used for DSA computation:  %ld.%03ld sec\n", a_sec, a_usec);
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
	if(help == LONG_HELP) {
		aux_fprint_version(stderr);

        	fprintf(stderr, "sign:  Sign Files\n\n\n");
		fprintf(stderr, "Description:\n\n"); 
		fprintf(stderr, "'sign' signs the given <files>. It uses algorithms and keys according\n");
		fprintf(stderr, "to the parameters -k and -a (default: SKnew/SignSK). For each\n");
		fprintf(stderr, "file in <files> it produces file.sig (containing the signature) and, if\n");
		fprintf(stderr, "-C is given, file.ctf (containing the user certificate and the forward\n");
 		fprintf(stderr, "certification path).\n\n\n");
	}
        fprintf(stderr, "usage:\n\n");
	fprintf(stderr, "sign [-CvzVWUfh] [-p <pse>] [-c <cadir>] [-k <key>] [-a <alg>] [-H <hashinput>] [<files>]\n\n");

        if(help == LONG_HELP) {

        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-C               Produce .ctf files containing user certificate and forward certification\n");
        fprintf(stderr, "                 path for each file to be signed. Otherwise, produce only .sig files\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for tests only)\n");
        fprintf(stderr, "-U               Show time used for cryptographic algorithms\n");
        fprintf(stderr, "-z               Control malloc/free behaviour\n");
        fprintf(stderr, "-h               Write this help text\n");
	fprintf(stderr, "-f               do not check validity time frame of signature certificate\n");
        fprintf(stderr, "-p <psename>     PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
        fprintf(stderr, "-c <cadir>       name of CA-directory (default: Environment variable CADIR or %s)\n", DEF_CADIR);
        fprintf(stderr, "-k <key>         PSE-object or key reference of signature key. Default: SKnew/SignSK\n");
        fprintf(stderr, "-a <signalg>     Signature algorithm. Default: md5WithRsaTimedate (RSA) or dsaWithSHA (DSS),\n");
        fprintf(stderr, "                 depending on the signature key\n");
        fprintf(stderr, "-H <hashinput>   PSE-object or key reference of hash input (sqmodn only)\n");
        fprintf(stderr, "<files>          Filenames\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM SIGN */
}

/***************************************************************
 *
 * Procedure strmtch
 *
 ***************************************************************/
#ifdef __STDC__

char *strmtch(
	char	 *a,
	char	 *b
)

#else

char *strmtch(
	a,
	b
)
char	 *a;
char	 *b;

#endif

{
	register char  *aa, *bb;

	while (*a) {
		aa = a;
		bb = b;
		while (*aa) {
			if (*aa != *bb)
				break;
			bb++;
			if (*bb == '\0')
				return (aa + 1);
			aa++;
		}
		a++;
	}
	return (CNULL);
}


/***************************************************************
 *
 * Procedure build_hashinput
 *
 ***************************************************************/
#ifdef __STDC__

int build_hashinput(
	int	  keyref,
	char	 *object
)

#else

int build_hashinput(
	keyref,
	object
)
int	  keyref;
char	 *object;

#endif

{
	Certificate    *cert;
	char           *proc = "build_hashinput";

	if (keyref) {
		if (sec_get_key(&key_hashinput, keyref, (Key *) 0) < 0) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "Can't read key with keyref %d\n", keyref);
			exit(-1);
		}
	} 
	else if (object) {
		pse_sel_hashinput.object.name = object;
		if (sec_read_PSE(&pse_sel_hashinput, &object_oid, &out_octets) < 0) {
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
		if (d2_KeyInfo(&out_octets, &key_hashinput) < 0) {
			cert = d_Certificate(&out_octets);
			if(!cert) {
				aux_fprint_error(stderr, verbose);
				fprintf(stderr, "Can't decode PSE-object %s\n", object);
				exit(-1);
			}
			key_hashinput.subjectkey.nbits = cert->tbs->subjectPK->subjectkey.nbits;
			key_hashinput.subjectkey.bits = cert->tbs->subjectPK->subjectkey.bits;
		}
	}
	hashinput.sqmodn_input.nbits = key_hashinput.subjectkey.nbits;
	hashinput.sqmodn_input.bits = key_hashinput.subjectkey.bits;
	pse_sel.pin = pse_sel_hashinput.pin;

	return(0);
}
