//////////////////////////////////////////////////////////////////////////////
// Stdlib.h
//
// A fake source file designed to pacify the PFL.
//////////////////////////////////////////////////////////////////////////////

// $Id: Stdlib.h,v 1.1.2.2 1998/05/08 18:30:14 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Stdlib_h	// [
#define Included_Stdlib_h

// Don't include both vtoolscp and vtoolsc.
#ifndef __vtoolscp_h_
#include <vtoolsc.h>
#endif	// __vtoolscp_h_

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#endif	// ] Included_Stdlib_h
