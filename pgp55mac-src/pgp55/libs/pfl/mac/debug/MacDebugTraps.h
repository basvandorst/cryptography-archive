/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDebugTraps.h,v 1.9 1997/09/18 01:34:28 lloyd Exp $
____________________________________________________________________________*/

#pragma once



#if ! PGP_DEBUG
/* debug traps should not compile into a non-DEBUG version */
	#undef USE_MAC_DEBUG_TRAPS
	
#define USE_MAC_DEBUG_TRAPS	0
#endif


#if USE_MAC_DEBUG_TRAPS	/* [ */

#include <Aliases.h>
#include <Dialogs.h>
#include <Icons.h>
#include <Menus.h>


PGP_BEGIN_C_DECLARATIONS


/*____________________________________________________________________________
	Note:
	The MDT_FALArgs directive can be used to record the file name
	in which a memory allocation is made.
	This information can be used by the DebugLeaks code.
	MDT_FALArgs generates a null-terminated C-string.
____________________________________________________________________________*/

#define MDT_FALArgs	 	__FILE__, __LINE__

#define MDT_FileAndLineParams	const char *srcFile_, long srcLine_



/*_____________________________ Memory Manager _____________________________*/


#define BlockMove				DebugTraps_BlockMove

#define BlockMoveData			DebugTraps_BlockMoveData

#define NewPtr(sz)				DebugTraps_NewPtr( sz, MDT_FALArgs)

#define NewPtrClear(sz)		DebugTraps_NewPtrClear( sz, MDT_FALArgs)

#define NewPtrSys(sz)			DebugTraps_NewPtrSys( sz, MDT_FALArgs)

#define NewPtrSysClear(sz)		\
	DebugTraps_NewPtrSysClear( sz, MDT_FALArgs)

#define GetPtrSize				DebugTraps_GetPtrSize

#define SetPtrSize				DebugTraps_SetPtrSize

#define PtrZone					DebugTraps_PtrZone

#define DisposePtr				DebugTraps_DisposePtr

#define PtrToHand(s,d,z)		\
			DebugTraps_PtrToHand( s, d, z, MDT_FALArgs)

#define NewHandle(sz)			DebugTraps_NewHandle( sz, MDT_FALArgs)

#define NewHandleClear(sz)		\
			DebugTraps_NewHandleClear( sz, MDT_FALArgs)

#define NewHandleSys(sz)		\
			DebugTraps_NewHandleSys( sz, MDT_FALArgs)

#define NewHandleSysClear(sz)	\
			DebugTraps_NewHandleSysClear( sz, MDT_FALArgs)

#define GetHandleSize			DebugTraps_GetHandleSize

#define SetHandleSize			DebugTraps_SetHandleSize

#define ReallocateHandle		DebugTraps_ReallocateHandle

#define DisposeHandle			DebugTraps_DisposeHandle

#define HLock					DebugTraps_HLock

#define HUnlock					DebugTraps_HUnlock

#define HGetState				DebugTraps_HGetState

#define HSetState				DebugTraps_HSetState

#define HandAndHand				DebugTraps_HandAndHand

#define HandToHand(h)			DebugTraps_HandToHand( h, MDT_FALArgs)

#define HPurge					DebugTraps_HPurge

#define HNoPurge				DebugTraps_HNoPurge

#define EmptyHandle				DebugTraps_EmptyHandle

#define HandleZone				DebugTraps_HandleZone

#define RecoverHandle			DebugTraps_RecoverHandle

#define RecoverHandleSys		DebugTraps_RecoverHandleSys

#define SetZone					DebugTraps_SetZone

#define GetZone					DebugTraps_GetZone

#define TempNewHandle(sz, err)	\
			DebugTraps_TempNewHandle( sz, err, MDT_FALArgs)

#define TempDisposeHandle		DebugTraps_TempDisposeHandle

#define MoveHHi					DebugTraps_MoveHHi

