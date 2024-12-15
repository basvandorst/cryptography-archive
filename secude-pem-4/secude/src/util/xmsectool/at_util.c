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

/*-----------------------at_util.c----------------------------------*/
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
/*	alias tool utility functions                                */
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
 *	statics		*
 ************************
 */





Widget		aliasList;







/*
 * 	read AliasList, init panels
 */
void
at_init_tool()
{
	char			*proc = "at_init_tool";
	AliasList		*sys_alist;

	
	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	aliasList = ATWIDGET("*at_list");

	at_check_aliaslist();
	
	if (! (sys_alist = aux_select_AliasList(systemalias))) XtSetSensitive(ATWIDGET("*at_system_toggle"), FALSE);
	else aux_free_AliasList(&sys_alist);

	XtSetArg(args[0], XmNuserData, CNULL);
	XtSetValues(aliasList, args, 1);

	at_fill_list();	

	IDLE_CURSOR();
}

/*
 * 	check AliasList
 */
void
at_check_aliaslist()
{
	char			*proc = "at_check_aliaslist";

	
	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	if (open_write_tempfile(atToplevel) >= 0)  {

		if (!aux_check_AliasList(logfile)) {

			close_tempfile();
			sprintf(dialog_message, "%s%s%s%s",
				"         Some double aliases were found!\n",
				"This may confuse (but not really bother) SecTool.\n",
				"If the System Alias File is stable now, You can\n",
				"     delete twins in Your PSE's Alias List.");

			if (!cont_show_dialog_open(atToplevel, dialog_message)) {
			
				XtManageChild(STWIDGET("*viewer_dialog"));
				start_editor();
			}

		}
		remove_tempfile();

	} else aux_check_AliasList(stderr);
}


/* 
 *	 fill alias_list
 */

void
at_fill_list()
{
	char				*proc = "at_fill_list";
	int				i;
	AliasFile			current_afile;
	unsigned			file_setting_value = XMST_NONE;
	unsigned			type_setting_value = XMST_NONE;
	Boolean				dname_in_both_files;
	char				*client_data;
	Name				*al_name;		
	char				*al_alias;
	char				alias_string[XMST_ALIASLISTSTR_LENGTH + 1];
	char				name_string[XMST_ALIASLISTSTR_LENGTH + 1];
	int				selected_row = 0;
	char				*fill = "";
	char				*dummy = "     ???";
	XmString			cpdstring, cpdalias, cpdname;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	if (XmToggleButtonGetState(ATWIDGET("*at_user_toggle"))) file_setting_value = XMST_ALIAS_USER;
	if (XmToggleButtonGetState(ATWIDGET("*at_system_toggle"))) {

		if (file_setting_value == XMST_ALIAS_USER) file_setting_value = XMST_ALIAS_BOTH;
		else file_setting_value = XMST_ALIAS_SYSTEM;
	}

	if (XmToggleButtonGetState(ATWIDGET("*at_localname_toggle"))) type_setting_value = XMST_ALIAS_LOCALNAME;
	if (XmToggleButtonGetState(ATWIDGET("*at_nextbest_toggle"))) type_setting_value = XMST_ALIAS_NEXTBEST;
	if (XmToggleButtonGetState(ATWIDGET("*at_rfcmail_toggle"))) type_setting_value = XMST_ALIAS_RFCMAIL;
	if (XmToggleButtonGetState(ATWIDGET("*at_x400mail_toggle"))) type_setting_value = XMST_ALIAS_X400MAIL;

	/* check which (if any) list item is selected */
	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	/* Clear List */
	
	XmListDeleteAllItems(aliasList);
	init_clientdata(aliaslist_clientdata);
	XtSetArg(args[0], XmNuserData, CNULL);
	XtSetValues(aliasList, args, 1);



	/*
         *	Read all DNames from aliaslist->dname, get alias
         */

	for (i = 0; i < 2; i++)  {

		current_afile = (i == 0) ? useralias : systemalias; 

		if ( (file_setting_value == XMST_ALIAS_USER && current_afile == systemalias) ||
		     (file_setting_value == XMST_ALIAS_SYSTEM && current_afile == useralias)	) continue;

		al_name = aux_alias_nxtname(TRUE);

		while (al_name) {

			if (!aux_alias_chkfile(al_name, current_afile))  {

				if (al_name) free(al_name);
				al_name = aux_alias_nxtname(FALSE);
				continue;
			}

			/* read demanded alias or next best */
			switch(type_setting_value)  {

				case XMST_ALIAS_LOCALNAME:

					al_alias = aux_Name2aliasf(al_name, LOCALNAME, current_afile);
					break;

				case XMST_ALIAS_RFCMAIL:

					al_alias = aux_Name2aliasf(al_name, RFCMAIL, current_afile);
					break;

				case XMST_ALIAS_X400MAIL:

					al_alias = aux_Name2aliasf(al_name, X400MAIL, current_afile);
					break;

				case XMST_ALIAS_NEXTBEST:

					al_alias = aux_Name2aliasf(al_name, ANYALIAS, current_afile);
					break;
			}

			if (!al_alias) {

				if (verbose) fprintf(stderr, "No alias of type %d stored for %s\n", type_setting_value, al_name);

				/* another type of alias ? */
				if (al_alias = aux_Name2aliasf(al_name, ANYALIAS, current_afile)) {

					free(al_alias);
					al_alias = aux_cpy_String(dummy);
				}
			}

			/* Format element length <= XMST_ALIASLISTSTR_LENGTH ! */
			sprintf(alias_string, "%-23.23s", al_alias);
			sprintf(name_string, "%2s%-75.75s", fill, al_name);
				
			/*
			 *	display dname with bold fixed font if dname is in both alias files
			 */

			cpdname = (XmString)NULL;
			dname_in_both_files = (aux_alias_chkfile(al_name, useralias)
						&& aux_alias_chkfile(al_name, systemalias));

			if ( (dname_in_both_files && (current_afile == useralias)) ||
			     (dname_in_both_files && (current_afile == systemalias) &&
			     (file_setting_value == XMST_ALIAS_SYSTEM)) ) {

				cpdname = CPDSTRINGBOLD(name_string);

			}
			else if (current_afile == useralias
					|| (!dname_in_both_files && (current_afile == systemalias))) {

				cpdname = CPDSTRINGFIXED(name_string);

			}

			/* Don't display double DNames twice */
			if (cpdname) {

				if (current_afile == useralias) cpdalias = CPDSTRINGBOLD(alias_string);
				else cpdalias = CPDSTRINGFIXED(alias_string);
				
				cpdstring = XmStringConcat(cpdalias, cpdname);

				if (append_clientdata(aliaslist_clientdata, (XtPointer)al_name))
					XmListAddItem(aliasList, cpdstring, 0);

				if (client_data && !strcmp(client_data, al_name))
					selected_row = count_clientdata(aliaslist_clientdata);

				XmStringFree(cpdstring);
				XmStringFree(cpdname);
				XmStringFree(cpdalias);
			}

			if (al_alias) free(al_alias);
			al_name = aux_alias_nxtname(FALSE);	/* don't free: user data! */
		}
	}

	if (selected_row > 0) {

		XmListSelectPos(aliasList, selected_row, TRUE);	   /* TRUE => call at_aliaslist_action_cb() */
		XmListSetBottomPos(aliasList, selected_row);

	} else XtUnmanageChild(ATWIDGET("*otheraliases_dialog"));	/* no selection */


	XmUpdateDisplay(applicationShell);
}


