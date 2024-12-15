/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CPopUpMenuSelectPatcher.h,v 1.2.20.1 1998/11/12 03:10:20 heller Exp $____________________________________________________________________________*/#pragma once #include "CPatcher.h"class CPopUpMenuSelectPatcher	:	public	CPatcher {public:#if GENERATINGCFM	enum {kProcInfo = 	kPascalStackBased		| RESULT_SIZE(SIZE_CODE(sizeof(long)))		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(MenuRef)))		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short)))};#else	enum {kProcInfo = 0};	typedef pascal long			(*PopUpMenuSelectProc)(MenuRef menu,									short top, short left, short popUpItem);#endif										CPopUpMenuSelectPatcher();	virtual						~CPopUpMenuSelectPatcher();protected:	static CPopUpMenuSelectPatcher *	sPopUpMenuSelectPatcher;		virtual long				NewPopUpMenuSelect(MenuRef menu, short top,									short left, short popUpItem);	virtual long				OldPopUpMenuSelect(MenuRef menu, short top,									short left, short popUpItem);	static pascal long			PopUpMenuSelectCallback(MenuRef menu,									short top, short left, short popUpItem);};