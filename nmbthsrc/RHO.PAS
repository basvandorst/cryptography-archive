program rho;               {Apply Pollard rho method to argument}
{$N+,E+}
uses DOS, CRT, nothy;
{$I GetInput.i }
{$I timer.i }

var
n,                         {The number to be factored}
c,                         {constant added in recurrence}
g, h,                      {factors found}
u,                         {u(i)}
v                          {u(2i)}
            :  comp;
i,                         {an index}
code                       {error level in converting string to number}
            :  integer;
x           :  extended;
ParamsSet   :  Boolean;

begin
  ParamsSet := False;
  If ParamCount = 1
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then
        begin
          n := x;
          c := 1
        end
      else ParamsSet := False;
    end;
  If ParamCount = 2
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then n := x
      else ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0)
          and (x >= - MaxAllow) and (x <= MaxAllow)
      then c := x
      else ParamsSet := False
    end;
  If not ParamsSet
  then
    begin
      WriteLn('Will employ the Pollard rho method to attempt to find');
      WriteLn('a proper factor of a given number n.  This should be done');
      WriteLn('only for n which are already known to be composite.');
      n := GetInput(WhereX, WhereY, '     Enter n = ',
          'where 1 < n ó 1000000000000000000.', 2, MaxAllow);
      WriteLn('Will use the recursion u(n+1) ð u(n)ý + c (mod n) where');
      c := GetInput(WhereX, WhereY, '           c = ',
          '   with ³c³ ó 1000000000000000000.', -MaxAllow, MaxAllow);
    end;
  SetTimer;
  u := 0;
  v := 0;
  repeat
    u := mult(u, u, n) + c;
    v := mult(v, v, n) + c;
    v := mult(v, v, n) + c;
    g := gcd(u-v, n);
    if KeyPressed then Halt;
    if g = n
    then
      begin
        c := c + 1;
        u := 0;
        v := 0
      end
  until (g > 1) and (g < n);
  h := n/g;
  ReadTimer;
  if not ParamsSet
  then
    begin
      ClrEoL;
      for i := 1 to 6 do
        begin
          GoToXY(1, WhereY - 1);
          ClrEoL
        end
    end;
  WriteLn(n:1:0, ' = ', g:1:0, 'ù', h:1:0);
  WriteLn('Factors found in ', TimerString, '.')
end.