#define PtrAndHand				DebugTraps_PtrAndHand




pascal void 	DebugTraps_BlockMove(const void *srcPtr,
					void *destPtr, Size byteCount);
pascal void 	DebugTraps_BlockMoveData(const void *srcPtr,
					void *destPtr, Size byteCount);

pascal Ptr		DebugTraps_NewPtr(Size byteCount, MDT_FileAndLineParams);
pascal Ptr		DebugTraps_NewPtrSys(Size byteCount, MDT_FileAndLineParams);
pascal Ptr		DebugTraps_NewPtrClear(Size byteCount, MDT_FileAndLineParams);
pascal Ptr		DebugTraps_NewPtrSysClear(Size byteCount,
					MDT_FileAndLineParams);
pascal Size		DebugTraps_GetPtrSize(Ptr p);
pascal void		DebugTraps_SetPtrSize(Ptr p, Size newSize);
pascal THz		DebugTraps_PtrZone(Ptr p);
pascal void		DebugTraps_DisposePtr(Ptr p);

pascal OSErr	DebugTraps_PtrToHand(const void *srcPtr,
					Handle *dstHndl, long size, MDT_FileAndLineParams);
pascal Handle	DebugTraps_NewHandle(Size byteCount, MDT_FileAndLineParams);
pascal Handle	DebugTraps_NewHandleClear(Size byteCount,
					MDT_FileAndLineParams);
pascal Handle	DebugTraps_NewHandleSys(Size byteCount,
					MDT_FileAndLineParams);
pascal Handle	DebugTraps_NewHandleSysClear(Size byteCount,
					MDT_FileAndLineParams);
pascal Size 	DebugTraps_GetHandleSize(Handle h );
pascal void 	DebugTraps_SetHandleSize(Handle h, Size newSize);
pascal void		DebugTraps_ReallocateHandle(Handle h, Size byteCount);
pascal void 	DebugTraps_DisposeHandle(Handle h);
pascal void 	DebugTraps_HLock(Handle h);
pascal void 	DebugTraps_HUnlock(Handle h);
pascal SInt8	DebugTraps_HGetState(Handle h);
pascal void		DebugTraps_HSetState(Handle h, SInt8 flags);
pascal OSErr 	DebugTraps_HandAndHand(Handle hand1, Handle hand2);
pascal OSErr	DebugTraps_HandToHand(Handle *theHndl, MDT_FileAndLineParams);
pascal void		DebugTraps_HPurge(Handle h);
pascal void		DebugTraps_HNoPurge(Handle h);
pascal void		DebugTraps_EmptyHandle(Handle h);
pascal THz		DebugTraps_HandleZone(Handle h);
pascal Handle	DebugTraps_RecoverHandle(Ptr p);
pascal Handle	DebugTraps_RecoverHandleSys(Ptr p);
pascal void		DebugTraps_MoveHHi(Handle h);
pascal OSErr	DebugTraps_PtrAndHand(const void *ptr1,
					Handle hand2, long size);

pascal void		DebugTraps_SetZone(THz hz);
pascal THz		DebugTraps_GetZone(void);

pascal Handle	DebugTraps_TempNewHandle(Size logicalSize,
					OSErr *resultCode, MDT_FileAndLineParams);
pascal void		DebugTraps_TempDisposeHandle( Handle h, OSErr *err);




/*_________________________ Resource Manager _______________________________*/



#define ReleaseResource	DebugTraps_ReleaseResource


#define CloseResFile		DebugTraps_CloseResFile

#define HomeResFile			DebugTraps_HomeResFile

#define CreateResFile		DebugTraps_CreateResFile

#define OpenResFile			DebugTraps_OpenResFile

#define UseResFile			DebugTraps_UseResFile

#define GetIndType			DebugTraps_GetIndType

#define Get1IndType			DebugTraps_Get1IndType

