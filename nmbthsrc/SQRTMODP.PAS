{program sqrtmodp;    Solve for the SQuaRe-rooT of a (MOD P) when
                      a is a quadratic residue.  The algorithm used is
                      RESSOL, due to Dan Shanks (1972), after Tonelli (1891)}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a, b, r, p    : comp;
x             : extended;
i,
code          : integer;

begin
  if ParamCount = 2
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0)
          and (x >= - MaxAllow) and (x <= MaxAllow)
      then a := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then p := x
      else Halt
    end
  else
    begin
      WriteLn('Will find a solution of xý ð a (mod p).');
      a := GetInput(WhereX, WhereY, 'Enter a = ', '', -MaxAllow, MaxAllow);
      p := GetInput(WhereX, WhereY, 'Enter p = ', '', 2, MaxAllow)
    end;
  r := SqrtModP(a, p);
  if ParamCount <> 2
  then for i := 1 to 3 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  WriteLn(r:1:0,'ý ð ',a:1:0,' (mod ',p:1:0,')')
end.
