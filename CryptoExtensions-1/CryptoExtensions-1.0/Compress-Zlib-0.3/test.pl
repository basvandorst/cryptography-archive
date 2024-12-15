
use Compress::Zlib ;

sub ok
{
    my ($no, $ok) = @_ ;

    ++ $total ;
    ++ $totalBad unless $ok ;

    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

$BUFLEN = 4096 ;

$hello = <<EOM ;
hello world
this is a test
EOM

$len   = length $hello ;

# Check zlib_version and ZLIB_VERSION are the same.
ok(1, Compress::Zlib::zlib_version == ZLIB_VERSION) ;

# gzip tests
#===========

$name = "test.gz" ;

ok(2, $fil = gzopen($name, "wb")) ;

ok(3, $fil->gzwrite($hello) == $len) ;

ok(4, ! $fil->gzclose ) ;

ok(5, $fil = gzopen($name, "rb") ) ;

ok(6, ($x = $fil->gzread($uncomp)) == $len) ;

ok(7, ! $fil->gzclose ) ;

unlink $name ;

ok(8, $hello eq $uncomp) ;

# now a bigger gzip test

$text = 'text' ;
$file = "$text.gz" ;

ok(9, $f = gzopen($file, "wb")) ;

# generate a long random string
foreach (1 .. 5000)
  { $contents .= chr int rand 255 }

$len = length $contents ;

ok(10, $f->gzwrite($contents) == $len ) ;

ok(11, ! $f->gzclose );

ok(12, $f = gzopen($file, "rb")) ;
 
ok(13, $f->gzread($uncompressed, $len) == $len) ;

ok(14, $contents eq $uncompressed) ;

ok(15, ! $f->gzclose ) ;

unlink($file) ;

# gzip - readline tests
# ======================

# first create a small gzipped text file
$name = "test.gz" ;
@text = (<<EOM, <<EOM, <<EOM, <<EOM) ;
this is line 1
EOM
the second line
EOM
the line after the previous line
EOM
the final line
EOM

$text = join("", @text) ;

ok(16, $fil = gzopen($name, "wb")) ;
ok(17, $fil->gzwrite($text) == length $text) ;
ok(18, ! $fil->gzclose ) ;

# now try to read it back in
ok(19, $fil = gzopen($name, "rb")) ;
$aok = 1 ; 
while ($fil->gzreadline($line) > 0) {
    ($aok = 0), last
	if $line ne $text[$lines] ;
    $remember .= $line ;
    ++ $lines ;
}
ok(20, $aok) ;
ok(21, $remember eq $text) ;
ok(22, $lines == @text) ;
ok(23, ! $fil->gzclose ) ;
unlink($name) ;

# a text file with a very long line (bigger than the internal buffer)
$line1 = ("abcdefghijklmnopq" x 2000) . "\n" ;
$line2 = "second line\n" ;
$text = $line1 . $line2 ;
ok(24, $fil = gzopen($name, "wb")) ;
ok(25, $fil->gzwrite($text) == length $text) ;
ok(26, ! $fil->gzclose ) ;

# now try to read it back in
ok(27, $fil = gzopen($name, "rb")) ;
while ($fil->gzreadline($line) > 0) {
    $got[$i] = $line ;    
    ++ $i ;
}
ok(28, $i == 2) ;
ok(29, $got[0] eq $line1 ) ;
ok(30, $got[1] eq $line2) ;

ok(31, ! $fil->gzclose ) ;

unlink $name ;

# a text file which is not termined by an EOL

$line1 = "hello hello, I'm back again\n" ;
$line2 = "there is no end in sight" ;

$text = $line1 . $line2 ;
ok(32, $fil = gzopen($name, "wb")) ;
ok(33, $fil->gzwrite($text) == length $text) ;
ok(34, ! $fil->gzclose ) ;

# now try to read it back in
ok(35, $fil = gzopen($name, "rb")) ;
@got = () ; $i = 0 ;
while ($fil->gzreadline($line) > 0) {
    $got[$i] = $line ;    
    ++ $i ;
}
ok(36, $i == 2) ;
ok(37, $got[0] eq $line1 ) ;
ok(38, $got[1] eq $line2) ;

ok(39, ! $fil->gzclose ) ;

unlink $name ;


# mix gzread and gzreadline <

# case 1: read a line, then a block. The block is
#         smaller than the internal block used by
#	  gzreadline
$line1 = "hello hello, I'm back again\n" ;
$line2 = "abc" x 200 ; 
$line3 = "def" x 200 ;

$text = $line1 . $line2 . $line3 ;
ok(40, $fil = gzopen($name, "wb")) ;
ok(41, $fil->gzwrite($text) == length $text) ;
ok(42, ! $fil->gzclose ) ;

# now try to read it back in
ok(43, $fil = gzopen($name, "rb")) ;
ok(44, $fil->gzreadline($line) > 0) ;
ok(45, $line eq $line1) ;
ok(46, $fil->gzread($line, length $line2) > 0) ;
ok(47, $line eq $line2) ;
ok(48, $fil->gzread($line, length $line3) > 0) ;
ok(49, $line eq $line3) ;
unlink $name ;

# change $/ <<TODO

# compress/uncompress tests
# =========================

$hello = "hello mum" ;

$compr = compress ($hello) ;
ok(50, $compr ne "") ;


$uncompr = uncompress ($compr) ;

ok(51, $hello eq $uncompr) ;


# bigger compress

$compr = compress ($contents) ;
ok(52, $compr ne "") ;

$uncompr = uncompress ($compr) ;

ok(53, $contents eq $uncompr) ;


# deflate/inflate - small buffer
# ==============================

$hello = "I am a HAL 9000 computer" ;
@hello = split('', $hello) ;
 
ok(54,  ($x, $err) = deflateInit( {Bufsize => 1} ) ) ;
ok(55, $x) ;
ok(56, $err == Z_OK) ;
 
foreach (@hello)
{
    ($X, $status) = $x->deflate($_) ;
    last unless $Status == Z_OK ;

    $Answer .= $X ;
}
 
ok(57, $status == Z_OK) ;

ok(58,    (($X, $status) = $x->flush())[1] == Z_OK ) ;
$Answer .= $X ;
 
 
 
@Answer = split('', $Answer) ;
 
ok(59, ($k, $err) = inflateInit( {Bufsize => 1}) ) ;
ok(60, $k) ;
ok(61, $err == Z_OK) ;
 
foreach (@Answer)
{
    ($Z, $status) = $k->inflate($_) ;
    $GOT .= $Z ;
    last if $status == Z_STREAM_END or $status != Z_OK ;
 
}
 
ok(62, $status == Z_STREAM_END) ;
ok(63, $GOT eq $hello ) ;


 
# deflate/inflate - larger buffer
# ==============================


ok(64, $x = deflateInit() ) ;
 
ok(65, (($X, $status) = $x->deflate($hello))[1] == Z_OK) ;

$Y = $X ;
 
 
ok(66, (($X, $status) = $x->flush() )[1] == Z_OK ) ;
$Y .= $X ;
 
 
 
ok(67, $k = inflateInit() ) ;
 
($Z, $status) = $k->inflate($Y) ;
 
ok(68, $status == Z_STREAM_END) ;
ok(69, $hello eq $Z ) ;


# all done.

if ($totalBad) 
    { print "$totalBad tests failed\n" }
else 
    { print "All $total tests successful\n" unless $totalBad }



