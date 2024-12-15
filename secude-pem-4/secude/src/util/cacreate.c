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
	CA creation


*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#ifdef NDBM
#include <ndbm.h>  
#else
#include <dbm.h>
#endif

#include "cadb.h"

#ifndef S_IRWXU
#define S_IRWXU (S_IREAD|S_IWRITE|S_IEXEC)
#endif

extern int errno;
extern char *sys_errlist[ ];

static int	cainit(), encrinit(), localinit(), crlinit();

Certificate      * af_create_Certificate();
Crl		 * CRL2Crl();

char	* getenv();
char    * cmd;
UTCTime * notbefore, * notafter;
Name	* caname;

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
	int	        i;
	RC		rc;
	DName		*ca_dname;
	char	        *psename, *cadir, *cadir_abs, * nextupdate = CNULL;
	char	        *home, *pin;
	Boolean	        onekeypaironly = TRUE, root = FALSE;
	OctetString	*encname;
	extern char	*optarg;
	extern int	optind, opterr;
	char	        opt, line[256];
	AlgId           *algorithm = (AlgId * )0, *sig_alg = (AlgId * )0;
	AlgId           *s_alg = DEF_SUBJECT_SIGNALGID, *e_alg = DEF_SUBJECT_ENCRALGID;
        ObjId           *oid;
	OctetString     *ostr;
        int             keysizes[2];
        int             ka = 0, kk = 0, kx = 0;
	int	        keysize = DEF_ASYM_KEYSIZE;
	OctetString     *serial = NULLOCTETSTRING;
	int             rcode;

#ifdef X500
	char	        * env_af_dir_authlevel;
#endif

	char 		*proc = "main (cacreate)";


        cmd = *parm;
        cadir = CNULL;
	caname	 = CNULL;
	psename	 = CNULL;
        sec_verbose = FALSE;

	keysizes[0] = DEF_ASYM_KEYSIZE;
	keysizes[1] = DEF_ASYM_KEYSIZE;
	ka = 2;

	optind = 1;
	opterr = 0;

	af_access_directory = FALSE;
	MF_check = FALSE;

	notbefore = notafter = (UTCTime * )0;

nextopt:

