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

/*-----------------------at_cb.c------------------------------------*/
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
/* MODULE    at_cb                                                  */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	alias tool callbacks                                        */
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
 *	EXTERNALS	*
 ************************
 */
extern Widget		aliasList;


/*
 ************************
 *	STATICS		*
 ************************
 */
static char		*last_search_pattern = CNULL;




void
at_destroy_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
	char			*proc = "at_destroy_cb";


	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc); 

	/*
	 *	refresh SecTool widgets if AliasList has been changed
	 *	but don't apply to PSE here!
	 */
	if (alias_save_required) st_fill_widgets();

	atToplevel = (Widget)NULL;
}



/*
 *	Close AliasTool
 */
void
at_close_button_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{

	/*
	 *	refresh SecTool widgets if AliasList has been changed
	 *	but don't apply to PSE here!
	 */
	if (alias_save_required) st_fill_widgets();

	XtUnrealizeWidget(atToplevel);
	atToplevel = (Widget)NULL;
}


/*
 *	Quit SecTool / Close AliasTool
 */
void
at_quit_button_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{

	/* Quit ? */
	toolquit_dialog_open(atToplevel);

	/* Close Tool was selected */

	/*
	 *	refresh SecTool widgets if AliasList has been changed
	 *	but don't apply to PSE here!
	 */
	if (alias_save_required) st_fill_widgets();

	XtUnrealizeWidget(atToplevel);
	atToplevel = (Widget)NULL;
}



void
at_sectool_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	XMapRaised(display, XtWindow(applicationShell));
}


void
at_directory_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	ok_dialog_open(atToplevel, "Sorry...\nDirectory Browser is not implemented.");

}

void
at_keypool_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	ok_dialog_open(atToplevel, "Sorry...\nKey Pool Editor is not implemented.");

}




void
at_debug_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	debugmode_on = TRUE;

	XmToggleButtonSetState(STWIDGET("*st_debug_toggle"), TRUE, FALSE);

	set_properties();

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(STWIDGET("*debug_dialog"));
}


void
at_viewer_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(STWIDGET("*viewer_dialog"));
}


void
at_config_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(STWIDGET("*config_dialog"));
}



void
at_xlock_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	ok_dialog_open(atToplevel, "Sorry...\nScreen Lock is not implemented.");
}



void
at_file_toggle_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmToggleButtonCallbackStruct 	*callback_data;
{
	static unsigned			last_file_setting_value = XMST_ALIAS_USER;
	static Boolean			ignore = FALSE;

	unsigned			file_setting_value = XMST_NONE;


	if (!XtIsSensitive(ATWIDGET("*at_system_toggle"))) {		/* No system alias file */
	
		XmToggleButtonSetState(ATWIDGET("*at_user_toggle"), TRUE, FALSE);
		
		return;
	}

	if (!ignore) {		/* ignore self-generated events */

		if (XmToggleButtonGetState(ATWIDGET("*at_user_toggle"))) file_setting_value = XMST_ALIAS_USER;
		if (XmToggleButtonGetState(ATWIDGET("*at_system_toggle"))) {
	
			if (file_setting_value == XMST_ALIAS_USER) file_setting_value = XMST_ALIAS_BOTH;
			else file_setting_value = XMST_ALIAS_SYSTEM;
		}
	
		if (file_setting_value == XMST_NONE) {
	
			if (last_file_setting_value == XMST_ALIAS_SYSTEM) {
	
				ignore = TRUE;
				last_file_setting_value = XMST_ALIAS_USER;
				XmToggleButtonSetState(ATWIDGET("*at_user_toggle"), TRUE, TRUE);
				
				return;
			}
			if (last_file_setting_value == XMST_ALIAS_USER) {
	
				ignore = TRUE;
				last_file_setting_value = XMST_ALIAS_SYSTEM;
				XmToggleButtonSetState(ATWIDGET("*at_system_toggle"), TRUE, TRUE);
				
				return;
			}
			
		} else last_file_setting_value = file_setting_value;
	}

	ignore = FALSE;

	if (last_search_pattern) free (last_search_pattern);
	last_search_pattern = CNULL;

	at_fill_list();

}


