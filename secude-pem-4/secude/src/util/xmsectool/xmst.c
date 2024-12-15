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

/*-----------------------xmst.c-------------------------------------*/
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
/* MODULE    main                                                   */
/*                                                                  */
/* INCLUDES  xmst.h                                                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* DESCRIPTION                                                      */
/*   xmst is SecuDE's OSF/Motif (tm) Tool to maintain the PSE       */
/*   based on psemaint.c	    				    */
/*                                                                  */
/* USAGE:                                                           */
/*     xmst [-c ca_dir] [-p pse_name] [-d dsaname] [-hmv] 	    */
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*								    */
/*	xmst (SecTool) depends on following X defaults:	            */
/*								    */
/*	*XmTextField*fontList:	fixed			            */
/*	*XmList*fontList:	6x13=listFixedFont,	            */
/*				6x13bold=listFixedBoldFont          */
/*								    */
/*------------------------------------------------------------------*/
/*								    */
/*	the following sel-defined resources can be set:		    */
/*	(with customized values)				    */
/*								    */
/*	*editor:		textedit -font 6x13 -Ww 80 -Wh 30   */
/*								    */
/*------------------------------------------------------------------*/


#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)xmst.c	1.0	01.11.1993";
#endif /* lint */
#endif /* REV_INFO */





/*
 ************************
 *	includes	*
 ************************
 */

#include "xmst.h"



/*
 ************************
 *	statics		*
 ************************
 */

static char	*uid_version;

static void	usage();
static void	init_toolkits();
static void	init_uil();
static void	create_shell();
static void	get_resources();
static void	open_sectool();
static void	init_secude();


/*
 *	Resource list
 */
static XtResource xmstResources[] = 
		{
/* Busy Cursor */	{ 	XmstNbusycursor, "Cursor",			/* name, class         */
			  	XmRCursor, sizeof(Cursor),			/* type, size          */
			  	XtOffset(ApplResType *, busycursor),		/* offset              */
			  	XmRCursor, (XtPointer)NULL			/* default type, value */
			},
/* Focus Color */	{ 	XmstNfocuscolor, "Pixel",			
			  	XmRPixel, sizeof(Pixel),			
			  	XtOffset(ApplResType *, focuscolor),		
			  	XmRPixel, (XtPointer)NULL			
			},
/* Active List Color */	{ 	XmstNactivelistcolor, "Pixel",			
			  	XmRPixel, sizeof(Pixel),			
			  	XtOffset(ApplResType *, activelistcolor),
			  	XmRPixel, (XtPointer)NULL		
			}
		};




/*
 *	UIL Names
 */
