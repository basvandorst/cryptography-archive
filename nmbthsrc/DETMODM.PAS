{program DetModM;         function DetModM which calculates Det(A) (mod m)
                           equipped with convenient user interface}
{$N+,E+}
uses CRT;
{$I GetInput.i }

type
Matrix            =  array[1..9] of array[1..9] of comp;

var
d,
m,                              {the modulus}
t,                              {a temporary variable}
maxcoeff                        {maximum modulus of elements of A}
                  : comp;
f,                              {field width for displaying elements}
i, j, k, l,
n,                              {the dimension of the matrix}
r,                              {number of terms of the matrix to display}
ti,                             {temporary integral variable}
ind,                            {amount to indent by}
maxn                            {Maximum n encountered}
                  : integer;
A                 : Matrix;
s                 : string[30]; {for handling format of matrix elements}
OriginalMode      : word;
Ch, Ch1           : char;
nf,                             {n fixed?}
Af,                             {A fixed?}
mf                              {m fixed?}
                  : Boolean;

{$I det.i }

procedure display(A : matrix; n, r : integer);

begin
  ClrScr;
  k := 1;
  maxcoeff := 0;
  for ti := 0 to r - 1 do
    begin
      i := 1 + (ti div n);
      j := 1 + (ti mod n);
      t := Abs(A[i, j]);
      if t > maxcoeff then maxcoeff := t
    end;
  str(maxcoeff:1:0, s);
  f := 2 + length(s);
  if f*n > 72
  then
    begin
      f := trunc(73/n);
      ind := 0
    end
  else ind := trunc((72 - n*f)/2);
  for ti := 1 to ind do Write(' ');
  Write('    Ú');
  for j := 1 to f*n do Write(' ');
  WriteLn(' ¿');
  for i := 1 to n do
    begin
      for ti := 1 to ind do Write(' ');
      if k = n then Write('A = ³') else Write('    ³');
      for j := 1 to n do
        begin
          if (i-1)*n + j > r then exit;
          str(Abs(A[i, j]):1:0, s);
          if length(s) + 2 <= f
          then Write(A[i, j]:f:0)
          else
            begin
              str(A[i, j], s);
              s := copy(s, 1, f-4) + 'E' + copy(s, length(s), 1);
              Write(s:f)
            end
        end;
      WriteLn(' ³');
      for ti := 1 to ind do Write(' ');
      k := k + 1;
      if k = n
      then
        begin
          Write('A = ³');
          for j := 1 to f*n do Write(' ');
          WriteLn(' ³')
        end;
      if (k <> n) and (k < 2*n)
      then
        begin
          Write('    ³');
          for j := 1 to f*n do Write(' ');
          WriteLn(' ³')
        end;
      k := k + 1;
    end;
  Write('    À');
  for j := 1 to f*n do Write(' ');
  WriteLn(' Ù');
  WriteLn;
end;

