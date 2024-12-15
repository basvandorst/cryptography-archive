@echo off

deltree /y build

call build

mkdir build\snap
mkdir build\snap\cryptix-lib
mkdir build\snap\ijce-lib

cd build\classes

copy cryptix-lib\*.* ..\snap\cryptix-lib
copy ijce-lib\*.* ..\snap\ijce-lib

jar cvf cryptix3.jar java netscape cryptix

copy cryptix3.jar ..\snap

cd ..\snap

jar cvfM cryptix3-snap-%1-bin.zip cryptix3.jar cryptix-lib ijce-lib

cd ..\..

mkdir tmp
mkdir tmp\src
xcopy /s src\*.* tmp\src
copy *.bat tmp
copy *.sh tmp
copy *.txt tmp

cd tmp\src
deltree /y CVS
deltree /y cryptix\CVS
deltree /y cryptix\examples\CVS
deltree /y cryptix\examples\pgp\CVS
deltree /y cryptix\math\CVS
deltree /y cryptix\mime\CVS
deltree /y cryptix\pgp\CVS
deltree /y cryptix\pgp\charset\CVS
deltree /y cryptix\pgp\engine\CVS
deltree /y cryptix\pgp\engine\toolkit\CVS
deltree /y cryptix\provider\CVS
deltree /y cryptix\provider\cipher\CVS
deltree /y cryptix\provider\elgamal\CVS
deltree /y cryptix\provider\key\CVS
deltree /y cryptix\provider\mac\CVS
deltree /y cryptix\provider\md\CVS
deltree /y cryptix\provider\mode\CVS
deltree /y cryptix\provider\padding\CVS
deltree /y cryptix\provider\rsa\CVS
deltree /y cryptix\security\CVS
deltree /y cryptix\security\rsa\CVS
deltree /y cryptix\test\CVS
deltree /y cryptix\tools\CVS
deltree /y cryptix\util\CVS
deltree /y cryptix\util\checksum\CVS
deltree /y cryptix\util\core\CVS
deltree /y cryptix\util\gui\CVS
deltree /y cryptix\util\io\CVS
deltree /y cryptix\util\math\CVS
deltree /y cryptix\util\mime\CVS
deltree /y cryptix\util\test\CVS
deltree /y cryptix\util\test\data\CVS
deltree /y cryptix-lib\CVS
deltree /y ijce-lib\CVS
deltree /y java\CVS
deltree /y java\lang\CVS
deltree /y java\security\CVS
deltree /y java\security\interfaces\CVS
deltree /y netscape\CVS
deltree /y netscape\security\CVS
cd ..

jar cvfM ..\build\snap\cryptix3-snap-%1-src.zip .

cd ..

deltree /y tmp
