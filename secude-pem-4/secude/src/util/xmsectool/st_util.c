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

/*-----------------------st_util.c----------------------------------*/
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
/* MODULE    st_util                                                */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	sectool routines                                            */
/*                                                                  */
/*------------------------------------------------------------------*/



#define NO	0
#define YES	1
#define SAVE	2




/*
 ************************
 *	INCLUDES	*
 ************************
 */

#include "xmst.h"



/* ---------------------
 * 	pse init routine 
 */
void
st_open_pse()
{
	char			*proc = "st_open_pse";
	

	if (sectool_verbose) fprintf(stderr, "--> st_open_pse\n"); 

	pseList = STWIDGET("*st_pselist");
	pkList = STWIDGET("*st_pklist");
	ekList = STWIDGET("*st_eklist");

	activeList = (Widget)NULL;		/* can be one of pse/pk/ekList */
	
	/*
	 *	Open PSE
	 */

	ppin = CNULL;

	if ( aux_create_AFPSESel(pse_path, ppin) < 0 ) {

		fprintf(stderr, "ERROR: Cannot create AFPSESel.\n"); 
		if (verbose) aux_fprint_error(stderr, 0);
		abort_dialog_open(applicationShell, "Unable to create AFPSESel.");
	}

	if (sec_psetest(pse_name) == ERR_in_psetest) {
	
		fprintf(stderr, "ERROR: sec_psetest failed.\n");
		if (verbose) aux_fprint_error(stderr, 0);
		abort_dialog_open(applicationShell, "Unable to configure Smart Card environment.");
	}
	
	
	if (sec_psetest(pse_name) == SWpse) {		/* Software PSE */
	
		pin_dialog_open(NULL);

	}
	
#ifdef SCA
	else {						/* Smart Card PSE */

		ok_dialog_open(applicationShell, "You are using a Smart Card environment.\n   Please pay attention to the display\n       of Your IC Card Terminal.");
		
		if (! (pse_sel = af_pse_open((ObjId *)0, FALSE)) )  {

			fprintf(stderr, "%s: unable to open SC PSE %s\n", sectool_parm[0], AF_pse.app_name);
			if (verbose) aux_fprint_error(stderr, 0);
			abort_dialog_open(applicationShell, "Unable to open Smart Card PSE.");
		}
	}
#endif /* SCA */

	if (verbose) fprintf(stderr, "  af_pse_open OK\n");

	if (af_check_if_onekeypaironly(&onekeypaironly))  {

		fprintf(stderr, "%s: unable to determine whether or not PSE shall hold one keypair only\n",
			sectool_parm[0]);
		if (verbose) aux_fprint_error(stderr, 0);
		abort_dialog_open(applicationShell, "Unable to determine whether or not\nPSE shall hold one keypair only.");
	}

	if (verbose) fprintf(stderr, "  af_check_if_onekeypaironly: %d keypairs\n", 2 - onekeypaironly);

	aux_free_PSESel(&pse_sel);

	std_pse.app_name = aux_cpy_String(AF_pse.app_name);
	std_pse.object.name = CNULL;
	std_pse.object.pin = aux_cpy_String(AF_pse.pin);
	std_pse.pin = aux_cpy_String(AF_pse.pin);
	std_pse.app_id = AF_pse.app_id;

	if (verbose) fprintf(stderr, "  std_pse: app_name = %s, app_id = %d\n", std_pse.app_name, std_pse.app_id); 

#ifdef AFDBFILE
	/*
	 *	Determine whether X.500 directory shall be accessed
	 */
	strcpy(afdb, AFDBFILE);        					 /* file = .af-db/	 */
	strcat(afdb, "X500");         					 /* file = .af-db/'X500' */
	if (open(afdb, O_RDONLY) < 0) x500 = FALSE;
#ifdef X500
	if (x500) directory_user_dname = af_pse_get_Name();
#endif /* X500 */
#endif /* AFDBFILE */


	set_property_widgets();
	set_properties();
}



/*
 *	Fill Base Widgets
 */
