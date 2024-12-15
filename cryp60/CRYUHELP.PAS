{Start of file CryUHelp.Pas ************************************************}

unit CryUHelp; {Help unit for Variable Precision floating decimal calculator}

{$I-} {Do our own i/o error checks}
{$N+} {Uses numeric coprocessor}
{$R-} {No index Range checking}
{$V-} {No string length checking}

interface

{***************************************************************************}
const
 Name  = 'CryU - A data encryption system written in Pascal, user interface';
 Version = 'Version 6.00, 1992-11-15, 1400 hours';
 Author  = 'Copyright (c) 1987-1992 by author: Harry J. Smith';
 Address = '19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.';

{ Actual Name, Version, Author, Address maintained here. }

{***************************************************************************}

{Developed in TURBO Pascal 5.5, maintained in 6.0}

{The following PROCEDUREs can be called externally:}

  procedure HotHelp(I : Integer);      {Hot help for F1, F5..F10}
  procedure CryUHotHelp(I : Integer);  {Hot help for F1, ..., F10}
  procedure Ind( N : Integer; St : String);    {Indent N}
  procedure IndLn( N : Integer; St : String);  {Indent N & new line}
  procedure Title;  {Output program's title messages}

implementation

uses
  Crt,     { Turbo Pascal interface }
  Prompt;

const
  UL = Char( 201);  UR = Char( 187);  {Characters for drawing a box, É »}
  VB = Char( 186);  HB = Char( 205);  {º Í}
  LL = Char( 200);  LR = Char( 188);  { È ¼}

var
  Ch   : Char;
  i, j : Integer;

{--------------------------------------}
procedure Center( St : String);
var
  I : Integer;
begin
  for I:= 1 to (80 - Length( St)) div 2 do
    Write(' ');
  Write( St);
end; {Center}

{--------------------------------------}
procedure CenterLn( St : String);
begin
  Center( St);
  WriteLn;
end; {Center}

{--------------------------------------}
procedure TopBox( First, Last : Integer);  {Draw top of a box}
var
  I : Integer;
begin
  for I:= 2 to First do  Write(' ');
  Write( UL);
  for I:= First to Last - 2 do  Write( HB);
  WriteLn( UR);
end; {TopBox}

{--------------------------------------}
procedure CenterBox( First, Last : Integer; St : String);
 {Draw center lines of a box}
var
  I : Integer;
begin
  for I:= 2 to First do  Write(' ');
  Write( VB);
  for I:= 1 to (Last - First - Length( St)) div 2 do  Write(' ');
  Write( St);
  for I:= 1 to (Last - First - Length( St) - 1) div 2 do  Write(' ');
  WriteLn( VB);
end; {CenterBox}

{--------------------------------------}
procedure BotBox( First, Last : Integer);  {Draw bottom of a box}
var
  I : Integer;
begin
  for I:= 2 to First do  Write(' ');
  Write( LL);
  for I:= First to Last - 2 do  Write( HB);
  Write( LR);
end; {BotBox}

{--------------------------------------}
procedure Title;  {Output program's title messages}
begin
  TopBox(6, 74);
  CenterBox(6, 74, Name);
  CenterBox(6, 74, Version);
  CenterBox(6, 74, Author);
  CenterBox(6, 74, Address);
  BotBox(6, 74);
  Write;
end; {Title}

{--------------------------------------}
procedure Ind( N : Integer; St : String);  {Indent N}
var
  I : Integer;
begin
  for I:= 1 to N do  Write(' ');
  Write( St);
end; {Ind}

{--------------------------------------}
procedure IndLn( N : Integer; St : String);  {Indent N & new line}
begin
  Ind(N, St);
  WriteLn;
end; {IndLn}

{--------------------------------------}
procedure DoBorder( X1, Y1, X2, Y2 : Byte; Title : String);  {Draw a border}
var
  I : Integer;
begin
  GoToXY( X1, Y1);
  TopBox( 1, X2 - X1 + 1);
  GoToXy( (X1 + X2 - Length( Title)) div 2, Y1);
  Write( Title);
  for I:= Y1+1 to Y2-1 do begin
    GoToXY( X1, I);  Write( VB);
    GoToXY( X2, I);  Write( VB);
  end;
  GoToXY( X1, Y2);
  BotBox( 1, X2 - X1 + 1);
  Window( X1+1, Y1+1, X2-1, Y2-1);
  ClrScr;
end; {DoBorder}

{--------------------------------------}
procedure F1;  {Hot help for F1}
var
  TextAttr1 : Byte;
  WindMin1  : Word;
  WindMax1  : Word;
begin
  TextAttr1:= TextAttr;
  WindMin1:= WindMin;
  WindMax1:= WindMax;
  TextBackground( Cyan);
  TextColor( White);
  DoBorder( 15, 7, 65, 14, ' Help ');
  GoToXY(1, 1);
  TextColor( Black);
  IndLn(3, 'F1  => This help menu');
  IndLn(3, 'F2  => Quit and exit to operating system*');
  IndLn(3, 'F3  => Restore previous input*');
  IndLn(3, 'F4  => Restore previous input and accept*');
  IndLn(3, 'F5  => Active control characters for editing');
  Ind(  3, 'ESC => Exit Help (* active on Command: line)');
  TextAttr:= TextAttr1;
  WindMin:= WindMin1;
  WindMax:= WindMax1;
  GoToXY( 65, 14);
end; {F1}

{--------------------------------------}
procedure CryUHotHelp(I : Integer);  {Hot help for F1, ..., F10}
var
  done : boolean;
  Ch   : char;
begin
  HotKey:= False;
  if (I = 59) or ((63 <= I) and (I <= 63)) then begin
    HotKey:= True;  done:= False;
    repeat
      if      (I = 59) then  F1
      else if (I = 63) then  HelpProm  {F5}
      else  I:= 0;
      if (I <> 0) and (I >= 63) then begin
        Center(' >>>> PRESS ESC TO EXIT HELP <<<< ');
      end;
      I:= 0;
      Ch:= ReadKey;
      if Ch = Chr(0) then begin
        I:= Ord( ReadKey);
      end
      else begin
        if (Ch = Chr( 27)) then  done:= True;  {ESC}
      end;
    until done;
  end;
end; {CryUHotHelp}

{--------------------------------------}
procedure HotHelp(I : Integer);  {Hot help for F1, F5..F10}
var
  done : boolean;
  TextAttr1 : Byte;
  WindMin1  : Word;
  WindMax1  : Word;
begin
  TextAttr1:= TextAttr;
  WindMin1:= WindMin;
  WindMax1:= WindMax;
  Screen1^:= Screen^;
  Screen1^.CursX:= WhereX;
  Screen1^.CursY:= WhereY;
  CryUHotHelp(I);
  Screen^:= Screen1^;
  TextAttr:= TextAttr1;
  WindMin:= WindMin1;
  WindMax:= WindMax1;
  GoToXY( Screen1^.CursX, Screen1^.CursY);
end; {HotHelp}

{--------------------------------------}
function ExitHelp: Boolean;  {Test for exit help}
const Echo = 0;
var
  I : Integer;  Ch : Char;
begin
  ExitHelp:= True;
  for I:= Echo downto 0 do begin
    Center(' **** PRESS ANY KEY TO CONTINUE HELP OR PRESS ESCAPE **** ');
    Ch:= ReadKeyM( HotHelp);
    WriteLn;
    if Ch = Char( 27) then begin {Esc}
      for I:= Echo downto 0 do  WriteLn;
      Exit;
    end;
    ClrScr;
    WriteLn;
  end;
  ExitHelp:= False;
end; {ExitHelp}

{--------------------------------------}
begin  {Init the unit}
  ;
end.

Revisions made -
--------

--------

{End of file CryUHelp.Pas **************************************************}
