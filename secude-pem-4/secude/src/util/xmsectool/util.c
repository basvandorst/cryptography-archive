/*
 *  SecuDE Release 4.2 (GMD)
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

/*-----------------------util.c-------------------------------------*/
/*                                                                  */
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)                  */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990 / "SecuDE" 1991/92/93                */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer 	                    */
/* Luehe/Surkau/Reichelt/Kolletzki		                    */
/*------------------------------------------------------------------*/
/* PACKAGE   util            VERSION   4.2                          */
/*                              DATE   01.11.1993                   */
/*                                BY   ws                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PROGRAM   xmst            VERSION   1.0                          */
/*                              DATE   01.11.1993                   */
/*                                BY   Kolletzki                    */
/*                                                                  */
/*                            REVIEW                                */
/*                              DATE                                */
/*                                BY                                */
/*                                                                  */
/* MODULE    util                                                   */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	sectool routines for gui stuff                              */
/*                                                                  */
/*------------------------------------------------------------------*/



/*
 ************************
 *	INCLUDES	*
 ************************
 */

#include "xmst.h"



/*
 ************************
 *	STATICS		*
 ************************
 */

static Boolean		debug_input_handler_installed = FALSE;
static char		value_string[256];

static void		_init_clientdata();
static Boolean		get_toggle_resource();
static char		*get_textfield_resource();
static void		put_toggle_resource();
static void		put_textfield_resource();

/*
 *	Leave sectool in a proper manner after an error occured
 */
void
exit_sectool()
{

	at_save_aliaslist();

	remove_tempfile();
	strcat(editor_tempfile, "%");
	remove_tempfile();
	
	if (XmToggleButtonGetState(STWIDGET("*config_autosave_toggle"))) XrmPutFileDatabase(config_database, config_rcfile);

	fprintf(stderr, "Bye bye...\n");

	exit(0);

}


/*
 *	Call XtNameToWidget for specified uil name, exit if not found
 */
Widget
name2widget(parentShell, s)
	Widget		parentShell;
	char		*s;
{
	Widget		widget = XtNameToWidget(parentShell, s);

	if (!widget) {

		fprintf(stderr, "XMST ERROR in XtNameToWidget(\"%s\")\n", s);

		exit_sectool();
	}

	return(widget);
}


/*
 *	LIST CLIENT DATA type
 *	This is my quick & dirty solution for XmList client data substitute.
 *	Hope there will be such an enhancement in the standard Motif lib in future...
 */


/*
 *	Init all client data vectors
 */
void
init_all_clientdata()
{

	_init_clientdata(pselist_clientdata);
	_init_clientdata(pklist_clientdata);
	_init_clientdata(eklist_clientdata);
	_init_clientdata(aliaslist_clientdata);

}


/*
 *	Init specified client data vector
 */
static
void
_init_clientdata(cd_vector)
	XtPointer	cd_vector[];
{
	int		entry;
	
	
	cd_vector[0] = (XtPointer)1;
	
	for (entry = 1; entry < XMST_MAX_LIST_ITEMS; entry++)
		cd_vector[entry] = (XtPointer)NULL;

}


/*
 *	Free & Init specified client data vector
 */
void
init_clientdata(cd_vector)
	XtPointer	cd_vector[];
{
	int		entry;
	
	
	cd_vector[0] = (XtPointer)1;
	
	for (entry = 1; entry < XMST_MAX_LIST_ITEMS; entry++) {

		if (cd_vector[entry]) free(cd_vector[entry]);
		cd_vector[entry] = (XtPointer)NULL;
	}
}


/*
 *	Insert client data at position in vector
 */
Boolean
insert_clientdata(cd_vector, cd, position)
	XtPointer	cd_vector[];
	XtPointer	cd;
	int		position;
{
	int		entry;

		
	if (count_clientdata(cd_vector) >= XMST_MAX_LIST_ITEMS) {

		fprintf(stderr, "XMST WARNING: insert_clientdata() client data vector overflow\n");

		return(FALSE);
	}

	for (entry = count_clientdata(cd_vector); entry >= position; entry--)
		cd_vector[entry + 1] = cd_vector[entry];

	entry++;
	cd_vector[entry] = cd;

	cd_vector[0]++;


	return(TRUE);

}


