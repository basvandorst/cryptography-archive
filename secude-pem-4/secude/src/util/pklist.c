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
	extern char	*optarg;
	extern int	optind, opterr;
	char	        *cmd = *parm, opt, *pin;
	char	        *psename = CNULL, *cadir = CNULL;
	int	        i;
	char	        listtype[7];
	KeyType         type;
	PKList          *list;
	char		*proc = "main (pklist)";

	strcpy(listtype, "PKList");
	type = SIGNATURE;

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "c:p:ehvzVW")) != -1 ) {
		switch (opt) {
		case 'c':
			if (cadir) usage(SHORT_HELP);
			else cadir = optarg;
			break;
		case 'p':
			if (psename) usage(SHORT_HELP);
			else psename = optarg;
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

	aux_set_pse(psename, cadir);

	if ( (list = af_pse_get_PKList (type)) == (PKList * )0 ) {
		aux_add_error(EINVALID, "af_pse_get_PKList failed", CNULL, 0, proc);
		aux_fprint_error(stderr, verbose);
		fprintf(stderr, "No %s\n", listtype);
		exit(-1);
	}

	fprintf(stderr, " ****************** %s ******************\n", listtype);
	aux_fprint_PKList(stderr, list);
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

        fprintf(stderr, "pklist: Print Cache of Trusted Public Keys\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'pklist' prints out the content of the cache of trusted public keys\n");
	fprintf(stderr, "(PKList or EKList) of the indicated PSE.\n\n\n");

        fprintf(stderr, "usage:\n\n");
	fprintf(stderr, "pklist [-ehvzVW] [-p <pse>] [-c <cadir>]\n\n");
 

        if(help == LONG_HELP) {
        	fprintf(stderr, "with:\n\n");
        	fprintf(stderr, "-p <psename>     PSE name (default: environment variable PSE or %s)\n", DEF_PSE);
        	fprintf(stderr, "-c <cadir>       Name of CA-directory (default: environment variable CADIR or %s)\n", DEF_CADIR);
		fprintf(stderr, "-e               print cache of trusted public ENCRYPTION keys (EKList)\n");
		fprintf(stderr, "                 (default: PKList)\n");
		fprintf(stderr, "-z               control malloc/free behaviour\n");
        	fprintf(stderr, "-h               write this help text\n");
        	fprintf(stderr, "-v               verbose\n");
        	fprintf(stderr, "-V               Verbose\n");
        	fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM PKLIST */
}