int
st_fill_widgets()
{
	char				*proc = "st_fill_widgets";
	PSEToc				*main_toc;
	Name				*owner_name;
	DName				*owner_mail;
	char				*create;
	char				*update;
	struct PSE_Objects 		*obj;
	char				setting_value = 0;
	PKList				*pk_list, *ek_list;
	char				*alias;
	char				*alias_subject;
	char				*alias_issuer;
	char				*alias_serial;
	XmString			cstring;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);


	BUSY_CURSOR();

	XmListDeleteAllItems(pseList);
	XmListDeleteAllItems(pkList);
	if (!onekeypaironly) XmListDeleteAllItems(ekList);
	init_clientdata(pselist_clientdata);
	init_clientdata(pklist_clientdata);
	if (!onekeypaironly) init_clientdata(eklist_clientdata);

	object_oid = &oid;

	/* toc-header-panels */
	if (ca_dir) XmTextSetString(STWIDGET("*st_ca_textfield"), ca_dir);
	else {
		XtSetSensitive(STWIDGET("*st_ca_button"), FALSE);
		XmTextSetString(STWIDGET("*st_ca_textfield"), "        N/A");
	}
	if (pse_path) XmTextSetString(STWIDGET("*st_pse_textfield"), pse_path);

	if (onekeypaironly) {

		XtSetArg(args[0], XmNlabelPixmap, onepairPixmap);
		XtSetValues(STWIDGET("*st_pairs_drawnbutton"), args, 1);

	} else {

		XtManageChild(STWIDGET("*st_ek_form"));

		XtSetArg(args[0], XmNlabelPixmap, twopairsPixmap);
		XtSetValues(STWIDGET("*st_pairs_drawnbutton"), args, 1);
	}

	XmUpdateDisplay(applicationShell);

	st_toc_widgets_update(CNULL);		

	/* PSE-List */
	if (sctoc)  {

		obj = sctoc->obj;
		while (obj) {
	
			create = aux_readable_UTCTime(obj->create);
			update = aux_readable_UTCTime(obj->update);

			st_pseList_update(	obj->name,
						create, 
						update, 
						&(obj->noOctets),
						XMST_SC_OBJECT		);

			if (create) free(create);
			if (update) free(update);

			obj = obj->next;
		}
	} 
	if (psetoc)  {

		obj = psetoc->obj;
		while (obj) {
	
			create = aux_readable_UTCTime(obj->create);
			update = aux_readable_UTCTime(obj->update);

			st_pseList_update(	obj->name,
						create,
						update,
						&(obj->noOctets),
						XMST_PSE_OBJECT		);

			if (create) free(create);
			if (update) free(update);

			obj = obj->next;
		}
	} 

	/* PKList */
	if (! (pk_list = (PKList *)af_pse_get(PKList_name, object_oid)) || aux_cmp_ObjId(object_oid, PKList_OID) )  {

		if (verbose) fprintf(stderr, "  Warning: Can't read PKList\n");
	
	}  else {
	
		while (pk_list)  {
	
			if (!pk_list->element) cont_quit_dialog_open(applicationShell, "Empty ToBeSigned field in PKList");
			else  {
	
				if (! (alias_subject = search_add_alias(pk_list->element->subject, LOCALNAME)) )
					alias_subject = aux_DName2Name(pk_list->element->subject);
	
				if (! (alias_issuer = search_add_alias(pk_list->element->issuer, LOCALNAME)) )
					alias_issuer = aux_DName2Name(pk_list->element->issuer);
	
				alias_serial = Serialnumber2String(pk_list->element->serialnumber);
	
				st_pkList_append(	SIGNATURE,
							alias_subject,
							alias_issuer,
							alias_serial,
							pk_list->element->subject,
							pk_list->element->issuer,
							pk_list->element->serialnumber	);
	
				free(alias_subject);
				free(alias_issuer);
				free(alias_serial);
			}
			pk_list = pk_list->next;
		}
		aux_free2_ObjId(object_oid);
		aux_free_PKList(&pk_list);
	}


	/* EKList */
	if (onekeypaironly)  {

		if (verbose) fprintf(stderr, "  EKList is not supported\n");

	} else  {

		if (! (ek_list = (PKList *)af_pse_get(EKList_name, object_oid)) || aux_cmp_ObjId(object_oid, EKList_OID) ) {

			if (verbose) fprintf(stderr, "  Warning: Can't read EKList\n");
	
		} else {
		
			while (ek_list)  {

				if (! (ek_list->element) )
					cont_quit_dialog_open(applicationShell, "Empty ToBeSigned field in EKList");
				else  {
	
					if (! (alias_subject = search_add_alias(ek_list->element->subject, LOCALNAME)) )
						alias_subject = aux_DName2Name(ek_list->element->subject);
	
					if (! (alias_issuer = search_add_alias(ek_list->element->issuer, LOCALNAME)) )
						alias_subject = aux_DName2Name(ek_list->element->issuer);
	
					alias_serial = Serialnumber2String(ek_list->element->serialnumber);
	
					st_pkList_append(	ENCRYPTION,
								alias_subject,
								alias_issuer,
								alias_serial,
								ek_list->element->subject,
								ek_list->element->issuer,
								ek_list->element->serialnumber	);
	
					free(alias_subject);
					free(alias_issuer);
					free(alias_serial);
				}
				ek_list = ek_list->next;
			}
			aux_free2_ObjId(object_oid);
			aux_free_PKList(&ek_list);
		}
	}

	st_toc_widgets_update(AliasList_name);		/* because of search_add_alias() */
	
	/* Set actual properties */
	set_properties();

	XmUpdateDisplay(applicationShell);

	IDLE_CURSOR();


	return(0);
}


