unit nothy;
{$N+,E+}
interface
uses CRT;

const
MaxAllow = 1E18;            {The procedures and functions which follow
                             are intended to run for integers of size
                             up to MaxAllow.}

type
primes        = array[1..15] of comp;
multiplicity  = array[1..15] of integer;
                            {Arrays of these types are used in the
                             procedure Canonic.}

function GCD(b, c : comp) : comp;
Procedure Canonic(n : comp; var k : integer; var p : primes;
 var m : multiplicity; Prog : Boolean);
function GetNextP(x : longint) : longint;
function condition(a, m : comp) : comp;
function mult(x, y, m : comp)  :  comp;
function power(a, b, m : comp)  :  comp;
procedure LinCon(a1, a0, m:  comp; var a, m1 : comp);
procedure CRThm(a1, m1, a2, m2 : comp; var a, m : comp);
function phi(n : comp) : comp;
function Carmichael(n : comp) : comp;
function primroot(p, a  : comp) : comp;
function order(a, m, c : comp) : comp;
function SqrtModP(a, p : comp) : comp;
function SPSP(a, m : comp) : Boolean;
function Jacobi(P, Q : comp) : integer;
function LucasU(n, a, b, m : comp) : comp;
function LucasV(n, a, b, m : comp) : comp;

implementation

function GCD(b, c : comp) : comp;

var
q, t       : comp;

begin                       {body of GCD}
if (b = 0) and (c = 0)
then
  begin
    WriteLn('gcd(0, 0) is undefined.');
    Halt
  end;
while c <> 0 do
  begin
    q := b/c;        {an integer quotient, rounded to nearest integer}
    t := c;
    c := b - q*c;
    b := t
  end;
gcd := Abs(b)
end;                        {of function GCD}


Procedure Canonic(n : comp; var k : integer; var p : primes;
 var m : multiplicity; Prog : Boolean);
    {construct the canonical factorization of n.  k is the number of
     distinct primes dividing n; the primes and their multiplicities are
     in the two arrays. Prog determines whether the procedure reports
     progress made to the screen.}

var
  d,                        {trial divisor}
  r                         {part of n which remains to be factored}
         :  comp;
  q                         {test quotient}
         :  extended;
  i                         {index for arrays}
         :  integer;
  Ch     :  char;

procedure test(d : comp;  var r : comp);  {trial division}

var
x0                          {cursor position}
         : integer;

begin                       {body of Canonic}
  q := r/d;
  if frac(q) = 0
  then
    begin
      k := k + 1;
      p[k] := d;
      m[k] := 1;
      r := int(q);
      if Prog
      then
        begin
          GoToXY(WhereX + 2, WhereY);
          Write(d:1:0);
          x0 := WhereX;
          GoToXY(34, WhereY + 1);
          ClrEoL;
          Write(r:1:0);
          GoToXY(x0, WhereY - 1)
        end;
      q := r/d;
      while frac(q) = 0 do
        begin
          m[k] := m[k] + 1;
          r := q;
          if Prog
          then
            begin
              GoToXY(WhereX, WhereY - 1);
              Write(m[k]);
              GoToXY(34, WhereY + 2);
              ClrEoL;
              Write(r:1:0);
              GoToXY(x0, WhereY - 1)
            end;
          q := r/d
        end;
    end;
end;      {of procedure test (which is nested within procedure canonic)}

begin                       {main body of canonic}
if n < 0
then
  begin
    WriteLn('The procedure Canonic does not accept numbers ó 0 ');
    WriteLn('such as ', n:1, '.');
    Halt
  end;
if n > 1E18
then
  begin
    WriteLn('The procedure Canonic does not handle numbers ');
    WriteLn('> 10^18, such as ', n:1:0, '.');
    Halt
  end;
r := n;
k := 0;
for i := 1 to 15 do         {initialize the arrays}
  begin
    p[i] := 0;
    m[i] := 0
  end;
