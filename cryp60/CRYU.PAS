{Start of file CryU.PAS ****************************************************}

program CryU;

uses Crt, Prompt, CryUHelp;

{***************************************************************************}
{
 Name  = 'CryU - A data encryption system written in Pascal, user interface';
 Version = 'Version 6.00, 1992-11-15, 1400 hours';
 Author  = 'Copyright (c) 1987-1992 by author: Harry J. Smith';
 Address = '19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.';
}
{ Actual Name, Version, Author, Address maintained in CryUHelp.Pas. }

{***************************************************************************}

{This program will encipher or decipher a file of data and rewrite
  it into a new file}

{$I- do our own i/o error checks}
{$R- Range check Array Indexes}
{$V- Allow any length Strings passed to procedures}

label Continue, Exit1;

const
  LastLine   = 25;    {Used in GoToXY to go to last line of CRT, 25 for PC}
  FirstCol   = 1;     {Used in GoToXY to go to first column of CRT}
  BufRecSize = 16384; {Number of 1-byte records in a buffer}
  BufIntSize = 8192;  {Integers in buffer, BufRecSize / 2}
  BufBytSize = 16384; {Bytes in buffer, 1 * BufRecSize}
  ConSize    = 18;    {Number of characters in CRY.CON file}
  PoolSize   = 2048;  {Integers in random number Pool, = 2**N}
  PoolMask   = 2047;  {Mask for index to random number Pool, = PoolSize - 1}
  NameN      = 32;    {Max number of characters in a file name}

type
  Buffer = record case Integer of  {I/O buffers}
             1: (Int : packed array[1..BufIntSize] of Integer);
             2: (BYT : packed array[1..BufBytSize] of Char);
           end;

