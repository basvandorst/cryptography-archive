program Lucas;                  {Compute the Lucas functions U(n), V(n)
                                 (mod m), where U(0) = 0, U(1) = 1,
                                  V(0) = 2, V(1) = a,
                                  U(n+1) = aU(n) + bU(n-1),
                                  V(n+1) = aV(n) + bV(n-1)}
{$N+,E+}
uses nothy, CRT;

{$I GetInput.i }

var
n,                              {The argument}
a, b,                           {parameters}
u, v,                           {The values}
m                               {The modulus}
     : comp;
x    : extended;
i,
code
     : integer;

begin
  If ParamCount = 4
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 0) and (x <= MaxAllow)
      then n := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then a := x
      else Halt;
      Val(ParamStr(3), x, code);
      if (frac(x) = 0) and (code = 0) and (Abs(x) <= MaxAllow)
      then b := x
      else Halt;
      Val(ParamStr(4), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 1) and (x <= MaxAllow)
      then m := x
      else Halt
    end;
  if ParamCount = 2
  then
    begin
      Val(ParamStr(1), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 0) and (x <= MaxAllow)
      then n := x
      else Halt;
      Val(ParamStr(2), x, code);
      if (frac(x) = 0) and (code = 0) and (x >= 1) and (x <= MaxAllow)
      then m := x
      else Halt;
      a := 1;
      b := 1
    end;
  if (ParamCount <> 4) and (ParamCount <> 2)
  then
    begin
      WriteLn('Will compute U(n; a, b), V(n; a, b) (mod m) where');
      WriteLn('U(0) = 0, U(1) = 1, V(0) = 2, V(1) = a,');
      WriteLn('U(n+1) = aU(n) + bU(n-1), V(n+1) = aV(n) + bV(n-1)');
      n := GetInput(WhereX, WhereY, 'Enter index n = ', '', 0, MaxAllow);
      a := GetInput(WhereX, WhereY, 'Enter parameter a = ',
                    '', -MaxAllow, MaxAllow);
      b := GetInput(WhereX, WhereY, 'Enter parameter b = ',
                    '', -MaxAllow, MaxAllow);
      m := GetInput(WhereX, WhereY, 'Enter modulus m = ',
                    '', 1, MaxAllow);
    end;
  u := LucasU(n, a, b, m);
  If (ParamCount <> 4) and (ParamCount <> 2)
  then for i := 1 to 7 do
    begin
      GoToXY(1, WhereY - 1);
      ClrEoL
    end;
  if (a = 1) and (b = 1)
  then WriteLn('F(', n:1:0, ') ð ', u:1:0, ' (mod ', m:1:0, ')')
  else
    begin
      Write('U(', n:1:0, '; ', a:1:0, ', ', b:1:0, ') ð ', u:1:0);
      WriteLn(' (mod ', m:1:0, ')')
    end;
  v := LucasV(n, a, b, m);
  if (a = 1) and (b = 1)
  then WriteLn('L(', n:1:0, ') ð ', v:1:0, ' (mod ', m:1:0, ')')
  else
    begin
      Write('V(', n:1:0, '; ', a:1:0, ', ', b:1:0, ') ð ', v:1:0);
      WriteLn(' (mod ', m:1:0, ')')
    end;
end.