static MrmRegisterArg	applicationRegvec[] =
		{
/* sectool */		{ "st_destroy_cb", (caddr_t)st_destroy_cb },
			{ "st_quit_button_cb", (caddr_t)st_quit_button_cb },
			{ "st_properties_cb", (caddr_t)st_properties_cb },
			{ "st_directory_button_cb", (caddr_t)st_directory_button_cb },
			{ "st_alias_button_cb", (caddr_t)st_alias_button_cb },
			{ "st_keypool_button_cb", (caddr_t)st_keypool_button_cb },
			{ "st_debug_button_cb", (caddr_t)st_debug_button_cb },
			{ "st_viewer_button_cb", (caddr_t)st_viewer_button_cb },
			{ "st_clipboard_button_cb", (caddr_t)st_clipboard_button_cb },
			{ "st_config_button_cb", (caddr_t)st_config_button_cb },
			{ "st_xlock_button_cb", (caddr_t)st_xlock_button_cb },
			{ "st_psetype_drawnbutton_cb", (caddr_t)st_psetype_drawnbutton_cb },
			{ "st_pse_check_button_cb", (caddr_t)st_pse_check_button_cb },
			{ "st_pse_changepin_button_cb", (caddr_t)st_pse_changepin_button_cb },
			{ "st_pselist_button_cb", (caddr_t)st_pselist_button_cb },
			{ "st_pselist_action_cb", (caddr_t)st_pselist_action_cb },
			{ "st_pklist_button_cb", (caddr_t)st_pklist_button_cb },
			{ "st_pklist_action_cb", (caddr_t)st_pklist_action_cb },
			{ "st_eklist_button_cb", (caddr_t)st_eklist_button_cb },
			{ "st_eklist_action_cb", (caddr_t)st_eklist_action_cb },
			{ "st_show_drawnbutton_cb", (caddr_t)st_show_drawnbutton_cb },
			{ "st_help_drawnbutton_cb", (caddr_t)st_help_drawnbutton_cb },
			{ "st_copy_drawnbutton_cb", (caddr_t)st_copy_drawnbutton_cb },
			{ "st_paste_drawnbutton_cb", (caddr_t)st_paste_drawnbutton_cb },
			{ "st_cut_drawnbutton_cb", (caddr_t)st_cut_drawnbutton_cb },
			{ "st_undo_drawnbutton_cb", (caddr_t)st_undo_drawnbutton_cb },
			{ "st_file_drawnbutton_cb", (caddr_t)st_file_drawnbutton_cb },
			{ "st_print_drawnbutton_cb", (caddr_t)st_print_drawnbutton_cb },
			{ "st_create_drawnbutton_cb", (caddr_t)st_create_drawnbutton_cb },
			{ "st_rename_drawnbutton_cb", (caddr_t)st_rename_drawnbutton_cb },
	
/* aliastool */		{ "at_destroy_cb", (caddr_t)at_destroy_cb },
			{ "at_close_button_cb", (caddr_t)at_close_button_cb },
			{ "at_quit_button_cb", (caddr_t)at_quit_button_cb },
			{ "at_sectool_button_cb", (caddr_t)at_sectool_button_cb },
			{ "at_directory_button_cb", (caddr_t)at_directory_button_cb },
			{ "at_keypool_button_cb", (caddr_t)at_keypool_button_cb },
			{ "at_debug_button_cb", (caddr_t)at_debug_button_cb },
			{ "at_viewer_button_cb", (caddr_t)at_viewer_button_cb },
			{ "at_config_button_cb", (caddr_t)at_config_button_cb },
			{ "at_xlock_button_cb", (caddr_t)at_xlock_button_cb },
			{ "at_file_toggle_cb", (caddr_t)at_file_toggle_cb },
			{ "at_type_toggle_cb", (caddr_t)at_type_toggle_cb },
			{ "at_aliaslist_action_cb", (caddr_t)at_aliaslist_action_cb },
			{ "at_find_cb", (caddr_t)at_find_cb },
			{ "at_apply_drawnbutton_cb", (caddr_t)at_apply_drawnbutton_cb },
			{ "at_reset_drawnbutton_cb", (caddr_t)at_reset_drawnbutton_cb },
			{ "at_clear_drawnbutton_cb", (caddr_t)at_clear_drawnbutton_cb },
			{ "at_insert_drawnbutton_cb", (caddr_t)at_insert_drawnbutton_cb },
			{ "at_change_drawnbutton_cb", (caddr_t)at_change_drawnbutton_cb },
			{ "at_delete_drawnbutton_cb", (caddr_t)at_delete_drawnbutton_cb },
			{ "at_otheraliases_pushbutton_cb", (caddr_t)at_otheraliases_pushbutton_cb },

/* dialogs */		{ "pin_dialog_cb", (caddr_t)pin_dialog_cb },
			{ "chpin_dialog_cb", (caddr_t)chpin_dialog_cb },
			{ "addalias_dialog_cb", (caddr_t)addalias_dialog_cb },
			{ "ok_dialog_cb", (caddr_t)ok_dialog_cb },
			{ "error_dialog_cb", (caddr_t)error_dialog_cb },
			{ "warning_dialog_cb", (caddr_t)warning_dialog_cb },
			{ "question_dialog_cb", (caddr_t)question_dialog_cb },
			{ "debug_dialog_cb", (caddr_t)debug_dialog_cb },
			{ "viewer_dialog_cb", (caddr_t)viewer_dialog_cb },
			{ "otheraliases_dialog_cb", (caddr_t)otheraliases_dialog_cb },
			{ "objectselection_dialog_cb", (caddr_t)objectselection_dialog_cb },
			{ "file_dialog_cb", (caddr_t)file_dialog_cb },
			{ "file_selection_dialog_cb", (caddr_t)file_selection_dialog_cb },
			{ "file_object_selection_dialog_cb", (caddr_t)file_object_selection_dialog_cb },
			{ "toolquit_dialog_cb", (caddr_t)toolquit_dialog_cb },
			{ "clipboard_dialog_cb", (caddr_t)clipboard_dialog_cb },
			{ "config_dialog_cb", (caddr_t)config_dialog_cb },
			{ "config_accessdir_toggle_cb", (caddr_t)config_accessdir_toggle_cb },
			{ "config_useeditor_toggle_cb", (caddr_t)config_useeditor_toggle_cb },

/* misc */		{ "check_passwd_cb", (caddr_t)check_passwd_cb },
			{ "textfield_focus_cb", (caddr_t)textfield_focus_cb }
		};