var
  RI       : array [0..8] of Integer; {Current random Int. from 9 generators}
  RS       : array [0..8] of Integer; {Initial random seeds for 9 generators}
  RC       : array [0..8] of Integer; {Current value of clear continue seeds}
  Pool     : array [0..PoolMask] of Integer; {Pool of random integers}
  RR       : Integer;   {Random Int. running combination of RI[0] thru RI[8]}
  Next     : Integer;   {Random Int. running combination of RI[0] thru RI[8]}
  InName   : String[NameN];  {Input file name}
  OutName  : String[NameN];  {Output file name}
  Ans      : String[NameN];  {Operator's answer}
  TempName : String[NameN];  {Temporary name of output file "d:ffffffff.$$$"}
  DDrive   : String[2];   {Default drive, e.g. B:}
  AnsKey   : String[72];  {Current input key string}
  LastKey  : String[72];  {Previous input key string}
  Key      : String[24];  {Input key string in standard form}
  InFile   : file;        {Input file}
  OutFile  : file;        {Output file}
  ConFile  : file;        {Continuation file 'CRY.CON'}
  KeyFile  : text;        {A text file with a one line key}
  TBytes   : Real;        {Total bytes processed}
  FError   : Boolean;     {File conversion error flag}
  Exists   : Boolean;     {Output file already exists flag}
  Encipher : Boolean;     {Encipher a file, False => Decipher}
  InitCon  : Boolean;     {Initializing CRY.CON, False => Encipher/Decipher}
  Repeat2  : Boolean;     {Repeat flag used in RN(2)}
  ReadRecs : LongInt;     {No. of 1-byte records to read this read}
  RemRecs  : LongInt;     {No. of 1-byte records remaining in file}
  InCnt    : LongInt;     {# of Bytes read}
  BufIO    : Buffer;      {I/O Buffer}
  KeyI     : array [1..24] of Integer;  {Key in Integer form, 6 bits each}

{--------------------------------------}
function RN(N : Integer): Integer;
  {Returns the next random integer from RI[I], 0 <= N <= 8}
  label Exit2;
  var I, RT : Integer;
begin
  case N of
    0: {1st Congruential Generator, 16 bits}
       {Random integer from -32768 to 32767 inclusive}
       {Cycle length = 65536 = 2^16}
       begin
         RI[0]:= 25173 * RI[0] + 6925;
         RN:= RI[0];
       end;

    1: {1st Shift-register Generator, 16 bits}
       {Random integer from -32768 to 32767 inclusive, zero not generated}
       {Generator = -22620, Cycle length = 65535 = 3 * 5 * 17 * 257}
       begin
         if Odd( RI[1]) then
           RI[1]:= (RI[1] SHR 1) XOR -22620
         else
           RI[1]:= RI[1] SHR 1;
         RN:= RI[1];
       end;

    2: {2nd Congruential Generator, 16 bits}
       {Random integer from -32768 to 32767 inclusive}
       {Cycle length = 65537 = prime, zero repeats once}
       begin
         if RI[2] = 0 then begin
           if Repeat2 then begin
             Repeat2:= False;
             goto Exit2;
           end else
             Repeat2:= True;
         end;
         RI[2]:= 23629 * RI[2] + 13849;
Exit2:
         RN:= RI[2];
       end;

    3: {2nd Shift-register Generator, 16 bits}
       {Random integer from -32768 to 32767, not all generated}
       {Generator = -07493, Cycle length = 65521 = prime, (65535 - 15)}
       begin
         if Odd( RI[3]) then
           RI[3]:= (RI[3] SHR 1) XOR - 7493
         else
           RI[3]:= RI[3] SHR 1;
         if RI[3] = 1 then
           for I := 1 to 14 do  RT:= RN(3);  {Throw 14 away}
         RN:= RI[3];
       end;

    4: {3rd Congruential Generator, 16 bits}
       {Random integer from -32768 to 32767, not all generated}
       {Cycle length = 65519 = prime, (65536 - 17)}
       begin
         RI[4]:= 4821 * RI[4] + 13001;
         if RI[4] = 0 then
           for I:= 1 to 17 do  RT:= RN(4);  {Throw 17 away}
         RN:= RI[4];
       end;

    5: {3rd Shift-register Generator, 16 bits}
       {Random integer from -32768 to 32767, not all generated}
       {Generator = -25501, Cycle length = 65497 = prime, (65535 - 39)}
       begin
         if Odd( RI[5]) then
           RI[5]:= (RI[5] SHR 1) XOR -25501
         else
           RI[5]:= RI[5] SHR 1;
         if RI[5] = 1 then
           for I:= 1 to 38 do  RT:= RN(5);  {Throw 38 away}
         RN:= RI[5];
       end;

    6: {4th Congruential Generator, 16 bits}
       {Random integer from -32768 to 32767, not all generated}
       {Cycle length = 65479 = prime, (65536 - 57)}
       begin
         RI[6]:= 10349 * RI[6] + 7001;
         if RI[6] = 0 then
           for I:= 1 to 57 do  RT:= RN(6);  {Throw 57 away}
         RN:= RI[6];
       end;

    7: {4th Shift-register Generator, 16 bits}
       {Random integer from -32768 to 32767, not all generated}
       {Generator = -18916, Cycle length = 65449 = prime, (65535 - 87)}
       begin
         if Odd( RI[7]) then
           RI[7]:= (RI[7] SHR 1) XOR -18916
         else
           RI[7]:= RI[7] SHR 1;
         if RI[7] = 1 then
           for I:= 1 to 86 do  RT:= RN(7);  {Throw 86 away}
         RN:= RI[7];
       end;

    8: {5th Congruential Generator, 16 bits}
       {Random integer from -32768 to 32767, not all generated}
       {Cycle length = 65447 = prime, (65536 - 89)}
       begin
         RI[8]:= 30133 * RI[8] + 14001;
         if RI[8] = 0 then
           for I:= 1 to 89 do  RT:= RN(8);  {Throw 89 away}
         RN:= RI[8];
       end;
  end; {case N}
end; {RN}

{--------------------------------------}
function RC9 : Integer;  {Combination of RN(0) thru RN(8) one each}
  var I, RC : Integer;
begin
  RC:= 0;
  for I:= 0 to 8 do
    RC:= RC + RN(I);
  RC9:= RC;
end; {RC9}

{--------------------------------------}
procedure ChangeIt( var BufE : Buffer; Bytes : Integer);
  {Apply pseudo key to Bytes bytes in place in BufE}
  var I, Ind : Integer;
begin
       {This is the heart of the encryption method}

  with BufE do begin
    for I:= 1 to ((Bytes+1) DIV 2) do begin
      Ind:= RR AND PoolMask;
      Int[I]:= Int[I] XOR Pool[Ind];
      RR:= RR + RN( Next);
      Pool[Ind]:= Pool[Ind] XOR RR;
      Next:= Next + 1;
      if Next = 9 then  Next:= 0;
    end;
  end;
end; {ChangeIt}

{--------------------------------------}
procedure ConvKey;  {Convert the encryption key to 9 seeds}
                    { Input: Key[J], J = 1, ..., 24}
                    { Output: RS[I], I = 0, ..., 8}
var I, J, K : Integer;
{
    [0]    [1]    [2]    [3]    [4]    [5]    [6]    [7]  ...   [22]   [23]
  111111 111111 111111 111111 111111 111111 111111 111111      111111 111111
     6      6     4  2    6      6    2  4     6      6           6      6
  ****** seed0 *****...... seed1 ......***** seed2 ****** ... * seed8 ******
        16 bits           16 bits           16 bits            16 bits

  144 bit key converted to 144 bits of seed, seeds of zero are changed to 1.
}
begin
  for I:= 1 to 24 do  KeyI[I]:= Ord( Key[I]) AND 63;
  I:= 0;  J:= 1;
  for K:= 0 to 2 do begin
    RS[I]:=   (KeyI[J]   SHL 10) OR (KeyI[J+1] SHL 4) OR (KeyI[J+2] SHR 2);

    RS[I+1]:= (KeyI[J+2] SHL 14) OR (KeyI[J+3] SHL 8) OR (KeyI[J+4] SHL 2) OR
              (KeyI[J+5] SHR 4);

    RS[I+2]:= (KeyI[J+5] SHL 12) OR (KeyI[J+6] SHL 6) OR KeyI[J+7];
        I:= I + 3;  J:= J + 8;
  end;
  Repeat2:= True;
end; {ConvKey}

{--------------------------------------}
procedure Standard( var Key : String);  {Convert a key to Standard format}
  var I : Integer;  {Utility index}
begin
  for I:= 1 to Length( Key) do begin
    Key[I]:= Chr( Ord( Key[I]) AND 127);
    if Ord( Key[I]) > 95 then  Key[I]:= Chr( Ord( Key[I]) - 32);
    if Ord( Key[I]) < 32 then  Key[I]:= Chr( Ord( Key[I]) + 32);
    if Key[I] = ' ' then  Key[I]:= '/';
  end;
end; {Standard}

{--------------------------------------}
function TestStop : Boolean;  {Test for user quit request}
  var Quit : Boolean;
begin
  TestStop:= False;
  if StopTyped then begin
    WriteLn;
    Write('Do you wish to Quit the program ? (Y/N): ');
    GetYesNo( Quit, HotHelp);
    WriteLn;
    TestStop:= Quit;
    StopTyped:= True;
  end;
end; {TestStop}

{--------------------------------------}
procedure GetKey;  {Get encryption key from operator}
  label Exit3;
  var  I,J  : Integer;  {Utility indexes}
       Done : Boolean;  {Utility done flag}
       Ch   : Char;     {Utility character}
begin
  WriteLn;
  Done:= False;
  repeat
    WriteLn;
    AnsKey:= LastKey;
    repeat
      WriteLn(
'Input a key of 0 to 72 characters that you can remember and press return:');
      WriteLn('Or a key file name (F1 for help, F2 to Exit)   .<-- col. 48');
      Prom( FirstCol, LastLine, 72, LastKey, AnsKey, HotHelp);
      if TestStop then  goto Exit3;
    until NOT StopTyped;
    Assign (KeyFile, AnsKey);  { Try to open key file }
    Reset( KeyFile);
    if (IOResult = 0) AND (AnsKey <> '') then begin
      Read( KeyFile, AnsKey);
      Close( KeyFile);
    end;
    LastKey:= AnsKey;
    WriteLn;
    ChgLen( Key, 24, 24);  {24 long}
    Standard( AnsKey);
    WriteLn( AnsKey, ' = Input key in standard form');

    if Length( AnsKey) > 24 then begin
      for I:= 2 to 24 do
        AnsKey[I]:= Chr( Ord( AnsKey[I]) + Ord( AnsKey[I-1]));
      J:= Length( AnsKey);
      for I:= 26 to Length( AnsKey) do begin
        J:= J-1;
        AnsKey[J]:= Chr( Ord( AnsKey[J]) + Ord( AnsKey[J+1]));
      end;
    end;

    for I:= 1 to 24 do  Key[I]:= Chr(0);
    J:= 1;
    for I:= 1 to Length( AnsKey) do begin
      Key[J]:= Chr( Ord( Key[J]) + Ord( AnsKey[I]));
      J:= J MOD 24 + 1;
    end;
    for I:= 1 to 24 do
      if (Key[I] = Chr(0)) then  Key[I]:= ' ';
    Standard( Key);
    ConvKey;
    WriteLn( Key, ' = 24-character key in standard form');
    WriteLn;
    Write('Is this key what you want? (Y/N): ');
    GetYesNo( Done, HotHelp);
    if NOT Done then  WriteLn('Key rejected');
  until Done;
  WriteLn('Key accepted');
Exit3:
end; {GetKey}

{--------------------------------------}
procedure GetTempName;  {Generate a temporary name for the output file}
  var  I,J : Integer;   {Utility indexes}
begin
  if Exists then begin
    ChgLen( TempName, NameN, NameN);  {NameN bytes long}
    for I:= 1 to NameN do  TempName[I]:= ' ';  {All blanks}
    for I:= 1 to Length( OutName) do  TempName[I]:= OutName[I];
    J:= 0;
    for I:= 1 to NameN do begin
      if (TempName[I] = '.') OR (TempName[I] = ' ') then
        if J = 0 then  J:= I;
    end;
    if J > (NameN - 3) then  J:= NameN - 3;
    Insert('.$$$', TempName, J);
  end else
    TempName:= OutName;
end; {GetTempName}

{--------------------------------------}
procedure FixName;  {Add default drive d: to file name or set default drive}
begin
  if (Length( Ans) < 2) OR (Ans[2] <> ':') then
    Ans:= DDrive + Ans;
  if Length( Ans) = 2 then begin
    DDrive:= Ans;
    Ans:= '';
  end;
end; {FixName}

{--------------------------------------}
procedure GetInName;  {Get name of input file and open for reading}
  label Exit4;
  var Done : Boolean;  {Utility done flag}
begin
  Done:= False;
  repeat
    repeat
      WriteLn;
      Ans:= InName;
      repeat
        Write('Default Drive = ', DDrive, '  Enter name of Input file: ');
        Prom( FirstCol+47, LastLine, NameN, InName, Ans, HotHelp);
        if TestStop then  goto Exit4;
      until NOT StopTyped;
      FixName;
      InName:= Ans;
    until (Length( InName) > 0);
    WriteLn;
    Assign( InFile, InName);
    Reset( InFile, 1);
    if IOResult = 0 then begin
      RemRecs:= FileSize( InFile);
      if RemRecs = 0
        then  WriteLn('File "', InName, '" has zero length')
        else  Done:= True;
    end else
      WriteLn('File "', InName, '" not found');
  until Done;
Exit4:
end; {GetInName}

{--------------------------------------}
procedure GetOutName;  {Get name of output file and open for rewrite}
  label Exit5;
  var Done : Boolean;  {Utility done flag}
begin
  Done:= False;
  repeat
    repeat
      WriteLn;
      Ans:= OutName;
      repeat
        Write('Default Drive = ', DDrive, '  Enter name of Output file: ');
        Prom( FirstCol+47, LastLine, NameN, Ans, Ans, HotHelp);
        if TestStop then  goto Exit5;
      until NOT StopTyped;
      FixName;
      OutName:= Ans;
    until (Length( OutName) > 0);
    WriteLn;
    Assign( OutFile, OutName);
    Reset( OutFile, 1);
    Exists:= False;
    if IOResult = 0 then begin
      Exists:= True;
      Write('File "', OutName,
        '" already exists, erase it after conversion? (Y/N): ');
      GetYesNo( Done, HotHelp);
      if TestStop then  goto Exit5;
    end else
      Done:= True;
    if Done then begin
      GetTempName;
      Assign( OutFile, TempName);
      Rewrite( OutFile, 1);
      if IOResult <> 0 then begin
        WriteLn('Cannot open output file');
        Done:= False;
      end;
    end;
  until Done;
Exit5:
end; {GetOutName}

{--------------------------------------}
procedure GetEDI( var Value : Char;
    HotHelpA : HotHelpT);  {Get E/D/I answer from operator}
  var Ch : Char;  Done : Boolean;
begin
  Write('E', BS);
  repeat
    Ch:= ReadKeyM( HotHelpA);
    Done:= Ch in ['E', 'e', 'D', 'd', 'I', 'i', CR, ESC, StopC];
  until Done;
  EscTyped:= (Ch = ESC);
  StopTyped:= (Ch = StopC);
  if StopTyped then  Ch:= ' ';
  Value:= UpCase( Ch);
  if (Value in [CR, ESC]) then  Value:= 'E';
  WriteLn( Value);
end; {GetEDI}

{--------------------------------------}
procedure EncOrDec;  {Ask operator for Encipher or Decipher option}
  var Ch : Char;  Done : Boolean;  Ans : String[1];
begin
  repeat
    WriteLn;
    Ans:= 'E';
    Write('Encipher, Decipher or Init File CRY.CON? (E/D/I): ');
    GetEDI( Ch, HotHelp);
    if TestStop then  Exit;
  until NOT StopTyped;
  InitCon:=  Ch IN ['I', 'i'];
  Encipher:= Ch IN ['E', 'e', CR];
  if InitCon then  WriteLn('Init CRY.CON')
  else if Encipher then  WriteLn('Encipher')
  else if Ch IN ['D', 'd'] then  WriteLn('Decipher');
end; {EncOrDec}

{--------------------------------------}
procedure WriteIt;  {Write a multiple of 1 byte records}
begin
  BlockWrite( OutFile, BufIO, ReadRecs);
  if IOResult <> 0 then begin
    WriteLn('Unable to write output file');
    FError:= True;
  end;
end; {WriteIt}

{--------------------------------------}
procedure InitEn;  {Initialize for enciphering}
  var RecS : Integer;
begin
  Assign( ConFile, 'CRY.CON');
  Reset( ConFile, 1);
  if IOResult = 0 then begin
    RecS:= FileSize( ConFile);
    if RecS <> ConSize then begin
      WriteLn('File CRY.CON not of proper length');
      FError:= True;
    end;
  end
  else begin
    WriteLn('File CRY.CON not found');
    FError:= True;
  end;
  if NOT FError then begin
    BlockRead( ConFile, Pool, ConSize);
    if IOResult <> 0 then begin
      WriteLn('Can not read file CRY.CON');
      FError:= True;
    end
    else begin
      BlockWrite( OutFile, Pool, ConSize);
      if IOResult <> 0 then begin
        WriteLn('Unable to write 1st record of output file');
        FError:= True;
      end;
    end;
  end;
  Close( ConFile);
  if IOResult <> 0 then begin
    WriteLn('Can close file CRY.CON');
    FError:= True;
  end;
end; {InitEn}

{--------------------------------------}
procedure InitDe;  {Initialize for deciphering}
begin
  RemRecs:= RemRecs - ConSize;
  if RemRecs = 0 then begin
    WriteLn('File "', InName, '" is too short for an enciphered file');
    FError:= True;
  end
  else begin
    BlockRead( InFile, Pool, ConSize);
    if IOResult <> 0 then begin
      WriteLn('Unable to read 1st record of input file');
      FError:= True;
    end;
  end;
end; {InitDe}

{--------------------------------------}
procedure WriteCon;  {Write CRY.CON to disk}
  var I : Integer;
begin
  Assign( ConFile, 'CRY.CON');
  Rewrite( ConFile, 1);
  if IOResult <> 0 then begin
    WriteLn('Can not open file CRY.CON');
    FError:= True;
  end
  else begin
    for I:= 0 to 8 do
      RI[I]:= RC[I];
    for I:= PoolSize-1 downto 0 do
      Pool[I]:= RC9;
    BlockWrite( ConFile, Pool, ConSize);
    if IOResult <> 0 then begin
      WriteLn('Unable to write file CRY.CON');
      FError:= True;
    end;
    Close( ConFile);
    if IOResult <> 0 then begin
      WriteLn('Can close file CRY.CON');
      FError:= True;
    end;
  end;
  if FError then
    Write('File CRY.CON update Aborted');
end; {WriteCon}

{--------------------------------------}
procedure InitPool;  {Initialize pool of random integers}
  var I : Integer;
begin
  for I:= 0 to 8 do begin
    RC[I]:= Pool[I];
    RI[I]:= RS[I] XOR Pool[I];
    if RI[I] = 0 then  RI[I]:= 1;
  end;
  RR:= RC9;
  for I:= 0 to PoolSize-1 do begin
    RR:= RR + RN( Next);
    Pool[I]:= RR;
    Next:= Next + 1;
    if Next = 9 then  Next:= 0;
  end;
  RR:= RC9;
end; {InitPool}

{--------------------------------------}
procedure DoInitCon;  {Initialize file CRY.CON}
  var I  : Integer;
      Ch : Char;
begin
  FError:= False;
  Assign( ConFile, 'CRY.CON');
  Rewrite( ConFile, 1);
  if IOResult <> 0 then begin
    WriteLn('Can not open file CRY.CON');
    FError:= True;
  end
  else begin
    for I:= 0 to 8 do
      RI[I]:= RS[I];
    for I:= PoolSize-1 downto 0 do
      Pool[I]:= RC9;
    BlockWrite( ConFile, Pool, ConSize);
    if IOResult <> 0 then begin
      WriteLn('Unable to write file CRY.CON');
      FError:= True;
    end;
    Close( ConFile);
    if IOResult <> 0 then begin
      WriteLn('Can close file CRY.CON');
      FError:= True;
    end;
  end;
  if FError then
    Write('File CRY.CON initialization Aborted')
  else
    Write('File CRY.CON initialized successfully');
  WriteLn(', hit any key to continue ...');
  Ch:= ReadKey;
end; {DoInitCon}

{--------------------------------------}
procedure EraseKey;  {Erase all trace of the encryption key}
  var I : Integer;
begin
  for I:= 1 to 72 do  AnsKey[I]:= ' ';
  for I:= 1 to 24 do begin
    Key[I]:= ' ';  KeyI[I]:= 0;
  end;
  for I:= 0 to 8  do begin
    RI[I]:= 0;  RS[I]:= 0;
  end;
  for I:= 0 to PoolSize-1 do
    Pool[I]:= 0;
  RR:= 0;
end; {EraseKey}

{--------------------------------------}
procedure Process;  {Process input file to output file}
  var Ans : String[1];  {Utility operator's answer}
      Ch  : Char;       {Utility character}
      I   : Integer;
begin
  TBytes:= 0;
  FError:= False;
  if Encipher
    then  InitEn
    else  InitDe;
  if NOT FError then begin
    InitPool;
{DIAG
for I:= 0 to 8 do  Write('RS[', I, '] = ', RS[I], '  ');
for I:= 0 to 8 do  Write('RI[', I, '] = ', RI[I], '  ');}
    repeat
{DIAG  WriteLn('DIAG: RemRecs = ', RemRecs);}
      if BufRecSize < RemRecs
        then  ReadRecs:= BufRecSize
        else  ReadRecs:= RemRecs;
      BlockRead( InFile,  BufIO, ReadRecs);
      if IOResult = 0 then begin
        InCnt:= 1 * ReadRecs;
        TBytes:= TBytes + InCnt;
        ChangeIt( BufIO, InCnt);  {Apply pseudo key}
        WriteIt;
        RemRecs:= RemRecs - ReadRecs;
      end
      else begin
        WriteLn('Unable to read input file');
        FError:= True;
      end;
      WriteLn( TBytes:8:0, ' Bytes Processed');
    until (RemRecs = 0) OR FError;
  end;
  Close( OutFile);
  if IOResult <> 0 then begin
    WriteLn('Unable to close output file');
    FError:= True;
  end;
  Close( InFile);
  if IOResult <> 0 then  WriteLn('Unable to close input file');
  if FError then begin
    if Exists then  Erase( OutFile);
    Write('File conversion Aborted');
  end
  else begin
    if Exists then begin
      Assign( OutFile, OutName);
      Erase( OutFile);
      Assign( OutFile, TempName);
      Rename( OutFile, OutName);
    end;
    WriteCon;
    Write('File converted successfully');
  end;
  WriteLn(', hit any key to continue ...');
  Ch:= ReadKey;
end; {Process}

{--------------------------------------}
procedure Init;  {Initialize program only once}
  var I : Integer;  {Utility index}
begin
  TextBackground( Blue);
  TextColor( Yellow);
  ClrScr;
  GoToXY( FirstCol, LastLine);
  Title;
  WriteLn;
  LastKey:= 'ABCDEF';
  DDrive:=  'A:';
  InName:=  'B:';
  OutName:= 'B:';
  WriteLn;
  IndLn(2,
 'Select characters for your key from the following list of 64 characters:');
  WriteLn; Write(' ':29);
  for I:= 0 to 15 do  Write( Chr( I+64));  WriteLn; Write(' ':29);
  for I:= 0 to 15 do  Write( Chr( I+80));  WriteLn; Write(' ':29);
  for I:= 0 to 15 do  Write( Chr( I+32));  WriteLn; Write(' ':29);
  for I:= 0 to 15 do  Write( Chr( I+48));  WriteLn;
  WriteLn; IndLn(2,
'Your key will be converted to a 24-character key in standard form.'
  ); IndLn(2,
'The standard form is then used to compute seeds for the nine different'
  ); IndLn(2,
'random number generators used to produce the pseudo infinite key.'
  ); IndLn(2,
'The generated pseudo infinite key will not repeat itself until after it is'
  ); IndLn(2,
'about 2**148 bits long.'
  );
end; {Init}

{--------------------------------------}
begin {Main program (CryU)}
  Init;
  repeat
    Next:= 0;
    GetKey;     if StopTyped then  goto Exit1;
    EncOrDec;   if StopTyped then  goto Exit1;
    if InitCon then  DoInitCon
    else begin
      InName:= OutName;  {Default InName is last OutName}
      GetInName;         if StopTyped then  goto Exit1;
      OutName:= InName;  {Default OutName is last InName}
      GetOutName;        if StopTyped then  goto Exit1;
      Process;
    end;
    Continue:
  until False;  {F2 to stop}
  Exit1:
  EraseKey;
end. {Main program (CryU)}

{End of file CryU.PAS ******************************************************}
