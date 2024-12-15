# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=simple - Win32 Release
!MESSAGE No configuration specified.  Defaulting to simple - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "simple - Win32 Release" && "$(CFG)" != "simple - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "simple.mak" CFG="simple - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simple - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simple - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "simple - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "simple - Win32 Release"

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

ALL : "$(OUTDIR)\simple.dll"

CLEAN : 
	-@erase "$(INTDIR)\spgpABuf.obj"
	-@erase "$(INTDIR)\spgpAddK.obj"
	-@erase "$(INTDIR)\spgpAFile.obj"
	-@erase "$(INTDIR)\spgpbuf.obj"
	-@erase "$(INTDIR)\spgpcallback.obj"
	-@erase "$(INTDIR)\spgpChkR.obj"
	-@erase "$(INTDIR)\spgpdearmor.obj"
	-@erase "$(INTDIR)\spgpdecrypt.obj"
	-@erase "$(INTDIR)\spgpEBuf.obj"
	-@erase "$(INTDIR)\spgpEFile.obj"
	-@erase "$(INTDIR)\spgpExtK.obj"
	-@erase "$(INTDIR)\spgpinit.obj"
	-@erase "$(INTDIR)\spgpkcheck.obj"
	-@erase "$(INTDIR)\spgpnameid8.obj"
	-@erase "$(INTDIR)\spgpNKey.obj"
	-@erase "$(INTDIR)\spgpRBuf.obj"
	-@erase "$(INTDIR)\spgpreclist.obj"
	-@erase "$(INTDIR)\spgpRFile.obj"
	-@erase "$(INTDIR)\spgpring.obj"
	-@erase "$(INTDIR)\spgpRNG.obj"
	-@erase "$(INTDIR)\spgpSBuf.obj"
	-@erase "$(INTDIR)\spgpSFile.obj"
	-@erase "$(INTDIR)\spgpsigner.obj"
	-@erase "$(INTDIR)\spgptext.obj"
	-@erase "$(INTDIR)\spgpVBuf.obj"
	-@erase "$(INTDIR)\spgpVFile.obj"
	-@erase "$(OUTDIR)\simple.dll"
	-@erase "$(OUTDIR)\simple.exp"
	-@erase "$(OUTDIR)\simple.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /D "NDEBUG" /D "SPGPLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D DEBUG=0 /D UNFINISHED_CODE_ALLOWED=1 /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /D "NDEBUG" /D\
 "SPGPLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D DEBUG=0 /D\
 UNFINISHED_CODE_ALLOWED=1 /Fp"$(INTDIR)/simple.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\release\bn.lib ..\pgp\release\pgp.lib ..\pgp\keydb\release\keydb.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\bn\release\bn.lib ..\pgp\release\pgp.lib\
 ..\pgp\keydb\release\keydb.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/simple.pdb" /machine:I386 /out:"$(OUTDIR)/simple.dll"\
 /implib:"$(OUTDIR)/simple.lib" 
LINK32_OBJS= \
	"$(INTDIR)\spgpABuf.obj" \
	"$(INTDIR)\spgpAddK.obj" \
	"$(INTDIR)\spgpAFile.obj" \
	"$(INTDIR)\spgpbuf.obj" \
	"$(INTDIR)\spgpcallback.obj" \
	"$(INTDIR)\spgpChkR.obj" \
	"$(INTDIR)\spgpdearmor.obj" \
	"$(INTDIR)\spgpdecrypt.obj" \
	"$(INTDIR)\spgpEBuf.obj" \
	"$(INTDIR)\spgpEFile.obj" \
	"$(INTDIR)\spgpExtK.obj" \
	"$(INTDIR)\spgpinit.obj" \
	"$(INTDIR)\spgpkcheck.obj" \
	"$(INTDIR)\spgpnameid8.obj" \
	"$(INTDIR)\spgpNKey.obj" \
	"$(INTDIR)\spgpRBuf.obj" \
	"$(INTDIR)\spgpreclist.obj" \
	"$(INTDIR)\spgpRFile.obj" \
	"$(INTDIR)\spgpring.obj" \
	"$(INTDIR)\spgpRNG.obj" \
	"$(INTDIR)\spgpSBuf.obj" \
	"$(INTDIR)\spgpSFile.obj" \
	"$(INTDIR)\spgpsigner.obj" \
	"$(INTDIR)\spgptext.obj" \
	"$(INTDIR)\spgpVBuf.obj" \
	"$(INTDIR)\spgpVFile.obj"

