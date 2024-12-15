{program p-1;                 Factorization of n by Pollard's p - 1 method}
{$N+,E+}
uses DOS, CRT, Nothy;
{$I GetInput.i }
{$I Timer.i }

var
n,                           {number to be factored}
k,                           {exponent}
a,                           {base}
b,                           {a power of the base}
g,                           {gcd}
h                            {cofactor}
           : comp;
x                            {raw input}
           : extended;
i,                           {an index}
code       : integer;
ParamsSet  : Boolean;

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
          a := 2
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
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then a := x
      else ParamsSet := False
    end;
if not ParamsSet
then
    begin
      WriteLn('Will employ the Pollard p - 1 method to attempt to find');
      WriteLn('a proper factor of a given number n.  This should be');
      WriteLn('attempted only for n that are already known to be composite.');
      n := GetInput(WhereX, WhereY, 'Enter n = ',
            ' 1 < n ó 1000000000000000000', 2, 1E18);
      a := GetInput(WhereX, WhereY, 'Choose base a = ',
            '       1 < a ó 1000000000000000000', 2, MaxAllow)
    end;
  SetTimer;
  b := power(a, 24, n);
  k := 4;
  repeat
    k := k + 1;
    b := power(b, k, n);
    g := gcd(b - 1, n);
    if KeyPressed
    then
      begin
        WriteLn;
        Write('No success so far: (', a:1:0);
        GoToXY(WhereX, WhereY - 1);
        Write(k:1:0, '!');
        GoToXY(WhereX, WhereY + 1);
        WriteLn(' - 1, ', n:1:0, ') = 1');
        Halt
      end
  until g > 1;
  ReadTimer;
  If not ParamsSet
  then for i := 1 to 5 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  if g = n
  then WriteLn('No proper divisor found.')
  else
    begin
      h := n/g;
      WriteLn(n:1:0, ' = ', g:1:0, 'ù', h:1:0)
    end;
  WriteLn('Calculation took ', TimerString, '.')
end.