#define GetIndResource		DebugTraps_GetIndResource

#define Get1IndResource		DebugTraps_Get1IndResource

#define GetResource			DebugTraps_GetResource

#define Get1Resource		DebugTraps_Get1Resource

#define GetNamedResource	DebugTraps_GetNamedResource

#define Get1NamedResource	DebugTraps_Get1NamedResource

#define LoadResource		DebugTraps_LoadResource

#define ReleaseResource		DebugTraps_ReleaseResource

#define DetachResource(r)	\
			DebugTraps_DetachResource( r, MDT_FALArgs )

#define GetResAttrs			DebugTraps_GetResAttrs

#define GetResInfo			DebugTraps_GetResInfo

#define SetResInfo			DebugTraps_SetResInfo

#define AddResource			DebugTraps_AddResource

#define GetResourceSizeOnDisk	DebugTraps_GetResourceSizeOnDisk

#define GetMaxResourceSize	DebugTraps_GetMaxResourceSize

#define RsrcMapEntry		DebugTraps_RsrcMapEntry

#define SetResAttrs			DebugTraps_SetResAttrs

#define ChangedResource		DebugTraps_ChangedResource

#define RemoveResource		DebugTraps_RemoveResource

#define UpdateResFile		DebugTraps_UpdateResFile

#define WriteResource		DebugTraps_WriteResource

#define GetResFileAttrs		DebugTraps_GetResFileAttrs

#define SetResFileAttrs		DebugTraps_SetResFileAttrs

#define OpenRFPerm			DebugTraps_OpenRFPerm

#define HOpenResFile		DebugTraps_HOpenResFile

#define HCreateResFile		DebugTraps_HCreateResFile

#define FSpOpenResFile		DebugTraps_FSpOpenResFile

#define FSpCreateResFile	DebugTraps_FSpCreateResFile

#define ReadPartialResource	DebugTraps_ReadPartialResource

#define WritePartialResource	DebugTraps_WritePartialResource

#define SetResourceSize		DebugTraps_SetResourceSize

#define GetNextFOND			DebugTraps_GetNextFOND

	
	
pascal void		DebugTraps_ReleaseResource(Handle theResource);
pascal void		DebugTraps_CloseResFile(short refNum);
pascal short	DebugTraps_HomeResFile(Handle theResource);
pascal void		DebugTraps_CreateResFile(ConstStr255Param fileName);
pascal short	DebugTraps_OpenResFile(ConstStr255Param fileName);
pascal void		DebugTraps_UseResFile(short refNum);
pascal void		DebugTraps_GetIndType(ResType *theType, short index);
pascal void		DebugTraps_Get1IndType(ResType *theType, short index);
pascal Handle	DebugTraps_GetIndResource(ResType theType, short index);
pascal Handle	DebugTraps_Get1IndResource(ResType theType, short index);
pascal Handle	DebugTraps_GetResource(ResType theType, short theID);
pascal Handle	DebugTraps_Get1Resource(ResType theType, short theID);
pascal Handle	DebugTraps_GetNamedResource(ResType theType,
					ConstStr255Param name);
pascal Handle	DebugTraps_Get1NamedResource(ResType theType,
					ConstStr255Param name);
pascal void		DebugTraps_LoadResource(Handle theResource);
pascal void		DebugTraps_ReleaseResource(Handle theResource);
pascal void		DebugTraps_DetachResource(Handle theResource,
					MDT_FileAndLineParams);
pascal short	DebugTraps_GetResAttrs(Handle theResource);
pascal void		DebugTraps_GetResInfo(Handle theResource,
					short *theID, ResType *theType, Str255 name);
pascal void		DebugTraps_SetResInfo(Handle theResource, short theID,
					ConstStr255Param name);
pascal void		DebugTraps_AddResource(Handle theData, ResType theType,
					short theID, ConstStr255Param name);
