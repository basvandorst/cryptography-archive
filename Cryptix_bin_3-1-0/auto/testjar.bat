@echo off
REM testjar.bat written automagically by testjar.sh
REM local java options (uncomment if needed)
REM if "%jopt%"=="" set jopt=
REM local JVM
if "%java%"=="" set java=java

REM the PWD setting does not work with OS/2
if "%PWD%"=="" set PWD=%~f1
set OLDCP=%CLASSPATH%

REM good tests for ./SPT_0-1-0.jar - bad tests commented out
set CLASSPATH=;%PWD%\IJCE_0-1-0.jar;%PWD%\SPT_0-1-0.jar;%PWD%
%java% %jopt% cryptix.provider.Install
if errorlevel != 10 goto exit
%java% %jopt% cryptix.test.TestBR
%java% %jopt% cryptix.test.Test3LFSR
%java% %jopt% cryptix.test.TestBase64Stream
REM end ./SPT_0-1-0.jar

REM good tests for ./ALG_0-1-0.jar - bad tests commented out
set CLASSPATH=;%PWD%\SPT_0-1-0.jar;%PWD%\IJCE_0-1-0.jar;%PWD%\ALG_0-1-0.jar;%PWD%
%java% %jopt% cryptix.test.TestIJCE
%java% %jopt% cryptix.test.TestDES
%java% %jopt% cryptix.test.TestDES_EDE3
%java% %jopt% cryptix.test.TestBlowfish
%java% %jopt% cryptix.test.TestCAST5
%java% %jopt% cryptix.test.TestIDEA
%java% %jopt% cryptix.test.TestLOKI91
%java% %jopt% cryptix.test.TestRC2
%java% %jopt% cryptix.test.TestRC4
%java% %jopt% cryptix.test.TestSAFER
%java% %jopt% cryptix.test.TestSquare
%java% %jopt% cryptix.test.TestSPEED
%java% %jopt% cryptix.test.TestHMAC
%java% %jopt% cryptix.test.TestMD2
%java% %jopt% cryptix.test.TestMD4
%java% %jopt% cryptix.test.TestRIPEMD128
%java% %jopt% cryptix.test.TestRIPEMD160
%java% %jopt% cryptix.test.TestHAVAL
%java% %jopt% cryptix.test.TestSHA1
%java% %jopt% cryptix.test.TestSHA0
%java% %jopt% cryptix.test.TestMD5
REM end ./ALG_0-1-0.jar

REM good tests for ./RSA_0-1-0.jar - bad tests commented out
set CLASSPATH=;%PWD%\SPT_0-1-0.jar;%PWD%\IJCE_0-1-0.jar;%PWD%\ALG_0-1-0.jar;%PWD%\RSA_0-1-0.jar;%PWD%
%java% %jopt% cryptix.test.TestRSA
REM end ./RSA_0-1-0.jar

REM good tests for ./ELGAMAL_0-1-0.jar - bad tests commented out
set CLASSPATH=;%PWD%\SPT_0-1-0.jar;%PWD%\IJCE_0-1-0.jar;%PWD%\ALG_0-1-0.jar;%PWD%\ELGAMAL_0-1-0.jar;%PWD%
%java% %jopt% cryptix.test.TestElGamal
REM end ./ELGAMAL_0-1-0.jar

REM good tests for ./OLD_0-1-4.jar - bad tests commented out
set CLASSPATH=;%PWD%\SPT_0-1-0.jar;%PWD%\IJCE_0-1-0.jar;%PWD%\ALG_0-1-0.jar;%PWD%\OLD_0-1-4.jar;%PWD%
%java% %jopt% cryptix.security.Blowfish
%java% %jopt% cryptix.security.IDEA
%java% %jopt% cryptix.security.SPEED
%java% %jopt% cryptix.security.DES
%java% %jopt% cryptix.security.MD5
%java% %jopt% cryptix.security.SHA
%java% %jopt% cryptix.security.SHA0
%java% %jopt% cryptix.security.HashMD5
%java% %jopt% cryptix.security.HashSHA0
%java% %jopt% cryptix.security.HashSHA
REM end ./OLD_0-1-4.jar

REM good tests for ./TOOLS_0-1-0.jar - bad tests commented out
set CLASSPATH=;%PWD%\SPT_0-1-0.jar;%PWD%\IJCE_0-1-0.jar;%PWD%\ALG_0-1-0.jar;%PWD%\TOOLS_0-1-0.jar;%PWD%
%java% %jopt% cryptix.test.TestUnixCrypt
%java% %jopt% cryptix.test.TestScar
REM end ./TOOLS_0-1-0.jar

REM good tests for ./PGP26_0-1-0.jar - bad tests commented out
set CLASSPATH=;%PWD%\OLD_0-1-4.jar;%PWD%\RSA_0-1-0.jar;%PWD%\SPT_0-1-0.jar;%PWD%\PGP26_0-1-0.jar;%PWD%
%java% %jopt% cryptix.test.TestPGP
REM end ./PGP26_0-1-0.jar

:exit
set CLASSPATH=%OLDCP%
REM end testjar.bat
