program JacobTab;    {Table of values of the Jacobi symbol (P/Q)}
{$N+,E+}
uses crt, printer;

const
limit        = 1E18;
PrinterLimit = 500;                {maximum number of lines in
                                    printed table}
var
h, i,                              {indices}
ind,                               {amount to indent, to center title}
l                                  {length of string,
                                    or line number in table}
             : integer;
j,                                 {requested first value in table}
k,                                 {actual first value}
Q                                  {second argument of Jacobi symbol}
             : comp;
x            : extended;

OriginalMode : word;
Ch           : char;
St           : string[40];
funckey,
inputok,
Neg                                {are there negative values of P
                                    displayed in the table?}
             : Boolean;

function GetOddInput(XCoord, YCoord : integer; prompt, comm : string;
                  min, max : comp) : comp;
var
x            : extended;     {raw input}
x0                          {x coordinate of cursor when ready
                             to read input}
             : integer;
InputOk      : Boolean;

begin                    {body of function GetOddInput}
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(Prompt);
  x0 := WhereX;
  if comm <> ''
  then
    begin
      WriteLn;
      ClrEoL;
      Write(comm);
      GoToXY(x0, WhereY - 1)
    end;
  InputOk := False;
  repeat
    ReadLn(x);
    InputOk := True;
    if frac(x) > 0 then InputOk := False;
    if x < min then InputOk := False;
    if x > max then InputOk := False;
    if frac(x/2) = 0 then InputOk := False;
    if not InputOk
    then
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOk;
  ClrEoL;
  GetOddInput := x
end;             {of function GetOddInput}

function GetInput(XCoord, YCoord : integer; prompt, comm : string;
                  min, max : comp) : comp;
var
x            : extended;     {raw input}
x0                          {x coordinate of cursor when ready
                             to read input}
             : integer;
InputOk      : Boolean;

begin                    {body of function GetInput}
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(Prompt);
  x0 := WhereX;
  if comm <> ''
  then
    begin
      WriteLn;
      ClrEoL;
      Write(comm);
      GoToXY(x0, WhereY - 1)
    end;
  InputOk := False;
  repeat
    ReadLn(x);
    InputOk := True;
    if frac(x) > 0 then InputOk := False;
    if x < min then InputOk := False;
    if x > max then InputOk := False;
    if not InputOk
    then
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOk;
  ClrEoL;
  GetInput := x
end;             {of function GetInput}

function condition(a, m : comp) : comp;

var
b, t     : comp;

begin
  if m <= 0
  then
    begin
      WriteLn('The modulus ', m:1:0, ' must be positive.');
      Halt
    end;
  if m > limit
  then
    begin
      WriteLn('The modulus ', m:1:0, ' must not exceed 1E18.');
      Halt
    end;
  t := a/m;
  b := a - t*m;                {Thus b ð a (mod m), ³b³ ó m/2}
  if b < 0
  then b := b + m;             {Force 0 ó b < m}
  condition := b
end;                {of function condition}

function Jacobi(P, Q : comp) : integer;

var
t, u               : comp;
J,                               {The Jacobi symbol being formed}
k
                   : integer;
x                  : extended;

begin
  x := Q/2;
  if frac(x) = 0
  then
    begin
      WriteLn('Cannot compute Jacobi symbol (P/Q) when Q is even.');
      WriteLn('Here P = ', P:1:0,', and Q = ', Q:1:0,'.');
      Halt
    end;
  if Q < 1
  then
    begin
      WriteLn('The Jacobi symbol (P/Q) is undefined when Q < 1.');
      WriteLn('Here P = ', P:1:0, ', and Q = ', Q:1:0, '.');
      Halt
    end;
  if P = 0
  then
    begin
      Jacobi := 0;
      Exit
    end;
  J := 1;
  if P < 0
  then
    begin
      P := -P;
      t := Q/4;
      if Q < 4*t                  {if Q ð 3 (mod 4)}
      then J := -J
    end;
while Q > 1 do
  begin
    P := condition(P, Q);
    if P = 0
    then
      begin
        Jacobi := 0;
        exit
      end;
    k := 0;
    x := P/2;
    while frac(x) = 0 do          {while P is even}
      begin
        P := x;
        x := P/2;
        k := k + 1
      end;                        {k is now the power of 2 in P}
    if k mod 2 = 1                {if k is odd}
    then
      begin
        t := Q/8;
        t := Q - 8*t;
        if (t = -3) or (t = 3)    {if Q ð ñ3 (mod 8)}
        then J := -J
      end;
    t := Q;                       {exchange P and Q}
    Q := P;
    P := t;
    t := P/4;
    t := P - 4*t;
    u := Q/4;
    u := Q - 4*u;
    if (t = -1) and (u = -1)      {if P ð Q ð 3 (mod 4)}
    then J := -J
  end;
  Jacobi := J
end;                              {of function Jacobi}

