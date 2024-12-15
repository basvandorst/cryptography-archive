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
KeyInfo         *keyinfo;
KeyInfo         key_hashinput;
int             mode;
Certificate    *cert;
Certificates   *certs;
FCPath         *fcpath;
PKRoot         *pkroot;
HashInput       hashinput;
BitString       in_bits, out_bits;
OctetString     in_octets, out_octets, *ostr, *in_ostr;
AlgId           algid;
ObjId           object_oid;
More            more;
Signature       *verify_signature;
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
	char           *cmd = *parm, opt, x500 = TRUE;
	char           *strmtch();
	char           *buf1, *buf2, *bb, *file, *sig = CNULL,
	               *ctf = CNULL, *fcp = CNULL, *pkr = CNULL;
	char           *originator_name, *originator_alias;
	char           *object = CNULL, *app = CNULL, *pin;
	char            certflag = TRUE, *signatureTimeDate;
	char           *psename = NULL, *cadir = NULL;
	char            k_option = FALSE, c_option = FALSE, f_option = FALSE,
	                t_option = FALSE;
	char           *certname = NULL, *fcpathname = NULL, *pkrootname = NULL,
	               *keyname = NULL;
	int             i, j, in, fd_in = 0, fd_out = 1, out, rc, alg,
	                rest, nfiles = 0, ifiles = 0, nf, optfiles = 0;
	int             c, keyref = 0;
	PSESel         *std_pse;
	unsigned int    blocksize;
	int             SCapp_available;

	char           *proc = "main (sign)";

#ifdef AFDBFILE
	char		afdb[256];
#endif

#ifdef X500
	char	       * env_af_dir_authlevel;
	af_dir_authlevel = DBA_AUTH_SIMPLE;
#endif
	af_access_directory = FALSE;

	af_chk_crl = FALSE;
	sec_time = FALSE;
	af_verbose = FALSE;
	PROG = cmd;

	key.keyref = 0;
	key.key = (KeyInfo *) 0;
	key.pse_sel = (PSESel *) 0;

/*
 *      get args
 */

	optind = 1;
	opterr = 0;

nextopt:


