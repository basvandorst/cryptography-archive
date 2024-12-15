{program lincon;           User shell for the procedure LinearCong}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a1,                       {The coefficient of x}
a0,                       {The constant term}
m,                        {The given modulus}
a,                        {The resulting residue class}
m1                        {The resulting modulus}
         : comp;

x        : extended;
i,                        {an index}
code                      {Error code in translating a string
                           to a real number}
         : integer;

begin
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
  LinCon(a1, a0, m, a, m1);
  if ParamCount <> 3 then for i := 1 to 4 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  if m1 = 0
  then
    begin
      Write('The linear congruence ', a1:1:0, 'x ð ', a0:1:0, ' (mod ');
      WriteLn(m:1:0, ')');
      WriteLn('has no solution, because');
      Write('(', a1:1:0, ', ', m:1:0, ') = ', a:1:0);
      WriteLn(' does not divide ', a0:1:0, '.')
    end
  else
    begin
      WriteLn(a1:1:0, 'x ð ', a0:1:0, ' (mod ', m:1:0, ') if and only if');
      WriteLn('x ð ', a:1:0, ' (mod ', m1:1:0, ').')
    end
end.