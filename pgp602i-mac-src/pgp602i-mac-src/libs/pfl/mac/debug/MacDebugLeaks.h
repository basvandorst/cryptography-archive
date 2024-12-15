/*____________________________________________________________________________	MacDebugLeaks.h		Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: MacDebugLeaks.h,v 1.8.28.1 1998/11/12 03:18:50 heller Exp $____________________________________________________________________________*/#pragma once#include "pgpLeaks.h"PGP_BEGIN_C_DECLARATIONS#if ! USE_PGP_LEAKS	#undef USE_MAC_DEBUG_LEAKS	#define USE_MAC_DEBUG_LEAKS     0#endif #if USE_MAC_DEBUG_LEAKS	/* [ *//* stack crawl requires A6 frames */#ifdef __MWERKS__	#if GENERATING68K		#pragma a6frames on	#endif#endif#include "pgpLeaks.h"extern LeakDeallocType *	kMacLeaks_DisposeHandleDeallocType;extern LeakDeallocType *	kMacLeaks_DisposePtrDeallocType;extern LeakDeallocType *	kMacLeaks_DisposeRgnDeallocType;extern LeakDeallocType *	kMacLeaks_DisposeIconSuiteDeallocType;extern LeakDeallocType *	kMacLeaks_DisposeAEDescDeallocType;extern LeakDeallocType *	kMacLeaks_DisposeWindowDeallocType;extern LeakDeallocType *	kMacLeaks_CloseWindowDeallocType;extern LeakDeallocType *	kMacLeaks_DisposeDialogDeallocType;extern LeakDeallocType *	kMacLeaks_CloseDialogDeallocType;extern LeakAllocType *		kMacLeaks_NewPtrAllocType;extern LeakAllocType *		kMacLeaks_NewPtrClearAllocType;extern LeakAllocType *		kMacLeaks_NewPtrSysAllocType;extern LeakAllocType *		kMacLeaks_NewPtrSysClearAllocType;extern LeakAllocType *		kMacLeaks_NewHandleAllocType;extern LeakAllocType *		kMacLeaks_NewHandleClearAllocType;extern LeakAllocType *		kMacLeaks_NewHandleSysAllocType;extern LeakAllocType *		kMacLeaks_NewHandleSysClearAllocType;extern LeakAllocType *		kMacLeaks_PtrToHandAllocType;extern LeakAllocType *		kMacLeaks_HandToHandAllocType;extern LeakAllocType *		kMacLeaks_TempNewHandleAllocType;extern LeakAllocType *		kMacLeaks_DetachResourceAllocType;extern LeakAllocType *		kMacLeaks_NewRgnAllocType;extern LeakAllocType *		kMacLeaks_NewMenuAllocType;extern LeakAllocType *		kMacLeaks_GetMenuBarAllocType;extern LeakAllocType *		kMacLeaks_GetNewMBarAllocType;extern LeakAllocType *		kMacLeaks_NewIconSuiteAllocType;extern LeakAllocType *		kMacLeaks_GetIconSuiteAllocType;extern LeakAllocType *		kMacLeaks_GetNewWindowAllocType;extern LeakAllocType *		kMacLeaks_NewWindowAllocType;extern LeakAllocType *		kMacLeaks_GetNewCWindowAllocType;extern LeakAllocType *		kMacLeaks_NewCWindowAllocType;extern LeakAllocType *		kMacLeaks_NewDialogAllocType;extern LeakAllocType *		kMacLeaks_GetNewDialogAllocType;extern LeakAllocType *		kMacLeaks_GetNewWindowStorageAllocType;extern LeakAllocType *		kMacLeaks_NewWindowStorageAllocType;extern LeakAllocType *		kMacLeaks_GetNewCWindowStorageAllocType;extern LeakAllocType *		kMacLeaks_NewCWindowStorageAllocType;extern LeakAllocType *		kMacLeaks_NewDialogStorageAllocType;extern LeakAllocType *		kMacLeaks_GetNewDialogStorageAllocType;extern LeakAllocType *		kMacLeaks_NewAliasAllocType;extern LeakAllocType *		kMacLeaks_NewAliasMinimalAllocType;extern LeakAllocType *		kMacLeaks_NewAliasMinimalFromFullPathAllocType;extern LeakAllocType *		kMacLeaks_AECreateDescAllocType;extern LeakAllocType *		kMacLeaks_AECreateDescAllocType;extern LeakAllocType *		kMacLeaks_AECoercePtrAllocType;extern LeakAllocType *		kMacLeaks_AECoerceDescAllocType;extern LeakAllocType *		kMacLeaks_AEDuplicateDescAllocType;extern LeakAllocType *		kMacLeaks_AECreateListAllocType;extern LeakAllocType *		kMacLeaks_AEGetNthDescAllocType;extern LeakAllocType *		kMacLeaks_AEGetAttributeDescAllocType;extern LeakAllocType *		kMacLeaks_AECreateAppleEventAllocType;extern LeakAllocType *		kMacLeaks_AEGetTheCurrentEventAllocType;extern LeakAllocType *		kMacLeaks_AEResolveAllocType;extern LeakAllocType *		kMacLeaks_CreateOffsetDescriptorAllocType;extern LeakAllocType *		kMacLeaks_CreateCompDescriptorAllocType;extern LeakAllocType *		kMacLeaks_CreateLogicalDescriptorAllocType;extern LeakAllocType *		kMacLeaks_CreateObjSpecifierAllocType;extern LeakAllocType *		kMacLeaks_CreateRangeDescriptorAllocType;void	MacLeaks_RememberHandle( const void * theHandle,			LeakAllocType *allocType,			const char *srcFileName, long lineNumber);			void	MacLeaks_ForgetHandle( const void * theHandle,			LeakDeallocType *deallocType );void	MacLeaks_RememberPtr( const void *thePtr,			LeakAllocType *allocType, UInt32 size,			const char *srcFileName, long lineNumber);void	MacLeaks_ForgetPtr( const void *thePtr, LeakDeallocType *deallocType);void	MacLeaks_RememberAEDesc( const AEDesc *theAEDesc, LeakAllocType *,			const char *srcFileName, long lineNumber);void	MacLeaks_ForgetAEDesc( const AEDesc *theAEDesc, LeakDeallocType *);void	MacLeaks_RememberObject( const void *object, UInt32 size,			const char *srcFileName, long lineNumber);void	MacLeaks_ForgetObject( const void *object );Boolean	MacLeaks_ItemIsRemembered( const void *handleOrPtr );void	MacLeaks_IgnoreItem( const void *item);void	MacLeaks_Suspend( void );void	MacLeaks_Resume( void );#else /* ] USE_MAC_DEBUG_LEAKS [ */	#define MacLeaks_RememberHandle(h, y, file, line)#define MacLeaks_ForgetHandle(h, d)				#define MacLeaks_RememberPtr(ptr, typ, sz, file, line)	#define MacLeaks_ForgetPtr( x, d)				#define MacLeaks_RememberAEDesc(x, y, file, line)#define MacLeaks_ForgetAEDesc(x, d)				#define MacLeaks_RememberObject( x, y )#define MacLeaks_ForgetObject( x )			#define MacLeaks_ItemIsRemembered( x )#define MacLeaks_DisposingHandle( x )			#define MacLeaks_DisposingPtr( x )			#define MacLeaks_IgnoreItem( x )#define MacLeaks_Suspend()		#define MacLeaks_Resume()		#endif /* ] USE_MAC_DEBUG_LEAKS */PGP_END_C_DECLARATIONS