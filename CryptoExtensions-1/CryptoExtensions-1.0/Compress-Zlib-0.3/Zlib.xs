/* Filename: Zlib.xs
 * Author  : Paul Marquess, <pmarquess@bfsec.bt.co.uk>
 * Created : 2nd October 1995
 * Version : 0.3
 *
 *   Copyright (c) 1995 Paul Marquess. All rights reserved.
 *   This program is free software; you can redistribute it and/or
 *   modify it under the same terms as Perl itself.
 *
 */

/* Part of this code is based on the file gzio.c */

/* gzio.c -- IO on .gz files
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */



#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <zutil.h>


typedef struct di_stream {
    z_stream stream;
    int	     bufsize; 
} di_stream;

typedef di_stream * deflateStream ;
typedef di_stream * Compress__Zlib__deflateStream ;
typedef di_stream * inflateStream ;
typedef di_stream * Compress__Zlib__inflateStream ;

/* typedef gzFile Compress__Zlib__gzFile ; */
typedef struct gzType {
    gzFile gz ;
    SV *   buffer ;
    int	   offset ;
}  gzType ;

typedef gzType* Compress__Zlib__gzFile ; 

#define Zip_zlib_version()	zlib_version
#define Zip_ZLIB_VERSION()	ZLIB_VERSION

#define Zip_gzwrite(file, buf) gzwrite(file->gz, buf, len)
#define Zip_gzerror(file) gzerror(file->gz, &errnum)
#define Zip_gzflush(file, flush) gzflush(file->gz, flush) 
#define Zip_gzclose(file) gzclose(file->gz)

#define Zip_adler32(buf, adler) adler32(adler, buf, len)
#define Zip_crc32(buf, crc) crc32(crc, buf, len)

#define GZERRNO	"Compress::Zlib::gzerrno"

char *z_errmsg[] = {
"stream end",          /* Z_STREAM_END    1 */
"",                    /* Z_OK            0 */
"file error",          /* Z_ERRNO        (-1) */
"stream error",        /* Z_STREAM_ERROR (-2) */
"data error",          /* Z_DATA_ERROR   (-3) */
"insufficient memory", /* Z_MEM_ERROR    (-4) */
"buffer error",        /* Z_BUF_ERROR    (-5) */
""};


static uLong adlerInitial ;
static uLong crcInitial ;
static int trace = 0 ;

static void
SetGzErrorNo(error_no)
int error_no ;
{
    char * errstr ;
    SV * gzerror_sv = perl_get_sv(GZERRNO, FALSE) ;

    if (error_no == Z_ERRNO) {
        error_no = errno ;
        errstr = Strerror(errno) ;
    }
    else
        errstr = z_errmsg[1 - error_no];

    sv_setiv(gzerror_sv, error_no) ;
    sv_setpv(gzerror_sv, errstr) ;
    SvIOK_on(gzerror_sv) ;

}

static void
SetGzError(file)
gzFile file ;
{
    int error_no ;

    (void)gzerror(file, &error_no) ;
    SetGzErrorNo(error_no) ;
}

static di_stream *
InitStream(bufsize)
    int bufsize ;
{
    di_stream *s = (di_stream *)safemalloc(sizeof(di_stream));

    if (s)  {
        /* s->stream.zalloc = (alloc_func) Safemalloc; */
        /* s->stream.zfree = (free_func) Safefree; */
        s->stream.zalloc = (alloc_func) 0;
        s->stream.zfree = (free_func) 0;
        s->stream.next_in = Z_NULL;
        s->stream.next_out = Z_NULL;
        s->stream.avail_in = s->stream.avail_out = 0;
        s->bufsize = bufsize ;
    }

    return s ;
    
}

#define SIZE 4096

static int
gzreadline(file, output)
  Compress__Zlib__gzFile file ;
  SV * output ;
{

    SV * store = file->buffer ;
    char *nl = "\n"; 
    char *p;
    char *out_ptr = SvPVX(store) ;
    int n;

    while (1) {

	/* anything left from last time */
	if (n = SvCUR(store)) {

    	    out_ptr = SvPVX(store) + file->offset ;
	    if (p = ninstr(out_ptr, out_ptr + n - 1, nl, nl)) {
            /* if (rschar != 0777 && */
                /* p = ninstr(out_ptr, out_ptr + n - 1, rs, rs+rslen-1)) { */

         	sv_catpvn(output, out_ptr, p - out_ptr + 1);

		file->offset += (p - out_ptr + 1) ;
	        n = n - (p - out_ptr + 1);
	        SvCUR_set(store, n) ;
	        return SvCUR(output);
            }
	    else /* no EOL, so append the complete buffer */
         	sv_catpvn(output, out_ptr, n);
	    
	}


	SvCUR_set(store, 0) ;
	file->offset = 0 ;

	n = gzread(file->gz, out_ptr, SIZE) ;

	if (n <= 0) 
	    /* Either EOF or an error */
	    /* so return what we have so far else signal eof */
	    return (SvCUR(output)>0) ? SvCUR(output) : n ;

	SvCUR_set(store, n) ;
    }
}



