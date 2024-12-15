{program mult;           User shell to calculate a*b (mod m)}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a, b,                     {the factors}
m,                        {the modulus}
c                         {the resulting residue class}
         : comp;

x        : extended;
i,                        {an index}
code                      {Error code in translating a string
                           to a real number}
         : integer;

begin              {main body}
  if ParamCount = 3
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0)
          and (x >= -MaxAllow) and (x <= MaxAllow)
      then b := x
      else Halt;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
      then m := x
      else Halt
    end
  else
    begin
      WriteLn('Will find a*b (mod m) where');
      a := GetInput(WhereX, WhereY, 'a = ', '', -MaxAllow, MaxAllow);
      b := GetInput(WhereX, WhereY, 'b = ', '', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'm = ', '', 1, MaxAllow);
    end;
  c :=  mult(a, b, m);
  if ParamCount <> 3
  then for i := 1 to 4 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  WriteLn(a:1:0, '*', b:1:0, ' ð ', c:1:0, ' (mod ', m:1:0, ')');
end.
