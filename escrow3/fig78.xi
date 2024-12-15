%!PS-Adobe-2.0 EPSF-1.2
%%DocumentFonts: Helvetica-Bold Helvetica Times-Roman
%%Pages: 1
%%BoundingBox: 78 154 541 615
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

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.835049 0 0 0.835049 247.833 759.263 ] concat
%I
[
(        Sending)
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
[ 0.616331 0 0 1.26656 -2.24171 392.426 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.835049 0 0 0.835049 125.583 709.518 ] concat
%I
[
(message)
(      M)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.835049 0 0 0.835049 454.835 691.043 ] concat
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
[ 0.835049 0 0 0.835049 463.603 711.555 ] concat
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
[ 0.835049 0 0 0.835049 117.593 669.521 ] concat
%I
[
(session key)
(        KS)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.835049 0 0 0.835049 270.728 684.596 ] concat
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
[ 0.835049 0 0 0.835049 283.181 698.619 ] concat
%I
[
(KFpub)
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
[ 1.02598 0 0 0.77725 69.0369 562.129 ] concat
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
[ 0.544153 0 0 0.906792 221.395 533.872 ] concat
%I
288 167 378 167 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.835049 0 0 0.835049 241.846 670.465 ] concat
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
[ 0.835049 0 0 0.835049 418.928 671.207 ] concat
%I
[
(EVS = [ UIP ,KUpub, {UIP,KUpub}KEPFpriv ]KS)
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
[ 0.77725 0 0 0.77725 106.393 523.15 ] concat
%I
109 177 148 177 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-times-medium-r-*-120-*
/Times-Roman 12 SetF
%I t
[ 1 0 0 1 279 602 ] concat
%I
[
(Figure 7: Operation of the sending program)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.84848 0 0 0.84848 137.726 323.133 ] concat
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
[ 0.84848 0 0 0.84848 146.635 343.461 ] concat
%I
[
(encrypted message C = [M]KS)
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
[ 1.04146 0 0 0.789757 32.3812 186.02 ] concat
%I
288 167 378 167 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-bold-r-*-140-*
/Helvetica-Bold 14 SetF
%I t
[ 0.84848 0 0 0.84848 466.279 371.147 ] concat
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
[ 0.723243 0 0 1.1254 163.414 45.575 ] concat
%I
377 195 606 295 Rect
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.84848 0 0 0.84848 374.096 258.099 ] concat
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
[ 0.789757 0 0 0.789757 310.101 161.148 ] concat
%I
104 126 148 147 Line
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
[ 0.908002 0 0 0.846744 62.7772 51.449 ] concat
%I
600 299 645 299 Line
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-helvetica-medium-r-*-140-*
/Helvetica 14 SetF
%I t
[ 0.84848 0 0 0.84848 655.025 309.212 ] concat
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
[ 0.84848 0 0 0.84848 101.241 302.855 ] concat
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
[ 0.84848 0 0 0.84848 499.643 321.47 ] concat
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
[ 0.84848 0 0 0.84848 493.007 335.882 ] concat
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
[ 0.84848 0 0 0.84848 448.885 299.062 ] concat
%I
[
([ [KS]KUpub | UIP ]KFpub)
(             =? LEAF)
] Text
End

Begin %I Text
%I cfg Black
0 0 0 SetCFg
%I f *-times-medium-r-*-120-*
/Times-Roman 12 SetF
%I t
[ 1 0 0 1 266 208 ] concat
%I
[
(Figure 8:  Operation of the receiving program)
] Text
End

End %I eop

showpage

%%Trailer

end
