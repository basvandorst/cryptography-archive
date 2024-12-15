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

/*
 *
 *      load_devkeyset
 *
 */

#include <fcntl.h>
#include <stdio.h>



#ifdef SCA

#include "af.h"		



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
	char	        *cmd = *parm, opt;
	char	        *psename = CNULL;

	OctetString	*app_id;
	KeyDevSel	*key_dev_info[MAX_SCOBJ+1];
	unsigned int 	no_of_devkeys;


	char 		*proc = "main (load_devkeyset)";


	psename	 = getenv("PSE");

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "p:tvVWh")) != -1 ) {
		switch (opt) {
		case 'p':
			psename = optarg;
			break;
                case 't':
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
		case 'h':
			usage(LONG_HELP);
			continue;
		default:
		case '?':
			usage(SHORT_HELP);
		}
	}



        if(!psename) psename = DEF_PSE;




	/*
	 *  Prepare handling of the device key set
	 */

	if ( pre_devkeyset(psename, &app_id, key_dev_info, &no_of_devkeys) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		if (aux_last_error() == EDEVLOCK) 
			fprintf(stderr, "Cannot open device for SCT (device busy)\n");
		else	fprintf(stderr, "Cannot handle device key set for application %s\n", psename);
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}




	/*
	 *  Request a keycard ( an already inserted smartcard will be ejected)
	 */

	if ( request_keycard(FALSE) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot request a keycard\n");
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}




	/*
	 *  load device key set from a keycard into the SCT
	 */

	if (load_devkeyset(app_id, key_dev_info, no_of_devkeys, DEV_OWN) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot load a device key set\n");
		aux_fprint_error(stderr, verbose);
		aux_free_OctetString(&app_id);
		sec_sc_eject(CURRENT_SCT);
		exit(-1);
	}


	fprintf(stderr, "Device key set for PSE %s has been loaded into the SCT\n", psename); 


	aux_free_OctetString(&app_id);


	/*
	 *  Eject the keycard.
	 */

	if ((sec_sc_eject(CURRENT_SCT)) == -1) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot eject keycard.\n");
		aux_fprint_error(stderr, verbose);
		exit(-1);
	}

	/*
	 *  Activate the device key set for the current application
	 */

	if ( re_devkeyset(psename) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot reactivate device key set\n");
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

        fprintf(stderr, "load_devkeyset: Load a device key set into an SCT\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'load_devkeyset' can be used by a PSE administrator to load a PSE specific \n");
	fprintf(stderr, "device key set from a keycard into an SCT. \n\n");
	fprintf(stderr, "An already inserted smartcard will be ejected. \n");
	fprintf(stderr, "When requested the user has to insert the keycard into the SCT and\n");
	fprintf(stderr, "to enter the PIN at the SCT.\n\n\n");

        fprintf(stderr, "usage:\n\n");
        fprintf(stderr, "load_devkeyset [-htvVW] [-p <pse>] \n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <pse>         PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
        fprintf(stderr, "-h               write this help text\n");
	fprintf(stderr, "-t               control malloc/free behaviour\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM LOAD_DEVKEYSET */
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

	fprintf(stderr, "\n\nUtility load_devkeyset is not available (Identifier SCA is not set)\n\n\n");


}

#endif