/*
 *	Append client data to vector
 */
Boolean
append_clientdata(cd_vector, cd)
	XtPointer	cd_vector[];
	XtPointer	cd;
{
	Boolean		rvalue;

		
	rvalue = insert_clientdata(cd_vector, cd, (int)cd_vector[0]);


	return(rvalue);

}


/*
 *	Delete client data at position in vector
 */
void
delete_clientdata(cd_vector, position)
	XtPointer	cd_vector[];
	int		position;
{
	int		entry;

		

	for (entry = position; entry < count_clientdata(cd_vector); entry++)
		cd_vector[entry] = cd_vector[entry + 1];

	cd_vector[entry] = (XtPointer)NULL;

	cd_vector[0]--;

}


/*
 *	Get number of client data entries in vector
 */
int
count_clientdata(cd_vector)
	XtPointer	cd_vector[];
{

	return( (int)cd_vector[0] - 1 );

}


/*
 *	Get client data at position in vector
 */
XtPointer
get_clientdata(cd_vector, position)
	XtPointer	cd_vector[];
	int		position;
{

	return( cd_vector[position] );

}


/*
 *	Transform Serialnumber to (char *) according to aux_fprint_Serialnumber() formatting;
 *	i.e. values with size of int are formatted to decimal integers, others to hex octets
 */
char *
Serialnumber2String(serial)
	OctetString		*serial;
{
	char			*proc = "Serialnumber2String";
	char			rstring[XMST_PKSERIAL_LENGTH];
	int	 		int_repr;
	L_NUMBER 		lnum[8];
	OctetString		*ostr;
	int			noc;
	char			*fill = "", *pfill = ".", *ppfill = " .. ";


	if (sectool_verbose) fprintf(stderr, "%s\n", proc);	

	if (!serial || !serial->octets) return(CNULL);

	strcpy(rstring, "");

	if (serial->noctets <= sizeof(int)) {

		octetstoln(serial, lnum, 0, serial->noctets);
		int_repr =  lnum[1];
		if (int_repr < 0) {

			if (verbose) fprintf(stderr, "Sectool: Problem with conversion of Serialnumber\n");

			return(CNULL);
		}
		sprintf(rstring, "%5d", int_repr);
		sprintf(rstring, "%-5.5s (decimal)", rstring);

	} else {

		ostr = aux_enchex(serial);
		if (!ostr || !ostr->octets) return(CNULL);

		if (ostr->noctets <= XMST_PKSERIAL_NOCTETS) {

			for (noc = 0; noc < ostr->noctets; noc+=2)
				sprintf(rstring, 	"%s%-2.2s%1s",
							rstring,
							ostr->octets[noc],
							pfill			);
			rstring[noc] = '\0';

		} else {

			sprintf(rstring, 	"%-2.2s%1s%-2.2s%4s%-2.2s%1s%-2.2s",
						ostr->octets,
						pfill,
						ostr->octets + 2,
						ppfill,
						ostr->octets + ostr->noctets - 4,
						pfill,
						ostr->octets + ostr->noctets - 2	);
		}

		aux_free_OctetString(&ostr);
	}

	return( aux_cpy_String(rstring) );
}



/* 
 *	Open temporary text file
 */
int
open_write_tempfile(parentShell)
Widget			parentShell;
{
	char		*proc = "open_write_tempfile";


	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc);

	remove_tempfile();
	logfile = fopen(editor_tempfile, "w");
	if (logfile == (FILE *)NULL)  {

		sprintf(dialog_message, "Can't open temporary file\n\"%s\"", editor_tempfile);
		if (verbose) fprintf(stderr, "Can't open temporary file %s.", editor_tempfile);
		cont_quit_dialog_open(parentShell, dialog_message);

		return(-1);
	}

	return(0);
}




/*
 *	Close temporary text file
 */
void
close_tempfile()
{
	char		*proc = "close_tempfile";


	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc);


	fclose(logfile);
	logfile = (FILE *)NULL;

	chmod(editor_tempfile, S_IRWXU & S_IRUSR);	/* Read permission for user only */
}


