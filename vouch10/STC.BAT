@echo off
if "%1"=="" goto x1
c:\vch\vSTc.exe  %1   c:\vch
goto end
:x1
echo    SecurityTool (cap): VCH 1.0: Copyright (c) 1993 Awais M. Hussain 
echo"Usage: STC.BAT  <In_File>                                               "
:end
