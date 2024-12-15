program LucasDem;                {Demonstrates the computation of the
                                  Lucas functions U(n), V(n)
                                  (mod m), where U(0) = 0, U(1) = 1,
                                  V(0) = 2, V(1) = a,
                                  U(n+1) = aU(n) + bU(n-1),
                                  V(n+1) = aV(n) + bV(n-1)}
{$N+,E+}
uses nothy, CRT;

{$I GetInput.i }

type
BinDigits       =  string[80];

var
n,                        {The argument}
n0,                       {second copy of the argument, to be operated on}
a, b,                     {parameters}
c,                        {a power of -b}
u, v,                     {The values}
m,                        {The modulus}
t, t0, t1,                {temporary variables}
u0, u1,                   {Lucas' U(k-1), U(k)}
v0, v1,                   {Lucas' V(k), V(k+1)}
r,                        {The odd part of n}
j                         {The least power of 2 greater than r}

            : comp;
x           : extended;
i,                        {the power of 2 dividing n}
h,                        {an index}
k,                        {an index}
l,                        {l = length(Binaryn}
code,                     {Error code in translating a string
                           to a real number}
x0                        {cursor coordinate}
            : integer;
Ch          : char;
ParamsSet,
FuncKey,
InputOk,
first       : Boolean;
St,
Binaryn,
ExpStr1,
ExpStr2
            : BinDigits;


(*
e,                        {= 0 or 1 -- the value of k (mod 2)}
q,                        {quotient}
*)

procedure Prompt;

var
x0, y0                    {coordinates of cursor location}
         : integer;
Ch       : Char;

begin
  x0 := WhereX;
  y0 := WhereY;
  if y0 = 25
  then
    begin
      WriteLn;
      y0 := 24
    end;
  GoToXY(1, 25);
  ClrEoL;
  Write('                         Press any key to continue ... ');
  Ch := ReadKey;
  if Ch = #0
  then Ch := ReadKey;
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(x0, y0)
end;                               {of procedure Prompt}

function BinExpan(n:comp) : BinDigits;

var
n0,                       {second copy of n, to be operated on}
t                         {least power of 2 > n}
                : comp;
i,                        {an index}
j,                        {number of leading bits}
l                         {length of the binary expansion}
                : integer;
St              : string[80];

begin
  t := 1;
  l := 0;
  while t <= n do
    begin
      t := 2*t;
      l := l + 1
    end;                     {t is now the least power of 2 > n}
  St := '';                  {l will be length(BinExpan) }
  n0 := n;                   {when BinExpan has been determined.}
  j := 0;
  while n0 > 0 do
    begin
      j := j + 1;
      n0 := 2*n0;
      if n0 >= t
      then
        begin
          n0 := n0 - t;
          St := St + '1'
        end
      else St := St + '0';
    end;
                        {All the non-zero digits in the binary expansion}
                        {of n are now in St, but the trailing 0's}
                        {must be appended.  There are l - j trailing 0's}
  for i := 1 to l - j do St := St + '0';
  if St = ''
  then St := '0';
  BinExpan := St
end;                               {of function BinExpan}

procedure Double;

begin
  WriteLn('We now double the index:');
  Write('   ', Ch, '(', BinExpan(2*k-1), ')');
  x0 := WhereX;
  Write(' = ', Ch, '(', BinExpan(k), ')ý');
  if b <> 0
  then
    begin
      if b > 0 then Write(' + ')
      else Write(' - ');
      if Abs(b) > 1
      then Write(Abs(b):1:0);
      WriteLn(Ch, '(', BinExpan(k-1), ')ý')
    end
  else WriteLn;
  GoToXY(x0, WhereY);
  t := mult(b, u0, m);
  t0 := mult(u1, u1, m) + mult(t, u0, m);
  t := 2*t + mult(a, u1, m);
  t1 := mult(u1, t, m);
  u0 := condition(t0, m);
  u1 := t1;
  WriteLn(' ð ', u0:1:0, ' (mod ', m:1:0, '),');
  Write('   ', Ch, '(', BinExpan(2*k), ')');
  x0 := WhereX;
  Write(' = ');
  if b <> 0
  then
    begin
      Write((2*b):1:0, Ch, '(', BinExpan(k-1), ')');
      Write(Ch, '(', BinExpan(k), ')')
    end;
  if a <> 0
  then
    begin
      if a > 0
      then Write(' + ')
      else Write(' - ');
      if Abs(a) > 1
      then Write(Abs(a):1:0);
      WriteLn(Ch, '(', BinExpan(k), ')ý')
    end
  else WriteLn;
  GoToXY(x0, WhereY);
  WriteLn(' ð ', u1:1:0, ' (mod ', m:1:0, ').');
  Prompt;
  k := 2*k
end;                           {of procedure Double}

procedure SideStep;

begin
  WriteLn('We now increase the index by 1 ("sidestep"):');
  Write('   ', Ch, '(', BinExpan(k), ') ð ');
  WriteLn(u1:1:0, ' (mod ', m:1:0, '),');
  t0 := u1;
  t1 := mult(a, u1, m) + mult(b, u0, m);
  u0 := t0;
  u1 := condition(t1, m);
  Write('   ', Ch, '(', BinExpan(k+1), ')');
  x0 := WhereX;
  Write(' = ');
  if a <> 0
  then
    begin
      if a < 0
      then Write(' - ');
      if Abs(a) > 1
      then Write(Abs(a):1:0);
      Write(Ch, '(', BinExpan(k), ')')
    end;
  if b <> 0
  then
    begin
      if b > 0
      then Write(' + ')
      else Write(' - ');
      if Abs(b) > 1
      then Write(Abs(b):1:0);
      WriteLn(Ch, '(', BinExpan(k-1), ')')
    end
  else WriteLn;
  GoToXY(x0, WhereY);
  WriteLn(' ð ', u1:1:0, ' (mod ', m:1:0, ').');
  k := k + 1;
  Prompt
end;

begin                              {main}
  ParamsSet := False;
  If ParamCount = 4
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 2) and (x <= MaxAllow)
      then n := x
      else ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a := x
      else ParamsSet := False;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then b := x
      else ParamsSet := False;
      Val(ParamStr(4), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 1) and (x <= MaxAllow)
      then m := x
      else ParamsSet := False
    end;
  if ParamCount = 2
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 2) and (x <= MaxAllow)
      then n := x
      else ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 1) and (x <= MaxAllow)
      then m := x
      else ParamsSet := False;
      a := 1;
      b := 1
    end;
  if not ParamsSet
  then
    begin
      WriteLn;
      WriteLn('Will compute U(n; a, b), V(n; a, b) (mod m) where');
      WriteLn('U(0) = 0, U(1) = 1, V(0) = 2, V(1) = a,');
      WriteLn('U(n+1) = aU(n) + bU(n-1), V(n+1) = aV(n) + bV(n-1)');
      n := GetInput(WhereX, WhereY, 'Enter index n = ',
      'where 2 ó n ó 1000000000000000000', 0, MaxAllow);
      a := GetInput(WhereX, WhereY, 'Enter parameter a = ',
      '       where ³a³ ó 1000000000000000000', -MaxAllow, MaxAllow);
      b := GetInput(WhereX, WhereY, 'Enter parameter b = ',
      '       where ³b³ ó 1000000000000000000', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'Enter modulus m = ',
      '   where 1 ó m ó 1000000000000000000', 1, MaxAllow);
    end;
  if (a = 1) and (b = 1)
  then Write('Choose between F(n) and L(n) : ')
  else Write('Choose between U(n) and V(n) : ');
  repeat
    Ch := ReadKey;
    if Ch <> #0
    then FuncKey := False
    else
      begin
        FuncKey := True;
        Ch := ReadKey
      end;
    InputOk := False;
    if (a = 1) and (b = 1) and (not FuncKey) and (UpCase(Ch) = 'F')
    then
      begin
        InputOk := True;
        First := True
      end;
    if (a = 1) and (b = 1) and (not FuncKey) and (UpCase(Ch) = 'L')
    then
      begin
        InputOk := True;
        First := False
      end;
    if ((a <> 1) or (b <> 1)) and (not FuncKey) and (UpCase(Ch) = 'U')
    then
      begin
        InputOk := True;
        First := True
      end;
    if ((a <> 1) or (b <> 1)) and (not FuncKey) and (UpCase(Ch) = 'V')
    then
      begin
        InputOk := True;
        First := False
      end;
  until InputOk;
  WriteLn(UpCase(Ch));
  WriteLn;
  WriteLn('     For clarity, as the calculation is performed, all indices');
  WriteLn('are displayed in binary.  The desired index is');
  WriteLn('      n = ', n:1:0, ' (decimal)');

  WriteLn('        = ', BinExpan(n) + ' (binary)');
  WriteLn;
  Prompt;
  if First
  then
    begin
      if (a = 1) and (b = 1)
      then Ch := 'F'
      else Ch := 'U';
      i := 0;
      r := n;
      x := r/2;
      while frac(x) = 0 do
        begin
          i := i + 1;
          r := x;
          x := r/2
        end;                      {n = r*2^i}
      j := 1;
      while j <= r do j := 2*j;
      u0 := 0;
      u1 := 1;
      WriteLn('We begin with');
      WriteLn('   ', Ch, '(0) = 0,');
      WriteLn('   ', Ch, '(1) = 1.');
      Prompt;
      r := 2*r;
      r := r - j;
      k := 1;
      while r > 0 do
        begin
          r := 2*r;
          Double;
          if r >= j         {binary digit is 1}
          then
            begin
              r := r - j;
              SideStep
            end;
        end;
      for h := 1 to i do Double;
      WriteLn;
      WriteLn('That is, in decimal notation,');
      Write('   ', Ch, '(', n:1:0, ') ð ', u1:1:0, ' (mod ', m:1:0, ')');
      if Ch = 'F'
      then WriteLn('.')
      else
        begin
          WriteLn;
          WriteLn('when a = ', a:1:0, ' and b = ', b:1:0, '.')
        end
    end
  else
    begin
      if (a = 1) and (b = 1)
      then Ch := 'L'
      else Ch := 'V';
      i := 0;
      c := 1;
      r := n;
      x := r/2;
      while frac(x) = 0 do
        begin
          i := i + 1;
          r := x;
          x := r/2
        end;                      {n = r*2^i}
      j := 1;
      while j <= r do j := 2*j;
       v0 := 2;
      v1 := a;
      WriteLn('We begin with');
      WriteLn('   ', Ch, '(0) = 2,');
      WriteLn('   ', Ch, '(1) = ', a:1:0, '.');
      Prompt;
      k := 0;
      while r > 0 do
        begin
          r := 2*r;
          if r >= j         {binary digit is 1}
          then
            begin
              r := r - j;
              t0 := mult(v0, v1, m) - mult(a, c, m);
              t := mult(c, -b, m);
              t1 := mult(v1, v1, m) - 2*t;
              c := mult(c, t, m);
              v0 := condition(t0, m);
              v1 := condition(t1, m);
              Write('Next we replace the index k by 2k+1');
              WriteLn(' (double with sidestep):');
              WriteLn;
              Write('   ', Ch, '(', BinExpan(2*k+1), ')');
              x0 := WhereX;
              Write(' = ', Ch, '(', BinExpan(k), ')');
              Write(Ch, '(', BinExpan(k+1), ')');
              if (a <> 0) and (b <> 0)
              then
                begin
                  if a > 0
                  then Write(' - ')
                  else Write(' + ');
                  if Abs(a) > 1
                  then Write(Abs(a):1:0);
                  Write('(', -b:1:0, ')');
                  GoToXY(WhereX, WhereY - 1);
                  Write(BinExpan(k));
                  GoToXY(WhereX, WhereY + 1);
                  WriteLn
                end
              else WriteLn;
              GoToXY(x0, WhereY);
              WriteLn(' ð ', v0:1:0, ' (mod ', m:1:0, '),');
              WriteLn;
              Write('   ', Ch, '(', BinExpan(2*k+2),  ')');
              x0 := WhereX;
              Write(' = ', Ch, '(', BinExpan(k+1), ')ý - 2(', -b:1:0, ')');
              GoToXY(WhereX, WhereY - 1);
              Write(BinExpan(k+1));
              GoToXY(x0, WhereY + 1);
              WriteLn;
              GoToXY(x0, WhereY);
              WriteLn(' ð ', v1:1:0, ' (mod ', m:1:0, ').');
              k := 2*k + 1;
              Prompt
            end
          else
            begin
              t0 := mult(v0, v0, m) - 2*c;
              t1 := mult(v0, v1, m) - mult(a, c, m);
              c := mult(c, c, m);
              v0 := condition(t0, m);
              v1 := condition(t1, m);
              WriteLn('Next we double the index:');
              WriteLn;
              Write('   ', Ch, '(', BinExpan(2*k), ')');
              x0 := WhereX;
              Write(' = ', Ch, '(', BinExpan(k), ')ý - 2(', -b:1:0, ')');
              GoToXY(WhereX, WhereY - 1);
              Write(BinExpan(k));
              GoToXY(x0, WhereY + 1);
              WriteLn;
              GoToXY(x0, WhereY);
              WriteLn(' ð ', v0:1:0, ' (mod ', m:1:0, '),');
              WriteLn;
              Write('   ', Ch, '(', BinExpan(2*k+1), ')');
              x0 := WhereX;
              Write(' = ', Ch, '(', BinExpan(k), ')');
              Write(Ch, '(', BinExpan(k+1), ')');
              if a <> 0
              then
                begin
                  if a > 0
                  then Write(' - ')
                  else Write(' + ');
                  if Abs(a) > 1
                  then Write(Abs(a):1:0);
                  Write('(', -b:1:0, ')');
                  GoToXY(WhereX, WhereY - 1);
                  Write(BinExpan(k));
                  GoToXY(WhereX, WhereY + 1)
                end;
              WriteLn;
              GoToXY(x0, WhereY);
              WriteLn(' ð ', v1:1:0, ' (mod ', m:1:0, ').');
              k := 2*k;
              Prompt
            end;
        end;
      if i > 0
      then
        begin
          Write('It remains to double the index');
          if i > 1
          then WriteLn(' ', i:1, ' times.  Since')
          else WriteLn('.  Since');
          WriteLn(Ch, '(2k) is expressed in terms of ', Ch, '(k), we can');
          WriteLn('dispense with ', Ch, '(k+1) and ', Ch, '(2k+1).');
          WriteLn;
          Prompt
        end
      else WriteLn;
      for h := 1 to i do
        begin
          v0 := mult(v0, v0, m) - 2*c;
          v0 := condition(v0, m);
          c := mult(c, c, m);
          Write('   ', Ch, '(', BinExpan(2*k), ')');
          x0 := WhereX;
          Write(' = ', Ch, '(', BinExpan(k), ')ý - 2(', -b:1:0, ')');
          GoToXY(WhereX, WhereY - 1);
          Write(BinExpan(k));
          GoToXY(x0, WhereY + 1);
          WriteLn;
          GoToXY(x0, WhereY);
          WriteLn(' ð ', v0:1:0, ' (mod ', m:1:0, ').');
          WriteLn;
          k := 2*k;
          Prompt
        end;
      WriteLn('That is, in decimal notation,');
      Write('   ', Ch, '(', n:1:0, ') ð ', v0:1:0, ' (mod ', m:1:0, ')');
      if Ch = 'L'
      then WriteLn('.')
      else
        begin
          WriteLn;
          WriteLn('with a = ', a:1:0, ' and b = ', b:1:0, '.')
        end
    end
end.