static int
not_here(s)
char *s;
{
    croak("%s not implemented on this architecture", s);
    return -1;
}



static double
constant(name, arg)
char *name;
int arg;
{
    errno = 0;
    switch (*name) {
    case 'A':
	break;
    case 'B':
	break;
    case 'C':
	break;
    case 'D':
	if (strEQ(name, "DEF_MEM_LEVEL"))
#ifdef DEF_MEM_LEVEL
	    return DEF_MEM_LEVEL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEF_WBITS"))
#ifdef DEF_WBITS
	    return DEF_WBITS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEFLATED"))
#ifdef DEFLATED
	    return DEFLATED;
#else
	    goto not_there;
#endif
	break;
    case 'E':
	break;
    case 'F':
	break;
    case 'G':
	break;
    case 'H':
	break;
    case 'I':
	break;
    case 'J':
	break;
    case 'K':
	break;
    case 'L':
	break;
    case 'M':
	if (strEQ(name, "MAX_WBITS"))
#ifdef MAX_WBITS
	    return MAX_WBITS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "DEF_MEM_LEVEL"))
	break;
    case 'N':
	break;
    case 'O':
	if (strEQ(name, "OS_CODE"))
#ifdef OS_CODE
	    return OS_CODE;
#else
	    goto not_there;
#endif
	break;
    case 'P':
	break;
    case 'Q':
	break;
    case 'R':
	break;
    case 'S':
	break;
    case 'T':
	break;
    case 'U':
	break;
    case 'V':
	break;
    case 'W':
	break;
    case 'X':
	break;
    case 'Y':
	break;
    case 'Z':
	if (strEQ(name, "Z_ASCII"))
#ifdef Z_ASCII
	    return Z_ASCII;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_BEST_COMPRESSION"))
#ifdef Z_BEST_COMPRESSION
	    return Z_BEST_COMPRESSION;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_BEST_SPEED"))
#ifdef Z_BEST_SPEED
	    return Z_BEST_SPEED;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_BINARY"))
#ifdef Z_BINARY
	    return Z_BINARY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_BUF_ERROR"))
#ifdef Z_BUF_ERROR
	    return Z_BUF_ERROR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_DATA_ERROR"))
#ifdef Z_DATA_ERROR
	    return Z_DATA_ERROR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_DEFAULT_COMPRESSION"))
#ifdef Z_DEFAULT_COMPRESSION
	    return Z_DEFAULT_COMPRESSION;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_DEFAULT_STRATEGY"))
#ifdef Z_DEFAULT_STRATEGY
	    return Z_DEFAULT_STRATEGY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_ERRNO"))
#ifdef Z_ERRNO
	    return Z_ERRNO;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_FILTERED"))
#ifdef Z_FILTERED
	    return Z_FILTERED;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_FINISH"))
#ifdef Z_FINISH
	    return Z_FINISH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_FULL_FLUSH"))
#ifdef Z_FULL_FLUSH
	    return Z_FULL_FLUSH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_HUFFMAN_ONLY"))
#ifdef Z_HUFFMAN_ONLY
	    return Z_HUFFMAN_ONLY;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_MEM_ERROR"))
#ifdef Z_MEM_ERROR
	    return Z_MEM_ERROR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_NO_FLUSH"))
#ifdef Z_NO_FLUSH
	    return Z_NO_FLUSH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_NULL"))
#ifdef Z_NULL
	    return Z_NULL;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_OK"))
#ifdef Z_OK
	    return Z_OK;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_PARTIAL_FLUSH"))
#ifdef Z_PARTIAL_FLUSH
	    return Z_PARTIAL_FLUSH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_STREAM_END"))
#ifdef Z_STREAM_END
	    return Z_STREAM_END;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_STREAM_ERROR"))
#ifdef Z_STREAM_ERROR
	    return Z_STREAM_ERROR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_SYNC_FLUSH"))