/*
 ************************
 *	Main program	*
 ************************
 */

int
main(cnt, parm)
	int 		cnt;
	char 		**parm;
{


	sectool_cnt = cnt;
	sectool_parm = parm;



	/*
	 *	Initialize Mrm/Xt/Context/Display/Arguments
	 */
	init_toolkits();


	/*
	 *	Create Shells
	 */
	create_shell();


	/*
	 *	Read application resources from XMst / as command line options
	 */
	get_resources();


	/*
	 *	Initialize UIL hierarchy
	 */
	init_uil();


	/*
	 *	Initialize SecuDE interface
	 */
	init_secude();


	/*
	 *	Open PSE
	 */
	st_open_pse();


	/*
	 *	Fill Base Widgets
	 */
	st_fill_widgets();

	
	/*
	 *	Not in Debug Mode: Drop stacked errors that happened during initialization
	 */
	if (!Debug) {

		aux_free_error();
		XmTextSetString(STWIDGET("*debug_text"), "");
	}


	/*
	 *  Loop and process events
	 */
	XtAppMainLoop(applicationContext);
}



static
void
init_toolkits()
{

	/*
	 *	Init Mrm/Xt/Context/Display/Arguments
	 */


	XrmInitialize();
	XtToolkitInitialize();
	MrmInitialize();
	
	applicationContext = XtCreateApplicationContext();

	if (! (display = XtOpenDisplay(	applicationContext, NULL,
					"xmst",    		
    					"XMst", 
    					NULL, 0,
    					&sectool_cnt, sectool_parm	)) ) {

		fprintf(stderr, "Unable to open display\n");
		exit_sectool();
	}

	screen = DefaultScreenOfDisplay(display);
}


static
void
create_shell()
{

	XtSetArg(args[0], XmNtitle, "SecuDE Tool 1.0  for SecuDE Release 4.3  (c)GMD Darmstadt 1994");

	applicationShell = 	XtAppCreateShell(	"stMainShell", NULL,
							applicationShellWidgetClass,
							display,
							args, 1				);

	/* Alias-/DirTool have their own procedures to handle destruction/recreation */
	atToplevel = (Widget)NULL;
	dtToplevel = (Widget)NULL;

}



static
void
get_resources()
{
	ApplResType		applResources;

	
	/* read all resources */
	XtGetApplicationResources(	applicationShell,
					&applResources,
					xmstResources, XtNumber(xmstResources),
					(Arg *)NULL, 0					);


	/* set global variables */

	busy_cursor =		applResources.busycursor;
	focus_color =		applResources.focuscolor;
	active_list_color =	applResources.activelistcolor;


}



