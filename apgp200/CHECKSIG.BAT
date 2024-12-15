@echo off
rem
rem  Run this file to check signature 
rem  certificates for AutoPGP .EXE files
rem
pgp +verbose=0 decrypt.sig decrypt.exe
pgp +verbose=0 encrypt.sig encrypt.exe
pgp +verbose=0 install.sig install.exe
pgp +verbose=0 register.sig register.exe
