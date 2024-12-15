/*
 *  SecuDE Release 4.3 (GMD), STARCOS Version 1.1
 */


/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*----------------Read SC Configuration File------------------------*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)       	    */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``SECUDE'' 1993	                                    */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PACKAGE   readconfig              VERSION   4.3                  */
/*                                      DATE   7.9.93	            */
/*                                        BY   U. Viebeg            */
/* DESCRIPTION                                                      */
/*                                                                  */
/* EXPORT                                                           */
/*   check_SCapp_configuration() Check consistency of configuration */
/*				 data for an SC-application.	    */
/*   get_default_configuration() Set "sc_app_list[]" to default	    */
/*  				 configuration.			    */
/*   display_SC_configuration() Display the actual SC configuration */
/*				("sc_app_list[]").  		    */
/*   read_SC_configuration()    Read SC configuration file into     */
/*				global structure "sc_app_list[]".   */
/*                                                                  */
/* STATIC                                                           */
/*   add_predef_values		Add predefined objects (e.g. SC_PIN)*/
/*				and if app_id is missing the default*/
/*				application id is added.	    */
/*   char2int()			Transform ASCII string to integer   */
/*				value.				    */
/*   check_app_name()		Check whether the actual read 	    */
/*				application name is unique in       */
/*				"sc_app_list[]".		    */
/*   check_config_list()	Check whether the object list of    */
/*				an SC-application contains all      */
/*				mandatory objects.		    */
/*   check_obj_info()		Check whether the actual read       */
/* 				object is unique in object list.    */
/*   free_sc_app_list()		Release all allocated memory in     */
/*   				in "sc_app_list[]".		    */
/*   get_app_info()		Get information (e.g. object list)  */
/*                              for current application.	    */
/*   get_default_index()        Get index in default object list.   */
/*   get_first_app_record()     Get first application record in     */
/*			        file (incl. SC_encrypt, SC_verify). */
/*   get_next_correct_record()  Get next correct record in file and */
/* 				return the parameters.		    */
/*   get_next_word_from_record() Return next word in record.	    */
/*   get_obj_par()		Get object parameters from read     */
/*				record.				    */
/*   handle_mandatory_obj()     Handle mandatory objects in         */
/*				object list.			    */
/*   is_char_relevant()		Check whether character is relevant.*/
/*   is_obj_in_list()	        Check whether given object is part  */
/*			        of given list.			    */ 
/*   is_record_correct()	Check whether parameter in record   */
/*				are correct and return the par.     */
/*   is_record_relevant()	Check whether read record is a 	    */
/*				comment record. 		    */
/*   read_record()	        Read one record from file.	    */
/*   								    */
/*                                                                  */
/* IMPORT              		              		            */
/*                                                                  */
/*  Auxiliary Functions of SECUDE		                    */
/*   aux_add_error()		Add error to error stack.	    */
/*   aux_dechex()	        Decode printable hex 		    */
/*			        representation into hex.	    */
/*								    */
/*                                                                  */
/*  Global Variables                                                */
/*								    */
/*   sc_app_list[]		List of the applications available  */
/*				on the SC, including the list of    */
/*				all objects (app specific),         */
/*				-which shall be stored on the SC or */
/*				-which are stored on the SC.	    */
/*   default_sc_app_list[]	Default SC application list.	    */
/*   default_sc_obj_list[]	Default SC object list.		    */
/*   man_sc_obj_list[] 		List of the mandatory objects 	    */
/*				belonging to one SC application.    */
/*   onekeypair_sc_obj_list[] 	List of the mandatory objects 	    */
/*				for an SC application with one RSA  */
/*				keypair.   			    */
/*   twokeypairs_sc_obj_list[] 	List of the mandatory objects 	    */
/*				for an SC application with two RSA  */
/*				keypairs.   			    */
/*------------------------------------------------------------------*/

#ifdef SCA

#include "secsc_config.h"	/* contains default SC configuration lists	 */



#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>



/*
 *    Local declarations
 */



typedef enum {
	NO_KEYWORD, APP_NAME, APP_ID, IGNORE, SC_ENCRYPT, SC_VERIFY, OBJ_NAME}               ParType;


typedef struct Par_In_Record {
	ParType         par_type;
	union {
		char           *app_name;
		OctetString    *app_id;
		Boolean         boolean_flag;
		SCObjEntry      obj_par;
	}               conf_par;
}               ParInRecord;





#ifdef __STDC__
static int	add_predef_values	(SCAppEntry *sc_app_entry);
static RC	free_sc_app_list	();
static int	get_app_info		(FILE *fp_conf, SCAppEntry *sc_app_entry, char **next_app_name);
static int	get_first_app_record	(FILE *fp_conf, ParInRecord *par_in_record);
static int	get_next_correct_record	(FILE *fp_conf, ParInRecord *par_in_record);
static int	read_record		(FILE *fp, char record[], int rmax);
static Boolean	is_record_correct	(char record[], ParInRecord *par_in_record);
static int	get_obj_par		(char *record, int *rindex, SCObjEntry *obj_par);
static int	is_record_relevant	(char *record);
static char	*get_next_word_from_record	(char *record, int *rindex);
static Boolean	is_char_relevant	(char one_char);
static int	check_app_name		(char *read_app_name);
static int	handle_mandatory_obj	(SCAppEntry *sc_app_entry);
static int	get_default_index	(char *object_name);
static int	check_config_list	(SCAppEntry *sc_app_entry, char *man_obj_list[]);
static int	is_obj_in_list		(char *obj_name, char *obj_list[]);
static int	check_obj_info		(SCAppEntry *sc_app_entry, SCObjEntry *new_obj_par);
static int	char2int		(char *s);

#else

static int	add_predef_values();
static int      char2int();
static int      check_app_name();
static int 	check_config_list();
static int      check_obj_info();
static int      get_app_info();
static int      get_default_index();
static int      get_first_app_record();
static int      get_next_correct_record();
static char    *get_next_word_from_record();
static int      handle_mandatory_obj();
static int      get_obj_par();
static Boolean  is_char_relevant();
static int	is_obj_in_list();
static Boolean  is_record_correct();
static int      is_record_relevant();
static RC       free_sc_app_list();
static int      read_record();

#endif /* not __STDC__ */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  read_SC_configuration				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Read the SC configuration file and stores the information   */
/*  in the global list "sc_app_list[]".			        */
/*								*/
/*  Case 1:							*/
/*  If configuration file can not be opened 			*/
/*      => - "sc_app_list[]" remains unchanged.			*/
/*         - return(0).						*/
/*								*/
/*  Case 2:							*/
/*  If configuration file can be opened, the file is read until */
/*  EOF is found or more than MAX_SC_APP applications are read.	*/
/*								*/
/*  In case of error(s): -  all errors are stacked with 	*/
/*  			    "aux_add_error()",			*/
/*                       -  sc_app_list is set to 0,            */
/*                       -  return (-1).			*/
/*								*/
/****************************************************************/
/*  Handling of SC_encrypt, SC_verify flag:			*/
/*  One record for each flag may be in the configuration file	*/
/*  before the record with the first application name.		*/
/*								*/
/****************************************************************/
/*  Handling of mandatory objects:				*/
/*  For each application it is checked whether the belonging    */
/*  object list contains all mandatory objects.  If a  		*/
/*  mandatory object is missing, the default values for this    */
/*  object is added to the current object list.			*/
/*  The global list "man_sc_obj_list[]" contains the list of the*/
/*  mandatory objects.						*/
/*								*/
/****************************************************************/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   which_SCconfig	       = USER_CONF			*/
/*				 Search for the SC configuration*/
/*			         in the user directory.		*/
/*			       = SYSTEM_CONF			*/
/*			         Take name of SC configuration  */
/*				 file from define variable      */
/*				 SCINIT.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*   add_predef_values	       Add predefined objects (e.g.     */
/*			       SC_PIN) and if app_id is missing */
/*			       the default application id is 	*/
/*			       added.	    			*/
/*   free_sc_app_list()	       Release all allocated memory in  */
/*   			       in "sc_app_list[]".		*/
/*   check_app_name()	       Check whether the actual read 	*/
/*			       application name is unique in    */
/*			       "sc_app_list[]".		        */
/*   get_app_info()	       Get information (e.g. object list)*/
/*                             for current application.	        */
/*   get_first_app_record()    Get first application record in  */
/*			       file.				*/
/*   handle_mandatory_obj()    Handle mandatory objects in      */
/*			       object list.			*/
/*   aux_add_error()	       Add error to error stack.	*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure read_SC_configuration
 *
 ***************************************************************/
#ifdef __STDC__

RC read_SC_configuration(
	WhichSCConfig	  which_SCconfig
)

#else

RC read_SC_configuration(
	which_SCconfig
)
WhichSCConfig	  which_SCconfig;

#endif

