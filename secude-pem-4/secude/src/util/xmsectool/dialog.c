/*
 *  SecuDE Release 4.3 (GMD)
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

/*-----------------------dialog.c-----------------------------------*/
/*                                                                  */
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)                  */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990 / "SecuDE" 1991/92/93                */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer 	                    */
/* Luehe/Surkau/Reichelt/Kolletzki		                    */
/*------------------------------------------------------------------*/
/* PACKAGE   util            VERSION   4.3                          */
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
/* MODULE    dialog                                                 */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	dialog creators/callbacks                                   */
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

static char			dialog_pin[XMST_PIN_LENGTH + 1];
static Boolean			dialog_finished;
static Boolean			dialog_ok;

static void			dialog_loop();
static void			write_object();
static void			read_object();
static XmStringTable		get_object_table();
static void			free_object_table();



/*
 *	Loop until dialog is finished
 */
static
void
dialog_loop()
{

	dialog_finished = FALSE;
	while (!dialog_finished) XtAppProcessEvent(applicationContext, XtIMAll);
}


/*
 *	PIN dialog
 */
void
pin_dialog_open(pin_failure_count_init)
	int		pin_failure_count_init;
{
	char		*proc = "pin_dialog_open";
	Widget 		dialog = STWIDGET("*pin_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	sprintf(dialog_message, "Enter PIN for PSE\n\"%s\"", pse_path);
	cmpd_message = CPDSTRING(dialog_message);
	XtSetArg(args[0], XmNselectionLabelString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);

	XtManageChild(dialog);

	passwd = aux_cpy_String("");
	XmTextSetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), "");

	pin_failure_count = pin_failure_count_init;

	ALARM();

	dialog_loop();
}


void
pin_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> pin_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:
			/* get pin */
			break;

		case XmCR_CANCEL:
			/*
			 *	EXIT by user
			 */
			exit_sectool();
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	/*
	 *	empty PIN: wait for next event
	 */
	strcpy(dialog_pin, passwd);
	if (!strcmp(dialog_pin, ""))  {		/* btw.: PIN with blank margins is allowed */

		XtManageChild(widget);
		ALARM();

		return;
	}

	if (aux_create_AFPSESel(pse_path, dialog_pin) < 0)  {

		fprintf(stderr, "%s: ", sectool_parm[0]);
		fprintf(stderr, "Cannot create AFPSESel.\n"); 
		if (verbose) aux_fprint_error(stderr, 0);

		abort_dialog_open(applicationShell, "Cannot create AFPSESel.");
	}

	if (verbose) fprintf(stderr, "  aux_create_AFPSESel() OK\n");

	if(! (pse_sel = af_pse_open((ObjId *)0, FALSE)) || !sec_pin_check(pse_sel, "pse", dialog_pin) )  {

		if (verbose) aux_fprint_error(stderr, 0);

		if (verbose) fprintf(stderr, "%s: unable to open PSE %s\n", sectool_parm[0], AF_pse.app_name);

		++pin_failure_count;
		if (pin_failure_count >= XMST_PIN_FAILURES)  {

			if (verbose) fprintf(stderr, "Seems as if you don't know the magic word ...\n");
			abort_dialog_open(applicationShell, "Seems as if you don't know\nthe magic word ...");
		}
		else {

			passwd = aux_cpy_String("");
			XmTextSetString(XmSelectionBoxGetChild(widget, XmDIALOG_TEXT), "");

			XtManageChild(widget);
			ALARM3();
		}
	
		return;
	}

	dialog_finished = TRUE;
}


/*
 *	Change PIN dialog
 */
