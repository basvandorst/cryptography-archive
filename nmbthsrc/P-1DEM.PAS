{program p-1Dem;            Demonstration of Pollard's p - 1 method}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
n,                         {The number to be factored}
a,                         {The base}
b,                         {a power of the base}
g, h                       {factors found}
             :  comp;
i,                         {indices}
code                       {error level in converting string to number}
             :  integer;
k            :  longint;
x            :  extended;
ParamsSet    : Boolean;

procedure Prompt;

var
x0, y0                    {coordinates of cursor location}
         : integer;
Ch       : Char;

begin
  x0 := WhereX;
  y0 := WhereY;
  if y0 = 25
  then
    begin
      WriteLn;
      y0 := 24
    end;
  GoToXY(1, 25);
  ClrEoL;
  Write('           Press any key to continue ... (or Esc to terminate) ');
  Ch := ReadKey;
  if Ch = #0
  then Ch := ReadKey;
  if Ch = #27
  then Halt;
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(x0, y0)
end;                               {of procedure Prompt}

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
      else ParamsSet := False
    end;
  if ParamCount = 2
  then
    begin
      ParamsSet := True;
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then n := x
      else ParamsSet := False;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 1) and (x <= MaxAllow)
      then a := 2
      else ParamsSet := False
    end;
  if not ParamsSet
  then
    begin
      WriteLn;
      WriteLn('Will employ the Pollard p - 1 method to attempt to find');
      WriteLn('a proper factor of a given number n.  This should be done');
      WriteLn('only for n that are already known to be composite.');
      n := GetInput(WhereX, WhereY, '     Enter n = ',
          'where 1 < n ó 1000000000000000000.', 2, 1E18);
      a := GetInput(WhereX, WhereY, '     Enter base a = ',
          '     where 1 < a ó 1000000000000000000', 2, 1E18);
    end;
  ClrScr;
  WriteLn;
  k := 0;
  b := a;
  i := 0;
  repeat
    i := i + 1;
    k := k + 1;
    Write('(', a:1:0);
    GoToXY(WhereX, WhereY - 1);
    Write(k:1, '!');
    GoToXY(WhereX, WhereY + 1);
    Write(' - 1, ', n:1:0, ') = ');
    b := power(b, k, n);
    g := gcd(b - 1, n);
    Write('(', (b - 1):1:0, ', ', n:1:0, ') = ', g:1:0);
    if i = 12
    then
      begin
        i := 0;
        Prompt;
        ClrScr;
        WriteLn
      end
    else
      begin
        WriteLn;
        WriteLn
      end;
  until g > 1;
  if g = n
  then WriteLn('No proper divisor found.')
  else
    begin
      h := n/g;
      WriteLn(n:1:0, ' = ', g:1:0, 'ù', h:1:0)
    end;
end.
