@ Echo Off

cvs update
call vcvars32 x86
call PGPbuild release
md release
call release
cd install
cd data
md eudora
cd eudora
del pgpplugin.dll
cd ..
md WinSysDir
cd WinSysDir
del *.*
cd ..
md PGPmail
cd PGPmail
del pgptray.exe
del pgpkeys.exe
cd ..\..\..
cd release
copy pgpplugin.dll ..\install\data\Eudora
copy *.exe ..\install\data\PGPmail
copy *.dll ..\install\data\WinSysDir
cd ..\install
call build
md singleinstall
cd singleinstall
md disk1
cd disk1
del *.*
cd ..
md disk2
cd disk2
del *.*
cd..
md disk3
cd disk3
del *.*
cd ..
rd disk1
rd disk2
rd disk3
md disk1
md disk2
md disk3
cd ..
xcopy /I disk1\*.* singleinstall\disk1
xcopy /I disk2\*.* singleinstall\diak2
xcopy /I disk3\*.* singleinstall\disk3