/* 
 *	 fill alias textfields
 */

void
at_fill_textfields()
{
	char				*proc = "at_fill_textfields";
	char				*client_data;
	char				*user_data;
	char				*localnamealias;
	char				*rfcmailalias;
	char				*x400mailalias;
	DName				*dname;
	int				total_rows;
	char				*fill = "";
	char				*dummy = "";
	char				*divider = " : ";
	int				menu_row;
	char				*alias_string;
	char				*alias;
	XmString			otheraliases_list[XMST_MAX_OTHERALIASES];
	int				item = 0;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);


	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	if (!client_data)  {

		name = aux_cpy_String("");
		localnamealias = aux_cpy_String("");
		rfcmailalias = aux_cpy_String("");
		x400mailalias = aux_cpy_String("");
		
		XtUnmanageChild(ATWIDGET("*otheraliases_dialog"));

	} else  {

		dname = aux_Name2DName(client_data);
	
		/* fill textfields */

		name = aux_cpy_String(client_data);
		if (! (localnamealias = aux_DName2alias(dname, LOCALNAME)) ) localnamealias = aux_cpy_String(dummy);
		if (! (rfcmailalias = aux_DName2alias(dname, RFCMAIL)) ) rfcmailalias = aux_cpy_String(dummy);
		if (! (x400mailalias = aux_DName2alias(dname, X400MAIL)) ) x400mailalias = aux_cpy_String(dummy);

		if (alias_string = aux_alias_getall(client_data)) {

			alias = strtok(alias_string, ":");

			do  {
				if (item >= XMST_MAX_OTHERALIASES) {

					sprintf(dialog_message, "Too many aliases for DName\n\"%s\"", client_data);
					ok_dialog_open(atToplevel, dialog_message);
		
					break;
				}
				otheraliases_list[item] = (aux_alias(alias) == useralias)
								? CPDSTRINGBOLD(alias)
								: CPDSTRINGFIXED(alias);

				item++;
				alias = strtok(CNULL, ":");
	
			} while (alias);

			free(alias_string);

			XtSetArg(args[0], XmNlistItemCount, item);
			XtSetArg(args[1], XmNlistItems, otheraliases_list);
			XtSetValues(ATWIDGET("*otheraliases_dialog"), args, 2);
		}

		aux_free_DName(&dname);
	}

	/* free old user data */
	XtSetArg(args[0], XmNuserData, &user_data);
	XtGetValues(ATWIDGET("*at_dname_textfield"), args, 1);
	if (user_data) free(user_data);

	XtSetArg(args[0], XmNuserData, &user_data);
	XtGetValues(ATWIDGET("*at_localname_textfield"), args, 1);
	if (user_data) free(user_data);

	XtSetArg(args[0], XmNuserData, &user_data);
	XtGetValues(ATWIDGET("*at_rfcmail_textfield"), args, 1);
	if (user_data) free(user_data);

	XtSetArg(args[0], XmNuserData, &user_data);
	XtGetValues(ATWIDGET("*at_x400mail_textfield"), args, 1);
	if (user_data) free(user_data);

	/* enter new user data */
	XtSetArg(args[0], XmNuserData, (char *)name);
	XtSetArg(args[1], XmNvalue, (char *)name);
	XtSetValues(ATWIDGET("*at_dname_textfield"), args, 2);

	XtSetArg(args[0], XmNuserData, (char *)localnamealias);
	XtSetArg(args[1], XmNvalue, (char *)localnamealias);
	XtSetValues(ATWIDGET("*at_localname_textfield"), args, 2);

	XtSetArg(args[0], XmNuserData, (char *)rfcmailalias);
	XtSetArg(args[1], XmNvalue, (char *)rfcmailalias);
	XtSetValues(ATWIDGET("*at_rfcmail_textfield"), args, 2);

	XtSetArg(args[0], XmNuserData, (char *)x400mailalias);
	XtSetArg(args[1], XmNvalue, (char *)x400mailalias);
	XtSetValues(ATWIDGET("*at_x400mail_textfield"), args, 2);
}
	



