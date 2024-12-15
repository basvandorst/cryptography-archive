//////////////////////////////////////////////////////////////////////////////
// String.h
//
// A fake source file designed to pacify the PFL.
//////////////////////////////////////////////////////////////////////////////

// $Id: String.h,v 1.1.2.2 1998/05/08 18:30:18 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_String_h	// [
#define Included_String_h

// Don't include both vtoolscp and vtoolsc.
#ifndef __vtoolscp_h_
#include <vtoolsc.h>
#endif	// __vtoolscp_h_

// Get rid of certain harmless warnings.
#pragma warning(disable: 4013 4022 4090)

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#endif	// ] Included_String_h
