program FctrlTab;  {Display factorials (mod m)}
{$N+,E+}
uses CRT, Printer, nothy;

const
  PrinterLimit = 60; {Limit on the number of lines in the printed table}
  MaxMod = 999999;   {Must have m < 1E6 so that entries fit on the screen}

var
m                      {modulus}
     :  comp;
x    :  extended;
i, j, k                {indices of the factorials}
     :  longint;
fcl                    {fcl[i] = i! (mod m)}
     :  array[0..10100] of single;
OriginalMode         {Save original screen mode for restoration}
         : word;
Ch       : char;     {Answer to prompt}
funckey,             {was a function key pressed?}
inputok              {is the input acceptable?}
         : boolean;

procedure Display(j : longint);   {display the factorials, starting at 10j}

var
f                      {factorial entered in table}
     :  comp;
St   :  string[20];
h, i, k,               {indices}
l,                     {length of a string}
ind                    {amount to indent by}
     :  integer;

begin
  TextColor(14);
  TextBackground(1);
  ClrScr;
  Str(m:1:0, St);
  l := Length(St);
  ind := (63 - l) div 2;   {amount to indent by, to center title}
  GoToXY(ind, 1);
  WriteLn('FACTORIALS (MOD ', m:1:0, ')');
  WriteLn;
  TextColor(15);
  Write('    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn('  10j+6  10j+7  10j+8  10j+9');
  WriteLn;
  k := 10*j;
  for i := 0 to 9 do
    begin
      TextColor(15);
      Write((j+i):6);
      TextColor(14);
      Write('  ');
      for h := 0 to 9 do
        begin
          if k <= m
          then f := fcl[k]
          else f := 0;
          k := k + 1;
          Write(f:7:0)
        end;
      WriteLn;
      WriteLn
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write('    PgUp         PgDn         J');
  TextColor(0);
  Write('ump         ');
  TextColor(4);
  Write('M');
  TextColor(0);
  Write('odulus         ');
  TextColor(4);
  Write('P');
  TextColor(0);
  Write('rint         ');
  TextColor(4);
  Write('Esc');
  GoToXY(1, 25);             {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0)
end;                         {of procedure Display}

procedure PrinTab;           {PRINT factorials up to PrinterLimit lines}

var
  St      :  string[12];
  f       :  single;      {factorial}
  h, i, j, ind,           {indices}
  l,                      {length of string}
  n,                      {upper limit of entries}
  r                       {index of last row}
          :  integer;

begin
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(1, 24);
  ClrEoL;
  GoToXY(1, 25);
  if m < 10*PrinterLimit
  then n := round(m)
  else n := 10*PrinterLimit - 1;
  r := trunc(n/10);
  Write('Sending j! (mod ', m:1:0);
  Write(') to the printer, for 0 ó j < ', n:1, ' ... ');
  Str(m:1:0, St);
  l := length(St);
  ind := (63 - l) div 2;   {amount to indent by, to center title}
  for i := 1 to ind do Write(Lst, ' ');
  WriteLn(Lst, 'FACTORIALS (MOD ', m:1:0, ')');
  WriteLn(Lst);
  Write(Lst, '    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn(Lst, '  10j+6  10j+7  10j+8  10j+9');
  WriteLn(Lst);
  j := 0;
  f := 1;
  i := 0;
  while (f > 0) and (j <= n) do
    begin
      Write(Lst, i:6);
      i := i + 1;
      Write(Lst, '  ');
      for h := 0 to 9 do
        begin
          if j <= n
          then
            begin
              f := fcl[j];
              Write(Lst, f:7:0)
            end;
          j := j + 1
        end;
      WriteLn(Lst)
    end;
  WriteLn(Lst);
  Write('done.');
  Delay(2000)
end;                                  {of procedure PrinTab}

function GetInput(XCoord, YCoord : integer; prompt, comm : string;
                                         min, max : comp) : comp;

var
x                                    {raw input}
         : extended;
x0                                   {x coordinate of cursor when ready
                                      to read input}
         : integer;
InputOK  : Boolean;

begin                                {body of function GetInput}
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(prompt);
  x0 := WhereX;
  if comm <> ''
  then
    begin
      WriteLn;
      ClrEoL;
      Write(comm);
      GoToXY(x0, WhereY - 1)
    end;
  InputOK := False;
  repeat
    ReadLn(x);
    if (frac(x) = 0) and (x <= max) and (x >= min)
    then InputOK := True
    else
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOK;
  ClrEoL;
  GetInput := x
end;                             {of function GetInput}

procedure CalcArray;

var
i, n       : integer;
f          : comp;

begin
  if m < 10100
  then n := round(m)
  else n := 10100;
  f := 1;
  fcl[0] := 1;
  for i := 1 to n do
    begin
      f := condition(f*i, m);
      fcl[i] := f
    end
end;

begin  {main body}
  OriginalMode := LastMode;
  Ch := 'F';
  funckey := false;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('Display factorials (mod m)');
  m := GetInput(WhereX, WhereY, '          Enter m = ',
  '(must have 0 < m < 1000000, so that entries fit on the screen)',
   1, MaxMod);
  Write('Calculating ... ');
  CalcArray;
  j := 0;
  repeat
    if (not funckey) and ((Ch = 'j') or (Ch = 'J'))
    then
        j := round(GetInput(1, 24,
        'Jump to row j = ', '0 ó j ó 1000', 0, 1000));
    if (not funckey) and ((Ch = 'm') or (Ch = 'M'))
      then
        begin
          m := GetInput(1, 24, 'Enter new modulus m = ',
          '  (must have 0 < m < 1000000, so that entries fit on the screen)',
           1, MaxMod);
          Write('Recalculating ... ');
          CalcArray
        end;
      if (funckey) and (Ch = #73)          {PgUp}
      then
        begin
          j := j - 10;
          if j < 0 then j := 0
        end;
      if (funckey) and (Ch = #81)          {PgDn}
      then
        begin
          j := j + 10;
          if j > 1000
          then j := 1000;
        end;
      Display(j);
      repeat
        Ch := ReadKey;
        if Ch <> #0
        then funckey := false
        else
          begin
            funckey := true;
            Ch := ReadKey
          end;
        inputok := false;
        if funckey and ((Ch = #73) or (Ch = #81))
        then inputok := true;
        if (not funckey)
          and (Ch in [#27, 'j', 'J', 'm', 'M', 'p', 'P'])
        then inputok := true;
      until inputok;
      if (Ch = 'P') or (Ch = 'p') then PrinTab;
  Until (not funckey) and (ch = #27);
TextMode(OriginalMode)
end.

