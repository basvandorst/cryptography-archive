
{
        PAS_DEMO.PAS
        test program for compatibility & speed of BLOWFISH.TPU
        language   : Turbo/Borland Pascal
        last update: 06/06/96
        (c)1996 Markus Hahn & Cedric Reinartz
}


program PAS_DEMO;
uses Crt, Dos, Blowfish;


const
  { official test vectors from DDJ 10/95... }
  key1        : String = 'abcdefghijklmnopqrstuvwxyz';
  testdata1_p : array[1..2] of ULONG = ($424c4f57, $46495348);
  testdata1_c : array[1..2] of ULONG = ($324ed0fe, $f413a203);

  key2        : String = 'Who is John Galt?';
  testdata2_p : array[1..2] of ULONG = ($fedcba98, $76543210);
  testdata2_c : array[1..2] of ULONG = ($cc91732b, $8022f684);

  BIGBUFSIZE  = 32000;
  TESTLOOPS   = 128;    { suitable for a DX4 }
  SCRLOOPS    = 1024;



{
        ULONG2HEX - converts a ULONG into a string
        -> ULONG
        <- String
}
function ULONG2Hex(ulTheVal : ULONG) : String;
const
  HEXTAB : String[16] = '0123456789abcdef';
var
      nI : Integer;
   sTemp : String[8];
begin
  sTemp[0]:=Chr(8);
  for nI:=0 to 7 do
    sTemp[8-nI]:=HEXTAB[((ulTheVal shr (nI*4)) and $0f) + 1];
  ULONG2Hex:=sTemp;
end;


{
        RANDOM32 - creates a 32 bit random value
        <- random ULONG
}
function Random32 : ULONG;
begin
  Random32:=(ULONG(Random($ffff)) shl 16) or Random($ffff);
end;


{ big buffer for benchmark }
type
  PBigBuf = ^TBigBuf;
  TBigBuf = array[1..BIGBUFSIZE] of Byte;


{ global data... }
var
  ulCBCIVL   : ULONG;   { IV = initialisation vector }
  ulCBCIVR   : ULONG;
  ulCBCLeft  : ULONG;
  ulCBCRight : ULONG;
  ulEqual    : ULONG;
  sMyKey     : String;
  testbuf    : array[1..6] of ULONG;
  unI        : UINT;
  biggy      : PBigBuf;
  ulStart    : ULONG;
  ulTime     : ULONG;
  ulRate     : ULONG;
  h,m,s,c    : UINT;
  pBoxes     : P_ULONG_Buffer;
  BF_key     : array[1..1058] of ULONG;
  BF_keysave : array[1..1058] of ULONG;
  pScreen    : P_ULONG_Buffer;



