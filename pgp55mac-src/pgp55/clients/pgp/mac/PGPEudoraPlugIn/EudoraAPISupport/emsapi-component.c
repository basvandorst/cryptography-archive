/*____________________________________________________________________________
____________________________________________________________________________*/
 /* ======================================================================

    Eudora Extended Message Services API SDK Macintosh 1.0b3 (July 11 1996)
    This SDK supports EMSAPI version 1
    Copyright 1995, 1996 QUALCOMM Inc.
    Send comments and questions to <emsapi-info@qualcomm.com>

    Generated: Mon Jul 15 15:41:27 PDT 1996
    Filename: emsapi-component.c
		Last Edited: July 24, 1996
    
    THIS FILE SHOULD BE COMPILED ALONG WITH THE COMPONENT
    BE SURE YOU SUPPLY A usertrans.h WHEN YOU COMPILE IT

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be changed.

*/

#pragma once

#include <stdio.h>
#include "emsapi-mac.h"
#include "usertrans.h"

#include <Errors.h>
#ifndef __powerc
    #include <A4Stuff.h>
#endif

#include "MacDebugTrapsOff.h"



enum {
    kems_plugin_versionRtn = 256,
    kems_plugin_initRtn = 257,
    kems_translator_infoRtn = 258,
    kems_can_translate_fileRtn = 259,
    kems_can_translate_bufRtn = 260,
    kems_translate_fileRtn = 261,
    kems_translate_bufRtn = 262,
    kems_plugin_finishRtn = 263
};



/* ----------------------------------------------------------------------
    Declarations of real functions that handle the standard 
    housekeeping calls of any component.
 */
pascal ComponentResult  FatOpen(ComponentInstance self);
pascal ComponentResult  FatClose(Handle storage, ComponentInstance self);
pascal ComponentResult  FatCanDo(short selector);
pascal ComponentResult  FatVersion(void);
pascal ComponentResult  main (ComponentParameters *params, Handle storage);


/* ----------------------------------------------------------------------
     These are a bunch of macros to twist the code below into
     what's needed to accept component manager calls for both
     the 68K and Power PC
 */
#ifdef __powerc
#define CallComponentFunctionWithStorageUniv(storage, params, funcName) \
   CallComponentFunctionWithStorage(storage, params, &funcName##RD)
#define CallComponentFunctionUniv(params, funcName) \
   CallComponentFunction(params, &funcName##RD)
#define INSTANTIATE_ROUTINE_DESCRIPTOR(funcName) \
	RoutineDescriptor funcName##RD = \
   BUILD_ROUTINE_DESCRIPTOR (upp##funcName##ProcInfo, funcName)
#else
#define CallComponentFunctionWithStorageUniv(storage, params, funcName) \
   CallComponentFunctionWithStorage(storage, params, \
   (ComponentFunctionUPP)funcName)
#define CallComponentFunctionUniv(params, funcName) \
   CallComponentFunction(params, (ComponentFunctionUPP)funcName)
#endif


#ifdef __powerc
/* ----------------------------------------------------------------------
  These are Power pc specific descriptions of stack frames. These must be
  in synch with the fake declarations above and the ones in emsapi.h.
 */
enum {
    uppFatOpenProcInfo = kPascalStackBased
         | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
         | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ComponentInstance)))
};

enum {
    uppFatCloseProcInfo = kPascalStackBased
         | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
         | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
         | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ComponentInstance)))
};

enum {
    uppFatCanDoProcInfo = kPascalStackBased
         | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
         | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
};

enum {
    uppFatVersionProcInfo = kPascalStackBased
         | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
};

enum {
    uppFatTargetProcInfo = kPascalStackBased
         | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
         | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
         | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ComponentInstance)))
};

enum {
    uppems_plugin_versionProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short *)))
};

enum {
    uppems_plugin_initProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short    *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short *)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(Handle *)))
};

enum {
    uppems_translator_infoProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(unsigned long *)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(Handle *)))
};

enum {
    uppems_can_translate_fileProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(emsMIMEtypeP)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(FSSpec *)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(StringPtr ***)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(long *)))
};

enum {
    uppems_can_translate_bufProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(emsMIMEtypeP)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(unsigned char **)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(StringPtr ***)))
        | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(10, SIZE_CODE(sizeof(long *)))
};

