{program primroot;     User interface for function primroot}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
p,                    {the prime}
a,                    {the initial point}
g                     {the primitive root}
            :   comp;
x           :   extended;
i,
code        :   integer;
ParamsSet   :   Boolean;

begin
  ParamsSet := False;
  if ParamCount = 1
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (code = 0) and (frac(x) = 0) and (x > 0) and (x <= MaxAllow)
      then
        begin
          p := x;
          a := 0
        end
      else ParamsSet := False
    end;
  if ParamCount = 2
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (code = 0) and (frac(x) = 0) and (x > 0) and (x <= MaxAllow)
      then p := x
      else ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0)
          and (x >= -MaxAllow) and (x <=MaxAllow)
      then a := x
      else ParamsSet := False
    end;
  if not ParamsSet
  then
    begin
      WriteLn('Will find the least primitive root g of p, g > a, where ');
      p := GetInput(WhereX, WhereY, 'p = ', '', 2, MaxAllow);
      a := GetInput(WhereX, WhereY, 'a = ', '', -MaxAllow, MaxAllow)
    end;
  g := primroot(p, a);
  if not ParamsSet
  then for i := 1 to 3 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  if g = 0
  then
    begin
      WriteLn(p:1:0, ' is not prime.');
      Halt
    end;
  if (a = 0) or ((a = 1) and (p > 2))
  then
    begin
      Write('The least positive primitive root of ', p:1:0);
      WriteLn(' is ', g:1:0, '.')
    end
  else
    begin
      Write('The least primitive root g of ', p:1:0, ', g > ', a:1:0);
      WriteLn(', is ', g:1:0, '.')
    end
end.