#ifdef Z_SYNC_FLUSH
	    return Z_SYNC_FLUSH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "Z_UNKNOWN"))
#ifdef Z_UNKNOWN
	    return Z_UNKNOWN;
#else
	    goto not_there;
#endif
	break;
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}


MODULE = Compress::Zlib		PACKAGE = Compress::Zlib	PREFIX = Zip_

REQUIRE:	1.924
PROTOTYPES:	DISABLE

BOOT:
    {
        /* Create the $gzerror scalar */
        SV * gzerror_sv = perl_get_sv(GZERRNO, TRUE) ;
        sv_setiv(gzerror_sv, 0) ;
        sv_setpv(gzerror_sv, "") ;
        SvIOK_on(gzerror_sv) ;
    }


char*
Zip_zlib_version()

char *
Zip_ZLIB_VERSION()

double
constant(name,arg)
	char *		name
	int		arg


Compress::Zlib::gzFile
gzopen(path, mode)
	char *	path
	char *	mode
	CODE:
	gzFile	gz = gzopen(path, mode) ;
	SetGzErrorNo(errno ? Z_ERRNO : Z_MEM_ERROR) ;
	if (gz) {
	    RETVAL = (gzType*)safemalloc(sizeof(gzType)) ;
	    /* RETVAL->buffer = newSVpv("", SIZE) ; */
    	    RETVAL->buffer = newSV(SIZE) ;
    	    SvPOK_only(RETVAL->buffer) ;
    	    SvCUR_set(RETVAL->buffer, 0) ; 
	    RETVAL->offset = 0 ;
	    RETVAL->gz = gz ;
	}
	else
	    RETVAL = NULL ;
	OUTPUT:
	  RETVAL




MODULE = Compress::Zlib	PACKAGE = Compress::Zlib::gzFile PREFIX = Zip_

#define Zip_gzread(file, buf, len) gzread(file->gz, bufp, len)

int
Zip_gzread(file, buf, len=4096)
	Compress::Zlib::gzFile	file
	unsigned	len
	SV *		buf
	voidp		bufp = NO_INIT
	int		bufsize = 0 ;
	CODE:
	if (SvREADONLY(buf) && curcop != &compiling)
            croak("gzread: buffer parameter is read-only");
        if (!SvUPGRADE(buf, SVt_PV))
            croak("cannot use buf argument as lvalue");
        SvPOK_only(buf);
        SvCUR_set(buf, 0);
	/* any left over from gzreadline ? */
	if ((bufsize = SvCUR(file->buffer)) > 0) {
	    int movesize ;
	    RETVAL = bufsize ;

	    if (bufsize < len) {
		movesize = bufsize ;
	        len -= movesize ;
	    }
	    else {
	        movesize = len ;
	        len = 0 ;
	    }

       	    sv_catpvn(buf, SvPVX(file->buffer) + file->offset, movesize);

	    file->offset += movesize ;
	    SvCUR_set(file->buffer, bufsize - movesize) ;
	}

	if (len) {
	    bufp = (Byte*)SvGROW(buf, bufsize+len+1);
	    RETVAL = gzread(file->gz, ((Bytef*)bufp)+bufsize, len) ;
	    SetGzError(file->gz) ; 
            if (RETVAL >= 0) {
		RETVAL += bufsize ;
                SvCUR_set(buf, RETVAL) ;
                SvTAINT(buf) ;
                *SvEND(buf) = '\0';
            }
	}
	OUTPUT:
	   RETVAL

int
gzreadline(file, buf)
	Compress::Zlib::gzFile	file
	SV *		buf
	CODE:
	if (SvREADONLY(buf) && curcop != &compiling) 
            croak("gzreadline: buffer parameter is read-only"); 
        if (!SvUPGRADE(buf, SVt_PV))
            croak("cannot use buf argument as lvalue");
        SvPOK_only(buf);
	/* sv_setpvn(buf, "", SIZE) ; */
        SvGROW(buf, SIZE) ;
        SvCUR_set(buf, 0);
	RETVAL = gzreadline(file, buf) ;
	SetGzError(file->gz) ; 
	OUTPUT:
	  RETVAL
	CLEANUP:
        if (RETVAL >= 0) {
            /* SvCUR(buf) = RETVAL; */
            SvTAINT(buf) ;
            /* *SvEND(buf) = '\0'; */
        }

