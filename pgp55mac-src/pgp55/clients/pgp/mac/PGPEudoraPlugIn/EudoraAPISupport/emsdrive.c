/* ======================================================================
   
    Eudora Extended Message Services API SDK Macintosh 1.0b3 (July 11 1996)
    This SDK supports EMSAPI version 1
    Copyright 1995, 1996 QUALCOMM Inc.
    Send comments and questions to <emsapi-info@qualcomm.com>

    Filename: emsdrive.c
   
   Test driver for Eudora Translator API

   Last Edited: July 23, 1996

   Laurence Lundblade <lgl@qualcomm.com>
   Scott Gruby <sgruby@qualcomm.com>
   Myra Callen <t_callen@qualcomm.com>

 */
#pragma once

#include "Quickdraw.h"
#include <string.h>
#include <stdlib.h>
#include <Icons.h>
#include <Types.h>

#include "emsapi-mac.h"
#include "copycat.h"
#include "mimetype.h"
#include "usertrans.h"


/*****************************************************************/
/*  G L O B A L S  and  C O N S T A N T S
/*****************************************************************/

Str255		quitString = "\pClick in window to quit";

#define kBaseResID			128
#define kErrorALRTid		128
#define kAboutALRTid		129
#define kProgressALRTid		130

#define kVisible			true
#define kMoveToFront		(WindowPtr)-1L
#define kSleep				60L
#define kNullPtr			0L

#define mApple				kBaseResID
#define iAbout				1

#define mFile				kBaseResID+1
#define iQuit				1

#define	kHeaderOffset		42
#define kDummyHOffset		233
#define kTransHeightOffset 	60
#define kNumberofButtons	2
#define kTransTopOffset		4
#define kSecondButtonOffset	30
#define kButHeight			18
#define kTransButWidth		120
#define kTestButWidth		90
#define	kIconHOffset		4
#define kButHOffset			42
#define	kTextHOffset		168
#define kWindowWidth		528

#define kOK					1
#define kCancel				2
#define kFactorySettings	3

#define kProgressDlogID		500
#define kPercentItem		3

#define kMimeDlogID			600
#define kMimeStrBaseID		401
#define KMimeDefStrBaseID	501
#define kBaseMimeItem		22
#define kTypeItem			22
#define kSubtypeItem		23
#define kBaseParamItem		24

#define kAddressDlogID		700
#define kAddressStrBaseID	701
#define KAddDefStrBaseID	801
#define kBaseAddressItem	16

#define kContextDlogID		800
#define kOnArrival			6
#define kOnDisplay			7
#define kOnRequest			8
#define kQ4Completion		9
#define kQ4Transmission		10

#define kDefModuleIcon		10128
#define kDefTransIcon		10129

Boolean gDone;

char *translatorErrors[] = {
	"EMSR_OK",
	"EMSR_UNKNOWN_FAIL",
	"EMSR_CANT_TRANS",
	"EMSR_INVALID_TRANS",
	"EMSR_NO_ENTRY",
	"EMSR_NO_INPUT_FILE",
	"EMSR_CANT_CREATE",
	"EMSR_TRANS_FAILED",
	"EMSR_INVALID",
	"EMSR_NOT_NOW",
	"EMSR_NOW", 
	"EMSR_ABORTED"};
	

/* Keep track of translatorButtons on translator modules window */
struct translatorButtonstructure {
    ControlHandle  handle;
    void				  (*proc)();
    void				   *arg;
} translatorButtons[100];

typedef struct moduleInfoS {
    Handle 					globalsH;
    short 					transCount;
    StringHandle			moduleDesc;
    short					moduleID;
    Handle					iconH;
} moduleInfo;

typedef struct translatorInfo {
    short					translatorNumber;
    long					type;
    long					subtype;
    unsigned long			flags;
} translatorInfo;

/* Module Info from init call */
static short				gTranslatorCount, gModuleID;
long						gContext;
static Handle				gModuleIconH, gModuleGlobalsH;
static StringHandle			gModuleNameH;
static ControlHandle		gMimeButH, gAddressButH, gContextButH;
static emsMIMEtype		  **gMimeTypeH;
static emsMIMEparam		  **gMimeParamsH[6];
static short				gOurResRefNum;
static Handle			  **gAddressesH;
static WindowPtr			gTranslatorWindow = nil, wPtr;


/*****************************************************************/
/*  P R O T O T Y P E S
/*****************************************************************/

void  	ToolboxInit( void );
void  	main( void );
void	MenuBarInit( void );
void 	DoInitialize();
short	DrawWindow(WindowPtr window);
void	EventLoop( void );
void	DoEvent( EventRecord eventPtr );
void    CleanUp( void );


void	DoMimeDlog( void );
void	RestoreMimeDefaults( DialogPtr dlog );
void	SaveMimeSettingsToFile( DialogPtr dlog );
void	SetupMimeDialog( DialogPtr dlog );
void	BuildNewMimeData( void );

void	DoAddressDlog( void );
void	RestoreAddressDefaults( DialogPtr dlog );
void	SaveAddressSettingsToFile( DialogPtr dlog );
void	SetupAddressDialog( DialogPtr dlog );
void	GetAddressChanges( DialogPtr dlog );
void	InitAddressData( void );

void	DoContextDlog( void );
void	TurnOffAllRadials( DialogPtr dlog );

void	HandleMouseDown( EventRecord *eventPtr );

void	HandleMenuChoice( long menuChoice );

void	HandleAppleChoice( short item );
void	HandleFileChoice( short item );

void	DoError( Str255 errorString );
void    DoTranslation(ControlHandle theControl, void *);
void    CanTranslation(ControlHandle theControl, void *);

void 	DoUpdate();
void	DoMessage( Str255 errorString );
void 	SetWindowFont(DialogPtr dlog, short fontNum, short fontSize,
			Style fontStyle, short fontMode);

void 	DoDisposeHandle(Handle);
Handle 	DoNewHandle(size_t);

// borrowed from emsapi-MacGlue.h
/* ========== set up for call backs ======================================*/
#if 0

