{program crt;           User shell for the procedure CRThm}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a1,                       {The first residue class}
m1,                       {The first modulus}
a2,                       {The second residue class}
m2,                       {The second modulus}
a,                        {The resulting residue class}
m,                        {The resulting modulus}
r1,                       {a1 (mod (m1, m2))}
r2                        {a2 (mod (m1, m2))}
         : comp;

x        : extended;
i,                        {an index}
code                      {Error code in translating a string
                           to a real number}
         : integer;

begin
  if ParamCount = 4
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a1 := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
      then m1 := x
      else Halt;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a2 := x
      else Halt;
      Val(ParamStr(4), x, code);
      if (frac(x) = 0) and (code = 0) and (x > 0) and (x <= MaxAllow)
      then m2 := x
      else Halt
    end
  else
    begin
      Write('Will find the intersection of two given arithmetic ');
      WriteLn('progressions');
      WriteLn('a1 (mod m1), a2 (mod m2) where');
      a1 := GetInput(WhereX, WhereY, 'a1 = ', '', -MaxAllow, MaxAllow);
      m1 := GetInput(WhereX, WhereY, 'm1 = ', '', 1, MaxAllow);
      a2 := GetInput(WhereX, WhereY, 'a2 = ', '', -MaxAllow, MaxAllow);
      m2 := GetInput(WhereX, WhereY, 'm2 = ', '', 1, MaxAllow);
    end;
  CRThm(a1, m1, a2, m2, a, m);
  if ParamCount <> 4 then for i := 1 to 6 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  if m = 0
  then
    begin
      r1 := condition(a1, a);
      r2 := condition(a2, a);
      Write('The arithmetic progressions ', a1:1:0, ' (mod ', m1:1:0, '), ');
      WriteLn(a2:1:0, ' (mod ', m2:1:0, ')');
      WriteLn('have no element in common because');
      WriteLn(a:1:0, '³', m1:1:0, ', ', a:1:0, '³', m2:1:0, ', ');
      WriteLn('but ', a1:1:0, ' ð ', r1:1:0, ' (mod ', a:1:0, '),');
      WriteLn('    ', a2:1:0, ' ð ', r2:1:0, ' (mod ', a:1:0, ').')
    end
  else
    begin
      Write('x ð ', a1:1:0, ' (mod ', m1:1:0, ') and x ð ');
      WriteLn(a2:1:0, ' (mod ', m2:1:0, ')');
      WriteLn('if and only if x ð ', a:1:0, ' (mod ', m:1:0, ').')
    end
end.