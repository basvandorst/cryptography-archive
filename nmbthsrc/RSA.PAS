program RSA;            {environment for displaying the RSA system
                         of public-key encription}
{$N+,E+}
uses CRT, nothy, Printer;

const
maxhist = 10;

var
m,                      {modulus to which calculations are performed}
k1, k2,                 {exponents for coding and decoding}
p,                      {a prime factor of m}
phi,                    {phi(m)}
a,                      {a residue class (mod m)}
BlockSize,              {10^k -- limits the minimum size of m}
MaxRes,                 {maximum residue in array -- limits the size of m}
g                       {a gcd}
             : comp;
t, x                    {temporary variables used for screening input
                         and testing for divisibility}
             : extended;
text1, text2            {the message as ASCII text}
             : string[80];
residue      : array[1..240] of comp;
primes                  {the prime factors of m}
             : array[1..15] of comp;
multip                  {multiplicities of the primes dividing m}
             : array[1..15] of integer;
mhist, khist : array[1..maxhist] of comp;
                        {History of encrypting parameters}
Error1,                 {Error detected in putative prime factorization?}
Error2,                 {Error in entering í(m)}
Error3,                 {(k, í(m)) > 1}
TextIn,                 {Is there a message under consideration?}
BlockSizeSet,           {Has the value of k been fixed?}
ResiduesIn,             {Do we have an array of Residues?}
PhiKnown,               {do we know phi(m)}
ModSet,                 {has the modulus been set?}
FuncKey,                {Was a function key pressed}
InputOk,                {Is the input ok?}
ExpSet                  {Has the exponent k1 been set?}
             : boolean;
OriginalMode : word;
Ch,                     {input for keyboard}
Ch1, Ch2
             : char;
asciistring,            {the message written as an ascii string}
St, St1, St2,
St3          : string[240];
e,                      {number of encryptions performed since
                         message was read in}
h, i, j,                {indices}
k,                      {number of digits in a block}
l,                      {length of a string}
ind,                    {amount to indent by}
bl,                     {block length -- number of characters per block}
n,                      {number of residues}
maxn,                   {maximum allowable number of blocks}
x0, y0,                 {cursor coordinates}
omega,                  {number of distinct prime factors of m}
code
             : integer;
v            : longint;

function GetInput(XCoord, YCoord : integer; prompt, comm : string;
                                         min, max : comp) : comp;

var
x                                    {raw input}
         : extended;
x0                                   {x coordinate of cursor when ready
                                      to read input}
         : integer;
InputOK  : Boolean;

begin                                {body of function GetInput}
  GoToXY(XCoord, YCoord);
  ClrEoL;
  Write(prompt);
  x0 := WhereX;
  if comm <> ''
  then
    begin
      WriteLn;
      ClrEoL;
      Write(comm);
      GoToXY(x0, WhereY - 1)
    end;
  InputOK := False;
  repeat
    ReadLn(x);
    if (frac(x) = 0) and (x <= max) and (x >= min)
    then InputOK := True
    else
      begin
        GoToXY(x0, WhereY - 1);
        ClrEoL
      end;
  until InputOK;
  ClrEoL;
  GetInput := x
end;

