program polysolv;      {Evaluate a polynomial at all residue classes,
                        count the number of roots, and record as many
                        as possible.}
{$N+,E+}
uses CRT, nothy, printer;

const
MaxMod = 9999;
MaxTerms = 20;
MaxZeros = 100;

var
i,                              {an index}
i0,                             {upper limit of index}
l,                              {length of a string}
m,                              {the modulus}
t,                              {number of terms in the polynomial}
x0, y0,                         {cursor coordinates}
z                               {number of zeros counted}
                     : longint;
Coefficient,
Exponent             : array[1..MaxTerms] of comp;
Zero                 : array[1..MaxZeros] of comp;
ModSet,                         {Has the modulus been set}
PolySet,                        {Has the polynomial been chosen?}
ZerosCounted,                   {Have the zeros been counted?}
FuncKey,                        {Was a function key pressed?}
InputOk                         {Is the key pressed admissible?}
                     : Boolean;
OriginalMode         : word;
Ch                              {input for keyboard}
                     : char;
St1, St2, St3        : string[80];

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
end;                                  {of procedure GetInput}

procedure DefPoly;

var
c              : comp;

begin
  t := 0;
  TextColor(14);
  TextBackground(1);
  ClrScr;
  x0 := 5;
  y0 := 10;
  GoToXY(5, 20);
  Write('Enter the monomial terms of the polynomial,');
  Write(' with exponents decreasing.');
  t := 0;
  c := 1;
  while (t < MaxTerms) and (c <> 0) do
    begin
      str((t+1):1, St1);
      St2 := 'Enter coefficient of term #' + St1 + ': ';
      c := GetInput(5, 22, St2, '     Put coefficient = 0 to terminate.',
                    -1E18, 1E18);
      if c <> 0
      then
        begin
          t := t + 1;
          coefficient[t] := c;
          St2 := 'Enter exponent of term #' + St1 + ': ';
          if t = 1
          then c := GetInput(5, 22, St2, '', 0, 1E18)
          else
            begin
              str(Exponent[t-1]:1:0, St3);
              St3 := '                0 ó exponent < ' + St3;
              c := GetInput(5, 22, St2, St3, 0, Exponent[t-1] - 1)
            end;
          exponent[t] := c;
          str(Abs(coefficient[t]):1:0, St1);
          l := length(St1);
          str(exponent[t]:1:0, St1);
          l := l + length(St1);
          if l + x0 > 75
          then y0 := y0 + 3;
          GoToXY(x0, y0);
          if (coefficient[t] > 0) and (t > 1)
          then Write(' + ');
          if coefficient[t] < 0
          then
            if t > 1
            then Write(' - ')
            else Write(' -');
          if (Abs(coefficient[t]) > 1) or (exponent[t] = 0)
          then Write(Abs(coefficient[t]):1:0);
          if exponent[t] > 0
          then Write('x');
          if exponent[t] > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(exponent[t]:1:0);
              GoToXY(WhereX, WhereY + 1)
            end;
          x0 := WhereX;
          y0 := WhereY;
        end;
    end;
end;                               {of procedure DefPoly}

function poly(x, m : comp) : comp;

var
a,                        {a residue class (mod m)}
d,                        {difference between successive exponents}
y                         {a power of x}
             : comp;
i                         {an index}
             : integer;

begin
  a := coefficient[1];
  for i := 1 to t - 1 do
    begin
      d := exponent[i] - exponent[i+1];
      y := power(x, d, m);
      a := mult(a, y, m);
      a := condition(a + coefficient[i+1], m);
    end;
  y := power(x, exponent[t], m);
  poly := mult(a, y, m)
end;                                          {of function poly}