void
chpin_dialog_open()
{
	char		*proc = "chpin_dialog_open";
	Widget 		dialog = STWIDGET("*chpin_dialog");
	XmString	cmpd_message;
	char		new_pin[XMST_PIN_LENGTH + 1];


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	BUSY_CURSOR();

#ifdef SCA

        if(sec_psetest(std_pse.app_name) == SCpse) goto re_encrypt;

#endif

	/*
	 *	OLD pin
	 */
	sprintf(dialog_message, "Enter Old PIN for PSE\n\"%s\"", pse_path);
	cmpd_message = CPDSTRING(dialog_message);
	XtSetArg(args[0], XmNselectionLabelString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XmTextSetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), "");
	passwd = aux_cpy_String("");

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(dialog);
	dialog_loop();

	if (strcmp(dialog_pin, std_pse.pin)) {

		ALARM();
		IDLE_CURSOR();

		return;
	}

	/*
	 *	NEW pin
	 */
	sprintf(dialog_message, "Enter New PIN for PSE\n\"%s\"", pse_path);
	cmpd_message = CPDSTRING(dialog_message);
	XtSetArg(args[0], XmNselectionLabelString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XmTextSetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), "");
	passwd = aux_cpy_String("");

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(dialog);
	dialog_loop();

	if (!strcmp(dialog_pin, "")) {

		ALARM();
		IDLE_CURSOR();

		return;
	}

	strcpy(new_pin, dialog_pin);

	/*
	 *	REENTER NEW pin
	 */
	sprintf(dialog_message, "Re-Enter New PIN for PSE\n\"%s\"", pse_path);
	cmpd_message = CPDSTRING(dialog_message);
	XtSetArg(args[0], XmNselectionLabelString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XmTextSetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), "");
	passwd = aux_cpy_String("");

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(dialog);
	dialog_loop();

	if (strcmp(new_pin, dialog_pin)) {

		ALARM();
		IDLE_CURSOR();

		return;
	}

re_encrypt:

	std_pse.object.name = CNULL;
	/* Re-encrypt PSE-toc and all PSE-objects */
	if (sec_chpin(&std_pse, new_pin) != 0) {

		sprintf(dialog_message, "Can't change PIN for PSE\n\"%s\"", pse_path);
		cont_quit_dialog_open(dialog, dialog_message);

		IDLE_CURSOR();

		return;
	}
	/* Free obsolete pins */
	if(AF_pse.pin) strzfree(&(AF_pse.pin));
	for (i = 0; i < PSE_MAXOBJ; i++) {
		if(AF_pse.object[i].pin) strzfree(&(AF_pse.object[i].pin));
	}

	/* Assign new pin-values to AFPSESel */
	if (aux_create_AFPSESel(pse_path, new_pin) < 0) {

		aux_fprint_error(stderr, verbose);

		sprintf(dialog_message, "Can't change PIN for PSE\n\"%s\"", pse_path);
		cont_quit_dialog_open(dialog, dialog_message);

		IDLE_CURSOR();

		return;
	}
	std_pse.app_name = aux_cpy_String(AF_pse.app_name);
	std_pse.object.pin = aux_cpy_String(AF_pse.pin);
	std_pse.pin = aux_cpy_String(AF_pse.pin);
	std_pse.app_id = AF_pse.app_id;

	IDLE_CURSOR();
}


void
chpin_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> chpin_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:
			/* get pin */

			break;

		case XmCR_CANCEL:
			/* cancel change pin */
			if (passwd) free(passwd);
			passwd = aux_cpy_String("");

			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	strcpy(dialog_pin, passwd);

	dialog_finished = TRUE;

	return;
}






/*
 *	Add Alias dialog
 */
void
addalias_dialog_open(message)
	char		*message;
{
	char		*proc = "addalias_dialog_open";
	Widget 		dialog = STWIDGET("*addalias_dialog");
	XmString	cmpd_message;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNselectionLabelString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmTextSetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT), "");
	XtManageChild(dialog);
	XmStringFree(cmpd_message);

	XMapRaised(display, XtWindow(applicationShell));

	dialog_loop();
}


void
addalias_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;
	char				*alias;
	char				*name;


	if (sectool_verbose) fprintf(stderr, "--> addalias_dialog_cb\n");

	XmUpdateDisplay(applicationShell);
 
	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:

			alias_to_add = aux_cpy_ReducedString(XmTextGetString(XmSelectionBoxGetChild(widget, XmDIALOG_TEXT)));
			break;

		case XmCR_CANCEL:

			dialog_finished = TRUE;

			return;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}


	if (alias_to_add)  {

		if (aux_alias(alias_to_add) != notstored)  {
	
			ok_dialog_open(applicationShell, "Alias already exists.");
			dialog_finished = FALSE;
			free(alias_to_add);
			alias_to_add = CNULL;
			XtManageChild(widget);

			return;

		} else dialog_finished = TRUE;

	} else XtManageChild(widget);
}



/*
 *	OK dialog
 */
void
ok_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "ok_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*ok_dialog");
	XmString	cmpd_message;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);

	XMapRaised(display, XtWindow(parentShell));

	XtManageChild(dialog);

	dialog_loop();
}


void
ok_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> ok_dialog_cb\n");

	XmUpdateDisplay(applicationShell);
	
	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	dialog_finished = TRUE;
}




/*
 *	Error dialogs
 */
void
abort_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "abort_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*abort_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);


	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	dialog_loop();
}


