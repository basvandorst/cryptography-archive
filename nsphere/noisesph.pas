{

  NOISESPH.PAS, A Noise Sphere plotter written in Turbo Pascal (23-Mar-96)
  Robert Rothenburg Walking-Owl <WlkngOwl@unix.asb.com>
  No copyright is claimed.  No guarantees made.

  Usage: noisesph file

  This program reads a file of random or pseudo-random data and plots
  a noise sphere of the data. Poor RNGs or sampling methods will show
  clear patterns (definite splotches or spirals).

  The theory behind this is to get a set of 3D polar coordinates from
  the RNG and plot them.  An array is kept of the values, which is
  rotated each time a new byte is read (see the code in the main
  procedure).

  Rather than plot one sphere which can be rotated around any axis,
  it was easier to plot the sphere from three different angles.

  This program is based on a description from the article below.  It
  was proposed as a means of testing pseudo-RNGs:

  Pickover, Clifford A. 1995. "Random number generators: pretty good
    ones are easy to find."  The Visual Computer (1005) 11:369-377.

}

{$N+}{$E+}
{$define BIOS}   { Define to use BIOS calls and to access 256 color VGA }
{$define USEASM} { Use assembler device routines (won't work in OS/2) }
program NoiseSphere;
  uses {$ifndef BIOS}Graph,{$endif} Crt, Strings;


{$ifndef BIOS}
const
  BGIPath = ''; { where those silly Borland *.BGI drivers are }
{$endif}

var
{$ifndef BIOS}
  GraphMode,
  GraphDriver: Integer;
{$endif}
  MidY, MidA, MidB, MidC, Scale: Word;

{ Initialize the graphics screen }
function InitScreen: Integer;
{$ifdef BIOS}
const
{ Adjust to the preferred mode... }
  Mode    = $5F; { $5F;  $13;  }
  GetMaxX = 640; { 640;  320;  }
  GetMaxY = 480; { 480;  200;  }
{$endif}
begin
{$ifdef BIOS}
  asm
    mov  ax, 0000h + Mode
    int  10h
  end;
{$else}
{$ifdef SVGA}
(*
  GraphDriver := EGA;  { Use EGAVGA.BGI by default... }
  GraphMode := VGAHi;
*)
  GraphDriver := InstallUserDriver('SVGA256', nil);
  GraphMode := 2;
{$else}
  DetectGraph(GraphDriver, GraphMode);
{$endif}
  InitGraph(GraphDriver,GraphMode,BGIPath);
  InitScreen := GraphResult;
{$endif}
  Scale := (GetMaxX div 6);
  MidA := Scale; MidB := Scale*3; MidC := Scale*5;
  MidY := (GetMaxY div 2);
  if MidY<Scale then Scale := MidY;
end;



var f: File;

{$ifdef USEASM}
  { Note: if we want to read from a device such as /dev/random we'd
    have to use a customized routine, since Pascal doesn't like opening
    character devices as binary files. }
  function OpenFile(var f: File; Filename: PChar): Word;
    assembler;
       asm
            push DS
            cld
            lds  SI, Filename
            mov  DX, SI
    @parseloop:                 { convert '/' to '\' in pathname }
            lodsb
            test al, al
            jz   @doneparse
            cmp  al, '/'
            jne  @parseloop
            mov  BYTE PTR DS:[SI-1], '\'
            jmp  @parseloop
    @doneparse:
            mov  AX, 3D80h       { open the file }
            Int  21h
            jnc  @ok             { does the file exist? }
       @err:
            xor  AX,AX
            jmp  @quit
       @ok:
            les  BX, f
            mov  ES:[BX], AX     { save file handle }

            mov  BX, AX          { put file into binary mode }
            mov  AX, 4400h       { get device info }
            Int  21h
            jc   @err
            test DX, 0080h       { is it a character device or a file? }
            jz  @skip            { is a file, we don't need to change mode }
            and  DX, 00FFh
            or   DX, 0020h       { "raw" mode }
            mov  AX, 4401h       { set device info }
            Int  21h
            jc   @err
       @skip:
            mov  AX, BX
       @quit:
            pop  DS
       end;

  procedure CloseFile(var f: File);
    assembler;
  asm
       les  BX, f
       mov  BX, ES:[BX]
       mov  AH, 3Eh
       Int  21h
  end;
{$endif}

  function GetRand(var f: File; var block; req: Word): Word;
{$ifndef USEASM}
  var
    i: Word;
  begin
    BlockRead(f, block, req, i);
    GetRand := i;
  end;
{$else}
    assembler;
  asm
       push  DS
       lds   BX, f
       mov   BX, DS:[BX]
       mov   CX, req
       lds   DX, block
       mov   AH, 3Fh
       Int   21h
       jnc   @ok
       xor   AX, AX
  @ok:
       pop   DS
  end;
{$endif}

  function GetRandByte: Byte;
  var b: Byte;
  begin
    GetRand(f, b, 1);
    GetRandByte := b;
  end;

type
  Polar = record
    r, theta, phi: Real;
  end;

  Cartesian = record
    x,y,z: Real;
    Color: Word;
  end;

function ScaleColor(Coord: Real): Word;
begin
{$ifdef BIOS}
{ Uses a grayscale mapping, which helps with a nice 3-d effect }
   ScaleColor := Trunc(8*(1.0+Coord))+16; { Map [-1..1] }
(* ScaleColor := Trunc(16*Coord)+16; { Map [0..1] } *)
{$else}
  ScaleColor := LightGray;
{$endif}
end;

procedure PolarToCartesian(var P: Polar; var C: Cartesian);
begin
{ No rotation was added. Instead we plot from three angles.... }
  C.x := P.r * Sin(P.phi) * Cos(P.theta);
  C.y := P.r * Sin(P.phi) * Sin(P.theta);
  C.z := P.r * Cos(P.phi);
{ We can assign colors based on x,y,z,r,theta/pi or phi/(2*pi) }
  C.Color := ScaleColor(C.y);
end;

{$ifdef BIOS}
procedure PutPixel(x, y: Integer; Color: Byte);
  assembler;
asm
     mov  ah, 0Ch
     mov  al, Color
     mov  bh, 0
     mov  cx, x
     mov  dx, y
     int  10h
end;
{$endif}

procedure Plot(var C: Cartesian);
begin
  with C do begin
    PutPixel(MidA+(Round(Scale*y)), MidY-(Round(Scale*z)), Color);
    PutPixel(MidB+(Round(Scale*x)), MidY-(Round(Scale*y)), Color);
    PutPixel(MidC+(Round(Scale*z)), MidY-(Round(Scale*x)), Color);
  end;
end;


function ByteToReal(b: Byte): Real;
begin
{ note that there will be some gaps since we're only using the
  equivalent of an 8-bit decimal here }
  ByteToReal := b / 256;
end;

var
  n: LongInt;
  X: Array [ 0..2 ] of Real;
  P: Polar;
  C: Cartesian;
{$ifdef USEASM}
  Filename: array [0..255] of Char;
{$endif}

  procedure InitSphere;
  begin
    for n := 0 to 2 do X[n] := ByteToReal(GetRandByte);
    n := 0;
  end;

begin
  if ParamCount=0
    then WriteLn('Usage: noisesph file')
    else begin
{$ifdef USEASM}
      StrPCopy(Filename,ParamStr(1));
      OpenFile(f, Filename);
{$else}
      Assign(f,ParamStr(1)); Reset(f,1); { open file }
{$endif}
      InitScreen;                        { go to graphics screen }
      InitSphere;                        { set up variables }
{ ------------------------------------------------------------------------}
      repeat                             { This is the heart of the plot! }
        with P do begin                  { convert random samples to }
          r := Sqrt(X[(n+2) mod 3]);     { polar coordinates }
          theta := pi * X[(n+1) mod 3];
          phi := 2 * pi * X[n];
        end;
        PolarToCartesian(P,C); Plot(C);  { plot sphere }
        X[n] := ByteToReal(GetRandByte); { add new sample }
        n := (n + 1) mod 3;
      until KeyPressed {$ifndef USEASM}or Eof(f){$endif};
{ ------------------------------------------------------------------------}
      ReadKey; { if user hasn't pressed a key, wait until one is pressed }
{$ifdef USEASM}
      CloseFile(f);
{$else}
      Close(f); { close file and go back to test screen }
{$endif}
{$ifndef BIOS}
      RestoreCrtMode;
{$else}
  asm
      mov  ax, 3
      int  10h
  end;
{$endif}
  end;
end.