{
	int             n = 0;
	int             i = 0;
	int             rc = 0;

	char           *homedir = "";
	char           *config_file_name = "";
	int             fd_conf;
	FILE           *fp_conf;
	int             function_rc = 0;
	int             app_no;
	char           *next_app_name;

	ParInRecord     par_in_record;


	char           *proc = "read_SC_configuration";

/********************************************************************************/
/*
 *	Compose name of configuration file: 
 */

	switch (which_SCconfig) {

	case USER_CONF: 

		/* 
		 *   Name = "home directory || SC_CONFIG_name"
		 */

		homedir = getenv("HOME");
		if (!homedir) {
			aux_add_error(ESYSTEM, "Getenv ('HOME') failed.", CNULL, 0, proc);
			return (-1);
		}
		config_file_name = (char *) malloc(strlen(homedir) + strlen(SC_CONFIG_name) + 64);
		if (!config_file_name) {
			aux_add_error(EMALLOC, "Configuration file name (within HOME directory)", CNULL, 0, proc);
			return (-1);
		}
		strcpy(config_file_name, homedir);
		if (strlen(homedir))
			if (config_file_name[strlen(config_file_name) - 1] != PATH_SEPARATION_CHAR)
				strcat(config_file_name, PATH_SEPARATION_STRING);
		strcat(config_file_name, SC_CONFIG_name);

		break;

	case SYSTEM_CONF: 

		/* 
		 *   configuration file name is taken from define variable "SCINIT".
		 */

#ifdef SCINIT
		config_file_name = (char *) malloc(256);
		if (!config_file_name) {
			aux_add_error(EMALLOC, "Configuration file name (SCINITFILE)", CNULL, 0, proc);
			return (-1);
		}

		strcpy(config_file_name, SCINIT);

#else
		fprintf(stderr,"Preprocessor identifier 'SCINIT' not defined\n");
		free(config_file_name);
		return (0);

#endif

		break;
	default:
		aux_add_error(EINVALID, "Invalid value for parameter (which_SCconfig)", CNULL, 0, proc);
		return -1;

	}  /* end switch */
		


#ifdef CONFIGTEST
	fprintf(stderr, "Name of SC configuration file: %s\n", config_file_name);
#endif



/********************************************************************************/
/*
 *	Open configuration file
 */

	if ((fd_conf = open(config_file_name, O_RDONLY)) < 0) {
#ifdef SECSCTEST
		fprintf(stderr, "Configuration file %s missing!\n", config_file_name);
#endif
		free(config_file_name);
		return (0);
	}
	if ((fp_conf = fdopen(fd_conf, "r")) == (FILE * ) 0) {
		aux_add_error(ESYSTEM, "Configuration file cannot be opened", config_file_name, char_n, proc);
	        close(fd_conf);
	        free(config_file_name);
		return(-1);
	}
/********************************************************************************/
/*
 *	Read configuration file and store information into application list
 */

	/* init */
	next_app_name = CNULL;

	/*
	 * Get first application name in configuration file (incl.
	 * SC_encrypt- , SC_verify-flag)
	 */

	rc = get_first_app_record(fp_conf, &par_in_record);
	if (rc == ERR_flag) {
		aux_add_error(ECONFIG, "APP: Error in get first application name ", CNULL, 0, proc);
		function_rc = ERR_flag;
	}
	if ((rc == EOF_flag) || (rc == EOF_with_ERR) || (par_in_record.par_type != APP_NAME)) {
		aux_add_error(ECONFIG, "APP: No application in configuration file", CNULL, 0, proc);
		goto err_case;
	}
	/* name of first application */
	next_app_name = par_in_record.conf_par.app_name;


	/*
	 * Loop for SC-application(s) in configuration file (until EOF or too
	 * many applications in file ( max number = MAX_SCAPP))
	 * 
	 * Errors are stacked with "aux_add_error()"
	 * 
	 */

	app_no = 0;
	sc_app_list[app_no].app_name = CNULL;

	while ((app_no < MAX_SCAPP) && (next_app_name != CNULL) &&
	       (rc != EOF_flag) && (rc != EOF_with_ERR)) {

		/* check next application name */
		rc = check_app_name(next_app_name);
		if (rc < 0) {
			AUX_ADD_ERROR;
			function_rc = ERR_flag;
		}

		/*
		 * new entry in sc_app_list (next_app_name becomes current
		 * app_name)
		 */
		sc_app_list[app_no].app_name = next_app_name;

		next_app_name = CNULL;

		/*
		 * get parameters (e.g. object list) for application until
		 * EOF or next app. found
		 */
		rc = get_app_info(fp_conf, &sc_app_list[app_no], &next_app_name);
		if ((rc == ERR_flag) || (rc == EOF_with_ERR)) {
			aux_add_error(ECONFIG, "APP: Error in configuration for application ", sc_app_list[app_no].app_name, char_n, proc);
			function_rc = ERR_flag;
		}
		

		/*
		 *  Add predefined values to the parameter list:
		 *
		 *	- add all predefined objects
		 *      - if an app_id is missing, 
	         *		the default application id is added.
		 */

		rc = add_predef_values(&sc_app_list[app_no]);
		if (rc == ERR_flag) {
			aux_add_error(ECONFIG, "APP: Error in configuration for application ", sc_app_list[app_no].app_name, char_n, proc);
			function_rc = ERR_flag;
		}


		/*
		 * If mandatory objects are missing in current object list =>
		 * add them.
		 */
		rc = handle_mandatory_obj(&sc_app_list[app_no]);
		if (rc == ERR_flag) {
			aux_add_error(ECONFIG, "APP: Error in configuration for application ", sc_app_list[app_no].app_name, char_n, proc);
			function_rc = ERR_flag;
		}
		app_no++;
		sc_app_list[app_no].app_name = CNULL;

	}			/* end while */


	if (app_no >= MAX_SCAPP) {
		aux_add_error(ECONFIG, "APP: Too many applications in configuration file", CNULL, 0, proc);
		goto err_case;
	}
	if ((rc == EOF_with_ERR) || (function_rc == ERR_flag)) {
		goto err_case;
	}

/*
 *	release allocated memory (normal end)
 */

	close(fd_conf);
	fclose(fp_conf);
	free(config_file_name);
	return (0);




/*
 *	release allocated memory (in case or error)
 */

err_case:

	/* release all allocated memory and set "sc_app_list[]" to 0 */
	rc = free_sc_app_list();

	close(fd_conf);
	fclose(fp_conf);
	free(config_file_name);
	if (next_app_name != CNULL)
		free(next_app_name);
	return (-1);

}				/* end read_SC_configuration */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  display_SC_configuration			       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Display the actual SC configuration ("sc_app_list[]").      */
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure display_SC_configuration
 *
 ***************************************************************/
#ifdef __STDC__

void display_SC_configuration(
)

#else

void display_SC_configuration(
)

#endif

{
	int             rc, n, i;


	fprintf(stderr, "S C   C O N F I G U R A T I O N \n\n");

	if (SC_encrypt == TRUE)
		fprintf(stderr, "    SC_encrypt= TRUE\n");
	else
		fprintf(stderr, "    SC_encrypt= FALSE\n");


	if (SC_verify == TRUE)
		fprintf(stderr, "    SC_verify=  TRUE\n");
	else
		fprintf(stderr, "    SC_verify=  FALSE\n");



	for (n = 0; sc_app_list[n].app_name; n++) {
		fprintf(stderr, "\n\n%d.  APPLICATION= %s\n", n + 1, sc_app_list[n].app_name);

		fprintf(stderr, "\n\nAPPLICATION_ID=:\n");
		aux_fxdump(stderr, sc_app_list[n].app_id->octets, sc_app_list[n].app_id->noctets, 0);
		fprintf(stderr, "\n");

		if (sc_app_list[n].ignore_flag == TRUE)
			fprintf(stderr, "    IGNORE FLAG= TRUE\n");
		else
			fprintf(stderr, "    IGNORE FLAG= FALSE\n");

		fprintf(stderr, "\n    List of the SC-objects:\n");
		for (i = 0; sc_app_list[n].sc_obj_list[i].name; i++) {
/*name*/ 		fprintf(stderr, "\n      %d. OBJECT=  %s", i + 1, sc_app_list[n].sc_obj_list[i].name);

/*id=key*/ 		if (sc_app_list[n].sc_obj_list[i].type == SC_KEY_TYPE) {
				fprintf(stderr, "\n         SC_KEY_TYPE  ");

/*key_number*/ 			fprintf(stderr, " key_no=      %d          ", sc_app_list[n].sc_obj_list[i].sc_id);

			} else {
				if (sc_app_list[n].sc_obj_list[i].type == SC_FILE_TYPE) {
/*id=File*/ 				fprintf(stderr, "\n         SC_FILE_TYPE ");

/*file_name*/ 				fprintf(stderr, " short_index= %d", sc_app_list[n].sc_obj_list[i].sc_id);

/*no of bytes*/ 			fprintf(stderr, "  NOB=%d ", sc_app_list[n].sc_obj_list[i].size);

				} else
					fprintf(stderr, "\nunknown object id\n");
			}

			fprintf(stderr, "   SM_PAR= (");
/*sec_mess sm_SCT*/
			if (sc_app_list[n].sc_obj_list[i].sm_SCT == SCT_SEC_NORMAL)
				fprintf(stderr, " NORM");
			else if (sc_app_list[n].sc_obj_list[i].sm_SCT == SCT_INTEGRITY)
				fprintf(stderr, " INTG");
			else if (sc_app_list[n].sc_obj_list[i].sm_SCT == SCT_CONCEALED)
				fprintf(stderr, " CONC");
			else if (sc_app_list[n].sc_obj_list[i].sm_SCT == SCT_COMBINED)
				fprintf(stderr, " COMB");
			else
				fprintf(stderr, "\nInvalid sm_sCT\n");
/*sec_mess sm_SC_read_command*/
			if (sc_app_list[n].sc_obj_list[i].sm_SC_read.command == ICC_SEC_NORMAL)
				fprintf(stderr, " NORM");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_read.command == ICC_AUTHENTIC)
				fprintf(stderr, " AUTH");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_read.command == ICC_CONCEALED)
				fprintf(stderr, " CONC");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_read.command == ICC_COMBINED)
				fprintf(stderr, " COMB");
			else
				fprintf(stderr, "\nInvalid sm_SC_read_command");

/*sec_mess sm_SC_read_response*/
			if (sc_app_list[n].sc_obj_list[i].sm_SC_read.response == ICC_SEC_NORMAL)
				fprintf(stderr, " NORM");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_read.response == ICC_AUTHENTIC)
				fprintf(stderr, " AUTH");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_read.response == ICC_CONCEALED)
				fprintf(stderr, " CONC");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_read.response == ICC_COMBINED)
				fprintf(stderr, " COMB");
			else
				fprintf(stderr, "\nInvalid sm_SC_read.response\n");

/*sec_mess sm_SC_write_command*/
			if (sc_app_list[n].sc_obj_list[i].sm_SC_write.command == ICC_SEC_NORMAL)
				fprintf(stderr, " NORM");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_write.command == ICC_AUTHENTIC)
				fprintf(stderr, " AUTH");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_write.command == ICC_CONCEALED)
				fprintf(stderr, " CONC");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_write.command == ICC_COMBINED)
				fprintf(stderr, " COMB");
			else
				fprintf(stderr, "\nInvalid sm_SC_write.command\n");

/*sec_mess sm_SC_write_response*/
			if (sc_app_list[n].sc_obj_list[i].sm_SC_write.response == ICC_SEC_NORMAL)
				fprintf(stderr, " NORM");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_write.response == ICC_AUTHENTIC)
				fprintf(stderr, " AUTH");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_write.response == ICC_CONCEALED)
				fprintf(stderr, " CONC");
			else if (sc_app_list[n].sc_obj_list[i].sm_SC_write.response == ICC_COMBINED)
				fprintf(stderr, " COMB");
			else
				fprintf(stderr, "\nInvalid sm_SC_write.response\n");

			fprintf(stderr, " )");

		}		/* end for objects */


	}			/* end for applications */
	fprintf(stderr, "\n\n");

}				/* end display_SC_configuration */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_default_configuration			       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Store the default configuration list "default_sc_app_list[]"*/
/*  in the configuration list "sc_app_list[]". For each         */
/*  application the "default_sc_obj_list[]" is used as default  */
/*  object list.						*/
/*								*/
/*??????????????????????????????????????????????????????????????*/
/*	Default value for the application id missing		*/
/*??????????????????????????????????????????????????????????????*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure get_default_configuration
 *
 ***************************************************************/
