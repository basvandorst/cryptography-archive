{$M 20000, 0, 650000}
PROGRAM dynamic(input,output);

USES pfloat,crt;

{--------------------------------------------------------------------}

FUNCTION HeapFunc(Size : Word) : integer; FAR;

BEGIN
  HeapFunc := 1;
  { If there is no memory available --> return value of new is NIL }
END;

{--------------------------------------------------------------------}

CONST maxdim = 2000;
TYPE  vector = ARRAY [0..maxdim-1] OF ^mfloat;
VAR  a : vector;     { this is an array of pointers to mfloat variables }
     i : integer;

BEGIN
  HeapError := @HeapFunc; { Installs user Heap error function. }
  clrscr;
  writeln;
  writeln;
  writeln('                       Dynamic data structures');
  writeln('                       =======================');
  writeln;
  writeln;
  writeln('The heap memory is free!');
  writeln('Number of free bytes: ', MemAvail);
  writeln;
  { initialize }
  FOR i := 0 TO maxdim - 1 DO
  BEGIN
    new(a[i]);
    { if there is no memory --> pointer = 0 }
    IF a[i] = NIL THEN
    BEGIN
      writeln('No memory available!');
      halt(1);
    END;
  END;
  writeln('Now the memory is allocated to the variables.');
  writeln('Number of free bytes: ', MemAvail);
  writeln('Please wait!');
  writeln;
  delay(2000);
  { calculate (it has no sence here) }
  FOR i := 0 TO maxdim - 1  DO
  BEGIN
    ldtomf(a[i]^,i);
    sqrtm(a[i]^);
  END;
  { output of the result }
  FOR i := 0 TO maxdim - 1 DO
    writeln('result ',i,' = ',mftostr(a[i]^,50,'.6F'));
  { gain back memory }
  FOR i := 0 TO maxdim - 1 DO
    IF a[i] <> NIL THEN dispose(a[i]);  { checks, if the pointer isn't zero }
  writeln;
  writeln;
  writeln('Now the heap memory is free again!');
  writeln('Number of free bytes: ', MemAvail);
END.