{program jacobi;       Shell for the function Jacobi}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
P, Q           :  comp;
x              :  extended;
code,
i,
J                          {The Jacobi symbol}
               :  integer;

begin
  if (ParamCount = 2)
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0)
         and (x >= -MaxAllow) and (x <= MaxAllow)
      then P := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0)
          and (x > 0) and (x <= MaxAllow)
      then Q := x
      else Halt;
    end
  else
    begin
      WriteLn('Will compute the Jacobi symbol (P/Q).');
      P := GetInput(WhereX, WhereY, 'Enter P = ', '', -MaxAllow, MaxAllow);
      Q := GetInput(WhereX, WhereY, 'Enter Q = ', '', 1, MaxAllow)
    end;
  J := Jacobi(P, Q);
  if ParamCount <> 2
  then for i := 1 to 3 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  WriteLn('(', P:1:0, '/', Q:1:0, ') = ', J:1)
end.