begin                             {main body}
OriginalMode := LastMode;
TextBackground(1);
TextColor(14);
ClrScr;
WriteLn;
WriteLn('          Will exhibit values of the Jacobi symbol (P/Q)');
Write('     for specified Q and varying P.  ');
WriteLn('Q must be odd, 1 ó Q < 10^18.');
WriteLn;
Q := GetOddInput(WhereX, WhereY, '     Enter initial value Q = ',
     '     (Q odd, 0 < Q < 10^18)', 1, limit - 1);
Ch := 'F';
j := -90;
funckey := false;
Repeat
  if (funckey) and (Ch = #73)     {PgUp}
  then
    begin
      if (j >= 0) and (j - 200 < 0)
      then j := j - 199
      else j := j - 200;
      if j < -limit
      then j := -limit
    end;
  if (funckey) and (Ch = #81)     {PgDn}
  then
    begin
      if (j < 0) and (j + 200 >= 0)
      then j := j + 199
      else j := j + 200;
      if j > Limit - 200 then j := limit - 200;
    end;
  if (not funckey) and (Upcase(Ch) = 'J')
  then
    begin
      j := GetInput(1, 24, 'Jump to P = ',
            '  (-10^18 < P < 10^18)', -limit + 1, limit - 1);
      if j > Limit - 200
      then j := Limit - 200;
    end;
  if (not funckey) and (Upcase(Ch) = 'Q')
  then q := GetOddInput(1, 24, 'Enter new Q = ',
                        '(Q odd, 0 < Q < 10^18)', 1, limit - 1);
  TextColor(14);
  TextBackground(1);
  ClrScr;
  Str(Q:1:0, St);
  l := length(St);
  ind := (62 - l) div 2;
  for i := 1 to ind do Write(' ');
  WriteLn('JACOBI SYMBOL (P/', Q:1:0, ')');
  WriteLn;
  Str(Abs(j):1:0, St);
  l := length(St);
  ind := (36 - l) div 2;
  for i := 1 to ind do Write(' ');
  Write('P');
  for i := 1 to (19 - ind) do Write(' ');
  Write('     0    1    2    3    4    5');
  WriteLn('    6    7    8    9');
  WriteLn;
  x := j;
  x := x/10;
  k := Int(x);
  l := 0;
  i := 0;
  if j < 0
  then Neg := True
  else Neg := False;
  while l < 20 do
    begin
      if  i+k <> 0
      then Write((i+k):18:0, '   ');
      if (i+k = 0) and Neg
      then Write('                 -   ');
      if (i+k = 0) and (not Neg)
      then Write('                     ');
      for h := 0 to 9 do
        if Neg
        then Write(Jacobi(10*(i+k)-h, Q):5)
        else Write(Jacobi(10*(i+k)+h, Q):5);
      WriteLn;
      if (i+k = 0) and Neg
      then Neg := False
      else i := i + 1;
      l := l + 1
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write('    PgUp          PgDn          J');
  TextColor(0);
  Write('ump');
  TextColor(4);
  Write('          Q          P');
  TextColor(0);
  Write('rint');
  TextColor(4);
  Write('          Esc');
  GoToXY(1, 25);                                    {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0);
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
    if (not funckey) and (Ch IN [#27, 'j', 'J', 'p', 'P', 'q', 'Q'])
    then inputok := true
  until inputok;
  if (not funckey) and (UpCase(Ch) = 'P')      {Print}
  then
    begin
      GoToXY(1, 25);
      ClrEoL;
      Write('                         Printing ... ');
      Str(Q:1:0, St);
      l := length(St);
      ind := (62 - l) div 2;
      for i := 1 to ind do Write(lst, ' ');
      WriteLn(lst, 'JACOBI SYMBOL (P/', Q:1:0, ')');
      WriteLn(lst);
      Str(Abs(j):1:0, St);
      l := length(St);
      ind := (36 - l) div 2;
      for i := 1 to ind do Write(lst, ' ');
      Write(lst, 'P');
      for i := 1 to (19 - ind) do Write(lst, ' ');
      Write(lst, '     0    1    2    3    4    5');
      WriteLn(lst, '    6    7    8    9');
      WriteLn(lst);
      l := 0;
      i := 0;
      if j < 0
      then Neg := True
      else Neg := False;
      while l < PrinterLimit do
        begin
          if  i+k <> 0
          then Write(lst, (i+k):18:0, '   ');
          if (i+k = 0) and Neg
          then Write(lst, '                 -   ');
          if (i+k = 0) and (not Neg)
          then Write(lst, '                     ');
          for h := 0 to 9 do
            if Neg
            then Write(lst, Jacobi(10*(i+k)-h, Q):5)
            else Write(lst, Jacobi(10*(i+k)+h, Q):5);
          WriteLn(lst);
          if (i+k = 0) and Neg
          then Neg := False
          else i := i + 1;
          l := l + 1
        end;
      WriteLn(lst);
      Write(' done.')
    end;
until (not funckey) and (Ch = #27);
TextMode(OriginalMode)
end.

