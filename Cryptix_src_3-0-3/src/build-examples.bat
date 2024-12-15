@echo off
if "%javac%"=="" set javac=javac -J-ms2m

%javac% %* cryptix\examples\*.java cryptix\examples\math\*.java
rem %javac% %* cryptix\examples\ssl\*.java
rem %javac% %* cryptix\examples\pgp\*.java cryptix\examples\pgp\engine\*.java