pascal long		DebugTraps_GetResourceSizeOnDisk(Handle theResource);
pascal long		DebugTraps_GetMaxResourceSize(Handle theResource);
pascal long		DebugTraps_RsrcMapEntry(Handle theResource);
pascal void		DebugTraps_SetResAttrs(Handle theResource, short attrs);
pascal void		DebugTraps_ChangedResource(Handle theResource);
pascal void		DebugTraps_RemoveResource(Handle theResource);
pascal void		DebugTraps_UpdateResFile(short refNum);
pascal void		DebugTraps_WriteResource(Handle theResource);
pascal short	DebugTraps_GetResFileAttrs(short refNum);
pascal void		DebugTraps_SetResFileAttrs(short refNum, short attrs);
pascal short	DebugTraps_OpenRFPerm(ConstStr255Param fileName,
					short vRefNum, SInt8 permission);
pascal short	DebugTraps_HOpenResFile(short vRefNum, long dirID,
					ConstStr255Param fileName, SInt8 permission);
pascal void		DebugTraps_HCreateResFile(short vRefNum, long dirID,
					ConstStr255Param fileName);
pascal short	DebugTraps_FSpOpenResFile(const FSSpec *spec,
					SignedByte permission);
pascal void		DebugTraps_FSpCreateResFile(const FSSpec *spec,
					OSType creator, OSType fileType, ScriptCode scriptTag);
pascal void		DebugTraps_ReadPartialResource(Handle theResource,
					long offset, void *buffer, long count);
pascal void		DebugTraps_WritePartialResource(Handle theResource,
					long offset, const void	*buffer, long count);
pascal void		DebugTraps_SetResourceSize(Handle theResource, long newSize);
pascal Handle	DebugTraps_GetNextFOND(Handle fondHandle);







/*_____________________________ Quickdraw _________________________________*/


#define NewRgn()			DebugTraps_NewRgn( MDT_FALArgs )

#define DisposeRgn			DebugTraps_DisposeRgn

pascal RgnHandle			DebugTraps_NewRgn( MDT_FileAndLineParams );
pascal void					DebugTraps_DisposeRgn( RgnHandle theRgn );



/*_____________________________ Menu Manager _________________________________*/


#define NewMenu( id, title )	DebugTraps_NewMenu( id, title,\
									MDT_FALArgs )

#define GetMenuBar( )			DebugTraps_GetMenuBar( MDT_FALArgs )

#define GetNewMBar( id )		DebugTraps_GetNewMBar( id, MDT_FALArgs)

pascal MenuRef				DebugTraps_NewMenu( short menuID,
								ConstStr255Param menuTitle,
								MDT_FileAndLineParams);
pascal MenuRef				DebugTraps_GetMenu( short menuID,
								MDT_FileAndLineParams);
pascal Handle				DebugTraps_GetMenuBar( MDT_FileAndLineParams );
pascal Handle				DebugTraps_GetNewMBar(short menuBarID,
								MDT_FileAndLineParams);




/*_____________________________ Icons _________________________________*/


#define NewIconSuite( suite )				\
			DebugTraps_NewIconSuite( suite, MDT_FALArgs )

#define GetIconSuite( suite, resID, sel,  )	\
			DebugTraps_GetIconSuite( suite, resID, sel, MDT_FALArgs )

#define DisposeIconSuite					DebugTraps_DisposeIconSuite


pascal OSErr		DebugTraps_NewIconSuite(Handle *theIconSuite,
						MDT_FileAndLineParams);
pascal OSErr		DebugTraps_GetIconSuite( Handle *theIconSuite,
						SInt16 theResID, IconSelectorValue selector,
						MDT_FileAndLineParams);
pascal OSErr		DebugTraps_DisposeIconSuite(Handle theIconSuite,
						Boolean disposeData);




/*_____________________________ Alias Manager ______________________________*/

#define NewAlias(from, targ, al)		\
			DebugTraps_NewAlias( from, targ, al, MDT_FALArgs )