#ifdef __STDC__

RC get_default_configuration(
)

#else

RC get_default_configuration(
)

#endif

{
	int             n = 0;
	int             i = 0;


	char           *proc = "get_default_configuration";

	for (n = 0; default_sc_app_list[n]; n++) {

		sc_app_list[n].app_name = default_sc_app_list[n];
		sc_app_list[n].app_id = NULLOCTETSTRING;
		sc_app_list[n].ignore_flag = SC_ignore_SWPSE;

		/* get SC object list */
		for (i = 0; default_sc_obj_list[i].name; i++)
			sc_app_list[n].sc_obj_list[i] = default_sc_obj_list[i];
		sc_app_list[n].sc_obj_list[i].name = CNULL;
	}

	sc_app_list[n].app_name = CNULL;

	return (0);

}				/* end get_default_configuration */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  free_sc_app_list				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Release all allocated memory in "sc_app_list[]" and set 	*/
/*  "sc_app_list[]" to 0.					*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure free_sc_app_list
 *
 ***************************************************************/
#ifdef __STDC__

static RC free_sc_app_list(
)

#else

static RC free_sc_app_list(
)

#endif

{

	int             n = 0;
	int             i = 0;

	char 		*proc  = "free_sc_app_list";




	for (n = 0; sc_app_list[n].app_name; n++) {

		if (sc_app_list[n].app_name)
			free(sc_app_list[n].app_name);
		if (sc_app_list[n].app_id)
			aux_free_OctetString(&sc_app_list[n].app_id);

		for (i = 0; sc_app_list[n].sc_obj_list[i].name; i++) {
			if (sc_app_list[n].sc_obj_list[i].name)
				free(sc_app_list[n].sc_obj_list[i].name);
		}		

	}

	sc_app_list[0].app_name = CNULL;

	return (0);

}





/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_app_info					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Read information (object list) for the current application 	*/
/*  and returns the information in paramter "sc_app_entry" .    */
/*								*/
/*  Read file until next application or EOF. Errors are stacked */
/*  with "aux_add_error()".					*/
/*								*/
/****************************************************************/
/*  Handling of the flags SC_Encrypt and SC_verify		*/
/*  These flags are ignored within the list of the application  */
/*  parameters.							*/
/*								*/
/****************************************************************/
/*  IGNORE flag, APPLICATION ID:				*/
/*  These parameters are expected to be in the records between	*/
/*  the application name and the belonging object list.	        */
/*								*/
/****************************************************************/
/*  Handling of the OBJECTS					*/
/*  If the number of objects read for one application is greater*/
/*  than MAX_SCOBJ, all further objects are stored in 		*/
/*  "sc_app_entry->sc_obj_list[MAX_SCOBJ]".			*/
/*  Within the object-list all other application parameters are */
/*  ignored.							*/
/*  								*/
/****************************************************************/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   fp_conf			Pointer to configuration file 	*/
/*   *sc_app_entry  		Pointer to an entry in the      */
/*				"sc_app_list[]".		*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*   sc_app_entry  		Entry of the "sc_app_list":	*/
/*				- ignore flag			*/
/*				- object_list			*/
/*   next_app_name		name of the next application    */
/*				(memory for next_app_name is    */
/*				 allocated)			*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  ERR_flag		       Error			       	*/
/*  EOF_flag		       EOF found		       	*/
/*  EOF_with_ERR	       Error and EOF found .       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   check_obj_info()	       Check whether the actual read    */
/* 			       object is unique in object list. */
/*   get_next_correct_record() Get next correct record in file  */
/* 			       and return the parameters.       */
/*   aux_add_error()	       Add error to error stack.	*/
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure get_app_info
 *
 ***************************************************************/
#ifdef __STDC__

static int get_app_info(
	FILE		 *fp_conf,
	SCAppEntry	 *sc_app_entry,
	char		**next_app_name
)

#else

static int get_app_info(
	fp_conf,
	sc_app_entry,
	next_app_name
)
FILE		 *fp_conf;
SCAppEntry	 *sc_app_entry;
char		**next_app_name;

#endif

{
	int             n = 0;
	int             i = 0;
	int             rc = 0;


	int             obj_no;
	ParInRecord     par_in_record;
	Boolean         found_next_app_name;
	Boolean         found_object_name;

	int             function_rc = 0;


	char           *proc = "get_app_info";

	/* init parameters for an application*/
	sc_app_entry->ignore_flag = SC_ignore_SWPSE;
	sc_app_entry->app_id = NULLOCTETSTRING;
	


	/*
	 * Get next correct record in file and return all parameters
	 */

	rc = get_next_correct_record(fp_conf, &par_in_record);
	if (rc == ERR_flag) {
/*	   	aux_add_error (ECONFIG , "OBJ: Error in get correct record ", CNULL, 0, proc); */
		function_rc = ERR_flag;
	}
	if ((rc == EOF_flag) || (rc == EOF_with_ERR)) {
		return (rc);
	}

	/*
	 * Loop for parameters belonging to the application:
	 *    - IGNORE flag, this parameter is optional (default value = TRUE)
	 *    - APPLICATION IDentifier, this parameter is optional 
	 *
	 * Loop ends in case of an error, EOF found, first object or next application found.
	 */


	found_object_name = FALSE;
	found_next_app_name = FALSE;

	while ((rc != EOF_flag) && (rc != EOF_with_ERR) &&
	       (found_next_app_name == FALSE) && (found_object_name == FALSE)) {

		switch (par_in_record.par_type) {
		case APP_NAME:
			/* get name of the next application in file */
			*next_app_name = par_in_record.conf_par.app_name;
			found_next_app_name = TRUE;
			break;

		case IGNORE:
			sc_app_entry->ignore_flag = par_in_record.conf_par.boolean_flag;
			break;

		case SC_ENCRYPT:
		case SC_VERIFY:

			/*
			 * within the list of the application parameters the flags 
			 * "SC_ENCRYPT", "SC_VERIFY" are ignored.
			 */

			break;

		case APP_ID:
			sc_app_entry->app_id = par_in_record.conf_par.app_id;
			break;

		case OBJ_NAME:
			found_object_name = TRUE;
			break;

		default:
			aux_add_error(ECONFIG, "Invalid parameter record for application ", sc_app_entry->app_name, char_n, proc);
			function_rc = ERR_flag;
			break;

		}		/* end switch */


		if ((found_next_app_name == FALSE) && (found_object_name == FALSE)) {
			rc = get_next_correct_record(fp_conf, &par_in_record);

			if (rc == ERR_flag) {
/*	    			aux_add_error (ECONFIG , "OBJ: Error in get correct record ", CNULL, 0, proc);*/
				function_rc = ERR_flag;
			}
			if ((rc == EOF_flag) || (rc == EOF_with_ERR)) 
				return (rc);

		}
	}			/* end while */




	/*
	 * Loop for objects belonging to actual application (until error or
	 * EOF or next application found)
	 */

	obj_no = 0;
	sc_app_entry->sc_obj_list[obj_no].name = CNULL;

	while ((rc != EOF_flag) && (rc != EOF_with_ERR) &&
	       (found_next_app_name == FALSE)) {

		switch (par_in_record.par_type) {
		case APP_NAME:
			/* get name of the next application in file */
			*next_app_name = par_in_record.conf_par.app_name;
			found_next_app_name = TRUE;
			break;

		case IGNORE:
		case SC_ENCRYPT:
		case SC_VERIFY:
		case APP_ID:

			/*
			 * within the object list application parameters as "IGNORE", 
			 * "SC_ENCRYPT", "SC_VERIFY", "APPLICATION_ID 
			 * are ignored.
			 */

			break;

		case OBJ_NAME:

			/*
			 * check new object (e.g. compare with already
			 * entered objects)
			 */
			rc = check_obj_info(sc_app_entry, &par_in_record.conf_par.obj_par);
			if (rc < 0) {
				aux_add_error(ECONFIG, "OBJ: Invalid object for application", sc_app_entry->app_name, char_n, proc);
				function_rc = ERR_flag;
			}

			/*
			 * enter new object in obj_list[obj_no].
			 */
			sc_app_entry->sc_obj_list[obj_no] = par_in_record.conf_par.obj_par;

#ifdef CONFIGTEST
			fprintf(stderr, "Name of new entered object : %s\n", sc_app_entry->sc_obj_list[obj_no].name);
#endif


			obj_no++;

			/*
			 * if obj_no >= MAX_SCOBJ => obj_no will be set to
			 * MAX_SCOBJ
			 */
			if (obj_no >= MAX_SCOBJ) {
				aux_add_error(ECONFIG, "OBJ: Too many objects for application ", sc_app_entry->app_name, char_n, proc);
				obj_no = MAX_SCOBJ;
				function_rc = ERR_flag;
			}
			sc_app_entry->sc_obj_list[obj_no].name = CNULL;
			break;

		default:
			aux_add_error(ECONFIG, "Invalid parameter record for application ", sc_app_entry->app_name, char_n, proc);
			function_rc = ERR_flag;
			break;

		}		/* end switch */

		if (found_next_app_name == FALSE) {
			rc = get_next_correct_record(fp_conf, &par_in_record);
			if ((rc == ERR_flag) || (rc == EOF_with_ERR)) {
/*	         aux_add_error (ECONFIG , "OBJ: Error in get correct record ", CNULL, 0, proc); */
				function_rc = ERR_flag;
			}
		}
	}			/* end while */


	/*
	 * Return value
	 */

	if (rc == EOF_with_ERR)
		return (EOF_with_ERR);

	if (rc == EOF_flag) {
		if (function_rc == ERR_flag)
			return (EOF_with_ERR);
		else
			return (EOF_flag);
	} else
		return (function_rc);




}				/* end get_app_info */










