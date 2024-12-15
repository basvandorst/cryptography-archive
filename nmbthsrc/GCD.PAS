{program gcd;         Shell program for calculating the gcd}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
b0, c0            : extended;
b, c, g           : comp;
i,
code              : integer;
St                : string;

begin
  if ParamCount = 2
  then
    begin
      Val(ParamStr(1), b0, code);
      if (code <> 0) or (frac(b0) > 0) or (Abs(b0) > MaxAllow)
      then Halt;
      b := b0;
      Val(ParamStr(2), c0, code);
      if (code <> 0) or (frac(c0) > 0) or (Abs(c0) > MaxAllow)
      then Halt;
      c := c0;
      GoToXY(1, WhereY - 1)
    end
  else
    begin
      WriteLn('Will calculate (b, c).');
       b := GetInput(WhereX, WhereY, '   Enter b = ',
       'where ³b³ ó 1000000000000000000', -MaxAllow, MaxAllow);
       c := GetInput(WhereX, WhereY, '   Enter c = ',
       'where ³c³ ó 1000000000000000000', -MaxAllow, MaxAllow);
       for i := 1 to 3 do
         begin
           ClrEoL;
           GoToXY(1, WhereY - 1)
         end
  end;
  ClrEoL;
  g := gcd(b, c);
  WriteLn('(', b:1:0, ', ', c:1:0, ') = ', g:1:0, '.')
end.