/*
 *	Remove temporary text file
 */
void
remove_tempfile()
{
	char		*proc = "remove_tempfile";


	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc);

	unlink(editor_tempfile);
}

/*
 *	Start external editor if resource is set
 */
void
start_editor()
{
	char		command_string[256];
	OctetString	*file_ostr;
	char		*file_string;
		


	if (external_editor && !XtIsManaged(STWIDGET("*viewer_dialog"))) {

		sprintf(command_string, "%s %s &", external_editor, editor_tempfile);
		system(command_string);

	} else {
		
		if (! (file_ostr = aux_file2OctetString(editor_tempfile)) ) {
		
			ok_dialog_open(applicationShell, "Can't read temporary file.");
			
		} else {
		
			if (file_string = CATNSTR(CNULL, file_ostr->octets, file_ostr->noctets)) {
			
				XmTextSetString(STWIDGET("*viewer_text"), file_string);
				free(file_string);
				XMapRaised(display, XtWindow(applicationShell));
				XtManageChild(STWIDGET("*viewer_dialog"));
			}
			aux_free_OctetString(&file_ostr);
		}
	}
}







Boolean
debug_workproc(debug_text)
	Widget				debug_text;
{
	static struct ErrStack  	*err = NULL;
	static struct ErrStack  	*prev_err = NULL;
	static int			last_position = 0;
	static int			group = 1;
	char				text[256];


	if (!err_stack) return( debugmode_on ? FALSE : TRUE );			/* no errors at all */
	if (err == err_stack) return( debugmode_on ? FALSE : TRUE );		/* no new error on stack */
	if (err_stack == &err_malloc) return( debugmode_on ? FALSE : TRUE ); 	/* malloc failed already, can't do much */

	sprintf(text, "[%d]\n", group++);
	XmTextInsert(debug_text, last_position, text);
	last_position = XmTextGetLastPosition(debug_text);
		
	for (prev_err = err, err = err_stack; err && err != prev_err; err = err->next) {
	
		sprintf(text, "%s in %s: %s (%d)\n", err->e_is_error ? "ERROR" : "WARNING",
			err->e_proc, err->e_text, err->e_number);
	
		XmTextInsert(debug_text, last_position, text);
	}
	err = err_stack;

	XmTextShowPosition(debug_text, (last_position = XmTextGetLastPosition(debug_text)));

	XtManageChild(STWIDGET("*debug_dialog"));


	return( debugmode_on ? FALSE : TRUE );
}





/*
 *	Callback for password textfields (shows '*' for each character)
 *	Password is stored in static char *passwd
 */
void
check_passwd_cb(text_w, unused, cbs)
	Widget        			text_w;
	XtPointer     			unused;
	XmTextVerifyCallbackStruct 	*cbs;
{
    	char 				*new;
    	int 				len;


    	if (cbs->reason == XmCR_ACTIVATE) {

	       	return;
    	}

    	if (cbs->text->ptr == (char)NULL) { 					/* backspace */

        	cbs->endPos = strlen(passwd); 				/* delete from here to end */
        	passwd[cbs->startPos] = 0; 				/* backspace--terminate */

        	return;
    	}

    	if (cbs->text->length > 1) {

        	cbs->doit = False; 					/* don't allow "paste" operations */

        	return; 						/* make the user *type* the password! */
    	}

    	new = (char *)malloc(cbs->endPos + 2); 				/* new char + NULL terminator */

    	if (passwd) {

        	strcpy(new, passwd);
        	free(passwd);

    	} else new[0] = (char)NULL;

    	passwd = new;
    	strncat(passwd, cbs->text->ptr, cbs->text->length);
    	passwd[cbs->endPos + cbs->text->length] = 0;

    	for (len = 0; len < cbs->text->length; len++)
        	cbs->text->ptr[len] = '*';
}


/*
 *	Callback for textfields to change color depending on focus possession
 */