#ifdef X500
	while ( (opt = getopt(cnt, parm, "a:A:d:t:c:s:e:k:p:n:l:f:u:hqrvzDVW")) != -1 ) {
#else
	while ( (opt = getopt(cnt, parm, "a:c:s:e:k:p:n:l:f:u:hqrvzDVW")) != -1 ) {
#endif
		switch (opt) {
		case 'a':
                        oid = aux_Name2ObjId(optarg);
                        if (aux_ObjId2AlgType(oid) != SIG) usage(SHORT_HELP);
			algorithm = aux_ObjId2AlgId(oid);
			break;
		case 'n':
			if (serial) usage(SHORT_HELP);
			else serial = aux_create_SerialNo(optarg);
			break;
		case 'f':
			if (notbefore) usage(SHORT_HELP);
			else notbefore = optarg;
			break;
		case 'l':
			if (notafter) usage(SHORT_HELP);
			else notafter = optarg;
			break;
		case 'u':
			if (nextupdate) usage(SHORT_HELP);
			else nextupdate = optarg;
			break;
		case 's':
                        oid = aux_Name2ObjId(optarg);
                        if ( (aux_ObjId2AlgType(oid) != SIG) && (aux_ObjId2AlgType(oid) != ASYM_ENC) ) 
				usage(SHORT_HELP);
			s_alg = aux_ObjId2AlgId(oid);
			kk = 0;
			break;
		case 'e':
                        oid = aux_Name2ObjId(optarg);
                        if(aux_ObjId2AlgType(oid) != ASYM_ENC) usage(SHORT_HELP);
			e_alg = aux_ObjId2AlgId(oid);
			kk = 1;
			break;
                case 'k':
			keysize = atoi(optarg);
			if ( (keysize < MIN_ASYM_KEYSIZE) || (keysize > MAX_ASYM_KEYSIZE)) usage(SHORT_HELP);
			keysizes[kk] = keysize;
			break;
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
			break;
                case 'q':
                        onekeypaironly = FALSE;
                        break;
                case 'r':
                        root = TRUE;
                        break;
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
		case 'D':
                        af_access_directory = TRUE;
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
		caname = parm[optind++];
		goto nextopt;
	}


	if ((optind < cnt)) usage(SHORT_HELP);

	if(algorithm){
		if(aux_ObjId2AlgEnc(algorithm->objid) != aux_ObjId2AlgEnc(s_alg->objid)) {
			fprintf(stderr, "%s: ",cmd);
                	fprintf(stderr, "signature algorithm does not fit to signature key\n");
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
	}

	if(root == TRUE && onekeypaironly == FALSE){
		if(aux_ObjId2AlgEnc(s_alg->objid) != aux_ObjId2AlgEnc(e_alg->objid)) {
			fprintf(stderr, "%s: ",cmd);
                	fprintf(stderr, "Signature and encryption keypairs of Root-CA have different algorithms.\n");
                	fprintf(stderr, "          Therefore, ENCRYPTION certificate cannot be signed with signature key\n");
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
	}

	if(!serial) {
		serial = aux_new_OctetString(1);
		serial->octets[0] = 0x00;
	}

	sig_alg = algorithm;
	if(!sig_alg){
		if(aux_ObjId2AlgEnc(s_alg->objid) == RSA) 
			sig_alg = aux_cpy_AlgId(md2WithRsaEncryption);
		if(aux_ObjId2AlgEnc(s_alg->objid) == DSA) 
			sig_alg = aux_cpy_AlgId(dsaWithSHA);
	}


	/* first steps: 
		create PSE
		associate name
		generate sign key
		store this as PKRoot
		and a pseudo certificate
	   gives PSE with signature key
	*/

        if(!cadir) cadir = DEF_CADIR;

        if(cadir[0] != PATH_SEPARATION_CHAR) {
		home = getenv("HOME");
		if (!home) home = "";
		cadir_abs = (char *)malloc(strlen(home)+strlen(cadir)+10);
		if (!cadir_abs) {
			aux_add_error(EMALLOC, "cadir_abs", cmd, char_n, proc);
			exit(-1);
		}
		strcpy(cadir_abs, home);
		strcat(cadir_abs, PATH_SEPARATION_STRING);
		strcat(cadir_abs, cadir);
	}
	else {
		cadir_abs = (char *)malloc(strlen(cadir)+10);
		if (!cadir_abs) {
			aux_add_error(EMALLOC, "cadir_abs", cmd, char_n, proc);
			exit(-1);
		}
		strcpy(cadir_abs, cadir);
	}
		
	if (mkdir(cadir_abs, S_IRWXU) < 0) {
                fprintf(stderr, "%s: Can't create %s (%d %s)\n", cmd, cadir, errno, sys_errlist[errno]);
		aux_add_error(EINVALID, "Can't create", cmd, char_n, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

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

	accept_alias_without_verification = TRUE;
        if(!caname) { /* read CA directory name from stdin */
again:
                fprintf(stderr, "%s: Directory name of new CA: ", cmd);
                line[0] = '\0';
                gets(line);
                caname = line;
		if(!(ca_dname = aux_alias2DName(caname))) {
                        fprintf(stderr, "%s: Invalid directory name\n", cmd);
                        goto again;
                }
        }
        else {
		if(!(ca_dname = aux_alias2DName(caname))) {
                        fprintf(stderr, "%s: Invalid directory name\n", cmd);
			aux_add_error(EINVALID, "Invalid directory name", cmd, char_n, proc);
			aux_fprint_error(stderr, verbose);
                        exit(-1);
                }
	}

	if (localinit(caname, cadir_abs) != 0) {
                rmdir(cadir);
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "Can't init CA");
		aux_add_error(EINVALID, "localinit failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
                exit(-1);
        }

	if (cainit(ca_dname, s_alg, cadir_abs, serial, sig_alg, keysizes[0], onekeypaironly, root) != 0) {
                rmdir(cadir);
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "Can't init CA");
		aux_add_error(EINVALID, "cainit failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
                exit(-1);
        }

	if (onekeypaironly == FALSE) { 
		if(encrinit(ca_dname, e_alg, cadir_abs, keysizes[1], root) != 0) {
                	rmdir(cadir);
			fprintf(stderr, "%s: ",cmd);
                	fprintf(stderr, "Can't init CA");
			aux_add_error(EINVALID, "encrinit failed", CNULL, 0, proc);
			aux_fprint_error(stderr, verbose);
                	exit(-1);
        	}
	}

	if(crlinit(cadir_abs, sig_alg, nextupdate) != 0){
                rmdir(cadir);
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "Can't init CA");
		aux_add_error(EINVALID, "crlinit failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
                exit(-1);
	}

        if(verbose) {
        	fprintf(stderr, "%s: CA with issuer name <%s> complete.\n", cmd, caname);
        	fprintf(stderr, "          CA-DB resides in ");
                if(cadir[0] != PATH_SEPARATION_CHAR) fprintf(stderr, "%s%s", home, PATH_SEPARATION_STRING);
                fprintf(stderr, "%s\n", cadir);
                fprintf(stderr, "          PSE %s contains PSE objects PKRoot, ", AF_pse.app_name);
		if(onekeypaironly == TRUE) fprintf(stderr, "SKnew, Cert,\n");
		else fprintf(stderr, "SignSK, SignCert, DecSKnew, EncCert,\n");
                fprintf(stderr, "                                             SerialNumber, CrlSet\n");
        }

	exit(0);
}


static int	
cainit(ca, s_alg, cadir_abs, serial, sig_alg, keysize, onepaironly, root)
DName	      * ca;
AlgId         * s_alg;
char          * cadir_abs;
OctetString   * serial;
AlgId         * sig_alg;
int             keysize;
Boolean         onepaironly;
Boolean	        root;
{
        PSESel      * pse, * pse_sel;
	PSEToc      * psetoc;
	Key	      nkey;
	KeyInfo       nkinfo;
	PKRoot	    * pkroot;
	Certificate * cert;
	OctetString   content;
	ObjId         objid;       
	ObjId       * obj_type;
	int	      fd, rc;
	Boolean       x500 = TRUE;
	int	      i;
	char        * logpath;
#ifdef AFDBFILE
	char	      afdb[256];
#endif
	char        * proc = "cainit";

	/* init ca data files */

	logpath = (char *)malloc(strlen(cadir_abs)+10);
	strcpy(logpath, cadir_abs);
	strcat(logpath, PATH_SEPARATION_STRING);
	strcat(logpath, CALOG);

	if ((logfile = fopen(logpath, LOGFLAGS)) == (FILE * ) 0) return(-1);
	LOGINIT;

	/*
	 *  Create PSE
         *
	 *  Set global flag "sec_onekeypair" used in function "sec_create"
	 */

	sec_onekeypair = onepaironly;

	if(!(pse = af_pse_create(NULLOBJID))) {
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "PSE creation failed");
		aux_add_error(EINVALID, "PSE creation failed", CNULL, 0, proc);
		LOGSECERR;
		return(-1);
	}
	aux_free_PSESel(&pse);


	/* create PSE object "SerialNumber" */

	if(af_pse_update_SerialNumber(serial) < 0){
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "unable to create SerialNumber on PSE");
		aux_add_error(EINVALID, "unable to create SerialNumber on PSE", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	/* now generate a new sign key, with default parm */

	nkey.keyref = 0;
	nkey.pse_sel = (PSESel * ) 0;
	nkey.key = &nkinfo;
	nkinfo.subjectAI = s_alg;
	if(aux_ObjId2ParmType(s_alg->objid) != PARM_NULL)
		*(int *)(nkinfo.subjectAI->param) = keysize;

	if(verbose) {
		if(onepaironly) fprintf(stderr, "%s: Generating CA key pair (algorithm %s)...\n", cmd, aux_ObjId2Name(s_alg->objid));
		else fprintf(stderr, "%s: Generating CA signature key pair (algorithm %s)...\n", cmd, aux_ObjId2Name(s_alg->objid));
	}

	if (af_gen_key(&nkey, SIGNATURE, FALSE) < 0) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "unable to generate sign key");
		aux_add_error(EINVALID, "unable to generate sign key", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	/* now build a prototype certificate ... */
	if(onepaironly == TRUE)
		cert = af_create_Certificate(&nkinfo, sig_alg, SKnew_name, ca, serial);
	else
		cert = af_create_Certificate(&nkinfo, sig_alg, SignSK_name, ca, serial);

	if (! cert) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "unable to create prototype certificate\n");
		aux_add_error(EINVALID, "unable to create prototype certificate", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	if(root && (notbefore || notafter)){
		/* af_sign() shall only be performed by RootCA, whose ENCRYPTION and
                   SIGNATURE certificates shall both be verified with its secret SIGNATURE
	           key  
		*/
	
		/* tbs->valid with its 'notbefore' and 'notafter' fields has been allocated
	           by routine af_create_Certificate() above
		 */

		if(notbefore) {
			free(cert->tbs->valid->notbefore);
			cert->tbs->valid->notbefore = notbefore;
		}
		if(notafter) {
			free(cert->tbs->valid->notafter);
			cert->tbs->valid->notafter = notafter;
		}
	
		/* re-encode ToBeSigned and re-sign certificate */
	
		aux_free_OctetString(&cert->tbs_DERcode);
		cert->tbs_DERcode = e_ToBeSigned(cert->tbs);
		if(!cert->tbs_DERcode){
			if(onepaironly == TRUE) aux_add_error(EENCODE, "e_ToBeSigned of prototype certificate FAILED", CNULL, 0, proc);
			else aux_add_error(EENCODE, "e_ToBeSigned of SIGNATURE prototype certificate FAILED", CNULL, 0, proc);
			LOGAFERR;
			return(-1);
		}

		if (af_sign(cert->tbs_DERcode, cert->sig, SEC_END) < 0) {
			aux_add_error(EINVALID, "Can't self-sign signature certificate", CNULL, 0, proc);
			LOGAFERR;
			return(-1);
		}
	}

	/* ... and store it on the PSE */
	if (af_pse_update_Certificate(SIGNATURE, cert, TRUE) < 0) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "unable to store self-signed signature certificate on PSE");
		aux_add_error(EINVALID, "unable to store self-signed signature certificate on PSE", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	/* now build PKRoot with public key */

	if(!(pkroot = aux_create_PKRoot(cert, (Certificate *)0))) {
		fprintf(stderr, "%s: ",cmd);
		  fprintf(stderr, "Can't create PKRoot\n");
		aux_add_error(aux_last_error(), "Can't create PKRoot", CNULL, 0, proc);
		return(-1);
	}

	/* ... and install it so far */
	if (af_pse_update_PKRoot(pkroot) < 0) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "update of PKRoot failed");
		aux_add_error(EINVALID, "update of PKRoot failed", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	/* Enter SignCert to certdb: */

	if(onepaironly == TRUE)
		af_cadb_add_Certificate(3, cert, cadir_abs);
	else
		af_cadb_add_Certificate(SIGNATURE, cert, cadir_abs);


	/* Enter SignCert into Directory: */	

	if (af_access_directory == TRUE) {
#ifdef AFDBFILE
		/* Determine whether X.500 directory shall be accessed */
		strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
		strcat(afdb, X500_name);           /* file = .af-db/'X500' */
		if (open(afdb, O_RDONLY) < 0) 
			x500 = FALSE;
#endif
#ifdef X500
		if ( x500 ) {
			directory_user_dname = aux_cpy_DName(cert->tbs->subject);
			if ( verbose ) {
				fprintf(stderr, "%s: Accessing the X.500 directory entry of ", cmd);
				fprintf(stderr, "owner = \"%s\" ...\n", aux_DName2Name(cert->tbs->subject));
			} 
			rc = af_dir_enter_Certificate(cert, cACertificate);
			if ( verbose ) {
				if ( rc < 0 )
					fprintf(stderr, "%s: Directory entry (X.500) failed.\n", cmd);
				else fprintf(stderr, "%s: Certificate entered into X.500 Directory.\n", cmd);
			      }
		}
#endif
#ifdef AFDBFILE
		if ( verbose ) {
			fprintf(stderr, "%s: Accessing the .af-db directory entry of ", cmd);
			fprintf(stderr, "owner = \"%s\" ...\n", aux_DName2Name(cert->tbs->subject));
		} 
		rc = af_afdb_enter_Certificate(cert, SIGNATURE);
		if ( verbose ) {
			if ( rc < 0 )
				fprintf(stderr, "%s: Directory entry (.af-db) failed.\n", cmd);
			else{
				if(onepaironly == TRUE)
					fprintf(stderr, "%s: Certificate entered into .af-db Directory.\n", cmd);
				else
					fprintf(stderr, "%s: %s Certificate entered into .af-db Directory.\n", cmd, "SIGNATURE");
			}
		}
#endif
	}  /* if(af_access_directory) */

	return(0);
}


static int	
localinit(ca, cadir_abs)
Name	* ca;
char    * cadir_abs;
{
#define	DBMOPENFL	O_RDWR|O_CREAT, S_IREAD|S_IWRITE

#ifdef NDBM
	DBM	* user;
	DBM	* cert;
	DBM     * crl;
#else
	FILE    * fd;
	char	  fn[64];
#endif
	char    * userdbpath, * certdbpath, * crldbpath;

	char    * proc = "localinit";

	userdbpath = (char *)malloc(strlen(cadir_abs) + 10);
	strcpy(userdbpath, cadir_abs);
	strcat(userdbpath, PATH_SEPARATION_STRING);
	strcat(userdbpath, USERDB);

	certdbpath = (char *)malloc(strlen(cadir_abs) + 10);
	strcpy(certdbpath, cadir_abs);
	strcat(certdbpath, PATH_SEPARATION_STRING);
	strcat(certdbpath, CERTDB);

	crldbpath = (char *)malloc(strlen(cadir_abs) + 20);
	strcpy(crldbpath, cadir_abs);
	strcat(crldbpath, PATH_SEPARATION_STRING);
	strcat(crldbpath, CRLDB);

	
	/* user dbm */

#ifdef NDBM
	user = dbm_open(userdbpath, DBMOPENFL);
	if (!user) return(-1);
	dbm_close(user);
#else
	strcpy(fn, userdbpath);
	strcat(fn, ".pag");
	fd = fopen(fn, "r");
	if (fd) fclose(fd);
	else {
		fd = fopen(fn, "w");
		fclose(fd);
	}	
	strcpy(fn, userdbpath);
	strcat(fn, ".dir");
	fd = fopen(fn, "r");
	if (fd)	fclose(fd);
	else {
		fd = fopen(fn, "w");
		fclose(fd);
	}
	if (dbminit(userdbpath) < 0) return(-1);
	else dbmclose();
#endif


	/* cert dbm */

#ifdef NDBM
	cert = dbm_open(certdbpath, DBMOPENFL);
	if (!cert) return(-1);
	dbm_close(cert);
#else
	strcpy(fn, certdbpath);
	strcat(fn, ".pag");
	fd = fopen(fn, "r");
	if (fd) fclose(fd);
	else {
		fd = fopen(fn, "w");
		fclose(fd);
	}
	strcpy(fn, certdbpath);
	strcat(fn, ".dir");
	fd = fopen(fn, "r");
	if (fd) fclose(fd);
	else {
		fd = fopen(fn, "w");
		fclose(fd);
	}
	if (dbminit(certdbpath) < 0) return(-1);
	else dbmclose();
#endif


	/* crl dbm */

#ifdef NDBM
	crl = dbm_open(crldbpath, DBMOPENFL);
	if (!crl) return(-1);
	dbm_close(crl);
#else
	strcpy(fn, crldbpath);
	strcat(fn, ".pag");
	fd = fopen(fn, "r");
	if (fd) fclose(fd);
	else {
		fd = fopen(fn, "w");
		fclose(fd);
	}
	strcpy(fn, crldbpath);
	strcat(fn, ".dir");
	fd = fopen(fn, "r");
	if (fd) fclose(fd);
	else {
		fd = fopen(fn, "w");
		fclose(fd);
	}
	if (dbminit(crldbpath) < 0) return(-1);
	else dbmclose();
#endif


	return 0;
}


static int	
encrinit(ca, e_alg, cadir_abs, keysize, root)
DName  *ca;
AlgId  *e_alg;
char   *cadir_abs;
int    keysize;
Boolean root;
{
	Key	      nkey;
	KeyInfo	      nkinfo;
	PSESel	      psesk;
	Boolean       x500 = TRUE;
	AlgId	    * sig_alg;
	OctetString * serialnumber;
	Certificate * cert;
	int           rc;
#ifdef AFDBFILE
	char          afdb[256];
#endif
	char	    * proc = "encrinit";

	/*	generate encryption key */

	nkey.key = &nkinfo;
	nkey.keyref = 0;
	nkey.pse_sel = (PSESel * ) 0;
	nkinfo.subjectAI = e_alg;
        if(aux_ObjId2ParmType(e_alg->objid) != PARM_NULL)
	        *(int *)(nkinfo.subjectAI->param) = keysize;

        if(verbose) fprintf(stderr, "%s: Generating CA encryption key pair (algorithm %s)...\n", cmd, aux_ObjId2Name(e_alg->objid));

	if (af_gen_key(&nkey, ENCRYPTION, FALSE) < 0) {
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "unable to generate encryption key");
		aux_add_error(EINVALID, "unable to generate encryption key", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	psesk.app_name	  = 	AF_pse.app_name;
	psesk.pin	  = 	aux_cpy_String(AF_pse.pin);
	psesk.app_id	  = 	AF_pse.app_id;
	psesk.object.name = 	DecSKnew_name;
	psesk.object.pin  =     aux_cpy_String(AF_pse.pin);

	nkey.pse_sel = &psesk;

	if(aux_ObjId2AlgEnc(nkinfo.subjectAI->objid) == RSA) 
		sig_alg = aux_cpy_AlgId(md2WithRsaEncryption);
	if(aux_ObjId2AlgEnc(nkinfo.subjectAI->objid) == DSA) 
		sig_alg = aux_cpy_AlgId(dsaWithSHA);

	serialnumber = af_pse_incr_serial();
	if(!serialnumber){
		aux_add_error(EINVALID, "problems with serial number", CNULL, 0, proc);
		return(-1);
	}

	cert = af_create_Certificate(&nkinfo, sig_alg, DecSKnew_name, ca, serialnumber);
	aux_free_OctetString(&serialnumber);

	if (!cert) {
		fprintf(stderr, "%s: ",cmd);
                fprintf(stderr, "unable to create prototype certificate\n");
		aux_add_error(EINVALID, "unable to create prototype certificate", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	if(root){
		/* af_sign() shall only be performed by RootCA, whose ENCRYPTION and
                   SIGNATURE certificates shall both be verified with its secret SIGNATURE
	           key  
		*/
	
		/* tbs->valid with its 'notbefore' and 'notafter' fields has been allocated
	           by routine af_create_Certificate() above
		 */

		if(notbefore) {
			free(cert->tbs->valid->notbefore);
			cert->tbs->valid->notbefore = notbefore;
		}
		if(notafter) {
			free(cert->tbs->valid->notafter);
			cert->tbs->valid->notafter = notafter;
		}
	
		if(notbefore || notafter){
			/* ToBeSigned has been modified, so re-encode it */
			aux_free_OctetString(&cert->tbs_DERcode);
			cert->tbs_DERcode = e_ToBeSigned(cert->tbs);
			if(!cert->tbs_DERcode){
				aux_add_error(EENCODE, "e_ToBeSigned of ENCRYPTION prototype certificate FAILED", CNULL, 0, proc);
				LOGAFERR;
				return(-1);
			}
		}

		/* certificate finished, now sign again */
		   
		if (af_sign(cert->tbs_DERcode, cert->sig, SEC_END) < 0) {
			aux_add_error(ESIGNATURE, "Can't self-sign ENCRYPTION certificate", CNULL, 0, proc);
			LOGAFERR;
			return(-1);
		}
	}

	/* ... and store it on the PSE */
	if (af_pse_update_Certificate(ENCRYPTION, cert, TRUE) < 0) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "unable to store self-signed encryption certificate on PSE");
		aux_add_error(EINVALID, "unable to store self-signed encryption certificate on PSE", CNULL, 0, proc);
		LOGAFERR;
		return(-1);
	}

	/* ... and store it on the CA DB */
	af_cadb_add_Certificate(ENCRYPTION, cert , cadir_abs);


	/* Enter EncCert into Directory: */

	if (af_access_directory == TRUE) {
#ifdef AFDBFILE
		/* Determine whether X.500 directory shall be accessed */
		strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
		strcat(afdb, X500_name);           /* file = .af-db/'X500' */
		if (open(afdb, O_RDONLY) < 0) 
			x500 = FALSE;
#endif
#ifdef X500
		if ( x500 ) {
			if ( verbose ) {
				fprintf(stderr, "%s: Accessing the X.500 directory entry of ", cmd);
				fprintf(stderr, "owner = \"%s\" ...\n", caname);
			} 
			rc = af_dir_enter_Certificate(cert, cACertificate);
			if ( verbose ) {
				if ( rc < 0 )
					fprintf(stderr, "%s: Directory entry (X.500) failed.\n", cmd);
				else fprintf(stderr, "%s: Certificate entered into X.500 Directory.\n", cmd);
			      }
		}
#endif
#ifdef AFDBFILE
		if ( verbose ) {
			fprintf(stderr, "%s: Accessing the .af-db directory entry of ", cmd);
			fprintf(stderr, "owner = \"%s\" ...\n", caname);
		} 
		rc = af_afdb_enter_Certificate(cert, ENCRYPTION);
		if ( verbose ) {
			if ( rc < 0 )
				fprintf(stderr, "%s: Directory entry (.af-db) failed.\n", cmd);
			else fprintf(stderr, "%s: %s Certificate entered into .af-db Directory.\n", cmd, "ENCRYPTION");
		}

#endif
	}  /* if (af_access_directory) */



	
	return (0);
}


static int	
crlinit(cadir_abs, sig_alg, nextupdate)
char   * cadir_abs;
AlgId  * sig_alg;
char   * nextupdate;
{
	CRL                  * crl;
	CRLWithCertificates  	* crlwithcerts;
	Boolean                   x500 = TRUE;
	int			  rc;
	Crl			* crlpse;
#ifdef AFDBFILE
	char                      afdb[256];
#endif

	char	    * proc = "crlinit";


	crl = (CRL * )malloc(sizeof(CRL));
	if (! crl) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "Can't allocate memory\n");
		aux_add_error(EMALLOC, "crl", CNULL, 0, proc);
		return(-1);
	}

	crl->tbs = (CRLTBS * )malloc(sizeof(CRLTBS));
	if (! crl->tbs) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "Can't allocate memory\n");
		aux_add_error(EMALLOC, "crl->tbs", CNULL, 0, proc);
		return(-1);
	}

	if (!(crl->tbs->issuer = af_pse_get_Name())) {
		aux_add_error(EREADPSE, "af_pse_get_Name failed", CNULL, 0, proc);
		return(-1);
	}

	crl->tbs->lastUpdate = aux_current_UTCTime();
	if(nextupdate){ 
		if (aux_interval_UTCTime(CNULL, crl->tbs->lastUpdate, nextupdate)) {
			fprintf(stderr, "%s: ",cmd);
          		fprintf(stderr, "Validity interval of CRL incorrectly specified\n");
			aux_add_error(EVALIDITY, "aux_interval_UTCTime failed", CNULL, 0, proc);
			return(-1);
		}
		crl->tbs->nextUpdate = nextupdate;
	}
	else
		crl->tbs->nextUpdate = aux_delta_UTCTime(crl->tbs->lastUpdate);

	crl->tbs->revokedCertificates = (SEQUENCE_OF_CRLEntry * )0;

	crl->sig = (Signature * )malloc(sizeof(Signature));
	if (! crl->sig) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "Can't allocate memory\n");
		aux_add_error(EMALLOC, "crl->sig", CNULL, 0, proc);
		return(-1);
	}
	crl->sig->signature.nbits = 0;
	crl->sig->signature.bits = CNULL;

	crl->sig->signAI = af_pse_get_signAI();
	if ( ! crl->sig->signAI || aux_ObjId2AlgType(crl->sig->signAI->objid) == ASYM_ENC )
		crl->sig->signAI = aux_cpy_AlgId(sig_alg);

	crl->tbs->signatureAI = aux_cpy_AlgId(crl->sig->signAI);

	if ((crl->tbs_DERcode = e_CRLTBS(crl->tbs)) == NULLOCTETSTRING) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "e_CRLTBS failed\n");
		aux_add_error(EENCODE, "e_CRLTBS failed", CNULL, 0, proc);
		return(-1);
	}

	if (af_sign(crl->tbs_DERcode, crl->sig, SEC_END) < 0 ) {
		aux_add_error(ESIGN, "af_sign failed", CNULL, 0, proc);
		return(-1);
	}

	crlwithcerts = (CRLWithCertificates * )malloc(sizeof(CRLWithCertificates));
	if (! crlwithcerts) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "Can't allocate memory\n");
		aux_add_error(EMALLOC, "crlwithcerts", CNULL, 0, proc);
		return(-1);
	}

	crlwithcerts->crl = crl;
	crlwithcerts->certificates = (Certificates * )0;

	af_cadb_add_CRLWithCertificates(crlwithcerts, cadir_abs);

	
	/* Store own CRL in PSE object CrlSet */

	crlpse = CRL2Crl(crl);
	rc = af_pse_add_CRL(crlpse);
	if (rc != 0) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "Cannot update PSE object CrlSet\n");
		aux_add_error(EWRITEPSE, "af_pse_add_CRL failed", CNULL, 0, proc);
		aux_free_Crl(&crlpse);
		aux_free_CRLWithCertificates(&crlwithcerts);
		return(-1);
	}
	aux_free_Crl(&crlpse);

	/* Enter CRL into Directory: */

	if (af_access_directory == TRUE) {
#ifdef AFDBFILE
		/* Determine whether X.500 directory shall be accessed */
		strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
		strcat(afdb, X500_name);           /* file = .af-db/'X500' */
		if (open(afdb, O_RDONLY) < 0) 
			x500 = FALSE;
#endif
#ifdef X500
		if ( x500 ) {
			if ( verbose ) {
				fprintf(stderr, "%s: Accessing the X.500 directory entry of ", cmd);
				fprintf(stderr, "owner = \"%s\" ...\n", caname);
			} 
			rc = af_dir_enter_CRL(crl);
			if ( verbose ) {
				if ( rc < 0 )
					fprintf(stderr, "%s: Directory entry (X.500) failed.\n", cmd);
				else fprintf(stderr, "%s: CRL entered into X.500 Directory.\n", cmd);
			}
		}
#endif
#ifdef AFDBFILE
		if ( verbose ) {
			fprintf(stderr, "%s: Accessing the .af-db directory entry of ", cmd);
			fprintf(stderr, "owner = \"%s\" ...\n", caname);
		} 
		rc = af_afdb_enter_CRL(crl);
		if ( verbose ) {
			if ( rc < 0 )
				fprintf(stderr, "%s: Directory entry (.af-db) failed.\n", cmd);
			else fprintf(stderr, "%s: CRL entered into .af-db Directory.\n", cmd);
		}
	
#endif
	}  /* if (af_access_directory) */

	aux_free_CRLWithCertificates(&crlwithcerts);


	return(0);
}




