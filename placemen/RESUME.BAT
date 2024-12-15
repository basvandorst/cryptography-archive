@echo off
rem  Resume.bat -- simple shell for Resumem.bat; copies files to
rem  local disk because executing the stuff directly from the file
rem  server seems to give problems.   /mrr  3-Sept-92
copy j:\frep\*.* c:\work
c:\work\resumem c:\work
