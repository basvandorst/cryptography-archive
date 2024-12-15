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
#include "af.h"
#include "cadb.h"

char * getenv();
static OctetString *getserial();

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
	int			  i;
	int 			  found;
	OctetString	        * serial;
	char			  buf[256];
	char			  alias[161], * name, * issuer;
	char	        	  calogfile[256];
	char		        * nextupdate = CNULL;
	DName 		        * dname;
	Certificate		* cert;
	Certificates            * certs;
	Boolean      	          update;
	AlgId                   * algorithm = (AlgId * )0;
	AlgId                   * keyalgid;
	char	      		* psename = CNULL, * cadir = CNULL, * xx;
	char	       		* cmd = * parm, opt, * pin;
	extern char		* optarg;
	extern int		  optind, opterr;
	char           		  interactive = FALSE;
	ObjId                   * oid;
	RC                        rc;
	UTCTime 		* lastUpdate, * nextUpdate;
	CRLEntry     		* crlentry;
	CRLWithCertificates		* crlwithcerts;
	SEQUENCE_OF_CRLEntry  * crlentryseq;
	Crl	                * crlpse;
	FILE		        * ff;
	char		          x500 = TRUE;
#ifdef AFDBFILE
	char		          afdb[256];
#endif
#ifdef X500
	char	                * env_af_dir_authlevel;
#endif
	char		        * proc = "main (revoke)";

	ff = fopen("/dev/tty", "r");

	optind = 1;
	opterr = 0;

	logfile = (FILE * )0;

	af_access_directory = FALSE;
	MF_check = FALSE;