typedef pascal short (*emsProgress_t)(short percent);

#if GENERATINGCFM
typedef UniversalProcPtr emsProgressUPP;
#else
typedef emsProgress_t emsProgressUPP;
#endif
#endif

enum {
    uppemsProgressProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
};


#if GENERATINGCFM
#define NewemsProgressProc(userRoutine)	 (emsProgressUPP) \
NewRoutineDescriptor((ProcPtr)(userRoutine), \
uppemsProgressProcInfo, GetCurrentArchitecture())
#else
#define NewemsProgressProc(userRoutine)	 \
((emsProgressUPP) (userRoutine))
#endif



/*****************************************************************/
/*  M A I N  P R O G R A M
/*****************************************************************/

void main( void )
{
    ToolboxInit();
	MenuBarInit();
	DoInitialize();
	EventLoop();
	CleanUp();
}


/* ----------------------------------------------------------------------
 * Put up window with module/translator info
 */

void DoInitialize()
{
	Boolean		done = false;
	Rect		theRect;
	short		cnt;
    long        theResult = EMSR_OK;
	Str255  	message;
	OSErr		err;
	
	/* get our resource ref num */
	gOurResRefNum = CurResFile();	

    /* Pretend we're like the component manager handle the globals storage */
    gModuleGlobalsH = NewHandle(sizeof(emsUserGlobals));

	/* Initialize this module and get the module information.*/
	theResult = ems_plugin_init(
					gModuleGlobalsH,
					&gTranslatorCount,
					&gModuleNameH,
					&gModuleID,
					&gModuleIconH);
	if(theResult != EMSR_OK) {
	    gTranslatorCount = 0;
	    // Report how it all went
		strcpy((char *)message, "plugin_init failed: ");
		if ( (theResult >= EMSR_OK) && (theResult <= EMSR_ABORTED) )
			strcat((char *)message, translatorErrors[theResult]);
		else
			strcat((char *)message, "bad result");
		CtoPstr((char *)message);
		DoMessage(message);
	}
	
	/* Substitute our dummy icon if translator doesn't have one. */
	if ( gModuleIconH == NULL )
    	err = GetIconSuite(&gModuleIconH,kDefModuleIcon,svAllAvailableData);

	/* initialize mime data */
	BuildNewMimeData();
	
	InitAddressData();
	gContext = EMSF_ON_REQUEST;
	
	SetRect(&theRect, 10, 50,
			kWindowWidth,
			(gTranslatorCount+1)*kTransHeightOffset + kHeaderOffset);	
	gTranslatorWindow = NewCWindow (0L,
		&theRect, "\pTranslator Debugger",
		 TRUE, 0, (WindowPtr)-1L, FALSE, 0L);
	SetPort(gTranslatorWindow);
	
	OffsetRect(&theRect, 100, 0);
	SetPort(gTranslatorWindow);
	
	/* Buttons for dummy testing information */
    SetRect(&theRect,
    		kDummyHOffset, 20,
    		kDummyHOffset+kTestButWidth, 20 + kButHeight );
	gMimeButH = NewControl(gTranslatorWindow,
				&theRect, "\pMime Types", TRUE,
                             0, 0, 0, pushButProc, 0L);	
    SetRect(&theRect,
    		kDummyHOffset+95, 20,
    		kDummyHOffset+kTestButWidth+95, 20 + kButHeight);
	gAddressButH = NewControl(gTranslatorWindow,
				&theRect, "\pAddresses", TRUE,
                             0, 0, 0, pushButProc, 0L);
    SetRect(&theRect,
    		kDummyHOffset+190, 20,
    		kDummyHOffset+kTestButWidth+190, 20 + kButHeight);
	gContextButH = NewControl(gTranslatorWindow,
					&theRect, "\pContext", TRUE,
                             0, 0, 0, pushButProc, 0L);
		
	/* create buttons for all the translators */
	if(theResult != EMSR_OK) {
		DoMessage("\pSorry, ems_plugin_init failed.");
	}
	else {
	
		for (cnt = 0; cnt < gTranslatorCount; cnt++ ) {

// create two buttons for every translator, save the handle in the array
SetRect(&theRect,
	kButHOffset,
	(cnt) * kTransHeightOffset + kHeaderOffset + kTransTopOffset,
	kButHOffset+kTransButWidth,
	(cnt) * kTransHeightOffset +
	kButHeight+kHeaderOffset+kTransTopOffset);
translatorButtons[(cnt*2)+1].handle =
	NewControl(gTranslatorWindow,&theRect,"\pCanTrans on File",TRUE,
    0, 0, 0, pushButProc, cnt+1);
SetRect(&theRect,
	kButHOffset,
	(cnt)*kTransHeightOffset+kHeaderOffset+kSecondButtonOffset,
	kButHOffset+kTransButWidth,
	(cnt)*kTransHeightOffset+kSecondButtonOffset+kButHeight+kHeaderOffset);
translatorButtons[(cnt*2)].handle =
	NewControl(gTranslatorWindow, &theRect, "\pTranslate File", TRUE,
    0, 0, 0, pushButProc, cnt+1);
		}
	}
	
	DoUpdate();
}

void DoUpdate()
{	short       returnError;
	
	BeginUpdate(gTranslatorWindow);

    returnError = DrawWindow(gTranslatorWindow);
	UpdateControls(gTranslatorWindow, gTranslatorWindow->visRgn);
	 	
	EndUpdate(gTranslatorWindow);

}


void CleanUp() 
{
	if ( gModuleGlobalsH != NULL ) {
	    ems_plugin_finish(gModuleGlobalsH);
	    DisposeHandle(gModuleGlobalsH);
	    gModuleGlobalsH = NULL;
    }
}



/* --------------------------------------------------------------------
    Avoid taking address of an inline functions. At least that's what the
    error messages say....
 */
void DoDisposeHandle(Handle h)
{
    DisposeHandle(h);
}

Handle DoNewHandle(size_t size)
{
    return(NewHandle(size));
}



