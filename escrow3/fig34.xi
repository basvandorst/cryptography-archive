%!PS-Adobe-2.0 EPSF-1.2
%%DocumentFonts: Helvetica-Bold Helvetica Times-Roman
%%Pages: 1
%%BoundingBox: 123 127 479 677
%%EndComments

50 dict begin

/arrowHeight 8 def
/arrowWidth 4 def
/none null def
/numGraphicParameters 17 def
/stringLimit 65535 def

/Begin {
save
numGraphicParameters dict begin
} def

/End {
end
restore
} def

/SetB {
dup type /nulltype eq {
pop
false /brushRightArrow idef
false /brushLeftArrow idef
true /brushNone idef
} {
/brushDashOffset idef
/brushDashArray idef
0 ne /brushRightArrow idef
0 ne /brushLeftArrow idef
/brushWidth idef
false /brushNone idef
} ifelse
} def

/SetCFg {
/fgblue idef
/fggreen idef
/fgred idef
} def

/SetCBg {
/bgblue idef
/bggreen idef
/bgred idef
} def

/SetF {
/printSize idef
/printFont idef
} def

/SetP {
dup type /nulltype eq {
pop true /patternNone idef
} {
dup -1 eq {
/patternGrayLevel idef
/patternString idef
} {
/patternGrayLevel idef
} ifelse
false /patternNone idef
} ifelse
} def

/BSpl {
0 begin
storexyn
newpath
n 1 gt {
0 0 0 0 0 0 1 1 true subspline
n 2 gt {
0 0 0 0 1 1 2 2 false subspline
1 1 n 3 sub {
/i exch def
i 1 sub dup i dup i 1 add dup i 2 add dup false subspline
} for
n 3 sub dup n 2 sub dup n 1 sub dup 2 copy false subspline
} if
n 2 sub dup n 1 sub dup 2 copy 2 copy false subspline
patternNone not brushLeftArrow not brushRightArrow not and and { ifill } if
brushNone not { istroke } if
0 0 1 1 leftarrow
n 2 sub dup n 1 sub dup rightarrow
} if
end
} dup 0 4 dict put def

/Circ {
newpath
0 360 arc
patternNone not { ifill } if
brushNone not { istroke } if
} def

/CBSpl {
0 begin
dup 2 gt {
storexyn
newpath
n 1 sub dup 0 0 1 1 2 2 true subspline
1 1 n 3 sub {
/i exch def
i 1 sub dup i dup i 1 add dup i 2 add dup false subspline
} for
n 3 sub dup n 2 sub dup n 1 sub dup 0 0 false subspline
n 2 sub dup n 1 sub dup 0 0 1 1 false subspline
patternNone not { ifill } if
brushNone not { istroke } if
} {
Poly
} ifelse
end
} dup 0 4 dict put def

/Elli {
0 begin
newpath
4 2 roll
translate
scale
0 0 1 0 360 arc
patternNone not { ifill } if
brushNone not { istroke } if
end
} dup 0 1 dict put def

/Line {
0 begin
2 storexyn
newpath
x 0 get y 0 get moveto
x 1 get y 1 get lineto
brushNone not { istroke } if
0 0 1 1 leftarrow
0 0 1 1 rightarrow
end
} dup 0 4 dict put def

/MLine {
0 begin
storexyn
newpath
n 1 gt {
x 0 get y 0 get moveto
1 1 n 1 sub {
/i exch def
x i get y i get lineto
} for
patternNone not brushLeftArrow not brushRightArrow not and and { ifill } if
brushNone not { istroke } if
0 0 1 1 leftarrow
n 2 sub dup n 1 sub dup rightarrow
} if
end
} dup 0 4 dict put def

/Poly {
3 1 roll
newpath
moveto
-1 add
{ lineto } repeat
closepath
patternNone not { ifill } if
brushNone not { istroke } if
} def

/Rect {
0 begin
/t exch def
/r exch def
/b exch def
/l exch def
newpath
l b moveto
l t lineto
r t lineto
r b lineto
closepath
patternNone not { ifill } if
brushNone not { istroke } if
end
} dup 0 4 dict put def

/Text {
ishow
} def

/idef {
dup where { pop pop pop } { exch def } ifelse
} def

/ifill {
0 begin
gsave
patternGrayLevel -1 ne {
fgred bgred fgred sub patternGrayLevel mul add
fggreen bggreen fggreen sub patternGrayLevel mul add
fgblue bgblue fgblue sub patternGrayLevel mul add setrgbcolor
eofill
} {
eoclip
originalCTM setmatrix
pathbbox /t exch def /r exch def /b exch def /l exch def
/w r l sub ceiling cvi def
/h t b sub ceiling cvi def
/imageByteWidth w 8 div ceiling cvi def
/imageHeight h def
bgred bggreen bgblue setrgbcolor
eofill
fgred fggreen fgblue setrgbcolor
w 0 gt h 0 gt and {
l b translate w h scale
w h true [w 0 0 h neg 0 h] { patternproc } imagemask
} if
} ifelse
grestore
end
} dup 0 8 dict put def

