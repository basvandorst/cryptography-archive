(***************************************************************************)
(* Program title : DES File Encryption routine                             *)
(* Written by    : C.Dunkley                                               *)
(***************************************************************************)
program DESFileEncrypt;

uses
  crt,dos;

{$I des.inc}                                       (* include DES routines *)

const
  MaxBuff = 6250*8;

var
  Buff     : array [1..MaxBuff] of byte;
  Size,Pos : longint;
  Fd       : file;
  TempKey  : string[16];
  Txt      : Byte8Type;
  Num      : word;
  CurH,CurL,L3,Num8,Off : integer;
(***************************************************************************)

(***************************************************************************)
(* Cursor control.                                                         *)
(***************************************************************************)
procedure Cursor(On : boolean);

var
  Regs : registers;

begin (* 1 Cursor *)
if not On then
  begin (* 2 *)
  Regs.ah:=3;
  Regs.bh:=1;
  intr($10,Regs);
  CurH:=Regs.ch;
  CurL:=Regs.cl;
  Regs.ah:=1;
  Regs.ch:=32;
  intr($10,Regs)
  end   (* 2 *)
else
  begin (* 2 *)
  Regs.ah:=1;
  Regs.bh:=1;
  Regs.ch:=CurH;
  Regs.cl:=CurL;
  intr($10,Regs)
  end   (* 2 *)
end;  (* 1 Cursor *)
(***************************************************************************)

(***************************************************************************)
(* Generate an error message and stop.                                     *)
(***************************************************************************)
procedure FatalERR(message : string);

begin (* 1 FatalERR *)
GotoXy(1,23);
HighVideo;
writeln('ERROR: ',message,' - program terminating');
LowVideo;
Cursor(true);
halt(0)                                               (* terminate program *)
end;  (* 1 FatalERR *)
(***************************************************************************)

(***************************************************************************)
(* Display help screen and terminate program.                              *)
(***************************************************************************)
procedure help;

begin (* 2 *)
writeln('         ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ',
        'ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿');
writeln('         ³ "Data Encryption Standard" ',
        'file protection utility. ³¿');
writeln('         ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ',
        'ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´³');
write('         ³          Command : ');
HighVideo;
write('des ');
LowVideo;
writeln('<mode> <key> <filename>      ³³');
writeln('         ³                      ',
        '                               ³³');
write('         ³<mode> .... ');
HighVideo;
write('e');
LowVideo;
write(' - Encipher, ');
HighVideo;
write('d');
LowVideo;
write(' - Decipher, or ');
HighVideo;
write('h');
LowVideo;
writeln(' - Help. ³³');
writeln('         ³<key> ..... Between 8 and ',
        '16 characters.             ³³');
writeln('         ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ',
        'ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ³');
writeln('          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ',
        'ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ');
GotoXy(1,24);
halt(0)                                               (* terminate program *)
end;  (* 2 *)
(***************************************************************************)

(***************************************************************************)
(* DES main:                                                               *)
(*                                                                         *)
(* Format : des <mode> <key> <filename>                                    *)
(*        : des h                                                          *)
(***************************************************************************)
begin (* 1 main *)
CheckBreak := false;
ClrScr;
LowVideo;
Cursor(false);
write('ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´');
HighVideo;
write('HyperDES File Encryption, Version 4.8');
LowVideo;
writeln('ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿');
write('ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´');
HighVideo;
write('By C.Dunkley');
LowVideo;
writeln('ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ');
GotoXy(5,3);
write('ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»');
GotoXy(5,4);
write('ºThe author accepts no responsibility for any loss of data or anyº');
GotoXy(5,5);
write('ÈÍ»other form of damage resulting from the use of this program!ÉÍ¼');
GotoXy(5,6);
write('  ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼');
GotoXy(13,17);
write('ÉÄ');
HighVideo;
write('REMEMBER!');
LowVideo;
write('ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ»');
GotoXy(13,18);
write('³If you find this program useful³');
HighVideo;
write('19 Angrave Close,');
LowVideo;
write('³');
GotoXy(13,19);
write('³then please consider sending a ³');
HighVideo;
write('  St.Anns,');
LowVideo;
write('       ³');
GotoXy(13,20);
write('³gift of');
HighVideo;
write(' œ10');
LowVideo;
write(' to the author at:  ³');
HighVideo;
write('    Nottingham.');
LowVideo;
write('  ³');
GotoXy(13,21);
write('ÈÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´');
HighVideo;
write('      NG3 3NE    ');
LowVideo;
write('³');
GotoXy(13,22);
write('                                ÈÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¼');
GotoXy(1,7);
if (paramstr(1)='h') or (paramstr(1)='H') or (paramstr(1)='') then help;
if paramcount<3 then FatalERR('Not enough parameters');
if paramcount>3 then FatalERR('Too many parameters');
if not ((paramstr(1)='E') or (paramstr(1)='e') or
        (paramstr(1)='D') or (paramstr(1)='d')) then
  FatalERR('Mode incorrectly specified (e or d)');