test(2, r);
test(3, r);
test(5, r);
d := 1;
while d*d < r do
  begin
    if KeyPressed
    then
      begin
        Ch := ReadKey;
        WriteLn;
        WriteLn;
        ClrEoL;
        Write('Attempt to factor interrupted after testing all ');
        WriteLn('trial divisors ó ', d:1:0, '.');
        if not Prog
        then
          begin
            ClrEoL;
            WriteLn;
            ClrEoL;
            if k = 0
            then WriteLn('No prime factors of ', n:1:0, ' were found.')
            else
              begin
                Write('Found the prime factors ');
                for i := 1 to k do
                  begin
                    Write(p[i]:1:0);
                    if m[i] > 1
                    then
                      begin
                        GoToXY(WhereX, WhereY - 1);
                        Write(m[i]:1);
                        GoToXY(WhereX, WhereY + 1)
                      end;
                    if i < k then Write(', ')
                  end;
                WriteLn('.');
                Write('The further factor ', r:1:0, ' remains ');
                WriteLn('to be factored.')
              end
          end;
        Halt
      end;
    d := d + 6;             {the trial divisors step through the reduced
                             residue classes (mod 30), starting with 7.}
    test(d, r);
    d := d + 4;
    test(d, r);
    d := d + 2;
    test(d, r);
    d := d + 4;
    test(d, r);
    d := d + 2;
    test(d, r);
    d := d + 4;
    test(d, r);
    d := d + 6;
    test(d, r);
    d := d + 2;
    test(d, r)
  end;
if r > 1 then
  begin
    k := k + 1;
    p[k] := r;
    m[k] := 1
  end
end;                         {of procedure Canonic}

function GetNextP(x : longint) : longint;

var
table              : array[1..103] of Boolean;
t, y, r, i, j      : longint;
tl                          {Table Length}
                   : integer;
n                  : real;
pf                          {Prime found}
                   : Boolean;

procedure sift(i : longint);

var
k, m, j     : longint;

begin
  k := 2*i + 1;
  j := (i - r) mod k;
  if j <= 0 then j := j + k;  {j is the least positive integer such
                               that 2(r+j) + 1 is a multiple of k}
  while j <= tl do
    begin
      if i < r + j     {if k < 2(r + j) + 1}
      then table[j] := false;
      j := j + k
    end;
end;                        {of procedure sift}

begin                       {body of function GetNextP}
  if x < 0
  then
    begin
      GetNextP := 0;
      exit
    end;
  if x > 1E9
  then
    begin
      GetNextP := 0;
      exit
    end;
  if x < 2
  then
    begin
      GetNextP := 2;
      exit
    end;
  tl := trunc(5*ln(x));
  pf := false;
  repeat
    t := x MOD 2;
    y := x + t;
    r := (y DIV 2) - 1;              {2(r+1) + 1 is the least odd number > x.
                                      The ith entry of the table corresponds
                                      to the odd number 2(r+i) + 1.}
    FillChar(table, tl, true);
    n := (round(Sqrt(2*(r+tl)+1))-1)/2;
    sift(1);
    sift(2);
    i := 3;
    repeat
      sift(i);
      i := i + 2;
      sift(i);
      i := i + 1;
      sift(i);
      i := i + 2;
      sift(i);
      i := i + 1;
      sift(i);
      i := i + 2;
      sift(i);
      i := i + 3;
      sift(i);
      i := i + 1;
      sift(i);
      i := i + 3
    until i > n;
    j := 1;
    while (table[j] = false) and (j <= tl) do j := j + 1;
    if j <= tl
    then pf := true
    else x := 2*(r+tl) + 1
  until pf;
  GetNextP := 2*(r + j) + 1
end;                            {of function GetNextP}


function condition(a, m : comp) : comp;

var
b, t     : comp;

