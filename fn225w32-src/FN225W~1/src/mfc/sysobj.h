/*
 # $Id: sysobj.h,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#if !defined(AFX_SYS_H__47E0B7E3_831F_11D2_80D1_0000E8D9F4F5__INCLUDED_)
#define AFX_SYS_H__47E0B7E3_831F_11D2_80D1_0000E8D9F4F5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class sysObj  
{
public:
					sysObj();
	virtual			~sysObj();

	int				procIsRunning(char *procName);

private:
	char *			IDToStr(DWORD id);
	int				checkWinNTProc(char *procName);
	int				checkWin95Proc(char *procName);
	int				checkProc(char *pathName, int pid, char *procName);

	OSVERSIONINFO	osInfo;
	BOOL			osInfoOK;
};

#endif
