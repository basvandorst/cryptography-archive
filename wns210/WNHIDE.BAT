@echo off
rem  WNHIDE.BAT   Copyright (C) 1994 by Ray (Arsen) Arachelian
rem                      All Rights Reserved, GTDR.
rem 
rem  This is a sample batch file showing how to hide an encrypted file
rem  in a PCX file.
rem

if exist %1 goto okay1
goto help
:okay1
if exist %3 goto start

:help
  echo Usage: WNHIDE PCXFILE.PCX BITS TOHIDE.TXT PASSWORD {OTHER WNSTORM OPTIONS}
  rem                  %1         %2   %3        %4       %5 %6 %7  %8 %9
  echo ÿ
  echo This will create WNSTEGO.PCX which contains the hidden encrypted message.
  goto end

:start
  rem remove temporary files
 
   del ~temp1.~xr
   del ~temp2.~xr
   cls

   wninject e %1 ~temp1.~xr %2

  rem encrypt the text file outputting  ~temp2.~xr and using the lsb's as the
  rem random number info.  Using stealth mode is required to strip off the
  rem header.
 
    wnstorm  e %3 ~temp2.~xr %4 -rf:~temp1.~xr -n %5 %6 %7 %8 %9

  rem now inject the results of wnstorm into a new copy of the pcxfile.

    wninject i %1 ~temp2.~xr wnstego.pcx  %2
  
  rem remove temporary files
    del ~temp1.~xr
    del ~temp2.~xr
 
 echo Your stegoed encrypted PCX file is WNSTEGO.PCX.
:end
