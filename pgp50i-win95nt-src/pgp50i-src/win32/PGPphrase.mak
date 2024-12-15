# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=PGPphrase - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to PGPphrase - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PGPphrase - Win32 Release" && "$(CFG)" !=\
 "PGPphrase - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPphrase.mak" CFG="PGPphrase - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPphrase - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PGPphrase - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "PGPphrase - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "PGPphrase - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPphrase\Release"
# PROP Intermediate_Dir "PGPphrase\Release"
# PROP Target_Dir ""
OUTDIR=.\PGPphrase\Release
INTDIR=.\PGPphrase\Release

ALL : "$(OUTDIR)\PGPphrase.lib"

CLEAN : 
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(OUTDIR)\PGPphrase.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPphrase.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPphrase\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPphrase.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/PGPphrase.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PGPphrase.obj"

"$(OUTDIR)\PGPphrase.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPphrase - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPphrase\Debug"
# PROP Intermediate_Dir "PGPphrase\Debug"
# PROP Target_Dir ""
OUTDIR=.\PGPphrase\Debug
INTDIR=.\PGPphrase\Debug

ALL : "$(OUTDIR)\PGPphrase.lib"

CLEAN : 
	-@erase "$(INTDIR)\PGPphrase.obj"
	-@erase "$(OUTDIR)\PGPphrase.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPphrase.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\PGPphrase\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPphrase.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/PGPphrase.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PGPphrase.obj"

"$(OUTDIR)\PGPphrase.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

# Name "PGPphrase - Win32 Release"
# Name "PGPphrase - Win32 Debug"

!IF  "$(CFG)" == "PGPphrase - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPphrase - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\PGPphrase\PGPphrase.c

!IF  "$(CFG)" == "PGPphrase - Win32 Release"

DEP_CPP_PGPPH=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPphrase\..\include\config.h"\
	".\PGPphrase\..\include\pgpcomdlg.h"\
	".\PGPphrase\..\include\pgpkeydb.h"\
	".\PGPphrase\..\include\pgpmem.h"\
	".\PGPphrase\..\include\PGPphrase.h"\
	".\PGPphrase\PGPphraseVer.h"\
	".\PGPphrase\PGPphrasex.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPphrase - Win32 Debug"

DEP_CPP_PGPPH=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPphrase\..\include\config.h"\
	".\PGPphrase\..\include\pgpcomdlg.h"\
	".\PGPphrase\..\include\pgpkeydb.h"\
	".\PGPphrase\..\include\pgpmem.h"\
	".\PGPphrase\..\include\PGPphrase.h"\
	".\PGPphrase\PGPphraseVer.h"\
	".\PGPphrase\PGPphrasex.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\PGPphrase.obj" : $(SOURCE) $(DEP_CPP_PGPPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPphrase\PGPphraseVer.h

!IF  "$(CFG)" == "PGPphrase - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPphrase - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