/*
 *	update toc-header-fields
 *	if object name is given: 
 *		update entry in PSE-List-Panel if in toc & in list
 *		append entry in list if in toc & not in list
 *		delete entry from list if not in toc
 */

int
st_toc_widgets_update(object_name)
	char				*object_name;
{
	char				*proc = "st_toc_widgets_update";
	PSEToc				*main_toc;
	Name				*owner_name;
	DName				*owner_dname;
	char				*name;
	char				*create;
	char				*update;
	struct PSE_Objects 		*obj;
	char				*fill = "";
	int				total_rows;
	int				entry;
	int				object_origin;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);

		
	/* read toc */
	if (sec_read_tocs(&std_pse, &sctoc, &psetoc) < 0 || (!sctoc && !psetoc))  {		

		aux_add_error(EREADPSE, "sec_read_tocs failed", CNULL, 0, proc);
		abort_dialog_open(applicationShell, "Can't read Table Of Contents.");
	}

	if (sctoc) main_toc = sctoc;
	else main_toc = psetoc;

	/* owner, dname, mail, created, changed textfields */
	XmTextSetString(STWIDGET("*st_owner_textfield"), main_toc->owner);

	owner_dname = af_pse_get_Name();
	if (owner_dname) {
	
		owner_name = aux_DName2Name(owner_dname);
		aux_free_DName(&owner_dname);
		if (owner_name) {
		
			XmTextSetString(STWIDGET("*st_dname_textfield"), owner_name);
			free(owner_name);
		}
	}

	/* SC/SWPSE/ONE setting */
	if (sctoc && !psetoc)		XtSetArg(args[0], XmNlabelPixmap, scPixmap);
	else if (psetoc && !sctoc)	XtSetArg(args[0], XmNlabelPixmap, psePixmap);
	else				XtSetArg(args[0], XmNlabelPixmap, scpsePixmap);

	XtSetValues(STWIDGET("*st_psetype_drawnbutton"), args, 1);

	XmUpdateDisplay(applicationShell);

	/* update for given object_name */
	if (object_name)  {
	
		obj = (struct PSE_Objects *)NULL;

		/* PSE-List */
		if (sctoc)  {

			obj = sctoc->obj;
			while (obj)  {
		
				if (!strcmp(object_name, obj->name))  {			/* SC object */
				
					create = aux_readable_UTCTime(obj->create);
					update = aux_readable_UTCTime(obj->update);

					object_origin = XMST_SC_OBJECT;
	
					break;
				}
				obj = obj->next;
			}
		} 

		if (!obj && psetoc)  {

			obj = psetoc->obj;
			while (obj)  {
		
				if (!strcmp(object_name, obj->name))  {			/* SW-PSE object */

					create = aux_readable_UTCTime(obj->create);
					update = aux_readable_UTCTime(obj->update);
		
					object_origin = XMST_PSE_OBJECT;
	
					break;

				}
				obj = obj->next;
			}
		}

		if (obj)  {

			st_pseList_update(	obj->name, 
						create, 
						update, 
						&(obj->noOctets), 
						object_origin		);
		
			if (create) free(create);
			if (update) free(update);
	
		} else  {			/* not in toc: try to delete from list */

			total_rows = count_clientdata(pselist_clientdata);
			entry = 1;
			while (entry < total_rows)  {

				if (!strcmp(object_name, (char *)get_clientdata(pselist_clientdata, entry)))  {

					delete_clientdata(pselist_clientdata, entry);
					XmListDeletePos(pseList, entry);

					break;
				}
				entry++;
			}
		}
	}	


	return(0);

}



