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

/*-----------------------st_cb.c------------------------------------*/
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
/* MODULE    st_cb                                                  */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	sectool callbacks                                           */
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

static int	show_pse();
static void	show_pk();
static void	verificate();




void
st_destroy_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
	char			*proc = "st_destroy_cb";
	int			answer;


	exit_sectool();
}



/*
 *	Status picture is drawn button: Refresh PSE data widgets
 */
void
st_psetype_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	af_pse_reset(CNULL);
	st_fill_widgets();

}



/*
 *	PSE List Callbacks
 */


void
st_pselist_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	st_activate_list(pseList, (XtPointer)NULL);

}



void
st_pselist_action_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmListCallbackStruct 	*callback_data;
{
	char			*client_data;
	int			failed;


	client_data = (char *)get_clientdata(pselist_clientdata, callback_data->item_position);

	switch (callback_data->reason) {

		case XmCR_BROWSE_SELECT:

			if (sectool_verbose) fprintf(stderr, "pseList: XmCR_BROWSE_SELECT\n");

			st_activate_list(widget, client_data);

			break;

		case XmCR_DEFAULT_ACTION:

			if (sectool_verbose) fprintf(stderr, "pseList: XmCR_DEFAULT_ACTION\n");

			BUSY_CURSOR();

			if (open_write_tempfile(atToplevel) >= 0) { 
				if (!show_pse(client_data)) {

					close_tempfile();
					start_editor();

				} else remove_tempfile();
			}

			IDLE_CURSOR();
			
			break;

		default:

	 		if (sectool_verbose) fprintf(stderr, "pseList: Oops - unknown callback reason!\n");
	}
 
}




/*
 *	PKList Callbacks
 */



void
st_pklist_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	st_activate_list(pkList, (XtPointer)NULL);

}



void
st_pklist_action_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmListCallbackStruct 	*callback_data;
{
	PKList_client_data	*client_data;


	client_data = (PKList_client_data *)get_clientdata(pklist_clientdata, callback_data->item_position);

	switch (callback_data->reason) {

		case XmCR_BROWSE_SELECT:

			if (sectool_verbose) fprintf(stderr, "pkList: XmCR_BROWSE_SELECT\n");

			st_activate_list(widget, client_data);

			break;

		case XmCR_DEFAULT_ACTION:

			if (sectool_verbose) fprintf(stderr, "pkList: XmCR_DEFAULT_ACTION\n");

			BUSY_CURSOR();

			show_pk(SIGNATURE, client_data);

			IDLE_CURSOR();

			break;

		default:

	 		if (sectool_verbose) fprintf(stderr, "pkList: Oops - unknown callback reason!\n");
	}
 
}




/*
 *	EKList Callbacks
 */


void
st_eklist_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	st_activate_list(ekList, (XtPointer)NULL);


}



void
st_eklist_action_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmListCallbackStruct 	*callback_data;
{
	PKList_client_data	*client_data;


	client_data = (PKList_client_data *)get_clientdata(eklist_clientdata, callback_data->item_position);

	switch (callback_data->reason) {

		case XmCR_BROWSE_SELECT:

			if (sectool_verbose) fprintf(stderr, "ekList: XmCR_BROWSE_SELECT\n");

			st_activate_list(widget, client_data);

			break;

		case XmCR_DEFAULT_ACTION:

			if (sectool_verbose) fprintf(stderr, "ekList: XmCR_DEFAULT_ACTION\n");

			BUSY_CURSOR();

			show_pk(ENCRYPTION, client_data);

			IDLE_CURSOR();

			break;

		default:

	 		if (sectool_verbose) fprintf(stderr, "ekList: Oops - unknown callback reason!\n");
	}
 
}



/*
 *	Menu Button Callbacks
 */


void
st_quit_button_cb(widget, tag, callback_data)
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{

	exit_sectool();

}


void
st_debug_button_cb(widget, tag, callback_data)
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
st_viewer_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(STWIDGET("*viewer_dialog"));
}


void
st_clipboard_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(STWIDGET("*clipboard_dialog"));
}


void
st_config_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	XMapRaised(display, XtWindow(applicationShell));
	XtManageChild(STWIDGET("*config_dialog"));
}


void
st_directory_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	ok_dialog_open(applicationShell, "Sorry...\nDirectory Browser is not implemented.");

}


