@echo off
::: Run this file to check signature
::: certificates for the br translation of PGP
::: - Rode esse arquivo para verificar os certificados de assinatura
::: - para os arquivos da tradu‡ao do PGP para portugues

::: This file is based on checksig.bat from Stale Schumacher's
:::   "Auto PGP v2.2Beta".  The following line was kept intact
:::   from the original file:
::: Thanks to John DeHaven for this file

::: Some of the signatures may fail if the wrong charset is used.
::: worse, pgp 2.6.2i or lower won't accept +charset=xxx command line option.

if "_RESTART"=="%1" goto RESTART
if "_SOLO"=="%1" goto SOLO

if ""=="%PGPPATH%" echo Por favor, ajuste a variavel de ambiente PGPPATH
if ""=="%PGPPATH%" goto END

pgp +batchmode +verbose=1
echo.
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Se o cabe‡alho do PGP nao apareceu acima desta linha, por favor
echo aborte esse batch (ctrl+break) e coloque o pgp no path.
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
pause Tecle algo para continuar

echo Verificando se a chave publica necessaria esta no molho publico.
pgp +batchmode +verbose=0 -kv 0x3fec6b39 > nul
if not errorlevel 1 echo Chave encontrada no molho publico.
if not errorlevel 1 goto CHECKIT
echo Adicionando chave no arquivo hmhkey.asc ao molho publico.
pgp +batchmode +verbose=0 -ka hmhkey.asc >nul
if not errorlevel 1 goto KEYADDED

echo.
echo ERRO: Nao pude encontrar a chave publica de  "Henrique Holschuh",
echo       nem pude encontrar o arquivo hmhkey.ASC para adiciona-la.
goto END

:KEYADDED
   pgp +batchmode +verbose=0 -kv 0x3fec6b39 >nul
   if errorlevel 1 goto PGPBUG
   echo.
   echo Voce nao possuia a chave publica de "Henrique Holschuh".
   echo Por enquanto, nos vamos usar a chave no arquivo HMHkey.asc,
   echo mas recomendo veementemente que voce obtenha tal chave de
   echo uma fonte intependente. Mais informa‡oes no arquivo NOTES.BR.
   echo.


:CHECKIT
   echo.
   echo Verificando assinaturas, por favor aguarde...
   CALL %0 _RESTART checksig bat language txt br hlp br2 hlp
   CALL %0 _SOLO notes.br pgpintro.asc

:REST1
   shift

:RESTART
   shift
   if ""=="%1" goto end
   if not exist %1.sig echo Nao pude encontrar o arquivo %1.SIG.
   if not exist %1.sig goto REST1
   if not exist %1.%2  echo Nao pude encontrar o arquivo %1.%2.
   if not exist %1.%2  goto REST1
   pgp +batchmode +verbose=0 +charset=latin1 %1.sig %1.%2 > nul
   if not errorlevel 1 echo OK: %1.%2 verificado!
if not errorlevel 1 goto REST1

   echo !!! AVISO !!!
   echo %1.%2 FALHOU no teste de autenticidade...
goto REST1

:SOLO
   shift
   if ""=="%1" goto end
   if not exist %1 echo Nao pude encontrar o arquivo %1.
   if not exist %1 goto SOLO
   if exist checksig.628 erase checksig.628
   pgp +batchmode +verbose=0 %1 -ochecksig.628 >nul
   if not errorlevel 1 echo OK: %1 verificado!
if not errorlevel 1 goto SOLO

   echo !!! AVISO !!!
   echo %1 FALHOU no teste de autenticidade... Tente verificar
   echo manualmente com pgp +charset=cp860 %1
goto SOLO

:PGPBUG
   echo.
   echo Nao consegui adicionar automaticamente a chave ao molho
   echo publico. Por favor, fa‡a-o manualmente atraves do comando
   echo    pgp -ka hmhkey.asc
   echo e rode novamente esse arquivo .BAT
   echo.

:END
   if exist checksig.628 erase checksig.628
