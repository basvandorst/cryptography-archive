program MultDem1;         {Demonstration of algorithm used
                           to calculate a*b (mod m) without overflow}
{$N+,E+}
uses CRT;
{$I GetInput.i }

var
MaxAllow,                 {Larget integer which can accurately be calculated}
Cutoff,                   {Square root of MaxAllow}
MaxMod,                   {Maximum modulus -- MaxAllow/3}
a, b,                     {the factors}
a0, b0,                   {stored values of the original factors}
m,                        {the modulus}
a1,                       {quotient obtained by long division}
r,                        {remainder after division}
g,                        {base used for expansion}
s,                        {sum formed to give answer}
s1,                       {new valuse of s}
b1                        {new value of b}
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
  WriteLn('     This is a demonstration of the simple algorithm described');
  WriteLn('     in Problem *21, Section 2.4, page 83, of Niven-Zuckerman');
  WriteLn('     -Montgomery, Fifth Edition.  This method is SLOW when the');
  WriteLn('     modulus is near the word length.  In such a situation one');
  WriteLn('     should use the algorithm demonstrated in MultDem2.  The');
  WriteLn('     Mult function implemented in the NoThy unit uses both');
  WriteLn('     methods, as demonstrated in MultDem3.');
  Prompt;
  WriteLn;
  WriteLn('Suppose that arithmetic can be performed up to 2d digits,');
  d := round(GetInput(WhereX, WhereY, 'where d = ', '1 ó d ó 9', 1, 9));
  MaxAllow := 1;
  Cutoff := 1;
  for i := 1 to d do
    begin
      Cutoff := 10*Cutoff;
      MaxAllow := 100*MaxAllow
    end;
  MaxMod := MaxAllow/3;
  ClrScr;
  WriteLn('Will find a*b (mod m) where');
  str(MaxAllow:1:0, St);
  str(MaxMod:1:0, St1);
  a := GetInput(WhereX, WhereY, '    a = ',
       '  ³a³ ó ' + St, -MaxAllow, MaxAllow);
  b := GetInput(WhereX, WhereY, '    b = ',
       '  ³b³ ó ' + St, -MaxAllow, MaxAllow);
  m := GetInput(WhereX, WhereY, '    m = ',
       '1 ó m ó ' + St1, 1, MaxMod);
  a0 := a;
  b0 := b;
  WriteLn('We first reduce a and b (mod m),');
  Write('a ð a'' (mod m)   ', a:1:0, ' ð ');
  a := condition(a, m);
  WriteLn(a:1:0, ' (mod ', m:1:0, '),');
  Write('b ð b'' (mod m)   ', b:1:0, ' ð ');
  b := condition(b, m);
  WriteLn(b:1:0, ' (mod ', m:1:0, ').');
  WriteLn;
  Prompt;
  WriteLn('We now continue with a'' and b'' in place of a and b.');
  Write('We can perform integer arithmetic on numbers up to ');
  WriteLn(MaxAllow:1:0, '.');
  if m < cutoff
  then
    begin
      WriteLn('Since mý < ', MaxAllow:1:0, ', we may calculate a*b,');
      WriteLn('and reduce this (mod m).');
      s := a*b;
      WriteLn('a*b = s          ', a:1:0, '*', b:1:0, ' = ', s:1:0, ',');
      Write('  s ð s'' (mod m)   ', s:1:0, ' ð ');
      s := condition(s, m);
      WriteLn(s:1:0, ' (mod ', m:1:0, ').');
      WriteLn
    end
  else
    begin
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
      s := 0;
      WriteLn('We seek to evaluate a*b + s (mod m), where initially s = 0.');
      WriteLn;
      Prompt;
      while a > 0 do
        begin
          a1 := a/g;
          r := a - a1*g;
          Write('We divide g into a, obtaining a quotient a'' and');
          WriteLn(' a remainder r.');
          WriteLn('For speed we round to the nearest integer.');
          Write('a = a''*g + r    ', a:1:0, ' = ', a1:1:0, '*', g:1:0);
          if r >= 0
          then WriteLn(' + ', r:1:0)
          else WriteLn(' - ', (-r):1:0);
          WriteLn;
          Prompt;
          Write('Hence a*b + s = (a''*g + r)*b + s = a''*(g*b)');
          WriteLn(' + (r*b + s).');
          WriteLn('Since g*b < g*m ó ', MaxAllow:1:0, ', we may calculate');
          Write('g*b, and reduce (mod m), say g*b ð b'' (mod m)');
          WriteLn(', 0 ó b'' < m.');
          Write('g*b = ...          ', g:1:0, '*', b:1:0);
          b1 := b*g;
          x0 := WhereX;
          WriteLn(' = ', b1:1:0);
          b1 := condition(b1, m);
          Write('    ð b'' (mod m)');
          GoToXY(x0, WhereY);
          WriteLn(' ð ', b1:1:0, ' (mod ', m:1:0, ').');
          WriteLn;
          Prompt;
          Write('Since ³r*b + s³ < (g/2)*m + m < g*m ó ', MaxAllow:1:0);
          WriteLn(',');
          WriteLn('we may calculate r*b + s, and reduce (mod m),');
          WriteLn('say r*b + s ð s'' (mod m), 0 ó s < m.');
          WriteLn('  r*b + s                   ', r:1:0, '*', b:1:0, ' + ', s:1:0);
          s1:= r*b + s;
          WriteLn('= ...                     = ', s1:1:0);
          s1 := condition(s1, m);
          WriteLn('ð s'' (mod m)              ð ', s1:1:0, ' (mod ', m:1:0, ').');
          WriteLn;
          Prompt;
          WriteLn('Hence');
          Write('  a*b + s                     ', a:1:0, '*', b:1:0);
          WriteLn(' + ', s:1:0);
          Write('ð a''*b'' + s'' (mod m)        ð ', a1:1:0, '*', b1:1:0);
          WriteLn(' + ', s1:1:0);
          WriteLn;
          Prompt;
          a := a1;
          b := b1;
          s := s1;
          if a > 0
          then
            begin
              WriteLn('We have made progress, since a'' < a/g + 1.');
              WriteLn('We set a = a'', b = b'', s = s'', and repeat.');
              WriteLn;
              Prompt
            end
        end;
      WriteLn('Since a'' = 0, s'' is the desired number:');
  end;
  WriteLn(a0:1:0, '*', b0:1:0, ' ð ', s:1:0, ' (mod ', m:1:0, ')');
  Prompt
end.
