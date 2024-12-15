# Microsoft Developer Studio Project File - Name="pgpmac" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPMAC - WIN32 WIN95 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pgpmac.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pgpmac.mak" CFG="PGPMAC - WIN32 WIN95 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pgpmac - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpmac - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpmac - Win32 Win95 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgpmac - Win32 Win95 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gz /MT /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "$(BASEDIR)\inc\ddk" /I "$(BASEDIR)\src\network\inc" /I "..\..\shared\ipsec" /I "..\shared" /I "..\..\shared" /I ".\shared" /I ".\shared\ndis" /I "..\..\shared\minisdk\include" /I "..\..\shared\compress" /I ".\ike" /I ".\minisdk" /D WINNT=1 /D _X86_=1 /D _i386=1 /D "STD_CALL" /D "CONDITION_HANDLING" /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D _DLL=1 /D "NDEBUG" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D "PGP_WIN32" /D "HIFN_LITTLE_ENDIAN" /FR /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ndis.lib int64.lib hal.lib ntoskrnl.lib ntdll.lib /nologo /entry:"DriverEntry@8" /dll /machine:I386 /nodefaultlib /out:"Release/pgpnet.sys" /libpath:"$(BASEDIR)\libfre\i386" /libpath:"$(BASEDIR)\lib\i386\free" /subsystem:native
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgpmac__"
# PROP BASE Intermediate_Dir "pgpmac__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /GX /Zi /Od /I "$(BASEDIR)\inc" /I "$(BASEDIR)\inc\ddk" /I "$(BASEDIR)\src\network\inc" /I "..\..\shared\ipsec" /I "..\shared" /I "..\..\shared" /I ".\shared" /I ".\shared\ndis" /I "..\..\shared\minisdk\include" /I "..\..\shared\compress" /I ".\ike" /I ".\minisdk" /D NTDEBUGTYPE=windbg /D "_DEBUG" /D DBG=1 /D WINNT=1 /D _X86_=1 /D _i386=1 /D "STD_CALL" /D "CONDITION_HANDLING" /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D _DLL=1 /D "NDEBUG" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D "PGP_WIN32" /D "HIFN_LITTLE_ENDIAN" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ndis.lib int64.lib hal.lib ntoskrnl.lib /nologo /entry:"DriverEntry@8" /dll /incremental:no /debug /machine:I386 /nodefaultlib /out:"Debug/pgpnet.sys" /pdbtype:sept /libpath:"$(BASEDIR)\libchk\i386" /libpath:"$(BASEDIR)\lib\i386\checked" /subsystem:native /subsystem:native
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgpmac__"
# PROP BASE Intermediate_Dir "pgpmac__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win95Debug"
# PROP Intermediate_Dir "Win95Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /MTd /W3 /Gm /GX /Zi /Od /I "$(BASEDIR)\src\network\inc" /I "$(BASEDIR)\inc" /I "..\..\shared\ipsec" /I "..\shared" /I "..\..\shared" /I "..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\libs\pfl\common\util" /I ".\shared" /I ".\shared\ndis" /I "..\..\shared\minisdk\include" /I ".\ike" /I ".\minisdk" /D "KERNEL_MESSAGE_EMULATION" /D "_DEBUG" /D DBG=1 /D "PGP_WIN32" /D WINNT=1 /D _X86_=1 /D _i386=1 /D "STD_CALL" /D "CONDITION_HANDLING" /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X=100 /D WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D _DLL=1 /D "NDEBUG" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /FR /FD /c
# ADD CPP /nologo /Gz /Zp1 /MTd /W3 /Gm /GX /Zi /Oi /I "$(98DDKROOT)\inc\win98" /I "$(98DDKROOT)\src\bpc\inc" /I "$(BASEDIR)\inc\ddk" /I "$(BASEDIR)\inc" /I "..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\libs\pfl\common\util" /I "..\..\shared\ipsec" /I "..\shared" /I "..\..\shared" /I ".\shared" /I ".\shared\ndis" /I "..\..\shared\minisdk\include" /I "..\..\shared\compress" /I ".\ike" /I ".\minisdk" /D "WIN40COMPAT" /D "CHICAGO" /D "DBG" /D "DEBUG" /D "PLUG_AND_PLAY" /D CUSTOM_VXD=1 /D DEVICE=PGPMAC /D "i386" /D "NDIS_KERNEL" /D "_X86_" /D NDIS2=1 /D "IS_32" /D "PGP_WIN32" /D "HIFN_LITTLE_ENDIAN" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ndis.lib int64.lib hal.lib ntoskrnl.lib ndis.lib int64.lib hal.lib ntoskrnl.lib ntdll.lib /nologo /entry:"DriverEntry@8" /dll /incremental:no /debug /machine:I386 /nodefaultlib /out:"Debug/pgpmac.sys" /pdbtype:sept /libpath:"c:\ddk\lib\i386\checked" /libpath:"$(BASEDIR)\lib\i386\checked" /subsystem:native /subsystem:native
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 libndis.clb vxdwraps.clb wrappers.clb /nologo /incremental:no /debug /machine:I386 /nodefaultlib /out:"Win95Debug/pgpnet.vxd" /libpath:"$(98DDKROOT)\src\bpc\lib" /libpath:"$(98DDKROOT)\LIB\i386\free" /libpath:"$(98DDKROOT)\src\net\ndis3\lib\debug" /libpath:".\shared\ndis\win95" /vxd
# SUBTRACT LINK32 /pdb:none /map

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgpmac_0"
# PROP BASE Intermediate_Dir "pgpmac_0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win95Release"
# PROP Intermediate_Dir "Win95Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /MTd /W3 /Gm /GX /Zi /O2 /I "c:\95ddk\inc32" /I "c:\95ddk\net\inc" /I "$(BASEDIR)\src\network\inc" /I "$(BASEDIR)\inc" /I "..\..\shared\ipsec" /I "..\shared" /I "..\..\shared" /I "..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\libs\pfl\common\util" /I ".\shared" /I ".\shared\ndis" /I "..\..\shared\minisdk\include" /I ".\ike" /I ".\minisdk" /I "$(95DDKROOT)\inc32" /I "$(95DDKROOT)\net\inc" /I "..\..\shared\compress" /D "DBG" /D "DEBUG" /D "PLUG_AND_PLAY" /D CUSTOM_VXD=1 /D DEVICE=PGPMAC /D "i386" /D "NDIS_KERNEL" /D "_X86_" /D "CHICAGO" /D NDIS2=1 /D "IS_32" /D "PGP_WIN32" /D "HIFN_LITTLE_ENDIAN" /FR /FD /c
# ADD CPP /nologo /Gz /Zp1 /MT /W3 /GX /O2 /I "$(98DDKROOT)\inc\win98" /I "$(98DDKROOT)\src\bpc\inc" /I "$(BASEDIR)\inc\ddk" /I "$(BASEDIR)\inc" /I "..\..\..\..\libs\pfl\common\prefs" /I "..\..\..\..\libs\pfl\common\util" /I "..\..\shared\ipsec" /I "..\shared" /I "..\..\shared" /I ".\shared" /I ".\shared\ndis" /I "..\..\shared\minisdk\include" /I "..\..\shared\compress" /I ".\ike" /I ".\minisdk" /D "WIN40COMPAT" /D "CHICAGO" /D "PLUG_AND_PLAY" /D CUSTOM_VXD=1 /D DEVICE=PGPMAC /D "i386" /D "NDIS_KERNEL" /D "_X86_" /D NDIS2=1 /D "IS_32" /D "PGP_WIN32" /D "HIFN_LITTLE_ENDIAN" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libndis.clb .\shared\ndis\win95\pgpndis.obj libndis.clb vxdwraps.clb wrappers.clb /nologo /entry:"DriverEntry@8" /dll /incremental:no /debug /machine:I386 /nodefaultlib /out:"Win95 Release/pgpmac.vxd" /pdbtype:sept /libpath:"C:\95ddk\NET\NDIS3\LIB\DEBUG" /libpath:"$(95DDKROOT)\NET\NDIS3\LIB\RETAIL" /libpath:"$(95DDKROOT)\LIB" /libpath:".\shared\ndis\win95" /subsystem:native /subsystem:native /vxd
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 libndis.clb vxdwraps.clb wrappers.clb /nologo /incremental:no /machine:I386 /nodefaultlib /out:"Win95Release/pgpnet.vxd" /libpath:"$(98DDKROOT)\src\bpc\lib" /libpath:"$(98DDKROOT)\LIB\i386\free" /libpath:"$(98DDKROOT)\src\net\ndis3\lib\retail" /libpath:".\shared\ndis\win95" /vxd /vxd
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "pgpmac - Win32 Release"
# Name "pgpmac - Win32 Debug"
# Name "pgpmac - Win32 Win95 Debug"
# Name "pgpmac - Win32 Win95 Release"
# Begin Group "Source files"