/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_first_app_record				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Read configuration file until first record with application */
/*  name is found or EOF.					*/
/*								*/
/****************************************************************/
/*  Handling of SC_encrypt, SC_verify flag:			*/
/*  The first relevant records of the file may contain the 	*/
/*  SC_ENCRYPT and/or SC_VERIFY flags. According to these flags */
/*  the global variables SC_encrypt, SC_verify are set.		*/
/*								*/
/****************************************************************/
/*  Case 1:							*/
/*  If app-name can be found:					*/
/*     => - app_name is set in "par_in_record->conf_par.app_name"*/
/*        - return(0)						*/
/*								*/
/*  Case 2:							*/
/*  If no app-name can be found (EOF reached):			*/
/*     => return (EOF_with_ERR)					*/
/*								*/
/*  Case 3:							*/
/*  If records with application parameters (IGNORE, OBJECT) are */
/*  found before the record with an application name:		*/
/*     => - error(s) are stacked with "aux_add_error()",	*/
/*        - "par_in_record->conf_par.app_name" is set to the the*/
/*          found application name,				*/
/*        - return(ERR_flag).					*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   fp_conf			Pointer to configuration file 	*/
/*   *par_in_record		Pointer to structure.		*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*   par_in_record		If not ERR, this structure	*/
/*			        contains the correct app_name	*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  ERR_flag		       Error			       	*/
/*  EOF_with_ERR	       Error and EOF found	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_next_correct_record()  Get next correct record in file */
/* 				and return the parameters.      */
/*   aux_add_error()	       Add error to error stack.	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure get_first_app_record
 *
 ***************************************************************/
#ifdef __STDC__

static int get_first_app_record(
	FILE		 *fp_conf,
	ParInRecord	 *par_in_record
)

#else

static int get_first_app_record(
	fp_conf,
	par_in_record
)
FILE		 *fp_conf;
ParInRecord	 *par_in_record;

#endif

{


	int             function_rc = 0;
	Boolean         app_name_found = FALSE;
	int             rc;

	char           *proc = "get_first_app_record";

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  get_first_app_record\n\n");*/
#endif


	do {

		rc = get_next_correct_record(fp_conf, par_in_record);
		if ((rc == EOF_flag) || (rc == EOF_with_ERR)) {
			aux_add_error(ECONFIG, "APP: Cannot find first app_name in file. ", CNULL, 0, proc);
			return (EOF_with_ERR);
		}
		if (rc == ERR_flag)
			function_rc = 0;	/* ignore all leading invalid
						 * records */

		/* check type of record */
		switch (par_in_record->par_type) {

		case APP_NAME:
			app_name_found = TRUE;
			break;

		case OBJ_NAME:
			aux_add_error(ECONFIG, "APP: Object record found before app_name", CNULL, 0, proc);
			/* release memory for object name */
			free(par_in_record->conf_par.obj_par.name);
			par_in_record->conf_par.obj_par.name = CNULL;
			function_rc = ERR_flag;
			break;

		case IGNORE:
			aux_add_error(ECONFIG, "APP: Ignore flag found before app_name", CNULL, 0, proc);
			function_rc = ERR_flag;
			break;

		case SC_ENCRYPT:
			SC_encrypt = par_in_record->conf_par.boolean_flag;
			break;

		case SC_VERIFY:
			SC_verify = par_in_record->conf_par.boolean_flag;
			break;

		default:
			aux_add_error(ECONFIG, "APP: Unknown record type found before app_name", CNULL, 0, proc);
			function_rc = ERR_flag;
			break;

		}		/* end switch */


	} while (app_name_found == FALSE);


	/*
	 * Return value
	 */

	return (function_rc);

}				/* end get_first_app_record */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_next_correct_record			       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Get next correct record in file and return all parameters	*/
/*  i.e. jumb over all leading comment-, blank- or  empty 	*/
/*  and incorrect records.					*/
/*								*/
/*  This function reads the file until a correct record is found*/
/*  or EOF. 							*/
/*								*/
/*  In case of an error (e.g. invalid keyword, invalid par.)    */
/*  the error together with the actual record is stacked  	*/
/*  in function "is_record_correct" with "aux_add_error()".	*/
/*  								*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   fp_conf			Pointer to configuration file 	*/
/*   *par_in_record		Pointer to structure.		*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*   par_in_record		If not EOF, this structure	*/
/*			        contains the correct par.	*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  ERR_flag		       Error			       	*/
/*  EOF_flag		       EOF found		       	*/
/*  EOF_with_ERR	       Error and EOF found	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   is_record_correct()       Check whether parameter in record*/
/*			       are correct and return the par.  */
/*   is_record_relevant()      Check whether read record is a 	*/
/*			       comment record. 		        */
/*   read_record()	       Read one record from file.	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure get_next_correct_record
 *
 ***************************************************************/
#ifdef __STDC__

static int get_next_correct_record(
	FILE		 *fp_conf,
	ParInRecord	 *par_in_record
)

#else

static int get_next_correct_record(
	fp_conf,
	par_in_record
)
FILE		 *fp_conf;
ParInRecord	 *par_in_record;

#endif

{


	int             function_rc = 0;
	Boolean         found_relevant_record = FALSE;
	Boolean         found_correct_record = FALSE;
	char            record[MAX_RECORD + 1];	/* one character for '\0'             */
	int             rc;

	char           *proc = "get_next_correct_record";

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  get_next_correct_record\n\n");*/
#endif


	do {

		rc = read_record(fp_conf, record, MAX_RECORD);

		if (rc != EOF_flag) {
			found_relevant_record = is_record_relevant(record);
			if (found_relevant_record == TRUE) {
				found_correct_record = is_record_correct(record, par_in_record);
				if (found_correct_record == FALSE)
					function_rc = ERR_flag;
			}
		}
	} while ((found_correct_record == FALSE) && (rc != EOF_flag));


	/*
	 * Return value
	 */

	if (rc == EOF_flag) {
		if (function_rc == ERR_flag)
			return (EOF_with_ERR);
		else
			return (EOF_flag);
	} else
		return (function_rc);

}				/* end get_next_correct_record */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  read_record					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Read one record or max n chars from file. The read chars	*/
/*  are concatenated with '\0' and returned in parameter 	*/
/*  "record".						        */
/*  								*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   fp_conf			Pointer to configuration file 	*/
/*   rmax			Max no. of chars to be read.	*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*   record			Read record			*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  EOF_flag		       EOF found		       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure read_record
 *
 ***************************************************************/
#ifdef __STDC__

static int read_record(
	FILE	 *fp,
	char	  record[],
	int	  rmax
)

#else

static int read_record(
	fp,
	record,
	rmax
)
FILE	 *fp;
char	  record[];
int	  rmax;

#endif

{

	int             ctr;	/* counter for read chars */
	int             c;	/* single read character */
	int             NL_found;


#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  read_record\n\n");*/
#endif

	ctr = 0;
	NL_found = 0;


	do {
		c = getc(fp);
		if (c != EOF) {
			if (c == '\n')
				NL_found = 1;
			record[ctr] = c;
			ctr++;
		}		/* end if */
	} while ((c != EOF) && (ctr < rmax) && (NL_found == 0));

	if (NL_found == 1) {
		record[ctr - 1] = '\0';
		return (ctr);
	}
	if (c == EOF)
		return (EOF_flag);


	record[ctr] = '\0';
	return (ctr);		/* return number of read characters */


}				/* read_record */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  is_record_relevant				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check delivered record and 					*/
/*        return FALSE, if record is empty or			*/
/*        	        if record consists of blanks or		*/
/*                      if record contains a comment.		*/
/*  Otherwise tailing comments are cut off in record and TRUE is*/
/*  returned.							*/
/*  The delivered record is supposed to be a string (null-	*/
/*  terminated). 						*/
/*  								*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE	         	Record is relevant.       	*/
/*   FALSE	         	Record is not relevant.       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure is_record_relevant
 *
 ***************************************************************/
#ifdef __STDC__

static int is_record_relevant(
	char	 *record
)

#else

static int is_record_relevant(
	record
)
char	 *record;

#endif

{


	int             i, ind;
	Boolean         relevant_sign_found;
	Boolean         comment_sign_found;
	char           *ptr_to_comment;

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  is_record_relevant\n\n");*/
#endif


	/*
	 * is record empty ?
	 */

	if ((record == CNULL) || (strlen(record) == 0))
		return (FALSE);


	/* get first character which is != BLANK, COMMENT, TAB */

	i = 0;
	relevant_sign_found = FALSE;
	while ((i < strlen(record)) &&
	       (record[i] != COMMENT) && (relevant_sign_found == FALSE)) {

		if ((record[i] == BLANK_CHAR) ||
		    (record[i] == TAB) ||
		    (record[i] == CR_CHAR))
			i++;
		else
			relevant_sign_found = TRUE;
	}			/* end while */


	if (relevant_sign_found == FALSE)
		return (FALSE);	/* record not relevant */


	/* cut off tailing comment in relevant record */
	comment_sign_found = FALSE;
	while ((i < strlen(record)) && (comment_sign_found == FALSE)) {

		if (record[i] == COMMENT) {
			record[i] = '\0';
			comment_sign_found = TRUE;
		} else
			i++;
	}			/* end while */


	return (TRUE);

}				/* is_record_relevant */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  is_record_correct		      		       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check whether record contains correct parameters. If values */
/*  are correct, all parameters are returned in "par_in_record".*/
/*								*/
/*  In case of an error (e.g. invalid keyword, invalid par.)    */
/*  the error together with the actual record is stacked with 	*/
/*  "aux_add_error()" and FALSE is returned.		 	*/
/*  								*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   record			Pointer to the record to be	*/
/*				checked.		 	*/
/*   *par_in_record		Pointer to structure.		*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*   par_in_record		In case of correct parameter    */
/*				values, this structure		*/
/*			        contains the correct par.	*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE	               Record is correct.	       	*/
/*   FALSE		       Record contains invalid values.	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_dechex()	       Decode printable hex 		*/
/*			       representation into hex.		*/
/*   char2int()		       Transform ASCII string to integer*/
/*			       value.				*/
/*   get_next_word_from_record() Return next word in record.	*/
/*   get_obj_par()	       Get object parameters from read  */
/*			       record.				*/
/*   is_char_relevant()	       Check whether character is       */
/*			       relevant.			*/
/*   aux_add_error()	       Add error to error stack.	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure is_record_correct
 *
 ***************************************************************/
#ifdef __STDC__

static Boolean is_record_correct(
	char		  record[],
	ParInRecord	 *par_in_record
)

#else

static Boolean is_record_correct(
	record,
	par_in_record
)
char		  record[];
ParInRecord	 *par_in_record;

#endif