void
textfield_focus_cb(text_w, unused, cbs)
	Widget        			text_w;
	XtPointer     			unused;
	XmTextVerifyCallbackStruct 	*cbs;
{
    	Widget 				parent;
    	Pixel	 			parent_color;


	if (focus_color) {

	    	switch (cbs->reason) {
	    	
	    		case XmCR_FOCUS:
	    		
	    			XtSetArg(args[0], XmNbackground, focus_color);
	    			XtSetValues(text_w, args, 1);
	
		       		break;
	
		       	case XmCR_LOSING_FOCUS:
		       	
	    			XtSetArg(args[0], XmNbackground, &parent_color);
	    			XtGetValues(XtParent(text_w), args, 1);
	
	    			XtSetArg(args[0], XmNbackground, parent_color);
	    			XtSetValues(text_w, args, 1);
	
		       	      	break;
		       	      	
			default:
		 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
		}
	}	
	


}



/* 
 *	Set property widgetss confirming to configuration
 */
void
set_property_widgets()
{

	XmToggleButtonSetState(STWIDGET("*st_debug_toggle"), Debug ? TRUE : FALSE, FALSE);

#ifdef X500
	XtManageChild(STWIDGET("*st_authentication_button"));
	
	XmToggleButtonSetState(STWIDGET("*st_auth_none_toggle"),
		(af_dir_authlevel == DBA_AUTH_NONE) ? TRUE : FALSE, FALSE);
	XmToggleButtonSetState(STWIDGET("*st_auth_simple_toggle"),
		(af_dir_authlevel == DBA_AUTH_SIMPLE) ? TRUE : FALSE, FALSE);
	XmToggleButtonSetState(STWIDGET("*st_auth_strong_toggle"),
		(af_dir_authlevel == DBA_AUTH_STRONG) ? TRUE : FALSE, FALSE);

#endif
	
#ifdef SCA
	XtManageChild(STWIDGET("*st_crypt_toggle"));

	if (sec_scttest() != SCTDev_avail) {

		XtSetSensitive(STWIDGET("*st_crypt_toggle"), FALSE);
		XmToggleButtonSetState(STWIDGET("*st_crypt_toggle"), FALSE, FALSE);
		SC_verify = SC_encrypt = FALSE;
		
	} else if (auto_detect_sct) {			/* sct is available: use for rsa */
		
		XmToggleButtonSetState(STWIDGET("*st_crypt_toggle"), TRUE, FALSE);
		SC_verify = SC_encrypt = TRUE;
		
	} else {					/* force software rsa despite sct is available */

		XmToggleButtonSetState(STWIDGET("*st_crypt_toggle"), FALSE, FALSE);
		SC_verify = SC_encrypt = FALSE;	
	}
#endif /* SCA */

}


/* 
 *	Set Nice Level, SCT Crypt Toggle, Debug Mode Toggle
 */
void
set_properties()
{
	char		*proc = "set_properties";
	Widget		widget;


	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc);

	/* Show options */

        print_keyinfo_flag = ALGID;
        print_cert_flag = TBS | ALG | SIGNAT;
	opt = 0;

	if ( XmToggleButtonGetState(widget = STWIDGET("*st_nice_low_toggle")) ) {

		af_verbose = FALSE;
		if (sec_debug > 0) sec_verbose = TRUE;		/* Grand Verbose flag -W set */
		else sec_verbose = FALSE;

		initialNiceToggle = widget;
	}
	if ( XmToggleButtonGetState(widget = STWIDGET("*st_nice_medium_toggle")) ) {

		af_verbose = TRUE;
		if (sec_debug > 0) sec_verbose = TRUE;		/* Grand Verbose flag -W set */
		else sec_verbose = FALSE;

		print_keyinfo_flag |= BSTR;
		print_keyinfo_flag |= KEYBITS;

		initialNiceToggle = widget;
	}
	if ( XmToggleButtonGetState(widget = STWIDGET("*st_nice_high_toggle")) ) {

		af_verbose = TRUE;
		sec_verbose = TRUE;

		print_keyinfo_flag |= BSTR;
		print_keyinfo_flag |= KEYBITS;
		opt |= ALG;
		opt |= DER;
		opt |= ISSUER;
		opt |= KEYINFO;
		opt |= SIGNAT;
		opt |= TBS;
		opt |= VAL;

		initialNiceToggle = widget;
	}
		
	if( opt && opt != DER) print_cert_flag = 0;
	print_cert_flag |= opt;
	