/* ----------------------------------------------------------------------
    Display the translators in a particular module with translatorButtons for
    invoking operatations
 */
short
DrawWindow(WindowPtr window)
{
    char        	buffer[4000];
    short       	cnt, buttonCount;
    Handle			transIconSuiteH = NULL;
    Rect			theIconRect, textBox;
    Str63       	theNumString;
    short       	api_version = 0;
    translatorInfo 	tInfo;
    long        	theResult = EMSR_OK;
    StringHandle   	descriptionStrH = NULL;
    OSErr			err;
	
	SetPort(window);
	TextSize(10);

    MoveTo(kDummyHOffset, 14);
    DrawString("\pSet Dummy Testing Information: ");
	
	// Module Icon
    SetRect(&theIconRect,
    		kIconHOffset, 5,
    		kIconHOffset+32, 5 + 32);
	if ( gModuleIconH && *gModuleIconH )
		PlotIconSuite(&theIconRect,atAbsoluteCenter,ttNone,gModuleIconH);
	
    // Module Name
    MoveTo(kButHOffset, 12);
    DrawString("\pModule: ");
    TextFace(bold);
    MoveTo(kButHOffset, 24);
    if ( gModuleNameH != NULL ) {
	    HLock( (Handle)gModuleNameH );
	    DrawString( *gModuleNameH );
	    HUnlock( (Handle)gModuleNameH );
    }
    else
	    DrawString( "\pNULL value" );    	
    TextFace( 0 ); //plain

    // API version
    MoveTo(kButHOffset, 37);
    DrawString("\pAPI Version: ");
	ems_plugin_version(&api_version);
	if ( api_version != 0 ) {
		NumToString(api_version, theNumString);
	    DrawString(theNumString);
    }
    else
	    DrawString( "\pNULL value" );

	// Draw line
    MoveTo(0,kHeaderOffset);
    LineTo(600,kHeaderOffset);

    // Go through the translators in the module
    for(cnt = 1; cnt <= gTranslatorCount; cnt++) {

        tInfo.type = 0;
        tInfo.subtype = 0;
        tInfo.flags = 0;
        
        /* Get the description of the translator */
        theResult = ems_translator_info(
        			gModuleGlobalsH,
        			cnt,
        			&tInfo.type,
        			&tInfo.subtype,
        			&tInfo.flags,
        			&descriptionStrH,
        			nil);
        tInfo.translatorNumber = cnt;
        
        /* Get the icon separately */
        transIconSuiteH = NULL;
        theResult = ems_translator_info(
        			gModuleGlobalsH,
        			cnt, 
        			nil,
        			nil,
        			nil,
        			nil,
        			&transIconSuiteH);

		/* Substitute our dummy icon if translator doesn't have one. */
		if ( transIconSuiteH == NULL )
    		err = GetIconSuite(&transIconSuiteH,
    			kDefTransIcon,svAllAvailableData);

	    MoveTo(kTextHOffset,
	    	(cnt-1)*kTransHeightOffset+kHeaderOffset+kTransTopOffset+9);
	    NumToString(cnt, theNumString);
	    PtoCstr(theNumString);
	    strcpy(buffer, "Translator #");
	    strcat(buffer, (char *)theNumString);
	    strcat(buffer, ": ");
	    DrawString(CtoPstr(buffer));
	    if ( descriptionStrH != NULL ) {
	        HLock((Handle)descriptionStrH);
		    DrawString(*descriptionStrH);
	        HUnlock((Handle)descriptionStrH);
	        DisposHandle((Handle)descriptionStrH);
	        descriptionStrH = NULL;
        }
        else
	    	DrawString( "\pNULL value" );        

        /* Draw the icon - icon is 32x32 */
        SetRect(&theIconRect,
           kIconHOffset,
           (cnt-1) * kTransHeightOffset +
           		kHeaderOffset + kTransTopOffset,
           kIconHOffset+32,
           (cnt-1) * kTransHeightOffset +
           	32 + kHeaderOffset + kTransTopOffset);
		if ( transIconSuiteH && *transIconSuiteH ) {
			PlotIconSuite(&theIconRect,
				atAbsoluteCenter,ttNone,transIconSuiteH);
			DisposeIconSuite(transIconSuiteH, TRUE);
			transIconSuiteH = NULL;
		}	

	    /* The translator info details */
	    strcpy(buffer,"Type: ");
        NumToString(tInfo.type, theNumString);
        PtoCstr(theNumString);
        strcat(buffer, (char *) theNumString);
	    strcat(buffer, "     Subtype: ");
	    NumToString(tInfo.subtype, theNumString);
	    PtoCstr(theNumString);
	    strcat(buffer, (char *)theNumString);
	    strcat(buffer, "     ");
		strcat(buffer,
			tInfo.flags &EMSF_REQUIRES_MIME ? "wants MIME, " : "");
		strcat(buffer,
			tInfo.flags & EMSF_GENERATES_MIME ? "makes MIME, " : "");
		
	    MoveTo(kTextHOffset,
	    	(cnt-1) * kTransHeightOffset +
	    		23 + kHeaderOffset + kTransTopOffset);
	    DrawString(CtoPstr(buffer));
	
	    /* Some of the options */
	    MoveTo(kTextHOffset,
	    		(cnt-1) * kTransHeightOffset +
	    		37 + kHeaderOffset + kTransTopOffset);
	    strcpy(buffer, "Options:");
	    DrawString(CtoPstr(buffer));
	
	    strcpy(buffer, "");
	    if ( tInfo.flags & EMSF_ON_ARRIVAL ) {
	    	strcpy(buffer, "On Arrival");
	    }
	    if ( tInfo.flags & EMSF_ON_DISPLAY ) {
	    	if ( strlen(buffer) != 0 ) strcat(buffer, ", ");
	    	strcat(buffer, "On Display");
	    }
	    if ( tInfo.flags & EMSF_WHOLE_MESSAGE ) {
	    	if ( strlen(buffer) != 0 ) strcat(buffer, ", ");
	    	strcat(buffer, "Whole-Message");
	    }
	    if ( tInfo.flags & EMSF_ON_REQUEST ) {
	    	if ( strlen(buffer) != 0 ) strcat(buffer, ", ");
	    	strcat(buffer, "On Request");
	    }
	    if ( tInfo.flags & EMSF_Q4_TRANSMISSION ) {
	    	if ( strlen(buffer) != 0 ) strcat(buffer, ", ");
	    	strcat(buffer, "Q4 Transmission");
	    }
	    if ( tInfo.flags & EMSF_Q4_COMPLETION ) {
	    	if ( strlen(buffer) != 0 ) strcat(buffer, ", ");
	    	strcat(buffer, "Q4 Completion");
	    }
	    		
    	SetRect(&textBox,
    			kTextHOffset+42,
    			(cnt-1) * kTransHeightOffset +
    				kHeaderOffset + kTransTopOffset + 27,
    			kTextHOffset+350,
    			(cnt-1) * kTransHeightOffset +
    				kHeaderOffset + kTransTopOffset + 56);
		CtoPstr(buffer);
        TETextBox (&(buffer[1]), buffer[0], &textBox, teFlushLeft);

        // The test translation button
		buttonCount = (cnt*2) - 2;
        translatorButtons[buttonCount].proc = (void (*)())DoTranslation;
        translatorButtons[buttonCount].arg =
        	(void *)NewPtr(sizeof (struct translatorInfo));
        *((struct translatorInfo *)
        	translatorButtons[buttonCount].arg) = tInfo;

        // The can translate button
		buttonCount = (cnt*2) - 1;
        translatorButtons[buttonCount].proc =
        	(void (*)())CanTranslation;
        translatorButtons[buttonCount].arg =
        	(void *)NewPtr(sizeof (struct translatorInfo));
        *((struct translatorInfo *)
        	translatorButtons[buttonCount].arg) = tInfo;

		// Draw line
    	MoveTo(kButHOffset,
    			(cnt) * kTransHeightOffset + kHeaderOffset );
    	LineTo(600,
    			(cnt) * kTransHeightOffset + kHeaderOffset );

    }

    return(gTranslatorCount);
}	


