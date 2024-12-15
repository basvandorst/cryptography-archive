/* vxdcall.h
 * Copyright (c) 1996 Vireo Software, Inc.
 * Vireo Software offers VtoolsD, the professional toolkit
 *                for VxD development in C and C++.
 * Vireo Software
 *      21 Half Moon Hill
 *      Acton, MA  01720
 *      voice: (508) 264-9200
 *      fax:   (508) 264-9205
 *      http:  //world.std.com/~vireo
 *      email: Vireo@vireo.com
 *
 *  This code may be freely used and distributed, as long as
 *  this copyright and permission notice is not removed or modified.
 */

/*  
 *  Please note that the argument to VxDCall or VxDJmp must be specified
 *  with leading underscores as defined in the service table.
 *
 *  You may refer to the appropriate header file, or to VTOOLSD/INCLUDE/VXDSVC.H
 *  to determine whether a leading underscore is required.  If the symbol
 *  LOCK_service_name is defined, no leading underscore is required.  If
 *  the symbol LOCK__service_name is defined, a leading underscore IS needed.
 *  For example:
 *
 *      BOOL __stdcall LOCK_Test_Debug_Installed (void) ;
 *      MEMHANDLE __cdecl LOCK__GetNulPageHandle (void) ;
 *
 *  Correct usage:
 *      VxDCall(Test_Debug_Installed);
 *      VxDCall(_GetNulPageHandle);
 *
 */

// The following is applied if using MSVC 4.1 (4.0 = 1000, 4.1 = 1010)

#if _MSC_VER == 1010

#undef VxDCall
#undef VxDJmp

extern VOID __stdcall __vxdcallorjmp__(DWORD svcid);

#define VxDCall(n) __vxdcallorjmp__(__##n);
#define VxDJmp(n) __vxdcallorjmp__(__##n + 0x8000);

#endif