begin
     ClrScr;
     Randomize;

     Blowfish_GetBoxes(@BF_key[1]);      { save original boxes }
     { the following compare is not necessary. Just to show the function }
     if Blowfish_SetRounds(16) <> 16 then writeln('Could not set Rounds');

     { Are we compatible? }

     { test vector #1 }
     TextColor(10); WriteLn('test data #1...'); TextColor(7);

     Blowfish_Init(@key1[1], Length(key1));
     { show the first 6 p-boxes }
     pBoxes:=Blowfish_GetBoxPointer;
     Write('pboxes : ');
     for unI:=0 to 5 do
       Write(ULONG2Hex(pBoxes^[unI]), ' ');
     WriteLn;
     { now en-/decrypt and compare with the official values }
     WriteLn('plaintext : ', ULONG2Hex(testdata1_p[1]),
             ' ',            ULONG2Hex(testdata1_p[2]));
     Blowfish_ECBEncrypt(@testdata1_p[1], 8);
     WriteLn('ciphertext: ', ULONG2Hex(testdata1_p[1]),
             ' ',            ULONG2Hex(testdata1_p[2]));
     WriteLn('DDJ 10/95 : ', ULONG2Hex(testdata1_c[1]),
             ' ',            ULONG2Hex(testdata1_c[2]));
     Blowfish_ECBDecrypt(@testdata1_p[1], 8);
     WriteLn('decrypted : ', ULONG2Hex(testdata1_p[1]),
             ' ',            ULONG2Hex(testdata1_p[2]));

     { #2 }
     TextColor(10); WriteLn('test data #2...'); TextColor(7);
     Blowfish_SetBoxes(@BF_key[1]);      { reload boxes }
     Blowfish_Init(@key2[1], Length(key2));
     pBoxes:=Blowfish_GetBoxPointer;
     Write('pboxes : ');
     for unI:=0 to 5 do
       Write(ULONG2Hex(pBoxes^[unI]), ' ');
     WriteLn;
     WriteLn('plaintext : ', ULONG2Hex(testdata2_p[1]),
             ' ',            ULONG2Hex(testdata2_p[2]));
     Blowfish_ECBEncrypt(@testdata2_p[1], 8);
     WriteLn('ciphertext: ', ULONG2Hex(testdata2_p[1]),
             ' ',            ULONG2Hex(testdata2_p[2]));
     WriteLn('DDJ 10/95 : ', ULONG2Hex(testdata2_c[1]),
             ' ',            ULONG2Hex(testdata2_c[2]));
     Blowfish_ECBDecrypt(@testdata2_p[1], 8);
     WriteLn('decrypted : ', ULONG2Hex(testdata2_p[1]),
             ' ',            ULONG2Hex(testdata2_p[2]));
     WriteLn(#13#10, 'press a key ...', #13#10);
     ReadKey;

     { test ECB routines, work with small buffer... }
     { 16 rounds }
     Blowfish_SetBoxes(@BF_key[1]);      { reload boxes }
     sMyKey:='Who wants some?';
     Blowfish_Init(@sMyKey[1], Length(sMyKey));
     TextColor(10); WriteLn('ECB test, 16 rounds...'); TextColor(7);
     WriteLn('original data:');
     { generate and show random numbers...}
     for unI:=1 to 6 do begin
       testbuf[unI]:=Random32;
       Write(ULONG2Hex(testbuf[unI]), '  ');
     end;
     WriteLn;
     WriteLn('encrypted data:');
     Blowfish_ECBEncrypt(@testbuf[1], 6*4);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('decrypted data:');
     Blowfish_ECBDecrypt(@testbuf[1], 6*4);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     { 32 rounds }
     Blowfish_SetRounds(32);      { set number of encryption rounds to 32 }
     Blowfish_SetBoxes(@BF_key[1]);      { reload boxes }
     sMyKey:='Who wants some?';
     Blowfish_Init(@sMyKey[1], Length(sMyKey));
     TextColor(10); WriteLn('ECB test, 32 rounds...'); TextColor(7);
     WriteLn('original data:');
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('encrypted data:');
     Blowfish_ECBEncrypt(@testbuf[1], 6*4);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('decrypted data:');
     Blowfish_ECBDecrypt(@testbuf[1], 6*4);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn; WriteLn;

     WriteLn(#13#10, 'press a key...', #13#10);
     ReadKey;

     { now test the CBC routines... }
     { 16 rounds }
     Blowfish_SetRounds(16);
     sMyKey:='Damn, I''m looking good!';
     TextColor(10); WriteLn('CBC test, 16 rounds...'); TextColor(7);
     ulCBCLeft:=Random32;     { create an IV "server" }
     ulCBCRight:=Random32;
     ulCBCIVL:=ulCBCLeft;
     ulCBCIVR:=ulCBCRight;
     WriteLn('IV: ', ULONG2Hex(ulCBCIVL),
             ' ',    ULONG2Hex(ulCBCIVR));
     WriteLn('original data:');
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('encrypted data:');
     Blowfish_SetBoxes(@BF_key[1]);    { reload boxes }
     Blowfish_Init(@sMyKey[1], Length(sMyKey));
     Blowfish_CBCEncrypt(@testbuf[1], 6*4, ulCBCIVL, ulCBCIVR);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('decrypted data :');
     { just do some overhead for provocation :) }
     sMyKey:='Damn, I''m looking good!Damn, I''m looking good!';
     Blowfish_SetBoxes(@BF_key[1]);
     Blowfish_Init(@sMyKey[1], Length(sMyKey));
     { decrypt in 2 parts to double check the TPU }
     ulCBCIVL:=ulCBCLeft;
     ulCBCIVR:=ulCBCRight;
     Blowfish_CBCDecrypt(@testbuf[1], 2*4, ulCBCIVL, ulCBCIVR);
     Blowfish_CBCDecrypt(@testbuf[3], 4*4, ulCBCIVL, ulCBCIVR);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     { 32 rounds }
     Blowfish_SetRounds(32);
     ulCBCIVL  :=ulCBCLeft;
     ulCBCIVR  :=ulCBCRight;
     TextColor(10); WriteLn('CBC test, 32 rounds...'); TextColor(7);
     WriteLn('IV: ', ULONG2Hex(ulCBCIVL),
             ' ',    ULONG2Hex(ulCBCIVR));
     WriteLn('original data :');
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('encrypted data :');
     Blowfish_SetBoxes(@BF_key[1]);      { reload Boxes }
     sMyKey:='Damn, I''m looking good!';
     Blowfish_Init(@sMyKey[1], Length(sMyKey));
     Blowfish_CBCEncrypt(@testbuf[1], 6*4, ulCBCLeft, ulCBCRight);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn;
     WriteLn('decrypted data :');
     Blowfish_SetBoxes(@BF_key[1]);      { reload Boxes }
     Blowfish_Init(@sMyKey[1], Length(sMyKey));
     { decrypt in 2 parts to double check the correct reentry }
     Blowfish_CBCDecrypt(@testbuf[1], 2*4, ulCBCIVL, ulCBCIVR);
     Blowfish_CBCDecrypt(@testbuf[3], 4*4, ulCBCIVL, ulCBCIVR);
     for unI:=1 to 6 do Write(ULONG2Hex(testbuf[unI]), '  ');
     WriteLn; WriteLn;

    { benchmark, using CBC and a 32kB test buffer }
    New(biggy);
    TextColor(10); WriteLn('performance with 16 rounds...'); TextColor(7);
    Blowfish_SetRounds(16);
    GetTime(h,m,s,c);
    ulStart:=ULONG(h)*3600*100 + ULONG(m)*60*100 + ULONG(s)*100 + ULONG(c);
    for unI:=1 to TESTLOOPS do
      Blowfish_CBCEncrypt(@biggy^[1], BIGBUFSIZE, ulCBCLeft, ulCBCRight);
    GetTime(h,m,s,c);
    ulTime:=(ULONG(h)*3600*100 + ULONG(m)*60*100 + ULONG(s)*100 + ULONG(c))-ulStart;
    ulRate:=((TESTLOOPS*BIGBUFSIZE) div ulTime) * 100;
    WriteLn(ulRate,  ' bytes per second');

    TextColor(10); WriteLn('performance with 32 rounds...'); TextColor(7);
    Blowfish_SetRounds(32);
    GetTime(h,m,s,c);
    ulStart:=ULONG(h)*3600*100 + ULONG(m)*60*100 + ULONG(s)*100 + ULONG(c);
    for unI:=1 to TESTLOOPS do
      Blowfish_CBCEncrypt(@biggy^[1], BIGBUFSIZE, ulCBCLeft, ulCBCRight);
    GetTime(h,m,s,c);
    ulTime:=(ULONG(h)*3600*100 + ULONG(m)*60*100 + ULONG(s)*100 + ULONG(c))-ulStart;
    ulRate:=((TESTLOOPS*BIGBUFSIZE) div ulTime) * 100;
    WriteLn(ulRate,  ' bytes per second');

    Dispose(biggy);


    { check the weak key detector... }
    TextColor(10); WriteLn(#13#10+'weak key check...'); TextColor(7);

    Blowfish_SetRounds(16);
    Blowfish_GetBoxes(@BF_key);
    Blowfish_SetRounds(32);
    Blowfish_GetBoxes(@BF_keysave);

    Write('16 rounds: ', Blowfish_WeakKey);
    Blowfish_SetRounds(16);
    ulEqual:=Random32;
    BF_key[18+Random(1024)]:=ulEqual;
    BF_key[18+Random(1024)]:=ulEqual;
    Blowfish_SetBoxes(@BF_key);
    WriteLn(' ', Blowfish_WeakKey);

    Blowfish_SetRounds(32);
    Blowfish_SetBoxes(@BF_keysave);
    Write('32 rounds: ', Blowfish_WeakKey);
    ulEqual:=Random32;
    BF_keysave[18+Random(1024)]:=ulEqual;
    BF_keysave[18+Random(1024)]:=ulEqual;
    Blowfish_SetBoxes(@BF_keysave);
    WriteLn(' ', Blowfish_WeakKey);

    { do something funny,
      encrypt the screen... }

    WriteLn(#13#10,
            'We will now encrypt your video memory. If the flicker has'+#13#10,
            'stopped you must press a key to decrypt/restore it again.'+#13#10,
            'You''ll also saw some equal parts, a good example _not_ to'+#13#10,
            'use ECB but CBC :)'+#13#10#13#10,
            'press a key...');
    ReadKey;
    Blowfish_SetRounds(16);
    { assume 80x25 color textmode }
    pScreen:=Ptr($b800, 0);
    { Blowfish is so fast we do some extra loops for better animation here,
      not necessary in the real world, of course }
    for unI:=1 to SCRLOOPS do Blowfish_ECBEncrypt(pScreen, 4000);
    ReadKey;
    for unI:=1 to SCRLOOPS do Blowfish_ECBDecrypt(pScreen, 4000);
    WriteLn(#13#10+'That''s all folks!');


end.
