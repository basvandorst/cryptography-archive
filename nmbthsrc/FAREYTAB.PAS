program FareyTab;  {Display portions of the Farey fractions
                    of prescribed order}
{$N+,E+}
uses CRT, Printer;

const
  PrinterLimit = 46; {Limit on Q for which table will be printed. With
                      Q = 46, the table just barely fits on 10 pages.}

var
Q,                   {Farey fractions of order Q}
a0, a1,              {Numerators of fractions}
q0, q1               {Denominators of fractions}
         : longint;
x,                   {Decimal about which table is centered}
t                    {temporary variable}
         : extended;
i, j, k              {Indices}
         : integer;
num, den : array[1..20] of longint;
quot     : array[1..20] of extended;
OriginalMode         {Save original screen mode for restoration}
         : word;
Ch       : char;     {Answer to prompt}
funckey,             {was a function key pressed?}
inputok              {is the input acceptable?}
         : boolean;

procedure EucAlg(a, b : longint; var x, y : longint);

var
m,                   {multiple used}
t,                   {temporary variable for swapping}
r0, r1,              {remainders}
x0, x1,              {coefficients of a}
y0, y1               {coefficients of b}
         : longint;

begin
  r0 := a;
  r1 := b;
  x0 := 1;
  x1 := 0;
  y0 := 0;
  y1 := 1;
  while r1 > 0 do
    begin
      m := r0 div r1;
      t := x1;
      x1 := x0 - m*x1;
      x0 := t;
      t := y1;
      y1 := y0 - m*y1;
      y0 := t;
      t := r1;
      r1 := r0 - m*r1;
      r0 := t
    end;
  x := x0;
  y := y0
end;                        {of procedure EucAlg}

procedure NextLarger(a0, q0 : longint; var a1, q1 : longint);

var
  k  : longint;

begin
  EucAlg(q0, a0, a1, q1);
  k := (Q+q1) div q0;
  a1 := a1 + k*a0;
  q1 := - q1 + k*q0
end;                       {of procedure NextLarger}

procedure NextSmaller(a0, q0 : longint; var a1, q1 : longint);

var
  k  : longint;

begin
  EucAlg(a0, q0, q1, a1);
  k := (Q-q1) div q0;
  a1 := - a1 + k*a0;
  q1 := q1 + k*q0
end;                       {of procedure NextSmaller}

procedure ContFrac(x : extended; var a0, q0 : longint);

var
h0, h1,                      {numerators}
k0, k1,                      {denominators}
m,                           {multiple}
t                            {temporary variable}
                : extended;

begin
  if x = 1
  then
    begin
      a0 := 1;
      q0 := 1;
      exit
    end;
  x := frac(x);
  if x = 0
  then
    begin
      a0 := 0;
      q0 := 1;
      exit
    end;
  h0 := 1;
  h1 := 0;
  k0 := 0;
  k1 := 1;
  repeat
    x := 1/x;
    m := int(x);
    x := x - m;
    t := h1;
    h1 := m*h1 + h0;
    h0 := t;
    t := k1;
    k1 := m*k1 + k0;
    k0 := t
  until (x = 0) or (k1 > Q);
  if k1 <= Q
  then
    begin
      a0 := round(h1);
      q0 := round(k1)
    end
  else
    begin
      a0 := round(h0);
      q0 := round(k0)
    end;
end;                  {of procedure ContFrac}

procedure Display;           {display the array of fractions}

var
St   :  string[12];
l,                     {length of a string}
ind                    {amount to indent by}
     :  integer;