#ifdef X500
	while ( (opt = getopt(cnt, parm, "a:c:p:d:t:A:u:hvzVWD")) != -1 ) {
#else
	while ( (opt = getopt(cnt, parm, "a:c:p:u:hvzVWD")) != -1 ) {
#endif
		switch (opt) {
		case 'a':
			if (algorithm) 
				usage(SHORT_HELP);
			else {
                        	oid = aux_Name2ObjId(optarg);
                        	if (aux_ObjId2AlgType(oid) != SIG) usage(SHORT_HELP);
				algorithm = aux_ObjId2AlgId(oid);
			}
			break;
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
			break;
                case 'z':
                        MF_check = TRUE;
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
		case 'u':
			if (nextupdate) usage(SHORT_HELP);
			else nextupdate = optarg;
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

	if (optind < cnt) 
		strcpy(alias, parm[optind++]);

	if (optind < cnt) 
		usage(SHORT_HELP);

        if(!cadir) cadir = getenv("CADIR");
        if(!cadir) cadir = DEF_CADIR;
	
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

	dname = af_pse_get_Name();
	name = aux_DName2Name(dname);
	aux_free_DName(&dname);
	crlwithcerts = af_cadb_get_CRLWithCertificates(name, cadir);
	if(! crlwithcerts || ! crlwithcerts->crl){
		fprintf(stderr, "%s: WARNING: Your own CRL is NOT stored in your local database!\n", cmd);
		exit(-1);
	}
	fprintf(stderr, "\nThis is your locally stored revocation list:\n\n");
	aux_fprint_CRL(stderr, crlwithcerts->crl);
	fprintf(stderr, "\n\nVerifying your locally stored CRL ...\n\n");
	certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME); 
	rc = af_verify(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END, certs, (UTCTime * )0, (PKRoot * )0);
	aux_fprint_VerificationResult(stderr, verifresult);
	aux_free_VerificationResult(&verifresult);
	if (rc == 0)
		fprintf(stderr, "%s: Verification of locally stored CRL s u c c e e d e d!\n\n", cmd);
	else {
		fprintf(stderr, "%s: WARNING: Verification of locally stored CRL f a i l e d!\n", cmd);
		exit(-1);
	}
	
	xx = "y";
	update = 0;
	while (strcmp(xx, "n")) {
		free (xx);
		xx = CNULL;
		fprintf(stderr, "\nEnter serial number of certificate which is to be revoked:\n");
		serial = getserial();
		i = 0;
		while (!serial && i < 3) {
			fprintf(stderr, "Serial number must be a positive integer!\n");
			serial = getserial();
			i++;
		}
		if (i == 3) exit (-1);
		cert = af_cadb_get_Certificate(serial, cadir);
		if (!cert) {
			fprintf(stderr, "\n");
			fprintf(stderr, "No certificate with serial number ");
			aux_fprint_Serialnumber(ff, serial);
			fprintf(stderr, "\n");
			fprintf(stderr, " in CA database!\n\n");
			fprintf(stderr, "New choice? [y/n]: ");
		}
		else {	
			crlentry = af_create_CRLEntry(serial);
			if (!af_search_CRLEntry(crlwithcerts->crl, crlentry)) { 
				fprintf(stderr, "\nThe following certificate with serial number ");
				aux_fprint_Serialnumber(ff, serial);
				fprintf(stderr, "\n");
				fprintf(stderr, "is being revoked:\n\n");
				aux_fprint_Certificate(stderr, cert);
				crlentryseq = (SEQUENCE_OF_CRLEntry * )malloc(sizeof(SEQUENCE_OF_CRLEntry));
				if (!crlentryseq) {
					fprintf(stderr, "Can't allocate memory");
					exit (-1);
				}

				crlentryseq->element = aux_cpy_CRLEntry(crlentry);
				aux_free_CRLEntry(&crlentry);

				crlentryseq->next = crlwithcerts->crl->tbs->revokedCertificates;  
				/* existing or NULL pointer */
				crlwithcerts->crl->tbs->revokedCertificates = crlentryseq;
				update = 1;
				fprintf(stderr, "\nMore certificates to be revoked? [y/n]: ");
			}
			else {
				fprintf(stderr, "\n");
				fprintf(stderr, "Certificate with serial number ");
				aux_fprint_Serialnumber(ff, serial);
				fprintf(stderr, "\n");
				fprintf(stderr, "already revoked !\n\n");
				fprintf(stderr, "New choice? [y/n]: ");
			}
		}
		gets(buf);
		xx = buf;
		while ( strcmp(xx, "y") && strcmp(xx, "n") ) {
			fprintf(stderr, "\nAnswer must be 'y' or 'n' !\n\n");
			fprintf(stderr, "\nNew choice? [y/n]: ");
			gets(buf);
			xx = buf;
		}
	}  /*while*/
		
	if (!update) {
		fprintf(stderr, "\nNo update done on revocation list!\n");
		aux_free_CRLWithCertificates(&crlwithcerts);
		exit (-1);
	}

	crlwithcerts->crl->tbs->lastUpdate = aux_current_UTCTime();
	if(nextupdate){ 
		if (aux_interval_UTCTime(CNULL, crlwithcerts->crl->tbs->lastUpdate, nextupdate)) {
			fprintf(stderr, "%s: ",cmd);
          		fprintf(stderr, "Validity interval of CRL incorrectly specified\n");
			exit(-1);
		}
		crlwithcerts->crl->tbs->nextUpdate = aux_cpy_String(nextupdate);
	}
	else
		crlwithcerts->crl->tbs->nextUpdate = get_nextUpdate(crlwithcerts->crl->tbs->lastUpdate);

	crlwithcerts->crl->sig = (Signature * )calloc(1, sizeof(Signature));
	if (! crlwithcerts->crl->sig) {
		fprintf(stderr, "%s: ",cmd);
  	        fprintf(stderr, "Can't allocate memory\n");
		exit(-1);
	}
	crlwithcerts->crl->sig->signature.nbits = 0;
	crlwithcerts->crl->sig->signature.bits = CNULL;

	keyalgid = af_pse_get_signAI();
	if(keyalgid && algorithm) {
		if(aux_ObjId2AlgEnc(algorithm->objid) != aux_ObjId2AlgEnc(keyalgid->objid)) {
			fprintf(stderr, "%s: ",cmd);
			fprintf(stderr, "sig_alg does not fit to key\n");
			exit(-1);
		}
	}
	if(!algorithm) {
		if(keyalgid) switch(aux_ObjId2AlgEnc(keyalgid->objid)) {
			case RSA:
				crlwithcerts->crl->sig->signAI = aux_cpy_AlgId(md2WithRsaEncryption);
				break;
			case DSA:
				crlwithcerts->crl->sig->signAI = aux_cpy_AlgId(dsaWithSHA);
				break;
		}
		else {
			fprintf(stderr, "%s: ",cmd);
 			fprintf(stderr, "can't determine sig alg\n");
			exit(-1);
		}
	}
	else
		crlwithcerts->crl->sig->signAI = aux_cpy_AlgId(algorithm);

	crlwithcerts->crl->tbs->signatureAI = aux_cpy_AlgId(crlwithcerts->crl->sig->signAI);

	if ((crlwithcerts->crl->tbs_DERcode = e_CRLTBS(crlwithcerts->crl->tbs)) == NULLOCTETSTRING) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Can't encode crlwithcerts->crl->tbs\n");
		exit (-1);
	}

	fprintf(stderr, "\nThe following CRL is to be signed. ");
	fprintf(stderr, "Please check it:\n\n");
	aux_fprint_CRLTBS(stderr, crlwithcerts->crl->tbs);
	fprintf(stderr, "\nDo you want to sign the displayed revocation list CRL ?\n");
	fprintf(stderr, "If you want to sign it, (re)enter the PIN of your chipcard:\n\n");

	af_pse_close(NULLOBJID);

	if ( af_sign(crlwithcerts->crl->tbs_DERcode, crlwithcerts->crl->sig, SEC_END) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Signature of revocation list failed\n");
		exit (-1);
	}


	/* Update on Directory entry, PSE, and CA directory: */

	fprintf(stderr, "\n**********************************************\n");

	/* update directory entry */
#ifdef AFDBFILE
	/* Determine whether X.500 directory shall be accessed */
	strcpy(afdb, AFDBFILE);         /* file = .af-db/ */
	strcat(afdb, X500_name);           /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) 
		x500 = FALSE;
