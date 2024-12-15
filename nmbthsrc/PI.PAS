{program pi;               Calculate ã(x)}
{$N+,E+}
uses DOS, CRT;

const
bl = 5000;              {block length--number of odd integers in a block}
limit = 1000000;        {maximum size of argument accepted--could be made
                         as large as 10^9}
var

smallp        : array[1..15803] of Boolean; {Small primes used for sifting.
                                         Smallp[i] = true if 2i + 1 is prime}
primes        : array[1..3400] of integer;
table         : array[1..bl] of integer;
b,                                 {number of blocks}
h, i, j, k, l,
code                               {error level when using Val}
              : integer;
m, n, n1, r,
pi, x         : longint;

begin
if ParamCount = 1
then
  begin
    if length(ParamStr(1)) > 9 then Halt;
    Val(ParamStr(1), x, code);
    if (code <> 0) or (frac(x) > 0) or (x < 2) or (x > limit)
    then Halt;
  end
else
  begin
    WriteLn('This routine applies sieving to calculate ã(x).');
    WriteLn('     Enter x = ');
    Write('where 2 ó x ó ', limit);
    gotoXY(16, WhereY - 1);
    ReadLn(x);
    ClrEoL;
    if (x < 2) or (x > limit)
    then Halt
  end;
FillChar(Smallp, 15803, 1);
for i := 1 to 86 do
  if smallp[i] = true                 {2i + 1 is prime}
  then
    begin
       k := 2*i + 1;
       j := 2*i*(i+1);                {Start sifting at 2j + 1 = (2i + 1)^2}
       while j <= 15803 do
         begin
           smallp[j] := false;        {2j + 1 is divisible by 2i + 1}
           j := j + k
         end
    end;                           {Array smallp, giving all primes < 31621
                                    has been constructed.  The last prime is
                                    31607, corresponding to i = 15803.  The
                                    array was sifted by all primes < 179,
                                    the last one being 173, corresponding to
                                    i = 86.}
h := 0;
for i := 1 to 15803 do
if smallp[i]
then
  begin
    h := h + 1;
    primes[h] := i
  end;
                                   {The array of sifting primes has been
                                    prepared}
n := 0;
pi := 1;    {count the prime 2}
b := 1 + trunc(x/(2*bl));
for r := 1 to b do
  begin
    for j := 1 to bl do table[j] := 1;
                                   {Table[j] corresponds to the number
                                    n + 2j - 1.}
    if n = 0 then table[1] := 0;   {The number 1 is not prime}
    n1 := n + 2*bl;             {The maximum number in the table}
    l := round((Sqrt(n1)-1)/2);     {Will sift by primes up to 2l + 1}
    h := 1;
    while (primes[h] <= l) and (h <= 3400) do
        begin
          i := primes[h];
          h := h + 1;
          k := 2*i + 1;            {k is the prime we are sifting by}
          m := trunc((n/k + 1)/2);   {(2m+1)k is the least odd multiple
                                    of k larger than n}
          j := round(m*k + i + 1 - n/2);
                                   {This is the corresponding
                                    index in the table}
           while j <= bl do
             begin
               if n/2 + j <> i + 1      {k <> n + 2j - 1}
               then table[j] := 0;
               j := j + k
             end
        end;
                                {table of primes between n and n+2*bl
                                 has now been prepared}
    for j := 1 to bl do
      if (table[j] = 1) and (n + 2*j - 1 <= x)      {if n + 2j - 1 is prime}
      then pi := pi + 1;
    n := n1;
  end;
GoToXY(1, WhereY - 1);
ClrEoL;
if ParamCount <> 1
then
  begin
    GoToXY(1, WhereY - 1);
    ClrEoL;
    GoToXY(1, WhereY - 1);
    ClrEoL
  end;
WriteLn('ã(', x:1, ') = ', pi:1)
end.