#ifdef X500
	while ((opt = getopt(cnt, parm, "k:f:c:d:t:p:A:hvzFVWTURD")) != -1)
		switch (opt) {
#else
	while ((opt = getopt(cnt, parm, "k:f:c:p:hvzFVWTURD")) != -1)
		switch (opt) {
#endif
		case 'z':
			MF_check = TRUE;
			continue;
		case 'v':
			verbose = 1;
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
		case 'h':
			usage(LONG_HELP);
			continue;
		case 'U':
			sec_time = TRUE;
			continue;
#ifdef SCA
		case 'T':
			SC_verify = TRUE;
			continue;
#endif
                case 'F':
                        af_FCPath_is_trusted = TRUE;
                        continue;
		case 'R':
			af_chk_crl = TRUE;
			continue;
		case 'k':
			k_option = TRUE;
			bb = optarg;
			while (*bb) {
				if (*bb < '0' || *bb > '9') {
					keyname = (char *) malloc(strlen(optarg) + 1);
					strcpy(keyname, optarg);
					break;
				}
				bb++;
			}
			if (!(*bb)) sscanf(optarg, "%d", &keyref);
			continue;
		case 'f':
			f_option = TRUE;
			fcpathname = (char *) malloc(strlen(optarg) + 1);
			strcpy(fcpathname, optarg);
			continue;
		case 'c':
			cadir = optarg;
			continue;
		case 'p':
			psename = optarg;
			continue;
		case 'D':
			af_access_directory = TRUE;
			continue;
#ifdef X500
		case 'd':
			af_dir_dsaname = aux_cpy_String(optarg);
			continue;
		case 't':
			af_dir_tailor = aux_cpy_String(optarg);
			continue;
		case 'A':
			if (! strcasecmp(optarg, "STRONG"))
				af_dir_authlevel = DBA_AUTH_STRONG;
			else if (! strcasecmp(optarg, "SIMPLE"))
				af_dir_authlevel = DBA_AUTH_SIMPLE;
			continue;
#endif
		default:
		case '?':
			usage(SHORT_HELP);
		}


	if (optind < cnt) {
		file = parm[optind];
		if(!optfiles) optfiles = optind;
		ifiles++;
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

	if (!ifiles) usage(SHORT_HELP); /* verify requires a file name */
	if (!nfiles) exit(-1);

	aux_set_pse(psename, cadir);

#ifdef X500
	if (af_dir_authlevel == DBA_AUTH_NONE) {
		env_af_dir_authlevel = getenv("AUTHLEVEL");
		if (env_af_dir_authlevel) {
			if (! strcasecmp(env_af_dir_authlevel, "STRONG"))
				af_dir_authlevel = DBA_AUTH_STRONG;
			else if (! strcasecmp(env_af_dir_authlevel, "SIMPLE"))
				af_dir_authlevel = DBA_AUTH_SIMPLE;
		}
	}
#endif

	if (!(std_pse = af_pse_open(0, FALSE))) {
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	aux_free_PSESel(&std_pse);

	pse_sel.app_name = AF_pse.app_name;
	pse_sel.object.name = CNULL;
	pse_sel.object.pin = AF_pse.pin;
	pse_sel.pin = AF_pse.pin;
	pse_sel.app_id = AF_pse.app_id;

	pse_sel_hashinput.app_name = AF_pse.app_name;
	pse_sel_hashinput.pin = pse_sel.pin;

#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, X500_name);           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif

#ifdef X500
	if (x500 && af_access_directory == TRUE) 
		directory_user_dname = af_pse_get_Name();
#endif

	if (k_option) {
		if (keyref) {
			keyinfo = (KeyInfo *)malloc(sizeof(KeyInfo));
			if (sec_get_key(keyinfo, keyref, (Key *) 0) < 0) {
				aux_fprint_error(stderr, verbose);
				fprintf(stderr, "Can't read key with keyref %d\n", keyref);
				exit(-1);
			}
			key.key = keyinfo;
			key.pse_sel = (PSESel *)0;
		}
		else if (keyname) {
			key.pse_sel = &pse_sel;
			pse_sel.object.name = keyname;
			if (sec_read_PSE(&pse_sel, &object_oid, &out_octets) < 0) {
				aux_fprint_error(stderr, verbose);
				exit(-1);
			}
			if (aux_cmp_ObjId(&object_oid, SignCert_OID) && aux_cmp_ObjId(&object_oid, Cert_OID)) {
				if (!(keyinfo = d_KeyInfo(&out_octets))) {
					aux_fprint_error(stderr, verbose);
					fprintf(stderr, "%s: Can't decode %s. Expected KeyInfo\n", cmd, pse_sel.object.name);
					exit(-1);
				}
				key.key = keyinfo;
				key.pse_sel = (PSESel *)0;
			}
			else if (!(cert = d_Certificate(&out_octets))) {
				aux_fprint_error(stderr, verbose);
				fprintf(stderr, "%s: Can't decode %s. Expected Certificate.\n", cmd, pse_sel.object.name);
				exit(-1);
			}
		}
	}
	if (t_option) {
		pse_sel.object.name = pkrootname;
		if (sec_read_PSE(&pse_sel, &object_oid, &out_octets) < 0) {
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
		if (aux_cmp_ObjId(&object_oid, PKRoot_OID)) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "%s: %s is not a PKRoot\n", cmd, pse_sel.object.name);
			exit(-1);
		}
		if (!(pkroot = d_PKRoot(&out_octets))) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "%s: Can't decode %s\n", cmd, pse_sel.object.name);
			exit(-1);
		}
	}
	if (f_option) {
		pse_sel.object.name = fcpathname;
		if (sec_read_PSE(&pse_sel, &object_oid, &out_octets) < 0) {
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
		if (aux_cmp_ObjId(&object_oid, FCPath_OID)) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "%s: %s is not a FCPath\n", cmd, pse_sel.object.name);
			exit(-1);
		}
		if (!(fcpath = d_FCPath(&out_octets))) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "%s: Can't decode %s\n", cmd, pse_sel.object.name);
			exit(-1);
		}
	}