void
cont_quit_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "cont_quit_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*cont_quit_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);


	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	XMapRaised(display, XtWindow(parentShell));

	dialog_loop();
}


Boolean
cont_canc_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "cont_canc_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*cont_canc_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	XMapRaised(display, XtWindow(parentShell));

	dialog_loop();

	return(dialog_ok);
}


void
toolquit_dialog_open(parentShell)
	Widget		parentShell;
{
	char		*proc = "toolquit_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*toolquit_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING("Confirm termination of SecTool\n  or just close this subtool.");
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	dialog_loop();
}

void
toolquit_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs = (XmSelectionBoxCallbackStruct *)callback_data;


	if (sectool_verbose) fprintf(stderr, "--> toolquit_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	switch (cbs->reason) {

		case XmCR_OK:		/* Quit */

			exit_sectool();
			break;

		case XmCR_CANCEL:	/* Close Tool */

			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	dialog_finished = TRUE;
}




void
error_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> error_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:
			dialog_finished = TRUE;
			break;

		case XmCR_CANCEL:
			exit_sectool();
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
}


/*
 *	Warning dialogs
 */
void
warning_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> warning_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:
			dialog_ok = TRUE;
			break;

		case XmCR_CANCEL:
			dialog_ok = FALSE;
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	dialog_finished = TRUE;
}


/*
 *	Question dialogs
 */
void
question_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> question_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:
			dialog_ok = TRUE;
			break;

		case XmCR_CANCEL:
			dialog_ok = FALSE;
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	dialog_finished = TRUE;
}



/*
 *	Debug dialog
 */
void
debug_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> debug_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:

			XtUnmanageChild(widget);
			break;

		case XmCR_CANCEL:

			aux_free_error();
			XmTextSetString(STWIDGET("*debug_text"), "");
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
}



/*
 *	Viewer dialog
 */
void
viewer_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> viewer_dialog_cb\n");

	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:

			XtUnmanageChild(widget);
			break;

		case XmCR_CANCEL:

			XmTextSetString(STWIDGET("*viewer_text"), "");
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
}



/*
 *	Configuration dialog
 */
Boolean
config_dialog_open()
{

	if (sectool_verbose) fprintf(stderr, "--> config_dialog_open\n");

	XMapRaised(display, XtWindow(applicationShell));
	
	XtSetArg(args[0], XmNdefaultButton, NULL);
	XtSetValues(STWIDGET("*config_dialog"), args, 1);

	XtManageChild(STWIDGET("*config_dialog"));
	
	dialog_loop();
	
	return(dialog_ok);
}


void
clipboard_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:

			XtUnmanageChild(widget);
			break;

		case XmCR_CANCEL:

			XmTextSetString(STWIDGET("*clipboard_text"), "");
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
	
	dialog_finished = TRUE;
}


void
config_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:

			put_configuration();
			
			dialog_ok = TRUE;
			XtUnmanageChild(widget);
			break;

		case XmCR_CANCEL:

			reset_configuration();
			
			dialog_ok = FALSE;
			XtUnmanageChild(widget);
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
	
	dialog_finished = TRUE;
}


void
config_accessdir_toggle_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmToggleButtonCallbackStruct 	*callback_data;
{

	XtSetSensitive(STWIDGET("*config_dsa_rowcolumn"), callback_data->set ? TRUE : FALSE);

}


void
config_useeditor_toggle_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmToggleButtonCallbackStruct 	*callback_data;
{

	XtSetSensitive(STWIDGET("*config_editor_rowcolumn"), callback_data->set ? TRUE : FALSE);

}



/*
 *	Save dialog
 */
Boolean
save_loose_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "save_loose_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*save_loose_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	XMapRaised(display, XtWindow(parentShell));

	dialog_loop();

	return(dialog_ok);
}


/*
 *	Signature/Encryption object dialog
 */
Boolean
signature_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "signature_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*signature_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	XMapRaised(display, XtWindow(parentShell));

	dialog_loop();

	return(dialog_ok);
}


/*
 *	Show dialog
 */
Boolean
cont_show_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char		*proc = "cont_show_dialog_open";
	Widget 		dialog = name2widget(parentShell, "*cont_show_dialog");
	XmString	cmpd_message;


	if (sectool_verbose) fprintf(stderr, "--> %s\n", proc);

	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNmessageString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	XtManageChild(dialog);

	XMapRaised(display, XtWindow(parentShell));

	dialog_loop();

	return(dialog_ok);
}