begin
  TextColor(14);
  TextBackground(1);
  ClrScr;
  Str(Q:1, St);
  l := Length(St);
  ind := (57 - l) div 2;   {amount to indent by, to center title}
  GoToXY(ind, 1);
  WriteLn('FAREY FRACTIONS OF ORDER ', Q:1);
  Write('                靈컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�');
  WriteLn('컴컴컴컴컴컴캠');
  ind := l + 33;
  For i := j to k do
    begin
      Write('                �');
      Str(num[i]:1, St);
      l := Length(St);
      GoToXY(28 - l, WhereY);
      Write(num[i]:1, '/', den[i]:1);
      GoToXY(ind, WhereY);
      Write(quot[i]:20:18);
      GoToXY(63, WhereY);
      WriteLn('�');
    end;
  Write('                聃컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�');
  WriteLn('컴컴컴컴컴컴캭');
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  if Q < 8
  then
    begin
      TextColor(0);
      Write('             New ');
      TextColor(4);
      Write('Q                    P');
      TextColor(0);
      Write('rint          ');
      TextColor(4);
      Write('           Esc');
    end;
  if (Q > 7) and (Q <= PrinterLimit)
  then
    begin
      TextColor(4);
      Write('    PgUp     PgDn     ');
      TextColor(0);
      Write('New x: ');
      TextColor(4);
      Write('D');
      TextColor(0);
      Write('ecimal or ');
      TextColor(4);
      Write('R');
      TextColor(0);
      Write('ational     New ');
      TextColor(4);
      Write('Q     P');
      TextColor(0);
      Write('rint     ');
      TextColor(4);
      Write('Esc');
    end;
  if Q > PrinterLimit
  then
    begin
      TextColor(4);
      Write('     PgUp       PgDn       ');
      TextColor(0);
      Write('New x: ');
      TextColor(4);
      Write('D');
      TextColor(0);
      Write('ecimal or ');
      TextColor(4);
      Write('R');
      TextColor(0);
      Write('ational       New ');
      TextColor(4);
      Write('Q       Esc');
    end;
  TextColor(7);                      {hide the cursor}
  GoToXY(1, 25);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0)
end;                         {of procedure Display}

procedure PrinTab;           {PRINT the entire TABle}

var
  St      :  string[12];
  i, ind,                 {indices}
  l1, l2, l3              {lengths of strings}
          :  integer;
  a0, a1,                 {numerators}
  q0, q1                              {and denominators of Farey fractions}
          :  longint;
  t       :  extended;

begin
  if Q > PrinterLimit then Exit;
  ClrScr;
  GoToXY(10, 12);
  Write('Sending the entire table to the printer ... ');
  Str(Q:1, St);
  l1 := Length(St);
  ind := (55 - l1) div 2;   {amount to indent by, to center title}
  for i := 1 to ind do Write(Lst, ' ');
  WriteLn(Lst, 'FAREY FRACTIONS OF ORDER ', Q:1);
  WriteLn(Lst);
  ind := l1 + 33;
  a0 := 0;
  q0 := 1;
  t := 0;
  repeat
    Write(Lst, '                 ');
    Str(a0:1, St);
    l2 := Length(St);
    Str(q0:1, St);
    l3 := Length(St);
    for i := 1 to 10 - l2 do Write(Lst, ' ');
    Write(Lst, a0:1, '/', q0:1);
    for i := 1 to (4 + l1 - l3) do Write(Lst, ' ');
    WriteLn(Lst, t:20:18);
    NextLarger(a0, q0, a1, q1);
    a0 := a1;
    q0 := q1;
    t := a0/q0
  until t > 1;
  WriteLn(Lst);
  Write('done.');
  Ch := 'Q';
  Delay(1000)
end;                                  {of procedure PrinTab}