/*
 *      prepare signature file, certificate file, fcpath file, pkroot file
 */

	for (nf = 0; nf < nfiles; nf++) {
		file = parm[optfiles + nf];
		sig = (char *) malloc(strlen(file) + 8);
		strcpy(sig, file);
		strcat(sig, EXT_SIGNATURE);

/*
 *      	read input file, signature etc. and verify
 */

		ostr = aux_file2OctetString(sig);
		if (!ostr) {
			aux_fprint_error(stderr, verbose);
			free(sig);
			continue;
		}
		free(sig);
		verify_signature = d_Signature(ostr);
		aux_free_OctetString(&ostr);
		if (!verify_signature) {
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "Can't decode signature %s\n", sig);
			continue;
		}
	
		if(!keyinfo) {
			ctf = (char *) malloc(strlen(file) + 8);
			strcpy(ctf, file);
			strcat(ctf, EXT_CERTIFICATE);
			if ((ostr = aux_file2OctetString(ctf))) {
				if (!(certs = d_Certificates(ostr))) {
					aux_fprint_error(stderr, verbose);
					fprintf(stderr, "Can't decode certificates from %s\n", ctf);
					free(ctf);
					aux_free_OctetString(&ostr);
					aux_free_Signature(&verify_signature);
					continue;
				}
				aux_free_OctetString(&ostr);
			}
			free(ctf);
	
			if(!certs) {
	
				/* no .ctf file, read Certificates from PSE */
	
				certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME);
				aux_free_error();
			}
	
			if(!certs) {
				if(cert) aux_create_Certificates(cert, fcpath);
			}
			else {
				if (cert) {
					if(certs->usercertificate) aux_free_Certificate(&certs->usercertificate);
					certs->usercertificate = cert;
				}
				if (fcpath) {
					if(certs->forwardpath) aux_free_FCPath(&certs->forwardpath);
					certs->forwardpath = fcpath;
				}
			}
		}
	
	
		more = SEC_MORE;
		in_ostr = (OctetString *)0;
		fd_in = open(file, O_RDONLY);
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
				fprintf(stderr, "Can't read inputfile %s\n", file);
				if(certs) aux_free_Certificates(&certs);
				continue;
			}
	
			more = SEC_END;
	*/
	
			if (!keyinfo) {
				rc = af_verify(&in_octets, verify_signature, more, certs, (UTCTime *) 0, pkroot);
				if(rc == 0 && more == SEC_MORE) continue;
				if(verbose) aux_fprint_VerificationResult(stderr, verifresult);
				aux_free_VerificationResult(&verifresult);
				if(in_ostr) aux_free_OctetString(&in_ostr);
				aux_free_Signature(&verify_signature);
				if (rc < 0) {
					if(verbose) aux_fprint_error(stderr, verbose);
					fprintf(stderr, "Verification of file %s  f a i l e d\n", file);
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
					aux_free_Certificates(&certs);
					continue;	
				} 
				else {
					originator_name = aux_DName2Name(certs->usercertificate->tbs->subject);
					originator_alias = aux_DName2alias(certs->usercertificate->tbs->subject, LOCALNAME);
					if (originator_alias) fprintf(stderr, "File %s signed by %s", file, originator_alias);
					else fprintf(stderr, "File %s signed by <%s>", file, originator_name);
					if(sec_SignatureTimeDate) {
						signatureTimeDate = aux_readable_UTCTime(sec_SignatureTimeDate);
						fprintf(stderr, " at %s\n", signatureTimeDate);
						free(sec_SignatureTimeDate);
						free(signatureTimeDate);
					}
					else fprintf(stderr, "\n");
					if(originator_name) free(originator_name);
					if(originator_alias) free(originator_alias);	
					aux_free_Certificates(&certs);
				}
			}
			else {
				hashinput.sqmodn_input.nbits = keyinfo->subjectkey.nbits;
				hashinput.sqmodn_input.bits = keyinfo->subjectkey.bits;
				if ((rc = sec_verify(&in_octets, verify_signature, more, &key, &hashinput)) < 0) {
					if(verbose) aux_fprint_error(stderr, verbose);
					fprintf(stderr, "Verification of %s f a i l e d\n", file);
					if(in_ostr) aux_free_OctetString(&in_ostr);
					aux_free_Signature(&verify_signature);				
				}
				else if(more == SEC_END) fprintf(stderr, "Signature of file %s O. K.\n", file);
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
		if(MF_check) MF_fprint(stderr);
	}
	if(sec_time) {
		a_hash_usec = a_hash_usec/1000;
		a_usec = a_usec/1000;
		fprintf(stderr, "Time used for hash computation: %ld.%03ld sec\n", a_hash_sec, a_hash_usec);
		if(rsa_sec || rsa_usec) fprintf(stderr, "Time used for RSA computation:  %ld.%03ld sec\n", a_sec, a_usec);
		else fprintf(stderr, "Time used for DSA computation:  %ld.%03ld sec\n", a_sec, a_usec);
	}
	

	exit(rc);

}


