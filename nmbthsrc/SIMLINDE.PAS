program SimLinDE;  {SIMultaneou LINear Diophantine Equations;
                    partial Smith reduction of the coefficient matrix.}
{$N+,E+}
uses CRT;
const
ArraySize = 10;
d = 7;          {Number of columns on screen alloted to each element}

var
DC,                         {Display calculations ?}
zeros,                      {Does the column consist entirely of 0's ?}
ColFixed,                   {Is the column fixed?}
RowFixed                    {Is the row fixed?}
                : Boolean;
i, i1, i2,                  {indices of rows}
j, j1, j2,                  {indices of columns}
m,                          {number of equations}
n,                          {number of variables}
m0,                         {rows after m0 contain only 0's}
n0,                         {columns after n0 contain only 0's}
r                           {rank(A)}
                : integer;
Ch                          {Answer from keyboard}
                : Char;
d1, d2,
t,
w
                : comp;
A,                          {coefficient matrix}
U                           {unimodular matrix generated}
                : array[1..ArraySize,1..ArraySize] of comp;
b, c, q                     {constant terms}
                : array[1..ArraySize] of comp;
OriginalMode    : word;

procedure Test(u : real);      {Test for overflow}

begin
  if Abs(u) > 1E18
  then
    begin
      WriteLn;
      Write('Entry larger than 1E18 encountered -- ');
      WriteLn('calculation aborted.');
      Halt
    end;
end;

procedure Display(z : real);

var
i0, j0 :  integer;

begin
{
WriteLn('Breakpoint #',z:2:2);
WriteLn; Delay(1000);
}
for i0 := 1 to m do
  begin
    for j0 := 1 to n do Write(A[i0,j0]:d:0);
    WriteLn(b[i0]:d:0)
  end;
WriteLn;
for i0 := 1 to n do
  begin
    for j0 := 1 to n do Write(U[i0,j0]:d:0);
    WriteLn
  end;
WriteLn;
end;

begin  {Main body}
OriginalMode := LastMode;
TextColor(14);
TextBackground(1);
ClrScr;
WriteLn('To find the complete solution set of a system of m simultaneous');
WriteLn('linear Diophantine equations in n variables, written in the form');
WriteLn('Ax = b, enter');
repeat
Write('m = ');
ReadLn(m);
if m > ArraySize
then WriteLn('Too large.  Must have m ó ', ArraySize:0,'.');
until m <= ArraySize;
m0 := m;
repeat
Write('n = ');
ReadLn(n);
if n > ArraySize
then WriteLn('Too large.  Must have n ó ', ArraySize:0,'.');
until n <= ArraySize;
n0 := n;
r := 0;
For i := 1 to m do               {Get input}
  begin
    for j := 1 to n do
      repeat
        Write('a(',i:0,', ', j:0,') = ');
        ReadLn(A[i,j]);
        if Abs(A[i,j]) >= 1E9
        then
          begin
            Write('Entry must have absolute value ');
            WriteLn('less than 1E9.  Try again.')
          end;
        if frac(A[i,j]) > 0
        then WriteLn('Entry must be an integer.  Try again.');
      until (Abs(A[i,j]) < 1E9) and (frac(A[i,j]) = 0);
    repeat
      Write('b(', i:0, ') = ');
      ReadLn(b[i]);
      if Abs(b[i]) >= 1E9
      then
        begin
          Write('Entry must have absolute value ');
          WriteLn('less than 1E9.  Try again.');
        end;
      if frac(b[i]) > 0
      then WriteLn('Entry must be an integer.  Try again.');
    until (Abs(b[i]) < 1E9) and (frac(b[i]) = 0);
  end;                           {Input complete}
for i := 1 to n do               {Initialize U}
  for j := 1 to n do
    if i = j
    then U[i,j] := 1
    else U[i,j] := 0;            {U is now an nxn identity matrix}
ClrScr;
WriteLn('The initial array is:');
WriteLn;
Display(1);
WriteLn;
Delay(1000);
repeat
  Write('Display calculation as it is performed? (y or n)');
  Ch := ReadKey
until Ch IN ['y', 'Y', 'n', 'N'];
if Ch IN ['y', 'Y']
then DC := true
else DC := false;
DelLine;
while (r < m0) and (r < n0) do
  begin
  zeros := true;
  for i := r + 1 to m0 do                 {looking for a non-zero}
    if A[i,r+1] <> 0 then zeros := false; {element in column r+1.}
  if zeros                                {If all elements in column}
  then                                    {r+1 are zero then swap columns}
    begin                                 {r+1 and n0.}
      if DC
      then
        begin
          WriteLn;
          WriteLn('Will next swap columns ',(r+1):0,' and ',n0:0,'.');
          WriteLn('Type <Return> to continue ... ');
          ReadLn;
          ClrScr
        end;
      for i := r + 1 to m0 do
        begin                           {Swap columns r+1 and n0}
          A[i,r+1] := A[i,n0];          {in both A and U}
          A[i, n0] := 0
        end;
      for i := 1 to n do
        begin
          t := U[i,r+1];
          U[i,r+1] := U[i,n0];
          U[i,n0] := t
        end;
      n0 := n0 - 1;
      if DC then Display(2);
    end
  else
    begin
      r := r + 1;
      RowFixed := false;
      ColFixed := false;
      repeat
        while not ColFixed do
          begin
            d1 := 0;
            d2 := 0;
            i1 := r;
            i2 := r;
            for i := r to m0 do
              begin
                if Abs(A[i,r]) >= d1
                then
                  begin
                    d2 := d1;
                    d1 := Abs(A[i,r]);
                    i2 := i1;
                    i1 := i;
                  end
                else
                  if Abs(A[i,r]) > d2
                  then
                    begin
                      d2 := Abs(A[i,r]);
                      i2 := i
                    end;
              end;
            if d2 > 0
            then
              begin
                if i1 = r
                then RowFixed := False;
                w := A[i1,r]/A[i2,r];
                if DC
                then
                  begin
                    WriteLn;
                    Write('Will next add ',-w:0:0,' times row ',i2:0);
                    WriteLn(' to row ',i1:0,'.');
                    WriteLn('Type <Return> to continue ... ');
                    ReadLn;
                    ClrScr
                  end;
                for j := r to n0 do
                  begin
                    A[i1,j] := A[i1,j] - w*A[i2,j];
                    Test(A[i1,j])
                  end;
                b[i1] := b[i1] - w*b[i2];
                Test(b[i1]);
                if DC then Display(3)
              end
            else
              begin
                if i1 > r
                then
                  begin
                    RowFixed := False;
                    if DC
                    then
                      begin
                        WriteLn;
                        WriteLn('Will next swap rows ',r:0,' and ',i1:0,'.');
                        WriteLn('Type <Return> to continue ... ');
                        ReadLn;
                        ClrScr
                      end;
                    for j := r to n0 do
                      begin
                        t := A[i1,j];
                        A[i1,j] := A[r,j];
                        A[r,j] := t
                      end;
                    t := b[i1];
                    b[i1] := b[r];
                    b[r] := t;
                    if DC then Display(4)
                  end;
                if A[r,r] < 0
                then
                  begin
                    if DC
                    then
                      begin
                        WriteLn;
                        WriteLn('Will next multiply row ',r:0,' by -1.');
                        WriteLn('Type <Return> to continue ... ');
                        ReadLn;
                        ClrScr
                      end;
                    for j := r to n0 do A[r,j] := -A[r,j];
                    b[r] := -b[r];
                    if DC then Display(5)
                  end;
                ColFixed := True
              end;
          end;
        while not RowFixed do
          begin
            d1 := 0;
            d2 := 0;
            j1 := r;
            j2 := r;    {now find two largest elements in row r}
            for j := r to n0 do
              begin
                if Abs(A[r,j]) >= d1
                then
                  begin
                    d2 := d1;
                    d1 := Abs(A[r,j]);
                    j2 := j1;
                    j1 := j
                  end
                else
                  if Abs(A[r,j]) > d2
                  then
                    begin
                      d2 := Abs(A[r,j]);
                      j2 := j
                    end;
              end;
            if d2 > 0
            then
              begin
                if j1 = r
                then ColFixed := False;
                w := A[r,j1]/A[r,j2];
                if DC
                then
                  begin
                    WriteLn;
                    Write('Will next add ',-w:0:0, ' times column ',j2:0);
                    WriteLn(' to column ',j1:0,'.');
                    WriteLn('Type <Return> to continue ... ');
                    ReadLn;
                    ClrScr
                  end;
                for i := r to m0 do
                  begin
                    A[i,j1] := A[i,j1] - w*A[i,j2];
                    Test(A[i,j1])
                  end;
                for i := 1 to n do
                  begin
                    U[i,j1] := U[i,j1] - w*U[i,j2];
                    Test(U[i,j1])
                  end;
                if DC then Display(6)
              end
            else
              begin
                if j1 > r
                then
                  begin
                    ColFixed := False;
                    if DC
                    then
                      begin
                        WriteLn;
                        Write('Will next swap columns ');
                        WriteLn(r:0, ' and ',j1:0,'.');
                        WriteLn('Type <Return> to continue ... ');
                        ReadLn;
                        ClrScr
                      end;
                    for i := r to m0 do
                      begin
                        t := A[i, j1];
                        A[i,j1] := A[i,r];
                        A[i,r] := t
                      end;
                    for i := 1 to n do
                      begin
                        t := U[i,j1];
                        U[i,j1] := U[i,r];
                        U[i,r] := t
                      end;
                    if DC then Display(7)
                  end;
                if A[r,r] < 0
                then
                  begin
                    if DC
                    then
                      begin
                        WriteLn;
                        WriteLn('Will next multiply column ',r:0,' by -1.');
                        WriteLn('Type <Return> to continue ... ');
                        ReadLn;
                        ClrScr
                      end;
                    for i := r to n0 do A[i,r] := -A[i,r];
                    for i := 1 to n do U[i,r] := -U[i,r];
                    if DC then Display(8)
                  end;
                  RowFixed := True
              end;
          end;
      until RowFixed and ColFixed;
    end;
    end;
ClrScr;
if DC
then
  begin
    ClrScr;
    WriteLn('The simplified array is now:');
    WriteLn;
    Display(9);
    WriteLn;
    Write('Type <Return> to continue ... ');
    ReadLn;
  end;
GoToXY(1, WhereY - 1);
ClrEoL;
for i := r+1 to m do
  if b[i] <> 0
  then
    begin
      Write('The given system has no solution in ');
      WriteLn('real variables, and hence none in integers.');
      Halt
    end;
for i := 1 to r do
  begin
    t := b[i]/A[i,i];
    if t*A[i,i] <> b[i]
    then
      begin
        Write('The given system has no solution as a congruence (mod ');
        WriteLn(A[i,i]:0:0, '),');
        WriteLn('and hence no solution in integers.');
        Halt
      end
    else  q[i] := t
  end;
for i := 1 to n do
  begin
    c[i] := 0;
    for j := 1 to r do c[i] := c[i] + U[i,j]*q[j]
  end;
if r = n
then
  begin
    Write('The given system has exactly one ');
    WriteLn('integral solution, namely ');
    for i := 1 to n do WriteLn('x',i:0,' = ',c[i]:0:0);
  end;
if n = r + 1
then
  begin
    Write('The given system has infinitely many ');
    WriteLn('integral solutions, namely ');
    WriteLn;
    for i := 1 to n do WriteLn('x', i:0,' = ',c[i]:0:0,' + ',U[i,n]:0:0,'k');
    WriteLn;
    WriteLn('where k runs over all integral values.');
  end;
if n > r + 1
then
  begin
    Write('The given system has infinitely many ');
    WriteLn('integral solutions, namely ');
    WriteLn;
    for i := 1 to n do
      begin
        Write('x',i:0,' = ', c[i]:0:0);
        for j := 1 to n - r do Write(' + ',U[i,r+j]:0:0,'k(',j:0,')');
        WriteLn;
        WriteLn;
      end;
    Write('where k(1), ..., k(',(n-r):0,') independently take ');
    WriteLn('integral values.')
  end;
ReadLn;
TextMode(OriginalMode)
end.