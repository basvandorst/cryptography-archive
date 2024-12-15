# Microsoft Developer Studio Project File - Name="PGPcdkKeyServer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPcdkKeyServer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPcdkKeyServer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPcdkKeyServer.mak" CFG="PGPcdkKeyServer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPcdkKeyServer - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcdkKeyServer - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPcdkKeyServer - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../../network/pgpsockets/pub" /I "../../../network/ldaplib/include" /I "../../../pfl/win32" /I "../../../pfl/common" /I "../../../pfl/common/prefs" /I "../../../pfl/common/file" /I "../../../pfl/common/util" /I "../" /I "../../pub/include" /I "../../priv/include" /I "../../priv/utilities" /I "../../priv/include/opaque" /I "../../priv/encrypt" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D "NDEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /Fp"Release/PGPcdkKS.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../pgpcdk/release/pgpsdk.lib ../../../network/pgpsockets/win/src/release/pgpsockets.lib ../../../network/ldaplib/win32/release/libldap.lib ../../../network/ldaplib/win32/release/liblber.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Release/PGPsdkKS.dll"
# Begin Custom Build - Creating static library .\Release\PGPsdkKSstatic.lib
InputPath=.\Release\PGPsdkKS.dll
SOURCE=$(InputPath)

".\Release\PGPsdkKSstatic.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	lib /nologo /out:.\Release\PGPsdkKSstatic.lib .\Release\*.obj .\Release\*.res

# End Custom Build

!ELSEIF  "$(CFG)" == "PGPcdkKeyServer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "../../../network/pgpsockets/pub" /I "../../../network/ldaplib/include" /I "../../../pfl/win32" /I "../../../pfl/common" /I "../../../pfl/common/prefs" /I "../../../pfl/common/file" /I "../../../pfl/common/util" /I "../" /I "../../pub/include" /I "../../priv/include" /I "../../priv/utilities" /I "../../priv/include/opaque" /I "../../priv/encrypt" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "NEEDPROTOS" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"Debug/PGPcdkKS.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../pgpcdk/debug/pgpsdk.lib ../../../network/pgpsockets/win/src/debug/pgpsockets.lib ../../../network/ldaplib/win32/debug/libldap.lib ../../../network/ldaplib/win32/debug/liblber.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Debug/PGPsdkKS.dll" /pdbtype:sept
# Begin Custom Build - Creating static library .\Debug\PGPsdkKSstatic.lib
InputPath=.\Debug\PGPsdkKS.dll
SOURCE=$(InputPath)

".\Debug\PGPsdkKSstatic.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	lib /nologo /out:.\Debug\PGPsdkKSstatic.lib .\Debug\*.obj .\Debug\*.res

# End Custom Build

!ENDIF 

# Begin Target

# Name "PGPcdkKeyServer - Win32 Release"
# Name "PGPcdkKeyServer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp"
# Begin Source File

SOURCE=.\PGPcdkKeyServer.def
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keyserver\pgpKs.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keyserver\pgpKsHTTP.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keyserver\pgpKsLDAP.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\util\pgpStrings.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=..\..\..\network\ldap\include\lber.h
# End Source File
# Begin Source File

SOURCE=..\..\..\network\ldap\include\ldap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pflContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\prefs\pflPrefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pflTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpBase.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\opaque\pgpBigNumOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpCipherFeedback.h
# End Source File
# Begin Source File

SOURCE=..\pgpConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpEncode.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\encrypt\pgpEventPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileSpec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileSpecStd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileUtilities.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpIO.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\opaque\pgpKeyDBOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\pgpKeyFilterPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpKeys.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpKeyServer.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\pgpKeyServerPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpKSErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpLeaks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpMem.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\opaque\pgpOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\win32\pgpPFLConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpPFLErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\opaque\pgpPipelineOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpPubTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\include\opaque\pgpRingOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\network\pgpsockets\pub\PGPSockets.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpSymmetricCipher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\pub\include\pgpUtilities.h
# End Source File
# Begin Source File

SOURCE="..\..\..\network\ldap\include\proto-lber.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\network\ldap\include\proto-ldap.h"
# End Source File
# End Group
# Begin Source File

SOURCE=.\PGPsdkKeyServer.rc
# End Source File
# End Target
# End Project
