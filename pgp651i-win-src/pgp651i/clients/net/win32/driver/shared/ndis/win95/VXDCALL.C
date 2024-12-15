/* vxdcall.c
 * Copyright (c) 1996 Vireo Software, Inc.
 *
 * Vireo Software offers VtoolsD, the professional toolkit
 * for VxD development in C and C++.
 *
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
 * This utility routine is used to back-patch the caller
 * to a CD xxxx xxxx instruction.  This hack is required 
 * to work around a bug in the MSVC 4.1 compiler which prevents
 * VxDjmp/ VxDcall from working correctly.
 *
 * See VXDCALL.H for instructions on installing and using this patch.
 *
 * (The compiler generates bad code when an ENUM is used within
 *  inline assembly code.)
 */


// The following is applied if using MSVC 4.1 (4.0 = 1000, 4.1 = 1010)

#if _MSC_VER == 1010

#define PUSHSIZE 5
#define CALLSIZE 5
#define CODESIZE (PUSHSIZE+CALLSIZE)

#define INT20H  0x20CD
#define NOPS    0x90909090

void __declspec(naked) __stdcall  __vxdcallorjmp__(unsigned long svcid)
{
    _asm {
      pushfd                        // protect code we are patching
      cli                           // from re-entry
      push  eax                         
      mov   eax, [esp+8]            // pick up return addr
      lea   eax, [eax-CODESIZE]     // back up to start of call
      mov   word ptr [eax], INT20H  // insert INT 20h instruction
      push  dword ptr [esp+12]      // pick up service id
      pop   dword ptr [eax+2]       // store in instruction stream
      mov   dword ptr [eax+6],NOPS  // filler
      mov   [esp+8], eax            // set return address
      pop   eax
      popfd
      ret   4
    }
}
#endif


