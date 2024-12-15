program spspdem;              {Demonstration of the Strong PSeudoPrime test}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a,                                   {the base}
m,                                   {the number being tested}
d,                                   {the odd part of m-1}
g,                                   {a gcd}
p,                                   {the power}
p0                                   {the old power}
               :  comp;
x              :  extended;
code,
i, j           :  integer;
ParamsSet      :  Boolean;

procedure Prompt;

var
x0, y0                    {coordinates of cursor location}
           : integer;
Ch         : Char;


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

begin                              {main body}
  ParamsSet := False;
  if ParamCount = 1
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0)
         and (x > 2) and (x <= MaxAllow)
      then
        begin
          m := x;
          a := 2
        end
      else ParamsSet := False
    end;
  if ParamCount = 2
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0)
         and (x >= - MaxAllow) and (x <= MaxAllow)
      then a := x
      else ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0)
        and (x > 2) and (x <= MaxAllow)
      then m := x
      else ParamsSet := False;
    end;
  if not ParamsSet
  then
    begin
      WriteLn('Will apply the strong pseudoprime test base a to m.');
      a := GetInput(WhereX, WhereY, 'Enter a = ',
      '³a³ ó 1000000000000000000', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'Enter m = ',
      '2 < m ó 1000000000000000000', 3, MaxAllow);
    end;
  x := a/m;
  if frac(x) = 0
  then
    begin
      WriteLn(a:1:0, ' ð 0 (mod ', m:1:0, ').  This tells us nothing');
      WriteLn('about whether', m:1:0, ' is prime or composite.');
      Halt
    end;
  if frac(m/2) = 0
  then
    begin
      WriteLn(m:1:0, ' is composite because 2³', m:1:0, '.');
      Halt
    end;
  j := 0;
  d := m - 1;
  x := d/2;
  while (frac(x) = 0) do
    begin
      j := j + 1;
      d := x;
      x := d/2
    end;
  ClrScr;
  WriteLn;
  WriteLn;
  Write(m:1:0, ' - 1 = 2');
  GoToXY(WhereX, WhereY - 1);
  Write(j:1);
  GoToXY(WhereX, WhereY + 1);
  if d > 1
  then Write('ù', d:1:0);
  WriteLn;
  Prompt;
  p := power(a, d, m);
  WriteLn;
  WriteLn;
  Write(a:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write(d:1:0);
  GoToXY(WhereX, WhereY + 1);
  WriteLn(' ð ', p:1:0, ' (mod ', m:1:0, ')');
  WriteLn;
  Prompt;
  if p = 1
  then
    begin
      WriteLn('No inconsistency is disclosed, so ');
      WriteLn(m:1:0, ' is a strong probable prime base ', a:1:0, '.');
      Halt
    end;
  if p = m - 1
  then
    begin
      WriteLn('No inconsistency is disclosed, so ');
      WriteLn(m:1:0, ' is a strong probable prime base ', a:1:0, '.');
      Halt
    end;
  for i := 1 to j - 1 do
    begin
      d := 2*d;
      p0 := p;
      p := mult(p, p, m);
      WriteLn;
      Write(a:1:0);
      GoToXY(WhereX, WhereY - 1);
      Write(d:1:0);
      GoToXY(WhereX, WhereY + 1);
      WriteLn(' ð ', p:1:0, ' (mod ', m:1:0, ')');
      WriteLn;
      Prompt;
      if p = m - 1
      then
        begin
          WriteLn('No inconsistency is disclosed, so ');
          WriteLn(m:1:0, ' is a strong probable prime base ', a:1:0, '.');
          Halt
        end;
      if p = 1 then
       begin
         Write(m:1:0, ' is composite, because ', p0:1:0, 'ý ð 1 (mod ');
         WriteLn(m:1:0, '),');
         WriteLn('but ', p0:1:0, ' is not ð ñ1 (mod ', m:1:0, ').');
         g := gcd(p0 - 1, m);
         WriteLn('BONUS: (', (p0-1):1:0, ', ', m:1:0, ') = ', g:1:0, ',');
         Write('and hence ', g:1:0);
         WriteLn(' is a proper divisor of ', m:1:0, '.');
         Halt
       end
    end;
  WriteLn;
  WriteLn;
  Write('m = ', m:1:0, ' is composite because ', a:1:0);
  GoToXY(WhereX, WhereY - 1);
  Write('(m-1)/2');
  GoToXY(WhereX, WhereY + 1);
  WriteLn(' is not ð ñ1 (mod m).');
end.

