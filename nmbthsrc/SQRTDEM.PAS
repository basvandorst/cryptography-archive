program sqrtdem;     {Find squareroot (mod p), using RESSOL.  That is,
                      SOLve for the square-root of a (mod p) when
                      a is a quadratic RESidue.  The algorithm is
                      due to Dan Shanks (1972), after Tonelli (1891)}
{$N+,E+}
uses CRT, nothy;

var
a, x, p    : comp;
code       : integer;

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
end;

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
  Write('         Press any key to continue ... (or Esc to terminate)');
  Ch := ReadKey;
  if Ch = #0
  then Ch := ReadKey;
  if Ch = #27
  then
    begin
      WriteLn;
      Halt
    end;
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(x0, y0)
end;                               {of procedure Prompt}

function sqrtmodp1(a, p : comp) : comp;

var
i0,                                        {index of cursor location}
i, k0, k1                 :  integer;
b, c, e, m, n,
r, t, t0, t1, z           :  comp;

begin
t := a/p;
a := a - t*p;
if a < 0 then a := a + p;
if p = 2
then
  begin
    sqrtmodp1 := a;
    exit
  end;
if a = 0
then
  begin
    sqrtmodp1 := 0;
    exit
  end;
t := p/2;
if 2*t = p
then
  begin
    WriteLn(p:1:0,' is composite because ', p:1:0, ' is even.');
    Halt
  end;
t := p - 1;
e := t/2;
k0 := -1;
repeat
  m := t;
  t := m/2;
  k0 := k0 + 1
until (2*t <> m);   {k0 is now the power of 2 in p - 1, and
                      m is the odd part of p - 1}
WriteLn;
WriteLn('By repeatedly dividing by 2 we discover that');
if k0 > 1 then WriteLn;
Write('               ',p-1:1:0, ' = 2');
if k0 > 1
then
  begin
    GoToXY(WhereX, WhereY-1);
    Write(k0:1);
    GoToXY(WhereX, WhereY+1)
  end;
WriteLn('ù', m:1:0);
t := power(a, (m-1)/2, p);
r := mult(a, t, p);
t := mult(t, t, p);
n := mult(a, t, p);
WriteLn;
if m > 1 then WriteLn;
Write('We note that (', a:1:0);
if m > 1
then
  begin
    GoToXY(WhereX, WhereY - 1);
    Write((m+1)/2:1:0);
    GoToXY(WhereX, WhereY + 1)
  end;
Write(')ý ð ', a:1:0);
if m > 1
then
  begin
    GoToXY(WhereX, WhereY - 1);
    Write(m:1:0);
    GoToXY(WhereX, WhereY + 1)
  end;
