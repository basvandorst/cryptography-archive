program EuAlDem3;  {Display Euclidean Algorithm, with determination of
                    x and y such that bx + cy = g, and rounding to the
                    nearest integer.}
{$N+,E+}
uses CRT, printer;
{$I GetInput.i}

const
MaxAllow = 1E18;

var
b, c,                             {numbers whose gcd will be calculated}
t                                 {temporary variable}
            : comp;
q,                                {quotients}
r,                                {remainders}
x,                                {coefficients of b}
y                                 {coefficients of c}
            : array[-1..100] of comp;
i, j,                             {indices}
j0,                               {last index in arrays}
l,                                {length of string}
ind                               {amount to indent by}
            : integer;
OriginalMode                      {save the original text mode}
            : word;

Ch          : char;
St          : string[22];
FuncKey,
InputOk     : Boolean;

begin
OriginalMode := LastMode;
TextColor(14);
TextBackground(1);
ClrScr;
WriteLn;
WriteLn('          Will calculate g = (b, c) for positive integers < 10^18,');
WriteLn('     and will find x and y so that bx + cy = g.');
WriteLn;
b := GetInput(WhereX, WhereY, '     Enter b = ',
     'where 0 < b < 1000000000000000000', 1, MaxAllow - 1);
c := GetInput(WhereX, WhereY, '     Enter c = ',
     'where 0 < c < 1000000000000000000', 1, MaxAllow - 1);

Ch := 'F';
FuncKey := false;
Repeat
  if (not FuncKey) and (UpCase(Ch) = 'B')
  then
    begin
      b := GetInput(1, 24, 'Enter new value of b = ',
              '        where 0 < b < 1000000000000000000', 1, MaxAllow - 1);
      j := -1
    end;
  if (not FuncKey) and (UpCase(Ch) = 'C')
  then
    begin
      c := GetInput(1, 24, 'Enter new value of c = ',
              '        where 0 < b < 1000000000000000000', 1, MaxAllow - 1);
      j := -1
    end;
  if not FuncKey
  then
    begin    {compute arrays}
      r[-1] := b; r[0] := c; x[-1] := 1;
      x[0] := 0; y[-1] := 0; y[0] := 1; i := 0;
      repeat
        i := i + 1;
        q[i] := r[i-2]/r[i-1];  {integer quotient, rounded to nearest int}
        r[i] := r[i-2] - q[i]*r[i-1];
        x[i] := x[i-2] - q[i]*x[i-1];
        y[i] := y[i-2] - q[i]*y[i-1]
      until r[i] = 0;
      j0 := i;
      j := -1
    end;            {computation of arrays is complete}
  if (funckey) and (Ch = #73)     {PgUp}
  then
    begin
      j := j - 20;
      if j < -1 then j := -1;
    end;
  if (funckey) and (Ch = #81)     {PgDn}
  then
    begin
      j := j + 20;
      if j > j0 - 16 then j := j0 - 16;
    end;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('                          THE EUCLIDEAN ALGORITHM');
  WriteLn;
  Write(' i             q(i+1)          r(i)');
  WriteLn('                x(i)               y(i)');
  i := j;
  while (i <= j0) and (i <= j + 19) do
    begin
      Write(i:2);
      if (i = -1) or (i = j0)
      then Write('                   ')
      else Write(q[i+1]:19:0);
      Write(r[i]:19:0, x[i]:20:0, y[i]:20:0);
      i := i + 1
    end;
  if WhereY <= 22
  then
    begin
      GoToXY(1, 22);
      TextColor(0);
      TextBackground(7);
      ClrEoL;
      str(b:1:0, St);
      l := 7 + length(St);
      str(c:1:0, St);
      l := l + length(St);
      str(r[j0-1]:1:0, St);
      l := l + length(St);
      ind := (80 - l) div 2;
      GoToXY(ind, WhereY);                  {center the answer on the line}
      WriteLn('(', b:1:0, ', ', c:1:0, ') = ', r[j0-1]:1:0);
      ClrEoL;
      str(x[j0-1]:1:0, St);
      l := l + length(St);
      str(y[j0-1]:1:0, St);
      l := l + length(St);
      l := l + 5;
      ind := (80 - l) div 2;
      GoToXY(ind, WhereY);                  {center the answer on the line}
      Write('(', x[j0-1]:1:0, ')ù', b:1:0, ' + (', y[j0-1]:1:0, ')ù');
      Write(c:1:0, ' = ', r[j0-1]:1:0);
      ClrEoL
    end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  if j0 < 17
  then
    begin
      Write('         b                  c                 P');
      TextColor(0);
      Write('rint                 ');
      TextColor(4);
      Write('Esc')
    end
  else
    begin
      Write('      PgUp          PgDn          b          c          P');
      TextColor(0);
      Write('rint          ');
      TextColor(4);
      Write('Esc')
    end;
  GoToXY(1, 25);                                    {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0);
  repeat
    Ch := ReadKey;
    if Ch <> #0
    then FuncKey := false
    else
      begin
        FuncKey := true;
        Ch := ReadKey
      end;
    InputOk := false;
    if FuncKey and ((Ch = #73) or (Ch = #81)) and (j0 > 16)
    then InputOk := true;
    if (not FuncKey) and (Ch IN [#27, 'b', 'B', 'c', 'C', 'p', 'P'])
    then InputOk := true
  until InputOk;
  if (not FuncKey) and (UpCase(Ch) = 'P')
  then
    begin
      WriteLn(lst, '                          THE EUCLIDEAN ALGORITHM');
      WriteLn(lst);
      Write(lst, ' i             q(i+1)          r(i)');
      WriteLn(lst, '                x(i)               y(i)');
      for i := -1 to j0 do
        begin
          Write(lst, i:2);
          if (i = -1) or (i = j0)
          then Write(lst, '                   ')
          else Write(lst, q[i+1]:19:0);
          WriteLn(lst, r[i]:19:0, x[i]:20:0, y[i]:20:0)
        end;
      WriteLn(lst);
      str(b:1:0, St);
      l := 7 + length(St);
      str(c:1:0, St);
      l := l + length(St);
      str(r[j0-1]:1:0, St);
      l := l + length(St);
      ind := (80 - l) div 2;
      for i := 1 to ind do Write(lst, ' ');  {center the answer on the line}
      WriteLn(lst, '(', b:1:0, ', ', c:1:0, ') = ', r[j0-1]:1:0);
      str(x[j0-1]:1:0, St);
      l := l + length(St);
      str(y[j0-1]:1:0, St);
      l := l + length(St);
      l := l + 5;
      ind := (80 - l) div 2;
      for i := 1 to ind do Write(lst, ' ');  {center the answer on the line}
      Write(lst, '(', x[j0-1]:1:0, ')ù', b:1:0, ' + (', y[j0-1]:1:0, ')ù');
      Write(lst, c:1:0, ' = ', r[j0-1]:1:0);
      WriteLn(lst)
    end;
until (not FuncKey) and (Ch = #27);
TextMode(OriginalMode)
end.