/*
 *	Other Aliases dialog
 */
void
otheraliases_dialog_open()
{
	char		*proc = "otheraliases_dialog_open";
	char		*client_data;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(ATWIDGET("*at_list"), args, 1);

	if (client_data) {

		XtManageChild(ATWIDGET("*otheraliases_dialog"));
	}
}

void
otheraliases_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs = (XmSelectionBoxCallbackStruct *)callback_data;
	char				*client_data;
	int				list_items;
	char				*alias;
	char				*name;


	if (sectool_verbose) fprintf(stderr, "--> otheraliases_dialog_open\n");

	XmUpdateDisplay(applicationShell);

	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(ATWIDGET("*at_list"), args, 1);

	alias = aux_cpy_ReducedString(XmTextGetString(XmSelectionBoxGetChild(widget, XmDIALOG_TEXT)));

	switch (cbs->reason) {

		case XmCR_OK:		/* Insert */

	
			if (!client_data || !alias) return;

			if (aux_alias(alias) != notstored) {

				name = aux_alias2Name(alias);
				sprintf(dialog_message, "This alias exists and is targeted at DName\n\"%s\"", name);
				ok_dialog_open(widget, dialog_message);
				free(name);

				return;
			}

			XtSetArg(args[0], XmNlistItemCount, &list_items);
			XtGetValues(widget, args, 1);

			if (list_items >= XMST_MAX_OTHERALIASES) {

				sprintf(dialog_message, "Too many aliases for DName\n\"%s\"", client_data);
				ok_dialog_open(widget, dialog_message);
				free(alias);

				return;
			}
	
			aux_add_alias_name(alias, client_data, useralias, FALSE, FALSE);

			break;

		case XmCR_APPLY:	/* Delete */

			if (!client_data || !alias) return;

			if (aux_alias(alias) != notstored) {

				name = aux_alias2Name(alias);
				if (strcmp(client_data, name)) {
		
					sprintf(dialog_message, "This alias is targeted at another DName:\n\"%s\"", name);
					ok_dialog_open(widget, dialog_message);
					free(name);
	
					return;
				}
				free(name);

			} else {
		
				ok_dialog_open(widget, "Unknown alias!");

				return;
			}
	
			if (aux_delete_alias(alias, useralias, FALSE) < 0) {
	
				ok_dialog_open(widget, "Can't delete System alias.");

				return;
			}
	
			break;

		case XmCR_CANCEL:	/* Dismiss */

			XtUnmanageChild(widget);

			return;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");

			return;
	}

	free(alias);

	alias_save_required = TRUE;

	at_fill_list();
}




/*
 *	Object Selection dialog
 */
char *
objectselection_dialog_open(parentShell, message)
	Widget		parentShell;
	char		*message;
{
	char				*proc = "objectselection_dialog_open";
	Widget 				dialog = name2widget(parentShell, "*objectselection_dialog");
	XmString			cmpd_message;
	XmStringTable			object_table;


	cmpd_message = CPDSTRING(message);
	XtSetArg(args[0], XmNselectionLabelString, cmpd_message);
	XtSetValues(dialog, args, 1);
	XmStringFree(cmpd_message);
	
	XtVaSetValues(	dialog,
			XmNlistItemCount,	count_clientdata(pselist_clientdata),
			XmNlistItems, 		object_table = get_object_table(),
			NULL								);
			
	free_object_table(object_table);

	XMapRaised(display, XtWindow(parentShell));

	XtManageChild(dialog);

	dialog_loop();
	
	if (dialog_ok) return(aux_cpy_ReducedString(XmTextGetString(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT))));
	else return(CNULL);
}

void
objectselection_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:
			dialog_ok = TRUE;
			break;

		case XmCR_CANCEL:
			dialog_ok = FALSE;
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	dialog_finished = TRUE;
}




/*
 *	File Selection dialog
 */
void
file_dialog_open(parentShell)
	Widget		parentShell;
{
	char				*proc = "file_dialog_open";
	Widget 				dialog = name2widget(parentShell, "*file_dialog");
	Widget				file_box = name2widget(parentShell, "*file_selection_box");
	Widget				object_box = name2widget(parentShell, "*file_object_selection_box");
	char				*data;
	XmString			selection;
	XmStringTable			object_table;


	XtSetArg(args[0], XmNuserData, &data);
	XtGetValues(pseList, args, 1);
	selection = XmStringCreateLocalized(data ? data : "");

	XtVaSetValues(	object_box,
			XmNlistItemCount,	count_clientdata(pselist_clientdata),
			XmNlistItems, 		object_table = get_object_table(),
			XmNtextString, 		selection,
			NULL								);
			
	free_object_table(object_table);
	XmStringFree(selection);

	XMapRaised(display, XtWindow(parentShell));

	XtManageChild(dialog);

	dialog_loop();
}