/* ----------------------------------------------------------------------
    Actually do a translation
 */
void DoTranslation(ControlHandle theControl, void *arg)
{
    StringPtr      		*errorMessage = NULL;
    Str255               buffer, theNumString;
    StringPtr		    *returnedMessage = NULL;
    struct  			 translatorInfo *trans = arg;
    short   			 theResult = EMSR_OK;
    long                 resultCode = 0;
   	StandardFileReply	 theGetReply, thePutReply;
    emsMIMEtype        **transMimeType; // These are handles to structures
    emsProgress			 progressUPP;

	transMimeType = NULL;
	errorMessage = NULL;
	returnedMessage = NULL;
	HiliteControl(theControl, 255); // Make control gray

    /* Get the input file */
	StandardGetFile(nil, -1, nil, &theGetReply);
	if (!theGetReply.sfGood) {
	    HiliteControl(theControl, 0);
    	return;
	}
	
    /* Get the output file */
	StandardPutFile("\pPick the Output File", "\pOutputFile", &thePutReply);
    /* Make sure the file exists, as required by API */
    FSpDelete(&thePutReply.sfFile);
    FSpCreate(&thePutReply.sfFile, 'ttxt', 'TEXT', smSystemScript);

    /* create callback function pointer for progress function */
	progressUPP = NewemsProgressProc(callProgress);

    if (!thePutReply.sfGood) {
	    HiliteControl(theControl, 0);
    	return;
	}

    HiliteControl(theControl, 0);
    DoUpdate();	  /* redraw window before calling translator */
	
	/* Eudora will call ems_can_translate before calling ems_translate.
		Here we are
	 * assuming you can translate for the purposes of testing your code.
	 */
	
    /* Call the translator in the module */
	theResult = ems_translate_file(
       gModuleGlobalsH,
       gContext,				/* Allow module to interact w/ user */
       trans->translatorNumber,/* Particular translator out of module */
       gMimeTypeH,				/* No MIME type (don't care) */
       &theGetReply.sfFile,			/* The input file */
      (StringHandle **) gAddressesH,
       progressUPP,			/* No progress/abort just yet */
       &transMimeType,	/* Don't worry about returned MIME type yet */
       &thePutReply.sfFile,	/* The outut file */
       (Handle *) NULL,			/* No icon is returned */
       &returnedMessage,			/* Message returned to user */
       &errorMessage,				/* Place to put error message */
       &resultCode);				/* Place to put error code */

	InvalRgn(gTranslatorWindow->visRgn);
    DoUpdate();	  /* redraw window after calling translator */

	// Report how it all went
	/* Basic status */
	strcpy((char *)buffer, "Result: ");
	if ( (theResult >= EMSR_OK) && (theResult <= EMSR_ABORTED) )
		strcat((char *)buffer, translatorErrors[theResult]);
	else
		strcat((char *)buffer, "bad result");
	strcat((char *)buffer, "\rCode: ");
	CtoPstr((char *)buffer);
	NumToString(resultCode, theNumString);
	catPP(theNumString, buffer);
	catPP("\p\rMessage: ", buffer);
	catPP(errorMessage != NULL ? (StringPtr)*errorMessage : "\p", buffer);
	DoMessage(buffer);
	
	/* The MIME type */
	if(transMimeType  && (theResult == EMSR_OK) ) {
	    Str255 msg;
	    Handle h;
	
	    h = string_mime_type(transMimeType);
	    HLock(h);
	    BlockMove(*h, msg+1, GetHandleSize(h) > 255 ? 255 : GetHandleSize(h));
	    HUnlock(h);
	    *msg = GetHandleSize(h) > 255 ? 255 : GetHandleSize(h);
		DoMessage(msg);
	}
	
	/* Extra messages */
	if(returnedMessage && *returnedMessage){
	   HLock((Handle)returnedMessage);
	   DoMessage(*returnedMessage);
	   HUnlock((Handle)returnedMessage);
	}
}


