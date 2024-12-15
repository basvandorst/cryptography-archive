/*/*
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
 *      gen_devkeyset
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
	char	        *psename = CNULL, *pin_arg = CNULL;

	OctetString	*app_id;
	KeyDevSel	*key_dev_info[MAX_SCOBJ+1];
	unsigned int 	no_of_devkeys;


	char 		*proc = "main (gen_devkeyset)";


	psename	 = getenv("PSE");

	optind = 1;
	opterr = 0;

	MF_check = FALSE;

	while ( (opt = getopt(cnt, parm, "n:p:htvVW")) != -1 ) {
		switch (opt) {
		case 'n':
			pin_arg = optarg;
			break;
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
	if(!pin_arg) pin_arg = CNULL;



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
	 *  Generate device key set for the given PSE
	 */

	if ( gen_devkeyset(app_id, key_dev_info, no_of_devkeys) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot generate a device key set.\n");
		aux_fprint_error(stderr, verbose);
		aux_free_OctetString(&app_id);
		exit(-1);
	}


	fprintf(stderr, "Device key set for PSE %s has been generated\n", psename); 




	/*
	 *  Write device key set on one or more keycards.
	 */

	if ( write_on_keycards(app_id, key_dev_info, no_of_devkeys, pin_arg) < 0 ) {
		fprintf(stderr, "\n\n%s: ",cmd);
		fprintf(stderr, "Cannot write device key set on a keycard.\n");
		aux_fprint_error(stderr, verbose);
		delete_devkeyset(app_id, key_dev_info, no_of_devkeys, DEV_ANY);
		fprintf(stderr, "The generated device key set has been deleted\n");
		aux_free_OctetString(&app_id);
		exit(-1);
	}




	/*
	 *  Delete device key set in the SCT.
	 */

	if (delete_devkeyset(app_id, key_dev_info, no_of_devkeys, DEV_ANY) < 0) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot delete device key set in the SCT\n");
		aux_fprint_error(stderr, verbose);
		aux_free_OctetString(&app_id);
		exit(-1);
	}

	aux_free_OctetString(&app_id);


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

        fprintf(stderr, "gen_devkeyset: Generate and save a device key set\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'gen_devkeyset' can be used by a PSE administrator to generate a device key set \n");
	fprintf(stderr, "for the given PSE and to store this set on one or more keycard(s). \n\n");
	fprintf(stderr, "An already inserted smartcard will be ejected. \n");
	fprintf(stderr, "When requested the user has to insert a smartcard which hasn't been written.\n");
	fprintf(stderr, "The device key set and the PIN are installed on the smartcard, which becomes \n");
	fprintf(stderr, "automatically a keycard.\n");
	fprintf(stderr, "A keycard can be used to load a PSE specific device key set into an SCT.\n\n\n");

        fprintf(stderr, "usage:\n\n");
        fprintf(stderr,"gen_devkeyset [-htvVW] [-p <pse>] [-n <pin>] \n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <pse>         PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
	fprintf(stderr, "-n <pin>         PIN which shall be installed on the keycard(s)\n");
        fprintf(stderr, "-h               write this help text\n");
	fprintf(stderr, "-t               control malloc/free behaviour\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM GEN_DEVKEYSET */
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

	fprintf(stderr, "\n\nUtility gen_devkeyset is not available (Identifier SCA is not set)\n\n\n");


}

#endif