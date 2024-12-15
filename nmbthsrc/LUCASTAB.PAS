program LucasTab;  {Display U(n), V(n) (mod m)}
{$N+,E+}
uses nothy, CRT, Printer;
{$I GetInput.i }

const
  PrinterLimit = 60; {Limit on the number of lines in the printed table}
  MaxMod = 999999;   {Must have m < 1E6 so that entries fit on the screen}

var
n,                     {Initial index in table}
a, b,                  {parameters in recurrence}
a1, b1,                {parameters reduced (mod m)}
m,                     {modulus}
t                      {temporary variable}
     :  comp;
x    :  extended;
i, j, k                {indices}
     :  longint;
fcn                    {fcn[i] = U(10j + i) or V(10j + i) (mod m)}
     :  array[-1..101] of comp;
OriginalMode         {Save original screen mode for restoration}
         : word;
Ch       : char;     {Answer to prompt}
funckey,             {was a function key pressed?}
inputok,             {is the input acceptable?}
first                {first or second Lucas function?}
         : boolean;

procedure Display(j : longint);   {display the values, starting at 10j}

var
e,                   {entry in table}
ep, en               {previous, next entries}
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
  Str(m:1:0, St);
  l := length(St);
  if (a = 1) and (b = 1) and first
  then
    begin
      ind := (51 - l) div 2;   {amount to indent by, to center title}
      GoToXY(ind, 1);
      WriteLn('FIBONACCI NUMBERS F(n) (MOD ', m:1:0, ')')
    end;
  if (a = 1) and (b = 1) and (not first)
  then
    begin
      ind := (55 - l) div 2;   {amount to indent by, to center title}
      GoToXY(ind, 1);
      WriteLn('LUCAS NUMBERS L(n) (MOD ', m:1:0, ')')
    end;
  if (a <> 1) or (b <> 1)
  then
    begin
      Str(a:1:0, St);
      l := l + length(St);
      Str(b:1:0, St);
      l := l + length(St);
      ind := (51 - l) div 2;
      GoToXY(ind, 1);
      Write('LUCAS NUMBERS ');
      if first
      then Write('U')
      else Write('V');
      WriteLn('(n; ', a:1:0, ', ', b:1:0, ') (mod ', m:1:0, ')')
    end;
  WriteLn;
  TextColor(15);
  Write('    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn('  10j+6  10j+7  10j+8  10j+9');
  WriteLn;
  ep := fcn[-1];
  e := fcn[0];
  en := fcn[1];
  for i := 0 to 9 do
    begin
      TextColor(15);
      Write((j+i):6);
      TextColor(14);
      Write('  ');
      for h := 0 to 9 do
        begin
          if (first and (((e = 0) and (en = 1)) or ((ep = 0) and (e = 1))))
          or ((not first) and
           (((e = 2) and (en = a)) or ((ep = 2) and (e = a))))
          then
            begin
              TextColor(15);
              Write(e:7:0);
              TextColor(14)
            end
          else Write(e:7:0);
          ep := e;
          e := en;
          en := fcn[10*i + h + 2]
        end;
      WriteLn;
      WriteLn
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write('  PgUp      PgDn      ');
  if first
  then Write('V')
  else Write('U');
  Write('      n      a      b      M');
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

procedure PrinTab;           {PRINT table up to PrinterLimit lines}

var
  u0, u1,                 {values to be printed}
  t                       {temporary variable}
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
  Write('Sending ');
  if first
  then Write('U')
  else Write('V');
  Write('(n; ', a:1:0, ', ', b:1:0, ') (mod ', m:1:0);
  Write(') to the printer, for 0 ó n < ', printerlimit, '0 ... ');
  Str(m:1:0, St);
  l := length(St);
  if (a = 1) and (b = 1) and first
  then
    begin
      ind := (51 - l) div 2;   {amount to indent by, to center title}
      for i := 1 to ind do Write(Lst, ' ');
      WriteLn(Lst, 'FIBONACCI NUMBERS F(n) (MOD ', m:1:0, ')')
    end;
  if (a = 1) and (b = 1) and (not first)
  then
    begin
      ind := (55 - l) div 2;   {amount to indent by, to center title}
      for i := 1 to ind do Write(Lst, ' ');
      WriteLn(Lst, 'LUCAS NUMBERS L(n) (MOD ', m:1:0, ')')
    end;
  if (a <> 1) or (b <> 1)
  then
    begin
      Str(a:1:0, St);
      l := l + length(St);
      Str(b:1:0, St);
      l := l + length(St);
      ind := (51 - l) div 2;
      for i := 1 to ind do Write(Lst, ' ');
      Write(Lst, 'LUCAS NUMBERS ');
      if first
      then Write(Lst, 'U')
      else Write(Lst, 'V');
      WriteLn(Lst, '(n; ', a:1:0, ', ', b:1:0, ') (mod ', m:1:0, ')')
    end;
  WriteLn(Lst);
  Write(Lst, '    j       10j  10j+1  10j+2  10j+3  10j+4  10j+5');
  WriteLn(Lst, '  10j+6  10j+7  10j+8  10j+9');
  WriteLn(Lst);
  if first
  then
    begin
      u0 := 0;
      u1 := 1
    end
  else
    begin
      u0 := 2;
      u1 := a1
    end;
  Write(Lst, 0:6);
  Write(Lst, '  ');
  Write(Lst, u0:7:0);
  Write(Lst, u1:7:0);
  For h := 2 to 9 do
    begin
      t := u1;
      u1 := condition(a1*u1 + b1*u0, m);
      u0 := t;
      Write(Lst, u1:7:0)
    end;
  WriteLn(Lst);
  for i := 1 to PrinterLimit - 1 do
    begin
      Write(Lst, i:6);
      Write(Lst, '  ');
      for h := 0 to 9 do
        begin
          t := u1;
          u1 := condition(a1*u1 + b1*u0, m);
          u0 := t;
          Write(Lst, u1:7:0)
        end;
      WriteLn(Lst)
    end;
  WriteLn(Lst);
  Write('done.');
  Delay(2000)
end;                                  {of procedure PrinTab}

begin                                 {main body}
  OriginalMode := LastMode;
  funckey := false;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('Display Lucas functions U(n; a, b), V(n; a, b) (mod m)');
  WriteLn('where U(0) = 0, U(1) = 1, V(0) = 2, V(1) = a, and');
  WriteLn('U(n+1) = aU(n) + bU(n-1), V(n+1) = aV(n) + bV(n-1).');
  WriteLn('Note that U(n; 1, 1) = F(n) are the Fibonacci numbers,');
  WriteLn('and V(n; 1, 1) = L(n) are the Lucas numbers.');
  Write('Choose between U and V: ');
  repeat
    Ch := ReadKey
  until UpCase(Ch) in ['U', 'V'];
  WriteLn(UpCase(Ch));
  if UpCase(Ch) = 'U'
  then first := true
  else first := false;
  Ch := 'F';
  j := 0;
  a := GetInput(WhereX, WhereY, 'Enter parameter a = ',
      '  (must have ³a³ < 1000000)', -MaxMod, MaxMod);
  b := GetInput(WhereX, WhereY, 'Enter parameter b = ',
      '  (must have ³b³ < 1000000)', -MaxMod, MaxMod);
  m := GetInput(WhereX, WhereY, 'Choose modulus  m = ',
        '(must have 0 < m < 1000000, so that entries fit on the screen)',
        1, MaxMod);
  a1 := condition(a, m);
  b1 := condition(b, m);
  repeat
    if (not funckey) and ((Ch = 'a') or (Ch = 'A'))
    then
      begin
        a := GetInput(1, 24, 'Enter new parameter a = ',
        '      (must have ³a³ < 1000000)', -MaxMod, MaxMod);
        a1 := condition(a, m);
      end;
    if (not funckey) and ((Ch = 'b') or (Ch = 'B'))
    then
      begin
        b := GetInput(1, 24, 'Enter new parameter b = ',
        '      (must have ³b³ < 1000000)', -MaxMod, MaxMod);
        b1 := condition(b, m);
      end;
    if (not funckey) and ((Ch = 'n') or (Ch = 'N'))
    then
      begin
        n := GetInput(1, 24, 'Enter new initial point n = ',
        '         (must have 0 ó n ó 999900)', 0, 999900);
        t := n/10;
        if 10*t > n then t := t - 1;
        j := trunc(t)
      end;
    if (not funckey) and ((Ch = 'm') or (Ch = 'M'))
    then
      begin
        m := GetInput(1, 24, 'Enter new modulus m = ',
        '  (must have 0 < m < 1000000, so that entries fit on the screen)',
         1, MaxMod);
        a1 := condition(a, m);
        b1 := condition(b, m)
      end;
    if (not funckey) and (Upcase(Ch) = 'U')
    then first := true;
    if (not funckey) and (Upcase(Ch) = 'V')
    then first := false;
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
        fcn[-1] := fcn[99];
        fcn[0] := fcn[100];
        fcn[1] := fcn[101]
      end;
    if ((funckey) and (Ch = #73))
       or ((not funckey) and (UpCase(Ch)
              in ['N', 'A', 'B', 'F', 'M', 'U', 'V']))
    then                          {calculate first entries in table}
      if j = 0
      then
        begin
          fcn[-1] := MaxMod + 2;
          if first
          then fcn[0] := 0
          else fcn[0] := 2;
          if first
          then fcn[1] := 1
          else fcn[1] := a1
        end
      else
        if first
        then
          begin
            fcn[-1] := LucasU(10*j - 1, a1, b1, m);
            fcn[0] := LucasU(10*j, a1, b1, m);
            fcn[1] := condition(a1*fcn[0] + b1*fcn[-1], m)
          end
        else
          begin
            fcn[-1] := LucasV(10*j - 1, a1, b1, m);
            fcn[0] := LucasV(10*j, a1, b1, m);
            fcn[1] := condition(a1*fcn[0] + b1*fcn[-1], m)
          end;
                                 {First entries of table has been calculated}
                                 {now complete rest of table}
    for i := 2 to 101 do
    fcn[i] := condition(a1*fcn[i-1] + b1*fcn[i-2], m);
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
        and (Ch in [#27, 'n', 'N', 'a', 'A', 'b', 'B', 'm', 'M', 'p', 'P'])
      then inputok := true;
      if (first) and (not funckey) and (Upcase(Ch) = 'V')
      then inputok := true;
      if (not first) and (not funckey) and (Upcase(Ch) = 'U')
      then inputok := true
    until inputok;
    if (Ch = 'P') or (Ch = 'p') then PrinTab;
  Until (not funckey) and (ch = #27);
TextMode(OriginalMode)
end.
