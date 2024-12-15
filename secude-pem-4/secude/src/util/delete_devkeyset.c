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
 *      delete_devkeyset
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
	KeyDevStatus	key_set_status;


	char 		*proc = "main (delete_devkeyset)";


	psename	 = getenv("PSE");
	key_set_status = DEV_ANY;


	optind = 1;
	opterr = 0;

	MF_check = FALSE;
					  
	while ( (opt = getopt(cnt, parm, "p:dtvVWh")) != -1 ) {
		switch (opt) {
		case 'p':
			psename = optarg;
			break;
                case 'd':
			key_set_status = DEV_OWN;
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


	if (key_set_status == DEV_OWN) {

		/*
		 *  Request a keycard ( an already inserted smartcard will be ejected)
		 */

		if ( request_keycard(FALSE) < 0 ) {
			fprintf(stderr, "%s: ",cmd);
			fprintf(stderr, "Cannot request a keycard\n");
			aux_fprint_error(stderr, verbose);
			exit(-1);
		}
	}



	/*
	 *  load device key set from a keycard into the SCT
	 */

	if (delete_devkeyset(app_id, key_dev_info, no_of_devkeys, key_set_status) < 0 ) {
		fprintf(stderr, "%s: ",cmd);
		fprintf(stderr, "Cannot delete the device key set for application %s\n", psename);
		aux_fprint_error(stderr, verbose);
		aux_free_OctetString(&app_id);
		if (key_set_status == DEV_OWN)
			sec_sc_eject(CURRENT_SCT);
		exit(-1);
	}


	fprintf(stderr, "Device key set for PSE %s has been deleted in the SCT\n", psename); 


	aux_free_OctetString(&app_id);


	if (key_set_status == DEV_OWN) {

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

        fprintf(stderr, "delete_devkeyset: Delete a device key set within an SCT\n\n\n");
	fprintf(stderr, "Description:\n\n");
	fprintf(stderr, "'delete_devkeyset' can be used by a PSE administrator to delete a PSE specific \n");
	fprintf(stderr, "device key set in an SCT. \n\n");
	fprintf(stderr, "Either a so called 'OWN' or an 'ANY' device key set can be deleted.\n\n");
	fprintf(stderr, "The deletion of an 'OWN' set is only in connection with the belonging keycard \n");
        fprintf(stderr, "possible. In this case an already inserted smartcard will be ejected. The user  \n");
	fprintf(stderr, "will be requested to insert the keycard on which the device key set is stored  \n");
	fprintf(stderr, "and to enter the PIN at the SCT.\n\n\n");

        fprintf(stderr, "usage:\n\n");
        fprintf(stderr, "delete_devkeyset [-hdtvVW] [-p <pse>] \n\n"); 


        if(help == LONG_HELP) {
        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-p <pse>         PSE name (default: Environment variable PSE or %s)\n", DEF_PSE);
        fprintf(stderr, "-d               delete the 'OWN' device key set in the SCT\n");
        fprintf(stderr, "                 otherwise the 'ANY' device key set will be deleted\n");
        fprintf(stderr, "-h               write this help text\n");
	fprintf(stderr, "-t               control malloc/free behaviour\n");
        fprintf(stderr, "-v               verbose\n");
        fprintf(stderr, "-V               Verbose\n");
        fprintf(stderr, "-W               Grand Verbose (for testing only)\n");
        }

        if(MF_check) MF_fprint(stderr);

        exit(-1);                                /* IRREGULAR EXIT FROM DELETE_DEVKEYSET */
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

	fprintf(stderr, "\n\nUtility delete_devkeyset is not available (Identifier SCA is not set)\n\n\n");


}


#endif