#ifdef X500
	/* Authentication method */

	af_dir_authlevel = DBA_AUTH_NONE;

	if ( XmToggleButtonGetState(widget = STWIDGET("*st_auth_none_toggle")) ) {

		af_dir_authlevel = DBA_AUTH_NONE;
		initialAuthToggle = widget;
	}
	if ( XmToggleButtonGetState(widget = STWIDGET("*st_auth_simple_toggle")) ) {

		af_dir_authlevel = DBA_AUTH_SIMPLE;
		initialAuthToggle = widget;
	}
	if ( XmToggleButtonGetState(widget = STWIDGET("*st_auth_strong_toggle")) ) {

		af_dir_authlevel = DBA_AUTH_STRONG;
		initialAuthToggle = widget;
	}
#endif /* X500 */


#ifdef SCA	
	/* RSA done in SCT / via software */
	SC_verify = SC_encrypt = XmToggleButtonGetState(STWIDGET("*st_crypt_toggle"));
#endif /* SCA */

	/* Debug mode toggle */

	if (debugmode_on = XmToggleButtonGetState(STWIDGET("*st_debug_toggle"))) {

		if (!debug_input_handler_installed) {

			XtAppAddWorkProc(	applicationContext,
						debug_workproc,
						STWIDGET("*debug_text")		);

			debug_input_handler_installed = TRUE;
		}
	} else {

		XtUnmanageChild(STWIDGET("*debug_dialog"));

		if (debug_input_handler_installed) debug_input_handler_installed = FALSE;
	}

}


/*
 *	Get configuration from file .xmstrc
 */
void
get_configuration()
{

	/* Generate new resource database if none exists */
	if (!config_database) {
	
		XmToggleButtonSetState(STWIDGET("*config_autosave_toggle"), TRUE, FALSE);
		put_configuration();
		start_with_configuration = TRUE;
	}
	
	auto_save_configuration = get_toggle_resource("config_autosave_toggle");
	Debug =	get_toggle_resource("config_showerror_toggle");
	confirm_overwrite = get_toggle_resource("config_overwrite_toggle");
	
	if (get_toggle_resource("config_useeditor_toggle")) {
	
		XtSetSensitive(STWIDGET("*config_editor_rowcolumn"), TRUE);
		external_editor = aux_cpy_ReducedString(get_textfield_resource("config_editor_textfield"));

	} else {
	
		XtSetSensitive(STWIDGET("*config_editor_rowcolumn"), FALSE);
		get_textfield_resource("config_editor_textfield");
		if (external_editor) free(external_editor);
		external_editor = CNULL;
	}		
	pse_name = aux_cpy_ReducedString(get_textfield_resource("config_psename_textfield"));
	ca_dir = aux_cpy_ReducedString(get_textfield_resource("config_caname_textfield"));
	
	XmToggleButtonSetState(STWIDGET("*st_nice_low_toggle"),
		get_toggle_resource("config_nicelow_toggle"), FALSE);
	XmToggleButtonSetState(STWIDGET("*st_nice_medium_toggle"),
		get_toggle_resource("config_nicemedium_toggle"), FALSE);
	XmToggleButtonSetState(STWIDGET("*st_nice_high_toggle"),
		get_toggle_resource("config_nicehigh_toggle"), FALSE);
	
#ifdef SCA
	auto_detect_sct = get_toggle_resource("config_autodetect_toggle");
	get_toggle_resource("config_softrsa_toggle");	/* value is not used because of radio behavior */
#else
	XtUnmanageChild(STWIDGET("*config_sct_rowcolumn"));
#endif /* SCA */
	
	af_chk_crl = get_toggle_resource("config_consultcrl_toggle");
	af_FCPath_is_trusted = get_toggle_resource("config_trustfcpath_toggle");
	chk_PEM_subordination = get_toggle_resource("config_nosubordination_toggle");
	random_from_pse = get_toggle_resource("config_pserandom_toggle");

	af_access_directory = get_toggle_resource("config_accessdir_toggle");

#ifdef X500
	if (af_access_directory) {
	
		XtSetSensitive(STWIDGET("*config_dsa_rowcolumn"), TRUE);
		af_dir_dsaname = aux_cpy_ReducedString(get_textfield_resource("config_dsaname_textfield"));
		af_dir_tailor = aux_cpy_ReducedString(get_textfield_resource("config_dsaptailor_textfield"));
		
	} else {
	
		XtSetSensitive(STWIDGET("*config_dsa_rowcolumn"), FALSE);
		get_textfield_resource("config_dsaname_textfield");
		get_textfield_resource("config_dsaptailor_textfield");
		if (af_dir_dsaname) free(af_dir_dsaname);
		af_dir_dsaname = CNULL;
		if (af_dir_tailor) free(af_dir_tailor);
		af_dir_tailor = CNULL;
	}
	XmToggleButtonSetState(STWIDGET("*st_auth_none_toggle"), 
		get_toggle_resource("config_authnone_toggle"), FALSE);
	XmToggleButtonSetState(STWIDGET("*st_auth_simple_toggle"), 
		get_toggle_resource("config_authsimple_toggle"), FALSE);
	XmToggleButtonSetState(STWIDGET("*st_auth_strong_toggle"), 
		get_toggle_resource("config_authstrong_toggle"), FALSE);
#else
	XtUnmanageChild(STWIDGET("*config_dsa_rowcolumn"));
#endif /* X500 */
}


