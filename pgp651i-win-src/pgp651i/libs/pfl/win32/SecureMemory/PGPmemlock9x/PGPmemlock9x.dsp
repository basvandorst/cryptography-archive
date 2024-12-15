# Microsoft Developer Studio Project File - Name="PGPmemlock9x" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPmemlock9x - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlock9x.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPmemlock9x.mak" CFG="PGPmemlock9x - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPmemlock9x - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPmemlock9x - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPmemlock9x - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PGPMEMLOCK9X_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "..\.." /I "$(98DDKROOT)\inc\win98" /D "BLD_COFF" /D "IS_32" /D "_X86_" /D "WIN40COMPAT" /Gs /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ./release/pgpMLdevdcl.obj vxdwraps.clb /nologo /machine:IX86 /nodefaultlib /def:"pgpMemLock.def" /out:"./release/PGPmemlock.vxd" /libpath:"$(98DDKROOT)\lib\i386\free" /vxd
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Assembling pgpMLdevdcl.asm
PreLink_Cmds=$(98DDKROOT)\bin\win98\ml /coff /DBLD_COFF /DIS_32 /W2 /c /Cx /DMASM6 /DWIN40COMPAT /I$(98DDKROOT)\inc\win98 /Fo./Release/ pgpMLdevdcl.asm
PostBuild_Desc=Compiling and adding VERSIONINFO to VXD
PostBuild_Cmds=copy .\release\PGPmemlock.vxd .\release\PGPml.vxd	$(98DDKROOT)\bin\win98\bin16\rc -r -i$(98DDKROOT)\inc\win98\inc16 -i$(VCINCLUDE) -fo.\Release\PGPml.res PGPml.rc	$(98DDKROOT)\bin\win98\adrc2vxd  .\release\PGPml.vxd .\release\PGPml.res	copy .\release\PGPml.vxd .\release\PGPmemlock.vxd	erase .\release\PGPml.vxd
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPmemlock9x - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PGPMEMLOCK9X_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /Zi /Od /I "..\.." /I "$(98DDKROOT)\inc\win98" /D "BLD_COFF" /D "IS_32" /D "_X86_" /D "WIN40COMPAT" /D "_DEBUG" /D "DEBUG" /D "PGP_DEBUG" /Zl /Gs /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ./debug/pgpMLdevdcl.obj vxdwraps.clb /nologo /dll /debug /machine:I386 /nodefaultlib /def:"pgpMemLock.def" /out:"./debug/PGPmemlock.vxd" /libpath:"$(98DDKROOT)\lib\i386\free" /vxd
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Assembling pgpMLdevdcl.asm
PreLink_Cmds=$(98DDKROOT)\bin\win98\ml /coff /DBLD_COFF /DIS_32 /W2 /c /Cx /DMASM6 /DWIN40COMPAT /I$(98DDKROOT)\inc\win98 /Fo./debug/ pgpMLdevdcl.asm
PostBuild_Desc=Compiling and adding VERSIONINFO to VXD
PostBuild_Cmds=copy .\debug\PGPmemlock.vxd .\debug\PGPml.vxd	$(98DDKROOT)\bin\win98\bin16\rc -r -dPGP_DEBUG=1 -i$(98DDKROOT)\inc\win98\inc16 -i$(VCINCLUDE) -fo.\debug\PGPml.res PGPml.rc	$(98DDKROOT)\bin\win98\adrc2vxd  .\debug\PGPml.vxd .\debug\PGPml.res	copy .\debug\PGPml.vxd .\debug\PGPmemlock.vxd	erase .\debug\PGPml.vxd
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPmemlock9x - Win32 Release"
# Name "PGPmemlock9x - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\pgpMemLock.c
DEP_CPP_PGPME=\
	"..\..\..\..\..\..\DDK\98DDK1098\inc\win98\basedef.h"\
	"..\..\..\..\..\..\DDK\98DDK1098\inc\win98\debug.h"\
	"..\..\..\..\..\..\DDK\98DDK1098\inc\win98\vmm.h"\
	"..\..\..\..\..\..\DDK\98DDK1098\inc\win98\vtd.h"\
	"..\..\..\..\..\..\DDK\98DDK1098\inc\win98\vwin32.h"\
	"..\..\..\..\..\..\DDK\98DDK1098\inc\win98\vxdwraps.h"\
	"..\..\pgpMemLockWin32.h"\
	
# End Source File
# Begin Source File

SOURCE=.\pgpMLdevdcl.asm
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\pgpVer.h
# End Source File
# End Group
# End Target
# End Project
