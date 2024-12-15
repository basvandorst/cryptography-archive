program slowgcd;    {calculates the gcd using only the definition}
uses DOS;
{$I timer.i }

var
b, c,               {the given integers}
g                   {their gcd}
       : longint;

function gcd(b, c : longint) : longint;
var
t,                  {trial common divisor}
d,                  {largest common divisor found so far}
m                   {m = min(|b|, |c|)}
      : longint;
begin               {body of function gcd}
  d := 0;
  if Abs(b) < Abs(c)
  then m := Abs(b)
  else m := Abs(c);
  for t := 1 to m do
  if (b mod t = 0) and (c mod t = 0)
  then d := t;
  gcd := d
end;                {of function gcd}

begin
  WriteLn;
  WriteLn('Will calculate gcd(b, c).');
  Write('Enter b = ');
  ReadLn(b);
  Write('Enter c = ');
  ReadLn(c);
  if (b = 0) and (c = 0)
  then
    begin
      WriteLn('gcd(0, 0) is undefined.');
      Halt
    end;
  SetTimer;
  g := gcd(b, c);
  ReadTimer;
  WriteLn('gcd(', b:1, ', ', c:1, ') = ', g:1, '.');
  WriteLn('This calculation took ', TimerString,'.');
end.

