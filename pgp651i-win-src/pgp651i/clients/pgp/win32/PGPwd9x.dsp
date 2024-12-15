# Microsoft Developer Studio Project File - Name="PGPwd9x" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPwd9x - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPwd9x.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPwd9x.mak" CFG="PGPwd9x - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPwd9x - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPwd9x - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPwd9x - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "PGPwd9x\release"
# PROP Intermediate_Dir "PGPwd9x\release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PGPwdel9X_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /Od /I "..\.." /I "$(98DDKROOT)\inc\win98" /I "..\shared" /I "include" /D "BLD_COFF" /D "IS_32" /D "_X86_" /D "WIN40COMPAT" /Gs /c
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
# ADD LINK32 PGPwd9x/release/PGPwd9xd.obj vxdwraps.clb /nologo /pdb:none /machine:IX86 /nodefaultlib /def:"PGPwd9x\PGPwd9x.def" /out:"PGPwd9x\release/PGPwd9x.vxd" /libpath:"$(98DDKROOT)\lib\i386\free" /vxd
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Assembling PGPwd9xd.asm
PreLink_Cmds=$(98DDKROOT)\bin\win98\ml /coff /DBLD_COFF /DIS_32 /W2 /c /Cx  /DMASM6 /DWIN40COMPAT /I$(98DDKROOT)\inc\win98 /I$(98DDKROOT)\src\base\vxdwraps  /Fo./PGPwd9x/Release/ PGPwd9x\PGPwd9xd.asm
PostBuild_Desc=Compiling and adding VERSIONINFO to VXD
PostBuild_Cmds=copy PGPwd9x\release\PGPwd9x.vxd PGPwd9x\release\PGPwd.vxd 	$(98DDKROOT)\bin\win98\bin16\rc -r -i$(98DDKROOT)\inc\win98\inc16  -fo.\PGPwd9x\Release\PGPwd.res PGPwd9x\PGPwd9x.rc	$(98DDKROOT)\bin\win98\adrc2vxd   PGPwd9x\release\PGPwd.vxd PGPwd9x\release\PGPwd.res	copy PGPwd9x\release\PGPwd.vxd  PGPwd9x\release\PGPwd9x.vxd	erase PGPwd9x\release\PGPwd.vxd
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPwd9x - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PGPwd9x\debug"
# PROP Intermediate_Dir "PGPwd9x\debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PGPwd9x_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /ZI /Od /I "..\.." /I "$(98DDKROOT)\inc\win98" /I "..\shared" /I "include" /D "BLD_COFF" /D "IS_32" /D "_X86_" /D "WIN40COMPAT" /D "_DEBUG" /D "DEBUG" /D "PGP_DEBUG" /Zl /Gs /c
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
# ADD LINK32 PGPwd9x/debug/PGPwd9xd.obj vxdwraps.clb /nologo /dll /pdb:none /debug /machine:I386 /nodefaultlib /def:"PGPwd9x\PGPwd9x.def" /out:"PGPwd9x/debug/PGPwd9x.vxd" /libpath:"$(98DDKROOT)\lib\i386\free" /vxd
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Assembling PGPwd9xd.asm
PreLink_Cmds=$(98DDKROOT)\bin\win98\ml /coff /DBLD_COFF /DIS_32 /W2 /c /Cx  /DMASM6 /DWIN40COMPAT /I$(98DDKROOT)\inc\win98 /I$(98DDKROOT)\src\base\vxdwraps  /Fo./PGPwd9x/debug/ PGPwd9x\PGPwd9xd.asm
PostBuild_Desc=Compiling and adding VERSIONINFO to VXD
PostBuild_Cmds=copy PGPwd9x\debug\PGPwd9x.vxd PGPwd9x\debug\PGPwd.vxd 	$(98DDKROOT)\bin\win98\bin16\rc -r -dPGP_DEBUG=1 -i$(98DDKROOT)\inc\win98\inc16  -fo.\PGPwd9x\debug\PGPwd.res PGPwd9x\PGPwd9x.rc	$(98DDKROOT)\bin\win98\adrc2vxd   PGPwd9x\debug\PGPwd.vxd PGPwd9x\debug\PGPwd.res	copy PGPwd9x\debug\PGPwd.vxd PGPwd9x\debug\PGPwd9x.vxd 	erase PGPwd9x\debug\PGPwd.vxd
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPwd9x - Win32 Release"
# Name "PGPwd9x - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PGPwd9x\PGPwd9x.c
DEP_CPP_PGPWD=\
	".\include\PGPwd9x.h"\
	"f:\98DDK\inc\win98\basedef.h"\
	"f:\98DDK\inc\win98\debug.h"\
	"f:\98DDK\inc\win98\ifs.h"\
	"f:\98DDK\inc\win98\regdef.h"\
	"f:\98DDK\inc\win98\vmm.h"\
	"f:\98DDK\inc\win98\vwin32.h"\
	"f:\98DDK\inc\win98\vxdwraps.h"\
	
# End Source File
# Begin Source File

SOURCE=.\PGPwd9x\PGPwd9xd.asm
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