/istroke {
gsave
brushDashOffset -1 eq {
[] 0 setdash
1 setgray
} {
brushDashArray brushDashOffset setdash
fgred fggreen fgblue setrgbcolor
} ifelse
brushWidth setlinewidth
originalCTM setmatrix
stroke
grestore
} def

/ishow {
0 begin
gsave
fgred fggreen fgblue setrgbcolor
/fontDict printFont findfont printSize scalefont dup setfont def
/descender fontDict begin 0 [FontBBox] 1 get FontMatrix end
transform exch pop def
/vertoffset 0 descender sub printSize sub printFont /Courier ne
printFont /Courier-Bold ne and { 1 add } if def {
0 vertoffset moveto show
/vertoffset vertoffset printSize sub def
} forall
grestore
end
} dup 0 3 dict put def

/patternproc {
0 begin
/patternByteLength patternString length def
/patternHeight patternByteLength 8 mul sqrt cvi def
/patternWidth patternHeight def
/patternByteWidth patternWidth 8 idiv def
/imageByteMaxLength imageByteWidth imageHeight mul
stringLimit patternByteWidth sub min def
/imageMaxHeight imageByteMaxLength imageByteWidth idiv patternHeight idiv
patternHeight mul patternHeight max def
/imageHeight imageHeight imageMaxHeight sub store
/imageString imageByteWidth imageMaxHeight mul patternByteWidth add string def
0 1 imageMaxHeight 1 sub {
/y exch def
/patternRow y patternByteWidth mul patternByteLength mod def
/patternRowString patternString patternRow patternByteWidth getinterval def
/imageRow y imageByteWidth mul def
0 patternByteWidth imageByteWidth 1 sub {
/x exch def
imageString imageRow x add patternRowString putinterval
} for
} for
imageString
end
} dup 0 12 dict put def

/min {
dup 3 2 roll dup 4 3 roll lt { exch } if pop
} def

/max {
dup 3 2 roll dup 4 3 roll gt { exch } if pop
} def

/arrowhead {
0 begin
transform originalCTM itransform
/taily exch def
/tailx exch def
transform originalCTM itransform
/tipy exch def
/tipx exch def
/dy tipy taily sub def
/dx tipx tailx sub def
/angle dx 0 ne dy 0 ne or { dy dx atan } { 90 } ifelse def
gsave
originalCTM setmatrix
tipx tipy translate
angle rotate
newpath
0 0 moveto
arrowHeight neg arrowWidth 2 div lineto
arrowHeight neg arrowWidth 2 div neg lineto
closepath
patternNone not {
originalCTM setmatrix
/padtip arrowHeight 2 exp 0.25 arrowWidth 2 exp mul add sqrt brushWidth mul
arrowWidth div def
/padtail brushWidth 2 div def
tipx tipy translate
angle rotate
padtip 0 translate
arrowHeight padtip add padtail add arrowHeight div dup scale
arrowheadpath
ifill
} if
brushNone not {
originalCTM setmatrix
tipx tipy translate
angle rotate
arrowheadpath
istroke
} if
grestore
end
} dup 0 9 dict put def

/arrowheadpath {
newpath
0 0 moveto
arrowHeight neg arrowWidth 2 div lineto
arrowHeight neg arrowWidth 2 div neg lineto
closepath
} def

/leftarrow {
0 begin
y exch get /taily exch def
x exch get /tailx exch def
y exch get /tipy exch def
x exch get /tipx exch def
brushLeftArrow { tipx tipy tailx taily arrowhead } if
end
} dup 0 4 dict put def

/rightarrow {
0 begin
y exch get /tipy exch def
x exch get /tipx exch def
y exch get /taily exch def
x exch get /tailx exch def
brushRightArrow { tipx tipy tailx taily arrowhead } if
end
} dup 0 4 dict put def

/midpoint {
0 begin
/y1 exch def
/x1 exch def
/y0 exch def
/x0 exch def
x0 x1 add 2 div
y0 y1 add 2 div
end
} dup 0 4 dict put def

/thirdpoint {
0 begin
/y1 exch def
/x1 exch def
/y0 exch def
/x0 exch def
x0 2 mul x1 add 3 div
y0 2 mul y1 add 3 div
end
} dup 0 4 dict put def

/subspline {
0 begin
/movetoNeeded exch def
y exch get /y3 exch def
x exch get /x3 exch def
y exch get /y2 exch def
x exch get /x2 exch def
y exch get /y1 exch def
x exch get /x1 exch def
y exch get /y0 exch def
x exch get /x0 exch def
x1 y1 x2 y2 thirdpoint
/p1y exch def
/p1x exch def
x2 y2 x1 y1 thirdpoint
/p2y exch def
/p2x exch def
x1 y1 x0 y0 thirdpoint
p1x p1y midpoint
/p0y exch def
/p0x exch def
x2 y2 x3 y3 thirdpoint
p2x p2y midpoint
/p3y exch def
/p3x exch def
movetoNeeded { p0x p0y moveto } if
p1x p1y p2x p2y p3x p3y curveto
end
} dup 0 17 dict put def