void
st_keypool_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	ok_dialog_open(applicationShell, "Sorry...\nKey Pool Editor is not implemented.");

}



void
st_alias_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	BUSY_CURSOR();

	if (!atToplevel) {

		XtSetArg(args[0], XmNtitle, "Alias Editor");
	
		atToplevel =		XtAppCreateShell(	"atMainShell", NULL,
								topLevelShellWidgetClass,
								display,
								args, 1				);
	
	
	
		if (MrmFetchWidget(	applicationHierarchy,
					"atMainShell",
					atToplevel,
					&atMainShell,
					&opaqueClass			) != MrmSUCCESS) {
	
			printf("SECTOOL: Can't fetch aliastool interface\n");

			exit(-1);
		}
		
		XtManageChild(atMainShell);
	
		XtSetArg(args[0], XmNiconPixmap, aliastoolPixmap);
		XtSetValues(atToplevel, args, 1);
		XtRealizeWidget(atToplevel);

		/* Add focus color Callbacks */		
	    	XtAddCallback(	XmSelectionBoxGetChild(ATWIDGET("*otheraliases_dialog"), XmDIALOG_TEXT),
				XmNfocusCallback, textfield_focus_cb,
				NULL );
	    	XtAddCallback(	XmSelectionBoxGetChild(ATWIDGET("*otheraliases_dialog"), XmDIALOG_TEXT),
				XmNlosingFocusCallback, textfield_focus_cb,
				NULL );
	
		at_init_tool();

	} else {

		XMapRaised(display, XtWindow(atToplevel));
	}

	IDLE_CURSOR();
}


void
st_xlock_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	ok_dialog_open(applicationShell, "Sorry...\nScreen Lock is not implemented.");
}




/*
 *	Called by st_checkpse_button, st_trustpath_button, and st_verify_button:
 *	Perform PSE-Check / Trust Path Check / Certificate Check
 */
void
st_pse_check_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	secretkey = (Key *)malloc(sizeof(Key));
	if(!secretkey) {
		fprintf(stderr, "XMST pse_checkpse_button_cb: Can't malloc memory for secretkey\n");
		return;
	}
	secretkey->key = (KeyInfo *)0;
	secretkey->keyref = 0;
	secretkey->alg = (AlgId *)0;
	secretkey->pse_sel = &std_pse;
	signpk = (KeyInfo *)0;
	encpk = (KeyInfo *)0;
	signsubject = NULLDNAME;
	encsubject = NULLDNAME;

	if (open_write_tempfile(applicationShell) < 0) return;

	BUSY_CURSOR();

	if (widget == STWIDGET("*st_checkpse_button")) cmd = CHECK;
	if (widget == STWIDGET("*st_trustpath_button")) cmd = TRUSTPATH;
	if (widget == STWIDGET("*st_verify_button")) {
	
                certificate =  af_pse_get_Certificate(SIGNATURE, NULLDNAME, NULLOCTETSTRING);
		if (!certificate) {
			if (onekeypaironly) ok_dialog_open(applicationShell, "Can't read Certificate from PSE.");
			else ok_dialog_open(applicationShell, "Can't read Signature Certificate from PSE.");
		} else
			verificate(certificate, FALSE);

		IDLE_CURSOR();
		
		return;
	}

	certs = af_pse_get_Certificates(SIGNATURE, NULLDNAME);
	if(!certs) {
		if(onekeypaironly == TRUE) {
		
			ok_dialog_open(applicationShell, "Can't read Certificate from PSE.");
			IDLE_CURSOR();
			free(secretkey);
		
			return;
		}
		else {
		
			ok_dialog_open(applicationShell, "Can't read SIGNATURE Certificate from PSE.");
			IDLE_CURSOR();
			
			return;
		}
	}
	signpk = aux_cpy_KeyInfo(certs->usercertificate->tbs->subjectPK);
	signsubject = aux_cpy_DName(certs->usercertificate->tbs->subject);
	if(cmd == CHECK) {
		if(onekeypaironly == TRUE)
			fprintf(logfile, "\nVerifying Cert with FCPath and PKRoot ... ");
		else
			fprintf(logfile, "\nVerifying SignCert with FCPath and PKRoot ... ");
	}
        rcode = af_verify_Certificates(certs, (UTCTime *)0, (PKRoot *)0);
	aux_free_Certificates(&certs);
	if((af_verbose || sec_verbose) && cmd == CHECK) aux_fprint_VerificationResult(logfile, verifresult);
	if(cmd == TRUSTPATH) aux_fprint_TrustPath(logfile, verifresult);
	aux_free_VerificationResult(&verifresult);
	if(cmd == CHECK) {
		if(rcode == 0) fprintf(logfile, "O.K.\n");
		else fprintf(logfile, "failed\n");
		if(onekeypaironly == TRUE)
			fprintf(logfile, "\nChecking whether the keys in Cert and SKnew are an RSA key pair ... ");
		else
			fprintf(logfile, "\nChecking whether the keys in SignCert and SignSK are an RSA key pair ... ");
	}
	if(onekeypaironly == TRUE)
		std_pse.object.name = SKnew_name;
	else
		std_pse.object.name = SignSK_name;
	rcode = sec_checkSK(secretkey, signpk);
	if(rcode < 0) {
		if(onekeypaironly == TRUE)
			fprintf(logfile, "\nRSA keys in SKnew and Cert do not fit\n");
		else
			fprintf(logfile, "\nRSA keys in SignSK and SignCert do not fit\n");
	}
	else if(cmd == CHECK) fprintf(logfile, "O.K.\n");
	if(onekeypaironly == TRUE  || cmd == TRUSTPATH) goto show_results;


