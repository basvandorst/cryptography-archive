program FastGCD;    {calculates the gcd using the Euclidean Algorithm}
{$N+,E+}
uses DOS, CRT;
{$I timer.i }
{$I GetInput.i }

const
r = 100;            {repetitions}
MaxAllow = 1E18;

var
b, c,               {the given integers}
g                   {their gcd}
       : comp;
i                   {index for the repititions}
       : integer;

function gcd(b, c : comp): comp;
var
r0,                 {old remainder}
r1,                 {new remainder}
q,                  {the quotient}
t                   {a temporary variable, for swapping}
       : comp;
begin       {body of function gcd}
r0 := Abs(b);
r1 := Abs(c);
while r1 > 0 do
  begin
    q := r0/r1;
                      {variables of type comp take only integer values.
                       Thus q is the integer nearest r0/r1, rounding up
                       in case of a tie.}
    if q*r1 > r0
    then q := q - 1;  {This forces rounding down, so that now q = [r0/r1].}
    t := r1;          {Save r1; it will become r0}
    r1 := r0 - q*r1;
    r0 := t
  end;
gcd := r0
end;        {of function gcd}

begin
  WriteLn;
  Write('Will calculate gcd(b, c) ', r:0,' times,');
  WriteLn(' for integers of size ó 10^18.');
  b := GetInput(WhereX, WhereY, '   Enter b = ',
       'where |b| ó 1000000000000000000', -MaxAllow, MaxAllow);
  c := GetInput(WhereX, WhereY, '   Enter c = ',
       'where |c| ó 1000000000000000000', -MaxAllow, MaxAllow);
  if (b = 0) and (c = 0)
  then
    begin
      WriteLn('gcd(0, 0) is undefined.');
      Halt
    end;
  SetTimer;
  for i := 1 to r do
    g := gcd(b, c);
  ReadTimer;
  WriteLn('gcd(', b:1:0, ', ', c:1:0, ') = ', g:1:0, '.');
  WriteLn('This calculation took ', TimerString, '.');
  Write('Divide by ', r:1);
  WriteLn(' to find the time required for one evaluation.');
end.
