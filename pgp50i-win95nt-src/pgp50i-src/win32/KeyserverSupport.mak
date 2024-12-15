# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=KeyserverSupport - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to KeyserverSupport - Win32\
 Debug.
!ENDIF 

!IF "$(CFG)" != "KeyserverSupport - Win32 Release" && "$(CFG)" !=\
 "KeyserverSupport - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "KeyserverSupport.mak" CFG="KeyserverSupport - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KeyserverSupport - Win32 Release" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "KeyserverSupport - Win32 Debug" (based on\
 "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "KeyserverSupport - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "KeyserverSupport - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "keyserversupport\Release"
# PROP Intermediate_Dir "keyserversupport\Release"
# PROP Target_Dir ""
OUTDIR=.\keyserversupport\Release
INTDIR=.\keyserversupport\Release

ALL : "$(OUTDIR)\KeyserverSupport.lib"

CLEAN : 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(OUTDIR)\KeyserverSupport.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/KeyserverSupport.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\keyserversupport\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/KeyserverSupport.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/KeyserverSupport.lib" 
LIB32_OBJS= \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\KeyserverSupport.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "KeyserverSupport - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "keyserversupport\Debug"
# PROP Intermediate_Dir "keyserversupport\Debug"
# PROP Target_Dir ""
OUTDIR=.\keyserversupport\Debug
INTDIR=.\keyserversupport\Debug

ALL : "$(OUTDIR)\KeyserverSupport.lib"

CLEAN : 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(OUTDIR)\KeyserverSupport.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/KeyserverSupport.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\keyserversupport\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/KeyserverSupport.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/KeyserverSupport.lib" 
LIB32_OBJS= \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\KeyserverSupport.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

# Name "KeyserverSupport - Win32 Release"
# Name "KeyserverSupport - Win32 Debug"

!IF  "$(CFG)" == "KeyserverSupport - Win32 Release"

!ELSEIF  "$(CFG)" == "KeyserverSupport - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\KeyserverSupport\main.c
DEP_CPP_MAIN_=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\KeyserverSupport\..\include\config.h"\
	".\KeyserverSupport\..\include\pgpkeydb.h"\
	".\KeyserverSupport\..\include\pgpkeyserversupport.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