static
void
init_uil()
{
	Pixel			fgPixel, bgPixel;
	XtPointer		pointer;
	MrmCode			mrm_type;


	/*
	 *	Open UIL Hierarchy/Register Names
	 */

	if (MrmOpenHierarchy/*PerDisplay( 		display,*/(
						1,
						applicationUidVector,
						NULL,	
			    			&applicationHierarchy		) != MrmSUCCESS) {

		fprintf(stderr, "Can't find file xmst.uid in $UIDPATH, $XAPPLRESDIR/uid, or $HOME[/uid] !\n");
		exit_sectool();
	}

	if (MrmRegisterNames(	applicationRegvec,
				XtNumber(applicationRegvec)	) != MrmSUCCESS) {

    		fprintf(stderr, "Error in xmst.uid: Can't register names.\n");
		exit_sectool();
	}


	/*
	 *	Check Version of UID file
	 */

	if (MrmFetchLiteral(	applicationHierarchy,
				"xmstUIDVersion",
				display,
				(XtPointer) &uid_version,
				&mrm_type			) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch string xmstUIDVersion !\n");
		exit_sectool();
	}
	if (mrm_type != MrmRtypeChar8) {
	
		fprintf(stderr, "Error in xmst.uid: Value xmstUIDVersion is no string !\n");
		exit_sectool();
	}

	if (strcmp(uid_version, XMST_UID_VERSION)) {
	
		fprintf(stderr, "Error in xmst.uid: Version %s found, needed version is %s !\n",
				 uid_version, XMST_UID_VERSION);
		exit_sectool();
	}

	/*
	 *	Fetch Main Shell, Colors, Pixmaps
	 */

	if (MrmFetchWidget(	applicationHierarchy,
				"stMainShell",
				applicationShell,
				&stMainShell,
				&opaqueClass			) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch stMainShell !\n");
		exit_sectool();
	}


	/*
	 *	Application Icon Bitmaps
	 */
	if (MrmFetchColorLiteral(	applicationHierarchy,
					"appsiconForegroundPixel",
					display,
					(Colormap)NULL,
					&fgPixel		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch appsiconForegroundPixel !\n");
		exit_sectool();
	}
					
	if (MrmFetchColorLiteral(	applicationHierarchy,
					"appsiconBackgroundPixel",
					display,
					(Colormap)NULL,
					&bgPixel		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch appsiconBackgroundPixel !\n");
		exit_sectool();
	}
					
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"sectoolIcon",
					screen,
					display,
					fgPixel,
					bgPixel,
					&sectoolPixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch sectoolIcon !\n");
		exit_sectool();
	}
		
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"aliastoolIcon",
					screen,
					display,
					fgPixel,
					bgPixel,
					&aliastoolPixmap	) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch aliastoolIcon !\n");
		exit_sectool();
	}
		
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"dirtoolIcon",
					screen,
					display,
					fgPixel,
					bgPixel,
					&dirtoolPixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch dirtoolIcon !\n");
		exit_sectool();
	}
		


	/*
	 *	Status Picture Bitmaps
	 */
	if (MrmFetchColorLiteral(	applicationHierarchy,
					"pictureForegroundPixel",
					display,
					(Colormap)NULL,
					&fgPixel		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch pictureForegroundPixel !\n");
		exit_sectool();
	}
					
	if (MrmFetchColorLiteral(	applicationHierarchy,
					"pictureBackgroundPixel",
					display,
					(Colormap)NULL,
					&bgPixel		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch pictureBackgroundPixel !\n");
		exit_sectool();
	}
					
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"scPicture",
					screen,
					display,
					fgPixel,
					bgPixel,
					&scPixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch scPicture !\n");
		exit_sectool();
	}
		
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"psePicture",
					screen,
					display,
					fgPixel,
					bgPixel,
					&psePixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch psePicture !\n");
		exit_sectool();
	}
		
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"scpsePicture",
					screen,
					display,
					fgPixel,
					bgPixel,
					&scpsePixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch scpsePicture !\n");
		exit_sectool();
	}
		
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"onepairPicture",
					screen,
					display,
					fgPixel,
					bgPixel,
					&onepairPixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch onepairPicture !\n");
		exit_sectool();
	}
		
	if (MrmFetchIconLiteral(	applicationHierarchy,
					"twopairsPicture",
					screen,
					display,
					fgPixel,
					bgPixel,
					&twopairsPixmap		) != MrmSUCCESS) {

		fprintf(stderr, "Error in xmst.uid: Can't fetch twopairsPicture !\n");
		exit_sectool();
	}
		

	/*
	 *	Add check_passwd_cb & textfield_focus_cb Callbacks to some Dialog Textfields
	 */
	
	/* PIN */
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*pin_dialog"), XmDIALOG_TEXT),
			XmNmodifyVerifyCallback, check_passwd_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*pin_dialog"), XmDIALOG_TEXT),
			XmNactivateCallback, check_passwd_cb,
			NULL								);
   	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*pin_dialog"), XmDIALOG_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*pin_dialog"), XmDIALOG_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);

	/* Change PIN */
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*chpin_dialog"), XmDIALOG_TEXT),
			XmNmodifyVerifyCallback, check_passwd_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*chpin_dialog"), XmDIALOG_TEXT),
			XmNactivateCallback, check_passwd_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*chpin_dialog"), XmDIALOG_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*chpin_dialog"), XmDIALOG_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);
			
	/* Add Alias */		
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*addalias_dialog"), XmDIALOG_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*addalias_dialog"), XmDIALOG_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);
			
	/* Object Selection */		
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*objectselection_dialog"), XmDIALOG_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*objectselection_dialog"), XmDIALOG_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);
			
	/* File/Object Selection */		
    	XtAddCallback(	XmFileSelectionBoxGetChild(STWIDGET("*file_selection_box"), XmDIALOG_FILTER_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmFileSelectionBoxGetChild(STWIDGET("*file_selection_box"), XmDIALOG_FILTER_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmFileSelectionBoxGetChild(STWIDGET("*file_selection_box"), XmDIALOG_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmFileSelectionBoxGetChild(STWIDGET("*file_selection_box"), XmDIALOG_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*file_object_selection_box"), XmDIALOG_TEXT),
			XmNfocusCallback, textfield_focus_cb,
			NULL								);
    	XtAddCallback(	XmSelectionBoxGetChild(STWIDGET("*file_object_selection_box"), XmDIALOG_TEXT),
			XmNlosingFocusCallback, textfield_focus_cb,
			NULL								);
			
}



static
void
open_sectool()
{
	XEvent		event;
		

	XtManageChild(stMainShell);
		
	XtSetArg(args[0], XmNiconPixmap, sectoolPixmap);
	XtSetValues(applicationShell, args, 1);
	XtRealizeWidget(applicationShell);

	XmUpdateDisplay(applicationShell);

}



static
void
init_secude()
{

	/* 
	 *      Set fix paths
	 */

	if (! (unix_home = getenv("HOME")) || !strlen(unix_home))  {

		fprintf(stderr, "Can't get $HOME from environment !\n");
		exit_sectool();
	}

	strcpy(editor_tempfile, unix_home);
	if (editor_tempfile[strlen(editor_tempfile) - 1] != PATH_SEPARATION_CHAR)
		strcat(editor_tempfile, PATH_SEPARATION_STRING);
	strcat(editor_tempfile, XMST_TEMP_FILE);

	strcpy(config_rcfile, unix_home);
	if (config_rcfile[strlen(config_rcfile) - 1] != PATH_SEPARATION_CHAR)
		strcat(config_rcfile, PATH_SEPARATION_STRING);
	strcat(config_rcfile, XMST_CONFIG_FILE);

#ifdef AFDBFILE
	strcpy(system_aliasfile, AFDBFILE);
	strcat(system_aliasfile, ALIASFILE);
#endif


	/* 
	 *      Set non-configurable options
	 */

	accept_alias_without_verification = FALSE;		
	MF_check = FALSE;
	af_verbose = sec_verbose = FALSE;
	sec_debug = 0;
	alias_save_required = FALSE;
		
	/* 
	 *      Pre-Set configurable options
	 */

	auto_save_configuration = FALSE;
	Debug = FALSE;
	confirm_overwrite = FALSE;
	external_editor = CNULL;
	
#ifdef SCA
	auto_detect_sct = FALSE;
	SC_verify = SC_encrypt = FALSE;
#endif /* SCA */
	
	af_chk_crl = FALSE;
	af_FCPath_is_trusted = TRUE;
	chk_PEM_subordination = TRUE;
	random_from_pse = TRUE;
	
	af_access_directory = FALSE;

#ifdef X500
	af_dir_authlevel = DBA_AUTH_SIMPLE;
	af_dir_dsaname = CNULL;
	af_dir_tailor = CNULL;
#endif /* X500 */


	while ( (opt = getopt(sectool_cnt, sectool_parm, "chv")) != -1 ) switch(opt) {

		case 'h':
			usage(LONG_HELP);
			continue;
		case 'c':
			start_with_configuration = TRUE;
			continue;
		/*
		 *	HIDDEN VERBOSE OPTION... TO BE REMOVED !
		 */
		case 'v':
			verbose = sectool_verbose = af_verbose = sec_verbose = MF_check = TRUE;
			sec_debug = 2;
			break;
		default:
			usage(SHORT_HELP);
	}                    

	/*
	 *	Manage & Realize sectool
	 */
	open_sectool();


	/*
	 *	Configuration
	 */
	
	config_database = XrmGetFileDatabase(config_rcfile);
	get_configuration();					/* get .xmstrc */
	if (start_with_configuration) {
		if (config_dialog_open()) {
			get_configuration();			/* get changes */
		}
	}
	
	if(!pse_name)  {

		if(ca_dir) pse_name = DEF_CAPSE;
		else pse_name = DEF_PSE;
	}

	if(ca_dir)   {

		pse_path = (char *)malloc(strlen(ca_dir)+strlen(pse_name) + 2);
		strcpy(pse_path, ca_dir);
		if(pse_path[strlen(pse_path) - 1] != '/') strcat(pse_path, "/");
		strcat(pse_path, pse_name);

	} else  {

		pse_path = (char *)malloc(strlen(pse_name) + 2);
		strcpy(pse_path, pse_name);
	}
}



/*
 *	SecTool usage help
 */
static
void
usage(help)
	int     help;
{

	aux_fprint_version(stderr);

	fprintf(stderr, "xmst: Maintain your PSE\n\n\n");
	fprintf(stderr, "Description:\n\n"); 
	fprintf(stderr, "'xmst/SecTool' is, like psemaint, a maintenance program which can be used by both\n");
	fprintf(stderr, "certification authority administrators and users for the purpose\n");
	fprintf(stderr, "of maintaining their PSEs. This includes moving information (e.g. keys,\n");
	fprintf(stderr, "certificates, revocation lists etc.) from Unix files or a X.500 Directory\n");
	fprintf(stderr, "into the PSE and vice versa, generating keys, changing PINs, displaying\n"); 
	fprintf(stderr, "the content of the PSE, and maintaining the user's aliases. In contrast\n");
        fprintf(stderr, "to psemaint, which is line-oriented, sectool is an Motif(tm) tool.\n\n\n");

        fprintf(stderr, "usage:\n\n");
	fprintf(stderr, "xmst [-ch]\n\n");

        if(help == LONG_HELP) {

        fprintf(stderr, "with:\n\n");
        fprintf(stderr, "-c          Start SecTool with Configuration Dialog.\n");
        fprintf(stderr, "-h          Print this help text.\n");
        }

        exit(0);

} 