void
at_type_toggle_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmToggleButtonCallbackStruct 	*callback_data;
{
	static Widget			last_widget = (Widget)NULL;


	/* RadioBox behaviour: there can be only one... ignore unset / same state */

	if (last_widget == widget) return;

	last_widget = widget;

	if (callback_data->set) at_fill_list();

}



void
at_find_cb(widget, tag, callback_data)
	Widget			widget;
	char   			*tag;
	XmAnyCallbackStruct 	*callback_data;
{
	char			*pattern;
	char			*client_data;


	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	pattern = XmTextGetString(ATWIDGET("*at_find_textfield"));

	if (pattern) {

		if (last_search_pattern && !strcmp(pattern, last_search_pattern)) at_find_pattern(client_data);
		else at_find_pattern(CNULL);

		if (last_search_pattern) free(last_search_pattern);
		last_search_pattern = aux_cpy_String(pattern);
	}
}


void
at_apply_drawnbutton_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{

	if (alias_save_required) {

		BUSY_CURSOR();

		/* write alias list to user's file */
		aux_put_AliasList(useralias);
		alias_save_required = FALSE;
	
		/* apply edits to sectool */
		st_fill_widgets();

		at_fill_list();
		at_fill_textfields();

	} else ALARM();
}



void
at_reset_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{
	char			*proc = "alias_reset_handler";


	if (alias_save_required && !cont_canc_dialog_open(atToplevel, "AliasList has been changed!\nYou may continue and discard edits\nor cancel reset."))  {
			
			return;
	}
	alias_save_required = FALSE;

	if (!aux_get_AliasList())  {

		cont_quit_dialog_open(atToplevel, "Can't read AliasList.");

	} else at_check_aliaslist();

	at_fill_list();
}



void
at_clear_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{
	char			*client_data;


	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	if (client_data) {

		XtSetArg(args[0], XmNuserData, CNULL);
		XtSetValues(aliasList, args, 1);

		at_fill_list();

	} else at_fill_textfields();
}



void
at_insert_drawnbutton_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
	char				*proc = "at_insert_drawnbutton_cb";
	char				*name;
	DName				*dname;
	char				*localnamealias;
	char				*rfcmailalias;
	char				*x400mailalias;
	Boolean				do_refresh = FALSE;


	/* adding new dname with at least one alias */
	name = aux_cpy_ReducedString(XmTextGetString(ATWIDGET("*at_dname_textfield")));

	if (!name)  {

		ok_dialog_open(atToplevel, "DName textfield is empty.");

		return;
	}

	dname = aux_Name2DName(name);
	if (!dname)  {

		ok_dialog_open(atToplevel, "DName field represents no distinguished name.");
		if (name) free(name);

		return;
	}

	if (aux_alias_chkfile(name, useralias))  {

		ok_dialog_open(atToplevel, "User's DName entry exists.\nUse Change function!");
		if (name) free(name);

		return;
	}

	localnamealias = aux_cpy_ReducedString(XmTextGetString(ATWIDGET("*at_localname_textfield")));
	rfcmailalias = aux_cpy_ReducedString(XmTextGetString(ATWIDGET("*at_rfcmail_textfield")));
	x400mailalias = aux_cpy_ReducedString(XmTextGetString(ATWIDGET("*at_x400mail_textfield")));

	if (localnamealias)  {

		if (aux_alias(localnamealias) != notstored) ok_dialog_open(atToplevel, "Local Name alias already exists.");
		else  {

			aux_add_alias(localnamealias, dname, useralias, TRUE, FALSE);
			do_refresh = TRUE;
		}
	}

	if (rfcmailalias)  {

		if (aux_alias(rfcmailalias) != notstored) ok_dialog_open(atToplevel, "Internet Mail alias already exists.");
		else  {

			aux_add_alias(rfcmailalias, dname, useralias, TRUE, FALSE);
			do_refresh = TRUE;
		}
	}

	if (x400mailalias)  {

		if (aux_alias(x400mailalias) != notstored) ok_dialog_open(atToplevel, "X400 Mail alias already exists.");
		else  {

			aux_add_alias(x400mailalias, dname, useralias, TRUE, FALSE);
			do_refresh = TRUE;
		}
	}
	
	if (do_refresh)  {

		XtSetArg(args[0], XmNuserData, (char *)name);
		XtSetValues(aliasList, args, 1);

		alias_save_required = TRUE;

		at_fill_list();

	} else ok_dialog_open(atToplevel, "Can't add DName entry.\nEnter at least one new alias!");

	if (name) free(name);
	if (dname) aux_free_DName(&dname);
	if (localnamealias) free(localnamealias);
	if (rfcmailalias) free(rfcmailalias);
	if (x400mailalias) free(rfcmailalias);
}