WriteLn('ù', a:1:0, ' (mod ', p:1:0, ').');
WriteLn('That is, ');
Write('     ', r:1:0, 'ý ð ', n:1:0, 'ù', a:1:0);
WriteLn(' (mod ', p:1:0, ').');
WriteLn;
if n > 1
then
  begin
    Prompt;
    t1 := n;
    k1 := 0;
    repeat
      t0 := t1;
      t1 := mult(t0, t0, p);
      k1 := k1 + 1
    until (t1 = 1) or (k1 > k0);
                                        {n has order 2^k1 or p is composite}
    if k1 > k0
    then
      begin
        WriteLn('p = ', p:1:0,' is composite');
        WriteLn('because ', a:1:0, '^(p-1) ð ', t0:1:0, ' (mod p),');
        WriteLn('not 1 as it should be.');
        Halt
      end;
    if (t1 = 1) and (t0 < p - 1)
    then
      begin
        WriteLn('p = ', p:1:0, ' is composite');
        WriteLn('because ', t0:1:0, 'ý ð 1 (mod p)');
        WriteLn('but ', t0:1:0, ' is not ð ñ1 (mod p) as it should be.');
        Halt
      end;
    if k1 = k0
    then
      begin
        WriteLn(a:1:0,' is a quadratic nonresidue of ',p:1:0);
        WriteLn('(assuming that ', p:1:0, ' really is prime).');
        Halt
      end;
    if k1 > 1 then WriteLn;
    Write('By repeated squaring, we find that ');
    Write(n:1:0, ' has order 2');
    if k1 > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(k1:0);
        GoToXY(WhereX, WhereY + 1)
      end;
    WriteLn(' (mod ', p:1:0, ').');
    if k1 > 1 then WriteLn;
    Write('Since 2');
    if k1 > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(k1:1);
        GoToXY(WhereX, WhereY + 1)
      end;
    WriteLn('³', e:1:0, ', this implies that');
    if e > 1 then WriteLn;
    Write('          ', a:1:0);
    if e > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(e:1:0);
        GoToXY(WhereX, WhereY + 1)
      end;
    WriteLn(' ð 1 (mod ', p:1:0, '),');
    Write('so that by Euler''s criterion ');
    WriteLn(a:1:0, ' is a quadratic residue of ', p:1:0, ',');
    WriteLn('assuming that ', p:1:0, ' really is prime.');
    Prompt;
    if k0 > 1 then WriteLn;
    Write('In order to find an element of order 2');
    if k0 > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(k0:0);
        GoToXY(WhereX, WhereY + 1)
      end;
    WriteLn(' we first locate a ');
    WriteLn('quadratic nonresidue modulo ', p:1:0, '.');
    z := 10000;
    repeat
      z := z + 1;
      t := power(z, e, p);
      if (t > 1) and (t < p - 1)
      then
        begin
          z := condition(z, p);
          WriteLn('p = ', p:1:0,' is composite');
          WriteLn('because ',z:1:0, '^((p-1)/2) ð ', t:1:0, ' (mod p),');
          WriteLn('not ñ1 (mod p) as it should be.');
          Halt
        end;
    until t = p - 1;       {z is a quadratic nonresidue}
    z := condition(z, p);
    if e > 1 then WriteLn;
    Write('Since ', z:1:0);
    if e > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(e:1:0);
        GoToXY(WhereX, WhereY + 1)
      end;
    WriteLn(' ð -1 (mod ', p:1:0, '),');
    WriteLn('by Euler''s criterion we deduce that');
    WriteLn(z:1:0, ' is a quadratic nonresidue modulo ', p:1:0, ',');
    WriteLn('assuming that ', p:1:0, ' really is prime.');
    Prompt;
    c := power(z, m, p);
    if (m > 1) or (k0 > 1) then WriteLn;
    Write('Hence ', z:1:0);
    if m > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(m:1:0);
        GoToXY(WhereX, WhereY + 1)
      end;
    Write(' ð ', c:1:0, ' (mod ', p:1:0, ')');
    Write(' has order 2');
    if k0 > 1
    then
      begin
        GoToXY(WhereX, WhereY - 1);
        Write(k0:1);
        GoToXY(WhereX, WhereY + 1)
      end;
    WriteLn('.');
    WriteLn;
    Prompt;
    repeat
      if k0 - k1 - 1 > 0
      then
        begin
          if k0 > 1 then WriteLn;
          Write('Since ', c:1:0, ' has order 2');
          if k0 > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(k0:1);
              GoToXY(WhereX, WhereY + 1)
            end;
          WriteLn(' (mod ', p:1:0, '),');
          WriteLn;
          if k0 - k1 > 2 then WriteLn;
          Write(c:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write('2');
          if k0 - k1 > 2
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write((k0 - k1 - 1):1);
              GoToXY(WhereX, WhereY + 1)
            end;
          GoToXY(WhereX, WhereY + 1);
          for i := 1 to k0 - k1 - 1 do c := mult(c, c, p);
          Write(' ð ', c:1:0, ' (mod ', p:1:0, ')');
          Write(' has order 2');
          GoToXY(WhereX, WhereY - 1);
          Write((k1+1):1);
          GoToXY(WhereX, WhereY + 1);
          WriteLn('.');
          Prompt
        end;
      b := c;
      c := mult(b, b, p);
      WriteLn('We multiply both sides of the congruence');
      WriteLn(r:1:0, 'ý ð ', n:1:0, 'ù', a:1:0, ' (mod ', p:1:0, ')');
      WriteLn('by ', b:1:0, 'ý ð ', c:1:0, ' (mod ', p:1:0,'),');
      WriteLn('to obtain the congruence');
      Write('(', b:1:0, 'ù', r:1:0, ')ý ð (', c:1:0, 'ù', n:1:0, ')');
      WriteLn(a:1:0, ' (mod ', p:1:0, ').');
      r := mult(b, r, p);
      t := n;
      n := mult(c, n, p);
      Write('That is, ', r:1:0, 'ý ð ', n:1:0, 'ù', a:1:0);
      WriteLn(' (mod ', p:1:0, ').');
      Prompt;
      if n > 1
      then
        begin
          if k1 > 1 then WriteLn;
          Write('Since ', t:1:0, ' and ', c:1:0, ' both have order 2');
          if k1 > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(k1:1);
              GoToXY(WhereX, WhereY + 1)
            end;
          WriteLn(',');
          t := n;
          k0 := k1;
          k1 := 0;
          while (t > 1) do
            begin
              t0 := t;
              t := mult(t0, t0, p);
              k1 := k1 + 1
            end;
          If t0 < p - 1
          then
            begin
              WriteLn('ERROR!  ', p:1:0, ' is composite, because');
              WriteLn(t0:1:0, 'ý ð 1 (mod ', p:1:0, '), but ');
              WriteLn('1 < ', t0:1:0, ' < ', (p-1):1:0,'.');
              Halt
            end;
          WriteLn('the order of their product, ', n:1:0, ',');
          WriteLn('is a smaller power of 2.  By repeated squaring');
          if k1 > 1 then WriteLn;
          Write('we discover that the order of ', n:1:0, ' is 2');
          if k1 > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(k1:1);
              GoToXY(WhereX, WhereY + 1)
            end;
          WriteLn('.');
          Prompt
        end;
    until n = 1;
    WriteLn;
  end;
if 2*r > p
then
  begin
    Write('The other solution of this congruence is ', p:1:0, ' - ', r:1:0);
    r := p - r;
    WriteLn(' = ', r:1:0, '.')
  end;
b := mult(r, r, p);
if a <> b
then
  begin
    WriteLn('The algorithm produced an incorrect answer.');
    WriteLn('Are you SURE that ', p:1:0, ' is prime?');
    Halt
  end;
sqrtmodp1 := r
end;             {of function sqrtmodp1}

begin
if ParamCount = 2
then
  begin
    Val(ParamStr(1), x,  code);
    if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
    then a := x
    else Halt;
    Val(ParamStr(2), x, code);
    if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
    then p := x
    else Halt
  end
else
  begin
    ClrScr;
    a := GetInput(WhereX, WhereY, 'Enter   a = ',
         'where ³a³ ó 1000000000000000000.',-MaxAllow, MaxAllow);
    p := GetInput(WhereX, WhereY, 'Enter prime modulus p = ',
         '          where 0 < p ó 1000000000000000000.', 2, MaxAllow)
  end;
x := sqrtmodp1(a, p);
WriteLn;
WriteLn(x:1:0,'ý ð ',a:1:0,' (mod ',p:1:0,')')
end.
