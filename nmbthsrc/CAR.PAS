program Car;     {User interface for function Carmichael}
{$N+,E+}
uses CRT, nothy;

var
n,                    {the argument}
c                     {the value of Carmichael's function}
     : comp;
x    : extended;
code : integer;

begin
  if ParamCount = 1
  then
    begin
      Val(ParamStr(1), x, code);
      if (code = 0) and (frac(x) = 0) and (x > 0) and (x <= MaxAllow)
      then n := x
      else Halt;
    end
  else
    begin
      Write('Will calculate Carmichael(n) where n = ');
      repeat
        ReadLn(x);
        if (frac(x) > 0) or (x <= 0) or (x > MaxAllow)
        then GoToXY(33, WhereY - 1);
      until (frac(x) = 0) and (x > 0) and (x <= MaxAllow);
      n := x
    end;
  c := Carmichael(n);
  if ParamCount <> 1
  then
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  WriteLn('Carmichael(', n:1:0, ') = ', c:1:0)
end.