enum {
    uppems_translate_fileProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(emsMIMEtypeP)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(FSSpec *)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(StringPtr ***)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(emsProgress)))
        | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(emsMIMEtypeP *)))
        | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(FSSpec *)))
        | STACK_ROUTINE_PARAMETER(10, SIZE_CODE(sizeof(Handle *)))
        | STACK_ROUTINE_PARAMETER(11, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(12, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(13, SIZE_CODE(sizeof(long *)))
};

enum {
    uppems_translate_bufProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(emsMIMEtypeP)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(unsigned char **)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(StringPtr ***)))
        | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(emsProgress)))
        | STACK_ROUTINE_PARAMETER(10, SIZE_CODE(sizeof(emsMIMEtypeP *)))
        | STACK_ROUTINE_PARAMETER(11, SIZE_CODE(sizeof(unsigned char **)))
        | STACK_ROUTINE_PARAMETER(12, SIZE_CODE(sizeof(Handle *)))
        | STACK_ROUTINE_PARAMETER(13, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(14, SIZE_CODE(sizeof(StringPtr **)))
        | STACK_ROUTINE_PARAMETER(15, SIZE_CODE(sizeof(long *)))
};

enum {
    uppems_plugin_finishProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
};


INSTANTIATE_ROUTINE_DESCRIPTOR(FatCanDo);
INSTANTIATE_ROUTINE_DESCRIPTOR(FatOpen);
INSTANTIATE_ROUTINE_DESCRIPTOR(FatClose);
INSTANTIATE_ROUTINE_DESCRIPTOR(FatVersion);

INSTANTIATE_ROUTINE_DESCRIPTOR(ems_plugin_version);
INSTANTIATE_ROUTINE_DESCRIPTOR(ems_plugin_init);
INSTANTIATE_ROUTINE_DESCRIPTOR(ems_translator_info);
INSTANTIATE_ROUTINE_DESCRIPTOR(ems_can_translate_file);
INSTANTIATE_ROUTINE_DESCRIPTOR(ems_translate_file);
#ifdef EMS_VERSION_2
INSTANTIATE_ROUTINE_DESCRIPTOR(ems_translate_buf);
#endif //EMS_VERSION_2
INSTANTIATE_ROUTINE_DESCRIPTOR(ems_plugin_finish);

RoutineDescriptor MainRD =
	BUILD_ROUTINE_DESCRIPTOR(uppComponentRoutineProcInfo, main);
ProcInfoType __procinfo = uppComponentRoutineProcInfo;
#endif __powerc


static Component self_save;

/* ----------------------------------------------------------------------
    This is the main entry point for the component. The calls for the
    component functions below are really gigantic horrible macros that
    do weird things
    
    The return values that Eudora actually sees are the translation API return
    values (EMSR_XXXX) in the case of an API call, and Macintosh system error
    in the case of one of the four Component Manager-specific calls.
 */
pascal ComponentResult main (ComponentParameters *params, Handle storage)
{
    ComponentResult result = noErr;

#ifndef __powerc
    long oldA4;
    oldA4 = SetCurrentA4();
#endif

	
    /* Did we get a Component Manager request code (< 0)? */
    if (params->what < 0)
    {
        switch (params->what)
        {
            case kComponentOpenSelect:
                result = CallComponentFunctionUniv(params, FatOpen);
                break;
            
            case kComponentCloseSelect:
                result = CallComponentFunctionWithStorageUniv(storage,
                		params, FatClose);
                break;  
            
            case kComponentCanDoSelect:
                result = CallComponentFunctionUniv(params, FatCanDo);
                break;  
            
            case kComponentVersionSelect:
                result = CallComponentFunctionUniv(params, FatVersion);
                break;  
            
            case kComponentRegisterSelect:  // not supported
            default:                        // unknown
                result = paramErr;
                break;
        }
    }
    else
    {                                // one of ours
        long resRefNum = OpenComponentResFile(self_save);
        if(resRefNum < 0)
        {
            return(EMSR_TRANS_FAILED);
        }
        switch (params->what) {

            case kems_plugin_versionRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_plugin_version);
                break;

            case kems_plugin_initRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_plugin_init);
                break;

            case kems_translator_infoRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_translator_info);
                break;

            case kems_can_translate_fileRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_can_translate_file);
                break;

            case kems_translate_fileRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_translate_file);
                break;

#ifdef EMS_VERSION_2
            case kems_translate_bufRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_translate_buf);
                break;
#endif //EMS_VERSION_2

            case kems_plugin_finishRtn:
                result = CallComponentFunctionWithStorageUniv(
                                      storage, params, ems_plugin_finish);
                break;


            default:                        // unknown
                result = paramErr;
                break;
        }
	CloseComponentResFile(resRefNum);
    }