begin                        {main body}
OriginalMode := LastMode;
ModSet := False;
PolySet := False;
ZerosCounted := False;
Ch := 'F';
repeat
  if (not FuncKey) and (UpCase(Ch) = 'C')     {Count the zeros}
  then
    begin
      ClrEoL;
      TextColor(0);
      Write('                         Counting the zeros ... ');
      z := 0;
      for i := 0 to m - 1 do
        if poly(i, m) = 0
        then
          begin
            z := z + 1;
            if z <= MaxZeros
            then Zero[z] := i;
          end;
      ZerosCounted := True
    end;
  if (not FuncKey) and (UpCase(Ch) = 'D')     {Define Poly}
  then
    begin
      DefPoly;
      PolySet := True;
      ZerosCounted := False
    end;
  if (not FuncKey) and (UpCase(Ch) = 'M')      {Set modulus m}
  then
    begin
      ClrEoL;
      GoToXY(1, 24);
      ClrEoL;
      str(MaxMod:1, St1);
      St1 := '  0 < m ó ' + St1;
      m := round(GetInput(1, 24, 'Enter m = ', St1, 1, MaxMod));
      ModSet := True;
      ZerosCounted := False
    end;
  if (not FuncKey) and (UpCase(Ch) = 'P')     {Print the data}
  then
    begin
      TextColor(0);
      ClrEoL;
      Write('                                Printing data ... ');
      WriteLn(lst);
      Write(lst, '     Let P(x) = ');
      for i := 1 to t do
        begin
          str(Abs(coefficient[i]):1:0, St1);
          l := length(St1);
          str(exponent[i]:1:0, St1);
          l := l + length(St1);
          if l + WhereX > 75
          then
            begin
              WriteLn(lst);
              WriteLn(lst)
            end;
          if (coefficient[i] > 0) and (i > 1)
          then Write(lst, ' + ');
          if (coefficient[i] < 0)
          then
            if i = 1
            then Write(lst, ' -')
            else Write(lst, ' - ');
          if (Abs(coefficient[i]) > 1) or (exponent[i] = 0)
          then Write(lst, Abs(coefficient[i]):1:0);
          if exponent[i] > 0
          then Write(lst, 'x');
          if exponent[i] > 1
          then Write(lst, '^', exponent[i]:1:0);
        end;
      WriteLn(lst, '.');
      Write(lst, 'The congruence P(x) ð 0 (mod ', m:1, ') has ');
      if z = 0
      then WriteLn(lst, 'no solution.')
      else
        begin
          Write(lst, 'exactly ', z:1, ' solution');
          if z > 1
          then WriteLn(lst, 's, ')
          else WriteLn(lst, ', ');
          if z <= MaxZeros
          then Write(lst, 'namely ')
          else Write(lst, 'of which the first ', MaxZeros:1, ' are ');
          WriteLn(lst, 'x = ');
          i := 0;
          while i < i0 do
            begin
              St1 := '';
              str(zero[i+1]:1:0, St2);
              while (length(St1 + St2) < 78) and (i < i0) do
                begin
                  St1 := St1 + St2;
                  i := i + 1;
                  if i < i0
                  then
                    begin
                      St1 := St1 + ', ';
                      str(zero[i+1]:1:0, St2)
                    end
                  else St1 := St1 + '.'
                end;
              WriteLn(lst, St1);
            end;
        end;
      WriteLn(lst)
    end;                     {of print}
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('                            POLYNOMIAL  SOLVER');
  WriteLn;
  Write('     Will find all roots of P(x) ð 0 (mod ');
  if ModSet
  then Write(m:1)
  else Write('m');
  Write(')');
  if PolySet
  then WriteLn(', where')
  else WriteLn('.');
  if PolySet
  then
    begin
      WriteLn;
      WriteLn;
      Write(' P(x) = ');
      for i := 1 to t do
        begin
          str(Abs(coefficient[i]):1:0, St1);
          l := length(St1);
          str(exponent[i]:1:0, St1);
          l := l + length(St1);
          if l + WhereX > 75
          then
            begin
              WriteLn;
              WriteLn;
              WriteLn;
              Write(' ')
            end;
          if (coefficient[i] > 0) and (i > 1)
          then Write(' + ');
          if (coefficient[i] < 0)
          then
            if i = 1
            then Write(' -')
            else Write(' - ');
          if (Abs(coefficient[i]) > 1) or (exponent[i] = 0)
          then Write(Abs(coefficient[i]):1:0);
          if exponent[i] > 0
          then Write('x');
          if exponent[i] > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(exponent[i]:1:0);
              GoToXY(WhereX, WhereY + 1)
            end;
        end;
        WriteLn;
        WriteLn
    end;
  if ZerosCounted
  then
    begin
      Write(' This congruence has ');
      if z = 0
      then Write('no solution.')
      else
        begin
          Write('exactly ', z:1, ' solution');
          if z > 1 then Write('s,');
          if z > MaxZeros
          then WriteLn(' of which the first ', MaxZeros:1, ' are')
          else WriteLn(' namely');
          WriteLn;
          Write(' x =');
          i0 := z;
          if i0 > MaxZeros
          then i0 := MaxZeros;
          for i := 1 to i0 do
            begin
              str(zero[i]:1:0, St1);
              if WhereX + length(St1) > 76
              then WriteLn;
              Write(' ', zero[i]:1:0);
              if i < i0
              then Write(',')
              else Write('.')
            end;
        end;
    end;
  TextColor(4);
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  if (ModSet) and (PolySet) and (not ZerosCounted)
  then
    begin
      Write('  C');
      TextColor(0);
      Write('ount zeros           ');
      TextColor(4)
    end;
  Write('  D');
  TextColor(0);
  Write('efine polynomial           ');
  TextColor(4);
  Write('  M');
  TextColor(0);
  Write('odulus           ');
  TextColor(4);
  if ZerosCounted
  then
    begin
      Write('  P');
      TextColor(0);
      Write('rint           ');
      TextColor(4)
    end;
  Write('  Esc');
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
    if (not FuncKey) and (Ch IN [#27, 'd', 'D', 'm', 'M'])
    then InputOk := true;
    if (not FuncKey) and (ModSet) and (PolySet)
        and (not ZerosCounted) and (UpCase(Ch) = 'C')
    then InputOk := True;
    if (not FuncKey) and (ZerosCounted) and (UpCase(Ch) = 'P')
    then InputOk := True;
  until InputOk;
until (not funcKey) and (Ch = #27);
TextMode(OriginalMode)
end.