# Microsoft Developer Studio Project File - Name="PGPcdk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPcdk - Win32 Debug Auth Only
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPcdk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPcdk.mak" CFG="PGPcdk - Win32 Debug Auth Only"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPcdk - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcdk - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcdk - Win32 Release Auth Only" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPcdk - Win32 Debug Auth Only" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPcdk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\file" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\lthread" /I ".." /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\crypto\bignum" /I "..\..\priv\crypto\cipher" /I "..\..\priv\crypto\compress" /I "..\..\priv\crypto\hash" /I "..\..\priv\crypto\pipe\crypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\parser" /I "..\..\priv\crypto\pipe\sig" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\crypto\pipe\utils" /I "..\..\priv\crypto\random" /I "..\..\priv\debug" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\priv\keys\pubkey" /I "..\..\priv\regexp" /I "..\..\priv\utilities\errors" /I "..\..\priv\utilities" /I "..\..\priv\utilities\helper" /I "..\..\priv\utilities\utils" /I "..\..\priv\utilities\prefs" /I "..\..\priv\clientlib" /I "..\..\priv\encrypt" /I "..\..\..\pfl\common\sorting" /I "..\..\priv\external\win32\bsafe\include" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ..\..\..\pfl\win32\pflCommon\Release\pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /def:".\PGPcdk.def" /out:".\Release/PGP_SDK.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Creating static library .\Release\PGPsdkstatic.lib
PostBuild_Cmds=lib /nologo /out:.\Release\PGPsdkstatic.lib .\Release\*.obj\
                  .\Release\*.res
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPcdk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\file" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\lthread" /I ".." /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\crypto\bignum" /I "..\..\priv\crypto\cipher" /I "..\..\priv\crypto\compress" /I "..\..\priv\crypto\hash" /I "..\..\priv\crypto\pipe\crypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\parser" /I "..\..\priv\crypto\pipe\sig" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\crypto\pipe\utils" /I "..\..\priv\crypto\random" /I "..\..\priv\debug" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\priv\keys\pubkey" /I "..\..\priv\regexp" /I "..\..\priv\utilities\errors" /I "..\..\priv\utilities" /I "..\..\priv\utilities\helper" /I "..\..\priv\utilities\utils" /I "..\..\priv\utilities\prefs" /I "..\..\priv\clientlib" /I "..\..\priv\encrypt" /I "..\..\..\pfl\common\sorting" /I "..\..\priv\external\win32\bsafe\include" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 ..\..\..\pfl\win32\pflCommon\Debug\pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\Debug/PGP_SDK.dll"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Creating static library .\Debug\PGPsdkstatic.lib
PostBuild_Cmds=lib /nologo /out:.\Debug\PGPsdkstatic.lib .\Debug\*.obj\
                  .\Debug\*.res
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPcdk - Win32 Release Auth Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPcdk__"
# PROP BASE Intermediate_Dir "PGPcdk__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release\Authentication"
# PROP Intermediate_Dir ".\Release\Authentication"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\file" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\lthread" /I ".." /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\crypto\bignum" /I "..\..\priv\crypto\cipher" /I "..\..\priv\crypto\compress" /I "..\..\priv\crypto\hash" /I "..\..\priv\crypto\pipe\crypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\parser" /I "..\..\priv\crypto\pipe\sig" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\crypto\pipe\utils" /I "..\..\priv\crypto\random" /I "..\..\priv\debug" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\priv\keys\pubkey" /I "..\..\priv\regexp" /I "..\..\priv\utilities\errors" /I "..\..\priv\utilities" /I "..\..\priv\utilities\helper" /I "..\..\priv\utilities\utils" /I "..\..\priv\utilities\prefs" /I "..\..\priv\clientlib" /I "..\..\priv\encrypt" /I "..\..\..\pfl\common\sorting" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\file" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\lthread" /I ".." /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\crypto\bignum" /I "..\..\priv\crypto\cipher" /I "..\..\priv\crypto\compress" /I "..\..\priv\crypto\hash" /I "..\..\priv\crypto\pipe\crypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\parser" /I "..\..\priv\crypto\pipe\sig" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\crypto\pipe\utils" /I "..\..\priv\crypto\random" /I "..\..\priv\debug" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\priv\keys\pubkey" /I "..\..\priv\regexp" /I "..\..\priv\utilities\errors" /I "..\..\priv\utilities" /I "..\..\priv\utilities\helper" /I "..\..\priv\utilities\utils" /I "..\..\priv\utilities\prefs" /I "..\..\priv\clientlib" /I "..\..\priv\encrypt" /I "..\..\..\pfl\common\sorting" /I "..\..\priv\external\win32\bsafe\include" /D PGP_ENCRYPT_DISABLE=1 /D PGP_DECRYPT_DISABLE=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=0 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\pfl\win32\pflCommon\Release\pflCommon.lib /nologo /subsystem:windows /dll /machine:I386 /def:".\PGPcdk.def" /out:".\Release/PGP_SDK.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ..\..\..\pfl\win32\pflCommon\Release\pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /def:".\PGPcdk.def" /out:".\Release\Authentication/PGP_SDK_LTD.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Creating static library\
   .\Release\Authentication\PGPsdkLtdStatic.lib
