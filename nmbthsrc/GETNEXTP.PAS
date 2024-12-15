{program GetNextP;      Get the Next Prime after the number named}
{$N+,E+}
uses DOS, CRT, nothy;
{$I timer.i }
{$I GetInput.i }
var
x, u            :  comp;
y, t            :  longint;
i,
code                        {error level in converting string to longint}
                :  integer;
found           :  Boolean;

begin
  if ParamCount = 1
  then
    begin
      Val(ParamStr(1), x, code);
      if (code <> 0) or (x < 0) or (x > 999999999999999988.0)
      then Halt
    end
  else x := (GetInput(WhereX, WhereY, 'Enter an integer x = ',
         '       where 0 ó x ó 1000000000000000000', 0,
                              1000000000000000000.0));
  if x <= 1E9
  then
    begin
      y := round(x);
      SetTimer;
      t := GetNextP(y);
      ReadTimer;
      Delete(TimerString, 1, 20);
      ClrEoL;
      GoToXY(1, WhereY - 1);
      ClrEoL;
      WriteLn('The least prime p > ', x:1:0, ' is ', t:1, '.');
      WriteLn('It took ', TimerString, ' to find this prime.')
    end;
  if (x > 1E9) and (x < 999999999999999989.0)
  then
    begin
      found := false;
      u := x;
      SetTimer;
      repeat
        u := u + 1;
        if spsp(2, u) then found := true;
        if found then if (not spsp(3, u)) then found := false;
        if found then if (not spsp(5, u)) then found := false;
        if found then if (not spsp(7, u)) then found := false;
        if found then if (not spsp(11, u)) then found := false
      until found;
      ReadTimer;
      Delete(TimerString, 1, 20);
      ClrEoL;
      GoToXY(1, WhereY - 1);
      ClrEoL;
      WriteLn('The least prime > ', x:1:0);
      WriteLn('is probably       ', u:1:0, '.');
      WriteLn('Use ProveP to confirm that this number really is prime.');
      WriteLn('It took ', TimerString, ' to find this probable prime.')
    end;
  if x > 999999999999999988.0
  then
    begin
      ClrEoL;
      GoToXY(1, WhereY - 1);
      ClrEoL;
      WriteLn('The least prime >  ', x:1:0);
      WriteLn('             is   1000000000000000003.')
    end
end.