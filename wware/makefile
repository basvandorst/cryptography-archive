all: sm.exe smv.exe c.txt b1.txt b2.txt

sm.exe: sm.c
  cl /DVERBOSE=0 sm.c
  del sm.obj

smv.exe: sm.c
  copy sm.c smv.c
  cl /DVERBOSE=1 smv.c
  del smv.c
  del smv.obj

b.txt: a.txt sm.exe
  rem Encrypting cleartext
  sm a.txt "The owls are not what they seem." b.txt

c.txt: b.txt sm.exe
  rem Decrypting ciphertext
  sm b.txt "The owls are not what they seem." c.txt 1

b1.txt: a.txt smv.exe
  rem Encrypting cleartext
  smv a.txt "My Hovercraft is full of Eels." b.txt > b1.txt

b2.txt: a.txt smv.exe
  rem Encrypting cleartext
  smv a.txt "My Hovercraft is full of eels." b.txt > b2.txt
