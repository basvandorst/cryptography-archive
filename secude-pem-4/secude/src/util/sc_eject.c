#include <stdio.h>
#include "secure.h"

#ifdef SCA

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
	int		sct_id = 1;

	char 		*proc = "main (sc_eject)";


	optind = 1;
	opterr = 0;


	while ( (opt = getopt(cnt, parm, "s:tvVh")) != -1 ) {
		switch (opt) {
		case 's':
			sct_id = atoi(optarg);
			if (sct_id < 0) 
			   usage(SHORT_HELP);
			break;
                case 't':
                        MF_check = TRUE;
                        break;
		case 'v':
			verbose = 1;
			continue;
		case 'V':
			verbose = 2;
			continue;
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}


	sc_sel.sct_id = sct_id;

	if ((sec_sc_eject(CURRENT_SCT)) == -1) {
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

        fprintf(stderr, "sc_eject: Eject smartcard\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'sc_eject' can be used to eject an inserted smartcard. \n");
	fprintf(stderr, "The SmartCard Terminal (SCT) can be specified with option -s.\n\n");

        fprintf(stderr, "usage:\n\n");
        fprintf(stderr, "sc_eject [-htvV] [-s <sct_id>]\n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
	fprintf(stderr, "-s <sct-id>      SCT identifier (default: 1) refers to the SCT\n");
	fprintf(stderr, "                 installation file\n");	  
        fprintf(stderr, "-h               write this help text\n");
	fprintf(stderr, "-t               control malloc/free behaviour\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM SC_EJECT */
}





#else

/* 
 *	If SCA is not defined, this utility is not available.
 */

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

	fprintf(stderr, "\n\nUtility sc_eject is not available (Identifier SCA is not set)\n\n\n");


}

#endif
