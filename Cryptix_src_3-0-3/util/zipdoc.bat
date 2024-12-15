@echo off
if "%OS%"=="Windows_NT" goto winnt
set noerrors= 
goto ok

:winnt
set "noerrors=2>nul"

:ok
if "%jar%"=="" set jar=jar

if exist doc\tree.html goto zip
call util\makedocs

:zip
echo Creating Cryptix-doc%CRYPTIX_VERSION%.zip file...
del ..\Cryptix-doc%CRYPTIX_VERSION%.zip %noerrors%
%jar% cfM ..\Cryptix-doc%CRYPTIX_VERSION%.zip guide doc *.html COPYRIGHT.txt 
