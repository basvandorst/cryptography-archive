# Microsoft Developer Studio Project File - Name="PGPsdkNetworkLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPSDKNETWORKLIB - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPsdkNetworkLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPsdkNetworkLib.mak" CFG="PGPSDKNETWORKLIB - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPsdkNetworkLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPsdkNetworkLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPsdkNetworkLib - Win32 Debug Auth Only" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPsdkNetworkLib - Win32 Release Auth Only" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPsdkNetworkLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".." /I "..\..\..\pfl\common\classes" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\win32/pgpAsyncDNS" /I "..\..\pub\include" /I "..\..\priv\utilities" /I "..\..\priv\networklib\\" /I "..\..\priv\networklib\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\common\lthread" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\encrypt" /I "..\..\..\network\ldaplib\include" /I "..\..\priv\crypto\pipe\file" /I "..\..\..\pfl\common\file" /I "..\..\priv\utilities\helper" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\utilities\utils" /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D "NDEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../pgpcdk/release/pgp_sdk.lib ../../../network/ldaplib/win32/release/libldap.lib ../../../network/ldaplib/win32/release/liblber.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /base:"0x11000000" /subsystem:windows /dll /machine:I386 /out:"./Release/PGPsdkNL.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Building static library.
PostBuild_Cmds=rename .\Release\PGPsdkNetworkLibDLLMain.obj        PGPsdkNetworkLibDLLMain.ob1	lib  /nologo /out:.\Release\PGPsdkNLStatic.lib        .\Release\*.obj ..\..\..\network\ldaplib\win32\release\libldap.lib       ..\..\..\network\ldaplib\win32\release\liblber.lib	rename       .\Release\PGPsdkNetworkLibDLLMain.ob1  PGPsdkNetworkLibDLLMain.obj
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPsdkNetworkLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "..\..\..\pfl\common\classes" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\win32/pgpAsyncDNS" /I "..\..\pub\include" /I "..\..\priv\utilities" /I "..\..\priv\networklib\\" /I "..\..\priv\networklib\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\common\lthread" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\encrypt" /I "..\..\..\network\ldaplib\include" /I "..\..\priv\crypto\pipe\file" /I "..\..\..\pfl\common\file" /I "..\..\priv\utilities\helper" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\utilities\utils" /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D "_DEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../pgpcdk/debug/pgp_sdk.lib ../../../network/ldaplib/win32/debug/libldap.lib ../../../network/ldaplib/win32/debug/liblber.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /base:"0x11000000" /subsystem:windows /dll /debug /machine:I386 /out:"./Debug/PGPsdkNL.dll"
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Building static libraries.
PostBuild_Cmds=rename .\Debug\PGPsdkNetworkLibDLLMain.obj        PGPsdkNetworkLibDLLMain.ob1	lib  /nologo /out:.\Debug\PGPsdkNLStatic.lib        .\Debug\*.obj ..\..\..\network\ldaplib\win32\debug\libldap.lib       ..\..\..\network\ldaplib\win32\debug\liblber.lib	rename       .\Debug\PGPsdkNetworkLibDLLMain.ob1  PGPsdkNetworkLibDLLMain.obj
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPsdkNetworkLib - Win32 Debug Auth Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPsdkNe"
# PROP BASE Intermediate_Dir "PGPsdkNe"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug\Authentication"
# PROP Intermediate_Dir ".\Debug\Authentication"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I ".." /I "..\..\..\pfl\win32" /I "..\..\pub\include" /I "..\..\priv\utilities" /I "..\..\priv\networklib\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\common\lthread" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\encrypt" /I "..\..\..\network\ldaplib\include" /I "..\..\priv\crypto\pipe\file" /I "..\..\..\pfl\common\file" /I "..\..\priv\utilities\helper" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\utilities\utils" /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D "_DEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "..\..\..\pfl\common\classes" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\win32/pgpAsyncDNS" /I "..\..\pub\include" /I "..\..\priv\utilities" /I "..\..\priv\networklib\\" /I "..\..\priv\networklib\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\common\lthread" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\encrypt" /I "..\..\..\network\ldaplib\include" /I "..\..\priv\crypto\pipe\file" /I "..\..\..\pfl\common\file" /I "..\..\priv\utilities\helper" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\utilities\utils" /D PGP_ENCRYPT_DISABLE=1 /D PGP_DECRYPT_DISABLE=1 /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D "_DEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../pgpcdk/debug/pgp_sdk.lib ../../../network/ldaplib/win32/debug/libldap.lib ../../../network/ldaplib/win32/debug/liblber.lib ..\..\..\pfl\win32\pflCommon\Debug\pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"./Debug/PGPsdkNL.dll"
# SUBTRACT BASE LINK32 /map
# ADD LINK32 ../pgpcdk/debug/authentication/pgp_sdk_ltd.lib ../../../network/ldaplib/win32/debug/libldap.lib ../../../network/ldaplib/win32/debug/liblber.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /base:"0x11000000" /subsystem:windows /dll /debug /machine:I386 /out:"./Debug/Authentication/PGPsdkNLLtd.dll"
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Building static library.
PostBuild_Cmds=rename .\Debug\Authentication\PGPsdkNetworkLibDLLMain.obj        PGPsdkNetworkLibDLLMain.ob1	lib  /nologo     /out:.\Debug\Authentication\PGPsdkNLLtdStatic.lib        .\Debug\Authentication\*.obj ..\..\..\network\ldaplib\win32\debug\libldap.lib       ..\..\..\network\ldaplib\win32\debug\liblber.lib	rename       .\Debug\Authentication\PGPsdkNetworkLibDLLMain.ob1  PGPsdkNetworkLibDLLMain.obj
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPsdkNetworkLib - Win32 Release Auth Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPsdkN0"
# PROP BASE Intermediate_Dir "PGPsdkN0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release\Authentication"
# PROP Intermediate_Dir ".\Release\Authentication"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I ".." /I "..\..\..\pfl\win32" /I "..\..\pub\include" /I "..\..\priv\utilities" /I "..\..\priv\networklib\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\common\lthread" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\encrypt" /I "..\..\..\network\ldaplib\include" /I "..\..\priv\crypto\pipe\file" /I "..\..\..\pfl\common\file" /I "..\..\priv\utilities\helper" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\utilities\utils" /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D "NDEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".." /I "..\..\..\pfl\common\classes" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\win32/pgpAsyncDNS" /I "..\..\pub\include" /I "..\..\priv\utilities" /I "..\..\priv\networklib\\" /I "..\..\priv\networklib\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\common\lthread" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\encrypt" /I "..\..\..\network\ldaplib\include" /I "..\..\priv\crypto\pipe\file" /I "..\..\..\pfl\common\file" /I "..\..\priv\utilities\helper" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\utilities\utils" /D PGP_ENCRYPT_DISABLE=1 /D PGP_DECRYPT_DISABLE=1 /D PGPSOCKETSLDAP=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D "NDEBUG" /D "NEEDPROTOS" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ../pgpcdk/release/pgp_sdk.lib ../../../network/ldaplib/win32/release/libldap.lib ../../../network/ldaplib/win32/release/liblber.lib ..\..\..\pfl\win32\pflCommon\release\pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"./Release/PGPsdkNL.dll"
# ADD LINK32 ../pgpcdk/release/authentication/pgp_sdk_ltd.lib ../../../network/ldaplib/win32/release/libldap.lib ../../../network/ldaplib/win32/release/liblber.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /base:"0x11000000" /subsystem:windows /dll /machine:I386 /out:"./Release/Authentication/PGPsdkNLLtd.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Building static library.
PostBuild_Cmds=rename .\Release\Authentication\PGPsdkNetworkLibDLLMain.obj        PGPsdkNetworkLibDLLMain.ob1	lib  /nologo     /out:.\Release\Authentication\PGPsdkNLLtdStatic.lib        .\Release\Authentication\*.obj     ..\..\..\network\ldaplib\win32\release\libldap.lib       ..\..\..\network\ldaplib\win32\release\liblber.lib	rename       .\Release\Authentication\PGPsdkNetworkLibDLLMain.ob1      PGPsdkNetworkLibDLLMain.obj
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPsdkNetworkLib - Win32 Release"
# Name "PGPsdkNetworkLib - Win32 Debug"
# Name "PGPsdkNetworkLib - Win32 Debug Auth Only"
# Name "PGPsdkNetworkLib - Win32 Release Auth Only"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CCRSEntrustServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CCRSServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CCRSVerisignServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CHTTPKeyServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CHTTPPGPKeyServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CHTTPXcertServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CKeyServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\CLDAPKeyServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\win32\pgpAsyncDNS\pgpAsyncDNS.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpBinaryTree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\util\pgpEndianConversion.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileIO.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileSpec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileSpecStd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileSpecVTBL.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileSpecWin32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpFileUtilities.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\ike\pgpIKE.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\ike\pgpIKEPrimes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpIO.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpIOUtilities.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\pgpKeyServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpMilliseconds.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\lthread\pgpMutex.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\pgpNetworkLibOptions.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpOptionList.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpPFLErrors.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\keyserver\pgpProxyServer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpRMWOLock.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\pgpSDKNetworkLib.c
# End Source File
# Begin Source File

SOURCE=.\PGPsdkNetworkLib.def
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\win32\PGPsdkNetworkLibDLLMain.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\lthread\pgpSemaphore.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\pgpsockets\common\pgpSockets.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpStdFileIO.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\util\pgpStrings.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\lthread\pgpThreads.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\tls\pgpTLS.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\networklib\tls\pgpTLSPrimes.c
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PGPsdkNetworkLib.rc
# End Source File
# End Group
# End Target
# End Project
