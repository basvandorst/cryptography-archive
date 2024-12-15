# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=clientlib - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to clientlib - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "clientlib - Win32 Release" && "$(CFG)" !=\
 "clientlib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "clientlib.mak" CFG="clientlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "clientlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "clientlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "clientlib - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "clientlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\clientlib.lib"

CLEAN : 
	-@erase "$(INTDIR)\pgpClientContext.obj"
	-@erase "$(INTDIR)\pgpClientEncode.obj"
	-@erase "$(INTDIR)\pgpClientKeyDB.obj"
	-@erase "$(INTDIR)\pgpClientMem.obj"
	-@erase "$(INTDIR)\pgpOptionList.obj"
	-@erase "$(OUTDIR)\clientlib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\utilities" /I "..\..\priv\encrypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\win32" /D "NDEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\pub\include" /I "..\..\priv\include"\
 /I "..\..\priv\include\opaque" /I "..\..\priv\utilities" /I\
 "..\..\priv\encrypt" /I "..\..\priv\crypto\pipe\file" /I\
 "..\..\priv\crypto\pipe\text" /I "..\..\priv\keys\keydb" /I\
 "..\..\priv\keys\keys" /I "..\..\win32" /I "..\..\..\pfl\common" /I\
 "..\..\..\pfl\win32" /D "NDEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D\
 "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)/clientlib.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/clientlib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/clientlib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\pgpClientContext.obj" \
	"$(INTDIR)\pgpClientEncode.obj" \
	"$(INTDIR)\pgpClientKeyDB.obj" \
	"$(INTDIR)\pgpClientMem.obj" \
	"$(INTDIR)\pgpOptionList.obj"

"$(OUTDIR)\clientlib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "clientlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\clientlib.lib"

CLEAN : 
	-@erase "$(INTDIR)\pgpClientContext.obj"
	-@erase "$(INTDIR)\pgpClientEncode.obj"
	-@erase "$(INTDIR)\pgpClientKeyDB.obj"
	-@erase "$(INTDIR)\pgpClientMem.obj"
	-@erase "$(INTDIR)\pgpOptionList.obj"
	-@erase "$(OUTDIR)\clientlib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\utilities" /I "..\..\priv\encrypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\win32" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /I "..\..\pub\include" /I\
 "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\utilities"\
 /I "..\..\priv\encrypt" /I "..\..\priv\crypto\pipe\file" /I\
 "..\..\priv\crypto\pipe\text" /I "..\..\priv\keys\keydb" /I\
 "..\..\priv\keys\keys" /I "..\..\win32" /I "..\..\..\pfl\common" /I\
 "..\..\..\pfl\win32" /D "_DEBUG" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D\
 "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)/clientlib.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/clientlib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/clientlib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\pgpClientContext.obj" \
	"$(INTDIR)\pgpClientEncode.obj" \
	"$(INTDIR)\pgpClientKeyDB.obj" \
	"$(INTDIR)\pgpClientMem.obj" \
	"$(INTDIR)\pgpOptionList.obj"

"$(OUTDIR)\clientlib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "clientlib - Win32 Release"
# Name "clientlib - Win32 Debug"

!IF  "$(CFG)" == "clientlib - Win32 Release"

!ELSEIF  "$(CFG)" == "clientlib - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\pgpClientContext.c

!IF  "$(CFG)" == "clientlib - Win32 Release"

DEP_CPP_PGPCL=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\pub\include\pgpPubTypes.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\..\win32\pgpConfig.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpClientContext.obj" : $(SOURCE) $(DEP_CPP_PGPCL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "clientlib - Win32 Debug"

DEP_CPP_PGPCL=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpClientContext.obj" : $(SOURCE) $(DEP_CPP_PGPCL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpClientEncode.c
DEP_CPP_PGPCLI=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpLeaks.h"\
	"..\..\..\pfl\common\pgpMem.h"\
	"..\..\..\pfl\common\pgpPFLErrors.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\crypto\pipe\file\pgpFileMod.h"\
	"..\..\priv\crypto\pipe\text\pgpTextFilt.h"\
	"..\..\priv\encrypt\pgpEncodePriv.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\include\pgpUsuals.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\priv\utilities\pgpOptionList.h"\
	"..\..\pub\include\pgpCipherFeedback.h"\
	"..\..\pub\include\pgpEncode.h"\
	"..\..\pub\include\pgpErrors.h"\
	"..\..\pub\include\pgpHash.h"\
	"..\..\pub\include\pgpKeys.h"\
	"..\..\pub\include\pgpPubTypes.h"\
	"..\..\pub\include\pgpSymmetricCipher.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\..\win32\pgpConfig.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpClientEncode.obj" : $(SOURCE) $(DEP_CPP_PGPCLI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\utilities\pgpOptionList.c