begin {Main body}
OriginalMode := LastMode;
nf := false;
Af := false;
mf := false;
Maxn := 0;
TextColor(0);
TextBackground(7);
ClrScr;
GoToXY(25, 3);
Write('Calculate det(A) (mod m)');
repeat
  GoToXY(20, 7);
  Write('Choose one of the alternatives A-F :');
  GoToXY(25, 9);
  TextColor(4);
  Write('A    A');
  TextColor(0);
  Write('ssign dimension of matrix');
  GoToXY(25, 10);
  TextColor(4);
  Write('B    B');
  TextColor(0);
  Write('uild matrix');
  GoToXY(25, 11);
  TextColor(4);
  Write('C    C');
  TextColor(0);
  Write('hoose modulus');
  GoToXY(25, 12);
  TextColor(4);
  Write('D    D');
  TextColor(0);
  Write('etermine value of det(A) (mod m)');
  GoToXY(25, 13);
  TextColor(4);
  Write('E    E');
  TextColor(0);
  Write('xit');
  GoToXY(25, 14);
  TextColor(4);
  Write('F    F');
  TextColor(0);
  Write('orm altered matrix');
  GoToXY(20, 16);
  repeat
    Ch := ReadKey
  until UpCase(Ch) IN ['A'..'F'];
  Ch := UpCase(Ch);
  if Ch = 'A'                             {Assign dimension n}
  then
    begin
      n := 1;
      repeat
        ClrScr;
        if (n < 1) or (n > 9)
        then
          begin
            GoToXY(10, 5);
            Write('The dimension n of A must be an integer ');
            Write('in the range 1 ó n ó 9.')
          end;
        GoToXY(35, 10);
        Write('n = ');
        Read(n)
      until (n > 0) and (n < 10);
      nf := True;
      if n > Maxn
      then
        begin
          Maxn := n;
          Af := False
        end;
    end;
  if Ch = 'B'                             {Build matrix A}
  then
    begin
      if nf = False
      then
        begin
          ClrScr;
          GoToXY(20, 10);
          Write('The dimension must be assigned first.');
          Delay(2000)
        end
      else
        begin
          for i := 1 to n do
            for j := 1 to n do
              begin
                display(A, n, (i-1)*n + j - 1);
                GoToXY(5, 20);
                ClrEoL;
                Write('Enter a(', i:1, ', ', j:1, ') = ');
                A[i, j] := round(GetInput(WhereX, WhereY, '',
      'The elements of A must have absolute value less than 1,000,000,000.',
                -999999999, 999999999));
              end;
          display(A, n, n*n);
          Delay(2000);
          Af := True;
        end;
    end;
  if Ch = 'C'                             {Choose modulus m}
  then
    begin
      repeat
        ClrScr;
        if mf
        then
          begin
            GoToXY(20, 7);
            Write('Current modulus is m = ', m:1:0, '.');
            GoToXY(20, 12);
            Write('Choose a new modulus, m = ')
          end
        else
          begin
            GoToXY(20, 12);
            Write('Choose the modulus, m = ')
          end;
        ReadLn(m);
        if (m < 1) or (m >= 1E9)
        then
          begin
            GoToXY(10, 17);
            Write('The modulus m must be an integer ');
            Write('in the interval [1, 1E9).');
            mf := false;
            Delay(2000)
          end
      until (m > 0) and (m < 1E9);
      mf := True
    end;
  if Ch = 'D'                             {Determine determinant}
  then
    begin
      if not nf
      then
        begin
          Write('The dimension n has not been assigned.');
          Delay(2000)
        end;
      if (nf) and (not Af)
      then
        begin
          Write('The matrix A has not been built.');
          Delay(2000)
        end;
      if (nf) and (Af) and (not mf)
      then
        begin
          Write('The modulus m has not been chosen.');
          Delay(2000);
        end;
      if (nf) and (Af) and (mf)
      then
        begin
          display(A, n, n*n);
          d := DetModM(A, n, m);
          str(d:1:0, s);
          ti := 16 + length(s);
          str(m:1:0, s);
          ti := ti + length(s);
          ind := trunc((80 - ti)/2);
          for ti := 1 to ind do Write(' ');
          WriteLn('det(A) ð ', d:1:0, ' (mod ', m:1:0, ')');
          ReadLn
        end
    end;
  if Ch = 'F'
  then
    begin
      if not nf
      then
        begin
          Write('The dimension n has not been assigned.');
          Delay(2000)
        end;
      if (nf) and (not Af)
      then
        begin
          Write('The matrix A has not been built.');
          Delay(2000)
        end;
      if (nf) and (Af)
      then
        begin
          ClrScr;
          Display(A, n, n*n);
          GoToXY(5, 20);
          Write('Enter coordinates (i, j) of element to be altered, i = ');
          repeat
            Ch1 := ReadKey
          until Ch1 in ['1'..'9'];
          i := Ord(Ch1) - 48;
          Write(i:1, ' j = ');
          repeat
            Ch1 := ReadKey
          until Ch1 in ['1'..'9'];
          j := Ord(Ch1) - 48;
          Write(j:1);
          GoToXY(5, 21);
          Write('Enter a(', i:1, ', ', j:1, ') = ');
          A[i, j] := round(GetInput(WhereX, WhereY, '',
      'The elements of A must have absolute value less than 1,000,000,000.',
          -1E9 + 1, 1E9 - 1));
          Display(A, n, n*n);
          Delay(2000)
        end;
    end;
      ClrScr
until Ch = 'E';
TextMode(OriginalMode)
end.