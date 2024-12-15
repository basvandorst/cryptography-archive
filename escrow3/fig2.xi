%!PS-Adobe-2.0 EPSF-1.2
%%DocumentFonts: Helvetica Times-Roman Helvetica-Bold
%%Pages: 1
%%BoundingBox: 69 99 559 558
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

Begin %I Pict
%I b u
%I cfg u
%I cbg u
%I f u
%I p u
%I t
[ 0.716418 0 0 0.716418 463.046 102.781 ] concat

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 80.2143 182.107 ] concat
%I
[
(LEGEND:)
] Text
End

Begin %I Rect
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.654202 0 0 0.60069 -400.392 -142.735 ] concat
%I
720 443 925 558 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 84.0428 162.964 ] concat
%I
[
([X]K)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 84.0428 147.649 ] concat
%I
[
({X}K)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 114.671 147.649 ] concat
%I
[
(X signed by K)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.627307 0 0 0.627307 114.671 162.964 ] concat
%I
[
(X encrypted by K)
] Text
End

End %I eop

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-times-medium-r-*-120-*
/Times-Roman 12 SetF
%I t
[ 1 0 0 1 262 139 ] concat
%I
[
(Figure 2: Complete software key escrow system)
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
[ 0.583552 0 0 1.07516 3.12025 357.867 ] concat
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
[ 0.622824 0 0 0.622824 239.173 668.834 ] concat
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
[ 0.284359 0 0 0.232074 148.344 412.294 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.622824 0 0 0.622824 153.354 385.477 ] concat
%I
[
(        Sending)
(Software Program)
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
[ 0.284359 0 0 0.488184 304.822 479.497 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.622824 0 0 0.622824 419.672 618.391 ] concat
%I
[
(Key Escrow)
(   Agent 1)
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
[ 0.365899 0 0 0.464025 298.114 362.528 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.622824 0 0 0.622824 441.404 492.592 ] concat
%I
[
(Law Enforcement)
(     Decryptor)
] Text
End

Begin %I Rect
%I b 13107
2 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.459692 0 0 0.855489 -33.9693 138.227 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.622824 0 0 0.622824 259.145 476.626 ] concat
%I
[
(    Software)
(Vendor / User)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 93.8171 340.174 ] concat
%I
[
(M)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 293.719 541.303 ] concat
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
[ 0.622824 0 0 0.622824 319.027 327.746 ] concat
%I
[
(LEAF = [ [KS]KUpub | UIP ]KFpub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 325.567 339.294 ] concat
%I
[
(encrypted message C = [M]KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 90.9184 325.132 ] concat
%I
[
(KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 169.859 329.786 ] concat
%I
[
(UIP,KUpub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 178.578 340.245 ] concat
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
[ 0 -0.622824 0.622824 0 513.271 556.943 ] concat
%I
[
(KUpriv2)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 464.375 467.696 ] concat
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
[ 0.622824 0 0 0.622824 461.898 658.323 ] concat
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
[ 0.622824 0 0 0.622824 382.846 673.661 ] concat
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
[ 0.622824 0 0 0.622824 226.307 580.61 ] concat
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
[ 0.622824 0 0 0.622824 236.895 593.344 ] concat
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
[ 0.622824 0 0 0.622824 276.816 632.216 ] concat
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
[ 0.622824 0 0 0.622824 250.286 604.624 ] concat
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
[ 0.622824 0 0 0.622824 488.359 618.391 ] concat
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
[ 0.622824 0 0 0.622824 419.984 592.882 ] concat
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
[ 0.622824 0 0 0.622824 488.67 593.82 ] concat
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
[ 0 -0.622824 0.622824 0 469.237 556.943 ] concat
%I
[
(KUpriv1)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0 -0.622824 0.622824 0 493.269 548.535 ] concat
%I
[
(UIP)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0 -0.622824 0.622824 0 449.235 548.535 ] concat
%I
[
(UIP)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 293.719 529.898 ] concat
%I
[
(UIP,KUpub)
] Text
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
[ 0.483094 0 0 0.511581 184.719 274.265 ] concat
%I
218 565 218 416 Line
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
[ 0.579714 0 0 0.579714 44.3105 233.303 ] concat
%I
109 177 148 177 Line
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
[ 0.579714 0 0 0.579714 44.3105 238.551 ] concat
%I
109 147 148 147 Line
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
[ 0.605886 0 0 0.676332 76.4752 217.369 ] concat
%I
288 167 378 167 Line
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
[ 0.284359 0 0 0.488184 373.509 479.497 ] concat
%I
377 195 606 295 Rect
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
[ 0.579714 0 0 0.62636 154.624 245.297 ] concat
%I
512 416 512 521 Line
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
[ 0.579714 0 0 0.62636 201.557 245.297 ] concat
%I
522 521 522 416 Line
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
[ 0.579714 0 0 0.62636 198.658 245.297 ] concat
%I
512 416 512 521 Line
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
[ 0.579714 0 0 0.62636 157.523 245.433 ] concat
%I
522 521 522 416 Line
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
[ 0.579714 0 0 0.347791 58.6254 276.464 ] concat
%I
557 192 557 351 Line
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
[ 0.666511 0 0 0.621548 123.134 292.602 ] concat
%I
600 299 645 299 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 557.869 481.811 ] concat
%I
[
(M)
] Text
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
[ 0.627678 0 0 0.579714 282.142 233.03 ] concat
%I
288 167 378 167 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.622824 0 0 0.622824 548.956 374.442 ] concat
%I
[
(       Receiving)
(Software Program)
] Text
End

Begin %I Rect
%I b 13107
2 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
none SetP %I p n
%I t
[ 0.530888 0 0 0.863474 326.64 124.415 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 483.919 293.418 ] concat
%I
[
(KS)
] Text
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
[ 0.579714 0 0 0.579714 427.31 200.837 ] concat
%I
119 164 162 200 Line
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
[ 0.666511 0 0 0.621548 252.769 139.77 ] concat
%I
600 299 645 299 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 687.503 328.979 ] concat
%I
[
(M)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.503168 0 0 0.503168 244.993 615.384 ] concat
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
[ 0.622824 0 0 0.622824 250.035 643.619 ] concat
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
[ 0.622824 0 0 0.622824 293.719 518.115 ] concat
%I
[
({UIP,KUpub}KEPFpriv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 293.719 552 ] concat
%I
[
(KEPFpub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 147.749 319.247 ] concat
%I
[
({UIP,KUpub}KEPFpriv)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 292.246 315.647 ] concat
%I
[
(EVS = [ UIP ,KUpub, {UIP,KUpub}KEPFpriv ]KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 574.181 336.467 ] concat
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
[ 0.622824 0 0 0.622824 569.075 347.493 ] concat
%I
[
(KEPFpub)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 536.188 318.867 ] concat
%I
[
([ [KS]KUpub | UIP ]KFpub)
(             =? LEAF)
] Text
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.38012 0 0 0.38012 81.863 485.746 ] concat
%I
393 356 693 356 Line
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
[ 0.760241 0 0 0.760241 76.9214 213.2 ] concat
%I
367 478 417 478 Line
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
[ 0.760241 0 0 0.8541 76.9214 168.43 ] concat
%I
417 478 417 558 Line
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
[ 0.760241 0 0 0.760241 76.9214 220.802 ] concat
%I
417 558 571 558 Line
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
[ 0.760241 0 0 0.676107 76.9214 267.887 ] concat
%I
571 558 571 533 Line
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
[ 0.760241 0 0 0.760241 76.9214 213.2 ] concat
%I
367 498 392 498 Line
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
[ 0.760241 0 0 0.8541 76.9214 166.552 ] concat
%I
392 498 392 578 Line
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
[ 0.760241 0 0 0.760241 76.9214 220.802 ] concat
%I
392 578 481 578 Line
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
[ 0.760241 0 0 0.702463 76.9214 254.084 ] concat
%I
481 578 481 533 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.622824 0 0 0.622824 172.973 351.054 ] concat
%I
[
(KEPFpub)
] Text
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.819867 0 0 0.760241 77.1747 144.778 ] concat
%I
137 374 391 374 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.760241 0 0 0.699422 100.489 257.75 ] concat
%I
391 245 640 245 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.760241 0 0 0.709558 100.489 255.216 ] concat
%I
640 245 640 619 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.775035 0 0 0.760241 91.0228 223.843 ] concat
%I
640 619 127 619 Line
End

Begin %I Line
%I b 13107
1 0 0 [2 2 2 2 2 2 2 2] 15 SetB
%I cfg Black
0 0 0 SetCFg
%I cbg White
1 1 1 SetCBg
%I p
0 SetP
%I t
[ 0.760241 0 0 1.08453 92.8865 23.1012 ] concat
%I
127 619 127 374 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.622824 0 0 0.622824 527.304 687.84 ] concat
%I
[
(  Protected)
(Environment)
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
[ 1.03769 0 0 0.952978 -168.343 108.023 ] concat
%I
530 305 619 305 Line
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
[ 0.952978 0 0 0.952978 -115.855 108.023 ] concat
%I
619 305 619 359 Line
End

End %I eop

showpage

%%Trailer

end
