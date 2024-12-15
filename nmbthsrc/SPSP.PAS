{program spsp;       Shell for the function SPSP}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a, m           :  comp;
x              :  extended;
code,
i              :  integer;
ParamsSet      :  Boolean;

begin
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
             and (x >= -MaxAllow) and (x <= MaxAllow)
          then a := x
          else ParamsSet := False;
          Val(ParamStr(2), x, code);
          if (frac(x) = 0) and (code = 0)
             and (x > 2) and (x <= MaxAllow)
          then m := x
          else ParamsSet := False
        end;
  if not ParamsSet
  then
    begin
      WriteLn('Will apply the strong pseudoprime test base a to m.');
      a := GetInput(WhereX, WhereY, '                 Enter a = ',
       '              where ³a³ ó 1000000000000000000', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, '                 Enter m = ',
      'where m is odd and 3 ó m ó 999999999999999999', 3, MaxAllow - 1)
    end;
if not ParamsSet
then for i := 1 to 3 do
  begin
    GoToXY(1, WhereY - 1);
    ClrEoL
  end;
  if frac(m/2) = 0
  then
    begin
      WriteLn(m:1:0, ' is composite because 2³', m:1:0, '.');
      Halt
    end;
if SPSP(a, m)
then
  begin
    Write('The number ', m:1:0);
    WriteLn(' is a strong probable prime base ', a:1:0, '.')
  end
else
  begin
    WriteLn('The number ', m:1:0, ' is composite ');
    WriteLn('because it fails the strong pseudoprime test base ', a:1:0, '.')
  end
end.
