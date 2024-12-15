@echo off

rem     MAKEALL.BAT
rem     batch file for rebuilding all OBJ, TPU and EXE
rem     Turbo Pascal 6.0 (or better), Microsoft Assembler 5.0
rem     (or better) and LINK.EXE are needed
rem     (c)1996 Markus Hahn & Cedric Reinartz


rem     compile Pascal demo...

masm bfeng386;

rem (building sequence for Turbo Pascal...)
rem tpc blowfish
rem tpc /$X+ pas_demo

rem (... or much simpler in Borland Pascal)
bpc /b pas_demo


rem     compile assembly demo...

masm asm_demo;
link asm_demo;