/*
 *	find pattern (given in alias_find_textfield) in alias list
 */

int
at_find_pattern(init_string)
	char				*init_string;
{
	char				*proc = "at_find_pattern";
	char				*name;
	char				*first_name;
	char				*pattern;
	int				entry;
	char				*list_data;
	char				*client_data;
	unsigned			file_setting_value = XMST_NONE;
	AliasFile			afile;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

	if (! (pattern = aux_cpy_ReducedString(XmTextGetString(ATWIDGET("*at_find_textfield")))) ) {

		return(-1);
	}

	XtSetArg(args[0], XmNuserData, &client_data);
	XtGetValues(aliasList, args, 1);

	if (XmToggleButtonGetState(ATWIDGET("*at_user_toggle"))) file_setting_value = XMST_ALIAS_USER;
	if (XmToggleButtonGetState(ATWIDGET("*at_system_toggle"))) {

		if (file_setting_value == XMST_ALIAS_USER) file_setting_value = XMST_ALIAS_BOTH;
		else file_setting_value = XMST_ALIAS_SYSTEM;
	}

	if (! (name = aux_search_AliasList(init_string, pattern))) ALARM();
	else {

		first_name = aux_cpy_String(name);

		if (file_setting_value != XMST_ALIAS_BOTH) {

			afile = (file_setting_value == XMST_ALIAS_USER) ? useralias : systemalias;

			do {
				if (!aux_alias_chkfile(name, afile)) {
			
					free(name);
					if (! (name = aux_search_AliasList("", pattern))) {

						free(first_name);
						free(pattern);

						ALARM();
						
						return(0);
					}
	
				} else break;

			} while (strcmp(first_name, name));
		}

		free(first_name);
			
		for (entry = count_clientdata(aliaslist_clientdata); entry > 0; entry--) {

			list_data = (char *)get_clientdata(aliaslist_clientdata, entry);
			if (!strcmp(list_data, name)) {
			
				if (client_data && !strcmp(client_data, list_data)) {	/* already selected */	

					ALARM();
					break;
				}
 
				XmListSelectPos(aliasList, entry, TRUE);  /* TRUE => call at_aliaslist_action-cb() */
				XmListSetBottomPos(aliasList, entry);

				break;
			}
		}
		if (entry <= 0) ALARM();

		free(name);
	}

	free(pattern);

	return(0);

}


/*
 *	Save AliasList if required
 */
void
at_save_aliaslist()
{
	char		*proc = "at_save_aliaslist";


	if (sectool_verbose) fprintf(stderr, "-->%s\n", proc); 

	if (alias_save_required) {

		if (save_loose_dialog_open(applicationShell, "The local Alias List has been changed.\nYou can save or loose all edits.")) {

			alias_save_required = FALSE;
			aux_put_AliasList(useralias);
		}
	}
}


















