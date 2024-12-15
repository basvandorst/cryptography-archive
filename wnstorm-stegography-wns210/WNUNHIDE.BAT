@echo off
rem  WNUNHIDE.BAT   Copyright (C) 1994 by Ray (Arsen) Arachelian
rem               All Rights Reserved, GTDR.
rem 
rem  This is a sample batch file showing how to restore a hidden encrypted
rem  out of a PCX file.
rem

if exist %1 goto okay1
goto help
:okay1

goto start

:help
  echo Usage: WNUNHIDE WNSTEGO.PCX BITS HIDDEN.TXT PASSWORD {OTHER WNSTORM OPTIONS}
  rem                   %1         %2   %3          %4       %5 %6 %7  %8 %9
  echo ÿ
  echo This will create WNSTEGO.PCX which contains the hidden encrypted message.
  goto end

:start
  rem remove temporary files
 
   del ~temp1.~xr
   cls

  rem extract lsb's from PCXFILE
    wninject e %1 ~temp1.~xr %2

  rem decrypt the text file outputting hidden file.
  rem Using stealth mode is required to strip off the header.
  
    wnstorm  d ~temp1.~xr %3 %4 -n %5 %6 %7 %8 %9

  rem remove temporary files
    del ~temp1.~xr

:end