if length(paramstr(2))>16 then                            (* key too long? *)
  FatalERR('Key too long (max.16 chars)');
if length(paramstr(2))<8 then                             (* key too long? *)
  FatalERR('Key too short (min.8 chars)');
TempKey:=paramstr(2);
for L3:= length(paramstr(2))+1 to 16 do
  TempKey[L3]:=chr(0);
Off:=1;
for L3:=1 to 8 do
  begin (* 2 *)
  Txt[L3]:=(ord(TempKey[Off]) mod 15) shl 4+(ord(TempKey[Off+1]) mod 15);
  inc(Off,2)
  end;  (* 2 *)
InitDes;                                 (* initialise DES ordering arrays *)
InitKey(Txt);                                          (* set up the key *)
assign(Fd,paramstr(3));
{$I-}
reset(Fd ,1);                                           (* open input file *)
if IOResult <>0 then                                  (* report any errors *)
  FatalERR('Opening "'+paramstr(3)+'"');
GotoXy(26,9);
write('ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿');
GotoXy(26,10);
write('³Filename : ');
HighVideo;
write(paramstr(3));
LowVideo;
GotoXy(51,10);
write('³¿');
GotoXy(26,11);
write('³Mode     : ');
HighVideo;
if ((paramstr(1)='E') or (paramstr(1)='e')) then
  begin (* 2 *)
  R:=KE;
  write('Encipher')
  end   (* 2 *)
else
  begin (* 2 *)
  R:=KD;
  write('Decipher')
  end;  (* 2 *)
LowVideo;
write('     ³³');
GotoXy(26,12);
write('ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´³');
GotoXy(26,13);
write('³Remaining:              ³³');
GotoXy(26,14);
write('ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ³');
GotoXy(26,15);
write(' ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ');
HighVideo;
Pos:=0;
Size:=filesize(Fd) div 80;                     (* number of blocks in file *)
GotoXY(38,13);
write(Size);
repeat
  blockread(Fd,Buff,MaxBuff,Num);                         (* and read data *)
  if IOResult <>0 then                                (* report any errors *)
    FatalERR('Reading "'+paramstr(3)+'"');
  Num8 := Num div 8;    (* how many complete 8 byte blocks have been read? *)
  Off:=1;                                           (* Offset within block *)
  If Num8>0 then                        (* if there is something in buffer *)
    begin (* 2 *)
    for L3:=1 to Num8 do               (* process block in 8 byte segments *)
      begin (* 3 *)
      if (L3 mod 10)=0 then
        begin (* 4 *)
        dec(Size);
        GotoXY(38,13);
        write(Size,' ')
        end;  (* 4 *)
      Txt[1]:=Buff[Off];
      Txt[2]:=Buff[1+Off];
      Txt[3]:=Buff[2+Off];
      Txt[4]:=Buff[3+Off];
      Txt[5]:=Buff[4+Off];
      Txt[6]:=Buff[5+Off];
      Txt[7]:=Buff[6+Off];
      Txt[8]:=Buff[7+Off];
      DES(Txt,Txt);
      Buff[Off]  :=Txt[1];
      Buff[1+Off]:=Txt[2];
      Buff[2+Off]:=Txt[3];
      Buff[3+Off]:=Txt[4];
      Buff[4+Off]:=Txt[5];
      Buff[5+Off]:=Txt[6];
      Buff[6+Off]:=Txt[7];
      Buff[7+Off]:=Txt[8];
      inc(Off,8)                         (* increment offset: 0,8,16, etc. *)
      end;  (* 3 *)
    seek(Fd,Pos);                              (* re-position file pointer *)
    blockwrite(Fd,Buff,Num8 shl 3);                 (* and write new block *)
    if IOResult <>0 then                              (* report any errors *)
      FatalERR('Writing "'+paramstr(3)+'"');
    inc(Pos,MaxBuff)                               (* move onto next block *)
  end   (* 2 *)
until (Num<MaxBuff);
GotoXy(27,13);
{$I+}
write('  ***FILE PROCESSED***');
lowvideo;
GotoXy(1,24);
sound(800);
delay(10);
sound(804);
delay(15);
nosound;
Cursor(true);
close(Fd)                                            (* close file and end *)
end.  (* 1 main *)
(***************************************************************************)
