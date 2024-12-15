rem Batch file to create PC-SCRAMBLER
rem Uses Medium memory model version of the miracl library

cl /c /AM scramble.c
cl /c /AM talk.c
cl /c /AM strings.c
cl /c /AM datalink.c
cl /c /AM random.c
cl /c /AM crypto.c

link /SEGMENTS:256 /NOE scramble+talk+strings+datalink++random+crypto,,,gfcm+gfdm+miraclm;

rem now create clamp and unclamp utilities, using small model miracl

cl /c clamp.c
cl /c unclamp.c
link clamp,,,miracl;
link unclamp,,,miracl;