{


	Boolean         found_relevant_record = FALSE;
	int             rindex;	/* index for record 	              */

	/* where to start read word in record */
	int             rc;
	char           *word;	/* one word in record	              */
	Boolean         flag;
	OctetString	appid_hex;

	char           *proc = "is_record_correct";

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  is_record_correct\n\n");*/
#endif


/*******************************************************************************************/
/*
 * Input:		 1. A relevant record.
 *			 2. A tailing comment has been cut off.
 *			 3. record is a null-terminated string.
 *
 * Next to do:		 1. Get first word in record, which is supposed to be one of the
 *		            following keywords: APP_KEY_WORD, OBJ_KEY_WORD, IGN_KEY_WORD
 *			 2. Get the parameters according to the keyword
 */


#ifdef CONFIGTEST
	if (record)
		fprintf(stderr, " relevant record: \n%s\n\n", record);
#endif

	/*
	 * get keyword (first word in record)
	 */

	rindex = 0;
	word = get_next_word_from_record(record, &rindex);
	if ((word == CNULL) || (strlen(word) == 0)) {
		aux_add_error(ECONFIG, "PAR: No keyword found in record", record, char_n, proc);
		return (FALSE);
	}

	/*
	 * keyword  analyse
	 */

	if (!strncmp(word, APP_ID_KEY_WORD, strlen(APP_ID_KEY_WORD)))
		par_in_record->par_type = APP_ID;
	else if (!strncmp(word, OBJ_KEY_WORD, strlen(OBJ_KEY_WORD)))
		par_in_record->par_type = OBJ_NAME;
	else if (!strncmp(word, APP_KEY_WORD, strlen(APP_KEY_WORD)))
		par_in_record->par_type = APP_NAME;
	else if (!strncmp(word, IGN_KEY_WORD, strlen(IGN_KEY_WORD)))
		par_in_record->par_type = IGNORE;
	else if (!strncmp(word, SC_ENC_KEY_WORD, strlen(SC_ENC_KEY_WORD)))
		par_in_record->par_type = SC_ENCRYPT;
	else if (!strncmp(word, SC_VER_KEY_WORD, strlen(SC_VER_KEY_WORD)))
		par_in_record->par_type = SC_VERIFY;

	else {
		par_in_record->par_type = NO_KEYWORD;
		free(word);
		aux_add_error(ECONFIG, "PAR: Unknown keyword found in record", record, char_n, proc);
		return (FALSE);
	}			/* end else */
	free(word);


	/*
	 * get parameters depending on keyword
	 */

	switch (par_in_record->par_type) {

	case OBJ_NAME:
		/* init object name */
		par_in_record->conf_par.obj_par.name = CNULL;

		/* get object parameters */
		rc = get_obj_par(record, &rindex, &par_in_record->conf_par.obj_par);
		if (rc == ERR_flag) {
/*	                    aux_add_error(ECONFIG, "PAR: Invalid parameter for object in record", record, char_n, proc); */
			if (par_in_record->conf_par.obj_par.name != CNULL)
				free(par_in_record->conf_par.obj_par.name);
			return (FALSE);
		}
		break;

	case APP_NAME:
		/* get app_name */
		word = get_next_word_from_record(record, &rindex);
		if ((word == CNULL) || (strlen(word) == 0)) {
			aux_add_error(ECONFIG, "PAR: No application name found in record", record, char_n, proc);
			return (FALSE);
		}
		par_in_record->conf_par.app_name = word;
		break;

	case APP_ID:
		/* get app_id */
		word = get_next_word_from_record(record, &rindex);
		if ((word == CNULL) || (strlen(word) == 0)) {
			aux_add_error(ECONFIG, "PAR: No application id found in record", record, char_n, proc);
			return (FALSE);
		}
		appid_hex.noctets = strlen(word);
		appid_hex.octets = word;
		if (!(par_in_record->conf_par.app_id = aux_dechex(&appid_hex))) {
			aux_add_error(ECONFIG, "Cannot decode hex representation of the application id", record, char_n, proc);
			return (FALSE);
		}
	
		/* check length of application id */
		if (par_in_record->conf_par.app_id->noctets !=  APPID_L) {
			aux_add_error(ECONFIG, "PAR: Invalid length of application id in record", record, char_n, proc);
			return (FALSE);
		}




		break;


	case IGNORE:
	case SC_ENCRYPT:
	case SC_VERIFY:
		/* get_boolean_flag */
		par_in_record->conf_par.boolean_flag = TRUE;

		word = get_next_word_from_record(record, &rindex);
		if ((word != CNULL) && (strlen(word) != 0)) {
			if (!strncmp(word, TRUE_WORD, strlen(TRUE_WORD)))
				par_in_record->conf_par.boolean_flag = TRUE;
			else if (!strncmp(word, FALSE_WORD, strlen(FALSE_WORD)))
				par_in_record->conf_par.boolean_flag = FALSE;
			else {
				aux_add_error(ECONFIG, "PAR: Invalid boolean flag in record", record, char_n, proc);
				free(word);
				return (FALSE);
			}
		}		/* end if */
		free(word);

		break;


	default:
		par_in_record->par_type = NO_KEYWORD;
		aux_add_error(ECONFIG, "PAR: Unknown keyword found in record", record, char_n, proc);
		return (FALSE);

	}			/* end switch */


	return (TRUE);


}				/* is_record_correct */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_obj_par					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Get parameters for object from delivered record.		*/
/*								*/
/*  The delivered record is supposed to be a string (null-	*/
/*  terminated).  This function gets the parameters from record */
/*  from the offset record + *rindex.				*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   record			Pointer to record.		*/
/*   rindex			Pointer to index in record.	*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*   obj_par			Parameters for object.    	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*   ERR_flag		       error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   char2int()		       Transform ASCII string to integer*/
/*			       value.				*/
/*   get_next_word_from_record() Return next word in record.	*/
/*   is_char_relevant()	       Check whether character is       */
/*			       relevant.			*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure get_obj_par
 *
 ***************************************************************/
#ifdef __STDC__

static int get_obj_par(
	char		 *record,
	int		 *rindex,
	SCObjEntry	 *obj_par
)

#else

static int get_obj_par(
	record,
	rindex,
	obj_par
)
char		 *record;
int		 *rindex;
SCObjEntry	 *obj_par;

#endif

{


	int             i;
	char           *word;
	int             no;
	SCT_SecMessMode SCT_sm;
	ICC_SecMessMode	SC_sm_list[NO_OF_SM];

	char           *proc = "get_obj_par";




	/*
	 * get object_name
	 */

	word = get_next_word_from_record(record, rindex);
	if ((word == CNULL) || (strlen(word) == 0)) {
		aux_add_error(ECONFIG, "PAR: No object name found in record", record, char_n, proc);
		return (ERR_flag);
	}
	obj_par->name = word;


	/*
	 * get object type (KEY, FILE)
	 */

	word = get_next_word_from_record(record, rindex);
	if ((word != CNULL ) && (strlen(word) != 0)) {
		if (!strncmp(word, SC_KEY_WORD, strlen(SC_KEY_WORD)))
			obj_par->type = SC_KEY_TYPE;
		else if (!strncmp(word, SC_FILE_WORD, strlen(SC_FILE_WORD)))
			obj_par->type = SC_FILE_TYPE;
		else {
			aux_add_error(ECONFIG, "PAR: Invalid type of object in record", record, char_n, proc);
			free(word);
			return (ERR_flag);
		}
	}
	 /* end if */ 
	else {
		aux_add_error(ECONFIG, "PAR: Type of object in record missing", record, char_n, proc);
		return (ERR_flag);
	}
	free(word);


	/*
	 * get id depending on type
	 */


	switch (obj_par->type) {

	case SC_KEY_TYPE:

		/*
		 * get key number
		 */

		word = get_next_word_from_record(record, rindex);
		if ((word != CNULL) && (strlen(word) != 0)) {
			no = char2int(word);
			if ((no < MIN_KEY_NUMBER) || (no > MAX_KEY_NUMBER)) {
				aux_add_error(ECONFIG, "PAR: Invalid key number in record ", record, char_n, proc);
			        free(word);
				return (ERR_flag);
			}
			obj_par->sc_id = no;


		} else {
			aux_add_error(ECONFIG, "PAR: Key number in record missing", record, char_n, proc);
			return (ERR_flag);
		}
		free(word);

		/* init parameters which are not used for a key_id */
		obj_par->size = 0;

		break;

	case SC_FILE_TYPE:

		/*
		 * get short index		 */

		word = get_next_word_from_record(record, rindex);
		if ((word != CNULL) && (strlen(word) != 0)) {
			no = char2int(word);
			if ((no < MIN_SHORT_INDEX) || (no > MAX_SHORT_INDEX)) {
				aux_add_error(ECONFIG, "PAR: Invalid short index for SC-file in record. ", record, char_n, proc);
			        free(word);
				return (ERR_flag);
			}
			obj_par->sc_id = no;
		} else {
			aux_add_error(ECONFIG, "PAR: Short index for file on SC missing", record, char_n, proc);
			return (ERR_flag);
		}
		free(word);


		/*
		 * get size = number of bytes to be reserved for WEF file on
		 * SC
		 */

		word = get_next_word_from_record(record, rindex);
		if ((word != CNULL) && (strlen(word) != 0)) {
			if ((no = char2int(word)) <= 0) {
				aux_add_error(ECONFIG, "PAR: Invalid no. of bytes for file on SC.", record, char_n, proc);
			        free(word);
				return (ERR_flag);
			}
			obj_par->size = no;
		}
		 /* end if */ 
		else {
			aux_add_error(ECONFIG, "PAR: File size in record missing", record, char_n, proc);
			return (ERR_flag);
		}
		free(word);

		break;
	default:
		break;

	}			/* end switch */



	/*
	 * Init secure messaging parameters with default value,
	 *    (secure messaging parameters are optional)
	 */

	obj_par->sm_SCT = SCT_SEC_NORMAL;
	obj_par->sm_SC_read.command = ICC_SEC_NORMAL;
	obj_par->sm_SC_read.response = ICC_SEC_NORMAL;
	obj_par->sm_SC_write.command = ICC_SEC_NORMAL;
	obj_par->sm_SC_write.response = ICC_SEC_NORMAL;

	for (i = 0; i < NO_OF_SM;)
		SC_sm_list[i++] = ICC_SEC_NORMAL;



	/*
	 * get secure messaging parameter for the communication DTE - SCT
         *     (first secure messaging parameter for object)
	 */

	word = get_next_word_from_record(record, rindex);

	if ((word != CNULL) && (strlen(word) != 0)) {
		if (!strncmp(word, NORM_WORD, strlen(NORM_WORD)))
			SC_sm_list[i++] = SCT_SEC_NORMAL;
		else if (!strncmp(word, INTG_WORD, strlen(INTG_WORD)))
			SC_sm_list[i++] = SCT_INTEGRITY;
		else if (!strncmp(word, CONC_WORD, strlen(CONC_WORD)))
			SC_sm_list[i++] = SCT_CONCEALED;
		else if (!strncmp(word, CONC_WORD, strlen(CONC_WORD)))
			SC_sm_list[i++] = SCT_COMBINED;
		else {
			aux_add_error(ECONFIG, "PAR: Invalid secure messaging parameter for object in record", record, char_n, proc);
		        free(word);
			return (ERR_flag);
		}
		free(word);
	}		/* end if */

	else
	 return(0);
		




	/*
	 * get secure messaging parameter for the communication SCT - SC
	 */

	i = 0;
	do {
		word = get_next_word_from_record(record, rindex);

		if ((word != CNULL) && (strlen(word) != 0)) {
			if (!strncmp(word, NORM_WORD, strlen(NORM_WORD)))
				SC_sm_list[i++] = ICC_SEC_NORMAL;
			else if (!strncmp(word, AUTH_WORD, strlen(AUTH_WORD)))
				SC_sm_list[i++] = ICC_AUTHENTIC;
			else if (!strncmp(word, CONC_WORD, strlen(CONC_WORD)))
				SC_sm_list[i++] = ICC_CONCEALED;
			else if (!strncmp(word, CONC_WORD, strlen(CONC_WORD)))
				SC_sm_list[i++] = ICC_COMBINED;
			else {
				aux_add_error(ECONFIG, "PAR: Invalid secure messaging parameter for object in record", record, char_n, proc);
			        free(word);
				return (ERR_flag);
			}
			free(word);
		}		/* end if */
	} while ((i < NO_OF_SM) && (word != CNULL));


	obj_par->sm_SC_read.command = SC_sm_list[0];
	obj_par->sm_SC_read.response = SC_sm_list[1];
	obj_par->sm_SC_write.command = SC_sm_list[2];
	obj_par->sm_SC_write.response = SC_sm_list[3];


	return(0);

}				/* get_obj_par */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_next_word_from_record			       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  This function returns the next word found in record from the*/
/*  offset record + *rindex.					*/
/*  The null-pointer is returned, if no word could be found.	*/
/*  The value of *rindex is incremented and points to the next  */
/*  character in record after the returned word.		*/
/*  Memory for the returned word is allocated by this function.	*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   record			Pointer to record.		*/
/*							       	*/
/* INOUT						       	*/
/*   rindex			Pointer to index in record.	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   NULL	         	No word found 		       	*/
/*   ptr. to word					       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   is_char_relevant()	       Check whether character is       */
/*			       relevant.			*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure get_next_word_from_record
 *
 ***************************************************************/
