::.....................................................................
rem    demo.bat    Sifr Demonstration             (c)Cybanim 12 Aug 1994
rem    make 2 small keys:    ( for 512 bit keys replace 280 with 512)
Luckey g alice /k280
Luckey g bob /k280
rem    set pin to nothing:
set PIN=0000
rem    sign and cipher Sifr.DOC as a demonstration:
Sifr -sct sifr.doc alice bob
rem    sifr.sid now has the enciphered sifr.doc
rem    now decipher and verify sifr.sid:
sifr sifr
rem    The deciphered file is now    Sifr.Sid
rem    Check that is is identical to Sifr.Doc