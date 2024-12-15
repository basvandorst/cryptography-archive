program pwrdem2;          {Demonstration of alternative powering
                           algorithm}
{$N+,E+}
uses CRT;
{$I GetInput.i }

const
MaxAllow = 1E18;

var
a,                        {the base}
k,                        {the exponent}
m,                        {the modulus}
p,                        {product being formed}
e,                        {= 0 or 1 -- the value of k (mod 2)}
q,                        {quotient}
t,                        {least power of 2 > k}
a0,                       {original base}
k0                        {original exponent}
         : comp;

x        : extended;
i,                        {an index}
j,                        {number of leading bits in binary expansion}
l,                        {l = length(Binaryk}
code,                     {Error code in translating a string
                           to a real number}
x0                        {cursor coordinate}
         : integer;
Ch       : char;
St,
Binaryk,
ExpStr1,
ExpStr2  : string[80];

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
end;                {of function condition}

function mult(a, b, m : comp)  :  comp;

const
cutoff = 1E9;

var
 q,                     {quotient obtained by long division}
 r,                     {remainder after division}
 g,                     {base used for expansion}
 s                      {sum formed to give answer}
        : comp;
begin
  a := condition(a, m);
  b := condition(b, m);
  if m < cutoff
  then
    begin
      s := a*b;
      mult := condition(s, m)
    end
  else
    begin
      g := MaxAllow/m;
      if g < 3 then
        begin
          WriteLn('The modulus m = ',m:1:0,' is too large ');
          WriteLn('for convenient multiplication of residue classes.');
          Halt
        end;
      s := 0;
      while a > 0 do
        begin
          q := a/g;
          r := a - q*g;
          s := condition(s + r*b, m);
          b := condition(b*g, m);
          a := q
        end;
    mult := s
  end
end;                       {of function mult}

begin                      {main body}
  if ParamCount = 3
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 0) and (x <= MaxAllow)
      then k := x
      else Halt;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
      then m := x
      else Halt
    end
  else
    begin
      str(MaxAllow:1:0, St);
      WriteLn('Will find a^k (mod m) where');
      a := GetInput(WhereX, WhereY, '    a = ',
            '  ³a³ ó ' + St, -MaxAllow, MaxAllow);
      k := GetInput(WhereX, WhereY, '    k = ',
            '0 ó k ó ' + St, 0, MaxAllow);
      m := GetInput(WhereX, WhereY, '    m = ',
            '0 < m ó ' + St, 1, MaxAllow);
    end;
  ClrScr;
  WriteLn('     This is a demonstration of an alternative method of powering.');
  WriteLn('Like the other method, it depends on repeated squaring, and the');
  WriteLn('number of multiplications performed is exactly the same:');
  WriteLn('[log k/log 2] + w(k), where w(k), called the `binary weight'' ');
  WriteLn('of k, is the number of 1''s in the binary expansion of k.');
  WriteLn('In this method the binary expansion of k is accessed from the top');
  WriteLn('down.  When a 1 is found in the binary expansion, the existing');
  WriteLn('power is multiplied by a.  This offers an advantage of speed in');
  WriteLn('situations in which multiplication by a is faster than most other');
  WriteLn('multiplications.');
  WriteLn('     For clarity, as the calculation is performed, all exponents');
  WriteLn('are displayed in binary.  The desired exponent is');
  WriteLn('      k = ', k:1:0, ' (decimal)');
  t := 1;                    {The first step is to determine the binary}
  l := 0;                    {expansion of k}
  while t <= k do
    begin
      t := 2*t;
      l := l + 1
    end;                     {t is now the least power of 2 > k}
  Binaryk := '';             {l will be length(Binaryk) }
  k0 := k;                   {when Binaryk has been determined.}
  j := 0;
  while k0 > 0 do
    begin
      j := j + 1;
      k0 := 2*k0;
      if k0 >= t
      then
        begin
          k0 := k0 - t;
          Binaryk := Binaryk + '1'
        end
      else Binaryk := Binaryk + '0';
    end;
                        {All the non-zero digits in the binary expansion}
                        {of k are now in Binaryk, but the trailing 0's}
                        {must be appended.  There are l - j trailing 0's}
  for i := 1 to l - j do Binaryk := Binaryk + '0';
  if Binaryk = ''
  then Binaryk := '0';
  WriteLn('        = ', Binaryk + ' (binary)');
  WriteLn;
  Prompt;
  p := 1;
  WriteLn;
  WriteLn;
  ExpStr1 := Binaryk;
  ExpStr2 := '';
  while ExpStr1 <> '' do
    begin
      Ch := ExpStr1[1];
      Delete(ExpStr1, 1, 1);
      if ExpStr2 <> ''
      then
        begin
          Write(a:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr2 + '0');
          GoToXY(WhereX, WhereY + 1);
          x0 := WhereX;
          Write(' = ', a:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr2);
          GoToXY(WhereX, WhereY + 1);
          Write('ù', a:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr2);
          GoToXY(WhereX, WhereY + 1);
          WriteLn;
          WriteLn;
          GoToXY(x0, WhereY);
          WriteLn(' ð ', p:1:0, 'ù', p:1:0);
          WriteLn;
          GoToXY(x0, WhereY);
          p := mult(p, p, m);
          Write(' ð ', p:1:0, ' (mod ', m:1:0, ')');
          WriteLn;
          WriteLn;
          WriteLn;
          Prompt
        end;
      if Ch = '1'
      then
        begin
          Write(a:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr2 + '1');
          GoToXY(WhereX, WhereY + 1);
          x0 := WhereX;
          Write(' = ', a:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr2 + '0');
          GoToXY(WhereX, WhereY + 1);
          WriteLn('ù', a:1:0);
          WriteLn;
          GoToXY(x0, WhereY);
          WriteLn(' ð ', p:1:0, 'ù', a:1:0);
          WriteLn;
          GoToXY(x0, WhereY);
          p := mult(p, a, m);
          WriteLn(' ð ', p:1:0, ' (mod ', m:1:0, ')');
          WriteLn;
          WriteLn;
          Prompt
        end;
      ExpStr2 := ExpStr2 + Ch;
    end;
  WriteLn;
  Write(a:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write(k:1:0);
  GoToXY(WhereX, WhereY + 1);
  WriteLn(' ð ', p:1:0, ' (mod ', m:1:0, ')');
end.