int
Zip_gzwrite(file, buf)
	Compress::Zlib::gzFile	file
	unsigned	len = SvCUR(ST(1)) ;
	voidp 		buf = (voidp)SvPVX(ST(1)) ;
	CLEANUP:
	  SetGzError(file->gz) ;

int
Zip_gzflush(file, flush)
	Compress::Zlib::gzFile	file
	int		flush
	CLEANUP:
	  SetGzError(file->gz) ;

int
Zip_gzclose(file)
	Compress::Zlib::gzFile		file
	CLEANUP:
	  SetGzErrorNo(RETVAL) ;

void
DESTROY(file)
	Compress::Zlib::gzFile		file
	CODE:
	    SvREFCNT_dec(file->buffer) ;
	    safefree(file) ;

char *
Zip_gzerror(file)
	Compress::Zlib::gzFile	file
	int		errnum = NO_INIT
	CLEANUP:
	    sv_setiv(ST(0), errnum) ;
            SvPOK_on(ST(0)) ;



MODULE = Compress::Zlib	PACKAGE = Compress::Zlib	PREFIX = Zip_


BOOT:
    adlerInitial = adler32(0L, Z_NULL, 0);
    crcInitial   = crc32(0L, Z_NULL, 0);

uLong
Zip_adler32(buf, adler=adlerInitial)
        uLong   adler
        uint    len = NO_INIT
        Bytef *  buf = (Byte*)SvPV(ST(1), len) ;
 
uLong
Zip_crc32(buf, crc=crcInitial)
        uLong   crc
        uint     len = NO_INIT
        Bytef *  buf = (Byte*)SvPV(ST(1), len) ;


MODULE = Compress::Zlib PACKAGE = Compress::Zlib

void
_deflateInit(level, method, windowBits, memLevel, strategy, bufsize)
    int	level
    int method
    int windowBits
    int memLevel
    int strategy
    int bufsize
  PPCODE:

    int err ;
    deflateStream s ;

    if (trace)
        warn("in _deflateInit(level=%d, method=%d, windowBits=%d, memLevel=%d, strategy=%d, bufsize=%d\n",
	level, method, windowBits, memLevel, strategy, bufsize) ;
    if (s = InitStream(bufsize) ) {
        err = deflateInit2(&(s->stream), level, 
			   method, windowBits, memLevel, strategy);
        if (err != Z_OK) {
            Safefree(s) ;
            s = NULL ;
        }
        
    }
    XPUSHs(sv_setref_pv(sv_newmortal(), 
	"Compress::Zlib::deflateStream", (void*)s));
    if (GIMME == G_ARRAY) 
        XPUSHs(sv_2mortal(newSViv(err))) ;

void
_inflateInit(windowBits, bufsize)
    int windowBits
    int bufsize
  PPCODE:
 
    int err = Z_OK ;
    inflateStream s ;
 
    if (trace)
        warn("in _inflateInit(windowBits=%d, bufsize=%d\n",
                windowBits, bufsize) ;
    if (s = InitStream(bufsize) ) {
        err = inflateInit2(&(s->stream), windowBits);
 
        if (err != Z_OK) {
            Safefree(s) ;
            s = NULL ;
        }
    }
    XPUSHs(sv_setref_pv(sv_newmortal(), 
                   "Compress::Zlib::inflateStream", (void*)s));
    if (GIMME == G_ARRAY) 
        XPUSHs(sv_2mortal(newSViv(err))) ;
 


MODULE = Compress::Zlib PACKAGE = Compress::Zlib::deflateStream

int 
deflate (s, buf)
    Compress::Zlib::deflateStream	s
    SV *	buf
    int		outsize = NO_INIT 
    SV * 	output = NO_INIT
    int		err = NO_INIT
  PPCODE:
  
    /* initialise the input buffer */
    s->stream.next_in = (Bytef*)SvPVX(buf) ;
    s->stream.avail_in = SvCUR(buf) ;

    /* and the output buffer */
    /* output = sv_2mortal(newSVpv("", s->bufsize)) ; */
    output = sv_2mortal(newSV(s->bufsize)) ;
    SvPOK_only(output) ;
    SvCUR_set(output, 0) ; 
    outsize = s->bufsize ;
    s->stream.next_out = (Bytef*) SvPVX(output) ;
    s->stream.avail_out = outsize;
    
    while (s->stream.avail_in != 0) {

        if (s->stream.avail_out == 0) {
            SvGROW(output, outsize + s->bufsize) ;
            s->stream.next_out = (Bytef*) SvPVX(output) + outsize ;
            outsize += s->bufsize ;
            s->stream.avail_out = s->bufsize ;
        }
        err = deflate(&(s->stream), Z_NO_FLUSH);
        if (err != Z_OK) 
            break;
    }

    if (err == Z_OK) {
        SvPOK_only(output);
        SvCUR_set(output, outsize - s->stream.avail_out) ;
    }
    else
        output = &sv_undef ;
    XPUSHs(output) ;
    if (GIMME == G_ARRAY) 
        XPUSHs(sv_2mortal(newSViv(err))) ;
  

