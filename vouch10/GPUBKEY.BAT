@echo  off
if "%1"=="" goto xerr
cls
echo Generating Public_Key parameters (q, p, g)
echo.
pubKey /q  %1  %2
pubKey /p  %1  %3
pubKey /g  %1
goto end
:xerr
echo  Generate public_key parameters (q, p, g)
echo Usage:
echo       gPubKey  Key_Filename  [ q_length   p_length ]
echo.
echo  default q_length=5 (160 bits) ; default p_length=16 (512 bits)
echo.
:end
