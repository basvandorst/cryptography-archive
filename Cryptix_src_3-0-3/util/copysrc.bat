@echo off
call util\clean

echo Copying source release...
md tmp tmp\doc tmp\guide tmp\images tmp\license tmp\native tmp\oldpgp tmp\src tmp\src10 tmp\util %noerrors%

copy README.html tmp >nul
copy FAQ.html tmp >nul

copy doc\placeholder.txt tmp\doc\packages.html >nul
copy doc\placeholder.txt tmp\doc >nul
copy doc\build.* tmp\doc >nul

xcopy /s /q guide\* tmp\guide >nul
xcopy /s /q images\* tmp\images >nul
xcopy /s /q license\* tmp\license >nul
xcopy /s /q native\* tmp\native >nul
xcopy /s /q oldpgp\* tmp\oldpgp >nul
xcopy /s /q src\* tmp\src >nul
xcopy /s /q src10\* tmp\src10 >nul
xcopy /s /q util\* tmp\util >nul

cd tmp
del /s /q .#* >nul
cd ..

rem Can't do this on 95/NT.
rem find tmp -name "CVS" |xargs rm -rf