void
DESTROY(s)
    Compress::Zlib::deflateStream	s
  CODE:
    deflateEnd(&s->stream) ;
    Safefree(s) ;


void
flush(s)
    Compress::Zlib::deflateStream	s
    int	outsize = NO_INIT
    SV * output = NO_INIT
    int err = Z_OK ;
  PPCODE:
  
    s->stream.avail_in = 0; /* should be zero already anyway */
  
    /* output = sv_2mortal(newSVpv("", s->bufsize)) ; */
    output = sv_2mortal(newSV(s->bufsize)) ;
    SvPOK_only(output) ;
    SvCUR_set(output, 0) ; 
    outsize = s->bufsize ;
    s->stream.next_out = (Bytef*) SvPVX(output) ;
    s->stream.avail_out = outsize;
      
    for (;;) {
        if (s->stream.avail_out == 0) {
	    /* consumed all the available output, so extend it */
	    SvGROW(output, outsize + s->bufsize) ;
            s->stream.next_out = (Bytef*)SvPVX(output) + outsize ;
	    outsize += s->bufsize ;
            s->stream.avail_out = s->bufsize ;
        }
        err = deflate(&(s->stream), Z_FINISH);
    
        /* deflate has finished flushing only when it hasn't used up
         * all the available space in the output buffer: 
         */
        if (s->stream.avail_out != 0 || err != Z_OK )
            break;
    }
  
    err =  (err == Z_STREAM_END ? Z_OK : err) ;
  
    if (err == Z_OK) {
        SvPOK_only(output);
        SvCUR_set(output, outsize - s->stream.avail_out) ;
    }
    else
        output = &sv_undef ;
    XPUSHs(output) ;
    if (GIMME == G_ARRAY) 
        XPUSHs(sv_2mortal(newSViv(err))) ;




MODULE = Compress::Zlib PACKAGE = Compress::Zlib::inflateStream

int 
inflate (s, buf)
    Compress::Zlib::inflateStream	s
    SV *	buf
    int		outsize = NO_INIT 
    SV * 	output = NO_INIT
    int		err = Z_OK ;
  PPCODE:
    /* initialise the input buffer */
    s->stream.next_in = (Bytef*)SvPVX(buf) ;
    s->stream.avail_in = SvCUR(buf) ;

    /* and the output buffer */
    /* output = sv_2mortal(newSVpv("", s->bufsize+1)) ; */
    output = sv_2mortal(newSV(s->bufsize+1)) ;
    SvPOK_only(output) ;
    SvCUR_set(output, 0) ; 
    outsize = s->bufsize ;
    s->stream.next_out = (Bytef*) SvPVX(output)  ;
    s->stream.avail_out = outsize;

    while (s->stream.avail_in != 0) {

        if (s->stream.avail_out == 0) {
            SvGROW(output, outsize + s->bufsize+1) ;
            s->stream.next_out = (Bytef*) SvPVX(output) + outsize ;
            outsize += s->bufsize ;
            s->stream.avail_out = s->bufsize ;
        }
        err = inflate(&(s->stream), Z_PARTIAL_FLUSH);
        if (err != Z_OK) 
            break;
    }

    if (err == Z_OK || err == Z_STREAM_END) {
        SvPOK_only(output);
        SvCUR_set(output, outsize - s->stream.avail_out) ;
        *SvEND(output) = '\0';
    }
    else
        output = &sv_undef ;
    XPUSHs(output) ;
    if (GIMME == G_ARRAY) 
        XPUSHs(sv_2mortal(newSViv(err))) ;


void
DESTROY(s)
    Compress::Zlib::inflateStream	s
  CODE:
    inflateEnd(&s->stream) ;
    Safefree(s) ;

MODULE = Compress::Zlib		PACKAGE = Compress::Zlib	PREFIX = Zip_

void
rs_test()
    CODE:
	/*
	warn("rs = [%s], rschar = %d[%c], rslen = %d, rspara = %d\n\n",
		rs, rschar, rschar, rslen, rspara) ;
	*/
