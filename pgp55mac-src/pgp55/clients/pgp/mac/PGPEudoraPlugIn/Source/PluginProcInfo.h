/*____________________________________________________________________________
	PluginProcInfo.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PluginProcInfo.h,v 1.1 1997/07/25 23:47:57 lloyd Exp $
____________________________________________________________________________*/


// PlugInInit()
enum {
    uppPlugInInitProcInfo = kCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(PluginError)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short    *)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(StringHandle *)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short *)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(Handle *)))
};

// PlugInFinish()
enum {
    uppPluginFinishProcInfo = kCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(PluginError)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
};


// GetTranslatorInfo()
enum {
    uppGetTranslatorInfoProcInfo = kCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(PluginError)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(unsigned long *)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(StringHandle *)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(Handle *)))
};


// CanTranslateFile()
enum {
    uppCanTranslateFileProcInfo = kCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(PluginError)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(emsMIMEtype **)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(const FSSpec *)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(StringHandle **)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(long *)))
        | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(StringHandle *)))
        | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(long *)))
};


// TranslateFile()
enum {
    uppTranslateFileProcInfo = kCStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(PluginError)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
        | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
        | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(emsMIMEtype **)))
        | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(const FSSpec *)))
        | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(StringHandle **)))
        | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(emsProgress)))
        | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(emsMIMEtype ***)))
        | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(const FSSpec *)))
        | STACK_ROUTINE_PARAMETER(10, SIZE_CODE(sizeof(Handle *)))
        | STACK_ROUTINE_PARAMETER(11, SIZE_CODE(sizeof(StringHandle *)))
        | STACK_ROUTINE_PARAMETER(12, SIZE_CODE(sizeof(StringHandle *)))
        | STACK_ROUTINE_PARAMETER(13, SIZE_CODE(sizeof(long *)))
};


