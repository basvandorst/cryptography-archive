{Start of file Prompt.Pas **************************************************}

unit Prompt; {Utilities to operate with strings and Prompts to
              get input from the keyboard}

{ Version = 'Version 2.10, last revised: 1992-11-15 1400 hours';
  Author  = 'Copyright (c) 1981-1992 by author: Harry J. Smith,';
  Address = '19628 Via Monte Dr., Saratoga, CA 95070.  All rights reserved.';
}
{***************************************************************************}

{$I-} {Do our own i/o error checks}
{$N+} {Uses numeric coprocessor}
{$R-} {No index Range checking}
{$V-} {Parameter string length need not match}

{Developed in TURBO Pascal 5.5, maintained in 6.0}

interface

uses  Crt; {Turbo Pascal interface}

const
  Space = ' ';
  HisMax = 20;  { Max # of previous History entries }

type
  HisRec = record   { History previous entries }
    St   : ^String; { Previous entry }
    Lock : Char;    { = þ if entry is locked from delete }
    Plus : Char;    { = + if entry is longer than con be displayed }
  end;
  ScreenPtr = ^ScreenType;
  ScreenType = Record
    Pos : Array [1..80, 1..25] of Record
      Ch : Char;
      At : Byte;
    End;
    CursX,
    CursY : Integer;
  End;

  HotHelpT = procedure(I : Integer); {Hot help for F1..F10}

  Directions = (DUp, DDown, DLeft, DRight, DCenter);
  CharSet    = set of Char;
  ScrCommand = (NullCmd, ClrLine, ClrScreen);
  CtrlType   =
    (NotCtrl,  DupChr,   BackUp,  InsChr,   BegFld,  EndFld,
     DelChr,   ClrToEnd, Accept,  Truncate, Restore, Escape,
     Helper,   Stopper,  NextWrd, PrevWrd,  DelBack, ClrToBeg,
     ClrNxWrd, ClrPrWrd, UpHis,   DwnHis);

var
  Screen : ScreenPtr;  {The real screen}
  Screen1: ScreenPtr;  {A copy of the screen}
  MonoScr: Boolean;    {True if not a color screen}
  HotKey : Boolean;    {A Hot Key was input}
  EchoPtr: ^Text;      {If <> nil, pointer to Text file to echo help}
  FirstX : Byte;       {Used to go to first column of Crt window}
  FirstY : Byte;       {Used to go to first row of Crt window}
  LastX  : Byte;       {Used to go to last column of Crt window}
  LastY  : Byte;       {Used to go to last row of Crt window}
  NumX   : Byte;       {Number of columns in Crt window}
  NumY   : Byte;       {Number of rows Crt window}
  CtrlChr, NormalChr, NullSet, WhiteSpace : CharSet;
  Ctrl : packed array[0..255] of CtrlType;
  Null, Bell, BS, Tab, LF, FF, CR, HelpC, StopC, ESC, FillC : Char;{Constants}
  EscTyped, HelpTyped, StopTyped : Boolean;
  Background : Char;
  NullStrg : String[1];
  RspChr : Char;
  His    : array[0..HisMax] of HisRec;  { History of previous entries }
  HisPtr : Integer; { Index to currently selected previous entry }
  HisTN  : Integer; { Number of entries in HisTab }
  HisLN  : Integer; { Number of locked entries in HisTab }

{The following procedures can be called externally:}

procedure AddHis( Response : String; Lock : Char);
  {Add a response to the History of previous entries}

procedure InitScr;
  {Clear screen, home cursor, and init screen constants}

function ReadKeyM( HotHelpA : HotHelpT) : Char;
  {Read key and map to 0 - 255}

procedure GetYesNo( var Value : Boolean;
   HotHelpA : HotHelpT); {Get yes/no answer from operator}

procedure MoreLn( St : String;
   HotHelpA : HotHelpT); {WriteLn but stop for more on last line}

procedure HelpProm;
  {Give operator a help message for using Prom.}

procedure GoToXYI( Col, Row : Integer);
  {Move the cursor to a specified position on the screen, Col>LastX is wrapped}

procedure GetChr( NumDelays: Integer; WithBells: Boolean; ValidChr: CharSet;
   HotHelpA : HotHelpT);
  {Detect or await typing of a key.}

procedure ChgLen( var Strg: String; MaxLen, NewLen: Integer);
  {Change the length of Strg to NewLen.}

procedure Justify( var SrcStrg: String; var DstStrg: String;
          HowJust: Directions; PadChr: Char; OldLen, NewLen: Integer);
  {Left/right/center justify a string for the specified length using PadChr.}

procedure ScreenMsg( XPos, YPos: Integer; Command: ScrCommand;
          var Message: String);
  {Display a message at a specified cursor position.}

procedure Prom( RspX, RspY, RspLen: Integer; var Default: String;
               var Response: String; HotHelpA : HotHelpT);
  {Prompt the operator for an input.}

procedure EditLn( Prompt : String; RspLen : Integer; var PrSt, St : String;
   HotHelpA : HotHelpT);
  {Edit a long line of input using Prom}

{************************************************************}

implementation

  {--------------------------------------}
  procedure AddHis( Response : String; Lock : Char);
  var
    Dupe  : Boolean;
    HisHi : Integer;
  begin {AddHis}
    HisHi:= 0;  Dupe:= False;
    while (HisHi < HisTN) and (not Dupe) do begin
      Inc( HisHi);
      if Response = His[ HisHi].St^ then  Dupe:= True;
    end;
    His[0]:= His[ HisHi];
    if (not Dupe) then begin
      if (HisTN = HisMax) and (not Dupe) then begin
        while (His[ HisHi].Lock <> ' ') do  Dec( HisHi);
        FreeMem( His[ HisHi].St, Length( His[ HisHi].St^) + 1);
        Dec( HisTN);
      end
      else  Inc( HisHi);
    end;
    for HisPtr:= HisHi downto 2 do begin
      His[ HisPtr]:= His[ HisPtr - 1];
    end;
    if Dupe then begin
      His[1]:= His[0];
    end
    else begin
      GetMem( His[1].St, Length( Response) + 1);
      His[1].St^:= Response;
      His[1].Lock:= Lock;
      if (Length( Response) > 70) then    His[1].Plus:= '+'
      else  His[1].Plus:= ' ';
      Inc( HisTn);
    end;
    His[0].St:= nil;
    His[0].Lock:= ' ';
    His[0].Plus:= ' ';
    HisPtr:= 1;
  end; {AddHis}

{-----------------------------}
procedure InitScr; {Clear screen, home cursor, and init screen constants}
var
  I : Integer;
begin
  ClrScr;
  FirstX:= WhereX;
  FirstY:= WhereY;
  repeat
    LastX:= WhereX;
    Write(' ');  I:= WhereX;
  until LastX >= I;
  repeat
    LastY:= WhereY;
    WriteLn;  I:= WhereY;
  until LastY >= I;
  NumX:= LastX - FirstX + 1;
  NumY:= LastY - FirstY + 1;
  GoToXY( FirstX, FirstY);
end; {InitScr}

{--------------------------------------}
function ReadKeyM( HotHelpA : HotHelpT) : Char; {Read key and map to 0 - 255}
var
  I : Integer;
  Ch : Char;
  XL, YL : Byte;
begin
  repeat
    HotKey:= False;
    Ch:= ReadKey;
    XL:= WhereX;  YL:= WhereY;
    GoToXY( LastX-5, 1);  Write('      ');  GoToXY( LastX-4, 1);
    if Ch = Char(0) then begin
      Ch:= Chr( Ord( ReadKey));
      Write('0:', Ord( Ch));
      I:= Ord( Ch);
      if (59 <= I) and (I <= 68) then begin
	HotKey:= True;  GoToXY( XL, YL);  HotHelpA(I); {User can change HotKey}
      end;
      Inc(I, 128);
      if I > 255 then  I:= 0;
      Ch:= Chr(I);
    end
    else begin
      Write( Ord( Ch));
    end;
  until not HotKey;
  ReadKeyM:= Ch;
  GoToXY( XL, YL);
end; {ReadKeyM}

{-----------------------------}
procedure GetYesNo( var Value : Boolean;
   HotHelpA : HotHelpT); {Get yes/no answer from operator}
var Ch : Char;  Done : Boolean;
begin
  Write('Y', BS);
  repeat
    Ch:= ReadKeyM( HotHelpA);
{   if Ch = ESC  then  Ch:= 'N'; . Don't allow a true ESC, use F4 only }
{   if Ch = #190 then  Ch:= ESC; . ESC means No now on Yes/No request }
    Done:= Ch in ['Y', 'y', 'N', 'n', CR, ESC, StopC];
{   if not Done then Write( Bell); }
  until Done;
  EscTyped:= (Ch = ESC);
  StopTyped:= (Ch = StopC);
  Value:= Ch in ['Y', 'y', CR];
  if Value then WriteLn('Yes')
  else WriteLn('No');
end; {GetYesNo}

{-----------------------------}
procedure MoreLn( St : String;
   HotHelpA : HotHelpT); {WriteLn but stop for more on last line}
var
  Ch : Char;
begin
  if WhereY = LastY then begin
    Write('--More--');
    Ch:= ReadKeyM( HotHelpA);
    ClrScr;
  end;
  WriteLn( St);
  if EchoPtr <> nil then  WriteLn( EchoPtr^, St);
end; {MoreLn}

{-----------------------------}
procedure HelpProm; {Help Prom}
var
  I  : Integer;
  Ch : Char;
begin
  ClrScr;
  GoToXY(1, 2);
  WriteLn(
    'The active control characters for editing (^ = Ctrl, BS = Backspace):');
  WriteLn;
  WriteLn(' 0) Down   or Up => Retrieve history of previous operator entries');
  WriteLn(' 1) ^Right or ^F => Jump to beginning of next word');
  WriteLn(' 2) ^Left  or ^A => Jump to beginning of previous word');
  WriteLn(' 3) Right  or ^D => Retype the character at current position');
  WriteLn(' 4) Left   or ^S => Back up a space and delete if inserting');
  WriteLn(' 5) Del    or ^G => Delete the character at current position');
  WriteLn(' 6) BS     or ^H => Delete the character to left of cursor');
  WriteLn(' 7) End    or ^X => Jump to end of input');
  WriteLn(' 8) Home   or ^E => Jump to beginning of input');
  WriteLn(' 9) ^End   or ^Y => Clear input from current position to end');
  WriteLn('10) ^Home  or ^B => Clear input to left of cursor');
  WriteLn('11) PgDn   or ^T => Clear word to right');
  WriteLn('12) PgUp   or ^W => Clear word to left');
  WriteLn('13) Ins    or ^V => Toggle insert mode');
  WriteLn('14) Enter  or ^M => Accept the entire input as is');
  WriteLn(
     '15) ^Enter or ^J => Accept input, truncate if not at beginning or end');
  WriteLn('16)           F2 => Quit and exit to operating system');
  WriteLn('17)           F3 => Restore previous input');
  WriteLn('18)           F4 => Restore previous input and accept');
  WriteLn('19)           F5 => This menu: Help with input key control');
  WriteLn;
end; {HelpProm}

{--------------------------------------}
procedure GoToXYI( Col, Row : Integer); {Move the cursor to a specified
  position on the screen, Col > LastX is wrapped}
begin
  while Col > LastX do begin
    Dec( Col, NumX);  Inc( Row);
  end;
  GoToXY( Col, Row);
end; {GoToXYI}

{--------------------------------------}
procedure GetChr( NumDelays: Integer; WithBells: Boolean; ValidChr: CharSet;
   HotHelpA : HotHelpT);
{Detects or awaits typing of a key, optionally issuing bells while waiting.
 RspChr is Null or the character typed.
 if NumDelays < 0,
   GetChr will wait until user types a valid character
   (any character, if ValidChr = NullSet)
 if NumDelays = 0,
   GetChr will return the last character typed prior to the call
 if NumDelays > 0,
   GetChr will wait the specified number of delays or exit when a key is
   typed.
}
var
  ValidKey : Boolean;
begin {GetChr}
  ValidKey:= True;
  ValidKey:= (ValidChr = NullSet);
  if (ValidChr = NullSet)
    then ValidKey:= True
    else ValidKey:= False;
  {Write( Chr(5)); Turn on cursor if you can}
  repeat
    while (NumDelays <> 0) and (not KeyPressed) do begin
      if WithBells then {Write( Bell)};
      Delay( 300); {Wait a bit}
      {Decrease NumDelays BY 1 if Positive}
      Dec( NumDelays, Ord( NumDelays > 0));
    end; {while}
    if  KeyPressed or (NumDelays = 0) then
    begin
      {Get a character from the keyboard}
      RspChr:= ReadKeyM( HotHelpA);
      {Write( Chr(5)); Turn on cursor if you can}
{     if EoLn( Kbd) then RspChr:= CR; This did not work with
      Turbo Pascal v 3.01a
}     if not ValidKey then
        if (RspChr in ValidChr) then
          ValidKey:= True
        else
          {Write( Bell)};
    end
    else
      RspChr:= Null;
  until ValidKey;
  EscTyped:= (RspChr = ESC) or (RspChr = #190);
end; {GetChr}

{--------------------------------------}
procedure ChgLen( var Strg: String; MaxLen, NewLen: Integer);
  {Change the Length of Strg to NewLen.}
var I : Integer;
begin
  if NewLen < MaxLen then  Strg[0]:= Char( NewLen)
  else  Strg[0]:= Char( MaxLen);
end; {ChgLen}

{--------------------------------------}
procedure Justify( var SrcStrg: String; var DstStrg: String;
                  HowJust: Directions; PadChr: Char; OldLen,
                  NewLen: Integer);
{Left/Right/Center Justify a String for the specified Length using PadChr}
var
  Len   : Integer;
  StPos : Integer;
  I, J  : Integer;
begin {Justify}
  {If SrcStrg is too long, shorten it}
  if OldLen >= NewLen then Len:= NewLen
  else begin
    Len:= OldLen;
    {Fill the destination String with PadChr}
    for I:= 1 to NewLen do DstStrg[I]:= PadChr;
  end;
  {Determine where in the destination String to begin
     moving the source String}
  if Len > 0 then begin
    case HowJust of
      DLeft:   StPos:= 1;
      DRight:  StPos:= NewLen - Len + 1;
      DCenter: StPos:= (NewLen - Len) DIV 2 + 1;
    end; {case}
    {Move the source String into the destination String}
    J:= StPos;
    for I:= 1 to Len do begin
      DstStrg[J]:= SrcStrg[I];
      Inc(J);
    end;
  end; {if}
  {Make the destination String the proper Length}
  ChgLen( DstStrg, NewLen, NewLen);
end; {Justify}

{--------------------------------------}
procedure InitCtrl;
{Initialize the Prompt control characters}
var
  TmpChr : Char;

  {--------------------------------------}
  procedure AddToCtrl( ASCIIVal: Integer; CtrlKind: CtrlType);
  begin {AddToCtrl}
    Ctrl[ ASCIIVal]:= CtrlKind;
    CtrlChr:= CtrlChr + [Chr( ASCIIVal)];
  end; {AddToCtrl}

begin {InitCtrl}
  CtrlChr:= NullSet;
  NormalChr:= NullSet;
  for TmpChr:= Space to Chr( 126) do  NormalChr:= NormalChr + [TmpChr];

  AddToCtrl( 4,   DupChr);   {^D}
  AddToCtrl( 205, DupChr);   {RightArrow}

  AddToCtrl( 19,  BackUp);   {^S}
  AddToCtrl( 203, BackUp);   {LeftArrow}

  AddToCtrl( 7,   DelChr);   {^G}
  AddToCtrl( 211, DelChr);   {Del}

  AddToCtrl( 8,   DelBack);  {^H = BackSpace}

  AddToCtrl( 24,  EndFld);   {^X}
  AddToCtrl( 207, EndFld);   {End}

  AddToCtrl( 5,   BegFld);   {^E}
  AddToCtrl( 199, BegFld);   {Home}

  AddToCtrl( 6,   NextWrd);  {^F}
  AddToCtrl( 244, NextWrd);  {^RightArrow}
  AddToCtrl( 208, DwnHis);   {DownArrow}

  AddToCtrl( 1,   PrevWrd);  {^A}
  AddToCtrl( 243, PrevWrd);  {^LeftArrow}
  AddToCtrl( 200, UpHis);    {UpArrow}

  AddToCtrl( 25,  ClrToEnd); {^Y}
  AddToCtrl( 245, ClrToEnd); {^End}

  AddToCtrl( 2,   ClrToBeg); {^B}
  AddToCtrl( 247, ClrToBeg); {^Home}

  AddToCtrl( 20,  ClrNxWrd); {^T}
  AddToCtrl( 209, ClrNxWrd); {PgDn}

  AddToCtrl( 23,  ClrPrWrd); {^W}
  AddToCtrl( 201, ClrPrWrd); {PgUp}

  AddToCtrl( 22,  InsChr);   {^V}
  AddToCtrl( 210, InsChr);   {Ins}

  AddToCtrl( 13,  Accept);   {^M = CR}

{ AddToCtrl( 18,  Restore);  .^R}
  AddToCtrl( 189, Restore);  {F3}

{ AddToCtrl( 144, Stopper);   .Alt-Q}
  AddToCtrl( 188, Stopper);   {F2}

  AddToCtrl( 10,  Truncate); {^J = ^CR = LF}

{ AddToCtrl( 27,  Escape);   .^[ = ESC}
  AddToCtrl( 190, Escape);   {F4}

{ AddToCtrl( 17,  Helper); } {^Q}
{ AddToCtrl( 187, Helper); } {F1}

  CtrlChr:= CtrlChr + [Chr( 127)];
end; {InitCtrl}

{--------------------------------------}
procedure ScreenMsg( XPos, YPos: Integer; Command: ScrCommand;
                    var Message: String);
begin {ScreenMsg}
  GoToXY( XPos, YPos);
  case Command of
    ClrLine:   ClrEol;
    ClrScreen: ClrScr;
  end; {case}
  Write( Message);
end; {ScreenMsg}

{--------------------------------------}
procedure Prom( RspX, RspY, RspLen: Integer; var Default:
               String; var Response: String; HotHelpA : HotHelpT);
var
  Inserting, Terminated, TmpBool : Boolean;
  FmtLen, InsPos, RspPos : Integer;
  CtrlKind : CtrlType;
  ValidChr : CharSet;
  TmpDef   : String[ 255];

  {--------------------------------------}
  procedure Shift( ShiftDir: Directions);
  begin {Shift}
    case ShiftDir of
      DLeft: begin
        if RspPos < Length( Response) then
          Delete( Response,
                     RspPos + Ord( (CtrlKind <> NotCtrl) and
                     ((CtrlKind <> BackUp) or
                     (RspPos = InsPos))), 1);
          ScreenMsg( RspX, RspY, NullCmd, Response);
          Write( FillC);
      end; {case DLeft}

      DRight: begin
        Insert('^', Response, RspPos + 1);
        ScreenMsg( RspX, RspY, NullCmd, Response);
      end; {case DRight}
    end; {case}
  end; {Shift}

  {--------------------------------------}
  function CharAt( ChrPos: Integer) : Char;
  {Returns the character at position ChrPos in Response}
  begin {CharAt}
    if ChrPos > Length( Response) then
      CharAt:= FillC
    else
      CharAt:= Response[ ChrPos];
    end; {CharAt}

    {--------------------------------------}
    procedure DoInsChr;
    LABEL 1;
    begin {DoInsChr}
      case Inserting of
        False: begin
          if Length( Response) < RspLen then begin
            if RspLen > (RspPos + 1) then
              Shift( DRight)
            else
              Write('^');
            InsPos:= RspPos;
            Inserting:= True;
          end
          else
            {Write( Bell)};
        end; {case False}
        True: begin
          if RspLen > RspPos then
            if (CtrlKind = NotCtrl) then
              Response[ RspPos + 1]:= RspChr
            else
              Shift( DLeft);
            Inserting:= False;
            TmpDef:= Response;
        end; {case True}
      end; {case}
1:
  end; {DoInsChr}

  {--------------------------------------}
  procedure DoNotCtrl;
  LABEL 1;
  var
    ERROR : Boolean;
  begin {DoNotCtrl}
    {Little bell}
    ERROR:= False;
    if RspPos = RspLen then
    begin
      if Inserting then DoInsChr;
      ERROR:= True;
    end;
    if ERROR then
    begin
      {Write( Bell)};
      GOTO 1; {Exit( DoNotCtrl);}
    end; {if}
    if Inserting then
      if Length( Response) = RspLen then
        DoInsChr
      else
        if RspLen > (RspPos + 1) then
          Shift( DRight);
        GoToXYI( RspX + RspPos, RspY);
        if RspChr in NormalChr then
          Write( RspChr);
        Inc( RspPos);
        if RspPos > Length( Response) then
          ChgLen( Response, FmtLen, RspPos);
        Response[ RspPos]:= RspChr;
1:
  end; {DoNotCtrl}

  {--------------------------------------}
  procedure DoDupChr;
  begin {DoDupChr}
    if RspPos < Length( Response) then
      Inc( RspPos)
    else
      {Write( Bell)};
    end; {DoDupChr}

  {--------------------------------------}
  procedure DoBackUp;
  var
    TmpChr : Char;
  begin {DoBackUp}
    if RspPos = 0 then  {Write( Bell)}
    else begin
      if Inserting then begin
        if (RspPos = InsPos) then begin
          DoInsChr;
          TmpChr:= CharAt( RspPos);
        end else begin
          Shift( DLeft);
          TmpChr:= '^';
        end;
        Dec( RspPos);
        GoToXY( RspX + RspPos, RspY);
        Write( TmpChr);
      end
      else begin
  {     if RspPos <= Length( TmpDef, FmtLen) then
        begin
          TmpChr:= TmpDef[ RspPos];
          Response[ RspPos]:= TmpChr;
        end else begin
          TmpChr:= FillC;
          Delete( Response, RspPos, 1);
        end;
 } {Removed feature to delete end of input with Left arrow}

        Dec( RspPos);
{       GoToXY( RspX + RspPos, RspY);}
{       Write( TmpChr);}
      end;
    end; {if}
  end; {DoBackUp}

  {--------------------------------------}
  procedure ChgPos( NewPos: Integer);
  begin {ChgPos}
    if RspPos = NewPos then
      {Write( Bell)}
    else
    begin
      RspPos:= NewPos;
      TmpDef:= Response;
    end; {if}
  end; {ChgPos}

  {-----------------}
  procedure DoBegFld;
  begin {DoBegFld}
    ChgPos(0);
  end; {DoBegFld}

  {-------------------------}
  procedure DoEndFld;
  begin {DoEndFld}
    ChgPos( Length( Response));
  end; {DoEndFld}

  {--------------------------------------}
  procedure DoNextWrd;
  var R : Integer;
  begin {DoNextWrd}
    R:= RspPos;
    while (R < Length( Response)) and
          (Response[ R+1] <> ' ') and (Response[ R+1] <> '.') do Inc(R);
    while (R < Length( Response)) and
         ((Response[ R+1] =  ' ') or (Response[ R+1] =  '.')) do Inc(R);
    ChgPos(R);
  end; {DoNextWrd}

  {--------------------------------------}
  procedure DoPrevWrd;
  var  R : Integer;
  begin {DoPrevWrd}
    R:= RspPos;
    while (R > 0) and
         ((Response[R] =  ' ') or (Response[R] =  '.')) do  Dec(R);
    while (R > 0) and
          (Response[R] <> ' ') and (Response[R] <> '.') do  Dec(R);
    ChgPos(R);
  end; {DoPrevWrd}

  {--------------------------------------}
  procedure DoDelChr;
  begin {DoDelChr}
    if RspPos < Length( Response) then
    begin
      Shift( DLeft);
      TmpDef:= Response;
    end
    else
      {Write( Bell)};
  end; {DoDelChr}

  {--------------------------------------}
  procedure DoDelBack;
  begin {DoDelBack}
    if RspPos > 0 then begin
      Dec( RspPos);
      DoDelChr;
    end else {Write( Bell)};
  end; {DoDelBack}

  {--------------------------------------}
  procedure SetDefault;
  begin
    ChgLen( Response, FmtLen, RspPos);
    Justify( Response, TmpDef, DLeft, FillC, RspPos, RspLen);
    ScreenMsg( RspX, RspY, NullCmd, TmpDef);
    TmpDef:= Response;
  end; {SetDefault}

  {--------------------------------------}
  procedure DoClrToEnd;
  begin {DoClrToEnd}
    if RspPos < Length( Response) then
      SetDefault
    else
      {Write( Bell)};
  end; {DoClrToEnd}

  {--------------------------------------}
  procedure DoClrToBeg;
  begin
    while RspPos > 0 do begin
      Delete( Response, RspPos, 1);
      Dec( RspPos);
    end;
      Justify( Response, TmpDef, DLeft, FillC,
              Length( Response), RspLen);
      ScreenMsg( RspX, RspY, NullCmd, TmpDef);
      TmpDef:= Response;
  end; {DoClrToBeg}

  {--------------------------------------}
  procedure DoClrNxWrd;
  begin {DoPrevWrd}
    while (RspPos < Length( Response)) and
          (Response[ RspPos+1] <>  ' ') and
          (Response[ RspPos+1] <>  '.') do begin
      Delete( Response, RspPos+1, 1);
    end;
    while (RspPos < Length( Response)) and
         ((Response[ RspPos+1] = ' ') or
          (Response[ RspPos+1] = '.')) do begin
      Delete( Response, RspPos+1, 1);
    end;
      Justify( Response, TmpDef, DLeft, FillC,
              Length( Response), RspLen);
      ScreenMsg( RspX, RspY, NullCmd, TmpDef);
      TmpDef:= Response;
  end; {DoClrNxWrd}

  {--------------------------------------}
  procedure DoClrPrWrd;
  begin {DoPrevWrd}
    while (RspPos > 0) and ((Response[ RspPos] =  ' ') or
          (Response[ RspPos] =  '.')) do begin
      Delete( Response, RspPos, 1);
      Dec( RspPos);
    end;
    while (RspPos > 0) and (Response[ RspPos] <> ' ') and
          (Response[ RspPos] <> '.') do begin
      Delete( Response, RspPos, 1);
      Dec( RspPos);
    end;
      Justify( Response, TmpDef, DLeft, FillC,
              Length( Response), RspLen);
      ScreenMsg( RspX, RspY, NullCmd, TmpDef);
      TmpDef:= Response;
  end; {DoClrPrWrd}

  {--------------------------}
  procedure DoAccept;
  begin {DoAccept}
    RspPos:= Length( Response);
    Terminated:= True;
  end; {DoAccept}

  {----------------------------}
  procedure DoTruncate;
  begin {DoTruncate}
    if RspPos = 0 then
      RspPos:= Length( Response);
    Terminated:= True;
  end; {DoTruncate}

  {-------------------------}
  procedure InitDefault( Default : String);
  begin
    Response:= Default;
    RspPos:= Length( Default);
    SetDefault;
    RspPos:= 0;
  end; {InitDefault}

  {--------------------------}
  procedure DoRestore;
  begin {DoRestore}
    if Response = Default then
      {Write( Bell)}
    else
      InitDefault( Default);
  end; {DoRestore}

  {------------------}
  procedure DoEscape;
  begin {DoEscape}
    DoRestore;
    DoAccept;
  end; {DoEscape}

  {-----------------}
  procedure DoHelp;
  begin {DoHelp}
    HelpTyped:= True;
    DoAccept;
  end; {DoHelp}

  {------------------}
  procedure DoStop;
  begin {DoStop}
    StopTyped:= True;
    DoAccept;
  end; {DoStop}

{--------------------------------------}
procedure ShowHis( var Return : Boolean);
var
  Done : Boolean;
  Ch   : Char;
  Key  : Integer;
  I    : Integer;
  St   : String;
  Ptr  : Integer;
  MaxSt: Integer;
  LJ   : Integer;
  RJ   : Integer;
begin
  Key:= 0;  Done:= False;  Return:= False;
  TextBackground( Cyan);
  TextColor( Black);
  MaxSt:= 52;
  for Ptr:= 1 to HisTN do begin
    I:= Length( His[ Ptr].St^);
    if (I > MaxSt) then  MaxSt:= I;
  end;
  if MaxSt > 70 then  MaxSt:= 70;
  LJ:= (70 - MaxST) div 2;
  RJ:= 70 - MaxSt - LJ;
  repeat
    if (Key = 82) then begin { Insert }
      if (His[ HisPtr].Lock <> ' ') then begin
        His[ HisPtr].Lock:= ' ';  Dec( HisLN);
      end
      else begin
        if (HisMax - HisLN > 2) then begin
          His[ HisPtr].Lock:= 'þ';  Inc( HisLN);
        end;
      end;
      if HisPtr > 1 then  Dec( HisPtr)
      else HisPtr:= HisTN;
    end;
    if (Key = 83) then begin { Delete }
      if (His[ HisPtr].Lock <> ' ') then begin { Locked }
        if HisPtr > 1 then  Dec( HisPtr)
        else HisPtr:= HisTN;
      end
      else begin
        FreeMem( His[ HisPtr].St, Length( His[ HisPtr].St^) + 1);
        if (His[ HisPtr].Lock <> ' ') then  Dec( HisLN);
        I:= HisPtr + 1;
        while (I <= HisTN) do begin
          His[ I-1]:= His[I];  Inc(I);
        end;
        His[ I-1].St:= nil;
        His[ I-1].Lock:= ' ';
        Dec( HisTN);
        if HisPtr > 1 then  Dec( HisPtr);
        if HisTN  > 0 then  Return:= True;
        Exit;
      end;
    end;
    if (Key = 72) or (Key = 75) then { Up }
      if HisPtr < HisTN then
        Inc( HisPtr)
      else
        Key:= 79; { Bottom }
    if (Key = 80) or (Key = 77) then { Down }
      if (HisPtr > 1) then
        Dec( HisPtr)
      else
        Key:= 71; { Top }
    if (Key = 71) or (Key = 73) then  HisPtr:= HisTN; { Top }
    if (Key = 79) or (Key = 81) then  HisPtr:= 1;     { Bottom }
    GoToXY( 4+LJ, 2);
    Write('Ú');  for I:= 1 to 17-LJ do  Write('Ä');
    Write(' History of Previous Operator Entries ');
    for I:= 1 to 17-RJ do  Write('Ä');  Write('¿');
    for Ptr:= HisTN downto 1 do begin
      GoToXY( 4+LJ, WhereY+1);
      St:= His[ Ptr].St^;
      for I:= Length( St) + 1 to MaxSt do  St:= St + ' ';
      St:= Copy( St, 1, 70);
      Write('³', His[ Ptr].Lock);
      if HisPtr = Ptr then begin
        TextBackground( White);
        TextColor( Black);
      end;
      Write( St);
      if HisPtr = Ptr then begin
        TextBackground( Cyan);
        TextColor( Black);
      end;
      Write( His[ Ptr].PLus, '³');
    end;
    GoToXY( 4+LJ, WhereY+1);
    Write('À');  for I:= 1 to 10-LJ do  Write('Ä');
    Write( Output, '> Press ESC, Enter, Up, Down, PgUp, PgDn, Ins, Del <');
    for I:= 1 to 10-RJ do  Write('Ä');  Write('Ù', BS);
    Key:= 0;
    Ch:= ReadKey;
    if Ch = Chr(0) then begin
      Key:= Ord( ReadKey);
    end
    else begin
      if Ch = ESC then  Done:= True;
      if Ch = CR  then begin
        Response:= His[ HisPtr].St^;
        Done:= True;
      end;
    end;
  until Done;
end; {ShowHis}

  {------------------}
  procedure PopHis( var Return : Boolean);
  type
    ScreenPtr = ^ScreenType;
    ScreenType = Record
      Pos : Array [1..80, 1..25] of Record
        Ch : Char;
        At : Byte;
      End;
      CursX,
      CursY : Integer;
    end;
  var
    TextAttr1 : Byte;
    WindMin1  : Word;
    WindMax1  : Word;
  begin {PopHis}
    TextAttr1:= TextAttr;
    WindMin1:= WindMin;
    WindMax1:= WindMax;
    Screen1^:= Screen^;
    Screen1^.CursX:= WhereX;
    Screen1^.CursY:= WhereY;

    ShowHis( Return);

    Screen^:= Screen1^;
    TextAttr:= TextAttr1;
    WindMin:= WindMin1;
    WindMax:= WindMax1;
    GoToXY( Screen1^.CursX, Screen1^.CursY);
  end; {PopHis}

  {------------------}
  procedure DoUpHis;
  var Return : Boolean;
  begin {DoUpHis}
    if HisTN > 0 then begin
      repeat
        PopHis( Return);
      until not Return;
      RspPos:= Length( Response);
      SetDefault;
      RspPos:= 0;
    end;
  end; {DoUpHis}

  {------------------}
  procedure DoDwnHis;
  begin {DoDwnHis}
    DoUpHis;
  end; {DoDwnHis}

  {--------------------------------------}
  procedure DispRsp( var SrcStrg: String; Len: Integer; ClrArea: Boolean);
  begin {DispRsp}
    if ClrArea then  Justify( NullStrg, TmpDef, DLeft, Space, 0, FmtLen)
               else  Justify( SrcStrg, TmpDef, DLeft, Background, Len, FmtLen);
    ScreenMsg( RspX, RspY, NullCmd, TmpDef);
  end; {DispRsp}

{--------------------------------------}
begin {Prom}
  Inserting:= False;
  Terminated:= False;
  RspChr:= Null;
  FmtLen:= RspLen;
  DispRsp( NullStrg, 0, True);
  ValidChr:= NormalChr + CtrlChr;
  InitDefault( Response); {Set and display original default}
  StopTyped:= False;
  HelpTyped:= False;
  repeat
    GoToXYI( RspX + RspPos, RspY);
    GetChr(0, False, ValidChr, HotHelpA);
    if RspChr in CtrlChr then
    begin
      if RspChr = Chr( 127) then RspChr:= BS;
      CtrlKind:= Ctrl[ Ord( RspChr)];
      if Inserting then
        if (CtrlKind <> InsChr) and
           (CtrlKind <> BackUp) then
          DoInsChr;
      case CtrlKind of
        DupChr:   DoDupChr;
        BackUp:   DoBackUp;
        InsChr:   DoInsChr;
        BegFld:   DoBegFld;
        EndFld:   DoEndFld;
        NextWrd:  DoNextWrd;
        PrevWrd:  DoPrevWrd;
        DelChr:   DoDelChr;
        DelBack:  DoDelBack;
        ClrToEnd: DoClrToEnd;
        ClrToBeg: DoClrToBeg;
        ClrNxWrd: DoClrNxWrd;
        ClrPrWrd: DoClrPrWrd;
        Accept:   DoAccept;
        Truncate: DoTruncate;
        Restore:  DoRestore;
        Escape:   DoEscape;
        Helper:   DoHelp;
        Stopper:  DoStop;
        UpHis:    DoUpHis;
        DwnHis:   DoDwnHis;
      end; {case}
    end else begin
      CtrlKind:= NotCtrl;
      DoNotCtrl;
    end; {if}
  until Terminated;
  ChgLen( Response, FmtLen, RspPos);
  if RspPos >= 0 then DispRsp( Response, RspPos, False);
  GoToXYI( RspX + RspPos, RspY);
  WriteLn;
  if Response <> '' then  AddHis( Response, ' ');
end; {Prom}

{--------------------------------------}
procedure EditLn( Prompt : String; RspLen : Integer; var PrSt, St : String;
   HotHelpA : HotHelpT);
  {Edit a long line of input using Prom}
var
  I     : Integer;
  Lines : Integer;
  Quit  : Boolean;
begin
  St:= '';
  Lines:= (Length( Prompt) + RspLen) div NumX;
  repeat
    GoToXY( FirstX, WhereY);
    Write( Prompt);
    for I:= 1 to RspLen do  Write(' ');
    Prom( FirstX+Length( Prompt), WhereY - Lines, RspLen, PrSt, St, HotHelpA);
    if EscTyped then  ;
    if StopTyped then begin
      WriteLn;
      Write('Do you wish to Quit the program ? (Y/N): ');
      GetYesNo( Quit, HotHelpA);
      WriteLn;
      if Quit then begin
        St:= '';  Exit;
      end;
    end;
    if HelpTyped then  HelpProm;
  until not HelpTyped and not StopTyped;
end; {EditLn}

{--------------------------------------}
function VidSeg : {Integer} Word;
Begin
MonoScr:= (Mem[ $0000:$0449] = Mono);
if MonoScr then
  VidSeg:= $B000
else
  VidSeg:= $B800;
end; {VidSeg}

{--------------------------------------}
{        Initialization section        }
{--------------------------------------}

{Prom Initialization}
begin
  StopTyped:= False;
  EchoPtr:= nil;
  Null:= Chr( 0);
  Bell:= Chr( 7);
  BS:=   Chr( 8);
  Tab:=  Chr( 9);
  LF:=   Chr( 10);
  FF:=   Chr( 12);
  CR:=   Chr( 13);
  HelpC:=Chr( 187); {F1}
  StopC:=Chr( 188); {F2}
  ESC:=  Chr( 27);
  FillC:=Chr( 95);  {Underscore}
{ PrintScreenCode:= Chr( 16);  .Ctrl-P = Dle}
  Background:= Space;
  NullSet:= [];
  WhiteSpace:= [CR, LF, Tab, ' '];
  ChgLen( NullStrg, 1, 0);
  InitCtrl;
  InitScr;
  for HisPtr:= 0 to HisMax do begin
    His[ HisPtr].St:= nil;
    His[ HisPtr].Lock:= ' ';
    His[ HisPtr].Plus:= ' ';
  end;
  HisTN:= 0;  HisLN:= 0;  HisPtr:= 1;
  New( Screen1);
  Screen:= Ptr( VidSeg, $0000);
end. {Prom Initialization}

Revisions made -
--------
* Changes procedure GetYesNo to allow a true ESC and not allow a F4
  1992-11-15 HJS

--------

{End of file Prompt.Pas ****************************************************}