/* ----------------------------------------------------------------------
    Test if we can translate
 */
void CanTranslation( ControlHandle theControl, void *arg)
{
    StringPtr 			   *errorMessage = NULL;
    Str255                	buffer;
    translatorInfo  		*trans = arg;
    short   			  	theResult = EMSR_OK;
    long                  	resultSize = 0;
   	StandardFileReply	  	theGetReply;

	{	errorMessage = NULL;
	
    	HiliteControl(theControl, 255); // Make control gray

	    // Get the input file
	    // This is removed for now since it might
	    // get removed in the next version
	    theGetReply.sfFile.vRefNum = 0;
		//StandardGetFile(nil, -1, nil, &theGetReply);
		//if (!theGetReply.sfGood) {
        //	HiliteControl(theControl, 0);
  	    //  return;
    	//}
    					
	   	HiliteControl(theControl, 0);
    	DoUpdate();	  /* redraw window before calling translator */
		
     	// Call the translator in the module
		theResult = ems_can_translate_file(
		               gModuleGlobalsH,
                       gContext,
                       trans->translatorNumber,
                       	/* Particular translators out of module */
                       gMimeTypeH,
                       &theGetReply.sfFile,
                       	/* The input file, may get removed in next version */
                       (StringHandle **)gAddressesH,
                       &resultSize,
                       &errorMessage,
                       (long *)NULL);

		InvalRgn(gTranslatorWindow->visRgn);
    	DoUpdate();	  /* redraw window after calling translator */
    	
		// Report how it all went
		strcpy((char *)buffer, "Result: ");
		if ( (theResult >= EMSR_OK) && (theResult <= EMSR_ABORTED) )
			strcat((char *)buffer, translatorErrors[theResult]);
		else
			strcat((char *)buffer, "bad result");
		CtoPstr((char *)buffer);
		catPP("\p\rMessage: ", buffer);
		catPP(errorMessage != NULL ? (StringPtr)*errorMessage : "\p", buffer);
				
		DoMessage(buffer);
	}
}


/* ============================================================================
     Basic Interface and Event management

============================================================================
*/


/************************************* DoEvent	 */

void	DoEvent(EventRecord theEventRecord)
{
	char		theChar;
	
	
	switch (theEventRecord.what)
	{
	
		case mouseDown:
			HandleMouseDown(&theEventRecord);
			break;
		case keyDown:
		case autoKey:
			theChar = theEventRecord.message & charCodeMask;
			if ( (theEventRecord.modifiers & cmdKey) != 0 )
				HandleMenuChoice( MenuKey( theChar ) );
			break;
		case updateEvt:
			DoUpdate();
		    break;
		
		case activateEvt:
        case keyUp:				/* don't care */
        case mouseUp:			/* don't care */
        case networkEvt:		/* don't care */
        case driverEvt:			/* don't care */
        case kHighLevelEvent:	/*	AppleEvents	*/
		case nullEvent:
		default:
			break;
	}

}


/************************************* HandleMouseDown */