void
at_change_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{
	char			*proc = "at_change_drawnbutton_cb";
	char			*client_data;
	char			*name;
	DName			*dname;
	char			*old_localnamealias;
	char			*old_rfcmailalias;
	char			*old_x400mailalias;
	char			*localnamealias;
	char			*rfcmailalias;
	char			*x400mailalias;
	Boolean			do_refresh = FALSE;


	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	if (!client_data) {

		ALARM();

		return;
	}

	name = aux_cpy_ReducedString(XmTextGetString(ATWIDGET("*at_dname_textfield")));

	if (!name)  {

		ok_dialog_open(atToplevel, "DName textfield is empty.");

		return;
	}
	dname = aux_Name2DName(name);
	if (!dname)  {

		ok_dialog_open(atToplevel, "DName field represents no distinguished name.");
		if (name) free(name);

		return;
	}

	if (aux_cmp_Name(client_data, name))  {

		ok_dialog_open(atToplevel, "DName field has changed.\nUse Insert function!");
		if (name) free(name);

		return;
	}


	/* insert new aliases on top of dname's alias list, delete old user aliases */	
	XtSetArg(args[0], XmNuserData, &old_localnamealias);
	XtSetArg(args[1], XmNvalue, &localnamealias);
	XtGetValues(ATWIDGET("*at_localname_textfield"), args, 2);

	XtSetArg(args[0], XmNuserData, &old_rfcmailalias);
	XtSetArg(args[1], XmNvalue, &rfcmailalias);
	XtGetValues(ATWIDGET("*at_rfcmail_textfield"), args, 2);

	XtSetArg(args[0], XmNuserData, &old_x400mailalias);
	XtSetArg(args[1], XmNvalue, &x400mailalias);
	XtGetValues(ATWIDGET("*at_x400mail_textfield"), args, 2);

	localnamealias = aux_cpy_ReducedString(localnamealias);
	rfcmailalias = aux_cpy_ReducedString(rfcmailalias);
	x400mailalias = aux_cpy_ReducedString(x400mailalias);

	if (!localnamealias && !rfcmailalias && !x400mailalias)  {

		ok_dialog_open(atToplevel, "Can't change DName entry.\nEnter at least one alias\nor use Delete function!");
		if (name) free(name);
		if (dname) aux_free_DName(&dname);
		if (localnamealias) free(localnamealias);
		if (rfcmailalias) free(rfcmailalias);
		if (x400mailalias) free(x400mailalias);

		return;
	}

	if ((!localnamealias && strcmp(old_localnamealias, "")) || (localnamealias && strcmp(old_localnamealias, localnamealias)))  {

		if (localnamealias && aux_alias(localnamealias) != notstored)  {

			ok_dialog_open(atToplevel, "Local name already exists.");
			if (name) free(name);
			if (dname) aux_free_DName(&dname);
			if (localnamealias) free(localnamealias);
			if (rfcmailalias) free(rfcmailalias);
			if (x400mailalias) free(x400mailalias);

			return;
		}

		if (localnamealias) aux_add_alias(localnamealias, dname, useralias, TRUE, FALSE);
		aux_delete_alias(old_localnamealias, useralias, FALSE);

		do_refresh = TRUE;
	}

	if ((!rfcmailalias && strcmp(old_rfcmailalias, "")) || (rfcmailalias && strcmp(old_rfcmailalias, rfcmailalias)))  {

		if (rfcmailalias && aux_alias(rfcmailalias) != notstored)  {

			ok_dialog_open(atToplevel, "Internet Mail address already exists.");
			if (name) free(name);
			if (dname) aux_free_DName(&dname);
			if (localnamealias) free(localnamealias);
			if (rfcmailalias) free(rfcmailalias);
			if (x400mailalias) free(x400mailalias);

			return;
		}

		if (rfcmailalias) aux_add_alias(rfcmailalias, dname, useralias, TRUE, FALSE);
		aux_delete_alias(old_rfcmailalias, useralias, FALSE);

		do_refresh = TRUE;
	}

	if ((!x400mailalias && strcmp(old_x400mailalias, "")) || (x400mailalias && strcmp(old_x400mailalias, x400mailalias)))  {

		if (x400mailalias && aux_alias(x400mailalias) != notstored)  {

			ok_dialog_open(atToplevel, "X.400 Mail address already exists.");
			if (name) free(name);
			if (dname) aux_free_DName(&dname);
			if (localnamealias) free(localnamealias);
			if (rfcmailalias) free(rfcmailalias);
			if (x400mailalias) free(x400mailalias);

			return;
		}

		if (x400mailalias) aux_add_alias(x400mailalias, dname, useralias, TRUE, FALSE);
		aux_delete_alias(old_x400mailalias, useralias, FALSE);

		do_refresh = TRUE;
	}

	if (name) free(name);
	aux_free_DName(&dname);
	if (localnamealias) free(localnamealias);
	if (rfcmailalias) free(rfcmailalias);
	if (x400mailalias) free(x400mailalias);

	if (do_refresh)  {

		alias_save_required = TRUE;

		at_fill_list();
	}
}