"$(OUTDIR)\simple.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simple - Win32 Debug"

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

ALL : "$(OUTDIR)\simple.dll"

CLEAN : 
	-@erase "$(INTDIR)\spgpABuf.obj"
	-@erase "$(INTDIR)\spgpAddK.obj"
	-@erase "$(INTDIR)\spgpAFile.obj"
	-@erase "$(INTDIR)\spgpbuf.obj"
	-@erase "$(INTDIR)\spgpcallback.obj"
	-@erase "$(INTDIR)\spgpChkR.obj"
	-@erase "$(INTDIR)\spgpdearmor.obj"
	-@erase "$(INTDIR)\spgpdecrypt.obj"
	-@erase "$(INTDIR)\spgpEBuf.obj"
	-@erase "$(INTDIR)\spgpEFile.obj"
	-@erase "$(INTDIR)\spgpExtK.obj"
	-@erase "$(INTDIR)\spgpinit.obj"
	-@erase "$(INTDIR)\spgpkcheck.obj"
	-@erase "$(INTDIR)\spgpnameid8.obj"
	-@erase "$(INTDIR)\spgpNKey.obj"
	-@erase "$(INTDIR)\spgpRBuf.obj"
	-@erase "$(INTDIR)\spgpreclist.obj"
	-@erase "$(INTDIR)\spgpRFile.obj"
	-@erase "$(INTDIR)\spgpring.obj"
	-@erase "$(INTDIR)\spgpRNG.obj"
	-@erase "$(INTDIR)\spgpSBuf.obj"
	-@erase "$(INTDIR)\spgpSFile.obj"
	-@erase "$(INTDIR)\spgpsigner.obj"
	-@erase "$(INTDIR)\spgptext.obj"
	-@erase "$(INTDIR)\spgpVBuf.obj"
	-@erase "$(INTDIR)\spgpVFile.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\simple.dll"
	-@erase "$(OUTDIR)\simple.exp"
	-@erase "$(OUTDIR)\simple.ilk"
	-@erase "$(OUTDIR)\simple.lib"
	-@erase "$(OUTDIR)\simple.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /D "_DEBUG" /D "SPGPLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D DEBUG=0 /D UNFINISHED_CODE_ALLOWED=1 /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /D\
 "_DEBUG" /D "SPGPLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D DEBUG=0 /D\
 UNFINISHED_CODE_ALLOWED=1 /Fp"$(INTDIR)/simple.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\debug\bn.lib ..\pgp\debug\pgp.lib ..\pgp\keydb\debug\keydb.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\bn\debug\bn.lib ..\pgp\debug\pgp.lib\
 ..\pgp\keydb\debug\keydb.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/simple.pdb" /debug /machine:I386 /out:"$(OUTDIR)/simple.dll"\
 /implib:"$(OUTDIR)/simple.lib" 
LINK32_OBJS= \
	"$(INTDIR)\spgpABuf.obj" \
	"$(INTDIR)\spgpAddK.obj" \
	"$(INTDIR)\spgpAFile.obj" \
	"$(INTDIR)\spgpbuf.obj" \
	"$(INTDIR)\spgpcallback.obj" \
	"$(INTDIR)\spgpChkR.obj" \
	"$(INTDIR)\spgpdearmor.obj" \
	"$(INTDIR)\spgpdecrypt.obj" \
	"$(INTDIR)\spgpEBuf.obj" \
	"$(INTDIR)\spgpEFile.obj" \
	"$(INTDIR)\spgpExtK.obj" \
	"$(INTDIR)\spgpinit.obj" \
	"$(INTDIR)\spgpkcheck.obj" \
	"$(INTDIR)\spgpnameid8.obj" \
	"$(INTDIR)\spgpNKey.obj" \
	"$(INTDIR)\spgpRBuf.obj" \
	"$(INTDIR)\spgpreclist.obj" \
	"$(INTDIR)\spgpRFile.obj" \
	"$(INTDIR)\spgpring.obj" \
	"$(INTDIR)\spgpRNG.obj" \
	"$(INTDIR)\spgpSBuf.obj" \
	"$(INTDIR)\spgpSFile.obj" \
	"$(INTDIR)\spgpsigner.obj" \
	"$(INTDIR)\spgptext.obj" \
	"$(INTDIR)\spgpVBuf.obj" \
	"$(INTDIR)\spgpVFile.obj"