begin  {main body}
  OriginalMode := LastMode;
  Ch := 'F';
  funckey := false;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('Display Farey fractions of order Q < 10^9,');
  Repeat
    Write('where Q = ');
    ReadLn(Q)
  Until (Q>0) and (Q < 1E9);
  if Q > 7
  then
    begin
      WriteLn('Initial table will be centered at x, ');
      WriteLn('which may be entered in decimal or rational form.');
      Write('Type "d" for decimal, "r" for rational   ');
      WriteLn;
      repeat
        Ch := ReadKey;
        Ch := UpCase(Ch);
      until Ch IN ['D', 'R'];
      if Ch = 'D' {get new decimal x}
      then
        begin
          Write('Enter decimal value of x, x = ');
          ReadLn(x);
        end;
      if Ch = 'R'     {get rational value of x}
      then
        begin
          WriteLn;
          Write('Enter x = a/q where a = ');
          ReadLn(a0);
          GoToXY(21, WhereY);
          Write('q = ');
          ReadLn(q0);
        end;
    end;
  repeat
    If Q < 8
    Then
      begin
        i := 1;
        j := 1;
        num[1] := 0;
        den[1] := 1;
        quot[1] := 0;
      end;
    if (Ch = 'd') or (Ch = 'D')
    then
      begin
        contfrac(x, a0, q0);
        t := a0/q0;
        if t < x
        then i := 10
        else i := 11;
        num[i] := a0;
        den[i] := q0;
        quot[i] := t
      end;
    if (Ch = 'r') or (Ch = 'R')
    then
      begin
        t := a0/q0;
        contfrac(t, a0, q0);
        num[10] := a0;
        den[10] := q0;
        quot[10] := a0/q0;
        i := 10
      end;
    if (not funckey) and ((Ch = 'q') or (Ch = 'Q')) and (Q > 7)
    then
      begin
        i := (j + k) div 2;
        contfrac(quot[i], a0, q0);
        num[i] := a0;
        den[i] := q0;
        quot[i] := a0/q0
      end;
    if (funckey) and (Ch = #73)
    then
      begin
        num[20] := num[j];
        den[20] := den[j];
        quot[20] := quot[j];
        i := 20;
        k := 20
      end;
    if (funckey) and (Ch = #81)
    then
      begin
        num[1] := num[k];
        den[1] := den[k];
        quot[1] := quot[k];
        i := 1;
        j := 1
      end;                       {row i of table has been calculated}
    j := i;                      {now complete rest of table}
    while (j > 1) and (quot[j] > 0) do
    begin
      j := j - 1;
      NextSmaller(num[j+1], den[j+1], num[j], den[j]);
      quot[j] := num[j]/den[j]
    end;
    k := i;
    while (k < 20) and (quot[k] < 1) do
    begin
      k := k + 1;
      NextLarger(num[k-1], den[k-1], num[k], den[k]);
      quot[k] := num[k]/den[k]
    end;
    Display;
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
      if funckey
      then
        if (Q > 7) and ((Ch = #73) or (Ch = #81))
        then inputok := true;
      if not funckey
      then
        begin
          if ch in [#27, 'q', 'Q']
          then inputok := true;
          if (Q > 7) and (ch in ['r', 'R', 'd', 'D'])
          then inputok := true;
          if (Q <= PrinterLimit) and ((Ch = 'p') or (Ch = 'P'))
          then inputok := true;
        end;
    until inputok;
    if (not funckey) and ((Ch = 'q') or (Ch = 'Q'))
    then
      begin
        ClrEoL;
        GoToXY(1, 24);
        ClrEoL;
        Write('     Enter new Q = ');
        repeat
          GoToXY(20, 24);
          ClrEoL;
          Read(q)
        until (q > 0) and (q < 1E9);
      end;
    if (Ch = 'd') or (Ch = 'D')
    then
      begin
        ClrEol;
        Write('Center table at the decimal x = ');
        Read(x)
      end;
    if (Ch = 'r') or (Ch = 'R')
    then
      begin
        GoToXY(1, 25);
        ClrEoL;
        GoToXY(1, 24);
        ClrEoL;
        Write('Center table at x = a/q where a = ');
        Read(a0);
        GoToXY(31, 25);
        Write('q = ');
        Read(q0)
      end;
    if (Ch = 'P') or (Ch = 'p') then PrinTab;
  Until (not funckey) and (ch = #27);
TextMode(OriginalMode)
end.

