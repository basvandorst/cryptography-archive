@echo off
if "%javac%"=="" set javac=javac -J-ms2m

%javac% %* java\lang\*.java netscape\security\*.java java\security\*.java java\security\interfaces\*.java