void	HandleMouseDown( EventRecord *eventPtr )
{
	WindowPtr		window;
	short			thePart;
	long			menuChoice;
	ControlHandle   whichControl;
	short           i;
	
	thePart = FindWindow( eventPtr->where, &window );
	switch ( thePart )
	{
		case inMenuBar:
			menuChoice = MenuSelect( eventPtr->where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow :
			SystemClick( eventPtr, window );
			break;
		case inContent:
			// SelectWindow( window );
			GlobalToLocal(&(eventPtr->where));
			FindControl(eventPtr->where, window, &whichControl);
			if ( ( gMimeButH != NULL ) && (gMimeButH == whichControl) ) {
				HiliteControl(whichControl, 255);
				DoMimeDlog();
				HiliteControl(whichControl, 0);
			}
			else if ( ( gAddressButH != NULL ) &&
					(gAddressButH == whichControl) ) {
				HiliteControl(whichControl, 255);
				DoAddressDlog();
				HiliteControl(whichControl, 0);
			}			
			else if ( ( gContextButH != NULL ) &&
					(gContextButH == whichControl) ) {
				HiliteControl(whichControl, 255);
				DoContextDlog();
				HiliteControl(whichControl, 0);
			}			
			else {
				/* loop for the two buttons for each translator */
				for(i = 0; i < (gTranslatorCount*kNumberofButtons); i++)
				{
					if ((translatorButtons[i].handle != NULL)
						&& (translatorButtons[i].handle == whichControl)
						&& (translatorButtons[i].proc != NULL)) {
 			        	(*(translatorButtons[i].proc))(whichControl,
 			        		translatorButtons[i].arg);
			    	}
			    }
			}
			break;
		case inDrag :
			DragWindow( window, eventPtr->where, &qd.screenBits.bounds );
			break;
	}
}


/****************** HandleMenuChoice ***********************/

void	HandleMenuChoice( long menuChoice )
{
	short	menu;
	short	item;
	
	if ( menuChoice != 0 )
	{
		menu = HiWord( menuChoice );
		item = LoWord( menuChoice );
		
		switch ( menu )
		{
			case mApple:
				HandleAppleChoice( item );
				break;
			case mFile:
				HandleFileChoice( item );
				break;
		}
		HiliteMenu( 0 );
	}
}


/****************** HandleAppleChoice ***********************/

void	HandleAppleChoice( short item )
{
	MenuHandle	appleMenu;
	Str255		accName;
	short		accNumber;
	
	switch ( item )
	{
		case iAbout:
			NoteAlert( kAboutALRTid, kNullPtr );
			break;
		default:
			appleMenu = GetMHandle( mApple );
			GetItem( appleMenu, item, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}


/****************** HandleFileChoice ***********************/

void	HandleFileChoice( short item )
{
	switch ( item )
	{
		case iQuit:
			gDone = true;
			break;
			
	}
}



/******************************** EventLoop *********/

void	EventLoop( void )
{		
	EventRecord		event;
	
	gDone = false;
	while ( gDone == false )
	{
		WaitNextEvent( everyEvent, &event, 30, NULL );
		DoEvent(event );
	}
}


/***************** DoMimeDlog ********************/

void	DoMimeDlog( void )
{
	DialogPtr		dlog;
	GrafPtr			savePort;
	Boolean			done = FALSE;
	short			itemHit;
	
	dlog = GetNewDialog( kMimeDlogID, kNullPtr, kMoveToFront );	
	SetupMimeDialog( dlog );
	if ( dlog != NULL )
	{
		GetPort( &savePort );
		SetPort( dlog );
		SetWindowFont(dlog, geneva, 9, 0, srcCopy);
		ShowWindow( dlog );
		
		while ( !done ) {
			ModalDialog( kNullPtr, &itemHit );
			switch (itemHit) {
				case kOK:
					done = TRUE;
					SaveMimeSettingsToFile( dlog );
					free_mime_type( gMimeTypeH );
					BuildNewMimeData();
					break;
				case kCancel:
					done = TRUE;
					break;
				case kFactorySettings:
					RestoreMimeDefaults( dlog );
					break;
				default:
					break;
			}
		}
	
		// clean up
		DisposeDialog(dlog);
		SetPort(savePort);
	}
}

/***************** RestoreMimeDefaults ********************/

void	RestoreMimeDefaults( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH;
	Rect			itemRect;
	StringHandle	textStrH;

	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 4; cnt++) {
		textStrH = (StringHandle)GetString(KMimeDefStrBaseID+cnt);
		GetDialogItem(dlog, kBaseMimeItem+cnt, &itemType, &itemH, &itemRect);
		if ( itemH != NULL ) {
			HLock( (Handle)textStrH );
			SetDialogItemText( itemH, *textStrH );
			HUnlock( (Handle)textStrH );
		}
	}
	for( cnt = 4; cnt < 14; cnt++) {
		GetDialogItem(dlog, kBaseMimeItem+cnt, &itemType, &itemH, &itemRect);
		if ( itemH != NULL ) {
			HLock( (Handle)textStrH );
			SetDialogItemText( itemH, "\p" );
			HUnlock( (Handle)textStrH );
		}
	}
}


/***************** SaveMimeSettingsToFile ********************/

void	SaveMimeSettingsToFile( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH, resH;
	Rect			itemRect;
	Str255			textStr;

	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 14; cnt++) {
		GetDialogItem(dlog, kBaseMimeItem+cnt, &itemType, &itemH, &itemRect);
		GetDialogItemText( itemH, textStr );
		
		resH = Get1Resource ('STR ', kMimeStrBaseID+cnt);
		if ( resH != NULL ) {
			LoadResource(resH);
			SetString((StringHandle)resH, textStr);
			ChangedResource(resH);
			WriteResource(resH);
			ReleaseResource(resH);
		}
	}
	UpdateResFile(gOurResRefNum);
}


/***************** SetupMimeDialog ********************/

void	SetupMimeDialog( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH;
	Rect			itemRect;
	StringHandle	textStrH;

	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 14; cnt++) {
		textStrH = GetString(kMimeStrBaseID+cnt);
		if ( textStrH != NULL ) {
			GetDialogItem(dlog,
				kBaseMimeItem+cnt, &itemType, &itemH, &itemRect);
			if ( itemH != NULL ) {
				HLock( (Handle)textStrH );
				SetDialogItemText( itemH, *textStrH );
				HUnlock( (Handle)textStrH );
			}
			ReleaseResource( (Handle)textStrH);
		}
	}
}


/***************** BuildNewMimeData ********************/

void	BuildNewMimeData()
{	short			cnt;
	StringHandle	typeStrH, subtypeStrH, nameStrH, valueStrH;
	
	UseResFile(gOurResRefNum);

	/* Read Mime strings from resource fork. */
	typeStrH = GetString(kMimeStrBaseID);
	subtypeStrH = GetString(kMimeStrBaseID+1);
	if ( (typeStrH != NULL ) && (subtypeStrH != NULL ) ) {
		HLock( (Handle)typeStrH  );
		HLock( (Handle)subtypeStrH  );
		gMimeTypeH = make_mime_type(*typeStrH, *subtypeStrH, NULL);
		HUnlock( (Handle)subtypeStrH  );
		HUnlock( (Handle)typeStrH  );
	}

	if ( gMimeTypeH != NULL ) {
		/* Loop for six parameters in dialog. */
		for( cnt = 0; cnt < 5; cnt++) {
			nameStrH = GetString(kMimeStrBaseID+2+(cnt*2));
			valueStrH = GetString(kMimeStrBaseID+3+(cnt*2));
			if ( (nameStrH != NULL)	&& (valueStrH != NULL) ) {
				HLock( (Handle) nameStrH);
				HLock( (Handle) valueStrH);
				if ( ((*nameStrH)[0] != 0) || ((*valueStrH)[0] != 0) )
					add_mime_parameter(gMimeTypeH,
						*nameStrH, *valueStrH);				
				HUnlock( (Handle) nameStrH);
				HUnlock( (Handle) valueStrH);
				ReleaseResource( (Handle)nameStrH);
				ReleaseResource( (Handle)valueStrH);
			}
		}
	}
}


/***************** DoAddressDlog ********************/

void	DoAddressDlog( void )
{
	DialogPtr		dlog;
	GrafPtr			savePort;
	Boolean			done = FALSE;
	short			itemHit;
	
	dlog = GetNewDialog( kAddressDlogID, kNullPtr, kMoveToFront );	
	SetupAddressDialog( dlog );
	if ( dlog != NULL )
	{
		GetPort( &savePort );
		SetPort( dlog );
		SetWindowFont(dlog, geneva, 9, 0, srcCopy);
		ShowWindow( dlog );
		
		while ( !done ) {
			ModalDialog( kNullPtr, &itemHit );
			switch (itemHit) {
				case kOK:
					done = TRUE;
					SaveAddressSettingsToFile( dlog );
					GetAddressChanges( dlog );
					break;
				case kCancel:
					done = TRUE;
					break;
				case kFactorySettings:
					RestoreAddressDefaults( dlog );
					break;
				default:
					break;
			}
		}
	
		// clean up
		DisposeDialog(dlog);
		SetPort(savePort);
	}
}