"$(OUTDIR)\simple.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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

# Name "simple - Win32 Release"
# Name "simple - Win32 Debug"

!IF  "$(CFG)" == "simple - Win32 Release"

!ELSEIF  "$(CFG)" == "simple - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\spgpVFile.c
DEP_CPP_SPGPV=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpVFile.obj" : $(SOURCE) $(DEP_CPP_SPGPV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpVBuf.c
DEP_CPP_SPGPVB=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpVBuf.obj" : $(SOURCE) $(DEP_CPP_SPGPVB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgptext.c
DEP_CPP_SPGPT=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgptext.obj" : $(SOURCE) $(DEP_CPP_SPGPT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpSFile.c
DEP_CPP_SPGPS=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpSFile.obj" : $(SOURCE) $(DEP_CPP_SPGPS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpSBuf.c
DEP_CPP_SPGPSB=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpSBuf.obj" : $(SOURCE) $(DEP_CPP_SPGPSB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpring.c
DEP_CPP_SPGPR=\
	"..\..\include\pgpHash.h"\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileNames.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpKeyDB.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpring.obj" : $(SOURCE) $(DEP_CPP_SPGPR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpRFile.c
DEP_CPP_SPGPRF=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpRFile.obj" : $(SOURCE) $(DEP_CPP_SPGPRF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpreclist.c
DEP_CPP_SPGPRE=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpreclist.obj" : $(SOURCE) $(DEP_CPP_SPGPRE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpRBuf.c
DEP_CPP_SPGPRB=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpRBuf.obj" : $(SOURCE) $(DEP_CPP_SPGPRB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpNKey.c
DEP_CPP_SPGPN=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpNKey.obj" : $(SOURCE) $(DEP_CPP_SPGPN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpnameid8.c
DEP_CPP_SPGPNA=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpnameid8.obj" : $(SOURCE) $(DEP_CPP_SPGPNA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpkcheck.c
DEP_CPP_SPGPK=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpkcheck.obj" : $(SOURCE) $(DEP_CPP_SPGPK) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpinit.c
DEP_CPP_SPGPI=\
	"..\..\include\pgpHash.h"\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpKeyDB.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpinit.obj" : $(SOURCE) $(DEP_CPP_SPGPI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpExtK.c
DEP_CPP_SPGPE=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpExtK.obj" : $(SOURCE) $(DEP_CPP_SPGPE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpEFile.c
DEP_CPP_SPGPEF=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpEFile.obj" : $(SOURCE) $(DEP_CPP_SPGPEF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpEBuf.c
DEP_CPP_SPGPEB=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpEBuf.obj" : $(SOURCE) $(DEP_CPP_SPGPEB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpdearmor.c
DEP_CPP_SPGPD=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpdearmor.obj" : $(SOURCE) $(DEP_CPP_SPGPD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpChkR.c
DEP_CPP_SPGPC=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpChkR.obj" : $(SOURCE) $(DEP_CPP_SPGPC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpcallback.c
DEP_CPP_SPGPCA=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpcallback.obj" : $(SOURCE) $(DEP_CPP_SPGPCA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpbuf.c
DEP_CPP_SPGPB=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpbuf.obj" : $(SOURCE) $(DEP_CPP_SPGPB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpAFile.c
DEP_CPP_SPGPA=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpAFile.obj" : $(SOURCE) $(DEP_CPP_SPGPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpAddK.c
DEP_CPP_SPGPAD=\
	"..\..\include\pgpHash.h"\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpKeyDB.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpAddK.obj" : $(SOURCE) $(DEP_CPP_SPGPAD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpABuf.c
DEP_CPP_SPGPAB=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpABuf.obj" : $(SOURCE) $(DEP_CPP_SPGPAB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpsigner.c
DEP_CPP_SPGPSI=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpsigner.obj" : $(SOURCE) $(DEP_CPP_SPGPSI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpRNG.c
DEP_CPP_SPGPRN=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpRNG.obj" : $(SOURCE) $(DEP_CPP_SPGPRN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spgpdecrypt.c
DEP_CPP_SPGPDE=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpConf.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDecPipe.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEncPipe.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpFileRef.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMemMod.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpRndSeed.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpRngRead.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigPipe.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTrstPkt.h"\
	".\../../include\pgpTrust.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\spgp.h"\
	".\spgpint.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\spgpdecrypt.obj" : $(SOURCE) $(DEP_CPP_SPGPDE) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
