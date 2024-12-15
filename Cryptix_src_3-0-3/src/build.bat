@echo off
if "%javac%"=="" set javac=javac -J-ms2m

call build-ijce
%javac% %* cryptix\*.java cryptix\util\core\*.java cryptix\provider\*.java cryptix\provider\cipher\*.java cryptix\provider\mode\*.java
%javac% %* cryptix\provider\padding\*.java cryptix\provider\key\*.java cryptix\provider\md\*.java cryptix\provider\mac\*.java
%javac% %* cryptix\provider\rsa\*.java cryptix\provider\elgamal\*.java cryptix\provider\rpk\*.java
%javac% %* cryptix\tools\*.java cryptix\util\i18n\*.java cryptix\util\io\*.java cryptix\util\math\*.java
%javac% %* cryptix\util\checksum\*.java cryptix\util\mime\*.java cryptix\util\test\*.java cryptix\test\*.java

rem %javac% %* cryptix\ui\*.java cryptix\ui\cli\*.java
rem %javac% %* cryptix\util\asn1\*.java cryptix\ssl\*.java cryptix\ssl\cipherspec\*.java
rem %javac% %* cryptix\pgp\*.java cryptix\pgp\charset\*.java cryptix\pgp\engine\*.java cryptix\pgp\engine\toolkit\*.java