static
void usage(help)
int     help;

{
	if(help == LONG_HELP) {
		aux_fprint_version(stderr);

        	fprintf(stderr, "verify: Verify Signatures of Files\n\n\n");
		fprintf(stderr, "Description:\n\n"); 
		fprintf(stderr, "'verify' verifies the given <files>. It uses algorithms and keys according\n");
		fprintf(stderr, "to the parameter -k (default: Cert/SignCert). For each\n");
		fprintf(stderr, "file in <files> it expects file.sig (containing the signature) and\n");
		fprintf(stderr, "optionally file.ctf (containing the user certificate and the forward\n");
 		fprintf(stderr, "certification path). If file.ctf does not exist, the verification will\n");
		fprintf(stderr, "only succeed if the file was signed by oneself.\n\n\n");
	}
        fprintf(stderr, "usage:\n\n");
#ifdef X500
#ifdef SCA
	fprintf(stderr, "verify [-DRvzFVWTUh] [-p <pse>] [-c <cadir>] [-d <dsa>] [-t <dsaptailor>] [-A <authlevel>] [-k <key>] \n");
#else
	fprintf(stderr, "verify [-DRvzFVWUh] [-p <pse>] [-c <cadir>] [-d <dsa>] [-t <dsaptailor>] [-A <authlevel>] [-k <key>] \n");
#endif
	fprintf(stderr, "       [-f <fcpath>] [<files>]\n\n");
#else
#ifdef SCA
	fprintf(stderr, "verify [-DRvzFVWTUh] [-p <pse>] [-c <cadir>] [-k <key>] [-f <fcpath>] [<files>]\n\n");
#else
	fprintf(stderr, "verify [-DRvzFVWUh] [-p <pse>] [-c <cadir>] [-k <key>] [-f <fcpath>] [<files>]\n\n");
#endif
#endif

        if(help == LONG_HELP) {

        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-D               Retrieve missing certificates from the Directory (X.500 or AF_DB)\n");
	fprintf(stderr, "-F               Consider own FCPath as trusted\n");
        fprintf(stderr, "-R               Consult certificate revocation lists for all cerificates which\n");
        fprintf(stderr, "                 are in the certification path\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for tests only)\n");
        fprintf(stderr, "-z               Control malloc/free behaviour\n");
#ifdef SCA
        fprintf(stderr, "-T               Perform each public key RSA operation in the smartcard  terminal\n");
        fprintf(stderr, "                 instead with the software in the workstation (the latter is default)\n");
#endif
        fprintf(stderr, "-U               Show time used for cryptographic algorithms\n");
        fprintf(stderr, "-h               Write this help text\n");
        fprintf(stderr, "-p <psename>     PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
        fprintf(stderr, "-c <cadir>       name of CA-directory (default: Environment variable CADIR or %s)\n", DEF_CADIR);
#ifdef X500
        fprintf(stderr, "-d <dsa>         name of the DSA to be accessed for retrieving certificates\n");
	fprintf(stderr, "-t <dsaptailor>  Location of dsaptailor file (default: dsaptailor in the ISODE ETCDIR directory)\n");
        fprintf(stderr, "                 and certificate revocation lists\n");
        fprintf(stderr, "-A <authlevel>   Level of authentication used for binding to the X.500 Directory\n");
       	fprintf(stderr, "                 It may be SIMPLE or STRONG (default: environment variable AUTHLEVEL, or NONE, if\n");
        fprintf(stderr, "                 this does not exist). STRONG implies the use of signed DAP operations\n");
#endif
        fprintf(stderr, "-k <key>         PSE-object (containing either a certificate or a key) or key reference \n");
        fprintf(stderr, "                 of verification key. Default: Cert/SignCert\n");
        fprintf(stderr, "-f <fcpath>      name of PSE-object which contains the Forward Certification Path\n");
        fprintf(stderr, "<files>          Filenames\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM VERIFY */
}



p_error(t1, t2, t3)
	char           *t1, *t2, *t3;
{
	fprintf(stderr, "%s", t1);
	if (t2 && strlen(t2))
		fprintf(stderr, ": %s", t2);
	if (t3 && strlen(t3))
		fprintf(stderr, " %s", t3);
	fprintf(stderr, "\n");
	exit(-1);
}

char           *
strmtch(a, b)
	char           *a, *b;
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
