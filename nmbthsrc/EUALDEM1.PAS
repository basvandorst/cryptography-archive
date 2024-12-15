program eualdem1;      {demonstrate the Euclidean Algorithm}
{$N+,E+}
uses CRT;
{$I getinput.i }

const
MaxAllow = 1E18;
var
t, r            : extended;
b, c, q         : comp;
x0              : integer;
Ch              : char;

begin
  WriteLn;
  b := GetInput(WhereX, WhereY, '     Enter b = ',
       'where 0 ó b ó 1000000000000000000', 0, MaxAllow);
  c := GetInput(WhereX, WhereY, '      Enter c = ',
       'where 0 ó c ó 1000000000000000000', 0, MaxAllow);
  WriteLn;
  if (b = 0) and (c = 0)
  then
    begin
      WriteLn('(0, 0) is undefined.');
      halt
    end;
  Write('  (', b:1:0, ', ', c:1:0, ')');
  while c <> 0 do
    begin
      t := b/c;
      q := t;
      if q > t then q := q - 1;       {force round down}
      r := b - q*c;                   {b = qc + r; the division algorithm}
      WriteLn(' = (', q:1:0, '*', c:1:0, ' + ', r:1:0, ', ', c:1:0, ')');
      b := c;
      c := r;
      Write('= (', b:1:0, ', ', c:1:0, ')');
      if c <> 0
      then
        begin
          x0 := WhereX;
          Write('  Press any key to continue...');
          Ch := ReadKey;
          GoToXY(x0, WhereY);
          ClrEoL
        end
    end;
  WriteLn(' = ', Abs(b):1:0);
  WriteLn
end.
