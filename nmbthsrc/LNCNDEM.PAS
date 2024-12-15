program LnCnDem;          {Demonstration of the procedure LinCon}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a1,                       {The coefficient of x}
a0,                       {The constant term}
c1,                       {The coefficient of x, reduced (mod m)}
c0,                       {The constant term, reduced (mod m)}
d1,                       {d1 = a1/r0, if r0|a0}
m,                        {The given modulus}
a,                        {The resulting residue class}
m1,                       {The resulting modulus}
r0,                       {old remainder}
r1,                       {new remainder}
u0,                       {old coefficient}
u1,                       {new remainder}
q,                        {quotient, rounded to nearest integer}
t                         {temporary variable}
         : comp;
q0,                       {quotient, a real number}
x        : extended;
i,                        {an index}
code                      {Error code in translating a string
                           to a real number}
         : integer;

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



begin                                 {main}
  ClrScr;
  if ParamCount = 3
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a1 := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a0 := x
      else Halt;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
      then m := x
      else Halt
    end
  else
    begin
      Write('Will find all solutions of the linear congruence ');
      WriteLn('ax ð b (mod m) where');
      a1 := GetInput(WhereX, WhereY, 'a = ', '', -MaxAllow, MaxAllow);
      a0 := GetInput(WhereX, WhereY, 'b = ', '', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'm = ', '', 1, MaxAllow);
    end;
  c1 := condition(a1, m);
  c0 := condition(a0, m);
  WriteLn;
  WriteLn('If necessary, we replace the given coefficients by coefficients');
  WriteLn('that lie between 0 and m - 1.');
  WriteLn('a = ', a1:1:0, ' ð ', c1:1:0, ' (mod ', m:1:0, ')');
  WriteLn('b = ', a0:1:0, ' ð ', c0:1:0, ' (mod ', m:1:0, ')');
  WriteLn;
  WriteLn('Next we apply the Euclidean Algorithm to ', c1:1:0, ' and');
  WriteLn(m:1:0, ' in order to determine the value of g = (a, m).');
  WriteLn('In the relation ax + my = g we calculate the value of x');
  WriteLn('but we do not need the value of y.  For speed we round to the');
  WriteLn('nearest integer.');
  Prompt;
  u0 := 0; u1 := 1;      {initialize sequence of coefficients}
  r0 := m; r1 := c1;     {initialize sequence of remainders}
  WriteLn;
  WriteLn('             q(i+1)                r(i)                x(i)');
  WriteLn('                    ',m:20:0, u0:20:0);
  while r1 <> 0 do
    begin              {Euclidean algorithm}
      q := r0/r1;      {qotient of remainders is rounded to nearest integer}
      WriteLn(q:20:0, r1:20:0, u1:20:0);
      t := r1;         {save remainder}
      r1 := r0 - q*r1; {construct new remainder, which may be negative}
      r0 := t;         {former remainder is now old remainder}
      t := u1;         {save coefficient}
      u1 := u0 - q*u1; {construct new coefficient}
      u0 := t          {former coefficient is now old coefficient}
    end;
  WriteLn('                                       0');
  WriteLn;
  if r0 < 0
  then
    begin
      WriteLn('Since the last non-zero remainder, r(j) = ', r0:1:0, ',');
      WriteLn('is negative, we reverse the sign of both r(j) and x(j).');
      r0 := -r0;
      u0 := -u0
    end;                 {r0 = (a1, m), a*u0 = r0 (mod m)}
  WriteLn('Thus (a, m) = ', r0:1:0, '.');
  Prompt;
  WriteLn('     The congruence ax ð b (mod m) has a solution');
  WriteLn('if and only if (a, m) divides b.');
  q0 := c0/r0;
  if frac(q0) > 0          {must have (a1, m)³a0}
  then
    begin
      a := r0;
      m1 := 0;
      Write('In the present example, (a, m) = ', r0:1:0);
      WriteLn(', which does not divide ', c0:1:0, '.');
      WriteLn('Hence there is no solution.');
      Write('The procedure LinCon returns the values ');
      WriteLn(r0:1:0, ', 0.')
    end
  else                   {(a1, m)³a0}
    begin
      m1 := m/r0;
      q := q0;             {The variable q, of type comp, is assigned to
                            be the integer nearest to the real variable q0}
      d1 := c1/r0;
      a := mult(u0, q, m1);
      Write('In the present example, (a, m) = ', r0:1:0, ' divides ');
      WriteLn(c0:1:0, ',');
      WriteLn(c0:1:0, '/', r0:1:0, ' = ', q:1:0, '.');
      Prompt;
      if r0 > 1
      then
        begin
          WriteLn('Also, ', c1:1:0, '/', r0:1:0, ' = ', d1:1:0, ',');
          WriteLn(m:1:0, '/', r0:1:0, ' = ', m1:1:0, '.');
          Write('Hence the congruence ', c1:1:0, 'x ð ', c0:1:0, ' (mod ');
          WriteLn(m:1:0, ')');
          Write('is equivalent to ', d1:1:0, 'x ð ', q:1:0, ' (mod ');
          WriteLn(m1:1:0, ').');
          Prompt;
          WriteLn('The identity ax(j) + my(j) = g may be written in ');
          WriteLn('conguential form, ax(j) ð g (mod m).  On dividing all ');
          WriteLn('three members by g, this gives');
          Write('(*)              ', d1:1:0, '*', u0:1:0, ' ð 1 (mod ');
          WriteLn(m1:1:0, ').');
          Prompt
        end
      else
        begin
          WriteLn('The identity ax(j) + my(j) = 1 may be written in');
          WriteLn('congruential form, ax(j) ð 1 (mod m).  That is, ');
          Write('(*)              ', c1:1:0, '*', u0:1:0, ' ð 1 (mod ');
          WriteLn(m1:1:0, ').');
          Prompt
        end;
      WriteLn('Since (', u0:1:0, ', ', m1:1:0, ') = 1,');
      WriteLn('the congruence ');
      Write('            ', d1:1:0, 'x ð ', q:1:0, ' (mod ');
      WriteLn(m1:1:0, ')');
      WriteLn('is equivalent to ');
      Write('      ', u0:1:0, '*', d1:1:0, 'x ');
      i := WhereX;
      WriteLn('ð ', u0:1:0, '*', q:1:0, ' (mod ', m1:1:0, ')');
      GoToXY(i, WhereY);
      WriteLn('ð ', a:1:0, ' (mod ', m1:1:0, ').');
      WriteLn('By (*) this in turn is equivalent to ');
      GoToXY(i-2, WhereY);
      WriteLn('x ð ', a:1:0, ' (mod ', m1:1:0, ').');
      Write('The procedure LinCon returns the two values ');
      WriteLn(a:1:0, ', ', m1:1:0,'.')
    end
end.
