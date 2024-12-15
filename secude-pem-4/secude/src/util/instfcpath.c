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
 *   	program to install given FCPath on PSE
 */

#include <stdio.h>
#include "cadb.h"

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
        FCPath          * path, * reduced_path = (FCPath * )0;
	PKRoot		* pkroot;
	Certificates    * certs;
        char            * filename = CNULL;
        int               i, rcode;
        OctetString     * ostr;
	extern char	* optarg;
	extern int	  optind, opterr;
	char	        * cmd = *parm, opt, *pin;
	char	        * psename = CNULL, *cadir = CNULL;

	char		* proc = "main (instfcpath)";


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

        if(ostr->noctets == 0) {
		fprintf(stderr,"%s: Warning: FCPath empty. No FCPath installed in PSE %s\n", cmd, AF_pse.app_name);
		exit(0);
	}

	if (!(path = d_FCPath(ostr))) {
		fprintf(stderr,"%s: Can't decode FCPath\n", cmd);
		aux_add_error(EDECODE, "d_FCPath failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	pkroot = af_pse_get_PKRoot();
	if (! pkroot) {
		fprintf(stderr, "%s: There is no public RootCA key stored on your PSE.\n", cmd);
		fprintf(stderr, "%s: Public RootCA key is required for verification of FCPath\n", cmd);
		aux_add_error(EINVALID, "af_pse_get_PKRoot failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	reduced_path = reduce_FCPath_to_HierarchyPath(path);
	if(! reduced_path){
		fprintf(stderr, "%s: FCPath does not contain a path of hierarchy certificates to own RootCA\n", cmd);
		aux_add_error(EINVALID, "reduce_FCPath_to_HierarchyPath failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	certs = transform_reducedFCPath_into_Certificates(reduced_path);
	if(! certs){
		fprintf(stderr, "%s: Cannot transform reduced FCPath into Certificates structure\n", cmd);
		aux_add_error(EINVALID, "transform_reducedFCPath_into_Certificates failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	rcode = af_verify_Certificates(certs, (UTCTime *)0, pkroot);
	if(verbose) aux_fprint_VerificationResult(stderr, verifresult);
	if(rcode < 0){
		fprintf(stderr, "%s: Cannot verify FCPath up to own RootCA\n", cmd);
		aux_add_error(EINVALID, "af_verify_Certificates failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	/* FCPath 'path' contains a path of hierarchy certificates that terminates at own
	   RootCA and that can be verified, so install it on PSE ...
	 */

	if(af_pse_update_FCPath(path) < 0) {
		fprintf(stderr, "%s: Can't install FCPath on PSE %s\n", cmd, AF_pse.app_name);
		aux_add_error(EINVALID, "af_pse_update_FCPath failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	if(verbose) fprintf(stderr,"%s: FCPath installed in PSE %s\n", cmd, AF_pse.app_name);

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

        fprintf(stderr, "instfcpath: Install Forward Certification Path on PSE\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'instfcpath' reads file <fcpath> or stdin, if <fcpath> is omitted,\n");
	fprintf(stderr, "and installs its content as PSE object FCPath on the indicated PSE.\n");
	fprintf(stderr, "A FCPath information that already exists on the target PSE will be overwritten.\n\n\n");

        fprintf(stderr, "usage:\n\n");
	fprintf(stderr,"instfcpath [-hvzVW] [-p <pse>] [-c <cadir>] [fcpath]\n\n");
 

        if(help == LONG_HELP) {
        	fprintf(stderr, "with:\n\n");
        	fprintf(stderr, "-p <psename>     PSE name (default: environment variable PSE or %s)\n", DEF_PSE);
        	fprintf(stderr, "-c <cadir>       Name of CA-directory (default: environment variable CADIR or %s)\n", DEF_CADIR);
		fprintf(stderr, "-z               control malloc/free behaviour\n");
        	fprintf(stderr, "-h               write this help text\n");
        	fprintf(stderr, "-v               verbose\n");
        	fprintf(stderr, "-V               Verbose\n");
        	fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
		fprintf(stderr, "<fcpath>         File containing FCPath (or stdin, if omitted)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM INSTFCPATH */
}
