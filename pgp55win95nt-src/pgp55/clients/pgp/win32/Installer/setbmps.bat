@ECHO OFF
if %1 == b copy bmps\business\setup.bmp   disk1
if %1 == b copy bmps\business\setuplo.bmp tempstuff\setup.bmp
if %1 == b copy bmps\business\setuplo.bmp uninstaller\tempstuff\splash.bmp

if %1 == p copy bmps\personal\setup.bmp   disk1
if %1 == p copy bmps\personal\setuplo.bmp tempstuff\setup.bmp
if %1 == p copy bmps\personal\setuplo.bmp uninstaller\tempstuff\splash.bmp

if %1 == f copy bmps\freeware\setup.bmp   disk1
if %1 == f copy bmps\freeware\setuplo.bmp tempstuff\setup.bmp
if %1 == f copy bmps\freeware\setuplo.bmp uninstaller\tempstuff\splash.bmp