/*
 *	Get single Boolean Toggle Button Value resource from config_database
 *	Set given widget's value to that resource
 *	Set to FALSE if resource was not specified correctly to one of (true/TRUE/True)
 */
static
Boolean
get_toggle_resource(widget_name)
	char			*widget_name;

{
	char			ext_widget_name[256];
	char			resource_name[256];
	char			*type;
	XrmValue		value;	

		
	if (!widget_name) return(FALSE);
	
	sprintf(ext_widget_name, "*%s", widget_name);
	sprintf(resource_name, "*%s.set", widget_name);
	
	if (XrmGetResource(config_database, resource_name, NULL, &type, &value)) {
	
		strncpy(value_string, (char *)value.addr, (int)value.size);
		if (!strcmp(value_string, "true") || !strcmp(value_string, "TRUE") || !strcmp(value_string, "True")) {
		
			XmToggleButtonSetState(STWIDGET(ext_widget_name), TRUE, FALSE);
			return(TRUE);
		}
	}
			
	XmToggleButtonSetState(STWIDGET(ext_widget_name), FALSE, FALSE);
		
	return(FALSE);	
		
}

	

/*
 *	Get single Textfield String Value resource from config_database
 *	Set given widget's value to that resource
 *	Returns value; Strings have to be copied before usage!
 */
static
char *
get_textfield_resource(widget_name)
	char			*widget_name;

{
	char			ext_widget_name[256];
	char			resource_name[256];
	char			*type;
	XrmValue		value;	

		
	if (!widget_name) return((char *)NULL);
	
	sprintf(ext_widget_name, "*%s", widget_name);
	sprintf(resource_name, "*%s.value", widget_name);
	
	if (XrmGetResource(config_database, resource_name, NULL, &type, &value)) {
	
		if (value.addr && value.size) {
		
			strncpy(value_string, (char *)value.addr, (int)value.size);
			XmTextSetString(STWIDGET(ext_widget_name), value_string);
			return(value_string);
		}
	}
			
	XmTextSetString(STWIDGET(ext_widget_name), "");
		
	return((char *)NULL);	
}

	





/*
 *	Put configuration database
 */