#define NewAliasMinimal( targ, al )		\
			DebugTraps_NewAliasMinimal( targ, al, MDT_FALArgs )

#define NewAliasMinimalFromFullPath( pl, fp, zn, sn, al)	\
			DebugTraps_NewAliasMinimalFromFullPath( pl, fp, zn, sn, al,\
			MDT_FALArgs)

pascal OSErr	DebugTraps_NewAlias(ConstFSSpecPtr fromFile,
					const FSSpec *target,
					AliasHandle *alias, MDT_FileAndLineParams);
pascal OSErr	DebugTraps_NewAliasMinimal(const FSSpec *target,
						AliasHandle *alias,
						MDT_FileAndLineParams);
pascal OSErr	DebugTraps_NewAliasMinimalFromFullPath(
					short fullPathLength,
					const void *fullPath, ConstStr32Param zoneName,
					ConstStr31Param serverName, AliasHandle *alias,
					MDT_FileAndLineParams);





/*______________________ Apple Event Related _______________________________*/


#define AECreateDesc( type, data, size, result )					\
	DebugTraps_AECreateDesc( type, data, size, result, MDT_FALArgs)	

#define AECoercePtr( type, data, size, toType, result )				\
	DebugTraps_AECoercePtr( type, data, size, toType, result, MDT_FALArgs)	

#define AECoerceDesc( desc, toType, result )						\
	DebugTraps_AECoerceDesc( desc, toType, result, MDT_FALArgs)

#define AEDisposeDesc												\
	DebugTraps_AEDisposeDesc

#define AEDuplicateDesc( desc, result )								\
	DebugTraps_AEDuplicateDesc( desc, result, MDT_FALArgs)

#define AECreateList( facPtr, facSize, isRec, result)				\
	DebugTraps_AECreateList( facPtr, facSize, isRec, result, MDT_FALArgs)

#define AEGetNthDesc( aeList, index, desType, keyw, res )		\
	DebugTraps_AEGetNthDesc( aeList, index, desType, keyw, res, MDT_FALArgs)

#define AEGetAttributeDesc( event, keyw, desType, result )			\
	DebugTraps_AEGetAttributeDesc( event, keyw, desType, result, MDT_FALArgs)

#define AECreateAppleEvent( aeClass, id, targ, retID, tranID, result )	\
	DebugTraps_AECreateAppleEvent( aeClass, id, targ, retID, tranID,\
	result, MDT_FALArgs)

#define AESend														\
	DebugTraps_AESend

#define AEGetTheCurrentEvent( event )									\
	DebugTraps_AEGetTheCurrentEvent( event, MDT_FALArgs)

#define AEResolve( objSpec, flags, desc )								\
	DebugTraps_AEResolve( objSpec, flags, desc, MDT_FALArgs)

#define CreateOffsetDescriptor( offset, desc )							\
	DebugTraps_CreateOffsetDescriptor( offset, desc, MDT_FALArgs)

#define CreateCompDescriptor(compare, op1, op2, dispose, desc )			\
	DebugTraps_CreateCompDescriptor(compare, op1, op2, dispose, desc,\
	MDT_FALArgs)

#define CreateLogicalDescriptor( terms, operator, dispose, desc )		\
	DebugTraps_CreateLogicalDescriptor( terms, operator, dispose, desc,\
	MDT_FALArgs)

#define CreateObjSpecifier( desClass, cont, keyForm, keyData,\
		dispose, objSpec )		\
		DebugTraps_CreateObjSpecifier( desClass, cont, keyForm, \
		keyData, dispose, objSpec, MDT_FALArgs)

#define CreateRangeDescriptor( start, stop, dispose, desc )				\
	DebugTraps_CreateRangeDescriptor( start, stop, dispose, desc, MDT_FALArgs)

	
	