/***************** RestoreAddressDefaults ********************/

void	RestoreAddressDefaults( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH;
	Rect			itemRect;
	StringHandle	textStrH;

	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 3; cnt++) {
		textStrH = (StringHandle)GetString(KAddDefStrBaseID+cnt);
		GetDialogItem(dlog, kBaseAddressItem+cnt, &itemType, &itemH, &itemRect);
		if ( itemH != NULL ) {
			HLock( (Handle)textStrH );
			SetDialogItemText( itemH, *textStrH );
			HUnlock( (Handle)textStrH );
		}
	}
	for( cnt = 3; cnt < 9; cnt++) {
		GetDialogItem(dlog, kBaseAddressItem+cnt, &itemType, &itemH, &itemRect);
		if ( itemH != NULL ) {
			HLock( (Handle)textStrH );
			SetDialogItemText( itemH, "\p" );
			HUnlock( (Handle)textStrH );
		}
	}
}


/***************** SaveAddressSettingsToFile ********************/

void	SaveAddressSettingsToFile( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH, resH;
	Rect			itemRect;
	Str255			textStr;

	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 9; cnt++) {
		GetDialogItem(dlog, kBaseAddressItem+cnt, &itemType, &itemH, &itemRect);
		GetDialogItemText( itemH, textStr );
		
		resH = Get1Resource ('STR ', kAddressStrBaseID+cnt);
		if ( resH != NULL ) {
			LoadResource(resH);
			SetString((StringHandle)resH, textStr);
			ChangedResource(resH);
			WriteResource(resH);
			ReleaseResource(resH);
		}
	}
	UpdateResFile(gOurResRefNum);
}


/***************** SetupAddressDialog ********************/

void	SetupAddressDialog( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH;
	Rect			itemRect;
	StringHandle	textStrH;

	UseResFile(gOurResRefNum);

	for( cnt = 0; cnt < 9; cnt++) {
		textStrH = GetString(kAddressStrBaseID+cnt);
		if ( textStrH != NULL ) {
			GetDialogItem(dlog,
				kBaseAddressItem+cnt, &itemType, &itemH, &itemRect);
			if ( itemH != NULL ) {
				HLock( (Handle)textStrH );
				SetDialogItemText( itemH, *textStrH );
				HUnlock( (Handle)textStrH );
			}
			ReleaseResource( (Handle)textStrH);
		}
	}
}


/***************** GetAddressChanges ********************/

void	GetAddressChanges( DialogPtr dlog )
{	short			itemType, cnt;
	Handle			itemH;
	Rect			itemRect;
	Str255			textStr;
	
	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 9; cnt++) {
		GetDialogItem(dlog,
			kBaseAddressItem+cnt, &itemType, &itemH, &itemRect);
		if ( itemH != NULL ) {
			GetDialogItemText( itemH, textStr );
			HLock( (Handle)gAddressesH );
			HLock( (Handle)(*gAddressesH)[cnt] );
			if ( (*gAddressesH)[cnt] != NULL )
				SetString( (StringHandle)(*gAddressesH)[cnt], textStr );
			HUnlock( (Handle)(*gAddressesH)[cnt] );
			HUnlock( (Handle)gAddressesH );
		}
	}	
}


/***************** InitAddressData ********************/

void	InitAddressData()
{	short			cnt;
	Handle			resH;

	gAddressesH = (Handle**)NewHandle(10 * sizeof(Handle));
	
	HLock( (Handle)gAddressesH );
	UseResFile(gOurResRefNum);
	for( cnt = 0; cnt < 9; cnt++) {
		resH = Get1Resource ('STR ', kAddressStrBaseID+cnt);
		if ( resH != NULL ) {
			HLock( resH );
			(*gAddressesH)[cnt] = (Handle)NewString( (const uchar *)*resH );
			HUnlock( resH );
			ReleaseResource(resH);
		}
	}
	(*gAddressesH)[cnt] = kNullPtr;
	HUnlock( (Handle)gAddressesH );
}


/***************** DoContextDlog ********************/

