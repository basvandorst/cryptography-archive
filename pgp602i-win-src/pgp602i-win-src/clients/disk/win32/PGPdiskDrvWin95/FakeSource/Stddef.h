//////////////////////////////////////////////////////////////////////////////
// Stddef.h
//
// A fake source file designed to pacify the PFL.
//////////////////////////////////////////////////////////////////////////////

// $Id: Stddef.h,v 1.1.2.2 1998/05/08 18:30:11 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Stddef_h	// [
#define Included_Stddef_h

// Don't include both vtoolscp and vtoolsc.
#ifndef __vtoolscp_h_
#include <vtoolsc.h>
#endif	// __vtoolscp_h_

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#endif	// ] Included_Stddef_h
