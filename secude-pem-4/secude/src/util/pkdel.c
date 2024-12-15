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

char *getenv();

#ifdef __STDC__
static int	incorrectName	(char *proc, char verbose);
#else
static int	incorrectName();
#endif

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
	int 		i;
	RC	        rc;
	OctetString     *serialnumber = NULLOCTETSTRING;
	OctetString     *tmp_ostr1 = NULLOCTETSTRING, *tmp_ostr2 = NULLOCTETSTRING;
	char		listtype[7];
	KeyType 	type;
	char		*issuer = CNULL, *subject = CNULL;
	DName 	        *issuer_dn, *subject_dn;
	PKList          *pklist;
	char	        *psename = CNULL, *cadir = CNULL;
	char	        *cmd = *parm, opt, *pin;
	extern char	*optarg;
	extern int	optind, opterr;
	char		*proc = "main (pkdel)";

	strcpy(listtype, "PKList");
	type = SIGNATURE;

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "c:p:o:i:n:ehvzVW")) != -1 ) {
		switch (opt) {
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
			break;
		case 'o':
			if (subject) usage(SHORT_HELP);
			else subject = optarg;
			break;
		case 'i':
			if (issuer) usage(SHORT_HELP);
			else issuer = optarg;
			break;
		case 'n':
			if (serialnumber) usage(SHORT_HELP);
			else serialnumber = aux_create_SerialNo(optarg);
			break;
		case 'e':
			strcpy(listtype, "EKList");
			type = ENCRYPTION;
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

	if(optind<cnt) usage(SHORT_HELP);

	aux_set_pse(psename, cadir);


	if (subject) {
		subject_dn = aux_alias2DName(subject);
		if (!subject_dn) incorrectName(proc, verbose);
	}
	else {
		if (!serialnumber) {
			fprintf(stderr, "%s: ",cmd);
			fprintf(stderr, "Serialnumber missing\n");
			aux_add_error(EINVALID, "serialnumber missing", CNULL, 0, proc);
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "\n");
			usage (SHORT_HELP);
		}
		subject_dn = NULLDNAME;
		if (issuer) {
			issuer_dn = aux_alias2DName(issuer);
			if (!issuer_dn) incorrectName(proc, verbose);
		}
		else {
			fprintf(stderr, "%s: ",cmd);
			fprintf(stderr, "issuer missing\n");
			aux_add_error(EINVALID, "issuer missing", CNULL, 0, proc);
			aux_fprint_error(stderr, verbose);
			fprintf(stderr, "\n");
			usage (SHORT_HELP);
		}
	}

	if ( af_pse_delete_PK(type, subject_dn, issuer_dn, serialnumber) < 0 ) {
		if (err_stack && (err_stack->e_number == EOBJNAME)) {
		        fprintf(stderr, "\nThere is no ToBeSigned with\n");
			if (issuer && serialnumber) {
		        	fprintf(stderr, " issuer \"%s\" and\n", issuer);
		       		fprintf(stderr, " SerialNo ");
				aux_fprint_Serialnumber(stderr, serialnumber);
			}
			else
			        fprintf(stderr, " owner \"%s\"\n", subject);

			fprintf(stderr, "stored in your %s. No update done!\n", listtype);
		}
	}
	else {
		fprintf(stderr, "\nToBeSigned with\n");
		if (issuer && serialnumber) {
		        fprintf(stderr, " issuer \"%s\" and\n", issuer);
		       	fprintf(stderr, " SerialNo ");
			aux_fprint_Serialnumber(stderr, serialnumber);
		}
		else
			fprintf(stderr, " owner \"%s\"\n", subject);

		fprintf(stderr, "removed from your %s.\n", listtype);

		fprintf(stderr, "\nYour updated %s now looks like this:\n\n", listtype);
		pklist = af_pse_get_PKList(type);
		if ( !pklist )
			fprintf(stderr, "Your %s is EMPTY!\n", listtype);
		else {
			fprintf(stderr, " ****************** %s ******************\n", listtype);
			aux_fprint_PKList (stderr, pklist);
		}
	}


	exit(0);
}


/***************************************************************
 *
 * Procedure incorrectName
 *
 ***************************************************************/
#ifdef __STDC__

static int incorrectName(
	char	 *proc,
	char	  verbose
)

#else

static int incorrectName(
	proc,
	verbose
)
char	 *proc;
char	  verbose;

#endif

{
	fprintf(stderr, "Name cannot be transformed into DName-structure.\n");
	aux_add_error(EINVALID, "Name cannot be transformed into DName-structure", CNULL, 0, proc);
	aux_fprint_error(stderr, verbose);
	exit(1);
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

        fprintf(stderr, "pkdel  Remove Public Key from Cache\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'pkdel' deletes entries from the cache of trusted public keys (PKList or\n");
	fprintf(stderr, "EKList). It either deletes all entries of the given <owner>, or the one entry\n");
	fprintf(stderr, "that is uniquely identified by its <issuer> and <serial> combination.\n\n\n");

        fprintf(stderr, "usage:\n\n");
	fprintf(stderr, "pkdel [-ehvzVW] [-p <pse>] [-c <cadir>] [-o <owner>] [-i <issuer>] [-n <serial>]\n\n");
 

        if(help == LONG_HELP) {
        	fprintf(stderr, "with:\n\n");
        	fprintf(stderr, "-p <psename>     PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
        	fprintf(stderr, "-c <cadir>       Name of CA-directory (default: Environment variable CADIR or %s)\n", DEF_CADIR);
		fprintf(stderr, "-o <owner>       Owner of public key\n");
		fprintf(stderr, "-i <issuer>      Issuer of public key\n");
		fprintf(stderr, "-n <serial>      Serial number of public key\n");
		fprintf(stderr, "-e               remove public key from cache of public ENCRYPTION keys (EKList)\n");
		fprintf(stderr, "-z               control malloc/free behaviour\n");
        	fprintf(stderr, "-h               write this help text\n");
        	fprintf(stderr, "-v               verbose\n");
        	fprintf(stderr, "-V               Verbose\n");
        	fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM INSTPKROOT */
}