begin
  if m <= 0
  then
    begin
      WriteLn('The modulus ', m:1:0, ' must be positive.');
      Halt
    end;
  if m > MaxAllow
  then
    begin
      WriteLn('The modulus ', m:1:0, ' must not exceed 1E18.');
      Halt
    end;
  t := a/m;
  b := a - t*m;                {Thus b ð a (mod m), ³b³ ó m/2}
  if b < 0
  then b := b + m;             {Force 0 ó b < m}
  condition := b
end;                {of function condition}

function mult(x, y, m : comp) : comp;

const
CutOff = 1E9;

var
m0,                        {m = m1*CutOff + m0}
q0,                        {q = q1*CutOff + q0}
t,                         {temporary variable}
x0,                        {x = x1*CutOff + x0}
y0,                        {y = y1*CutOff + y0}
z                       :     extended;
q, x1, y1, q1, m1       :     comp;

begin
  m := Abs(m);
  if Abs(x) >= m
  then
    begin
      q := x/m;
      x := x - m*q;
    end;
  if Abs(y) >= m
  then
    begin
      q := y/m;
      y := y - m*q
    end;
  if m <= 1E9
  then
    begin
      t := x*y;
      q := t/m;
      z := t - q*m;
      if z < 0 then z := z + m;
      mult := z;
      exit
    end;
  if m <= 1E12
  then
    begin
      x1 := x/1E6;
      x0 := x - x1*1E6;
      z := 1E6*y;
      q := z/m;
      z := z - q*m;
      z := x1*z + x0*y;
      q := z/m;
      z := z - q*m;
      if z < 0 then z := z + m;
      mult := z;
      exit
    end;
  if m <= 1E18
  then
    begin
      x1 := x/cutoff;
      x0 := x - x1*cutoff;
      y1 := y/cutoff;
      y0 := y - y1*cutoff;
      q := x*y/m;
      q1 := q/cutoff;
      q0 := q - q1*cutoff;
      m1 := m/cutoff;
      m0 := m - cutoff*m1;
      z := ((x1*y1 - q1*m1)*cutoff + x1*y0 + x0*y1 - q1*m0 - q0*m1)*cutoff
           + x0*y0 - q0*m0;
      if z < 0
      then z := z + m;
      mult := z;
      exit
    end;
  if m > 1E18
  then
    begin
      WriteLn('Cannot multiply residue classes');
      WriteLn('because the modulus ', m:1:0, ' is too big.');
      Halt
    end;
end;                            {of function mult}
(*
function mult(x, y, m : comp) : comp;

const
CutOff = 1E9;

var
m0, m1,                    {m = m1*CutOff + m0}
q,                         {quotient, rounded to nearest integer}
q0, q1,                    {q = q1*CutOff + q0}
t,                         {temporary variable}
x0, x1,                    {x = x1*CutOff + x0}
y0, y1,                    {y = y1*CutOff + y0}
c0, c1, c2,                {x*y = c2*1E18 + c1*1E9 + c0}
d0, d1, d2,                {q*m = d2*1E18 + d1*1E9 + d0}
z                       :     comp;
w                       :     extended;

begin
  m := Abs(m);
  q := x/m;
  x := x - m*q;
  q := y/m;
  y := y - m*q;
  if m <= 1E9
  then
    begin
      t := x*y;
      q := t/m;
      z := t - q*m;
      if z < 0 then z := z + m;
      mult := z;
      exit
    end;
  if m <= 1E12
  then
    begin
      x1 := x/1E6;
      x0 := x - x1*1E6;
      z := 1E6*y;
      q := z/m;
      z := z - q*m;
      z := x1*z;
      z := z + x0*y;
      q := z/m;
      z := z - q*m;
      if z < 0 then z := z + m;
      mult := z;
      exit
    end;
  if m <= 1E18
  then
    begin
      x1 := x/cutoff;
      x0 := x - x1*cutoff;
      y1 := y/cutoff;
      y0 := y - y1*cutoff;
      c2 := x1*y1;
      c1 := x1*y0 + x0*y1;
      c0 := x0*y0;
      w := x*y;
      q := w/m;
      q1 := q/cutoff;
      q0 := q - q1*cutoff;
      m1 := m/cutoff;
      m0 := m - cutoff*m1;
      d2 := q1*m1;
      d1 := q1*m0 + q0*m1;
      d0 := q0*m0;
      z := ((c2 - d2)*cutoff + c1 - d1)*cutoff + c0 - d0;
      if z < 0
      then z := z + m;
      mult := z;
      exit
    end;
  if m > 1E18
  then
    begin
      WriteLn('Cannot multiply residue classes');
      WriteLn('because the modulus ', m:1:0, ' is too big.');
      Halt
    end;
end;                            {of function mult}
*)
function power(a, b, m : comp)  :  comp;

