@echo off

if EXIST "Setup Files" rmdir /Q /S "Setup Files"

MKDIR "Setup Files"
MKDIR "Setup Files\Compressed Files"
MKDIR "Setup Files\Compressed Files\0009-English"
MKDIR "Setup Files\Compressed Files\0009-English\Intel 32"
MKDIR "Setup Files\Compressed Files\0009-English\OS Independent"
MKDIR "Setup Files\Compressed Files\Language Independent"
MKDIR "Setup Files\Compressed Files\Language Independent\Intel 32"
MKDIR "Setup Files\Compressed Files\Language Independent\OS Independent"
MKDIR "Setup Files\Uncompressed Files"
MKDIR "Setup Files\Uncompressed Files\0009-English"
MKDIR "Setup Files\Uncompressed Files\0009-English\Intel 32"
MKDIR "Setup Files\Uncompressed Files\0009-English\OS Independent"
