program MultDem2;         {Demonstration of an algorithm  which can be used
                           to calculate a*b (mod m) without overflow}
{$N+,E+}
uses CRT;
{$I GetInput.i }

var
MaxAllow,                 {Larget integer which can accurately be calculated}
Cutoff,                   {Square root of MaxAllow}
a, b,                     {the factors}
a1, a0,                   {a = a1*10*d + a0}
b1, b0,                   {b = b1*10^d + b0}
c2, c1, c0,               {a*b = c2*10^(2d) + c1*10^d + c0}
q,                        {= integer nearest ab/m}
q1, q0,                   {q = q1*10^d + q0}
m1, m0,                   {m = m1*10^d + m0}
d2, d1, d0,               {q*m = d2*10^(2d) + d1*10^d + d0}
e2, e1, e0,               {ei = ci - di, i = 1, 2, 3}
c,                        {final result}
m,                        {the modulus}
s                         {partial result}
         : comp;

x        : extended;
i,                        {an index}
d,                        {number of digits in 1/2 word length}
x0,                       {cursor coordinate}
code                      {Error code in translating a string
                           to a real number}
         : integer;
St, St1  : string[80];

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
  if m > MaxAllow
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
end;                           {of function condition}

begin                          {main body}
  ClrScr;
  WriteLn('     This program demonstrates a method for multiplying two');
  WriteLn('     residue classes (mod m).  This method is efficient when');
  WriteLn('     m is large -- say larger than 2/3 the word size.');
  Prompt;
  WriteLn;
  WriteLn('We suppose that fixed-point integer addition and multiplication');
  WriteLn('is accurately performed provided that the inputs and the output');
  WriteLn;
  Write('have absolute values not exceeding 2*10');
  GoToXY(WhereX, WhereY-1);
  Write('2d');
  GoToXY(WhereX, WhereY+1);
  WriteLn('.  We also assume that');
  WriteLn('floating-point real multiplication and division are accurate in');
  WriteLn('the first 2d decimal places,');
  d := round(GetInput(WhereX, WhereY, 'where d = ', '1 ó d ó 9', 1, 9));
  MaxAllow := 1;
  Cutoff := 1;
  for i := 1 to d do
    begin
      Cutoff := 10*Cutoff;
      MaxAllow := 100*MaxAllow
    end;
  ClrScr;
  WriteLn('Will find a*b (mod m) where');
  str(MaxAllow:1:0, St);
  str(CutOff:1:0, St1);
  a := GetInput(WhereX, WhereY, '    a = ',
       '  ³a³ ó ' + St, -MaxAllow, MaxAllow);
  b := GetInput(WhereX, WhereY, '    b = ',
       '  ³b³ ó ' + St, -MaxAllow, MaxAllow);
  m := GetInput(WhereX, WhereY, '    m = ',
       '1 ó m ó ' + St, 1, MaxAllow);
  WriteLn('Let a'' ð a (mod m), ³a''³ ó m/2:');
  Write('     a'' = ');
  q := a/m;
  a := a - q*m;
  WriteLn(a:1:0, '.');
  WriteLn('Let b'' ð b (mod m), ³b''³ ó m/2:');
  Write('     b'' = ');
  q := b/m;
  b := b - q*m;
  WriteLn(b:1:0, '.');
  WriteLn;
  Prompt;
  WriteLn('We continue with a'' and b'' in place of a and b.');
  Write('We can perform integer arithmetic on numbers up to ');
  WriteLn(MaxAllow:1:0, '.');
  if m < cutoff
  then
    begin
      WriteLn('Since mý < ', MaxAllow:1:0, ',');
      WriteLn('we may calculate a*b directly,');
      WriteLn('and reduce this product (mod m).');
      s := a*b;
      WriteLn('a*b = s          ', a:1:0, '*', b:1:0, ' = ', s:1:0, ',');
      Write('  s ð s'' (mod m)   ', s:1:0, ' ð ');
      s := condition(s, m);
      WriteLn(s:1:0, ' (mod ', m:1:0, ').');
      WriteLn
    end
  else
    begin
      WriteLn('Put B = ', St1, '.  Here B is a base used to represent');
      WriteLn('numbers.  We write a = a1*B + a0, with ³a0³ ó B/2:');
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
      Write('     ', q:1:0, ' = ', q1:1:0, '*B');
      if q0 >= 0
      then WriteLn(' + ', q0:1:0, '.')
      else WriteLn(' - ', Abs(q0):1:0, '.');
      WriteLn('Next we write m = m1*B + m0:');
      m1 := m/cutoff;
      m0 := m - cutoff*m1;
      Write('     ', m:1:0, ' = ', m1:1:0, '*B');
      if m0 >= 0
      then WriteLn(' + ', m0:1:0, '.')
      else WriteLn(' - ', Abs(m0):1:0, '.');
      Prompt;
      WriteLn('Hence');
      WriteLn('                   q*m = d2*Bý + d1*B + d0');
      WriteLn('where');
      WriteLn('          d2 = q1*m1, d1 = q1*m0 + q0*m1, d0 = q0*m0.');
      WriteLn('That is,');
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
      WriteLn('That is,');
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
      WriteLn('= ', c:1:0, ',');
      Prompt;
      WriteLn('and we conclude that');
      WriteLn('                  ',a:1:0, '*', b:1:0);
      Write('                ð ', c:1:0);
      if c < 0
      then
        begin
          c := c + m;
          WriteLn;
          Write('                ð ', c:1:0)
        end;
      WriteLn(' (mod ', m:1:0, ').');
    end
end.
