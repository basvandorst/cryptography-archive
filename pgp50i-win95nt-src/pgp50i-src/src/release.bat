@echo off
REM release install builder
REM $Id: release.bat,v 1.1.2.2 1997/06/12 02:41:14 mhw Exp $

mkdir ..\win32\lib
copy lib\bn\%1\bn.lib ..\win32\lib
copy lib\pgp\%1\pgp.lib ..\win32\lib
copy lib\pgp\keydb\%1\keydb.lib ..\win32\lib
copy lib\simple\%1\simple.lib ..\win32\lib
copy lib\bn\%1\bn.dll ..\win32\lib
copy lib\pgp\%1\pgp.dll ..\win32\lib
copy lib\pgp\keydb\%1\keydb.dll ..\win32\lib
copy lib\simple\%1\simple.dll ..\win32\lib