void
file_dialog_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XmSelectionBoxCallbackStruct	*cbs;


	if (sectool_verbose) fprintf(stderr, "--> file_dialog_cb\n");

	XmUpdateDisplay(applicationShell);
	
	cbs = (XmSelectionBoxCallbackStruct *)callback_data;
	switch (cbs->reason) {

		case XmCR_CANCEL:

			XtUnmanageChild(widget);
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}

	dialog_finished = TRUE;
}



void
file_selection_dialog_cb(widget, tag, callback_data)
	Widget					widget;
	char    				*tag;
	XmAnyCallbackStruct 			*callback_data;
{
	char					*proc = "file_selection_dialog_cb";
	XmFileSelectionBoxCallbackStruct	*cbs;
	XmStringTable				object_table;


	XmUpdateDisplay(applicationShell);

	cbs = (XmFileSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:				/* write to PSE */
			write_object(	STWIDGET("*file_dialog"),
					XmTextGetString(XmSelectionBoxGetChild(
						STWIDGET("*file_object_selection_box"), XmDIALOG_TEXT)),
					XmTextGetString(XmSelectionBoxGetChild(widget, XmDIALOG_TEXT))	);
					
        		st_fill_widgets();
        		
			XtVaSetValues(	STWIDGET("*file_object_selection_box"),
					XmNlistItemCount,	count_clientdata(pselist_clientdata),
					XmNlistItems, 		object_table = get_object_table(),
					NULL								);
			
			free_object_table(object_table);
        
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
}

void
file_object_selection_dialog_cb(widget, tag, callback_data)
	Widget					widget;
	char    				*tag;
	XmAnyCallbackStruct 			*callback_data;
{
	char					*proc = "file_object_selection_dialog_cb";
	XmSelectionBoxCallbackStruct		*cbs;
	XmString				dir_mask;


	XmUpdateDisplay(applicationShell);

	cbs = (XmSelectionBoxCallbackStruct *)callback_data;

	switch (cbs->reason) {

		case XmCR_OK:				/* read to File */
			read_object(	STWIDGET("*file_dialog"),
					XmTextGetString(XmSelectionBoxGetChild(widget, XmDIALOG_TEXT)),
					XmTextGetString(XmSelectionBoxGetChild(
						STWIDGET("*file_selection_box"), XmDIALOG_TEXT))	);
						
		        XtSetArg(args[0], XmNdirMask, &dir_mask);
		        XtGetValues(STWIDGET("*file_selection_box"), args, 1);
		        XmFileSelectionDoSearch(STWIDGET("*file_selection_box"), dir_mask);
        
			break;

		default:
	 		if (sectool_verbose) fprintf(stderr, "Oops - unknown callback reason!\n");
	}
}




/*
 *	Write object from file to PSE
 */
static
void
write_object(dialog, object_name, file_name)
	Widget			dialog;
	char			*object_name;
	char			*file_name;
{
	char			*proc = "write_object";
	OctetString		*content;
	void			*opaque;

	
	if (!object_name || !strcmp(object_name, "")
	    || !file_name || !strcmp(file_name, "")
	    || (file_name[strlen(file_name) - 1] == PATH_SEPARATION_CHAR)) {
	
		ALARM();
		
		return;
	}
	
	BUSY_CURSOR();
	
        if (! (content = aux_file2OctetString(file_name)) ) {
        
		sprintf(dialog_message, "Unable to read from file:\n\"%s\"", file_name);
		ok_dialog_open(dialog, dialog_message);
		IDLE_CURSOR();
		
		return;
        }
        
        object_oid = &oid;
	if (! (opaque = af_pse_decode(object_name, content, object_oid)) ) {
        
        	aux_free_OctetString(&content);
        	aux_free2_ObjId(object_oid);
		sprintf(dialog_message, "Unable to decode content of file:\n\"%s\"", file_name);
		ok_dialog_open(dialog, dialog_message);
		IDLE_CURSOR();
		
		return;
        }
        
        /* non-standard object: OID for single key pair PSE is returned for some objects */
 	if (!onekeypaironly) {
 	
 		if (!aux_cmp_ObjId(object_oid, Cert_OID)) {
 		
 			aux_free2_ObjId(object_oid);
 			printf(dialog_message, "Select the type of Certificate:\n\"%s\"", object_name);
 			if (signature_dialog_open(dialog, dialog_message))
 				aux_cpy2_ObjId(object_oid, SignCert_OID);
 			else
 				aux_cpy2_ObjId(object_oid, EncCert_OID);
 				
 		} else if (!aux_cmp_ObjId(object_oid, CSet_OID)) {
 		
 			aux_free2_ObjId(object_oid);
 			printf(dialog_message, "Select the type of Certificates:\n\"%s\"", object_name);
 			if (signature_dialog_open(dialog, dialog_message))
 				aux_cpy2_ObjId(object_oid, SignCSet_OID);
 			else
 				aux_cpy2_ObjId(object_oid, EncCSet_OID);
 				
 		} else if (!aux_cmp_ObjId(object_oid, PKList_OID)) {
 		
 			aux_free2_ObjId(object_oid);
 			printf(dialog_message, "Select the type of Trusted Key List:\n\"%s\"", object_name);
 			if (signature_dialog_open(dialog, dialog_message))
 				aux_cpy2_ObjId(object_oid, PKList_OID);
 			else
 				aux_cpy2_ObjId(object_oid, EKList_OID);
 				
 		}
 	}
 	
 	if (af_pse_update(object_name, opaque, object_oid)) {
        
		sprintf(dialog_message, "Unable to update object:\n\"%s\"", object_name);
		ok_dialog_open(dialog, dialog_message);
	}
	
 	af_pse_free(opaque, object_oid);
        aux_free_OctetString(&content);
	IDLE_CURSOR();
	
	
	return;
 }


/*
 *	Read object from PSE to file
 */
static
void
read_object(dialog, object_name, file_name)
	Widget			dialog;
	char			*object_name;
	char			*file_name;
{
	char			*proc = "read_object";
	char			*new_file_name;
	void			*opaque;
	ObjId			opaque_oid;
	OctetString		*content;
	
	
	if (!object_name || !strcmp(object_name, "") || !file_name || !strcmp(file_name, "")) {
	
		ALARM();
		
		return;
	}
	
	BUSY_CURSOR();

	if (file_name[strlen(file_name) - 1] != PATH_SEPARATION_CHAR) new_file_name = aux_cpy_String(file_name);
	else new_file_name = CATNSTR(file_name, object_name, strlen(object_name));
	
	if (! (opaque = af_pse_get(object_name, &opaque_oid)) ) {
	
        	free(new_file_name);
		sprintf(dialog_message,"Object contains no data or unable to open:\n\"%s\"", object_name);
		ok_dialog_open(dialog, dialog_message);
		IDLE_CURSOR();

		return;
	}
	
	if (! (content = af_pse_encode(object_name, opaque, &opaque_oid)) ) {
	
        	free(new_file_name);
		af_pse_free(opaque, &opaque_oid);
		aux_free2_ObjId(&opaque_oid);
		sprintf(dialog_message, "Unable to encode object:\n\"%s\"", object_name);
		ok_dialog_open(dialog, dialog_message);
		IDLE_CURSOR();

		return;
        }
       
        if (aux_OctetString2file(content, new_file_name, 2) < 0) {
        
		sprintf(dialog_message, "Unable to write to file:\n\"%s\"", new_file_name);
		ok_dialog_open(dialog, dialog_message);
        }
        
        free(new_file_name);
	af_pse_free(opaque, &opaque_oid);
	aux_free2_ObjId(&opaque_oid);
 	aux_free_OctetString(&content);
	IDLE_CURSOR();


	return;
}


/*
 *	Get XmStringTable of PSE-objects
 */
static
XmStringTable
get_object_table()
{
	char				*proc = "get_object_table";
	int				items;
	XmStringTable			object_table;


	items = count_clientdata(pselist_clientdata);
	
	object_table = (XmStringTable) XtMalloc ((items + 1) * sizeof (XmString));
	object_table[items] = (XmString)NULL;
	while (--items >= 0)
		object_table[items] = XmStringCreateLocalized((char *)get_clientdata(pselist_clientdata, (items + 1)));
	
	
	return(object_table);
}
	
	
	
	
/*
 *	Free XmStringTable of PSE-objects
 */
static
void
free_object_table(object_table)
XmStringTable				object_table;
{
	char				*proc = "free_object_table";
	int				items;


	for (items = 0; object_table[items]; items++)
		XmStringFree(object_table[items]);
		
	XtFree((char *)object_table);
}










