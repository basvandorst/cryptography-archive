program pwrdem1a;         {Demonstration of calculation of a^k (mod m)}
{$N+,E+}
uses CRT, nothy;
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
a0,                       {original base}
k0                        {original exponent}
         : comp;

x        : extended;
i,                        {an index}
code,                     {Error code in translating a string
                           to a real number}
x0                        {cursor coordinate}
         : integer;
St       : string[80];
Prod                      {Has the formation of the product commenced?}
         : Boolean;

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
  WriteLn;
  Write('   ', a:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write(k:1:0);
  GoToXY(WhereX, WhereY + 1);
  x0 := WhereX;
  k0 := k;
  a0 := a;
  Prod := False;
  p := 1;
  while k > 0 do
    begin
      q := k/2;                            {Thus q = [k/2] or [k/2] + 1}
      e := k - q*2;
      if e = -1
      then
        begin
          q := q - 1;                      {Force q = [k/2]}
          e := 1
        end;
      GoToXY(x0, WhereY);
      Write(' = ', a:1:0);
      GoToXY(WhereX, WhereY - 1);
      Write('2ù', q:1:0);
      if e = 1
      then Write('+1');
      GoToXY(WhereX, WhereY + 1);
      if Prod
      then Write('ù', p:1:0);
      WriteLn;
      WriteLn;
      WriteLn;
      Prompt;
      GoToXY(x0, WhereY);
      Write(' = ');
      if q > 0
      then
        begin
          Write('(', a:1:0, 'ý)');
          GoToXY(WhereX, WhereY - 1);
          Write(q:1:0);
          GoToXY(WhereX, WhereY + 1);
          if (e = 1) or (Prod) then Write('ù')
        end;
      if e = 1
      then
        begin
          Write(a:1:0);
          if Prod then Write('ù')
        end;
      if Prod
      then Write(p:1:0);
      WriteLn;
      WriteLn;
      WriteLn;
      Prompt;
      if e <> 0
      then
        begin
          p := mult(a, p, m);
          Prod := True
        end;
      a := mult(a, a, m);
      k := q;
      GoToXY(x0, WhereY);
      Write(' ð ');
      if k > 0
      then
        begin
          Write(a:1:0);
          GoToXY(WhereX, WhereY - 1);
          Write(k:1:0);
          GoToXY(WhereX, WhereY + 1);
          if Prod
          then Write('ù')
        end;
      if Prod
      then Write(p:1:0);
      WriteLn(' (mod ', m:1:0, ')');
      WriteLn;
      WriteLn;
      Prompt
  end;
  WriteLn;
  WriteLn;
  Write(a0:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write(k0:1:0);
  GoToXY(WhereX, WhereY + 1);
  WriteLn(' ð ', p:1:0, ' (mod ', m:1:0, ')');
end.