var
  p      :  comp;   {product being formed}
  e      :  real;   {= 0 or 1 -- the value of b (mod 2)}
  q      :  comp;   {quotient}
begin
p := 1;
if (b < 0) then
  begin
    WriteLn('Exponent ',b:1:0,' must be  non-negative.');
    Halt
  end;
while b > 0 do
  begin
    q := b/2;                            {Thus q = [b/2] or [b/2] + 1}
    e := b - q*2;
    if e = -1 then q := q - 1;           {Force q = [b/2]}
    if e <> 0 then p := mult(a, p, m);
    a := mult(a, a, m);
    b := q
  end;
power := p
end;   {of function power}

procedure LinCon(a1, a0, m:  comp; var a, m1 : comp);
                 {Solve the LINear CONgruence a1*x ð a0 (mod m).
                 If (a1, m) does not divide a0 then there is no solution,
                 and the procedure assigns a := (a1, m), m1 := 0.}
var
r0,              {old remainder}
r1,              {new remainder}
u0,              {old coefficient}
u1,              {new remainder}
q,               {quotient, rounded to nearest integer}
t                {temporary variable}
     : comp;
q0               {quotient, a real number}
     : extended;

begin
a1 := condition(a1, m);
a0 := condition(a0, m);
u0 := 0; u1 := 1;      {initialize sequence of coefficients}
r0 := m; r1 := a1;     {initialize sequence of remainders}
while r1 <> 0 do
  begin                {Euclidean algorithm}
    q := r0/r1;        {qotient of remainders is rounded to nearest integer}
    t := r1;           {save remainder}
    r1 := r0 - q*r1;   {construct new remainder, which may be negative}
    r0 := t;           {former remainder is now old remainder}
    t := u1;           {save coefficient}
    u1 := u0 - q*u1;   {construct new coefficient}
    u0 := t            {former coefficient is now old coefficient}
  end;
if r0 < 0
then
  begin
    r0 := -r0;
    u0 := -u0
  end;                 {r0 = (a1, m), a*u0 = r0 (mod m)}
q0 := a0/r0;
if frac(q0) > 0          {must have (a1, m)³a0}
then
  begin
    a := r0;
    m1 := 0
  end
else                   {(a1, m)³a0}
  begin
    m1 := m/r0;
    q := q0;             {The variable q, of type comp, is assigned to
                          be the integer nearest to the real variable q0}
    a := mult(u0, q, m1)
  end
end;   {of procedure LinCon}

procedure CRThm(a1, m1, a2, m2 : comp; var a, m : comp);

                                     {Chinese Remainder THeoreM, merging
                                      x ð a1 (mod m1), x ð a2 (mod m2)
                                      into x ð a (mod m).}
var
x, x1   : extended;
begin

LinCon(m1, a2-a1, m2, a, m);
x := m;
x1 := m1;
x := x1*x;        {multiply m and m1 as real numbers}
if x > MaxAllow
then
  begin
    WriteLn('There is a solution, but the resulting modulus');
    WriteLn('is too large to write with complete accuracy.');
    Halt
  end;
m := x;
if m > 0 then a := m1*a + a1;
end;                {of procedure CRThm}

