use ExtUtils::MakeMaker;

# If libz is not installed in some place obvious, uncomment the next
# line and change the path to point to the directory where libz is
# installed.

#$ZLIB_LIB     = '-L/usr/local/lib' ;

# Change the path on the next line to point to the directory where
# zlib.h, zconf.h and zutil.h can be found.

$ZLIB_INCLUDE = '-I./zlib' ;

# The next two line keep -w happy
$ZLIB_LIB = '' unless defined $ZLIB_LIB ; 
$ZLIB_INCLUDE = '' unless defined $ZLIB_INCLUDE ;

WriteMakefile(	
                NAME   	 => 'Compress::Zlib',
		VERSION  => '0.3',
		LIBS	 => [ "$ZLIB_LIB -lz" ],
	      	INC      => " $ZLIB_INCLUDE" ,
		'dist'	 => {COMPRESS=>'gzip', SUFFIX=>'gz'}
	    ) ;