# PROP Default_Filter "c"
# Begin Group "ipsec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPheader.c
# End Source File
# Begin Source File

SOURCE=.\ipsec\pgpIPsec.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecAH.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecBuffer.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecComp.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecContext.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecESP.c
# End Source File
# End Group
# Begin Group "minisdk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCAST5.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCBC.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCFB.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpDES3.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\pgpEndianConversion.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpHash.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpHMAC.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpIDEA.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpMD5.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMem.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMemoryMgr.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMemoryMgrWin32.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMutex.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpRIPEMD160.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpRMWOLock.c
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpSemaphore.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpSHA.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpSymmetricCipher.c
# End Source File
# End Group
# Begin Group "pm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMCommands.c
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMConfig.c
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMEvents.c
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMHost.c
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMSA.c
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPolicyManager.c
# End Source File
# End Group
# Begin Group "ndis"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shared\ndis\cfgmgr.c
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\PGPMAC.DEF

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\ndis\Pgpndis.asm

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

# Begin Custom Build
OutDir=.\Win95Debug
InputPath=.\shared\ndis\Pgpndis.asm

"$(OutDir)\pgpndis.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set      include=$(98DDKROOT)\inc\win98;$(98DDKROOT)\net\inc; 
	$(98DDKROOT)\bin\win98\ml.exe -coff -DBLD_COFF -DWIN40COMPAT -DIS_32 -W2 -Zd -c -Cx -DMASM6       -DVMMSYS -Zm -DNDIS_WIN -c  -DNDIS_STDCALL -DDEBUG -DDBG=1 -DDEBLEVEL=1       -DNDIS2=1 -DCHICAGO       -Fowin95debug\pgpndis.obj .\shared\ndis\pgpndis.asm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