/*
 *	update object in pse-list-panel / append if object not found
 */

int
st_pseList_update(name, created, changed, size, object_origin)
	char			*name;
	char			*created;
	char			*changed;
	int			*size;
	int			object_origin;
{
	char			*proc = "st_pseList_update";
	char			pselist_string[XMST_PSELISTSTR_LENGTH + 1];
	char			*pl_name;
	char			pl_size[7];		/* max. length is 7 digits! */
	char			*fill = "";
	int			total_rows;
	int			entry = 1;		/* first XmList entry is number 1, not 0! */
	XmString		cpdstring, cpdname, cpdrest;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);


	/* *** be careful when changing sprintf-formats! *** */

	sprintf(pl_size, "%6d", *size);
	sprintf(pselist_string, "%-12.12s%4s", name, fill);
	cpdname = (object_origin == XMST_SC_OBJECT) 	? CPDSTRINGBOLD(pselist_string)
							: CPDSTRINGFIXED(pselist_string);

	sprintf(pselist_string, "%-6.6s%1s%-4.4s%1s%-5.5s%4s%-6.6s%1s%-4.4s%1s%-5.5s%4s%-6.6s\0",
				created + 4,  fill,			/* Month/Day: 6 digits */
				created + 20, fill,			/* Year:      4        */
				created + 11, fill,			/* Time:      5        */
				changed + 4,  fill,			/* Month/Day: 6 digits */
				changed + 20, fill,			/* Year:      4        */
				changed + 11, fill,			/* Time:      5        */
				pl_size			);      	/* Size:      6        */

	cpdrest = CPDSTRINGFIXED(pselist_string);
	cpdstring = XmStringConcat(cpdname, cpdrest);

	total_rows = count_clientdata(pselist_clientdata);

	pl_name = aux_cpy_String(name);

	/* update if found */
	while (entry <= total_rows)  {

		if (!strcmp(pl_name, (char *)get_clientdata(pselist_clientdata, entry)))  {

			XmListReplaceItemsPos(pseList, &cpdstring, 1, entry);
			if (pl_name) free(pl_name);

			break;
		}
		entry++;
	}

	/* append if not found */
	if (entry > total_rows) {

		/* append to list if no client data vector overflow */
		if (append_clientdata(pselist_clientdata, (XtPointer)pl_name))
			XmListAddItem(pseList, cpdstring, 0);
	}

	XmStringFree(cpdstring);
	XmStringFree(cpdname);
	XmStringFree(cpdrest);

	XmUpdateDisplay(applicationShell);


	return(0);

}


/* 
 *	append entry in PK/EK-List-Panel
 */
int
st_pkList_append(type, subject, issuer, serial, dn_subject, dn_issuer, os_serial)
	KeyType				type;
	char				*subject;
	char				*issuer;
	char				*serial;
	DName				*dn_subject;
	DName				*dn_issuer;
	OctetString			*os_serial;
{
	char				*proc = "st_pkList_append";
	XtPointer			*list_cd_vector;
	Widget				List;
	PKList_client_data		*list_data;
	char				list_string[XMST_PKLISTSTR_LENGTH + 1];
	char				list_serial[XMST_PKSERIAL_LENGTH + 1];
	char				*fill = "";
	int				total_rows;
	XmString			cpdstring, cpdnames, cpdserial;


	if (sectool_verbose)  fprintf(stderr, "--> %s\n", proc);


	if (type == SIGNATURE)  {

		List = pkList;
		list_cd_vector = pklist_clientdata;

	} else {

		List = ekList;
		list_cd_vector = eklist_clientdata;
	}

	list_data = (PKList_client_data *)malloc(sizeof(PKList_client_data));
	list_data->subject = aux_cpy_DName(dn_subject);
	list_data->issuer = aux_cpy_DName(dn_issuer);
	list_data->serial = os_serial;

	/* *** be careful when changing sprintf-formats! *** */
	sprintf(list_string,	"%-14.14s%3s%-14.14s%3s",
				subject, fill,
				issuer, fill); 
	cpdnames = CPDSTRINGFIXED(list_string);
	cpdserial = CPDSTRINGFIXED(serial);
	cpdstring = XmStringConcat(cpdnames, cpdserial);
	
		
	if (append_clientdata(list_cd_vector, (XtPointer)list_data))
		XmListAddItem(List, cpdstring, 0);


	return(0);

}




