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
 *   	program to install given PKRoot on PSE
 */

#include <stdio.h>
#include "cadb.h"

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
        PSESel	        pse;
        PKRoot          *root;
        char            *filename = CNULL;
        int             i;
        OctetString     *ostr;
	extern char	*optarg;
	extern int	optind, opterr;
	char	        *cmd = *parm, opt, *pin;
	char	        *psename = CNULL, *cadir = CNULL;

	char 		*proc = "main (instpkroot)";

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "c:p:hvzVW")) != -1 ) {
		switch (opt) {
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
		filename = parm[optind++];

	if (optind < cnt) 
		usage(SHORT_HELP);

	aux_set_pse(psename, cadir);

	if (!(ostr = aux_file2OctetString(filename))) {
		fprintf(stderr,"%s: Can't read %s\n", cmd, filename);
		aux_add_error(EINVALID, "Can't read", filename, char_n, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	if (!(root = d_PKRoot(ostr))) {
		fprintf(stderr,"%s: Can't decode PKRoot\n", cmd);
		aux_add_error(EDECODE, "d_PKRoot failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	if (! cadir){	/* user PSE */
		/* Remove signature from 'root', as it is not needed by a user */
		if(root->newkey){
			if(root->newkey->sig) {
				aux_free_Signature(&(root->newkey->sig));
				root->newkey->sig = (Signature * )0;
			}
		}
	}

	if (af_pse_update_PKRoot(root) < 0) {
		fprintf(stderr,"%s: Can't install PKRoot on PSE %s\n", cmd, AF_pse.app_name);
		aux_add_error(EINVALID, "af_pse_update_PKRoot failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	if(verbose) fprintf(stderr,"%s: PKRoot installed in PSE %s\n", cmd, AF_pse.app_name);

	return(0);
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

        fprintf(stderr, "instpkroot: Install Public Root Information on PSE\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'instpkroot' reads file <pkroot> or stdin, if <pkroot> is omitted,\n");
	fprintf(stderr, "and installs its content as PSE object PKRoot on the indicated PSE.\n");
	fprintf(stderr, "A PKRoot information that already exists on the target PSE will be overwritten.\n\n\n");

        fprintf(stderr, "usage:\n\n");
	fprintf(stderr,"instpkroot [-hvzVW] [-p <pse>] [-c <cadir>] [pkroot]\n\n");
 

        if(help == LONG_HELP) {
        	fprintf(stderr, "with:\n\n");
        	fprintf(stderr, "-p <psename>     PSE name (default: environment variable PSE or %s)\n", DEF_PSE);
        	fprintf(stderr, "-c <cadir>       Name of CA-directory (default: environment variable CADIR or %s)\n", DEF_CADIR);
		fprintf(stderr, "-z               control malloc/free behaviour\n");
        	fprintf(stderr, "-h               write this help text\n");
        	fprintf(stderr, "-v               verbose\n");
        	fprintf(stderr, "-V               Verbose\n");
        	fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
		fprintf(stderr, "<pkroot>         File containing public root information (or stdin, if omitted)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM INSTPKROOT */
}
