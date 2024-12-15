program PowerTab;  {Display powers of a (mod m)}
{$N+,E+}
uses CRT, Printer, nothy;

const
  PrinterLimit = 60; {Limit on the number of lines in the printed table}
  MaxMod = 999999;   {Must have m < 1E6 so that entries fit on the screen}

var
a,                     {base of which powers are to be computed}
b,                     {base reduced modulo m}
m                      {modulus}
     :  comp;
x    :  extended;
i, j, k                {indices of the powers}
     :  longint;
pwr                    {pwr[i] = a^(10j + i) (mod m)}
     :  array[0..99] of comp;
OriginalMode         {Save original screen mode for restoration}
         : word;
Ch       : char;     {Answer to prompt}
funckey,             {was a function key pressed?}
inputok              {is the input acceptable?}
         : boolean;

procedure Display(j : longint);   {display the powers, starting at 10j}

var
p                      {power entered in table}
     :  comp;
St   :  string[20];
h, i,                  {indices}
l,                     {length of a string}
ind                    {amount to indent by}
     :  integer;

begin
  TextColor(14);
  TextBackground(1);
  ClrScr;
  Str(a:1:0, St);
  l := Length(St);
  Str(m:1:0, St);
  l := l + Length(St);
  ind := (63 - l) div 2;   {amount to indent by, to center title}
  GoToXY(ind, 1);
  WriteLn('POWERS OF ', a:1:0, ' (MOD ', m:1:0, ')');
  WriteLn;
  TextColor(15);
  Write('    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn('  10j+6  10j+7  10j+8  10j+9');
  WriteLn;
  for i := 0 to 9 do
    begin
      TextColor(15);
      Write((j+i):6);
      TextColor(14);
      Write('  ');
      for h := 0 to 9 do
        begin
          p := pwr[10*i + h];
          if (b = p) or (p = 1)
          then
            begin
              TextColor(15);
              Write(p:7:0);
              TextColor(14)
            end
          else Write(p:7:0)
        end;
      WriteLn;
      WriteLn
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write('    PgUp      PgDn      B');
  TextColor(0);
  Write('ase      ');
  TextColor(4);
  Write('E');
  TextColor(0);
  Write('xponent      ');
  TextColor(4);
  Write('M');
  TextColor(0);
  Write('odulus      ');
  TextColor(4);
  Write('P');
  TextColor(0);
  Write('rint      ');
  TextColor(4);
  Write('Esc');
  GoToXY(1, 25);             {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0)
end;                         {of procedure Display}

procedure PrinTab;           {PRINT powers up to PrinterLimit lines}

var
  p                       {powers to be entered in table}
          :  comp;
  St      :  string[12];
  h, i, ind,              {indices}
  l                       {length of string}
          :  integer;

begin
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(1, 24);
  ClrEoL;
  GoToXY(1, 25);
  Write('Sending ', a:1:0);
  GoToXY(WhereX, 24);
  Write('j');
  GoToXY(WhereX, 25);
  Write(' (mod ', m:1:0);
  Write(') to the printer, for 0 ó j < ', printerlimit, '0 ... ');
  Str(a:1:0, St);
  l := Length(St);
  Str(m:1:0, St);
  l := l + length(St);
  ind := (63 - l) div 2;   {amount to indent by, to center title}
  for i := 1 to ind do Write(Lst, ' ');
  WriteLn(Lst, 'POWERS OF ', a:1:0, ' (MOD ', m:1:0, ')');
  WriteLn(Lst);
  Write(Lst, '    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn(Lst, '  10j+6  10j+7  10j+8  10j+9');
  WriteLn(Lst);
  p := 1;
  for i := 0 to PrinterLimit - 1 do
    begin
      Write(Lst, i:6);
      Write(Lst, '  ');
      for h := 0 to 9 do
        begin
          Write(Lst, p:7:0);
          p := condition(b*p, m)
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

begin  {main body}
  OriginalMode := LastMode;
  Ch := 'F';
  funckey := false;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('Display powers of a (mod m)');
  a := GetInput(WhereX, WhereY, '          Enter a = ',
      '  (must have ³a³ < 1000000)', -MaxMod, MaxMod);
  m := GetInput(WhereX, WhereY, '          Enter m = ',
  '(must have 0 < m < 1000000, so that entries fit on the screen)',
   1, MaxMod);
  b := condition(a, m);
  repeat
    if Ch = 'F'
    then
      begin
        pwr[0] := 1;
        j := 0
      end;
    if (not funckey) and ((Ch = 'b') or (Ch = 'B'))
    then
      begin
        a := GetInput(1, 24, 'Enter new base a = ',
        ' (must have ³a³ < 1000000)', -MaxMod, MaxMod);
        b := condition(a, m);
        pwr[0] := power(b, 10*j, m)
      end;
    if (not funckey) and ((Ch = 'e') or (Ch = 'E'))
    then
      begin
        k := round(GetInput(1, 24,
        'Enter new exponent k = ',
        '(which will appear in the first line of the new table)',
        0, MaxMod));
        j := k div 10;
        pwr[0] := power(b, 10*j, m);
      end;
    if (not funckey) and ((Ch = 'm') or (Ch = 'M'))
      then
        begin
          m := GetInput(1, 24, 'Enter new modulus m = ',
          '  (must have 0 < m < 1000000, so that entries fit on the screen)',
           1, MaxMod);
          b := condition(a, m);
          pwr[0] := power(b, 10*j, m)
        end;
      if (funckey) and (Ch = #73)          {PgUp}
      then
        begin
          j := j - 10;
          if j < 0 then j := 0;
          pwr[0] := power(b, 10*j, m)
        end;
      if (funckey) and (Ch = #81)          {PgDn}
      then
        begin
          j := j + 10;
          pwr[0] := condition(pwr[99]*b, m)
        end;
                                   {First entry of table has been calculated}
                                   {now complete rest of table}
      for i := 1 to 99 do
      pwr[i] := condition(pwr[i-1]*b, m);
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
          and (Ch in [#27,'b', 'B', 'e', 'E', 'm', 'M', 'p', 'P'])
        then inputok := true;
      until inputok;
      if (Ch = 'P') or (Ch = 'p') then PrinTab;
  Until (not funckey) and (ch = #27);
TextMode(OriginalMode)
end.

