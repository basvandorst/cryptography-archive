program Hensel;        {Lift solutions of a congruence (mod p) to higher
                        powers of p.}
{$N+,E+}
uses CRT, nothy, printer;

const
MaxTerms = 20;
MaxZeros = 2000;
MaxMod = 2000;

var
a,                              {a residue class}
p,                              {the prime modulus}
m,                              {a modulus}
p0                              {a proposed value of p}
                     : comp;
x                    : extended;

i,                              {an index}
i0,                             {upper limit of index}
h,                              {h = 0 or 1, identifying set of
                                 arrays being operated on}
j,                              {the power of p in top row of display}
k,                              {an index of powers of p}
MaxJ,                           {The largest power of p <= 1E18}
l,                              {length of a string}
r,                              {index of the roots}
t,                              {number of terms in the polynomial}
x0, y0,                         {cursor coordinates}
fzd,                            {index of first zero displayed}
ind,                            {amount to indent by}
z,                              {number of zeros of the polynomial mod p}
w                               {number of singular zeros of the polynomial}
                     : longint;
Coefficient,
Exponent             : array[1..MaxTerms] of comp;
Zero, c              : array[1..MaxZeros] of comp;
pj                   : array[0..64] of comp;
ModSet,                         {Has the modulus been set}
PolySet,                        {Has the polynomial been chosen?}
FuncKey,                        {Was a function key pressed?}
InputOk,                        {Is the key pressed admissible?}
Values                          {Display values?}
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

function deriv(x, m : comp) : comp;

var
a, b,                     {residue classes (mod m)}
d,                        {difference between successive exponents}
y                         {a power of x}
             : comp;
i                         {an index}
             : integer;

begin
  a := coefficient[1]*exponent[1];
  for i := 1 to t - 1 do
    begin
      d := exponent[i] - exponent[i+1];
      if exponent[i+1] = 0
      then d := d - 1;
      y := power(x, d, m);
      a := mult(a, y, m);
      b := mult(coefficient[i+1], exponent[i+1], m);
      a := condition(a + b, m);
    end;
  if exponent[t] > 1
  then
    begin
      y := power(x, exponent[t] - 1, m);
      deriv := mult(a, y, m)
    end
  else
    deriv := a;
end;                                          {of function deriv}

procedure DisplayPoly;

begin
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
    WriteLn
end;                                          {of DisplayPoly}

