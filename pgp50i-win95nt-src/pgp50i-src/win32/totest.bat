rem @echo off
REM release install builder
REM $Id: totest.bat,v 1.1.1.1 1997/04/29 00:23:28 philipn Exp $

copy %1\pgprecip.dll c:\winnt35\system32
copy %1\pgpcmdlg.dll c:\winnt35\system32
copy %1\pgpwctx.dll c:\winnt35\system32
copy %1\pgptray.exe "c:\program files\pgp\pgp"
copy %1\pgpks.dll c:\winnt35\system32
copy %1\pgpkeys.exe "c:\program files\pgp\pgp"
copy %1\PGPlugin.dll "c:\program files\eudora\plugins"
copy %1\bn.dll c:\winnt35\system32
copy %1\simple.dll c:\winnt35\system32
copy %1\pgp.dll c:\winnt35\system32
copy %1\keydb.dll c:\winnt35\system32