PostBuild_Cmds=lib /nologo /out:.\Release\Authentication\PGPsdkLtdStatic.lib\
        .\Release\Authentication\*.obj           .\Release\Authentication\*.res
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PGPcdk - Win32 Debug Auth Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PGPcdk__"
# PROP BASE Intermediate_Dir "PGPcdk__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug\Authentication"
# PROP Intermediate_Dir ".\Debug\Authentication"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\file" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\lthread" /I ".." /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\crypto\bignum" /I "..\..\priv\crypto\cipher" /I "..\..\priv\crypto\compress" /I "..\..\priv\crypto\hash" /I "..\..\priv\crypto\pipe\crypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\parser" /I "..\..\priv\crypto\pipe\sig" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\crypto\pipe\utils" /I "..\..\priv\crypto\random" /I "..\..\priv\debug" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\priv\keys\pubkey" /I "..\..\priv\regexp" /I "..\..\priv\utilities\errors" /I "..\..\priv\utilities" /I "..\..\priv\utilities\helper" /I "..\..\priv\utilities\utils" /I "..\..\priv\utilities\prefs" /I "..\..\priv\clientlib" /I "..\..\priv\encrypt" /I "..\..\..\pfl\common\sorting" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\..\..\pfl\common\util" /I "..\..\..\pfl\win32" /I "..\..\..\pfl\common" /I "..\..\..\pfl\common\file" /I "..\..\..\pfl\common\prefs" /I "..\..\..\pfl\common\lthread" /I ".." /I "..\..\pub\include" /I "..\..\priv\include" /I "..\..\priv\include\opaque" /I "..\..\priv\crypto\bignum" /I "..\..\priv\crypto\cipher" /I "..\..\priv\crypto\compress" /I "..\..\priv\crypto\hash" /I "..\..\priv\crypto\pipe\crypt" /I "..\..\priv\crypto\pipe\file" /I "..\..\priv\crypto\pipe\parser" /I "..\..\priv\crypto\pipe\sig" /I "..\..\priv\crypto\pipe\text" /I "..\..\priv\crypto\pipe\utils" /I "..\..\priv\crypto\random" /I "..\..\priv\debug" /I "..\..\priv\keys\keydb" /I "..\..\priv\keys\keys" /I "..\..\priv\keys\pubkey" /I "..\..\priv\regexp" /I "..\..\priv\utilities\errors" /I "..\..\priv\utilities" /I "..\..\priv\utilities\helper" /I "..\..\priv\utilities\utils" /I "..\..\priv\utilities\prefs" /I "..\..\priv\clientlib" /I "..\..\priv\encrypt" /I "..\..\..\pfl\common\sorting" /I "..\..\priv\external\win32\bsafe\include" /D PGP_ENCRYPT_DISABLE=1 /D PGP_DECRYPT_DISABLE=1 /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D BNINCLUDE=bni80386c.h /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\pfl\win32\pflCommon\Debug\pflCommon.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\Debug/PGP_SDK.dll"
# ADD LINK32 ..\..\..\pfl\win32\pflCommon\Debug\pflCommon.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\Debug\Authentication/PGP_SDK_LTD.dll"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Desc=Creating static library\
   .\Debug\Authentication\PGPsdkLtdStatic.lib
PostBuild_Cmds=lib /nologo /out:.\Debug\Authentication\PGPsdkLtdStatic.lib\
       .\Debug\Authentication\*.obj            .\Debug\Authentication\*.res
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PGPcdk - Win32 Release"
# Name "PGPcdk - Win32 Debug"
# Name "PGPcdk - Win32 Release Auth Only"
# Name "PGPcdk - Win32 Debug Auth Only"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bn.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bn32.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bngermain.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bni32.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bni80386c.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnimem.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bninit32.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnjacobi.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnlegal.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnprime.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnprint.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnsieve.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpAddHdr.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpAnnotate.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpArmor.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpArmrFil.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\pgpBigNum.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpBSRSAGlue.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpBufMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpByteFIFO.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpCAST5.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpCBC.c
# End Source File
# Begin Source File

SOURCE=.\PGPcdk.def

!IF  "$(CFG)" == "PGPcdk - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "PGPcdk - Win32 Debug"