#ifdef __STDC__

static char *get_next_word_from_record(
	char	 *record,
	int	 *rindex
)

#else

static char *get_next_word_from_record(
	record,
	rindex
)
char	 *record;
int	 *rindex;

#endif

{

	char           *word;
	int             i;
	int             start_word;
	int             len_word;

	char           *proc = "get_next_word_from_record";

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  get_next_word_from_record\n\n");*/
#endif


	/*
	 * input check
	 */

	if ((record == CNULL) || (strlen(record) == 0) || (*rindex >= strlen(record))) {
		return (CNULL);
	}

	/*
	 * jump over all leading not relevant characters
	 */

	while ((*rindex < strlen(record)) && ((is_char_relevant(record[*rindex])) == FALSE)) {

		(*rindex)++;

	}


	if (*rindex >= strlen(record))
		return (CNULL);	/* no relevant character found */


	/*
	 * get relevant characters until first not relevant character
	 */

	start_word = *rindex;
	len_word = 0;

	while ((*rindex < strlen(record)) && ((is_char_relevant(record[*rindex])) == TRUE)) {

		(*rindex)++;

	}


	/*
	 * allocate memory for word, get word from record and return ptr to
	 * word.
	 */

	len_word = *rindex - start_word;


#ifdef CONFIGTEST
/*	fprintf(stderr, "in record: %s\n", record);
	fprintf(stderr, "strlen(record): %d\n", strlen(record));
	fprintf(stderr, "start_word: %d\n", start_word);
	fprintf(stderr, "len_word: %d\n", len_word);
	fprintf(stderr, "*rindex: %d\n", *rindex);*/

#endif



	if (len_word <= 0)
		return (CNULL);

	word = (char *) malloc(len_word + 1);
	if (!word) {
		aux_add_error(EMALLOC, "Word", CNULL, 0, proc);
		return (CNULL);
	}
	for (i = 0; i < len_word;)
		word[i++] = record[start_word++];
	word[i] = '\0';		/* terminate word with null */

#ifdef CONFIGTEST
/*	fprintf(stderr, "\nfound word: %s\n\n", word);*/

#endif


	return (word);

}				/* get_next_word_from_record */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  is_char_relevant				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Return TRUE if delivered character is  relevant else return */
/*  FALSE.							*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   one_char			One character.			*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE         	       Character is relevant.	       	*/
/*   FALSE         	       Character is not relevant.      	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure is_char_relevant
 *
 ***************************************************************/
#ifdef __STDC__

static Boolean is_char_relevant(
	char	  one_char
)

#else

static Boolean is_char_relevant(
	one_char
)
char	  one_char;

#endif

{

	char           *proc = "is_char_relevant";



	if (!one_char)
		return (FALSE);

	if ((one_char == BLANK_CHAR) ||
	    (one_char == COMMENT) ||
	    (one_char == COMMA) ||
	    (one_char == TAB) ||
	    (one_char == EQUAL) ||
	    (one_char == CR_CHAR) ||
	    (one_char == '\n') ||
	    (one_char == '\0'))
		return (FALSE);


	return (TRUE);

}				/* is_char_relevant */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  check_app_name					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check whether the actual read application name is unique in */
/*  "sc_app_list[]".			   	    		*/
/*								*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   read_app_name		application name		*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*   ERR_flag		       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure check_app_name
 *
 ***************************************************************/
#ifdef __STDC__

static int check_app_name(
	char	 *read_app_name
)

#else

static int check_app_name(
	read_app_name
)
char	 *read_app_name;

#endif

{

	char           *proc = "check_app_name";

#ifdef CONFIGTEST
	fprintf(stderr, "READ_CONFIG function:  check_app_name\n\n");
#endif


	/* check the given app_name */
	if (read_app_name) {
		if (strlen(read_app_name) > MAXL_APPNAME) {
			aux_add_error(ECONFIG, "APP: Application name too long", read_app_name, char_n, proc);
			return (ERR_flag);
		}
	} else {
		aux_add_error(EINVALID, "APP: Application name empty", CNULL, 0, proc);
		return (ERR_flag);
	}


	if (aux_AppName2SCApp(read_app_name)) {
		aux_add_error(ECONFIG, "APP: Application name not unique.", read_app_name, char_n, proc);
		return (ERR_flag);
	}
	return (0);

}				/* check_app_name */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  add_predef_values				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Add predefined values to the parameter list for one 	*/
/*  application.						*/
/*								*/
/*      - If the application identifier for the actual app. is 	*/
/*        missing, the default application id is added.		*/
/*	- All predefined objects are added to the object list.  */
/*        If a predefined object name or the belonging sc_id	*/
/*	  are already defined in the SCINITFILE, Err_flag is 	*/
/*	  returned.						*/
/*								*/		  
/*  Memory for all added values are allocated explicitly => it 	*/
/*  can be released with free().				*/
/*								*/
/*  The global list "predef_sc_obj_list[]" contains the list of */
/*  the predefined objects.					*/
/*  The global list "default_sc_obj_list[]" contains the list of*/
/*  the  defaults values for objects.				*/
/*  								*/
/*  If the number of objects is greater	than MAX_SCOBJ, ERR_flag*/
/*  is returned.				 		*/
/*  								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   sc_app_entry		One entry in SC application     */
/*                              list.				*/
/*   								*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*   ERR_flag		       Error			      	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_default_index()       Get index in default object list.*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure add_predef_values
 *
 ***************************************************************/
#ifdef __STDC__

int add_predef_values(
	SCAppEntry	 *sc_app_entry
)

#else

int add_predef_values(
	sc_app_entry
)
SCAppEntry	 *sc_app_entry;

#endif

{

	int             n, i;
	int             def_ind;
	Boolean         predef_obj_found;
	Boolean		SC_id_found;
	OctetString	appid_hex;
	int             function_rc = 0;
	char		err_msg[256];


	char           *proc = "add_predef_values";

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  add_predef_values\n\n");*/
#endif

	if (!sc_app_entry) {
		aux_add_error(EINVALID, "Invalid parameter value for sc_app_entry", CNULL, 0, proc);
		return (ERR_flag);
	}




	/*
	 *  If application id is missing, take default application id
	 */

	if ((!sc_app_entry->app_id) || (!sc_app_entry->app_id->octets)) {

		appid_hex.noctets = strlen(DEF_SC_APP_ID);
		appid_hex.octets  = DEF_SC_APP_ID;
		if (!(sc_app_entry->app_id = aux_dechex(&appid_hex))) {
			aux_add_error(ECONFIG, "Cannot decode hex representation of the default application id", CNULL, 0, proc);
			function_rc = ERR_flag;
		}
	}



	/*
	 * Loop for the list of the predefined objects:
	 */

	for (n = 0; predef_sc_obj_list[n]; n++) {

		def_ind = get_default_index(predef_sc_obj_list[n]);
		if ((def_ind < 0) || (!default_sc_obj_list[def_ind].name)) {
			aux_add_error(ECONFIG, "OBJ: Invalid default list for object ", predef_sc_obj_list[n], char_n, proc);
			return (ERR_flag);
		}

		predef_obj_found = FALSE;
		SC_id_found = FALSE;
		i = 0;
		while ((i <= MAX_SCOBJ) &&
		       (sc_app_entry->sc_obj_list[i].name) && 
			(predef_obj_found == FALSE) && (SC_id_found == FALSE)) {

			/*
			 *  Check uniqueness of object name
			 */

			if (strcmp(sc_app_entry->sc_obj_list[i].name, predef_sc_obj_list[n]) == 0) {
				aux_add_error(ECONFIG, "PAR: Object is a predefined object", predef_sc_obj_list[n], char_n, proc);
				predef_obj_found = TRUE;
			}
			else {

				/*
				 *  Check uniqueness of identifier on the SC
				 */

				if (( sc_app_entry->sc_obj_list[i].type == default_sc_obj_list[def_ind].type) &&
				    ( sc_app_entry->sc_obj_list[i].sc_id == default_sc_obj_list[def_ind].sc_id)) {

					sprintf(err_msg, "PAR: SC_id %d belongs to a predefined object!", sc_app_entry->sc_obj_list[i].sc_id);
					aux_add_error(ECONFIG, "Identifier not unique", err_msg, char_n, proc);
					SC_id_found = TRUE;
				}
				else
					i++;
				    
			} 

		}		/* end while */

		if ((predef_obj_found == TRUE) || (SC_id_found == TRUE)) {
			function_rc = ERR_flag;
		}
		else  {

			/*
			 * Object name and belonging SC-id (file, key) is unique
			 * = > Add predefined object to object list 
			 */

			if (i >= MAX_SCOBJ) {
				aux_add_error(ECONFIG, "OBJ: Too many objects for application ", sc_app_entry->app_name, char_n, proc);
				return (ERR_flag);
			}
			sc_app_entry->sc_obj_list[i] = default_sc_obj_list[def_ind];


			/*
			 *  Allocate memory for object name
			 */

	                sc_app_entry->sc_obj_list[i].name = (char *) malloc (strlen(default_sc_obj_list[def_ind].name));
			if (!sc_app_entry->sc_obj_list[i].name) {
				aux_add_error(EMALLOC, "object name", CNULL, 0, proc);
				return (ERR_flag);
			}
			strcpy (sc_app_entry->sc_obj_list[i].name, default_sc_obj_list[def_ind].name);

			i++;
			sc_app_entry->sc_obj_list[i].name = CNULL;

		}		/* end else */
	}			/* end for */

	return (function_rc);


}				/* add_predef_values */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  handle_mandatory_obj				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check whether object list contains all mandatory objects    */
/*  ("man_sc_obj_list[]"). If a mandatory object is missing, the*/
/*  default values for this object is added to the current 	*/
/*  object list ( in this case memory for the object name is	*/
/*  allocated explicitly.					*/
/*								*/
/*  The global list "man_sc_obj_list[]" contains the list of the*/
/*  mandatory objects.						*/
/*  The global list "default_sc_obj_list[]" contains the list of*/
/*  the  defaults values for objects.				*/
/*  								*/
/*  If the number of objects is greater	than MAX_SCOBJ, ERR_flag*/
/*  is returned.				 		*/
/*  								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   sc_app_entry		One entry in SC application     */
/*                              list.				*/
/*   								*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*   ERR_flag		       Error			      	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_default_index()       Get index in default object list.*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure handle_mandatory_obj
 *
 ***************************************************************/