# Begin Custom Build
OutDir=.\Win95Release
InputPath=.\shared\ndis\Pgpndis.asm

"$(OutDir)\pgpndis.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set      include=$(98DDKROOT)\inc\win98;$(98DDKROOT)\net\inc; 
	$(98DDKROOT)\bin\win98\ml.exe -DWIN40COMPAT -DIS_32 -W2 -Zd -c -Cx -DMASM6 -DVMMSYS -Zm       -DNDIS_WIN -c  -DNDIS_STDCALL -DNDIS2=1 -DCHICAGO       -Fowin95release\pgpndis.obj .\shared\ndis\pgpndis.asm 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\ndis\pgpvm.c

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\ndis\poclink.c
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpnioctl.c

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpnmac.c
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpnmain.c
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpnproto.c
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpnsupport.c
# End Source File
# Begin Source File

SOURCE=.\Shared\Ndis\vxdevent.c

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

# ADD CPP /Gd

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

# ADD CPP /Gd

!ENDIF 

# End Source File
# End Group
# Begin Group "compress"

# PROP Default_Filter ""
# Begin Group "deflate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\shared\compress\deflate\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\infblock.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\infutil.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\deflate\zutil.c
# End Source File
# End Group
# Begin Group "LZS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\shared\compress\LZS\LZSC.C
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\shared\compress\deflate\pgpCompDeflate.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\LZS\pgpCompLZS.c
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\pgpCompress.c
# End Source File
# End Group
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\shared\ndis\cfgdef.h
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\ipdef.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCAST5.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCASTBox5.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCBCPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpCFBPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\pgpCompDeflate.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\pgpCompLZS.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\pgpCompress.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\compress\pgpCompressPriv.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpConfig.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpDES3.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libs\pfl\common\util\pgpEndianConversion.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpHashPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpIDEA.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libs\pgpcdk\pub\include\pgpIKE.h

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# Begin Custom Build
ProjDir=.
InputPath=..\..\..\..\libs\pgpcdk\pub\include\pgpIKE.h
InputName=pgpIKE

"$(ProjDir)\ike\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(ProjDir)\ike\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# Begin Custom Build
ProjDir=.
InputPath=..\..\..\..\libs\pgpcdk\pub\include\pgpIKE.h
InputName=pgpIKE

"$(ProjDir)\ike\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(ProjDir)\ike\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

# Begin Custom Build
ProjDir=.
InputPath=..\..\..\..\libs\pgpcdk\pub\include\pgpIKE.h
InputName=pgpIKE

"$(ProjDir)\ike\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(ProjDir)\ike\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

# Begin Custom Build
ProjDir=.
InputPath=..\..\..\..\libs\pgpcdk\pub\include\pgpIKE.h
InputName=pgpIKE

"$(ProjDir)\ike\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(ProjDir)\ike\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPheader.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecAH.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecComp.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecContext.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecContextPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\ipsec\pgpIPsecESP.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpMD5.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMem.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMemoryMgr.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpMemoryMgrPriv.h
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMConfig.h
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMEvents.h
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMHost.h
# End Source File
# Begin Source File

SOURCE=.\shared\pm\pgpNetPMSA.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpPFLConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpRIPEMD160.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpRMWOLock.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\hash\pgpSHA.h
# End Source File
# Begin Source File

SOURCE=..\..\shared\minisdk\cipher\pgpSymmetricCipherPriv.h
# End Source File
# Begin Source File

SOURCE=.\minisdk\pgpThreads.h
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\version.h
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpn.h
# End Source File
# Begin Source File

SOURCE=.\shared\ndis\vpndbg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shared\PGPnetDriver.rc

!IF  "$(CFG)" == "pgpmac - Win32 Release"

# ADD BASE RSC /l 0x409 /i "shared"
# ADD RSC /l 0x409 /i "shared" /i "..\..\..\pgp\win32\include"

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Debug"

# ADD BASE RSC /l 0x409 /i "shared"
# ADD RSC /l 0x409 /i "shared" /i "..\..\..\pgp\win32\include"

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "pgpmac - Win32 Win95 Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
