cl /AL /c /I..\rsaref\source /I..\main rcerts.c
cl /AL /c /I..\rsaref\source /I..\main rcertsms.c
link /noi /stack:26000 /onerror:noexe @rcerts.lrf