pascal OSErr	DebugTraps_AECreateDesc(DescType typeCode,
					const void *dataPtr,
					Size dataSize, AEDesc *result,
					MDT_FileAndLineParams);
				
pascal OSErr	DebugTraps_AECoercePtr(DescType typeCode, const void *dataPtr,
					Size dataSize, DescType toType, AEDesc *result,
					MDT_FileAndLineParams);
				
pascal OSErr	DebugTraps_AECoerceDesc(const AEDesc *theAEDesc,
					DescType toType, AEDesc *result,
					MDT_FileAndLineParams);
				
pascal OSErr	DebugTraps_AEDisposeDesc( AEDesc *theAEDesc );

pascal OSErr	DebugTraps_AEDuplicateDesc(const AEDesc *theAEDesc,
					AEDesc *result,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_AECreateList(const void *factoringPtr,
					Size factoredSize,
					Boolean isRecord, AEDescList *resultList,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_AEGetNthDesc(const AEDescList *theAEDescList,
					long index,
					DescType desiredType, AEKeyword *theAEKeyword,
					AEDesc *result, MDT_FileAndLineParams);

pascal OSErr	DebugTraps_AEGetAttributeDesc(const AppleEvent *theAppleEvent,
					 AEKeyword theAEKeyword,
					DescType desiredType, AEDesc *result,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_AECreateAppleEvent(AEEventClass theAEEventClass,
					AEEventID theAEEventID,
					const AEAddressDesc *target, short returnID,
					long transactionID, AppleEvent *result,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_AESend(const AppleEvent *theAppleEvent,
					AppleEvent *reply, AESendMode sendMode,
					AESendPriority sendPriority, long timeOutInTicks,
					AEIdleUPP idleProc,
					AEFilterUPP filterProc);

pascal OSErr	DebugTraps_AEGetTheCurrentEvent(AppleEvent *theAppleEvent,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_AEResolve(const AEDesc *objectSpecifier,
					short callbackFlags, AEDesc *theToken,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_CreateOffsetDescriptor(long theOffset,
					AEDesc *theDescriptor,
					MDT_FileAndLineParams);
					
pascal OSErr	DebugTraps_CreateCompDescriptor(DescType comparisonOperator,
					AEDesc *operand1,
					AEDesc *operand2, Boolean disposeInputs,
					AEDesc *theDescriptor,
					MDT_FileAndLineParams);
					
pascal OSErr	DebugTraps_CreateLogicalDescriptor(AEDescList *theLogicalTerms,
					DescType theLogicOperator, Boolean disposeInputs,
					AEDesc *theDescriptor,
					MDT_FileAndLineParams);
					
pascal OSErr	DebugTraps_CreateObjSpecifier(DescType desiredClass,
					AEDesc *theContainer,
					DescType keyForm, AEDesc *keyData,
					Boolean disposeInputs, AEDesc *objSpecifier,
					MDT_FileAndLineParams);

pascal OSErr	DebugTraps_CreateRangeDescriptor(AEDesc *rangeStart,
					AEDesc *rangeStop,
					Boolean disposeInputs, AEDesc *theDescriptor,
					MDT_FileAndLineParams);



/*_____________________________ Window Mgr _________________________________*/


#define GetNewWindow( id, stor, behind )			\
		DebugTraps_GetNewWindow( id, stor, behind, MDT_FALArgs )

#define NewWindow( stor, rect, title, vis, proc, behind, goAway, ref)\
			DebugTraps_NewWindow( stor, rect, title, vis, proc,\
			behind, goAway, ref, MDT_FALArgs )

#define GetNewCWindow( id, stor, behind )		\
			DebugTraps_GetNewCWindow( id, stor, behind, MDT_FALArgs )

#define NewCWindow( stor, rect, title, vis, proc, behind, goAway, ref)\
			DebugTraps_NewCWindow( stor, rect, title, vis, proc,\
			behind, goAway, ref, MDT_FALArgs )

#define DisposeWindow	DebugTraps_DisposeWindow

#define CloseWindow		DebugTraps_CloseWindow

pascal WindowRef	DebugTraps_GetNewWindow(short windowID,
						void *wStorage, WindowRef behind,
						MDT_FileAndLineParams);


pascal WindowRef	DebugTraps_NewWindow(void *wStorage,
						const Rect *boundsRect,
						ConstStr255Param title, Boolean visible,
						short theProc, WindowRef behind,
						Boolean goAwayFlag, long refCon,
						MDT_FileAndLineParams);

pascal WindowRef	DebugTraps_GetNewCWindow(short windowID,
						void *wStorage, WindowRef behind,
						MDT_FileAndLineParams);

pascal WindowRef	DebugTraps_NewCWindow(void *wStorage,
						const Rect *boundsRect,
						ConstStr255Param title,
						Boolean visible, short procID, WindowRef behind,
						Boolean goAwayFlag, long refCon,
						MDT_FileAndLineParams);

pascal void			DebugTraps_DisposeWindow(WindowRef theWindow);
pascal void			DebugTraps_CloseWindow(WindowRef theWindow);


/*_____________________________ Dialog Mgr _________________________________*/


#define NewDialog( stor, rect, title, vis, procID, \
			behind, goAway, ref, itemList )\
			DebugTraps_NewDialog( stor, rect, title, vis, procID,\
			behind, goAway, ref, itemList, MDT_FALArgs)

#define GetNewDialog( id, stor, behind )		\
	DebugTraps_GetNewDialog( id, stor, behind, MDT_FALArgs)

#define DisposeDialog		DebugTraps_DisposeDialog

#define CloseDialog			DebugTraps_CloseDialog

#define ModalDialog			DebugTraps_ModalDialog

#define Alert				DebugTraps_Alert

#define StopAlert			DebugTraps_StopAlert

#define NoteAlert			DebugTraps_NoteAlert

#define CautionAlert		DebugTraps_CautionAlert

pascal DialogRef	DebugTraps_NewDialog(void *wStorage,
						const Rect *boundsRect, ConstStr255Param title,
						Boolean visible, short procID,
						WindowRef behind, Boolean goAwayFlag,
						long refCon, Handle itmLstHndl,
						MDT_FileAndLineParams);
						
pascal DialogRef	DebugTraps_GetNewDialog(short dialogID,
						void *dStorage, WindowRef behind,
						MDT_FileAndLineParams);
						
pascal void			DebugTraps_DisposeDialog( DialogRef theDialog);
pascal void			DebugTraps_CloseDialog( DialogRef theDialog);


pascal void		DebugTraps_ModalDialog( ModalFilterUPP modalFilter,
					short *itemHit);
pascal short	DebugTraps_Alert( short alertID, ModalFilterUPP modalFilter);
pascal short	DebugTraps_StopAlert( short alertID, ModalFilterUPP mf);
pascal short	DebugTraps_NoteAlert( short alertID, ModalFilterUPP mf);
pascal short	DebugTraps_CautionAlert( short alertID, ModalFilterUPP mf);

/*_____________________________ Graphics _________________________________*/

#define DisposeGDevice( d )	DebugTraps_DisposeGDevice( d )

#define DisposeCTable( t )	DebugTraps_DisposeCTable( t )

pascal void	DebugTraps_DisposeGDevice( GDHandle	device );
pascal void DebugTraps_DisposeCTable( CTabHandle cTable );


/*_____________________________ File Mgr _________________________________*/


#define FSpCreate(spec, cr, ty, sc)		\
			DebugTraps_FSpCreate(spec, cr, ty, sc)

pascal OSErr	DebugTraps_FSpCreate(const FSSpec *spec,
					OSType creator, OSType fileType, ScriptCode scriptTag );

PGP_END_C_DECLARATIONS

#endif	/* ] */