enccert:
	certs = af_pse_get_Certificates(ENCRYPTION, NULLDNAME);
	if(!certs) {
		fprintf(logfile, "Can't get ENCRYPTION Certificates from PSE (EncCert and/or FCPath missing)\n");
		if(signpk) aux_free_KeyInfo(&signpk);
		if(signsubject) aux_free_DName(&signsubject);
		free(secretkey);
		goto show_results;
	}
	encpk = aux_cpy_KeyInfo(certs->usercertificate->tbs->subjectPK);
	encsubject = aux_cpy_DName(certs->usercertificate->tbs->subject);
	if(signsubject) if(aux_cmp_DName(signsubject, encsubject)) {
		fprintf(logfile, "SignCert and EncCert have different subject names\n");
	}
	if(signsubject) aux_free_DName(&signsubject);
	aux_free_DName(&encsubject);
	fprintf(logfile, "\nVerifying EncCert with FCPath and PKRoot ...\n");
        rcode = af_verify_Certificates(certs, (UTCTime *)0, (PKRoot *)0);
	aux_free_Certificates(&certs);
	if(af_verbose || sec_verbose) aux_fprint_VerificationResult(logfile, verifresult);
	aux_free_VerificationResult(&verifresult);
	fprintf(logfile, "\nChecking whether the keys in EncCert and DecSKnew are an RSA key pair ... ");
	std_pse.object.name = DecSKnew_name;
	rcode = sec_checkSK(secretkey, encpk);
	if(rcode < 0) fprintf(logfile, "\nRSA keys in DecSKnew and EncCert do not fit\n");
	else fprintf(logfile, "O.K.\n");
	aux_free_KeyInfo(&signpk);
	aux_free_KeyInfo(&encpk);
	free(secretkey);

show_results:

	close_tempfile();
	start_editor();

	IDLE_CURSOR();
}


void
st_pse_changepin_button_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{

	chpin_dialog_open();
}


void
st_show_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{



}

/*
 *	Provide various information attachments for several PSE items
 */