begin                        {main body}
OriginalMode := LastMode;
Error1 := False;
Error2 := False;
Error3 := False;
TextIn := False;
PhiKnown := False;
ModSet := False;
ExpSet := False;
ResiduesIn := False;
BlockSizeSet := False;
Ch := 'A';
e := 0;
Repeat
  if UpCase(Ch) = 'B'                            {Set the Block Size}
  then
    begin
      TextColor(14);
      TextBackground(1);
      ClrScr;
      St1 := 'Enter the number of digits in each block, k = ';
      St2 := '                                          1 ó k ó 17';
      k := round(GetInput(5, 20, St1, St2, 1, 17));
      BlockSize := 1;
      for i := 1 to k do BlockSize := BlockSize*10;
      BlockSizeSet := True;
      ModSet := False;
      ExpSet := False;
      PhiKnown := False;
      ResiduesIn := False
    end;
  if UpCase(Ch) = 'E'                            {Encode}
  then
    begin
      Write('                 Computing new residue classes ... ');
      ClrEoL;
      GoToXY(1, 25);                                    {hide the cursor}
      TextColor(7);
      Write(' ');
      GoToXY(1, 25);
      for i := 1 to n do residue[i] := power(residue[i], k1, m);
      e := e + 1;
      if e <= maxhist
      then
        begin
          mhist[e] := m;
          khist[e] := k1
        end;
    end;
  if UpCase(Ch) = 'P'        {Print text, residues, and encrypting history}
  then
    begin
      WriteLn(lst);
      WriteLn(lst, '                                      RSA');
      WriteLn(lst);
      if TextIn
      then
        begin
          WriteLn(lst, '                                     TEXT');
          WriteLn(lst);
          WriteLn(lst, text1);
          WriteLn(lst);
          WriteLn(lst)
        end;
      if ResiduesIn
      then
        begin
          WriteLn(lst, '                                    RESIDUES');
          WriteLn(lst);
          if ModSet
          then
            begin
              str(m:1:0, St);
              l := length(St);
              if l < k then l := k
            end
          else l := k;
          for i := 1 to n do
            begin
              if l + WhereX > 75
              then WriteLn(lst);
              Write(lst, residue[i]:(l+2):0)
            end;
        end;
      WriteLn(lst);
      WriteLn(lst);
      if e > 0
      then
        begin
          WriteLn(lst, '                              ENCRYPTING HISTORY');
          WriteLn(lst);
          WriteLn(lst, '                               Block size = ', k:1);
          WriteLn(lst);
          Write(lst, '                             m');
          WriteLn(lst, '                   k');
          j := e;
          if j > maxhist
          then j := maxhist;
          for i := 1 to j do  WriteLn(lst, mhist[i]:35:0, khist[i]:20:0)
        end;
      WriteLn(lst);
    end;
  if UpCase(Ch) = 'R'        {Enter message as a sequence of residue
                             classes (mod m)}
  then
    begin
      TextColor(14);
      TextBackground(1);
      ClrScr;
      maxn := 240 div k;
      n := 0;
      a := 1;
      x0 := 1;
      y0 := 10;
      GoToXY(5, 18);
      if ModSet
      then
        begin
          WriteLn('Enter message as a sequence of at most ', maxn:1);
          Write('residue classes (mod ', m:1:0, ').');
          while (a >= 0) and (n < maxn) do
            begin
              str((n+1):1, St1);
              St1 := 'Residue #' + St1 + ': ';
              str(m:1:0, St2);
              St2 := '       0 ó residue < ' + St2
              + '   (Enter -1 to terminate.)';
              a := GetInput(5, 20, St1, St2, -1, m - 1);
              if a >= 0
              then
                begin
                  n := n + 1;
                  residue[n] := a;
                  GoToXY(x0, y0);
                  if k + x0 > 75
                  then WriteLn;
                  Write(a:(k+2):0);
                  x0 := WhereX;
                  y0 := WhereY
                end;
          end;
        end
      else
        begin
          WriteLn('Enter message as a sequence of at most ', maxn:1);
          Write(' integers r, each one in the interval 0 ó r ó ');
          Write((BlockSize - 1):1:0, '.');
          while (a >= 0) and (n < maxn) do
            begin
              str((n+1):1, St1);
              St1 := 'r(' + St1 + ') = ';
              str((BlockSize - 1):1:0, St2);
              St2 := '       0 ó r(i) ó ' + St2
              + '   (Enter -1 to terminate.)';
              a := GetInput(5, 20, St1, St2, -1, BlockSize - 1);
              if a >= 0
              then
                begin
                  n := n + 1;
                  residue[n] := a;
                  GoToXY(x0, y0);
                  if k + x0 > 75
                  then WriteLn;
                  Write(a:(k+2):0);
                  x0 := WhereX;
                  y0 := WhereY
                end;
          end;
        end;
      if n > 0
      then ResiduesIn := True
      else
        begin
          ResiduesIn := False;
          TextIn := False
        end;
      e := 0;
    end;
  If UpCase(Ch) = 'T'        {Enter the message as text}
  then
    begin
      ClrEoL;
      GoToXY(1, 24);
      ClrEoL;
      WriteLn('Enter message in text form, not exceeding one line:');
      ReadLn(text1);
      if text1 <> ''
      then TextIn := True
      else
        begin
          TextIn := False;
          ResiduesIn := False
        end;
      e := 0;
      asciistring := '';
      for i := 1 to length(text1) do        {create the asciistring}
        begin
          j := ord(text1[i]);
          str(j:1, St);
          while length(St) < 3 do St := '0' + St;
          asciistring := asciistring + St
        end;
    end;          {of message, input as text}
  if UpCase(Ch) = 'V'
  then
    begin
      Ch1 := 'A';
      repeat
        if UpCase(Ch1) = 'E'        {Enter Exponent k1}
        then
          begin
            str(m:1:0, St1);
            St1 := '           0 < k < ' + St1;
            ClrEoL;
            g := 1;
            repeat
              k1 := GetInput(1, 24, 'Enter exponent k = ', St1, 1, m - 1);
              if PhiKnown
              then g := gcd(k1, phi);
              if g > 1
              then
                begin
                  GoToXY(1, 23);
                  ClrEoL;
                  Write('     Must have (k, ', phi:1:0, ') = 1.  Try again.')
                end;
            until g = 1;
            ExpSet := True
          end;
        if UpCase(Ch1) = 'F'        {Enter prime factors of m}
        then
          begin
            a := m;
            omega := 0;
            TextColor(14);
            TextBackground(1);
            ClrScr;
            GoToXY(5, 10);
            Write(m:1:0, ' = ');
            x0 := WhereX;
            y0 := WhereY;
            GoToXY(5, 19);
            Write('Enter the distinct prime factors of ', m:1:0);
            repeat
              str((omega + 1):1, St1);
              St1 := 'Prime factor #' + St1 + ': ';
              p := GetInput(5, 21, St1, '', 2, a);
              t := a/p;
              if frac(t) = 0
              then Error1 := False
              else Error1 := True;
              if Error1
              then
                begin
                  str(p:1:0, St2);
                  str(a:1:0, St3);
                  St2 := St2 + ' does not divide ' + St3
                  + '.  This is your last chance.';
                  p := GetInput(5, 21, St1, St2, 2, a);
                  t := a/p;
                  if frac(t) = 0
                  then Error1 := False;
                end;
              if not Error1
              then
                begin
                  omega := omega + 1;
                  primes[omega] := p;
                  multip[omega] := 0;
                  while frac(t) = 0 do
                    begin
                      multip[omega] := multip[omega] + 1;
                      a := t;
                      t := a/p
                    end;
                  GoToXY(x0, y0);
                  Write(primes[omega]:1:0);
                  if multip[omega] > 1
                  then
                    begin
                      GoToXY(WhereX, WhereY - 1);
                      Write(multip[omega]:1);
                      GoToXY(WhereX, WhereY + 1)
                    end;
                  if a > 1 then Write(chr(249));
                  x0 := WhereX;
                  y0 := WhereY
                end;
            until (a = 1) or (Error1);
            if not Error1
            then
              begin
                phi := 1;
                for i := 1 to omega do
                  begin
                    phi := phi*(primes[i] - 1);
                    for h := 1 to multip[i] - 1 do phi := phi*primes[i]
                  end;
                GoToXY(5, 15);
                Write('í(', m:1:0, ') = ', phi:1:0);
                GoToXY(1, 19);
                ClrEoL;
                GoToXY(1, 21);
                ClrEoL;
                GoToXY(20, 17);
                Write('Press any key to continue ... ');
                Ch2 := ReadKey;
                if Ch2 = #0
                then Ch2 := ReadKey;
                PhiKnown := True
              end;
          end;                 {í(m) has been determined -- barring error}
        If UpCase(Ch1) = 'M'        {Enter modulus}
        then
          begin
            if ResiduesIn
            then
              begin
                a := 0;
                for i := 1 to n do if residue[i] > a then a := residue[i];
                a := a + 1
              end
            else a := BlockSize;
            str(a:1:0, St);
            m := GetInput(1, 24, 'Enter modulus m = ',
                 '      ' + St + ' ó m < 1000000000000000000', a, 1E18 - 1);
            ModSet := True;
            ExpSet := False;
            PhiKnown := False
          end;
        If (UpCase(Ch1) = 'P') and (not PhiKnown)        {Enter í(m)}
        then
          begin
            TextColor(14);
            TextBackground(1);
            ClrScr;
            str(m:1:0, St3);
            St1 := 'Enter í(' + St3 + ') = ';
            phi := GetInput(5, 20, St1, '', 2, m - 1);
            a := 1;
            repeat a := a + 1
            until gcd(a, m) = 1;
            if power(a, phi, m) = 1
            then Error2 := False
            else Error2 := True;
            if Error2
            then
              begin
                str(phi:1:0, St2);
                St2 := St2 + ' is not í(' + St3
                + ').  This is your last chance.';
                phi := GetInput(5, 20, St1, St2, 2, m - 1);
                if power(a, phi, m) = 1
                then Error2 := False;
              end;
            if not Error2
            then PhiKnown := True;
          end;                     {í(m) has been entered}
        If (UpCase(Ch1) = 'S')
                                   {swap parameters, in order to decode}
        then
          begin
            a := k1;
            k1 := k2;
            k2 := a
          end;
        If (UpCase(Ch1) IN ['F', 'P', 'E']) and (PhiKnown) and (ExpSet)
        then
          begin
            lincon(k1, 1, phi, k2, a);
            if a = 0
                          {if there is no solution because (k1, í(m)) > 1}
            then
              begin
                Error3 := True;
                ExpSet := False
              end;
          end;
        TextColor(14);
        TextBackground(1);
        ClrScr;
        WriteLn('                                 VARIABLES');
        WriteLn;
        WriteLn('              Block size = ', k:1);
        WriteLn;
        Write('                       m = ');
        if ModSet
        then Write(m:1:0);
        WriteLn;
        WriteLn;
        Write('                       k = ');
        if ExpSet
        then Write(k1:1:0);
        WriteLn;
        WriteLn;
        Write('                      k'' = ');
        if (ExpSet) and (PhiKnown)
        then Write(k2:1:0);
        WriteLn;
        WriteLn;
        if e > 0
        then
          begin
            WriteLn('                              ENCRYPTING HISTORY');
            Write('                             m');
            WriteLn('                   k');
            j := e;
            if j > maxhist
            then j := maxhist;
            for i := 1 to j do  WriteLn(mhist[i]:35:0, khist[i]:20:0)
          end;
        if Error1
        then
          begin
            GoToXY(1, 24);
            TextColor(0);
            TextBackground(7);
            ClrEoL;
            Write('         Attempt to list the prime factors of ');
            Write(m:1:0, ' was unsuccessful.')
          end;
        if Error2
        then
          begin
            GoToXY(1, 24);
            TextColor(0);
            TextBackground(7);
            ClrEoL;
            Write('         Attempt to enter í(', m:1:0);
            Write(') was unsuccessful.')
          end;
        If Error3
        then
          begin
            GoToXY(1, 24);
            TextColor(0);
            TextBackground(7);
            ClrEoL;
            Write('        k = ', k1:1:0);
            Write(' is inadmissible because (k, í(m)) > 1.')
          end;
        if UpCase(Ch1) = 'S'
        then
          begin
            GoToXY(1, 24);
            TextColor(0);
            TextBackground(7);
            ClrEoL;
            Write('                          Exponents have been swapped.')
          end;
        GoToXY(1, 25);
        TextColor(4);
        TextBackground(7);
        ClrEoL;
        if ModSet
        then
          begin
            Write(' E');
            TextColor(0);
            Write('xponent           ');
            TextColor(4)
          end;
        if (ModSet) and (not PhiKnown)
        then
          begin
            Write(' F');
            TextColor(0);
            Write('actors           ');
            TextColor(4)
          end;
        Write(' M');
        TextColor(0);
        Write('odulus           ');
        TextColor(4);
        if (ModSet) and (not PhiKnown)
        then
          begin
            Write(' P');
            TextColor(0);
            Write('hi           ');
            TextColor(4)
          end;
        if (ModSet) and (ExpSet) and (PhiKnown)
        then
          begin
            Write(' S');
            TextColor(0);
            Write('wap           ');
            TextColor(4)
          end;
        Write(' Esc');
        GoToXY(1, 25);                                    {hide the cursor}
        TextColor(7);
        Write(' ');
        GoToXY(1, 25);
        TextColor(0);
        repeat
          Ch1 := ReadKey;
          if Ch1 <> #0
          then FuncKey := false
          else
            begin
              FuncKey := true;
              Ch1 := ReadKey
            end;
          InputOk := false;
          if (not FuncKey) and (Ch1 IN [#27, 'm', 'M'])
          then InputOk := true;
          if (not FuncKey) and (ModSet) and (Ch1 IN ['e', 'E'])
          then InputOk := true;
          if (not FuncKey) and (ModSet) and (not PhiKnown)
             and (Ch1 IN ['f', 'F', 'p', 'P'])
          then InputOk := True;
          if (not FuncKey) and (ModSet) and (ExpSet) and (PhiKnown)
             and (Ch1 IN ['s', 'S'])
          then InputOk := True;
        until InputOk;
        Error1 := False;
        Error2 := False;
        Error3 := False;
      until (not FuncKey) and (Ch1 = #27);
    end;                        {of variables screen}
  if ((UpCase(Ch) = 'T') and (BlockSizeSet))
     or ((UpCase(Ch) = 'B') and (TextIn))
  then                          {construct array of residues from text}
    begin
      St1 := asciistring;
      n := length(St1) div k;
      if length(St1) mod k > 0 then n := n + 1;
      i := n;
      while St1 <> '' do
        begin
          if length(St1) > k
          then
            begin
              St2 := copy(St1, length(St1) - k + 1, k);
              delete(St1, length(St1) - k + 1, k)
            end
          else
            begin
              St2 := St1;
              St1 := ''
            end;
          val(St2, a, code);
          residue[i] := a;
          i := i - 1
        end;
      ResiduesIn := True
    end;
  if UpCase(Ch) IN ['E', 'R']   {recover text from the sequence
                                 of residue classes}
  then
    begin
      St := '';
      for i := 1 to n do
        begin
          a := residue[i];
          str(a:1:0, St1);
          while length(St1) < k do St1 := '0' + St1;
          St := St + St1
        end;
      asciistring := St;
      TextIn := True;
      text1 := '';
      while (TextIn) and (St <> '') do
        if length(St) >= 3
        then
          begin
            St1 := copy(St, length(St) - 2, 3);
            delete(St, length(St) - 2, 3);
            val(St1, j, code);
            if (j > 31) and (j < 255)
            then text1 := chr(j) + text1
            else
              if j <> 0
              then TextIn := False
          end
        else
          if (St = '0') or (St = '00')
          then St := ''
          else TextIn := False
    end;                        {text is recovered, if possible}
  TextColor(14);                      {Display data, list commands}
  TextBackGround(1);
  ClrScr;
  WriteLn('                                     RSA');
  St1 := '';
  if BlockSizeSet
  then
    begin
      str(k:1, St1);
      St1 := 'Block size = ' + St1
    end;
  if ModSet
  then
    begin
      Str(m:1:0, St2);
      St1 := St1 + '    m = ' + St2
    end;
  if ExpSet
  then
    begin
      Str(k1:1:0, St2);
      St1 := St1 + '     k = ' + St2
    end;
  if (ExpSet) and (PhiKnown)
  then
    begin
      Str(k2:1:0, St2);
      St1 := St1 + '     k'' = ' + St2
    end;
  ind := (80 - length(St1)) div 2;
  GoToXY(ind, WhereY);
  Write(St1);
  WriteLn;
  WriteLn;
  If TextIn
  then
    begin
      WriteLn('                           MESSAGE IN TEXT FORM:');
      WriteLn(text1);
      WriteLn;
      WriteLn('                        MESSAGE AS A STRING OF ASCII CODES');
      WriteLn(asciistring);
      WriteLn;
    end;
  If ResiduesIn
  then
    begin
      WriteLn('                      MESSAGE AS A SEQUENCE OF RESIDUES');
      if ModSet
      then
        begin
          str(m:1:0, St);
          l := length(St);
          if l < k then l := k
        end
      else l := k;
      for i := 1 to n do
        begin
          if l + WhereX > 76
          then WriteLn;
          Write(residue[i]:(l+2):0)
        end;
      WriteLn;
      WriteLn;
    end;
    if (e > 0) and (WhereY < 23)
    then
      begin
        WriteLn('                             ENCRYPTING HISTORY');
        Write('(m, k) = ');
        j := e;
        if j > maxhist
        then j := maxhist;
        i := 1;
        while (i <= j) and (WhereY < 25) do
          begin
            str(mhist[i]:1:0, St);
            l := length(St);
            str(khist[i]:1:0, St);
            l := l + length(St);
            if WhereX + l > 74
            then
              begin
                if WhereY = 24
                then Write('...');
                WriteLn
              end;
            Write('(', mhist[i]:1:0, ', ', khist[i]:1:0, ')');
            if i < j then Write(',  ');
            i := i + 1
          end;
      end;
  TextBackground(7);
  GoToXY(1, 25);
  ClrEoL;
  TextColor(4);
  Write(' B');
  TextColor(0);
  Write('lock size    ');
  TextColor(4);
  if (ModSet) and (ExpSet) and (ResiduesIn)
  then
    begin
      Write(' E');
      TextColor(0);
      Write('ncode    ');
      TextColor(4)
    end;
  If (ResiduesIn)
  then
    begin
      Write(' P');
      TextColor(0);
      Write('rint    ');
      TextColor(4)
    end;
  if BlockSizeSet
  then
    begin
      Write(' R');
      TextColor(0);
      Write('esidues    ');
      TextColor(4)
    end;
  Write(' T');
  TextColor(0);
  Write('ext    ');
  TextColor(4);
  if BlockSizeSet
  then
    begin
      Write(' V');
      TextColor(0);
      Write('ariables    ');
      TextColor(4)
    end;
  Write(' Esc');
  GoToXY(1, 25);                                    {hide the cursor}
  TextColor(7);
  Write(' ');
  GoToXY(1, 25);
  TextColor(0);
  repeat
    Ch := ReadKey;
    if Ch <> #0
    then FuncKey := false
    else
      begin
        FuncKey := true;
        Ch := ReadKey
      end;
    InputOk := false;
    if (not FuncKey) and (Ch IN [#27, 'b', 'B', 't', 'T'])
    then InputOk := true;
    if (not FuncKey) and (BlockSizeSet) and (Ch IN ['v', 'V'])
    then InputOk := true;
    if (not FuncKey) and (ModSet) and (Ch IN ['r', 'R'])
    then InputOk := true;
    if (not FuncKey) and (ModSet) and (ExpSet) and (ResiduesIn)
       and (Ch IN ['e', 'E'])
    then InputOk := True;
    if (not FuncKey) and (ResiduesIn)
       and (Ch IN ['p', 'P'])
    then InputOk := True
  until InputOk;
until (not funckey) and (Ch = #27);
TextMode(OriginalMode)
end.