/storexyn {
/n exch def
/y n array def
/x n array def
n 1 sub -1 0 {
/i exch def
y i 3 2 roll put
x i 3 2 roll put
} for
} def

%%EndProlog

%I Idraw 7 Grid 4 

%%Page: 1 1

Begin
%I b u
%I cfg u
%I cbg u
%I f u
%I p u
%I t
[ 0.8 0 0 0.8 0 0 ] concat
/originalCTM matrix currentmatrix def

Begin %I Poly
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.784534 0 0 1.44545 -90.9792 417.217 ] concat
%I 4
377 195
377 295
605 295
605 195
4 Poly
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.837325 0 0 0.837325 226.371 835.284 ] concat
%I
[
(         Key Escrow)
(Programming Facility)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.837325 0 0 0.837325 276.979 786.053 ] concat
%I
[
(KFpub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.837325 0 0 0.837325 240.974 801.383 ] concat
%I
[
(KEPFpub, KEPFpriv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.837325 0 0 0.837325 210.24 674.664 ] concat
%I
[
(KEPFpub,KEPFpriv = key escrow programming facility key pair)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.837325 0 0 0.837325 210.24 657.851 ] concat
%I
[
(KFpub = family key \(public component\))
] Text
End

Begin %I Poly
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.509947 0 0 0.677954 229.586 619.4 ] concat
%I 4
377 195
377 295
605 295
605 195
4 Poly
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.837325 0 0 0.837325 430.986 810.711 ] concat
%I
[
(Law Enforcement)
(     Decryptor)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.837325 0 0 0.837325 461.967 774.871 ] concat
%I
[
(KFpriv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.837325 0 0 0.837325 210.24 641.038 ] concat
%I
[
(KFpriv = family key \(private component\))
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-times-medium-r-*-120-*
/Times-Roman 12 SetF
%I t
[ 1 0 0 1 266 596 ] concat
%I
[
(Figure 3: Key escrow programming facility)
] Text
End

Begin %I Poly
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.836912 0 0 1.4786 -157.031 -24.331 ] concat
%I 4
377 195
377 295
605 295
605 195
4 Poly
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.856517 0 0 0.856517 190.51 403.319 ] concat
%I
[
(         Key Escrow)
(Programming Facility)
] Text
End

Begin %I Rect
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.391058 0 0 0.671362 264.643 132.292 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.856517 0 0 0.856517 422.587 323.303 ] concat
%I
[
(Key Escrow)
(   Agent 1)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 473.74 375.446 ] concat
%I
[
(UIP,KUpriv2)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 384.956 397.713 ] concat
%I
[
(UIP,KUpriv1)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 166.955 286.661 ] concat
%I
[
(KUpriv2 = KUpriv1 XOR KUpriv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 181.517 302.605 ] concat
%I
[
(KUpriv1 = random number)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 199.931 317.933 ] concat
%I
[
(UIP, KUpub, KUpriv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.856517 0 0 0.856517 517.047 323.303 ] concat
%I
[
(Key Escrow)
(   Agent 2)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 423.015 288.224 ] concat
%I
[
(UIP,KUpriv1)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 517.475 289.514 ] concat
%I
[
(UIP,KUpriv2)
] Text
End

Begin %I Rect
%I b 65535
2 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.391058 0 0 0.671362 359.103 132.292 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.691973 0 0 0.691973 192.652 331.887 ] concat
%I
[
(For each program instance:)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 258.349 241.636 ] concat
%I
[
(UIP = program unique identifier)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.856517 0 0 0.856517 258.349 225.099 ] concat
%I
[
(KUpub,KUpriv = program unique key pair)
] Text
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.661466 0 0 0.661466 30.7669 57.735 ] concat
%I
485 336 555 336 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.661466 0 0 0.661466 30.7669 57.735 ] concat
%I
555 336 555 460 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.676164 0 0 0.661466 22.6087 57.735 ] concat
%I
555 460 779 460 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.661466 0 0 0.661466 34.0742 57.735 ] concat
%I
779 460 779 415 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.661466 0 0 0.661466 30.7669 54.427 ] concat
%I
485 366 525 366 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.661466 0 0 0.661466 30.7669 57.735 ] concat
%I
525 361 525 495 Line
End

Begin %I Line
%I b 65535
1 0 0 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.721058 0 0 0.661466 -0.5185 57.735 ] concat
%I
525 495 635 495 Line
End

Begin %I Line
%I b 65535
1 0 1 [] 0 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.661466 0 0 0.661466 37.3815 57.735 ] concat
%I
635 495 635 415 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-times-medium-r-*-120-*
/Times-Roman 12 SetF
%I t
[ 1 0 0 1 258 174 ] concat
%I
[
(Figure 4: Generating program unique parameters)
] Text
End

End %I eop

showpage

%%Trailer

end