!ELSEIF  "$(CFG)" == "PGPcdk - Win32 Release Auth Only"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "PGPcdk - Win32 Debug Auth Only"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpCFB.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpCharMap.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpCiphrMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\clientlib\pgpClientEncode.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\clientlib\pgpClientKeyDB.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpCompMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpConf.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpContext.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpConvMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpCopyMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpCRC.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\debug\pgpDEBUGStartup.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\encrypt\pgpDecode.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpDecPipe.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpDefMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpDES3.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpDevNull.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpDSAKey.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpElGKey.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\encrypt\pgpEncode.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpEncPipe.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\encrypt\pgpEncSubr.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpEnumeratedSet.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpEnv.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\errors\pgpErrors.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpESK.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\encrypt\pgpEvent.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpFeatures.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFIFO.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFile.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpFileDB.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFileFIFO.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpFileMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFileNames.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFileRef.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpFileType.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpFilteredSet.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpFixedKey.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFlexFIFO.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpGroups.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpGroupsUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpHash.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\sig\pgpHashMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpHeader.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpHMAC.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpIDEA.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpInfMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpJoin.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeyFilter.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeyID.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeyIter.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeyLib.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeyMan.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpKeyMisc.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeySet.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpKeySpec.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpKeyUpd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\file\pgpLineEndIO.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpLiteral.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpMacBinary.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpMacFileMapping.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpMakePKE.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpMakeSig.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpMD2.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpMD5.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpMemDB.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpMemFile.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpMemMod.c
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpMemoryMgr.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpMemPool.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpMsg.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpMSRSAGlue.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpOptionList.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpPassCach.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpPipeFile.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpPKEMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpPktList.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpPrsAsc.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpPrsBin.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpPubKey.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpPublicKey.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpRadix64.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\random\pgpRandomPool.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\random\pgpRandomX9_17.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpReadAnn.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\regexp\pgpRegExp.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpRIPEMD160.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\random\pgpRndSeed.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\random\pgpRndWin32.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngMnt.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPars.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPkt.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPriv.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPub.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngRead.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpRSAGlue.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpRSAKey.c
# End Source File
# Begin Source File

SOURCE=.\pgpsdk.rc
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\prefs\pgpSDKPrefs.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpSHA.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\secshare\pgpShamir.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpSig.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\sig\pgpSigMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpSigPipe.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpSigSpec.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpSplit.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpStr2Key.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpSymmetricCipher.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpTextFilt.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpTimeBomb.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpTimeDate.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpTrstPkt.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpTrust.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpUnionDB.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpUnionSet.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpUtilities.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpVerifyRa.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpVMemMod.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpVrfySig.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keydb\pgpX509Keys.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpZBits.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpZDeflate.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\compress\pgpZInflate.c
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpZTrees.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bn.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bn32.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bngermain.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bni32.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnimem.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnjacobi.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnlegal.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnprime.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnprint.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\bignum\bnsieve.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpAddHdr.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpArmor.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpArmrFil.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpBufMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpByteFIFO.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpCAST5.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpCharMap.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpCiphrMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpCompMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpConf.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpContext.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpConvMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpCopyMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpCRC.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\debug\pgpDEBUGStartup.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpDecPipe.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpDefMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpDES3.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpDevNull.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpDSAKey.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpElGKey.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpEncPipe.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpEnv.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpESK.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFIFO.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFile.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpFileMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFileNames.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpFileRef.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpFileType.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpFixedKey.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\sig\pgpHashMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\cipher\pgpIDEA.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpInfMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpJoin.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpKeyMisc.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpKeySpec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpLeaks.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpLiteral.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpMakePKE.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpMakeSig.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpMD2.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpMD5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pfl\common\pgpMem.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpMemMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpMemPool.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\pgpOptionList.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpPassCach.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\crypt\pgpPKEMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpPktList.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpPrsAsc.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpPrsBin.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpPubKey.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\file\pgpRadix64.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\random\pgpRandomX9_17.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpReadAnn.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpRIPEMD160.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\random\pgpRndSeed.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngMnt.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPars.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPkt.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngPub.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpRngRead.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpRSAGlue.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpRSAKey.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\hash\pgpSHA.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\pubkey\pgpSig.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\sig\pgpSigMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpSigPipe.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\utils\pgpSigSpec.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpSplit.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpStr2Key.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\text\pgpTextFilt.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\utilities\helper\pgpTimeDate.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpTrstPkt.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\keys\keys\pgpTrust.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpVerifyRa.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\utils\pgpVMemMod.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\pipe\parser\pgpVrfySig.h
# End Source File
# Begin Source File

SOURCE=..\..\priv\crypto\compress\pgpZInflate.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