void
st_help_drawnbutton_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XtPointer			data;
	char				*pse_client_data;
	PKList_client_data		*pkek_client_data;
	void				*opaque;
	ObjId				opaque_oid;
	
	
	if (!activeList) {
	
		ALARM();
		
		return;
	}
	
	XtSetArg(args[0], XmNuserData, &data);
	XtGetValues(activeList, args, 1);
	
	if (!data) {
	
		ALARM();
		
		return;
	}
	
	BUSY_CURSOR();

	/*
	 *	PSE List Objects
	 */
	if (activeList == pseList) {

		pse_client_data = (char *)data;

		if (! (opaque = af_pse_get(pse_client_data, &opaque_oid)) ) {
		
			sprintf(dialog_message,"Object contains no data or unable to open:\n\"%s\"", pse_client_data);
			ok_dialog_open(applicationShell, dialog_message);
			IDLE_CURSOR();
	
			return;
		}
	
		/*
		 *	Certificates
		 */
		if (!aux_cmp_ObjId(&opaque_oid, SignCert_OID)
		    || !aux_cmp_ObjId(&opaque_oid, EncCert_OID)
		    || !aux_cmp_ObjId(&opaque_oid, Cert_OID)) 	{ 
	    
		   	verificate((Certificate *)opaque, TRUE);
		   	
		}
		
		/*
		 *	Default: Unknown PSE Object type
		 */
		else {
		
			sprintf(dialog_message, "There is no Information attachment provided for:\n\"%s\"",
						pse_client_data);
			ok_dialog_open(applicationShell, dialog_message);
		}
		
		af_pse_free(opaque, &opaque_oid);
		aux_free2_ObjId(&opaque_oid);
	}
	
	/*
	 *	Pk/EkList List Objects
	 */
	else if (activeList == pkList || activeList == ekList ) {

		pkek_client_data = (PKList_client_data *)data;

		sprintf(dialog_message, "There is no Information attachment provided for:\n\"%s\"",
			activeList == pkList ? PKList_name : EKList_name);
		ok_dialog_open(applicationShell, dialog_message);
	}
	
	IDLE_CURSOR();
}


/*
 *	Copy selected list item to clipboard.
 *	To provide arbitrary data formats, SecuDE's object format is used.
 */
void
st_copy_drawnbutton_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	int				status;
	
	
	if (!activeList) {
	
		ALARM();
		
		return;
	}
	
	
	








}


void
st_paste_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{



}


void
st_cut_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{


}


void
st_undo_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{



}


void
st_file_drawnbutton_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmAnyCallbackStruct 		*callback_data;
{
	XtPointer			data = NULL;
	char				*selected_file;
	
	
	file_dialog_open(applicationShell);
}


void
st_print_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{


}


void
st_create_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{


}


void
st_rename_drawnbutton_cb(widget, tag, callback_data)
	Widget			widget;
	char    		*tag;
	XmAnyCallbackStruct 	*callback_data;
{


}



/*
 *	This Callback is used by all non-menu childs of the Properties Menu Button
 * 	It provides RadioBox behavior inside a Pulldown Menu
 */
void
st_properties_cb(widget, tag, callback_data)
	Widget				widget;
	char    			*tag;
	XmToggleButtonCallbackStruct 	*callback_data;
{
    	static Widget 			authToggle = NULL;
    	static Widget 			niceToggle = NULL;

 
   	if (!authToggle) authToggle = initialAuthToggle;
   	if (!niceToggle) niceToggle = initialNiceToggle;

	if (widget == STWIDGET("*st_auth_none_toggle")||
	    widget == STWIDGET("*st_auth_simple_toggle") ||
	    widget == STWIDGET("*st_auth_strong_toggle")	) {

	    if (callback_data->set) {

		if (authToggle) XmToggleButtonSetState(authToggle, FALSE, FALSE);
		authToggle = widget;

	    } else if (widget == authToggle) XmToggleButtonSetState(widget, TRUE, FALSE);
	}

	if (widget == STWIDGET("*st_nice_low_toggle") ||
	    widget == STWIDGET("*st_nice_medium_toggle") ||
	    widget == STWIDGET("*st_nice_high_toggle")		) {

	    if (callback_data->set) {

		if (niceToggle) XmToggleButtonSetState(niceToggle, FALSE, FALSE);
		niceToggle = widget;

	    } else if (widget == niceToggle) XmToggleButtonSetState(widget, TRUE, FALSE);
	}

	/* Get all actual values */
	set_properties();

}


static
int
show_pse(name)
	char				*name;
{
	char				*proc = "show_pse";
	
	if (af_pse_fprint(logfile, name)) {

		sprintf(dialog_message, "Object contains no data or unable to open:\n\"%s\"", name);
		ok_dialog_open(applicationShell, dialog_message);

		return(-1);
	}

	return(0);
}







/* 
 * 	Show selected pk/eklist entry in editor / viewer
 */