begin                                         {main body}
OriginalMode := LastMode;
ModSet := False;
PolySet := False;
Values := True;
Ch := 'F';
repeat
  if (not FuncKey) and (UpCase(Ch) = 'D')     {Define Poly}
  then
    begin
      DefPoly;
      PolySet := True
    end;
  if (FuncKey) and (Ch = #80)                          {drop}
  then
    begin
      j := j - 10;
      if j < 10 then j := 10;
      if j > MaxJ then j := MaxJ
    end;                                               {of dropping}
  if (FuncKey) and (Ch = #75)                          {left}
  then
    begin
      fzd := fzd - 3;
      if fzd < 1
      then fzd := 1
    end;                                               {of left}
  if (FuncKey) and (Ch = #72)                          {lift}
  then
    begin
      j := j + 10;
      if j > MaxJ then j := MaxJ
    end;                                               {of lifting}
  if (not FuncKey) and (UpCase(Ch) = 'M')              {Set modulus p}
  then
    begin
      ClrEoL;
      GoToXY(1, 24);
      ClrEoL;
      str(MaxMod:1, St1);
      St1 := '               0 < p ó ' + St1;
      p0 := round(GetInput(1, 24, 'Enter prime modulus p = ',
                          St1, 1, MaxMod));
      ModSet := True;
      if (p0 > 2) and (not spsp(2, p0)) then ModSet := False;
      if (p0 > 3) and (not spsp(3, p0)) then ModSet := False;
      if (p0 > 5) and (not spsp(5, p0)) then ModSet := False;
      if ModSet
      then
        begin
          p := p0;
          MaxJ := -1;
          x := 1;
          while x < MaxAllow do
            begin
              MaxJ := MaxJ + 1;
              pj[MaxJ] := x;
              x := x*p
            end;
        end
      else z := -1
    end;                                         {of set modulus}
  if (FuncKey) and (Ch = #77)                    {move right}
  then
    begin
      fzd := fzd + 3;
      if fzd + 2 > z
      then fzd := z - 2;
      if fzd < 1 then fzd := 1
    end;                                         {of move right}
  if (not FuncKey) and (UpCase(Ch) = 'C')        {switch to coefficients}
  then Values := False;
  if (not FuncKey) and (UpCase(Ch) = 'V')        {switch to values}
  then Values := True;
  if ((not FuncKey) and (UpCase(Ch) IN ['M', 'D']))
      and (ModSet) and (PolySet)                 {find roots and lift them}
  then
    begin
      TextColor(14);
      TextBackground(1);
      ClrScr;
      GoToXY(1, 3);
      DisplayPoly;
      GoToXY(1, 24);
      Write('         Locating all roots of P(x) ð 0 (mod ', p:1:0,') ... ');
      fzd := 1;
      k := 1;
      z := 0;
      w := 0;
      j := 1;
      for i := 0 to round(p) - 1 do
        if poly(i, p) = 0
        then
          begin
            z := z + 1;
            if z <= MaxZeros
            then zero[z] := i;
            a := deriv(i, p);
            if a = 0
            then
              begin
                w := w + 1;
                if z <= MaxZeros
                then c[z] := 0
              end
            else
              if z <= MaxZeros
              then
                begin
                  lincon(-a, 1, p, a, m);
                  c[z] := a
                end;
          end;
      TextColor(14);
      TextBackground(1);
      ClrScr;
      GoToXY(1, 3);
      DisplayPoly;
      GoToXY(1, 10);
      If z = 0
      then
        begin
          ModSet := False;
          Write('     The congruence P(x) ð 0 (mod ', p:1:0);
          WriteLn(') has no solution.')
        end;
      If z = 1
      Then
        begin
          Write('     The congruence P(x) ð 0 (mod ', p:1:0);
          Write(') has one solution, which is ');
          if w = 0
          then Write('non');
          WriteLn('singular.')
        end;
      if z > 1
      then
        begin
          Write('     The congruence P(x) ð 0 (mod ', p:1:0);
          WriteLn(') has ', z:1, ' solutions,');
          if w = 0
          then
            if z = 2
            then WriteLn(' both of which are nonsingular.')
            else WriteLn(' all of which are nonsingular.');
          if w = 1
          then WriteLn(' of which one is singular.');
          if w = z
          then WriteLn(' all of which are singular.');
          if (w > 1) and (w < z)
          then WriteLn(' of which ', w:1, ' are singular.')
        end;
      if z > MaxZeros
      then
        begin
          Write('Because of memory limitations, only the first ');
          WriteLn(MaxZeros:1, ' will be considered.')
        end;
      if z > MaxZeros then z := MaxZeros;
      j := 10;
      if j > MaxJ then j := MaxJ;
      GoToXY(1, 24);
      if z > w                                   {if there are zeros to lift}
      then                                       {then lift them}
        begin
          Write('                            Now lifting the zeros...');
          while k < MaxJ do
            begin
              for i := 1 to z do
                begin
                  a := poly(zero[i], pj[k + 1]);
                  a := a/pj[k];
                  a := mult(a, c[i], p);
                  zero[i] := zero[i] + a*pj[k]
                end;
              k := k + 1
            end;
          GoToXY(1, 24);
          ClrEoL;
          Write('          The zeros have been lifted.')
        end
      else Write('                   ');
      Write('  Press any key to continue...');
      repeat
      until KeyPressed;
    end;                        {The zeros have been found, and arrays set}
  TextColor(14);
  TextBackground(1);
  ClrScr;
  WriteLn('                                  HENSEL');
  WriteLn;
  if PolySet
  then DisplayPoly;
  Write('                              P(x) ð 0 (mod ');
  if ModSet
  then Write(p:1:0)
  else Write('p');
  GoToXY(WhereX, WhereY - 1);
  Write('j');
  GoToXY(WhereX, WhereY + 1);
  Write(')');
  if (ModSet) and (PolySet)
  then
    begin
      GoToXY(79, WhereY);
      WriteLn('j');
      i := j;
      while (i > 0) and (i + 10 > j) do
        begin
          r := 1;
          while (r <= 3) and (r + fzd - 1 <= z) do
            begin
              if (i = 1) or (c[r + fzd - 1] <> 0)
              then
                begin
                  if Values
                  then str(condition(zero[fzd + r - 1], pj[i]):1:0, St1)
                  else
                    begin
                      a := condition(zero[fzd + r - 1], pj[i]);
                      a := a - condition(zero[fzd + r - 1], pj[i-1]);
                      str(a/pj[i-1]:1:0, St1)
                    end;
                  l := length(St1);
                  GoToXY(25*r - 12 - (l div 2), WhereY);
                  Write(St1)
                end;
              r := r + 1
            end;
          GoToXY(78, WhereY);
          if Values
          then WriteLn(i:2)
          else WriteLn((i-1):2);
          if (i > 1) and (i + 11 > j)
          then
            begin
              r := 1;
              while (r <= 3) and (r + fzd - 1 <= z) do
                begin
                  if c[r + fzd -1] <> 0
                  then
                    begin
                      GoToXY(25*r - 12, WhereY);
                      Write('³')
                    end
                  else
                    if (i = 2) and (poly(r + fzd - 1, p*p) = 0)
                    then
                      begin
                        GoToXY(25*r - 17, WhereY);
                        if p = 2 then Write('    ³ ³');
                        if p = 3 then Write('   ³ ³ ³');
                        if p = 5 then Write(' ³ ³ ³ ³ ³');
                        if p > 5 then Write(' ³ ³ ... ³')
                      end;
                  r := r + 1
                end;
              WriteLn
            end;
          i := i - 1
        end;
    end;
  if (UpCase(Ch) = 'M') and (not ModSet) and (z <> 0)
  then
    begin
      TextColor(4);
      TextBackground(7);
      GoToXY(1, 24);
      ClrEoL;
      Write('                ', p0:1:0, ' is not a prime.')
    end;
  TextColor(4);
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  Write('  D');
  TextColor(0);
  Write('efine polynomial    ');
  TextColor(4);
  Write('  M');
  TextColor(0);
  Write('odulus      ');
  if (ModSet) and (PolySet)
  then
    begin
      TextColor(4);
      if Values
      then
        begin
          Write('C');
          TextColor(0);
          Write('oefficients    ')
        end
      else
        begin
          Write('V');
          TextColor(0);
          Write('alues    ')
        end;
      TextColor(4);
      if j < MaxJ
      then Write('  ', #24, '  ');
      if j > 10
      then Write('  ', #25, '  ');
      if fzd > 1
      then Write('  ', #27, '  ');
      if fzd + 2 < z
      then Write('  ', #26, '  ')
    end;
  TextColor(4);
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
    if (ModSet) and (PolySet) and (j < MaxJ) and (FuncKey) and (Ch = #72)
    then InputOk := True;
    if (ModSet) and (PolySet) and (j > 10) and (FuncKey) and (Ch = #80)
    then InputOk := True;
    if (ModSet) and (PolySet) and (fzd > 1) and (FuncKey) and (Ch = #75)
    then InputOk := True;
    if (ModSet) and (PolySet) and (fzd + 2 < z)
       and (FuncKey) and (Ch = #77)
    then InputOk := True;
    if (ModSet) and (PolySet) and (not FuncKey) and (Values)
         and (UpCase(Ch) = 'C')
    then InputOk := True;
    if (ModSet) and (PolySet) and (not FuncKey) and (not Values)
         and (UpCase(Ch) = 'V')
    then InputOk := True;
  until InputOk;
until (not funcKey) and (Ch = #27);
TextMode(OriginalMode)
end.
