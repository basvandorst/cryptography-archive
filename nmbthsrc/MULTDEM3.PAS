program MultDem3;         {Demonstration of the function mult in the
                           unit NoThy}
{$N+,E+}
uses CRT;
{$I GetInput.i }

const
MaxAllow = 1E18;
CutOff = 1E9;

var
a, b,                     {the factors}
a0, b0,
m,                        {the modulus}
a1, b1,
q,                        {quotient, rounded to nearest integer}
g,                        {base used for expansion}
s,                        {temporary variable}
q1, q0,                   {q = q1*10^9 + q0}
m1, m0,                   {m = m1*10^9 + m0}
c2, c1, c0,               {(a1*10^9 + a0)*(b1*10^9 + b0)
                           = c2*10^18 + c1*10^9 + c0}
d2, d1, d0,               {q*m = d2*10^18 + d1*10^9 + d0}
e2, e1, e0,               {ei = ci - di, i = 1, 2, 3}
c                         {final result}
         : comp;

x        : extended;
i,                        {an index}
x0,                       {cursor coordinate}
code                      {Error code in translating a string
                           to a real number}
         : integer;
St       : string[80];

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

begin                          {main body}
  ClrScr;
  WriteLn('     We calculate a*b (mod m) by the method implemented in the');
  WriteLn('     function Mult in the unit NoThy.  We assume that addition');
  WriteLn('     and multiplication of integers is accurate provided that the');
  WriteLn('     inputs and output have modulus not exceeding 2*10^18.  We');
  WriteLn('     also assume that floating-point multiplication and division');
  WriteLn('     of real numbers is accurate in the first 18 places.');
  Prompt;
  WriteLn;
  ClrScr;
  WriteLn('Will find a*b (mod m) where');
  str(MaxAllow:1:0, St);
  a := GetInput(WhereX, WhereY, '    a = ',
       '  ³a³ ó ' + St, -MaxAllow, MaxAllow);
  b := GetInput(WhereX, WhereY, '    b = ',
       '  ³b³ ó ' + St, -MaxAllow, MaxAllow);
  m := GetInput(WhereX, WhereY, '    m = ',
       '1 ó m ó ' + St, 1, MaxAllow);
  WriteLn('We first reduce a and b (mod m):');
  WriteLn;
  Write('a ð a'' (mod m) ', a:1:0, ' ð ');
  q := a/m;
  a0 := a - q*m;
  WriteLn(a0:1:0, ' (mod ', m:1:0, ')');
  Write('b ð b'' (mod m) ', b:1:0, ' ð ');
  q := b/m;
  b0 := b - q*m;
  WriteLn(b0:1:0, ' (mod ', m:1:0, ')');
  WriteLn;
  Prompt;
  if m <= cutoff
  then
    begin
      WriteLn('Since mý < ', MaxAllow:1:0, ', we may calculate a*b,');
      WriteLn('directly, and reduce this (mod m).');
      c := a0*b0;
      WriteLn('a*b = c          ', a0:1:0, '*', b0:1:0, ' = ', c:1:0, ',');
      Write('  c ð c'' (mod m)   ', c:1:0, ' ð ');
      q := c/m;
      c := c - q*m;
      if c < 0 then c := c + m;
      WriteLn(c:1:0, ' (mod ', m:1:0, ').');
      WriteLn
    end;
  if (m > CutOff) and (m <= 1E12)
  then
    begin
      WriteLn;
      Write('Since 10');
      GoToXY(WhereX, WhereY-1);
      Write('9');
      GoToXY(WhereX, WhereY+1);
      Write(' < m ó 10');
      GoToXY(WhereX, WhereY-1);
      Write('12');
      GoToXY(WhereX, WhereY+1);
      WriteLn(', we proceed as in MultDem1.');
      g := MaxAllow/m;
      if g*m > MaxAllow then g := g - 1;    {force round down}
      Write('We set g = [', MaxAllow:1:0, '/m], so that g*m ó ');
      WriteLn(MaxAllow:1:0, '.');
      WriteLn('   g = ', g:1:0);
      Write('   g*m = ', g:1:0, '*', m:1:0);
      x0 := WhereX;
      WriteLn(' = ', g*m:1:0);
      GoToXY(x0, WhereY);
      WriteLn(' ó ', MaxAllow:1:0, '.');
      WriteLn('Since gý > m, the main loop of the algorithm is ');
      WriteLn('executed only once.  We begin by expressing ');
      WriteLn('the factor a in base g:');
      s := a0;
      WriteLn;
      Prompt;
      Write('a = a1*g + a0     ', a0:1:0, ' = ');
      a1 := a0/g;
      a0 := a0 - a1*g;
      Write(a1:1:0, '*', g:1:0);
      if a0 >= 0
      then WriteLn(' + ', a0:1:0)
      else WriteLn(' - ', (-a0):1:0);
      WriteLn;
      Prompt;
      WriteLn('Hence');
      WriteLn;
      WriteLn('a*b = (a1*g + a0)*b     ');
      Write('          ', s:1:0, '*', b0:1:0);
      x0 := WhereX;
      Write(' = (', a1:1:0, '*', g:1:0);
      if a0 >= 0
      then WriteLn(' + ', a0:1:0, ')*', b0:1:0)
      else WriteLn(' - ', (-a0):1:0, ')*', b0:1:0);
      WriteLn('    = a1*g*b + a0*b');
      GoToXY(x0, WhereY);
      Write(' = ', a1:1:0, '*', g:1:0, '*', b0:1:0);
      if a0 >= 0
      then WriteLn(' + ', a0:1:0, '*', b:1:0)
      else WriteLn(' - ', (-a0):1:0, '*', b:1:0, '.');
      WriteLn;
      Prompt;
      Write('Since ³a0*b³ < g*m < 10');
      GoToXY(WhereX, WhereY - 1);
      Write('18');
      GoToXY(WhereX, WhereY + 1);
      WriteLn(', we may perform the multiplication a0*b,');
      WriteLn('and reduce (mod m).  Similarly we may perform the');
      WriteLn('multiplication g*b and reduce (mod m).  Thus the above is');
      WriteLn;
      WriteLn('     ð a1*c1 + c0');
      GoToXY(x0, WhereY);
      c1 := g*b;
      q := c1/m;
      c1 := c1 - q*m;
      c0 := a0*b;
      q := c0/m;
      c0 := c0 - q*m;
      Write(' ð ', a1:1:0, '*', c1:1:0);
      if c0 >= 0
      then WriteLn(' + ', c0:1:0)
      else WriteLn(' - ', (-c0):1:0, '.');
      WriteLn;
      Prompt;
      Write('Since ³a1*c1³ < g*m < 10');
      GoToXY(WhereX, WhereY - 1);
      Write('18');
      GoToXY(WhereX, WhereY + 1);
      WriteLn(', we may perform the multiplication a1*c1,');
      WriteLn('and reduce (mod m).  Hence the above is ');
      WriteLn;
      c1 := c1*a1;
      q := c1/m;
      c1 := c1 - q*m;
      WriteLn('     ð c2 + c0');
      GoToXY(x0, WhereY);
      Write(' ð ', c1:1:0);
      if c0 >= 0
      then WriteLn(' + ', c0:1:0)
      else WriteLn(' - ', c0:1:0);
      GoToXY(x0, WhereY);
      c := c1 + c0;
      Write(' ð ', c:1:0);
      if c < 0
      then
        begin
          WriteLn;
          c := c + m;
          GoToXY(x0, WhereY);
          Write(' ð ', c:1:0)
        end;
      WriteLn(' (mod ', m:1:0, ').');
      WriteLn;
      Prompt;
    end;
  if (m > 1E12) and (m <= MaxAllow)
  then
    begin
      WriteLn;
      Write('Since 10');
      GoToXY(WhereX, WhereY-1);
      Write('12');
      GoToXY(WhereX, WhereY+1);
      Write(' < m ó 10');
      GoToXY(WhereX, WhereY-1);
      Write('18');
      GoToXY(WhereX, WhereY+1);
      Write(', we proceed as in MultDem2.  Put B = 10');
      GoToXY(WhereX, WhereY-1);
      Write('9');
      GoToXY(WhereX, WhereY+1);
      WriteLn('.');
      WriteLn('Here B is a base used to represent numbers.');
      WriteLn('We write a = a1*B + a0, with ³a0³ ó B/2:');
      WriteLn;
      a1 := a/cutoff;
      a0 := a - a1*cutoff;
      Write(a:1:0, ' = ', a1:1:0, '*B');
      if a0 >= 0
      then WriteLn(' + ', a0:1:0, '.')
      else WriteLn(' - ', Abs(a0):1:0, '.');
      WriteLn;
      Prompt;
      WriteLn('Similarly we write b = b1*B + b0 with ³b0³ ó B/2:');
      WriteLn;
      b1 := b/cutoff;
      b0 := b - b1*cutoff;
      Write(b:1:0, ' = ', b1:1:0, '*B');
      if b0 >= 0
      then WriteLn(' + ', b0:1:0, '.')
      else WriteLn(' - ', Abs(b0):1:0, '.');
      WriteLn;
      Prompt;
      WriteLn('We note that');
      WriteLn('          (a1*B + a0)*(b1*B + b0) = c2*Bý + c1*B + c0');
      WriteLn('where');
      WriteLn('           c2 = a1*b1, c1 = a1*b0 + a0*b1, c0 = a0*b0.');
      WriteLn('Thus');
      WriteLn;
      WriteLn('  ', a:1:0, '*', b:1:0);
      c2 := a1*b1;
      c1 := a1*b0 + a0*b1;
      c0 := a0*b0;
      Write('= ', c2:1:0, '*Bý');
      if c1 >= 0
      then Write(' + ', c1:1:0)
      else Write(' - ', Abs(c1):1:0);
      Write('*B');
      if c0 >= 0
      then WriteLn(' + ', c0:1:0, '.')
      else WriteLn(' - ', Abs(c0):1:0, '.');
      WriteLn;
      Prompt;
      WriteLn('Next we compute a*b/m as a floating-point real number,');
      WriteLn('and set q equal to the integer nearest this value.');
      x := a*b;
      q := x/m;
      WriteLn('We find that q = ', q:1:0, '.');
      WriteLn('Hence q*m is the multiple of m closest to a*b, so that');
      WriteLn('³a*b - q*m³ ó m/2.  We now compute q*m in the same way');
      WriteLn('that we computed a*b.  We write q = q1*B + q0:');
      q1 := q/cutoff;
      q0 := q - q1*cutoff;
      WriteLn;
      Write('     ', q:1:0, ' = ', q1:1:0, '*B');
      if q0 >= 0
      then WriteLn(' + ', q0:1:0, '.')
      else WriteLn(' - ', Abs(q0):1:0, '.');
      WriteLn;
      WriteLn('Next we write m = m1*B + m0:');
      m1 := m/cutoff;
      m0 := m - cutoff*m1;
      WriteLn;
      Write('     ', m:1:0, ' = ', m1:1:0, '*B');
      if m0 >= 0
      then WriteLn(' + ', m0:1:0, '.')
      else WriteLn(' - ', Abs(m0):1:0, '.');
      WriteLn;
      Prompt;
      WriteLn('Hence');
      WriteLn('                   q*m = d2*Bý + d1*B + d0');
      WriteLn('where');
      WriteLn('          d2 = q1*m1, d1 = q1*m0 + q0*m1, d0 = q0*m0.');
      WriteLn('That is,');
      WriteLn;
      d2 := q1*m1;
      d1 := q1*m0 + q0*m1;
      d0 := q0*m0;
      WriteLn('  ', q:1:0, '*', m:1:0);
      Write('= ', d2:1:0, '*Bý');
      if d1 >= 0
      then Write(' + ', d1:1:0)
      else Write(' - ', Abs(d1):1:0);
      Write('*B');
      if d0 >= 0
      then WriteLn(' + ', d0:1:0, '.')
      else WriteLn(' - ', Abs(d0):1:0, '.');
      Prompt;
      WriteLn;
      WriteLn('Hence');
      WriteLn('          a*b - q*m = ((c2 - d2)*B + c1 - d1)*B + c0 - d0.');
      WriteLn('which is to say');
      WriteLn;
      e2 := c2 - d2;
      e1 := c1 - d1;
      e0 := c0 - d0;
      WriteLn('  ', a:1:0, '*', b:1:0, ' - ', q:1:0, '*', m:1:0);
      Write('= ((', e2:1:0, ')*B');
      if e1 >= 0
      then Write(' + ', e1:1:0)
      else Write(' - ', Abs(e1):1:0);
      Write(')*B');
      if e0 >= 0
      then WriteLn(' + ', e0:1:0)
      else WriteLn(' - ', Abs(e0):1:0);
      c := e2*cutoff + e1;
      Write('= ', c:1:0, '*B');
      if e0 >= 0
      then WriteLn(' + ', e0:1:0)
      else WriteLn(' - ', Abs(e0):1:0);
      c := c*cutoff + e0;
      WriteLn('= ', c:1:0);
      if c < 0
      then
        begin
          c := c + m;
          WriteLn('ð ', c:1:0, ' (mod ', m:1:0, ').')
        end;
      WriteLn;
      Prompt
    end;
  WriteLn(a:1:0, '*', b:1:0, ' ð ', c:1:0, ' (mod ', m:1:0, ')');
  Prompt
end.