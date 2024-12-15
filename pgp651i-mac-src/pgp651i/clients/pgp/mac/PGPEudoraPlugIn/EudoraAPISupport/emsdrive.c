/* ======================================================================	Test driver for Eudora EMSAPI version v3	Filename:			EMSDrive.c	Last Edited:		March 7, 1997	Authors:			Laurence Lundblade, Scott Gruby,						Myra Callen, Bob Fronabarger	Copyright:			1995, 1996 QUALCOMM Inc.	Technical support:	<emsapi-info@qualcomm.com>*/#include <string.h>#include <stdio.h>#include <Icons.h>#include "emsapi-mac.h"#include "copycat.h"#include "mimetype.h"#include "usertrans.h"void WriteZero(void *pointer,long size);#define Zero(x) WriteZero(&(x),sizeof(x))#define SetSize(x)	(x).size = sizeof(x)void WriteZero(void *pointer,long size){	Byte *u = (Byte*) pointer;		while (size--)		*u++ = '\0';}/*****************************************************************//*  G L O B A L S  and  C O N S T A N T S/*****************************************************************/#define kBaseResID			128#define kErrorALRTid		300#define kNoteALRTid			310#define kAboutALRTid		400#define mApple				1501#define iAbout				1#define iAboutPlugins		2#define mFile				1502#define iQuit				17#define mEdit				1503#define mTranslators		220#define mMailbox			1504#define mMessage			1505#define mAttach				222#define mTransfer			1506#define mSpecial			1507#define mSettings			223#define mWindow				1514#define	kHeaderOffset		48#define kDummyHOffset		233#define kTransHeightOffset 	60#define kNumberofButtons	2#define kTransTopOffset		4#define kSecondButtonOffset	30#define kButHeight			18#define kTransButWidth		120#define kTestButWidth		96#define	kIconHOffset		4#define kButHOffset			42#define	kTextHOffset		168#define kWindowWidth		528#define kOK					1#define kCancel				2#define kFactorySettings	3#define kProgressDlogID		500#define kMimeDlogID			600#define kAddressDlogID		700#define kDefModuleIcon		10128#define kDefTransIcon		10129Boolean gDone = false;unsigned char *translatorErrors[] = {	"\pEMSR_OK",	"\pEMSR_UNKNOWN_FAIL",	"\pEMSR_CANT_TRANS",	"\pEMSR_INVALID_TRANS",	"\pEMSR_NO_ENTRY",	"\pEMSR_NO_INPUT_FILE",	"\pEMSR_CANT_CREATE",	"\pEMSR_TRANS_FAILED",	"\pEMSR_INVALID",	"\pEMSR_NOT_NOW",	"\pEMSR_NOW", 	"\pEMSR_ABORTED"};/* Keep track of translatorButtons on translator modules window */typedef struct TranslatorButtons {	emsTranslator			trans;	ControlHandle			ctlH1, ctlH2;	void					(*proc1)(emsTranslator*);	void					(*proc2)(emsTranslator*);} TranslatorButtons;/* Module Info from init call */static short				gModuleID, gOurResRefNum, gSpecialMenuLen;static long					gContext;static Handle				gModuleGlobalsH;static ControlHandle		gMimeButH, gAddressButH, gContextPop;static emsMIMEtype		  **gMimeTypeH;static StringHandle		  **gAddressesH;static WindowPtr			gDebugWindow = nil;static emsPluginInfo		gPluginInfo;static emsMailConfig		gMailConfig;static TranslatorButtons   *gTranslBtns;static EMSProgressUPP		gProgressUPP;static DialogPtr			gProgressDlg;static emsResultStatus		gResultStatus;/*****************************************************************//*  P R O T O T Y P E S/*****************************************************************/void  	main();void  	ToolboxInit();void	MenuBarInit();void 	DoInitialize();void	CleanUp();void	EventLoop();void	DoEvent(EventRecord *eventPtr);void	HandleMouseDown(EventRecord *eventPtr);void	HandleMenuChoice(long menuChoice);void 	DoUpdate(WindowPtr theWindow);void	DrawWindow(WindowPtr window);void	DoTranslation(emsTranslator *trans);void	CanTranslation(emsTranslator *trans);void	NoTranslation(emsTranslator *trans);void	DoMimeDlog();void	RestoreMimeDefaults(DialogPtr theDialog);void	SaveMimeSettingsToFile(DialogPtr theDialog);void	BuildNewMimeData();void	DoAddressDlog();void	RestoreAddressDefaults(DialogPtr theDialog);void	SaveAddressSettingsToFile(DialogPtr theDialog);void	SetupAddressDialog(DialogPtr theDialog);void	GetAddressChanges(DialogPtr theDialog);void	InitAddressData();short	ContextToMenuItem(long context);long	MenuItemToContext(short item);void	DoMessage(long status);void	DoError(long error, Str63 message);void	GetStringHdl(short id, Str255 theStr);void 	SetWindowFont(DialogPtr theDialog, short fontNum, short fontSize, Style fontStyle);short	ProgressProc(emsProgressData *prog);/*****************************************************************//*  M A I N  P R O G R A M/*****************************************************************//* ----------------------------------------------------------------------	Application entry */void main(void){	ToolboxInit();	MenuBarInit();	DoInitialize();	EventLoop();	CleanUp();}/* ----------------------------------------------------------------------	Standard toolbox initialization */void ToolboxInit(){	MaxApplZone();	InitGraf(&qd.thePort);	InitFonts();	InitWindows();	InitMenus();	TEInit();	InitDialogs(0L);	FlushEvents(everyEvent, 0);	InitCursor();}/* ----------------------------------------------------------------------	Setup menu bar */void MenuBarInit(void){	Handle			menuBar;	MenuHandle		menu;		menuBar = GetNewMBar(kBaseResID);	SetMenuBar(menuBar);	menu = GetMenuHandle(mApple);	AppendResMenu(menu, 'DRVR');	DrawMenuBar();}/* ----------------------------------------------------------------------	Put up window with module/translator info */void DoInitialize(void){	Rect			r;	short			i, v, api_version;	long			theResult;	OSErr			err;	MenuHandle		translMenu = nil, attachMenu, specialMenu;	Str255  		message;		/* get our resource ref num */	gOurResRefNum = CurResFile();	/* Pretend we're like the component manager, handle the globals storage */	gModuleGlobalsH = NewHandle(0);		SetSize(gMailConfig);	gMailConfig.configDir.vRefNum = -1;	gMailConfig.configDir.parID = 1;	gMailConfig.configDir.name[0] = '\0';	gMailConfig.userAddr.size = sizeof(emsAddress);	// etc...		Zero(gPluginInfo);	SetSize(gPluginInfo);		Zero(gResultStatus);	SetSize(gResultStatus);		/* create callback function pointer for progress function */	gProgressUPP = NewEMSProgressProc(ProgressProc);		/* Get this plugin's version. */	theResult = ems_plugin_version(gModuleGlobalsH, &api_version);	if (theResult != EMSR_OK)		api_version = -1;	/* Initialize this module and get the module information. */	theResult = ems_plugin_init(gModuleGlobalsH, EMS_VERSION, &gMailConfig, &gPluginInfo);	DoError(theResult, "\pplugin_init failed");	if (gPluginInfo.icon == nil) // Substitute our dummy icon if plugin doesn't have one		err = GetIconSuite(&gPluginInfo.icon, kDefModuleIcon, svAllAvailableData);	if (gPluginInfo.numTrans > 0) {		translMenu = NewMenu(mTranslators, "\pTranslators");		InsertMenu(translMenu, hierMenu);	}		attachMenu = NewMenu(mAttach, "\pAttach");	if (gPluginInfo.numAttachers == 0)		AppendMenu(attachMenu, "\p(None installed");	InsertMenu(attachMenu, hierMenu);	#if EMS_HAS_PLUGIN_CONFIG	{	MenuHandle		settingsMenu;		settingsMenu = NewMenu(mSettings, "\pSettings");	AppendMenu(settingsMenu, "\pX");	CopyPP(*gPluginInfo.desc, message);	CatPP("\p...", message);	SetMenuItemText(settingsMenu, 1, message);	InsertMenu(settingsMenu, hierMenu);	}#endif	if (gPluginInfo.numSpecials > 0) {		specialMenu = GetMenu(mSpecial);		AppendMenu(specialMenu, "\p(-");		gSpecialMenuLen = CountMItems(specialMenu);	}		BuildNewMimeData();	InitAddressData();	gContext = EMSF_Q4_TRANSMISSION;	// EMSF_ON_REQUEST;		v = gPluginInfo.numTrans * kTransHeightOffset + kHeaderOffset;	SetRect(&r, 10, 50, 10 + kWindowWidth, 50 + v);	gDebugWindow = NewCWindow(nil, &r, "\pTranslator Debugger", false, 0, (WindowPtr) -1L, false, 0);	SetPort(gDebugWindow);		/* Buttons for dummy testing information */	SetRect(&r, kWindowWidth - kTestButWidth - 16, 4, kWindowWidth - 16, 4 + kButHeight);	gAddressButH = NewControl(gDebugWindow, &r, "\pAddress", true, 0, 0, 1, pushButProc, 0);	OffsetRect(&r, 0, kButHeight + 4);	gMimeButH = NewControl(gDebugWindow, &r, "\pMime Type", true, 0, 0, 1, pushButProc, 0);	SetRect(&r, kDummyHOffset - 5, 27, kDummyHOffset + 180 - 5, 27 + 16);	i = ContextToMenuItem(gContext);	gContextPop = NewControl(gDebugWindow, &r, "\pContext:", true, i, 2020, 48, 1018, 0);	SetControlValue(gContextPop, i);#if EMS_HAS_TRANSLATOR_INFO	// create two buttons for every translator, save the handle in the array	gTranslBtns = (TranslatorButtons*) NewPtr(sizeof(TranslatorButtons) * gPluginInfo.numTrans);	for (i = 0; i < gPluginInfo.numTrans; i++) {		TranslatorButtons	translBtn;		translBtn.trans.size = sizeof(emsTranslator);		translBtn.trans.id = i + 1;		translBtn.trans.type = 0;		translBtn.trans.flags = 0;		translBtn.trans.desc = nil;		translBtn.trans.icon = nil;		translBtn.trans.properties = nil;		theResult = ems_translator_info(gModuleGlobalsH, &translBtn.trans);		if (theResult == EMSR_OK) {			v = (i * kTransHeightOffset) + kHeaderOffset + kTransTopOffset;			SetRect(&r, kButHOffset, v, kButHOffset + kTransButWidth, v + kButHeight);			translBtn.ctlH1 = NewControl(gDebugWindow, &r, "\pCan Trans File", true, 0, 0, 1, pushButProc, 0);						OffsetRect(&r, 0, kSecondButtonOffset - kTransTopOffset);	// 26			translBtn.ctlH2 = NewControl(gDebugWindow, &r, "\pTranslate File", true, 0, 0, 1, pushButProc, 0);#if EMS_HAS_CAN_TRANSLATE			translBtn.proc1 = CanTranslation;#else			translBtn.proc1 = NoTranslation;#endif#if EMS_HAS_TRANSLATE_FILE			translBtn.proc2 = DoTranslation;#else			translBtn.proc2 = NoTranslation;#endif			gTranslBtns[i] = translBtn;						if (translBtn.trans.desc != nil) {				AppendMenu(translMenu, "\pX");				CopyPP(*translBtn.trans.desc, message);				SetMenuItemText(translMenu, CountMItems(translMenu), message);			}		}	}#endif#if EMS_HAS_ATTACHER_INFO	for (i = 0; i < gPluginInfo.numAttachers; i++) {		emsMenu		menuRec;				SetSize(menuRec);		menuRec.id = i;		menuRec.desc = nil;		theResult = ems_attacher_info(gModuleGlobalsH, &menuRec);		AppendMenu(attachMenu, "\pX");		if (theResult == EMSR_OK && menuRec.desc != nil) {			CopyPP(*menuRec.desc, message);			DisposeHandle((Handle) menuRec.desc);		}		else			CopyPP("\pError", message);		SetMenuItemText(attachMenu, CountMItems(attachMenu), message);	}#endif#if EMS_HAS_SPECIAL_INFO	for (i = 0; i < gPluginInfo.numSpecials; i++) {		emsMenu		menuRec;				SetSize(menuRec);		menuRec.id = i;		menuRec.desc = nil;		theResult = ems_special_info(gModuleGlobalsH, &menuRec);		AppendMenu(specialMenu, "\pX");		if (theResult == EMSR_OK && menuRec.desc != nil) {			CopyPP(*menuRec.desc, message);			DisposeHandle((Handle) menuRec.desc);		}		else			CopyPP("\pError", message);		SetMenuItemText(specialMenu, CountMItems(specialMenu), message);	}#endif	ShowWindow(gDebugWindow);}/* ----------------------------------------------------------------------	Clean up globals before quit*/void CleanUp(void) {	TranslatorButtons	translator;	short				i;		if (gModuleGlobalsH != nil) {		ems_plugin_finish(gModuleGlobalsH);		DisposeHandle(gModuleGlobalsH);		gModuleGlobalsH = nil;	}	if (gTranslBtns != nil) {		for (i = 0; i < gPluginInfo.numTrans; i++) {			translator = gTranslBtns[i];			if (translator.trans.desc != nil)				DisposeHandle((Handle) translator.trans.desc);			if (translator.trans.icon != nil)				DisposeIconSuite(translator.trans.icon, true);		}		DisposePtr((Ptr) gTranslBtns);	}	DisposeRoutineDescriptor(gProgressUPP);}#pragma mark -/*****************************************************************//*  E V E N T  H A N D L I N G/*****************************************************************/\/* ----------------------------------------------------------------------	Do event loop */void EventLoop(void){			EventRecord		event;		while (!gDone) {		WaitNextEvent(everyEvent, &event, 30, nil);		DoEvent(&event);	}}/* ----------------------------------------------------------------------	Handle event */void DoEvent(EventRecord *theEvent){	char		theChar;		switch (theEvent->what) {		case mouseDown:			HandleMouseDown(theEvent);			break;		case keyDown:		case autoKey:			theChar = theEvent->message & charCodeMask;			if (theEvent->modifiers & cmdKey)				HandleMenuChoice(MenuKey(theChar));			break;		case updateEvt:			DoUpdate((WindowPtr) theEvent->message);			break;	}}/* ----------------------------------------------------------------------	Handle mouse down events */void HandleMouseDown(EventRecord *eventPtr){	WindowPtr			window;	Point				mousePt;	short				i, thePart, part;	ControlHandle   	whichControl;	ControlActionUPP	actionUPP;	TranslatorButtons	translBtn;		thePart = FindWindow(eventPtr->where, &window);	switch (thePart) {		case inMenuBar:			HandleMenuChoice(MenuSelect(eventPtr->where));			HiliteMenu(0);			break;		case inSysWindow:			SystemClick(eventPtr, window);			break;		case inContent:			if (window == gDebugWindow) {				SelectWindow(window);				mousePt = eventPtr->where;				GlobalToLocal(&mousePt);				whichControl = nil;				part = FindControl(mousePt, window, &whichControl);				if (whichControl == nil)					break;				actionUPP = (whichControl == gContextPop) ? (ControlActionUPP) -1 : nil;				part = TrackControl(whichControl, mousePt, actionUPP);				if (part == kControlButtonPart) {					HiliteControl(whichControl, 255);					if (whichControl == gAddressButH)						DoAddressDlog();					else if (whichControl == gMimeButH)						DoMimeDlog();					else if (whichControl == gContextPop) {						part = TrackControl(gContextPop, mousePt, (ControlActionUPP) -1);						if (part != 0) {							part = GetControlValue(gContextPop);							part++;						}					}					else {						for (i = 0; i < gPluginInfo.numTrans; i++) {							translBtn = gTranslBtns[i];							if (whichControl == translBtn.ctlH1) {								(*translBtn.proc1)(&translBtn.trans);								break;							}							else if (whichControl == translBtn.ctlH2) {								(*translBtn.proc2)(&translBtn.trans);								break;							}						}					}					HiliteControl(whichControl, 0);				}				else if (part == kControlLabelPart) {					part = GetControlValue(gContextPop);					gContext = MenuItemToContext(part);				}			}			break;		case inDrag:			DragWindow(window, eventPtr->where, &qd.screenBits.bounds);			break;	}}/* ----------------------------------------------------------------------	Handle menu events */void HandleMenuChoice(long menuChoice){	short			menu, item;	MenuHandle		appleMenu;	Str255			theStr;		if (menuChoice != 0) {		menu = HiWord(menuChoice);		item = LoWord(menuChoice);		switch (menu) {			case mApple:				switch (item) {					case iAbout:						Alert(kAboutALRTid, nil);						break;					case iAboutPlugins:						break;					default:						appleMenu = GetMenuHandle(mApple);						GetMenuItemText(appleMenu, item, theStr);						OpenDeskAcc(theStr);				}				break;			case mFile:				if (item == iQuit)					gDone = true;				break;#if EMS_HAS_TRANSLATOR_INFO			case mTranslators:	// mEdit				{				TranslatorButtons	translBtn = gTranslBtns[item - 1];				(*translBtn.proc2)(&translBtn.trans);	// Do Translation				}				break;#endif#if EMS_HAS_ATTACHER_HOOK			case mAttach:		// mMessage				{				emsMenu			menuRec;				FSSpec			dir;				emsDataFileH	attachH = nil;				long			status, numAttach = 0;								SetSize(menuRec);				menuRec.id = item - 1;				menuRec.desc = nil;				Zero(dir);				status = ems_attacher_hook(gModuleGlobalsH, &menuRec, &dir, &numAttach, &attachH);				if (attachH != nil) {					DisposeHandle((Handle) attachH);					NumToString(numAttach, theStr);					ParamText(theStr, "\p files attached", "\p", "\p");					Alert(kNoteALRTid, nil);				}				DoError(status, "\pattacher_hook failed");				}				break;#endif#if EMS_HAS_SPECIAL_HOOK			case mSpecial:				{				emsMenu		menuRec;				long		status;								SetSize(menuRec);				menuRec.id = item - gSpecialMenuLen - 1;				menuRec.desc = nil;				status = ems_special_hook(gModuleGlobalsH, &menuRec);				DoError(status, "\pspecial_hook failed");				}				break;#endif#if EMS_HAS_PLUGIN_CONFIG			case mSettings:				{				long		status2;								status2 = ems_plugin_config(gModuleGlobalsH, &gMailConfig);				DoError(status2, "\pplugin_config failed");				}				break;#endif		}	}}/* ----------------------------------------------------------------------	Do window update*/void DoUpdate(WindowPtr theWindow){	GrafPtr		savePort;		GetPort(&savePort);	SetPort(theWindow);	BeginUpdate(theWindow);	if (theWindow == gDebugWindow)		DrawWindow(theWindow);	EndUpdate(theWindow);	SetPort(savePort);}#pragma mark -/*****************************************************************//*  T R A N S L A T O R S/*****************************************************************//* ----------------------------------------------------------------------	Display the translators in a particular module with translatorButtons for	invoking operatations*/void DrawWindow(WindowPtr window){	short	   	api_version;#if EMS_HAS_TRANSLATOR_INFO	short		i;#endif	Rect		theIconRect;	Str63	   	theStr;		TextSize(10);	UpdateControls(window, window->visRgn);	MoveTo(kDummyHOffset, 16);	DrawString("\pSet Dummy Testing Information:");		// Module Icon	SetRect(&theIconRect, kIconHOffset, 5, kIconHOffset + 32, 5 + 32);	if (gPluginInfo.icon && *gPluginInfo.icon)		PlotIconSuite(&theIconRect, atAbsoluteCenter, ttNone, gPluginInfo.icon);		// Module Name	MoveTo(kButHOffset, 12);	DrawString("\pModule:");	if (gPluginInfo.desc != nil) {		CopyPP(*gPluginInfo.desc, theStr);		TextFace(bold);		MoveTo(kButHOffset, 24);		DrawString(theStr);		TextFace(normal);	}		// API version	MoveTo(kButHOffset, 37);	DrawString("\pAPI Version: ");	ems_plugin_version(gModuleGlobalsH, &api_version);	NumToString(api_version, theStr);	DrawString(theStr);	// Draw line	MoveTo(0, kHeaderOffset);	LineTo(600, kHeaderOffset);	#if EMS_HAS_TRANSLATOR_INFO	// Go through the translators in the module	for (i = 0; i < gPluginInfo.numTrans; i++) {		short	   		j, theType, v, err;		Rect			textBox;		unsigned long	mask;		emsTranslator	translInfo;		char			s[512];		translInfo = gTranslBtns[i].trans;		v = i * kTransHeightOffset + kHeaderOffset + kTransTopOffset;				/* Draw the icon */		SetRect(&theIconRect, kIconHOffset, v, kIconHOffset + 32, v + 32);		if (translInfo.icon == nil)		// substitute dummy icon if translator doesn't have one			err = GetIconSuite(&translInfo.icon, kDefTransIcon, svAllAvailableData);		if (translInfo.icon && *translInfo.icon)			PlotIconSuite(&theIconRect, atAbsoluteCenter, ttNone, translInfo.icon);		MoveTo(kTextHOffset, v + 9);		sprintf(s, "Translator # %d: ", i + 1);		DrawString(CtoPstr(s));		if (translInfo.desc != nil) {			CopyPP(*translInfo.desc, theStr);			DrawString(theStr);		}		/* The translator info details */		theType = (translInfo.type == EMST_NO_TYPE) ? 1 : (translInfo.type / 16)  + 1;		GetIndString(theStr, 2000, theType);		sprintf(s, "Type: %#s", theStr);		if (translInfo.flags & EMSF_REQUIRES_MIME)			strcat(s, ", wants MIME");		if (translInfo.flags & EMSF_GENERATES_MIME)			strcat(s, ", makes MIME");		if (translInfo.flags & EMSF_WHOLE_MESSAGE)			strcat(s, ", whole msg");		MoveTo(kTextHOffset, v + 23);		DrawString(CtoPstr(s));			/* Some of the options */		MoveTo(kTextHOffset, v + 37);		DrawString("\pContext:");		s[0] = '\0';		for (j = 0; j < 8; j++) {			mask = (1L << j);			if ((translInfo.flags & mask) == mask) {				if (strlen(s) != 0)					strcat(s, ", ");				GetIndString(theStr, 3000, j + 1);				strcat(s, p2cstr(theStr));			}		}		SetRect(&textBox, kTextHOffset + 42, v + 27, kTextHOffset + 350, v + 56);		CtoPstr(s);		TETextBox(s + 1, s[0], &textBox, teFlushLeft);		// Draw line		v = (i + 1) * kTransHeightOffset + kHeaderOffset;		MoveTo(kButHOffset, v);		LineTo(600, v);		gTranslBtns[i].trans = translInfo;	}#endif}	/* ----------------------------------------------------------------------	Actually do a translation */#if EMS_HAS_TRANSLATE_FILEvoid DoTranslation(emsTranslator *trans){	short   			theResult = EMSR_OK;   	StandardFileReply	getReply, putReply;	emsMIMEtypeH		transMimeType = nil;	emsDataFile			dataFileIn, dataFileOut;	EMSProgressUPP		progressUPP = nil;	StandardGetFile(nil, -1, nil, &getReply);	if (!getReply.sfGood)		return;	StandardPutFile("\pCreate the Output File", "\pOutputFile", &putReply);	if (!putReply.sfGood)		return;	DoUpdate(gDebugWindow);		// Eudora will call ems_can_translate before calling ems_translate. Here we are	// assuming you can translate for the purposes of testing your code. 	SetSize(dataFileIn); 	dataFileIn.context = gContext; 	dataFileIn.mimeInfo = gMimeTypeH;	dataFileIn.header = nil; 	dataFileIn.file = getReply.sfFile;	dataFileOut = dataFileIn;	dataFileOut.mimeInfo = nil;	//	HandToHand((Handle*) &dataFileOut.mimeInfo); // duplicate 	dataFileOut.file = putReply.sfFile;#if EMS_HAS_QUEUED_PROPERTIES	if (gContext == EMSF_Q4_TRANSMISSION) {	// pretend user is toggling icon on		long	selected = true;				theResult = ems_queued_properties(gModuleGlobalsH, trans, &selected);		if (!selected) {			DoMessage(EMSR_DATA_UNCHANGED);			return;		}	}#endif	if (putReply.sfReplacing)		FSpDelete(&putReply.sfFile);	FSpCreate(&putReply.sfFile, 'ttxt', 'TEXT', smSystemScript);	gProgressDlg = nil;	if (gContext == EMSF_ON_ARRIVAL || gContext == EMSF_Q4_TRANSMISSION) {		gProgressDlg = GetNewDialog(kProgressDlogID, nil, (WindowPtr) -1L);	}	theResult = ems_translate_file(gModuleGlobalsH, trans,				   &dataFileIn, gProgressUPP, &dataFileOut, &gResultStatus);	if (gProgressDlg != nil) {		DisposeDialog(gProgressDlg);	}#if EMS_HAS_QUEUED_PROPERTIES	if (gContext == EMSF_Q4_TRANSMISSION) {		if (trans->properties != nil) {			DisposeHandle((Handle) trans->properties);			trans->properties = nil;		}	}#endif	InvalRgn(gDebugWindow->visRgn);	DoUpdate(gDebugWindow);	DoMessage(theResult);	// Report how it all went}#endif/* ----------------------------------------------------------------------	Test if we can translate */#if EMS_HAS_CAN_TRANSLATEvoid CanTranslation(emsTranslator *trans){	long				theResult;   	StandardFileReply	reply;   	emsDataFile			dataFile;	StandardGetFile(nil, -1, nil, &reply);	if (!reply.sfGood)		return;	 	// Call the translator in the module 	SetSize(dataFile); 	dataFile.context = gContext; 	dataFile.mimeInfo = gMimeTypeH;	// ?	dataFile.header = nil;			// ? 	dataFile.file = reply.sfFile;	theResult = ems_can_translate(gModuleGlobalsH, trans, &dataFile, &gResultStatus);	InvalRgn(gDebugWindow->visRgn);	DoUpdate(gDebugWindow);		// redraw window after calling translator	DoMessage(theResult);		// Report how it all went}#endif/* ----------------------------------------------------------------------	Test if we can translate */void NoTranslation(emsTranslator *trans){	DoMessage(EMSR_CANT_TRANS);}#pragma mark -/*****************************************************************//*  M I M E   D I A L O G/*****************************************************************/#define kMimeStrBaseID		401#define kMimeDefStrBaseID	501#define kBaseMimeItem		20#define kTypeItem			22#define kSubtypeItem		23#define kBaseParamItem		24/* ----------------------------------------------------------------------	Handle Mime dialog */void DoMimeDlog(void){	DialogPtr		theDialog;	DialogRecord	dlgRec;	GrafPtr			savePort;	short			itemType, i;	Handle			itemH;	Rect			itemRect;	Boolean			done = false;	short			itemHit;	Str255			theStr;		GetPort(&savePort);	UseResFile(gOurResRefNum);	theDialog = GetNewDialog(kMimeDlogID, &dlgRec, (WindowPtr) -1L);	if (theDialog == nil)		return;	for (i = 0; i < 14; i++) {		GetStringHdl(kMimeStrBaseID + i, theStr);		GetDialogItem(theDialog, kBaseMimeItem + i, &itemType, &itemH, &itemRect);		if (itemH != nil)			SetDialogItemText(itemH, theStr);	}	SelectDialogItemText(theDialog, kBaseMimeItem, 0, 32767);	SetPort(theDialog);	SetWindowFont(theDialog, geneva, 9, normal);	SetDialogDefaultItem(theDialog, kOK);	SetDialogCancelItem(theDialog, kCancel);	ShowWindow(theDialog);		while (!done) {		ModalDialog(nil, &itemHit);		switch (itemHit) {			case kOK:				done = true;				SaveMimeSettingsToFile(theDialog);				FreeMimeType(gMimeTypeH);				BuildNewMimeData();				break;			case kCancel:				done = true;				break;			case kFactorySettings:				RestoreMimeDefaults(theDialog);				break;			default:				break;		}	}	CloseDialog(theDialog);	SetPort(savePort);}/* ----------------------------------------------------------------------	Restore default mime values */void RestoreMimeDefaults(DialogPtr theDialog){	short		itemType, i;	Handle		itemH;	Rect		itemRect;	Str255		theStr;	for (i = 0; i < 4; i++) {		GetStringHdl(kMimeDefStrBaseID + i, theStr);		GetDialogItem(theDialog, kBaseMimeItem + i, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, theStr);	}	for (i = 4; i < 14; i++) {		GetDialogItem(theDialog, kBaseMimeItem + i, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, "\p");	}}/* ----------------------------------------------------------------------	Save dialog contents */void SaveMimeSettingsToFile(DialogPtr theDialog){	short			itemType, i;	Handle			itemH;	Rect			itemRect;	Str255			textStr;	StringHandle	strH;	UseResFile(gOurResRefNum);	for (i = 0; i < 14; i++) {		GetDialogItem(theDialog, kBaseMimeItem + i, &itemType, &itemH, &itemRect);		GetDialogItemText(itemH, textStr);				strH = GetString(kMimeStrBaseID + i);		if (strH != nil) {			SetString(strH, textStr);			ChangedResource((Handle) strH);			WriteResource((Handle) strH);			ReleaseResource((Handle) strH);		}	}	UpdateResFile(gOurResRefNum);}/* ----------------------------------------------------------------------	Setup mime dialog */void BuildNewMimeData(void){	short			i;	Str255			str1, str2;		UseResFile(gOurResRefNum);	GetStringHdl(kMimeStrBaseID, str1);	GetStringHdl(kMimeStrBaseID + 1, str2);	gMimeTypeH = MakeMimeType(str1, str2, nil);	if (gMimeTypeH != nil) {	/* Loop for six parameters in dialog. */		for (i = 0; i < 6; i++) {			GetStringHdl(kMimeStrBaseID + 2 + (i * 2), str1);			GetStringHdl(kMimeStrBaseID + 3 + (i * 2), str2);			if ((str1[0] != 0) || (str2[0] != 0))				AddMimeParameter(gMimeTypeH, str1, str2);						}	}}#pragma mark -/*****************************************************************//*  A D D R E S S   D I A L O G/*****************************************************************/#define kAddressStrBaseID	701#define KAddDefStrBaseID	801#define kBaseAddressItem	14/* ----------------------------------------------------------------------	Handle address dialog */void DoAddressDlog(void){	DialogPtr		theDialog;	DialogRecord	dlgRec;	GrafPtr			savePort;	Boolean			done = false;	short			itemHit;		GetPort(&savePort);	UseResFile(gOurResRefNum);	theDialog = GetNewDialog(kAddressDlogID, &dlgRec, (WindowPtr) -1L);	if (theDialog != nil) {		SetPort(theDialog);		SetWindowFont(theDialog, geneva, 9, normal);		SetDialogDefaultItem(theDialog, kOK);		SetDialogCancelItem(theDialog, kCancel);		SetupAddressDialog(theDialog);		ShowWindow(theDialog);		while (!done) {			ModalDialog(nil, &itemHit);			switch (itemHit) {				case kOK:					SaveAddressSettingsToFile(theDialog);					GetAddressChanges(theDialog);				case kCancel:					done = true;					break;				case kFactorySettings:					RestoreAddressDefaults(theDialog);					break;			}		}		CloseDialog(theDialog);		SetPort(savePort);	}}/* ----------------------------------------------------------------------	Load dialog settings from resources */void RestoreAddressDefaults(DialogPtr theDialog){	short		itemType, i;	Handle		itemH;	Rect		itemRect;	Str255		theStr;	for (i = 0; i < 3; i++) {		GetStringHdl(KAddDefStrBaseID + i, theStr);		GetDialogItem(theDialog, kBaseAddressItem + i, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, theStr);	}	for (i = 3; i < 9; i++) {		GetDialogItem(theDialog, kBaseAddressItem + i, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, "\p");	}}/* ----------------------------------------------------------------------	Save dialog settings to resources */void SaveAddressSettingsToFile(DialogPtr theDialog){	short			itemType, i;	Handle			itemH;	Rect			itemRect;	Str255			textStr;	StringHandle	strH;	UseResFile(gOurResRefNum);	for (i = 0; i < 9; i++) {		GetDialogItem(theDialog, kBaseAddressItem + i, &itemType, &itemH, &itemRect);		GetDialogItemText(itemH, textStr);				strH = GetString(kAddressStrBaseID + i);		if (strH != nil) {			SetString(strH, textStr);			ChangedResource((Handle) strH);			WriteResource((Handle) strH);			ReleaseResource((Handle) strH);		}	}	UpdateResFile(gOurResRefNum);}/* ----------------------------------------------------------------------	Initialize dialog settings from resources */void SetupAddressDialog(DialogPtr theDialog){	short			itemType, i;	Handle			itemH;	Rect			itemRect;	Str255			theStr;	for (i = 0; i < 9; i++) {		GetStringHdl(kAddressStrBaseID + i, theStr);		GetDialogItem(theDialog, kBaseAddressItem + i, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, theStr);	}	SelectDialogItemText(theDialog, kBaseAddressItem, 0, 32767);}/* ----------------------------------------------------------------------	Copy dialog settings to addresses global */void GetAddressChanges(DialogPtr theDialog){	short			itemType, i;	Handle			itemH;	Rect			itemRect;	StringHandle	strH;	Str255			textStr;		for (i = 0; i < 9; i++) {		GetDialogItem(theDialog, kBaseAddressItem + i, &itemType, &itemH, &itemRect);		GetDialogItemText(itemH, textStr);		strH = (*gAddressesH)[i];		if (strH == nil)			strH = (StringHandle) NewHandle(0);		SetString(strH, textStr);		(*gAddressesH)[i] = strH;	}	}/* ----------------------------------------------------------------------	Create addresses global from resources */void InitAddressData(void){	short			i;	StringHandle	resH;	UseResFile(gOurResRefNum);	gAddressesH = (StringHandle**) NewHandleClear(10 * sizeof(StringHandle));	for (i = 0; i < 9; i++) {		resH = GetString(kAddressStrBaseID + i);		if (resH != nil) {			DetachResource((Handle) resH);			(*gAddressesH)[i] = resH;		}	}	(*gAddressesH)[9] = nil;	// last StringHandle is nil}#pragma mark -/*****************************************************************//*  C O N T E X T/*****************************************************************/short ContextToMenuItem(long context){	short		item;		switch (context & 0x000FFFF) {		case EMSF_ON_ARRIVAL:		item = 1;		break;		case EMSF_ON_DISPLAY:		item = 2;		break;		case EMSF_ON_REQUEST:		item = 3;		break;		case EMSF_Q4_TRANSMISSION:	item = 4;		break;	}	return item;}long MenuItemToContext(short item){	long		context;		switch (item) {		case 1:		context = EMSF_ON_ARRIVAL;		break;		case 2:		context = EMSF_ON_DISPLAY;		break;		case 3:		context = EMSF_ON_REQUEST;		break;		case 4:		context = EMSF_Q4_TRANSMISSION;	break;	}	return context;}#pragma mark -/*****************************************************************//*  S U P P O R T  R O U T I N E S/*****************************************************************//* ----------------------------------------------------------------------	Display message dialog */void DoMessage(long status){	Str255		theStr;	Str31		numStr;		CopyPP("\pResult: ", theStr);	if ((status >= EMSR_OK) && (status <= EMSR_ABORTED)) {		CatPP(translatorErrors[status], theStr);		if (gResultStatus.desc != nil) {			CatPP(*gResultStatus.desc, theStr);			DisposeHandle((Handle) gResultStatus.desc);			gResultStatus.desc = nil;		}	}	else		CatPP("\pbad result", theStr);	CatPP("\p\rCode: ", theStr);	NumToString(gResultStatus.code, numStr);	CatPP(numStr, theStr);	ParamText(theStr, "\p", "\p", "\p");	Alert(kNoteALRTid, nil);}/* ----------------------------------------------------------------------	Display error dialog if error */void DoError(long error, Str63 message){	Str255		errStr;		if (error != EMSR_OK) {		// Report how it all went		CopyPP(message, errStr);		CatPP("\p: ", errStr);		if ((error >= EMSR_OK) && (error <= EMSR_ABORTED))			CatPP(translatorErrors[error], errStr);		else			CatPP("\pbad result", errStr);		ParamText(errStr, "\p", "\p", "\p");		Alert(kErrorALRTid, nil);		ExitToShell();	}}/* ----------------------------------------------------------------------	Copy STR resource to Str255 */void GetStringHdl(short id, Str255 theStr){	StringHandle	strH;		strH = GetString(id);	if (strH != nil) {		CopyPP(*strH, theStr);		ReleaseResource((Handle) strH);	}	else		theStr[0] = '\0';}/* ----------------------------------------------------------------------	Set dialog's font */void SetWindowFont(DialogPtr theDialog, short fontNum, short fontSize, Style fontStyle){	FontInfo	fInfo;	GrafPtr		savePort;	TEHandle	theTE;		theTE = ((DialogPeek) theDialog)->textH;	if (theTE != nil) {		GetPort(&savePort);		SetPort(theDialog);				TextFont(fontNum);		TextSize(fontSize);		TextFace(fontStyle);	   			(**theTE).txFont = fontNum;		(**theTE).txFace = fontStyle;		(**theTE).txSize = fontSize;	   	GetFontInfo(&fInfo);		(**theTE).fontAscent = fInfo.ascent;		(**theTE).lineHeight = fInfo.ascent + fInfo.descent + fInfo.leading;		SetPort(savePort);	}}/* ----------------------------------------------------------------------	Simulate Eudora's progress routine */#define kPercentItem		2#define kMessageItem		3short ProgressProc(emsProgressData *prog){	GrafPtr			savePort;	WindowPtr		theWindow;	short			itemType, part, result = 0;	Str15			pctStr;	Handle			itemH;	ControlHandle	hCtl;	Point			mousePt;	Rect			itemRect;	/* The translator should call this function periodically with emsProgressData.value	 * between 0 and 100 (percent). The translator should check the return value.	 * 		false - continue translation	 *		true - abort translation	 *	 * If you are supplying your own progress function, call this with -1	 * to check for abort.	 */	if (gProgressDlg == nil)		return result;	if (prog->value == 0) {		ShowWindow(gProgressDlg);		return result;	}	if (prog->value == 100) {		HideWindow(gProgressDlg);		return result;	}	GetPort(&savePort);	SetPort(gProgressDlg);	if (!EmptyRgn(((DialogPeek) gProgressDlg)->window.updateRgn)) {		BeginUpdate(gProgressDlg);		UpdateDialog(gProgressDlg, gProgressDlg->visRgn);		EndUpdate(gProgressDlg);	}	if (prog->value >= 0) {		NumToString(prog->value, pctStr);		GetDialogItem(gProgressDlg, kPercentItem, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, pctStr);	}	if (prog->message != nil) {		GetDialogItem(gProgressDlg, kMessageItem, &itemType, &itemH, &itemRect);		SetDialogItemText(itemH, prog->message);	}	if (Button()) {		GetMouse(&mousePt);		LocalToGlobal(&mousePt);		part = FindWindow(mousePt, &theWindow);		if (part == inContent && theWindow == gProgressDlg) {			GlobalToLocal(&mousePt);			part = FindControl(mousePt, gProgressDlg, &hCtl);			if (part == kControlButtonPart) {	// we only have one button...				part = TrackControl(hCtl, mousePt, nil);				if (part == kControlButtonPart)					result = 1;		// user cancelled			}		}	}	SystemTask();	SetPort(savePort);	return result;}