#endif
#ifdef X500
	if (x500 && af_access_directory == TRUE) {
		directory_user_dname = af_pse_get_Name();
		if ( verbose )
			fprintf(stderr, "\nTrying to update your X.500 directory entry ...");
		rc = af_dir_enter_CRL(crlwithcerts->crl);
		if ( verbose ) {
			if ( rc < 0 ) 
				fprintf(stderr, "\n Directory entry (X.500) f a i l e d !\n");
			else fprintf(stderr, "\n Done!\n");
			fprintf(stderr, "\n**********************************************\n");
		}
	}
#endif
#ifdef AFDBFILE
	if (af_access_directory == TRUE) {
		if ( verbose )
			fprintf(stderr, "\nTrying to update your .af-db directory entry ...");
		rc = af_afdb_enter_CRL(crlwithcerts->crl);
		if ( verbose ) {
			if ( rc < 0 ) 
				fprintf(stderr, "\n Directory entry (.af-db) f a i l e d !\n");
			else fprintf(stderr, "\n Done!\n");
			fprintf(stderr, "\n**********************************************\n");
		}
	}
#endif

	/* update PSE object CrlSet, even if the directory entry failed */
	crlpse = CRL2Crl (crlwithcerts->crl);
	fprintf(stderr, "\nUpdating PSE object CrlSet ...\n");
	rc = af_pse_add_CRL(crlpse);
	if (rc != 0) {
		fprintf(stderr, "\n Cannot update PSE object CrlSet.\n");
		aux_free_Crl (&crlpse);
	}
	else fprintf(stderr, "\n Done!\n");
	aux_free_Crl (&crlpse);
	fprintf(stderr, "\n**********************************************\n");

	/* update crlwithcerts database in CA directory, even if the directory entry failed */
	fprintf(stderr, "\nUpdating 'crlwithcerts' database in CA directory \"%s\" ...\n", cadir);

	if(*cadir != PATH_SEPARATION_CHAR) {
		strcpy(calogfile, getenv("HOME"));
		strcat(calogfile, PATH_SEPARATION_STRING);
		strcat(calogfile, cadir);
	}
	else strcpy(calogfile, cadir);
	strcat(calogfile, PATH_SEPARATION_STRING);
	strcat(calogfile, "calog");
	logfile = fopen(calogfile, LOGFLAGS);
	if(logfile == (FILE * ) 0) {
		fprintf(stderr, "%s: Can't open %s\n", cmd, CALOG);
		exit(-1);
	}
	rc = af_cadb_add_CRLWithCertificates(crlwithcerts, cadir);
	if(rc != 0){
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot store your updated CRL in your 'crlwithcerts' database!\n");
		exit(-1);
	}
	fclose(logfile);
	logfile = (FILE * )0;
	fprintf(stderr, "\nMost current version of CRL stored in 'crlwithcerts' database in ");
	fprintf(stderr, "CA directory \"%s\".\n\n", cadir);

	exit(0);
}



static
OctetString * getserial() {
	char        *dd, number[1200];
	OctetString *ret;
	RC	    rc;
	char        *proc = "getserial";

	fprintf(stderr, " Serial number: ");
	dd = aux_cpy_String(gets(number));
 	if(!dd) return(NULLOCTETSTRING);
	if (strlen(dd) == 0) {
		free(dd);
		return(NULLOCTETSTRING);
	}

	ret = aux_create_SerialNo(dd);
	free(dd);

        return(ret);
}



static
void usage(help)
int     help;
{
	aux_fprint_version(stderr);
        fprintf(stderr, "revoke: Revoke one or more Certificates (CA command)\n\n\n");
        fprintf(stderr, "usage:\n\n");
#ifdef X500
        fprintf(stderr,"revoke [-hvzVWD] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-u <nextupdate>] [-d <dsa name>] [-t <dsaptailor>] [-A <authlevel>]\n\n"); 
#else
	fprintf(stderr,"revoke [-hvzVWD] [-p <pse>] [-c <cadir>] [-a <issueralg>] [-u <nextupdate>]\n\n");
#endif

        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <psename>     PSE name (default: .capse)\n");
        fprintf(stderr, "-c <cadir>       name of CA-directory (default: .ca)\n");
	fprintf(stderr, "-a <issueralg>   Issuer algorithm associated with the signature of the Crl\n");
	fprintf(stderr, "-u <nextupdate>    Time and date of next scheduled update of revocation list\n");
	fprintf(stderr, "-z               enable memory checking\n");
        fprintf(stderr, "-h               write this help text\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
	fprintf(stderr, "-D               store updated revocation list in Directory (X.500 or .af-db)\n");
#ifdef X500
	fprintf(stderr, "-d <dsa name>    Name of the DSA to be initially accessed (default: locally configured DSA)\n");
	fprintf(stderr, "-t <dsaptailor>  Location of dsaptailor file (default: dsaptailor in the ISODE ETCDIR directory)\n");
	fprintf(stderr, "-A <authlevel>   level of authentication used for binding to the X.500 Directory\n");
#endif
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM REVOKE */
}
