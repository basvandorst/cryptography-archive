@rem *** Nie zmieniaj tego pliku. ***
@echo off
move !instal.dat instal.bat >nul
command /E:1024 /C instal.bat %1 %2
move instal.bat !instal.dat >nul
