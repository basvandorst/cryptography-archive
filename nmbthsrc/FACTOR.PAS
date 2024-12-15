program factor;          {Shell program for the procedure canonic}
{$N+,E+}
uses DOS, CRT, nothy;
{$I GetInput.i }
{$I timer.i }

var
n0            : extended;
n                                 {number to be factored}
              : comp;
i,                                {an index}
k,                                {number of distinct prime factors found}
code                              {Error level returned when using Val}
              : integer;
p             : primes;
m             : multiplicity;

begin
if ParamCount = 1
then
  begin
    if length(ParamStr(1)) > 19 then Halt;
    Val(ParamStr(1), n0, code);
    if (code <> 0) or (frac(n0) > 0) or (Abs(n0) > MaxAllow)
    then Halt;
    n := n0
  end
else
  begin
    WriteLn('This routine applies trial division.');
    n := GetInput(WhereX, WhereY, '   Enter n = ',
    'where ³n³ ó 1000000000000000000', -MaxAllow, MaxAllow);
  end;
  if n = 0
  then
    begin
      WriteLn('0 has no factorization.');
      Halt
    end;
  if Abs(n) = 1
  then
    begin
      WriteLn(n:1:0, ' = ', n:1:0);
      Halt
    end;
  ClrEoL;
  WriteLn;
  ClrEoL;
  WriteLn('Prime factors found: ');
  ClrEoL;
  Write('Number remaining to be factored: ', n:1:0);
  GoToXY(20, WhereY - 1);
  SetTimer;
  canonic(Abs(n), k, p, m, True);
  ReadTimer;
  GoToXY(1, WhereY + 1);
  for i := 1 to 3 do
    begin
      ClrEoL;
      GoToXY(1, WhereY - 1)
    end;
  if ParamCount <> 1
  then
    begin
      ClrEoL;
      GoToXY(1, WhereY - 1)
    end;
  ClrEoL;
  if (k = 1) and (m[1] = 1)
  then WriteLn(Abs(n):1:0, ' is prime.')
  else
    begin
      WriteLn;
      Write(n:1:0, ' = ');
      if n < 0 then Write('- ');
      for i := 1 to k do
        begin
          Write(p[i]:1:0);
          if m[i] > 1
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(m[i]:1);
              GoToXY(WhereX, WhereY+1)
            end;
          if i < k
          then Write('ù')
          else WriteLn;
        end
    end;
  WriteLn('Factorization found in ', TimerString, '.')
end.