static
void usage(help)
int     help;
{

	aux_fprint_version(stderr);

        fprintf(stderr, "cacreate: Create CA (CA command)\n\n\n");
	fprintf(stderr, "Description:\n\n"); 
	fprintf(stderr, "'cacreate' creates a CA directory containing the CA PSE with one or two\n");
        fprintf(stderr, "asymmetric keypairs,  self-signed prototype certificate(s), an empty\n");
        fprintf(stderr, "revocation list, and the SerialNumber on it, and the CA database.\n\n\n");

        fprintf(stderr, "usage:\n\n");
#ifdef X500
	fprintf(stderr, "cacreate [-hrqvzDVWXY] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-s <signalg>] [-k <keysize>]\n");
	fprintf(stderr, "         [-e <encalg>] [-k <keysize>] [-n <serial>] [-u <nextupdate>]\n");
	fprintf(stderr, "         [-f <notbefore>] [-l <notafter>] [-d <dsa name>] [-t <dsaptailor>] [-A <authlevel>] [CA-Name]\n\n");
#else
	fprintf(stderr, "cacreate [-hrqvzDVWXY] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-s <signalg>] [-k <keysize>]\n");
	fprintf(stderr, "         [-e <encalg>] [-k <keysize>] [-n <serial>] [-u <nextupdate>]\n");
	fprintf(stderr, "         [-f <notbefore>] [-l <notafter>] [CA-Name]\n\n");
#endif


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <psename>       PSE name (default: environment variable CAPSE or .capse)\n");
        fprintf(stderr, "-c <cadir>         Name of CA-directory (default: environment variable CADIR or .ca)\n");
	fprintf(stderr, "-a <issueralg>     Issuer algorithm associated with the signature of the prototype certificate(s)\n");
	fprintf(stderr, "                   (default: md2WithRsaEncryption)\n");
	fprintf(stderr, "-s <signalg>       Signature algorithm (default: rsa)\n");
	fprintf(stderr, "-k <keysize>       Keysize of RSA signature key\n");
	fprintf(stderr, "-e <encalg>        Encryption algorithm (default: rsa)\n");
	fprintf(stderr, "-k <keysize>       Keysize of RSA encryption key\n");
	fprintf(stderr, "-n <serial>        Initial value of the serial number to be used by the CA. <serial> must be a\n");
	fprintf(stderr, "                   string from [0..9,A..F] representing the hexadecimal notation of the number.\n");
	fprintf(stderr, "-r                 create Root-CA, which means that if the generated PSE holds two keypairs,\n");
	fprintf(stderr, "                   the corresponding certificates are both signed by the signature key\n");
	fprintf(stderr, "                   (default: the encryption certificate is signed by the secret decryption key)\n");
	fprintf(stderr, "-D                 store self-signed certificate(s) in Directory (X.500 or .af-db)\n");
	fprintf(stderr, "-u <nextupdate>    Time and date of next scheduled CRL update (default: current date + one year)\n");
	fprintf(stderr, "-f <notbefore>     First date on which self-signed certificate is valid (default: current date).\n");
	fprintf(stderr, "                   (is only evaluated if option -r was supplied)\n");
	fprintf(stderr, "-l <notafter>      Last date on which self-signed certificate is valid (default: notbefore + one year)\n");
	fprintf(stderr, "                   (is only evaluated if option -r was supplied)\n");
	fprintf(stderr, "-q                 create PSE that contains two RSA keypairs (default: one RSA keypair only)\n");
	fprintf(stderr, "-t                 control malloc/free behaviour\n");
        fprintf(stderr, "-h                 write this help text\n");
        fprintf(stderr, "-v                 verbose\n");
        fprintf(stderr, "-V                 Verbose\n");
        fprintf(stderr, "-W                 Grand Verbose (for testing only)\n");
        fprintf(stderr, "-X                 Read random number generator seed from PSE-object Random\n");
        fprintf(stderr, "-Y                 Init random number generator seed through keyboard input\n");
#ifdef X500
	fprintf(stderr, "-d <dsa name>      Name of the DSA to be initially accessed (default: locally configured DSA)\n");
	fprintf(stderr, "-t <dsaptailor>    Location of dsaptailor file (default: dsaptailor in the ISODE ETCDIR directory)\n");
	fprintf(stderr, "-A <authlevel>     Level of authentication used for X.500 Directory access\n");
	fprintf(stderr, "                   <authlevel> may have one of the values 'SIMPLE' or 'STRONG'\n");
	fprintf(stderr, "                   (default: environment variable AUTHLEVEL or 'No authentication')\n");
	fprintf(stderr, "                   STRONG implies the use of signed DAP operations\n");
#endif
	fprintf(stderr, "<CA-Name>          Intended owner of the generated CA PSE\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM CACREATE */
}
