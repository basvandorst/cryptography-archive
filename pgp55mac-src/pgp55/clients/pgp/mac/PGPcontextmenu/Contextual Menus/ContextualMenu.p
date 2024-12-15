{$IFC UNDEFINED UsingIncludes}
{$SETC UsingIncludes := 0}
{$ENDC}

{$IFC NOT UsingIncludes}
 UNIT ContextualMenu;
 INTERFACE
{$ENDC}

{$IFC UNDEFINED __CONTEXTUALMENU__}
{$SETC __CONTEXTUALMENU__ := 1}

{$I+}
{$SETC ContextualMenuIncludes := UsingIncludes}
{$SETC UsingIncludes := 1}

{$IFC UNDEFINED __APPLEEVENTS__}
{$I AppleEvents.p}
{$ENDC}
{$IFC UNDEFINED __EVENTS__}
{$I Events.p}
{$ENDC}
{$IFC UNDEFINED __MENUS__}
{$I Menus.p}
{$ENDC}

{$PUSH}
{$ALIGN MAC68K}
{$LibExport+}


CONST
	_ContextualMenuDispatch		= $AA72;						{  Contextual Menu Trap Number }

																{  Gestalt Selectors  }
	gestaltContextualMenuAttr	= 'cmnu';
	gestaltContextualMenuPresent = 0;
	gestaltContextualMenuTrapAvailable = 1;

																{  Values indicating what kind of help the application supports  }
	kCMHelpItemNoHelp			= 0;
	kCMHelpItemAppleGuide		= 1;
	kCMHelpItemOtherHelp		= 2;							{  Values indicating what was chosen from the menu  }
	kCMNothingSelected			= 0;
	kCMMenuItemSelected			= 1;
	kCMShowBalloonSelected		= 2;
	kCMShowHelpSelected			= 3;

FUNCTION InitContextualMenus: OSStatus;
	{$IFC NOT GENERATINGCFM}
	INLINE $7001, $AA72;
	{$ENDC}
FUNCTION IsShowContextualMenuClick({CONST}VAR inEvent: EventRecord): BOOLEAN;
	{$IFC NOT GENERATINGCFM}
	INLINE $7002, $AA72;
	{$ENDC}
FUNCTION ContextualMenuSelect(inMenuRef: MenuRef; inGlobalLocation: Point; inBalloonAvailable: BOOLEAN; inHelpType: UInt32; inHelpItemString: Str255; {CONST}VAR inSelection: AEDesc; VAR outUserSelectionType: UInt32; VAR outMenuID: SInt16; VAR outMenuItem: UInt16): OSStatus;
	{$IFC NOT GENERATINGCFM}
	INLINE $7003, $AA72;
	{$ENDC}
{$ALIGN RESET}
{$POP}

{$SETC UsingIncludes := ContextualMenuIncludes}

{$ENDC} {__CONTEXTUALMENU__}

{$IFC NOT UsingIncludes}
 END.
{$ENDC}