void
at_delete_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{
	char			*proc = "at_delete_drawnbutton_cb";
	char			*client_data;
	char			*alias;

	
	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	if (!client_data) {

		ALARM();

		return;
	}

	if (!aux_alias_chkfile(client_data, useralias))  {

		sprintf(dialog_message, "No User aliases stored for\n%s", client_data);
		ok_dialog_open(atToplevel, dialog_message);

		return;
	}

	sprintf(dialog_message, "Delete User's DName entry for\n%s ?", client_data);
	if (! cont_canc_dialog_open(atToplevel, dialog_message)) {

		return;
	}

	while (alias = aux_Name2aliasf(client_data, ANYALIAS, useralias))
		if (aux_delete_alias(alias, useralias, FALSE) < 0) break;

	XtSetArg(args[0], XmNuserData, CNULL);
	XtSetValues(aliasList, args, 1);

	alias_save_required = TRUE;

	at_fill_list();
	at_fill_textfields();
}




void
at_aliaslist_action_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmListCallbackStruct 	*callback_data;
{
	static char		*client_data;


	if (client_data) free(client_data);
	client_data = aux_cpy_String((char *)get_clientdata(aliaslist_clientdata, callback_data->item_position));

	switch (callback_data->reason) {

		case XmCR_BROWSE_SELECT:

			if (sectool_verbose) fprintf(stderr, "aliasList: XmCR_BROWSE_SELECT\n");

			XtSetArg(args[0], XmNuserData, client_data);
			XtSetValues(widget, args, 1);

			at_fill_textfields(); 

			break;

		case XmCR_DEFAULT_ACTION:

			if (sectool_verbose) fprintf(stderr, "aliasList: XmCR_DEFAULT_ACTION\n");

			otheraliases_dialog_open();

			break;

		default:

	 		if (sectool_verbose) fprintf(stderr, "aliasList: Oops - unknown callback reason!\n");
	}
 
}




void
at_otheraliases_pushbutton_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{

	otheraliases_dialog_open();
}