static
void
show_pk(type, list_data)
	KeyType				type;
	PKList_client_data 		*list_data;
{
	char				*proc = "show_pk";
	ToBeSigned			*to_be_signed;


	if (! (to_be_signed = af_pse_get_TBS(type, list_data->subject, list_data->issuer, list_data->serial)) ) {
	
		return;
	}

	if (open_write_tempfile() >= 0) {

		fprintf(logfile, " ________________________________________________\n");
		fprintf(logfile, "|                                                |\n");
		fprintf(logfile, "|    Certificate from %-23.23s    |\n", type == SIGNATURE ? PKList_name : EKList_name);
		fprintf(logfile, "|________________________________________________|\n\n");
		aux_fprint_ToBeSigned(logfile, to_be_signed);
	        aux_free_ToBeSigned(&to_be_signed);
	
		close_tempfile();
		start_editor();

	} 
}




/* 
 * 	Show verification result in editor / viewer
 */
static
void
verificate(cert_object, random_fcpath)
	Certificate			*cert_object;
	Boolean 			random_fcpath;
{
	char				*proc = "verificate";
	char				*selected_object;
	void				*opaque;
	ObjId				opaque_oid;


	if (open_write_tempfile() >= 0) {
		
		if (!random_fcpath) fcpath = af_pse_get_FCPath(NULLDNAME);
		else {
		
			if (! (selected_object = objectselection_dialog_open(applicationShell,
						"\nSelect an PSE-Object of the type\nFCPath or Certificate:")) ) {
						
				return;
			}
						
			if (opaque = af_pse_get(selected_object, &opaque_oid)) {
			
				/* object is fcpath */
				if (!aux_cmp_ObjId(&opaque_oid, FCPath_OID)) {
				
				 	fcpath = aux_cpy_FCPath((FCPath *)opaque);
				
				/* object is certificate */
				} else if (!aux_cmp_ObjId(&opaque_oid, SignCert_OID) 
				    	   || !aux_cmp_ObjId(&opaque_oid, EncCert_OID) 
				    	   || !aux_cmp_ObjId(&opaque_oid, Cert_OID))	{
				   
					if (! (fcpath = aux_create_FCPath((Certificate *)opaque)) ) {
					
						sprintf(dialog_message,
							"Cannot transform Certificate into FCPath:\n\"%s\"",
							selected_object);
						ok_dialog_open(applicationShell, dialog_message);
						af_pse_free(opaque, &opaque_oid);
						aux_free2_ObjId(&opaque_oid);
						free(selected_object);
						
						return;
					}
					
				/* object type mismatch */
				} else {
				   
					sprintf(dialog_message,
						"Object is neither a FCPath nor a Certificate:\n\"%s\"",
						selected_object);
					ok_dialog_open(applicationShell, dialog_message);
					af_pse_free(opaque, &opaque_oid);
					aux_free2_ObjId(&opaque_oid);
					free(selected_object);
					
					return;
				}
				
				af_pse_free(opaque, &opaque_oid);
				aux_free2_ObjId(&opaque_oid);
				free(selected_object);	
							
			} else {
			
				sprintf(dialog_message,
					"Can't read object from PSE:\n\"%s\"",
					selected_object);
				ok_dialog_open(applicationShell, dialog_message);
				free(selected_object);
				
				return;
			}
		}
	
                if (! (certs = aux_create_Certificates(cert_object, fcpath)) ) {
		
			ok_dialog_open(applicationShell, "Can't create Certificates");
			
			return;
		}
			
		fprintf(logfile, " ________________________________________________\n");
		fprintf(logfile, "|                                                |\n");
		fprintf(logfile, "|               Verify Certificate               |\n");
		fprintf(logfile, "|________________________________________________|\n\n");

		pkroot = (PKRoot *)0;
                print_keyinfo_flag = ALGID;

                rcode = af_verify_Certificates(certs, (UTCTime *)0, pkroot);
                
		if (rcode < 0 ) {
		
			fprintf(logfile, "Verification  f a i l e d\n");
			aux_fprint_error(logfile, verbose);
		}
                aux_free_Certificates(&certs);
                if (pkroot) aux_free_PKRoot(&pkroot);
                
		aux_fprint_VerificationResult(logfile, verifresult);
		
		aux_free_VerificationResult(&verifresult);
											
		close_tempfile();
		start_editor();
	} 
}