#ifdef __STDC__

int handle_mandatory_obj(
	SCAppEntry	 *sc_app_entry
)

#else

int handle_mandatory_obj(
	sc_app_entry
)
SCAppEntry	 *sc_app_entry;

#endif

{

	int             n, i;
	int             def_ind;
	Boolean         man_obj_found;

	char           *proc = "handle_mandatory_obj";

#ifdef CONFIGTEST
/*	   fprintf(stderr, "READ_CONFIG function:  handle_mandatory_obj\n\n");*/
#endif

	if (!sc_app_entry) {
		aux_add_error(ECONFIG, "Invalid parameter value for sc_app_entry", CNULL, 0, proc);
		return (ERR_flag);
	}



	/*
	 * Loop for the list of the mandatory objects:
	 */

	for (n = 0; man_sc_obj_list[n]; n++) {

		man_obj_found = FALSE;
		i = 0;
		while ((i <= MAX_SCOBJ) &&
		       (sc_app_entry->sc_obj_list[i].name) && (man_obj_found == FALSE)) {

			if (strcmp(sc_app_entry->sc_obj_list[i].name, man_sc_obj_list[n]) == 0)
				man_obj_found = TRUE;
			else
				i++;
		}		/* end while */


		if (man_obj_found == FALSE) {

			/*
			 * Mandatory object missing in object list => add
			 * mandatory object.
			 */

			if (i >= MAX_SCOBJ) {
				aux_add_error(ECONFIG, "OBJ: Too many objects for application ", sc_app_entry->app_name, char_n, proc);
				return (ERR_flag);
			}
			def_ind = get_default_index(man_sc_obj_list[n]);
			if ((def_ind < 0) || (!default_sc_obj_list[def_ind].name)) {
				aux_add_error(ECONFIG, "OBJ: Invalid default list for object ", man_sc_obj_list[n], char_n, proc);
				return (ERR_flag);
			}
			sc_app_entry->sc_obj_list[i] = default_sc_obj_list[def_ind];


			/*
			 *  Allocate memory for object name
			 *  => free(obj_name) causes no error
			 */

	                sc_app_entry->sc_obj_list[i].name = (char *) malloc (strlen(default_sc_obj_list[def_ind].name));
			if (!sc_app_entry->sc_obj_list[i].name) {
				aux_add_error(EMALLOC, "object name", CNULL, 0, proc);
				return (ERR_flag);
			}
			strcpy (sc_app_entry->sc_obj_list[i].name, default_sc_obj_list[def_ind].name);

			i++;
			sc_app_entry->sc_obj_list[i].name = CNULL;

		}		/* end if */
	}			/* end for */

	return (0);


}				/* handle_mandatory_obj */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_default_index				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Get index in default object list ("sc_obj_list[]") for the  */
/*  delivered object name.					*/
/*  								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   object_name		Object name.			*/
/*   								*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   index (>= 0)	       Index in default object list.  	*/
 /*   ERR_flag		       No entry found.			*//* */
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure get_default_index
 *
 ***************************************************************/
#ifdef __STDC__

int get_default_index(
	char	 *object_name
)

#else

int get_default_index(
	object_name
)
char	 *object_name;

#endif

{
	int             n;
	char           *proc = "get_default_index";

	if (!object_name) {
		aux_add_error(EINVALID, "OBJ: Invalid value for parameter 'object_name' ", CNULL, 0, proc);
		return (ERR_flag);
	}
	for (n = 0; default_sc_obj_list[n].name; n++) {
		if (strcmp(default_sc_obj_list[n].name, object_name) == 0)
			return (n);
	}

	return (ERR_flag);


}				/* end get_default_index */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  check_SCapp_configuration			       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check consistency of configuration data for an SC-		*/
/*  application.	    					*/
/*								*/
/*								*/
/*  1) Perform SC configuration (read SC configuration file 	*/
/*     (".scinit")).       					*/
/*								*/
/*  2) Check whether given application (app_name) is an 	*/
/*     SC-application.   					*/
/*     								*/
/*     Case 1:  Application is an SC-application:		*/
/*	        Depending on the value of the parameter 	*/
/*	        "onekeypair" it is checked whether the 		*/
/*		configuration file contains the necessary	*/
/*		objects for this kind of application.		*/
/*								*/
/*     Case 2: Application is not an SC-application:		*/
/*             => return error					*/
/*  								*/
/*  3) Depending on parameter "onekeypair" it is checked,	*/
/*     a) whether the size of "SCToc" is sufficient for the     */
/*        relevant objects in the objectlist for the given 	*/
/*        application.						*/
/*							        */
/*     b) whether the free space on the SC is sufficient for 	*/
/*        the relevant objects in the objectlist for the given 	*/
/*        application.						*/
/*							        */
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name		       Name of the application for which*/
/*			       the configuration data shall be  */
/*			       checked.				*/
/*   onekeypair		       TRUE means that the application  */
/*			            works with one RSA keypair.	*/
/*			       FALSE means that the application */
/*			             works with two RSA 	*/
/*				     keypairs.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0		  	       Configuration data for the given */
/*			       application are consistent.	*/
/*   -1		               - Necessary objects missing in	*/
/*			         configuration data.		*/
/*   		               - error (e.g. application not an */
/*			         SC-application)		*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   check_config_list()	Check whether the object list of*/
/*				an SC-application contains all  */
/*				mandatory objects.		*/
/*   is_obj_in_list()	        Check whether given object is   */
/*			        part of given list.	        */ 
/*   aux_AppName2SCApp()	Get information about an SC app.*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure check_SCapp_configuration
 *
 ***************************************************************/
#ifdef __STDC__

RC check_SCapp_configuration(
	char	 *app_name,
	Boolean	  onekeypair
)

#else

RC check_SCapp_configuration(
	app_name,
	onekeypair
)
char	 *app_name;
Boolean	  onekeypair;

#endif

