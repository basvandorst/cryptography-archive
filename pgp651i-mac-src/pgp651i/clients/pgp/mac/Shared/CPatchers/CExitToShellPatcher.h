/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CExitToShellPatcher.h,v 1.1 1999/04/02 08:59:02 jason Exp $____________________________________________________________________________*/#pragma once #include "CPatcher.h"class CExitToShellPatcher	:	public	CPatcher {public:#if TARGET_RT_MAC_CFM	enum {kProcInfo =	kPascalStackBased};#else	enum {kProcInfo = 0};	typedef pascal void				(*ExitToShellProc)();#endif											CExitToShellPatcher();	virtual							~CExitToShellPatcher();protected:	static CExitToShellPatcher *	sExitToShellPatcher;		virtual void					NewExitToShell();	virtual void					OldExitToShell();	static pascal  void				ExitToShellCallback();};