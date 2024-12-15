
program pwrdem1b;         {Demonstration of powering algorithm}
{$N+,E+}
uses nothy, CRT;
{$I GetInput.i }

const
MaxAllow = 1E18;

var
a,                        {the base}
k,                        {the exponent}
m,                        {the modulus}
p,                        {product being formed}
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
ExpStr2,
ExpStr3  : string[80];

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
  Binaryk := '';             {l will be length(Binaryk) once}
  k0 := k;                   {Binaryk has been determined.}
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
  WriteLn('        = ', Binaryk + ' (binary)');
  WriteLn;
  Prompt;
  p := 1;
  a0 := a;
  WriteLn;
  WriteLn;
  ExpStr1 := Binaryk;
  ExpStr2 := '';
  ExpStr3 := '1';
  p := 1;
  while ExpStr1 <> '' do
    begin
      Ch := ExpStr1[length(ExpStr1)];
      if Ch = '1'
      then
        begin
          Write(a0:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(Ch + ExpStr2);
          GoToXY(WhereX, WhereY + 1);
          x0 := WhereX;
          Write(' = ', a0:1:0);
          if ExpStr2 <> ''
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(ExpStr2);
              GoToXY(WhereX, WhereY + 1);
              Write('ù', a0:1:0)
            end;
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr3);
          GoToXY(WhereX, WhereY + 1);
          WriteLn;
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
      delete(ExpStr1, length(ExpStr1), 1);
      ExpStr2 := Ch + ExpStr2;
      if ExpStr1 <> ''
      then
        begin
          Write(a0:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr3 + '0');
          GoToXY(WhereX, WhereY + 1);
          x0 := WhereX;
          Write(' = ', a0:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr3);
          GoToXY(WhereX, WhereY + 1);
          Write('ù', a0:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(ExpStr3);
          GoToXY(WhereX, WhereY + 1);
          WriteLn;
          WriteLn;
          GoToXY(x0, WhereY);
          WriteLn(' ð ', a:1:0, 'ù', a:1:0);
          WriteLn;
          GoToXY(x0, WhereY);
          a := mult(a, a, m);
          WriteLn(' ð ', a:1:0, ' (mod ', m:1:0, ')');
          WriteLn;
          WriteLn;
          Prompt
        end;
      ExpStr3 := ExpStr3 + '0'
    end;
  WriteLn;
  Write(a0:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write(k:1:0);
  GoToXY(WhereX, WhereY + 1);
  WriteLn(' ð ', p:1:0, ' (mod ', m:1:0, ')');
end.