void	DoContextDlog( void )
{
	DialogPtr		dlog;
	GrafPtr			savePort;
	Boolean			done = FALSE;
	short			itemHit;
	Handle			theControlH;
	Rect			itemRect;
	short			itemType;
		
	dlog = GetNewDialog( kContextDlogID, kNullPtr, kMoveToFront );	
	if ( dlog != NULL )
	{
		GetPort( &savePort );
		SetPort( dlog );
//		SetWindowFont(dlog, geneva, 9, 0, srcCopy);

		TurnOffAllRadials( dlog );
		
		/* Set radial for current value. */
	switch ( gContext ) {
	case EMSF_ON_ARRIVAL:
		GetDialogItem(dlog,
			kOnArrival, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		break;
	case EMSF_ON_DISPLAY:
		GetDialogItem(dlog,
			kOnDisplay, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		break;
	case EMSF_ON_REQUEST:
		GetDialogItem(dlog,
			kOnRequest, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		break;
	case EMSF_Q4_COMPLETION:
		GetDialogItem(dlog,
			kQ4Completion, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		break;
	case EMSF_Q4_TRANSMISSION:
		GetDialogItem(dlog,
			kQ4Transmission, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		break;
	default:
		break;
		}

		ShowWindow( dlog );
		while ( !done ) {
	ModalDialog( kNullPtr, &itemHit );
	switch (itemHit) {
	case kOK:
		done = TRUE;
		break;
	case kCancel:
		done = TRUE;
		break;
	case kOnArrival:
		TurnOffAllRadials( dlog );
		GetDialogItem(dlog,
			kOnArrival, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		gContext = EMSF_ON_ARRIVAL;
		break;
	case kOnDisplay:
		TurnOffAllRadials( dlog );
		GetDialogItem(dlog,
			kOnDisplay, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		gContext = EMSF_ON_DISPLAY;
		break;
	case kOnRequest:
		TurnOffAllRadials( dlog );
		GetDialogItem(dlog,
			kOnRequest, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		gContext = EMSF_ON_REQUEST;
		break;
	case kQ4Completion:
		TurnOffAllRadials( dlog );
		GetDialogItem(dlog,
			kQ4Completion, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		gContext = EMSF_Q4_COMPLETION;
		break;
	case kQ4Transmission:
		TurnOffAllRadials( dlog );
		GetDialogItem(dlog,
			kQ4Transmission, &itemType, &theControlH, &itemRect);	
		SetControlValue((ControlHandle)theControlH, true);
		gContext = EMSF_Q4_TRANSMISSION;
		break;
	default:
		break;
		}
		}
	
		// clean up
		DisposeDialog(dlog);
		SetPort(savePort);
	}
}


/***************** TurnOffAllRadials ********************/

void	TurnOffAllRadials( DialogPtr dlog )
{	Handle		theControlH;
Rect		itemRect;
short		itemType;

GetDialogItem(dlog, kOnArrival, &itemType, &theControlH, &itemRect);	
SetControlValue((ControlHandle)theControlH, false);

GetDialogItem(dlog, kOnDisplay, &itemType, &theControlH, &itemRect);	
SetControlValue((ControlHandle)theControlH, false);

GetDialogItem(dlog, kOnRequest, &itemType, &theControlH, &itemRect);	
SetControlValue((ControlHandle)theControlH, false);

GetDialogItem(dlog, kQ4Completion, &itemType, &theControlH, &itemRect);	
SetControlValue((ControlHandle)theControlH, false);

GetDialogItem(dlog, kQ4Transmission, &itemType, &theControlH, &itemRect);	
SetControlValue((ControlHandle)theControlH, false);
}


/***************** DoMessage ********************/

void	DoMessage( Str255 errorString )
{
	ParamText( errorString, "\p", "\p", "\p" );
	
	NoteAlert( kErrorALRTid, nil );
}


/***************** DoError ********************/

void	DoError( Str255 errorString )
{
	ParamText( errorString, "\p", "\p", "\p" );
	
	StopAlert( kErrorALRTid, nil );
	
	ExitToShell();
}


/****************** MenuBarInit ***********************/

void	MenuBarInit( void )
{
	Handle			menuBar;
	MenuHandle		menu;
	
	menuBar = GetNewMBar( kBaseResID );
	
	if ( menuBar == NULL )
		DoError( "\pCouldn't load the MBAR resource..." );
	
	SetMenuBar( menuBar );

	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
	
	DrawMenuBar();
}


/****************** SetWindowFont ***********************/

void SetWindowFont(DialogPtr dlog, short fontNum, short fontSize, Style
fontStyle, short fontMode)
{
	FontInfo	fInfo;
	GrafPtr		savePort;
	
	GetPort(&savePort);
	SetPort(dlog);
	
	TextFont(fontNum);
    TextSize(fontSize);
    TextFace(fontStyle);
    TextMode(fontMode);
   	
   	GetFontInfo(&fInfo);

    (*((DialogPeek)dlog)->textH)->fontAscent = fInfo.ascent;
    (*((DialogPeek)dlog)->textH)->lineHeight =
    	fInfo.ascent + fInfo.descent + fInfo.leading;
    (*((DialogPeek)dlog)->textH)->txFont = fontNum;
    (*((DialogPeek)dlog)->textH)->txFace = fontStyle;
    (*((DialogPeek)dlog)->textH)->txMode = fontMode;
    (*((DialogPeek)dlog)->textH)->txSize = fontSize;

    SetPort(savePort);
}


/****************** ToolboxInit ***********************/

void ToolboxInit()
{
	MaxApplZone();				/* grow the heap to its maximum size */

	InitGraf(&qd.thePort);	  	/* init Quickdraw and global variables		*/
	InitFonts();		   		/* initialize font manager					*/
	InitWindows();		   		/* init window manager, setup WMgr GrafPort	*/
	InitMenus();				  				/* initialize menu Manager	*/
	TEInit();		      		/* inititalize TextEdit						*/
	InitDialogs(0L);			/* initialize Dialog manager				*/

	FlushEvents(everyEvent, 0);	/* clear the Event queue of all events 		*/
	InitCursor();		      	/* set the Cursor to arrow instead of clock	*/

} /* end of ToolboxInit */

short callProgress(emsProgress glop, short percent)
{ 	DialogPtr		dlog;
	GrafPtr			savePort;
	Boolean			done = FALSE;
	short			itemHit, returnValue;
	Str255			percentStr;
	short			itemType;
	Handle			itemH;
	Rect			itemRect;
#pragma unused(glop)
	/* The translator should call this function periodically
	 * with the argument (0)
	 * (just begun) and 100 (complete).
	 * The translator should check the return
	 * value.
	 * 		0 - continue translation
	 *		1 - abort translation
	 *
	 * If you are supplying your own progress function, call this with -1
	 * to check for abort.
	 */

  returnValue = 0;
	dlog = GetNewDialog( kProgressDlogID, kNullPtr, kMoveToFront );	
	if ( dlog != NULL ) {
		GetPort( &savePort );
		SetPort( dlog );
		
		NumToString( percent, percentStr );
		GetDialogItem(dlog, kPercentItem, &itemType, &itemH, &itemRect);
		if ( itemH != NULL )
			SetDialogItemText( itemH, percentStr );

		ShowWindow( dlog );
		while ( !done ) {
			ModalDialog( kNullPtr, &itemHit );
			switch (itemHit) {
				case kOK:
					done = TRUE;
					returnValue = 0;
					break;
				case kCancel:
					done = TRUE;
					returnValue = 1;
					break;
				default:
					break;
			}
		}
		// clean up
  	DisposeDialog(dlog);		
	  SetPort(savePort);
	}
	return( returnValue );
}