DEP_CPP_PGPOP=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpLeaks.h"\
	"..\..\..\pfl\common\pgpMem.h"\
	"..\..\..\pfl\common\pgpPFLErrors.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\crypto\pipe\file\pgpFileMod.h"\
	"..\..\priv\crypto\pipe\text\pgpTextFilt.h"\
	"..\..\priv\encrypt\pgpEncodePriv.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\include\pgpUsuals.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\priv\utilities\pgpOptionList.h"\
	"..\..\pub\include\pgpCipherFeedback.h"\
	"..\..\pub\include\pgpEncode.h"\
	"..\..\pub\include\pgpErrors.h"\
	"..\..\pub\include\pgpHash.h"\
	"..\..\pub\include\pgpKeys.h"\
	"..\..\pub\include\pgpPubTypes.h"\
	"..\..\pub\include\pgpSymmetricCipher.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\..\win32\pgpConfig.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpOptionList.obj" : $(SOURCE) $(DEP_CPP_PGPOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpClientMem.c
DEP_CPP_PGPCLIE=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpLeaks.h"\
	"..\..\..\pfl\common\pgpMem.h"\
	"..\..\..\pfl\common\pgpPFLErrors.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\crypto\pipe\file\pgpFileMod.h"\
	"..\..\priv\crypto\pipe\text\pgpTextFilt.h"\
	"..\..\priv\encrypt\pgpEncodePriv.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\include\pgpUsuals.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\priv\utilities\pgpOptionList.h"\
	"..\..\pub\include\pgpCipherFeedback.h"\
	"..\..\pub\include\pgpEncode.h"\
	"..\..\pub\include\pgpErrors.h"\
	"..\..\pub\include\pgpHash.h"\
	"..\..\pub\include\pgpKeys.h"\
	"..\..\pub\include\pgpPubTypes.h"\
	"..\..\pub\include\pgpSymmetricCipher.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\..\win32\pgpConfig.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpClientMem.obj" : $(SOURCE) $(DEP_CPP_PGPCLIE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpClientKeyDB.c

!IF  "$(CFG)" == "clientlib - Win32 Release"

DEP_CPP_PGPCLIEN=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpLeaks.h"\
	"..\..\..\pfl\common\pgpMem.h"\
	"..\..\..\pfl\common\pgpPFLErrors.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\keys\keydb\pgpKeyDB.h"\
	"..\..\priv\keys\keys\pgpTrust.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\priv\utilities\pgpOptionList.h"\
	"..\..\pub\include\pgpCipherFeedback.h"\
	"..\..\pub\include\pgpEncode.h"\
	"..\..\pub\include\pgpErrors.h"\
	"..\..\pub\include\pgpHash.h"\
	"..\..\pub\include\pgpKeys.h"\
	"..\..\pub\include\pgpPubTypes.h"\
	"..\..\pub\include\pgpSymmetricCipher.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\..\win32\pgpConfig.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpClientKeyDB.obj" : $(SOURCE) $(DEP_CPP_PGPCLIEN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "clientlib - Win32 Debug"

DEP_CPP_PGPCLIEN=\
	"..\..\..\pfl\common\pgpBase.h"\
	"..\..\..\pfl\common\pgpDebug.h"\
	"..\..\..\pfl\common\pgpLeaks.h"\
	"..\..\..\pfl\common\pgpMem.h"\
	"..\..\..\pfl\common\pgpPFLErrors.h"\
	"..\..\..\pfl\common\pgpTypes.h"\
	"..\..\..\pfl\win32\pgpPFLConfig.h"\
	"..\..\priv\include\opaque\pgpOpaqueStructs.h"\
	"..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h"\
	"..\..\priv\include\pgpUsuals.h"\
	"..\..\priv\keys\keydb\pgpKeyDB.h"\
	"..\..\priv\keys\keys\pgpRngPub.h"\
	"..\..\priv\keys\keys\pgpTrust.h"\
	"..\..\priv\utilities\pgpContext.h"\
	"..\..\priv\utilities\pgpOptionList.h"\
	"..\..\pub\include\pgpCipherFeedback.h"\
	"..\..\pub\include\pgpEncode.h"\
	"..\..\pub\include\pgpErrors.h"\
	"..\..\pub\include\pgpHash.h"\
	"..\..\pub\include\pgpKeys.h"\
	"..\..\pub\include\pgpPubTypes.h"\
	"..\..\pub\include\pgpSymmetricCipher.h"\
	"..\..\pub\include\pgpUtilities.h"\
	"..\..\win32\pgpConfig.h"\
	"..\include\opaque\pgpBigNumOpaqueStructs.h"\
	"..\include\opaque\pgpKeyDBOpaqueStructs.h"\
	"..\include\opaque\pgpRingOpaqueStructs.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\pgpClientKeyDB.obj" : $(SOURCE) $(DEP_CPP_PGPCLIEN) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
