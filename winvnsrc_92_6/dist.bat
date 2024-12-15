set include=c:\msvc\include;c:\msvc\mfc\include;c:\winvn\include
nmake clean
nmake version
mawk -f incver.awk version.h >version.i
del version.h
rename version.i version.h
cd gensock
nmake clean
cd ..
nmake zip
nmake release
zip c:\winvnsrc.zip c:\winvn\winvngen.map
copy winvngen.exe \gendist\winvn.exe
cd gensock
nmake release
copy pwksock.dll \gendist\pwksock.dll
copy pwksock.dll \winvn\pwksock.dll
copy gensock.dll \gendist\gwinsock.dll
copy gensock.dll \winvn\gwinsock.dll
cd \gendist
del c:\winvn.zip
zip c:\winvn.zip *.*
