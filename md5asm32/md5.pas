{ =========================================================================}
{ MD5 Implementation for Turbo Pascal (requires 386 or greater machines)   }
{ =========================================================================}
{                                                                          }
{ This source code is (c) Copyright 1994, Robert Rothenburg Walking-Owl.   }
{                                         <rrothenb@ic.sunysb.edu>         }
{                                                                          }
{ These sources are "original", based entirely on the MD5-A Internet Draft }
{ (10 July 1991) descriptions of the algorithm.  Please read that document }
{ for more information of MD5 specifications, et cetera.                   }
{                                                                          }
{ License: this code, as well as any derivative code, may be used only for }
{ personal and/or non-commercial purposes.  Modified versions of this code }
{ may not be distributed without the author's permission.                  }
{                                                                          }
{ Any software (including compiled executables) containing this code may   }
{ only be distributed as free software--that is, no fees of any kind are   }
{ to be asked of users of that software.                                   }
{                                                                          }
{ This source code must be distributed as-is, with no modifications.       }
{                                                                          }
{ The author makes no guarantees or warrantees of any kind about this code.}
{ Use at your own risk.  The author will not be held accountable for any   }
{ damages resulting from the use or mis-use of this software, etc.         }
{                                                                          }
{ =========================================================================}

unit MD5; { AFAIK, this code should work for Turbo Pascal 4.0 and above }

interface

type
  Digest = Array [ 0..15 ] of Byte;
{ Actually, Digest = Array [ 0..3 ] of LongInt }

{ Returns the Cpu the system is running 0 = 86..186 2 = 286 3 = 386 4=486 }
{ Since these routines *do* require a 386 machine, it'd be smart to test! }
function GetCpu: Word; far;

procedure MD5Init(var Hash); far;
{ Initializes the Hash/Digest value prior to calculating MD5 transforms }

procedure MD5Hash(var Data, Hash); far;
{ Hashes on MD5 transform on the Data[64] }

procedure MessageMD5(var Message; Size: Word; var Hash);
{ Calculates the MD5 for the data in Message (0 through 65455 bytes) }

procedure FileMD5(var f: file; var Hash);
{ Calculates the MD5 for a file (like MD5SUM or CHK-SAFE) }

implementation

function GetCpu: Word;
  external; {$L GetCpu.Obj}

procedure MD5Init(var Hash);
  external; {$L MD5.Obj}

procedure MD5Hash(var Data, Hash);
  external;

procedure MessageMD5(var Message; Size: Word; var Hash);
{ Note: whatever space is allocated for Message, make sure there is room }
{ to add an extra 512 bits (64 bytes) of padding!                        }
var
  Buffer:       Array [ 0..65520 ] of Byte absolute Message;
  Index:        Word;
  Data:        ^Byte;
  Count:        LongInt;
begin
  MD5Init(Hash);
  Count := LongInt(Size ShL 3); { Count = number bits in message }

{ Pad the data congurent to 448 mod 512 bits (56 mod 64 bytes). }
{ If the data is already congruent to 448, add 512 bits...      }
  Buffer[ Size ] := $80; Inc(Size);
  while (Size mod 64)<>56
    do begin
         Buffer[ Size ] := 0;
         Inc(Size);
       end;

{ Append the 64-bit count of the number of bits in the message (prior to }
{ padding). If message longer than 2^64 bits, use low quadword           }
  Move(Count,Buffer[Size],4); Inc(Size,4); Count := 0;
  Move(Count,Buffer[Size],4); Inc(Size,4);
  Index := 0;

  repeat
    Data := Addr(Buffer[Index]);
    MD5Hash(Data^,Hash);             { Hash 512 bit (64 byte) packets }
    Inc(Index,64);
  until Index = Size;
end;

procedure FileMD5(var f: file; var Hash);
{ =========================================================================}
{ Speed comparisons of various MD5 hashing utilities (using Intel 486/33): }
{                                                                          }
{ This routine (using 32-bit 386 code) on 1.9Mb of data:   aprox.   6 sec. }
{ MD5SUM (incl. with PGP 2.3a sources):                            15 sec. }
{ CHK-SAFE v2.01:                                                  52 sec. }
{ =========================================================================}
var
  Buffer:     Array [ 0..4159 ] of Byte;
  Index,
  BufferSize: Word;
  Data:       ^Byte;
  Finalized:  Boolean;
  DatTim,
  CountLo,
  CountHi:    LongInt;
begin
  { Assumes file is already open properly! }
  MD5Init(Hash);
  Finalized := False;
  CountLo := 0;
  CountHi := 0;
  repeat
    BlockRead(f,Buffer,4096,BufferSize);
    CountLo := CountLo + BufferSize;
    if BufferSize<>4096
      then begin
          Buffer[ BufferSize ] := $80; Inc(BufferSize);
          while (BufferSize mod 64)<>56
            do begin
               Buffer[ BufferSize ] := 0;
               Inc(BufferSize);
              end;
          CountHi := LongInt(CountLo ShR 29);
        { Warning: Turbo Pascal 7.0 may have problems shifting LongInts }
          CountLo := LongInt(CountLo ShL  3);
          Move(CountLo,Buffer[BufferSize],4); Inc(BufferSize,4);
          Move(CountHi,Buffer[BufferSize],4); Inc(BufferSize,4);
          Finalized := True;
        end;
    Index := 0;
    repeat
      Data := Addr(Buffer[Index]);
      MD5Hash(Data^,Hash);
      Inc(Index,64);
    until Index = BufferSize;
  until Finalized;
end;

end.