/*
 *	get DName's alias of given type, or force entering a new alias for it
 *	return alias, CNULL if canceled
 */

char *
search_add_alias(dname, type)
	DName				*dname;
	AliasType			type;
{
	char				*proc = "search_add_alias";
	char				*alias;
	char				*name;


	if (verbose) fprintf(stderr, "--> %s\n", proc);


	if (! (alias = aux_DName2alias(dname, type)))  {

		name = aux_DName2Name(dname);

		if (verbose) fprintf(stderr, "Warning: No alias found for %s\n", name);

		sprintf(dialog_message, "There is no System-/User-Alias for\n\n  \"%s\"  \n\nEnter new %s:", name, alias_message[type]);

		alias_to_add = CNULL;

		addalias_dialog_open(dialog_message);

		if (alias_to_add) aux_add_alias_name(alias_to_add, name, useralias, TRUE, TRUE);

		free(name);

		return(alias_to_add);
	}


	return(alias);

}





/*
 *	Make active list to target for further clipboard operations
 *	Set user data
 *	Affected are the PSE-related lists: pseList, pkList, ekList
 */
void
st_activate_list(active_list, data)
	Widget			active_list;
	XtPointer		data;
{
    	Pixel	 		parent_color;
    	
    	
    	activeList = active_list;

	/*
	 *	PSE List
	 */
	if (activeList == pseList) {
	
		XtSetArg(args[0], XmNuserData, (char *)data);
		XtSetValues(pseList, args, 1);
		if (!data) XmListDeselectAllItems(pseList);

	    	XtSetArg(args[0], XmNbackground, active_list_color);
		XtSetValues(STWIDGET("*st_pselist_button"), args, 1);
		
	} else {
	
		XtSetArg(args[0], XmNuserData, (char *)NULL);
		XtSetValues(pseList, args, 1);
		XmListDeselectAllItems(pseList);
		
    		XtSetArg(args[0], XmNbackground, &parent_color);
    		XtGetValues(XtParent(pseList), args, 1);
    		XtSetArg(args[0], XmNbackground, parent_color);
		XtSetValues(STWIDGET("*st_pselist_button"), args, 1);
	}
	
	/*
	 *	PK List
	 */
	if (activeList == pkList) {
	
		XtSetArg(args[0], XmNuserData, (PKList_client_data *)data);
		XtSetValues(pkList, args, 1);
		if (!data) XmListDeselectAllItems(pkList);
		
	    	XtSetArg(args[0], XmNbackground, active_list_color);
		XtSetValues(STWIDGET("*st_pklist_button"), args, 1);
		
	} else {
	
		XtSetArg(args[0], XmNuserData, (PKList_client_data *)NULL);
		XtSetValues(pkList, args, 1);
		XmListDeselectAllItems(pkList);
		
    		XtSetArg(args[0], XmNbackground, &parent_color);
    		XtGetValues(XtParent(pkList), args, 1);
    		XtSetArg(args[0], XmNbackground, parent_color);
		XtSetValues(STWIDGET("*st_pklist_button"), args, 1);
	}
	
	/*
	 *	EK List
	 */
	if (activeList == ekList) {
	
		XtSetArg(args[0], XmNuserData, (PKList_client_data *)data);
		XtSetValues(ekList, args, 2);
		if (!data) XmListDeselectAllItems(ekList);
		
	    	XtSetArg(args[0], XmNbackground, active_list_color);
		XtSetValues(STWIDGET("*st_eklist_button"), args, 1);
		
	} else {
	
		XtSetArg(args[0], XmNuserData, (PKList_client_data *)NULL);
		XtSetValues(ekList, args, 1);
		XmListDeselectAllItems(ekList);
		
    		XtSetArg(args[0], XmNbackground, &parent_color);
    		XtGetValues(XtParent(ekList), args, 1);
    		XtSetArg(args[0], XmNbackground, parent_color);
		XtSetValues(STWIDGET("*st_eklist_button"), args, 1);
	}
}