void
put_configuration()
{

	put_toggle_resource("config_autosave_toggle");
	put_toggle_resource("config_showerror_toggle");
	put_toggle_resource("config_overwrite_toggle");
	put_toggle_resource("config_useeditor_toggle");
	put_textfield_resource("config_editor_textfield");
	
	put_textfield_resource("config_psename_textfield");
	put_textfield_resource("config_caname_textfield");
	
	put_toggle_resource("config_nicelow_toggle");
	put_toggle_resource("config_nicemedium_toggle");
	put_toggle_resource("config_nicehigh_toggle");
	
#ifdef SCA
	put_toggle_resource("config_autodetect_toggle");
	put_toggle_resource("config_softrsa_toggle");
#endif /* SCA */
	
	put_toggle_resource("config_consultcrl_toggle");
	put_toggle_resource("config_trustfcpath_toggle");
	put_toggle_resource("config_nosubordination_toggle");
	put_toggle_resource("config_pserandom_toggle");
	
#ifdef X500	
	put_toggle_resource("config_accessdir_toggle");
	put_textfield_resource("config_dsaname_textfield");
	put_textfield_resource("config_dsaptailor_textfield");
	put_toggle_resource("config_authnone_toggle");
	put_toggle_resource("config_authsimple_toggle");
	put_toggle_resource("config_authstrong_toggle");
#endif /* X500 */
}


/*
 *	Put single Boolean Toggle Button Value resource to config_database
 */
static
void
put_toggle_resource(widget_name)
	char			*widget_name;

{
	char			ext_widget_name[256];
	char			resource_name[256];
	XrmValue		value;	

		
	if (!widget_name) return;
	
	sprintf(ext_widget_name, "*%s", widget_name);
	sprintf(resource_name, "*%s.set", widget_name);
	
	strcpy(value_string, XmToggleButtonGetState(STWIDGET(ext_widget_name)) ? "true" : "false");
	value.size = strlen(value_string) + 1;
	value.addr = value_string;
	
	XrmPutResource(&config_database, resource_name, "String", &value);		
}

	

/*
 *	Put single Textfield String Value resource to config_database
 */
static
void
put_textfield_resource(widget_name)
	char			*widget_name;

{
	char			ext_widget_name[256];
	char			resource_name[256];
	XrmValue		value;
	

	if (!widget_name) return;
	
	sprintf(ext_widget_name, "*%s", widget_name);
	sprintf(resource_name, "*%s.value", widget_name);
	
	strcpy(value_string, XmTextGetString(STWIDGET(ext_widget_name)));
	value.size = strlen(value_string) + 1;
	value.addr = value_string;
	
	XrmPutResource(&config_database, resource_name, "String", &value);		
		
}

	

/*
 *	Reset Widgets if configuration dialog was dismissed
 */
void
reset_configuration()
{

	if (config_database) {
	
		get_toggle_resource("config_autosave_toggle");
		get_toggle_resource("config_showerror_toggle");
		get_toggle_resource("config_overwrite_toggle");
		
		if (get_toggle_resource("config_useeditor_toggle")) {
		
			XtSetSensitive(STWIDGET("*config_editor_rowcolumn"), TRUE);
			get_textfield_resource("config_editor_textfield");

		} else {
		
			XtSetSensitive(STWIDGET("*config_editor_rowcolumn"), FALSE);
			get_textfield_resource("config_editor_textfield");
		}		
		get_textfield_resource("config_psename_textfield");
		get_textfield_resource("config_caname_textfield");
		
		get_toggle_resource("config_nicelow_toggle");
		get_toggle_resource("config_nicemedium_toggle");
		get_toggle_resource("config_nicehigh_toggle");
		
#ifdef SCA
		get_toggle_resource("config_autodetect_toggle");
		get_toggle_resource("config_softrsa_toggle");
#endif /* SCA */
		
		get_toggle_resource("config_consultcrl_toggle");
		get_toggle_resource("config_trustfcpath_toggle");
		get_toggle_resource("config_nosubordination_toggle");
		get_toggle_resource("config_pserandom_toggle");

#ifdef X500
		if (get_toggle_resource("config_accessdir_toggle")) {
		
			XtSetSensitive(STWIDGET("*config_dsa_rowcolumn"), TRUE);
			get_textfield_resource("config_dsaname_textfield");
			get_textfield_resource("config_dsaptailor_textfield");
			
		} else {
		
			XtSetSensitive(STWIDGET("*config_dsa_rowcolumn"), FALSE);
			get_textfield_resource("config_dsaname_textfield");
			get_textfield_resource("config_dsaptailor_textfield");
		}
		get_toggle_resource("config_authnone_toggle");
		get_toggle_resource("config_authsimple_toggle");
		get_toggle_resource("config_authstrong_toggle");
#endif /* X500 */
	}
}


	