#ifndef __powerc
    SetA4(oldA4);
#endif

    return result;
}


/* ----------------------------------------------------------------------
    Standard component function to open a component
 */
pascal ComponentResult FatOpen(ComponentInstance self)
{
    ComponentResult     result = noErr;
    emsUserGlobals** globals;

    globals = (emsUserGlobals**)NewHandleClear(sizeof(emsUserGlobals));
    if (globals != nil) {
        
        // remember ourselves
        (*globals)->self = (Component)self;
        self_save = (Component)self;
        
        // tell the component manager that we have global storage
        SetComponentInstanceStorage(self, (Handle)globals);
    } else result = MemError() ? MemError() : memFullErr; 
    	 // NewHandleClear failed

    return result;
}


/* ----------------------------------------------------------------------
    Standard component function to close a component
 */
pascal ComponentResult FatClose(Handle storage, ComponentInstance self)
{
#pragma unused(self)
    ComponentResult     result = noErr;
    emsUserGlobals** globals = (emsUserGlobals**) storage;

    if (globals != nil) {
        DisposeHandle((Handle)globals);
        globals = nil;
    }
    return result;
}


/* ----------------------------------------------------------------------
    Standard component function to test availability of a function
    The EMS_HAS_xxx constants are supplied by the translator writer
    in the file usertrans.h
 */
pascal ComponentResult FatCanDo(short selector)
{
    switch(selector) {
        
        // component Manager request codes
        case kComponentOpenSelect:
        case kComponentCloseSelect:
        case kComponentCanDoSelect:
        case kComponentVersionSelect:
            return true;
            break;
        

        case kems_plugin_versionRtn:
            return EMS_HAS_PLUGIN_VERSION;
            break;

        case kems_plugin_initRtn:
            return EMS_HAS_PLUGIN_INIT;
            break;

        case kems_translator_infoRtn:
            return EMS_HAS_TRANSLATOR_INFO;
            break;

        case kems_can_translate_fileRtn:
            return EMS_HAS_CAN_TRANSLATE_FILE;
            break;

        case kems_can_translate_bufRtn:
            return EMS_HAS_CAN_TRANSLATE_BUF;
            break;

        case kems_translate_fileRtn:
            return EMS_HAS_TRANSLATE_FILE;
            break;

        case kems_translate_bufRtn:
            return EMS_HAS_TRANSLATE_BUF;
            break;

        case kems_plugin_finishRtn:
            return EMS_HAS_PLUGIN_FINISH;
            break;


        case kComponentRegisterSelect:  // not supported
        default:                        // unknown request
            return false;
            break;
    }
}


/* ----------------------------------------------------------------------
    Standard component function to return the components version
    The version number here is maintained in parallel with the 
    API version number.  The high 16 bits are the API version,
    and the low 16 are the component implementation version (unused).
 */
pascal ComponentResult FatVersion(void)
{   
    return EMS_VERSION << 16;
}




/* ======================================================================
   This junk is for the call backs that Eudora passes to a translator

Pete thinks this should be done with a #define instead of a function
call, but #define is ugly and it would have to be in emsapi-mac.h which
the translator writer looks at.  Having it here where they don't look
at it hides it.

Pete sez the #define is more efficient and anyone who writes code
for the Power PC will not flinch at this macro. So, it's here if
you want to use it instead of calling callProgress below.  Pete sez
the compiler even does type checking on this.  This is not obvious to
us ex-UNIX-heads that expect compilers to be OS neutral, but it may
be to you!

#if GENERATINGCFM
#define CallemsProgressProc(userRoutine, percent)	 
	CallUniversalProc((UniversalProcPtr)(userRoutine),
		uppemsProgressProcInfo,(percent))
#else
#define CallemsProgressProc(userRoutine, percent)
		 (*(userRoutine))((percent))
#endif

 */

/* ----------------------------------------------------------------------
   Call the progress function

args: glop    - The function or ProcPointer passed into Eudora
      percent - A value between 0 and 100 indicating progress

returns: 0 if translation should continue or 1 if it shoul abort

The enum describes the arguments which are on the stack
 */
enum {
    uppemsProgressProcInfo = kPascalStackBased 
        | RESULT_SIZE(SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
};

short callProgress(emsProgress glop, short percent)
{
#if GENERATINGCFM
    return(CallUniversalProc((UniversalProcPtr)(glop),
    	uppemsProgressProcInfo,(percent)));
#else
    return((*glop)(percent));
#endif
}