function phi(n : comp) : comp;      {Euler's phi function}

var
f     : comp;
i, j,                      {indices}
k                          {the number of distinct primes dividing n}
      : integer;
p     : primes;            {"primes" = "array[1..15] of comp"}
m     : multiplicity;      {"multiplicity" = "array[1..15] of integer}

begin
if n <= 0
then
  begin
    WriteLn('phi(', n:1:0, ') is undefined.');
    Halt
  end;
if n > MaxAllow
then
  begin
    WriteLn('Cannot evaluate phi(n) because n = ', n:1:0, ' is too large.');
    Halt
  end;
canonic(n, k, p, m, false);
f := 1;
for i := 1 to k do
  begin
    f := f*(p[i] -1);
    if m[i] > 1
    then for j := 1 to m[i] - 1 do f := f*p[i];
  end;
phi := f
end;                       {of function phi}

function Carmichael(n : comp) : comp;      {Carmichael's function}

var
f, c, g
      : comp;
i, j,                      {indices}
k                          {the number of distinct primes dividing n}
      : integer;
p     : primes;            {"primes" = "array[1..15] of comp"}
m     : multiplicity;      {"multiplicity" = "array[1..15] of integer}

begin
  if n <= 0
  then
    begin
      WriteLn('Carmichael(', n:1:0, ') is undefined.');
      Halt
    end;
  if n > MaxAllow
  then
    begin
      WriteLn('Cannot evaluate Carmichael(n) because n = ', n:1:0, ' is too large.');
      Halt
    end;
  canonic(n, k, p, m, false);
  c := 1;
  for i := 1 to k do
    begin
      f := p[i] - 1;
      if m[i] > 1
      then for j := 2 to m[i] do f := f*p[i];
      if (p[i] = 2) and (m[i] > 2) then f := f/2;
      g := gcd(f, c);
      c := f*c;
      c := c/g
    end;
  Carmichael := c
end;                       {of function Carmichael}

function primroot(p, a  : comp) : comp;

                       {Returns the least primitive root g of p such that
                        g > a.  Returns 0 if p is found to be composite.
                        When a primitive root is found, p is proved to be
                        prime.}

const
Limit = 200;

var
c, g, t
      :  comp;
pr    :  primes;
m     :  multiplicity;
i,
k
      :  integer;
x     :  extended;

begin
  if p = 2
  then
    begin
      x := a/2;
      if frac(x) = 0
      then primroot := a + 1
      else primroot := a + 2;
      exit
    end;
  canonic(p-1, k, pr, m, False);
                      {Get the canonical factorization of p - 1.
                       The number of distinct primes dividing p - 1
                       is k, the primes are in the array "pr",
                       and their multiplicities in "m".}
  c := 1;
  g := a;
  while (c = 1) and (g < a + Limit) do
    begin
      g := g + 1;
      t := condition(g, p);
      if t = 0 then g := g + 2;
      if (t = 1) or (t = 4) then g := g + 1;
      if power(g, p-1, p) <> 1
      then
        begin
          primroot := 0;
          exit
        end;
      i := 0;
      repeat
        i := i + 1;
        c := power(g, (p-1)/pr[i], p)
      until (c = 1) or (i = k);
      if c > 1 then
        begin
          primroot := g;
          exit
        end
  end;
  Write('The number ',p:1:0,' has no primitive root ');
  WriteLn('g for which ', a:1:0, ' < g ó ', (a + Limit):1:0, '.');
  Halt
end;      {of function primroot}

function order(a, m, c : comp) : comp;
                       {The order of a (mod m).  If (a, m) > 1 it returns 0.
                        c is a multiple of the order, such as phi(m) or
                        Carmichael(m)}
var
i, j,                               {indices}
k                                   {The number of prime factors of c}
                : integer;
f,                                  {initially c, eventually ord}
r,
v
                : comp;
p                                   {An array containing the
                                     prime factors of c}
                : primes;
mul                                 {An array giving the multiplicity
                                     of the prime divisors of c}
                : multiplicity;

begin
  if gcd(a, m) > 1
  then
    begin
      order := 0;
      exit
    end;
  if condition(a, m) = 1
  then
    begin
      order := 1;
      exit
    end;
  if power(a, c, m) <> 1
  then
    begin
      WriteLn('Error in calculating the order of ', a:1:0, ' (mod ', m:1:0, ')');
      WriteLn('because ', c:1:0, ' is not a multiple of the order of a (mod m).');
      Halt
    end;
  canonic(c, k, p, mul, False);
  f := c;
  for i := 1 to k do
    begin
      v := 1;
      j := mul[i];
      while (j > 0) and (v = 1) do
        begin
          r := f/p[i];
          j := j - 1;
          v := power(a, r, m);
          if v = 1 then f := r
        end;
    end;
 order := f
end;                                {of the function order}

function SqrtModP(a, p : comp) : comp;

var
i, k0, k1                 :  integer;
b, c, e, m, n,
r, t, t0, t1, z           :  comp;

begin
if p < 2
then
  begin
    WriteLn('SqrtModP can be applied only with a positive prime modulus--');
    WriteLn('p = ', p:1:0, ' is unacceptable.');
    Halt
  end;
t := a/p;
a := a - t*p;
if a < 0 then a := a + p;
if p = 2
then
  begin
    sqrtmodp := a;
    exit
  end;
if a = 0
then
  begin
    sqrtmodp := 0;
    exit
  end;
t := p/2;
if 2*t = p
then
  begin
    WriteLn(p:1:0,' is composite because ', p:1:0, ' is even.');
    Halt
  end;
t := p - 1;
e := t/2;
k0 := -1;
repeat
  m := t;
  t := m/2;
  k0 := k0 + 1
until (2*t <> m);   {k0 is now the power of 2 in p - 1, and
                      m is the odd part of p - 1}
t := power(a, (m-1)/2, p);
r := mult(a, t, p);
t := mult(t, t, p);
n := mult(a, t, p);
if n > 1
then
  begin
    z := 10000;
    repeat
      z := z + 1;
      t := power(z, e, p);
      if (t > 1) and (t < p - 1)
      then
        begin
          WriteLn('Error in SqrtModP: p = ', p:1:0,' is composite');
          WriteLn('because ',z:1:0, '^((p-1)/2) ð ', t:1:0, ' (mod p),');
          WriteLn('not ñ1 (mod p) as it should be.');
          Halt
        end;
    until t = p - 1;       {z is a quadratic nonresidue}
    c := power(z, m, p);
    t1 := n;
    k1 := 0;
    repeat
      t0 := t1;
      t1 := mult(t0, t0, p);
      k1 := k1 + 1
    until (t1 = 1) or (k1 > k0);
                                        {n has order 2^k1 or p is composite}
    if k1 > k0
    then
      begin
        WriteLn('Error in SqrtModP: p = ', p:1:0,' is composite');
        WriteLn('because ', a:1:0, '^(p-1) ð ', t0:1:0, ' (mod p),');
        WriteLn('not 1 as it should be.');
        Halt
      end;
    if (t1 = 1) and (t0 < p - 1)
    then
      begin
        WriteLn('Error in SqrtModP: p = ', p:1:0, ' is composite');
        WriteLn('because ', t0:1:0, 'ý ð 1 (mod p)');
        WriteLn('but ', t0:1:0, ' is not ð ñ1 (mod p) as it should be.');
        Halt
      end;
    if k1 = k0
    then
      begin
        WriteLn(a:1:0,' is a quadratic nonresidue of ',p:1:0);
        WriteLn('(assuming that ', p:1:0, ' really is prime).');
        Halt
      end;
    repeat
      for i := 1 to k0 - k1 - 1 do c := mult(c, c, p);
      b := c;
      c := mult(b, b, p);
      r := mult(b, r, p);
      n := mult(c, n, p);
      t := n;
      k0 := k1;
      k1 := 0;
      while (t > 1) do
        begin
          t := mult(t, t, p);
          k1 := k1 + 1
        end;
    until n = 1
  end;
if 2*r > p
then r := p - r;
b := mult(r, r, p);
if a <> b
then
  begin
    Write('Error in SqrtModP: In seeking a solution of xý ð ');
    WriteLn(a:1:0, ' (mod ', p:1:0, '),');
    WriteLn('the value x = ', r:1:0, ' was returned, which is incorrect.');
    WriteLn('Are you SURE that ', p:1:0, ' is prime?');
    Halt
  end;
sqrtmodp := r
end;             {of function sqrtmodp}

function SPSP(a, m : comp) : Boolean;

var
d, p      : comp;
i, j      : integer;
x         : extended;

begin
  if m < 2
  then
    begin
      WriteLn('Cannot apply the strong pseudoprime');
      WriteLn(' test to a number < 2,');
      WriteLn('such as ', m:1:0, '.');
      Halt
    end;
  x := a/m;
  if frac(x) = 0
  then
    begin
      SPSP := True;
      exit
    end;
  j := 0;
  d := m - 1;
  x := d/2;
  if frac(x) > 0
  then
    if m = 2
    then
      begin
        SPSP := True;
        exit
      end
    else
      begin
        SPSP := False;
        exit
      end;
  SPSP := True;
  while (frac(x) = 0) do
    begin
      j := j + 1;
      d := x;
      x := d/2
    end;
  p := power(a, d, m);
  if p = 1 then exit;
  if p = m - 1 then exit;
  for i := 1 to j - 1 do
    begin
      p := mult(p, p, m);
      if p = m - 1 then exit;
      if p = 1 then
       begin
        SPSP := False;
        Exit
       end
    end;
  SPSP := False
end;                              {of function SPSP}

function Jacobi(P, Q : comp) : integer;

var
t, u               : comp;
J,                               {The Jacobi symbol being formed}
k
                   : integer;
x                  : extended;

begin
  x := Q/2;
  if frac(x) = 0
  then
    begin
      WriteLn('Cannot compute Jacobi symbol (P/Q) when Q is even.');
      WriteLn('Here P = ', P:1:0,', and Q = ', Q:1:0,'.');
      Halt
    end;
  if Q < 1
  then
    begin
      WriteLn('The Jacobi symbol (P/Q) is undefined when Q < 1.');
      WriteLn('Here P = ', P:1:0, ', and Q = ', Q:1:0, '.');
      Halt
    end;
  if P = 0
  then
    begin
      Jacobi := 0;
      Exit
    end;
  J := 1;
  if P < 0
  then
    begin
      P := -P;
      t := Q/4;
      if Q < 4*t                  {if Q ð 3 (mod 4)}
      then J := -J
    end;
while Q > 1 do
  begin
    P := condition(P, Q);
    if P = 0
    then
      begin
        Jacobi := 0;
        exit
      end;
    k := 0;
    x := P/2;
    while frac(x) = 0 do          {while P is even}
      begin
        P := x;
        x := P/2;
        k := k + 1
      end;                        {k is now the power of 2 in P}
    if k mod 2 = 1                {if k is odd}
    then
      begin
        t := Q/8;
        t := Q - 8*t;
        if (t = -3) or (t = 3)    {if Q ð ñ3 (mod 8)}
        then J := -J
      end;
    t := Q;                       {exchange P and Q}
    Q := P;
    P := t;
    t := P/4;
    t := P - 4*t;
    u := Q/4;
    u := Q - 4*u;
    if (t = -1) and (u = -1)      {if P ð Q ð 3 (mod 4)}
    then J := -J
  end;
  Jacobi := J
end;                              {of function Jacobi}

function LucasU(n, a, b, m : comp) : comp;
                            {U(n; a, b) (mod m) where U(0) = 0, U(1) = 1,
                             U(n+1) = aU(n) + bU(n-1).}

var
i,                          {The number of powers of 2 dividing n}
k                           {An index}
        : integer;
t, t0, t1,                  {temporary variables}
u0, u1,                     {Lucas' U(n-1), U(n)}
r,                          {The odd part of n}
j                           {The least power of 2 greater than r}

       : comp;
x       : extended;

procedure Double;

begin
  t := mult(b, u0, m);
  t0 := mult(u1, u1, m) + mult(t, u0, m);
  t := 2*t + mult(a, u1, m);
  t1 := mult(u1, t, m);
  u0 := t0;
  u1 := t1
end;                           {of procedure Double}

procedure SideStep;

begin
  t0 := u1;
  t1 := mult(a, u1, m) + mult(b, u0, m);
  u0 := t0;
  u1 := t1
end;

begin
  if n < 0
  then
    begin
      Write('Error in Lucas: ');
      WriteLn('The Lucas function U(n) is undefined when n < 0.');
      WriteLn('Here n = ', n:1:0, '.');
      Halt
    end;
  if n = 0
  then
    begin
      LucasU := 0;
      exit
    end;
  a := condition(a, m);
  b := condition(b, m);
  i := 0;
  r := n;
  x := r/2;
  while frac(x) = 0 do
    begin
      i := i + 1;
      r := x;
      x := r/2
    end;                      {n = r*2^i}
  j := 1;
  while j <= r do j := 2*j;
  u0 := 0;
  u1 := 1;
  r := 2*r;
  r := r - j;
  while r > 0 do
    begin
      r := 2*r;
      Double;
      if r >= j         {binary digit is 1}
      then
        begin
          r := r - j;
          SideStep
        end;
    end;
  for k := 1 to i do Double;
  u1 := condition(u1, m);
  LucasU := u1
end;                          {of function LucasU}

function LucasV(n, a, b, m : comp) : comp;
                            {V(n; a, b) (mod m) where V(0) = 2, V(1) = a,
                             V(n+1) = aV(n) + bV(n-1).}
var
i,                          {The number of powers of 2 dividing n}
k                           {An index}
        : integer;
t, t0, t1,                  {temporary variables}
v0, v1,                     {Lucas' V(n), V(n+1)}
c,                          {(-b)^n}
r,                          {The odd part of n}
j                           {The least power of 2 greater than r}
        : comp;
x       : extended;

begin
  if n < 0
  then
    begin
      Write('Error in Lucas: ');
      WriteLn('The Lucas function V(n) is undefined when n < 0.');
      WriteLn('Here n = ', n:1:0, '.');
      Halt
    end;
  if n = 0
  then
    begin
      LucasV := 2;
      exit
    end;
  a := condition(a, m);
  b := condition(b, m);
  i := 0;
  c := 1;
  r := n;
  x := r/2;
  while frac(x) = 0 do
    begin
      i := i + 1;
      r := x;
      x := r/2
    end;                      {n = r*2^i}
  j := 1;
  while j <= r do j := 2*j;
  v0 := 2;
  v1 := a;
  while r > 0 do
    begin
      r := 2*r;
      if r >= j         {binary digit is 1}
      then
        begin
          r := r - j;
          t0 := mult(v0, v1, m) - mult(a, c, m);
          t := mult(c, -b, m);
          t1 := mult(v1, v1, m) - 2*t;
          c := mult(c, t, m);
          v0 := t0;
          v1 := t1
        end
      else
        begin
          t0 := mult(v0, v0, m) - 2*c;
          t1 := mult(v0, v1, m) - mult(a, c, m);
          c := mult(c, c, m);
          v0 := t0;
          v1 := t1
        end;
    end;
  for k := 1 to i do
    begin
      v0 := mult(v0, v0, m) - 2*c;
      c := mult(c, c, m);
    end;
  v0 := condition(v0, m);
  LucasV := v0
end;                          {of function LucasV}

end.