{

#define SC_OBJ_LIST	sc_app_entry->sc_obj_list
#define ONEKEY_LIST	onekeypair_sc_obj_list
#define TWOKEY_LIST	twokeypairs_sc_obj_list

	int		SC_available;
	SCAppEntry     *sc_app_entry;
	SCObjEntry     *sc_obj_entry;
	Boolean		data_consistent;
	unsigned int	i;
	Boolean		obj_relevant;
	unsigned int	no_of_entries;
	unsigned int	no_of_units;
	unsigned int 	rest;
	char		err_msg[256];
	unsigned int	req_Toc_size;





	char            *proc = "check_SCapp_configuration";



#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	fprintf(stderr, "                   Application-Name: %s\n", app_name);
	if (onekeypair == TRUE)
		fprintf(stderr, "                with onekeypair\n");
	else	fprintf(stderr, "                with twokeypairs\n");

#endif


	if (app_name == CNULL) {
		aux_add_error(EINVALID, "No application name specified", CNULL, 0, proc);
		return (-1);
	}


	/*
	 *  Perform SC configuration
	 */

	if ((SC_available = SC_configuration()) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (SC_available == FALSE) {

		/*
		 *  no SC configuration file found)
		 */

		aux_add_error(ECONFIG, "No SC configuration file found.", CNULL, 0, proc);
		return (-1);
	}



	/*
	 *  Check whether application is an SC-application
	 */

	sc_app_entry = aux_AppName2SCApp(app_name);
	if (sc_app_entry == (SCAppEntry * ) 0) 	{
		aux_add_error(EINVALID, "Application is not an SC-application", app_name, char_n, proc);
		return (-1);
	}


	/*
	 *  Check consistency of the configuration data for the given application.
	 */

	if (onekeypair == TRUE) 
		data_consistent = check_config_list(sc_app_entry, onekeypair_sc_obj_list);
	else 	data_consistent = check_config_list(sc_app_entry, twokeypairs_sc_obj_list);


	if (data_consistent == -1) {
		aux_add_error(ECONFIG, "Inconsistent configuration data for SC-application ", app_name, char_n, proc);	
		return (-1);
	}

	
	/*
	 *  Depending on parameter "onekeypair" the relevant objects are counted
	 *  and the units, these objects will need on the SC, are accumulated.
	 */

	i = 0;
	no_of_entries = 0;
	no_of_units   = 0;

	while ((i <= MAX_SCOBJ) && (SC_OBJ_LIST[i].name)) {

		/*
		 *  First check whether object is relevant
		 */
		
		obj_relevant = TRUE;
#ifdef SECSCTEST
		fprintf(stderr, "SC_OBJ_LIST[i].name: %s\n", SC_OBJ_LIST[i].name);
#endif

		if (is_obj_in_list(SC_OBJ_LIST[i].name, predef_sc_obj_list) == TRUE) {

			/*
			 *  Predefined objects as SC_PIN, SC_PUK, PSE_PIN, the
			 *    device key set don't count
			 */

			obj_relevant = FALSE;
		}
		else {
			if (onekeypair == TRUE) {
		        	if ((is_obj_in_list(SC_OBJ_LIST[i].name, ONEKEY_LIST) == FALSE) &&
		                    (is_obj_in_list(SC_OBJ_LIST[i].name, TWOKEY_LIST) == TRUE)) 
					/*
					 *  Object not relevant, 
					 *    - is only needed for an SC with two key pairs
					 */

					obj_relevant = FALSE;
			}
			else {
			    	if ((is_obj_in_list(SC_OBJ_LIST[i].name, TWOKEY_LIST) == FALSE) &&
			            (is_obj_in_list(SC_OBJ_LIST[i].name, ONEKEY_LIST) == TRUE)) 
					/*
					 *  Object not relevant, 
					 *    - is only needed for an SC with one key pair
					 */

					obj_relevant = FALSE;
			}

		}
		
		
		if (obj_relevant == TRUE) {

			/*
			 *  Object is relevant
			 */
#ifdef SECSCTEST
			fprintf(stderr, "Relevant SC_OBJ_LIST[i].name: %s\n", SC_OBJ_LIST[i].name);
#endif
			if (strcmp(SC_OBJ_LIST[i].name, SCToc_name) != 0)
				no_of_entries++;

			if (SC_OBJ_LIST[i].type == SC_FILE_TYPE) {
				rest = SC_OBJ_LIST[i].size % UNIT_SIZE;
				if (rest > 0)
					no_of_units += SC_OBJ_LIST[i].size / UNIT_SIZE + 1;
				else
					no_of_units += SC_OBJ_LIST[i].size / UNIT_SIZE;

#ifdef SECSCTEST
				fprintf(stderr, "SC_OBJ_LIST[i].size: %d\n", SC_OBJ_LIST[i].size);
				fprintf(stderr, "rest:                %d\n", rest);
				fprintf(stderr, "no_of_units:         %d\n", no_of_units);
#endif
			}

		}
		i++;
	}


	/*  
	 *     Compare the number of objects with the size of the object "SCToc". If
	 *     the given size of "SCToc" is too small, an error will be returned.
	 *     Compare the computed units with the free space on the SC. If the
	 *     free space on the SC is too small an error is returned.
	 */


	sc_obj_entry = aux_AppObjName2SCObj(app_name, SCToc_name);
	if (sc_obj_entry == NULLSCOBJENTRY) {
		aux_add_error(ECONFIG, "Missing SCToc entry for SC_application ", app_name, char_n, proc);
		return (-1);
	}

	/* no. of entries plus header of Toc */
	req_Toc_size = (no_of_entries + 1) * MAX_TOC_ENTRY;

#ifdef SECSCTEST

	fprintf(stderr, "                   No of Toc Entries:      %d\n", no_of_entries);
	fprintf(stderr, "                   Required space for Toc: %d\n", req_Toc_size);
	fprintf(stderr, "                   Size of Toc:            %d\n", sc_obj_entry->size);
#endif

	if (req_Toc_size > sc_obj_entry->size) {
		sprintf(err_msg, "(Required size of SCToc for application %s is %d)", app_name, req_Toc_size);
		aux_add_error(ECONFIG, "Size of SCToc is too small.", (char *) err_msg, char_n, proc);
		return (-1);
	}
			

	/*???????????????????????????????????????????????????????*/
	/* je nachdem, ob SECUDE das Schluesselfile selbst anlegt, oder
	   von STARMAC ein Schluesselfile fuer 3 Schluessel angelegt wird
	   koennte hier noch unterschieden werden, ob einarmige SC oder
	   2 armige. Eine einarmige hat ein kleineres Schluesselfile, somit
	   mehr Platz fuer Rest.
	*/

	/*???????????????????????????????????????????????????????*/
	/*
	 *  Each WEF will take one additional unit
	 */

/*	no_of_units += no_of_entries;  */
	/*???????????????????????????????????????????????????????*/
#ifdef SECSCTEST
	fprintf(stderr, "                   Required number of units:   %d\n", no_of_units);
	fprintf(stderr, "                   Number of free units on SC: %d\n", MAX_SC_SPACE);
#endif

	if (no_of_units > MAX_SC_SPACE) {
		sprintf(err_msg, "(Available space for application %s is %d, required size is %d)", app_name, MAX_SC_SPACE * UNIT_SIZE, no_of_units * UNIT_SIZE);
		aux_add_error(ECONFIG, "Not enough space on SC!", (char *) err_msg, char_n, proc);
		return (-1);
	}





	return (0);


}				/* end check_SCapp_configuration */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  check_config_list				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check whether a given object list of an SC-application 	*/
/*  contains all mandatory objects.				*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   sc_app_entry		One entry in SC application     */
/*                              list.				*/
/*   man_obj_list		List of the mandatory objects.  */
/*   								*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0		         	All mandatory objects found in  */
/*				given "sc_app_entry".		*/
/*   -1			      	Objects missing.	      	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure check_config_list
 *
 ***************************************************************/
#ifdef __STDC__

int check_config_list(
	SCAppEntry	 *sc_app_entry,
	char		 *man_obj_list[]
)

#else
int check_config_list(
	sc_app_entry,
	man_obj_list
)
SCAppEntry	 *sc_app_entry;
char		 *man_obj_list[];

#endif

{

	int             n, i;
	Boolean         man_obj_found;

	char           *proc = "check_config_list";


	if (!sc_app_entry) {
		aux_add_error(EINVALID, "Invalid parameter value for sc_app_entry", CNULL, 0, proc);
		return (-1);
	}

	/*
	 * Loop for the list of the mandatory objects:
	 */

	for (n = 0; man_obj_list[n]; n++) {

		man_obj_found = FALSE;
		i = 0;
		while ((i <= MAX_SCOBJ) &&
		       (sc_app_entry->sc_obj_list[i].name) && (man_obj_found == FALSE)) {

			if (strcmp(sc_app_entry->sc_obj_list[i].name, man_obj_list[n]) == 0)
				man_obj_found = TRUE;
			else
				i++;
		}		/* end while */


		if (man_obj_found == FALSE) {

			/*
			 * Mandatory object missing.
			 */

			aux_add_error(ECONFIG, "Mandatory SC-object missing in configuration file", man_obj_list[n], char_n,  proc);
			return (-1);

		}		
	}			

	return (0);


}				/* check_config_list */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  is_obj_in_list				       		*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check whether a given object is part of a given list. 	*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   obj_name			Name of object.			*/
/*   obj_list			List of objects.		*/
/*   								*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0		         	All mandatory objects found in  */
/*				given "sc_app_entry".		*/
/*   -1			      	Objects missing.	      	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure is_obj_in_list
 *
 ***************************************************************/
#ifdef __STDC__
int is_obj_in_list(
	char	*obj_name,
	char	*obj_list[]
)

#else
int is_obj_in_list(
	obj_name,
	obj_list
)
char	 *obj_name;
char	 *obj_list[];

#endif

{

	int             i;
	Boolean         obj_found;

	char           *proc = "is_obj_in_list";


	if (!obj_list) {
		aux_add_error(EINVALID, "Invalid parameter value for obj_list", CNULL, 0, proc);
		return (-1);
	}


	obj_found = FALSE;
	i = 0;
	while ((obj_list[i]) && (obj_found == FALSE)) {

		if (strcmp(obj_name, obj_list[i]) == 0)
			obj_found = TRUE;
		else
			i++;
	}		

	return(obj_found);


}				/* is_obj_in_list */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  check_obj_info					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Check new object for an application.			*/
/*  (e.g. compare with already entered object for the actual    */
/*   application).						*/
/*								*/
/*								*/
/* IN			     DESCRIPTION		       	*/
/*   sc_app_entry		Entry in sc_app_list for the    */
/*                              actual application.		*/
/*   new_obj_par		Parameter for the new object.	*/
/*   								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*   ERR_flag		       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure check_obj_info
 *
 ***************************************************************/
#ifdef __STDC__

int check_obj_info(
	SCAppEntry	 *sc_app_entry,
	SCObjEntry	 *new_obj_par
)

#else

int check_obj_info(
	sc_app_entry,
	new_obj_par
)
SCAppEntry	 *sc_app_entry;
SCObjEntry	 *new_obj_par;

#endif

{

	char           *proc = "check_obj_info";

#ifdef CONFIGTEST
	fprintf(stderr, "READ_CONFIG function:  check_obj_info\n\n");
#endif


	/* check input parameters */
	if ((!sc_app_entry) || (!sc_app_entry->app_name) ||
	    (!new_obj_par) || (!new_obj_par->name)) {
		aux_add_error(ECONFIG, "PAR: Invalid input values for check_obj_info", CNULL, 0, proc);
		return (ERR_flag);
	}
	if (aux_AppObjName2SCObj(sc_app_entry->app_name, new_obj_par->name)) {
		aux_add_error(ECONFIG, "PAR: Object name not unique", new_obj_par->name, char_n, proc);
		return (ERR_flag);
	}
	return (0);

}				/* check_obj_info */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  char2int					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform an character string (ASCII) into an integer value */
/*  and returns this integer value.			 	*/
/*								*/
/*  If a character in string is not a digit, -1 is returned.	*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   s 	   		       Pointer to char_string to be	*/
/*			       transformed.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*    -1		       Error			        */
/*   int_value	 	       Integer value.			*/
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure char2int
 *
 ***************************************************************/
#ifdef __STDC__

static int char2int(
	char	 *s
)

#else

static int char2int(
	s
)
char	 *s;

#endif

{

	unsigned int    n;

	char           *proc = "char2int";

#ifdef CONFIGTEST
	fprintf(stderr, "\nATOI: string: %s", s);
#endif


	if (!s) {
		aux_add_error(ECONFIG, "PAR: ASCII string = NULL", CNULL, 0, proc);
		return (-1);
	}
	n = 0;

	while (*s) {
		if ((*s < '0') || (*s > '9')) {
			aux_add_error(ECONFIG, "PAR: ASCII character is not a digit", CNULL, 0, proc);
			return (-1);
		}
		n = 10 * n + *s - '0';
		s++;
	}

#ifdef CONFIGTEST
	fprintf(stderr, ", integer: %d\n", n);
#endif
	return (n);
}				/* end char2int */


#endif				
