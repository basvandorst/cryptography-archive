program CRTDem;                      {Demonstration of the Chinese
                                      Remainder Theorem}
{$N+,E+}
uses CRT, nothy;
{$I GetInput.i }

var
a1,                       {The first residue class}
m1,                       {The first modulus}
a2,                       {The second residue class}
a3,                       {another residue class}
m2,                       {The second modulus}
a,                        {The resulting residue class}
m,                        {The resulting modulus}
r1,                       {a1 (mod (m1, m2))}
r2                        {a2 (mod (m1, m2))}
         : comp;

x, x1    : extended;
code                      {Error code in translating a string
                           to a real number}
         : integer;

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
  Write('                         Press any key to continue ... ');
  Ch := ReadKey;
  if Ch = #0
  then Ch := ReadKey;
  GoToXY(1, 25);
  ClrEoL;
  GoToXY(x0, y0)
end;                               {of procedure Prompt}

begin                              {main body}
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
  ClrScr;
  WriteLn('     We determine the intersection of the arithmetic progressions');
  WriteLn('         x ð ', a1:1:0, ' (mod ', m1:1:0, '),');
  WriteLn('         x ð ', a2:1:0, ' (mod ', m2:1:0, ').');
  Write('The first arithmetic progression consists of those');
  WriteLn(' x of the form');
  Write(m1:1:0, 'j + ', a1:1:0, '.  We wish to determine those values');
  WriteLn(' of j');
  Write('for which this number lies in the second arithmetic ');
  WriteLn('progression, i.e.');
  Write('        ', m1:1:0, 'j + ', a1:1:0, ' ð ', a2:1:0, ' (mod ');
  WriteLn(m2:1:0, ').');
  Prompt;
  a3 := condition(a2 - a1, m2);
  Write('Since ', a2:1:0, ' - ', a1:1:0, ' ð ', a3:1:0);
  WriteLn(' (mod ', m2:1:0, '),');
  WriteLn('this gives the linear congruence');
  WriteLn('       ', m1:1:0, 'j ð ', a3:1:0, ' (mod ', m2:1:0, ').');
  Prompt;
  WriteLn('By an appeal to the procedure LINCON, we find that this ');
  Write('congruence has ');
  LinCon(m1, a3, m2, a, m);
  if m = 0
  then
    begin
      Write('no solution because (', m1:1:0, ', ', m2:1:0, ') = ');
      WriteLn(a:1:0, ',');
      WriteLn('and ', a:1:0, ' does not divide ', a3:1:0, '.');
      Prompt;
      WriteLn('     In summary, the simultaneous congruences');
      WriteLn('               x ð ', a1:1:0, ' (mod ', m1:1:0, '),');
      WriteLn('               x ð ', a2:1:0, ' (mod ', m2:1:0, ')');
      WriteLn('have no solution because');
      Write('         ', a:1:0, '³', m1:1:0, ',');
      WriteLn('    ', a:1:0, '³', m2:1:0, ',');
      Write('but ', a1:1:0, ' and ', a2:1:0, ' are distinct (mod ');
      WriteLn(a:1:0, ').');
      WriteLn('Indeed,');
      Write('             ', a1:1:0, ' ð ', condition(a1, a):1:0);
      WriteLn(' (mod ', a:1:0, '),');
      Write('             ', a2:1:0, ' ð ', condition(a2, a):1:0);
      WriteLn(' (mod ', a:1:0, ').')
    end
  else
    begin
      WriteLn('the unique solution ');
      WriteLn('         j ð ', a:1:0, ' (mod ', m:1:0, ').');
      Prompt;
      WriteLn('That is, j is of the form');
      WriteLn('          j = ', m:1:0, 'k + ', a:1:0, '.');
      Prompt;
      WriteLn('Hence');
      WriteLn('         x = ', m1:1:0, 'j + ', a1:1:0);
      Write('           = ', m1:1:0, '(', m:1:0, 'k + ', a:1:0);
      Write(') + ', a1:1:0);
      x := m;
      x1 := m1;
      x := x*x;
      if x > MaxAllow
      then
        begin
          WriteLn('.');
          Prompt;
          WriteLn('That is, x ð a (mod m) where ');
          WriteLn('         a = ', m1:1:0, 'ù', a:1:0, ' + ', a1:1:0, ',');
          WriteLn('         m = ', m1:1:0, 'ù', m:1:0, '.');
          Write('Here a and m are well-defined, but we would need double-');
          WriteLn('precision');
          WriteLn('arithmetic to find their decimal representations.');
          Prompt;
          WriteLn('     In summary, the simultaneous congruences');
          WriteLn('               x ð ', a1:1:0, ' (mod ', m1:1:0, '),');
          WriteLn('               x ð ', a2:1:0, ' (mod ', m2:1:0, ')');
          WriteLn('are satisfied by precisely those x for which');
          WriteLn('               x ð a (mod m).')
        end
      else
        begin
          WriteLn;
          m := m*m1;
          a := a*m1 + a1;
          WriteLn('           = ', m:1:0, 'k + ', a:1:0, '.');
          Prompt;
          WriteLn('That is,');
          WriteLn('          x ð ', a:1:0, ' (mod ', m:1:0, ').');
          Prompt;
          WriteLn('     In summary, the simultaneous congruences ');
          WriteLn('               x ð ', a1:1:0, ' (mod ', m1:1:0, '),');
          WriteLn('               x ð ', a2:1:0, ' (mod ', m2:1:0, ')');
          WriteLn('are satisfied by precisely those x for which');
          WriteLn('               x ð ', a:1:0, ' (mod ', m:1:0, ').')
        end
    end
end.
