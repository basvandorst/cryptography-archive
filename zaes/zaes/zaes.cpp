#include <mem.h>
#include <ctype.h>

#include "file.h"

#ifndef NDEBUG
  //#define VIEW
  #ifdef VIEW
    //#define PASS_VIEW
  #endif
#endif

const word N256 = 256;
const word N16 = 16;

const word BUF_LEN = 0x6000;

const byte pad[ N16 ] = {
  0xA2, 0xD5, 0x44, 0xC0, 0x72, 0x60, 0x13, 0xC8,
  0xA3, 0x05, 0xD7, 0xE8, 0x3C, 0xC8, 0x0B, 0x2A
};

const byte LEMPTY = 0;
const byte REMPTY = 0;

byte gt[ N256 ][ N256 ];
byte gt_1x[ N256 ][ N256 ];
byte gt_1y[ N256 ][ N256 ];

byte buf[ BUF_LEN ];
File input_file, output_file;
enum moo_t { ZAES_ECB, ZAES_CBC, ZAES_PCBC, ZAES_CFB_128, ZAES_OFB_128 };

inline word g( word x, word y ) { return gt[x][y]; }

inline word ung_y( word x, word s ) { return gt_1y[x][s]; }

inline word ung_x( word y, word s ) { return gt_1x[y][s]; }

void init_d()
{
  File adf_1x;
  adf_1x.open( "zaes_1x.adf", O_RDONLY | O_BINARY );
  assert( adf_1x.filelength() == N256*N256 );
  adf_1x.read( gt_1x, N256*N256 );
  #ifndef NDEBUG
    word x, y;
    for( x=0; x<N256; x++ ) {
      for( y=0; y<N256; y++ ) {
        word s = g( x, y );
        assert( gt_1x[y][s] == x );
      }
    }
  #endif
  File adf_1y;
  adf_1y.open( "zaes_1y.adf", O_RDONLY | O_BINARY );
  assert( adf_1y.filelength() == N256*N256 );
  adf_1y.read( gt_1y, N256*N256 );
  #ifndef NDEBUG
    for( x=0; x<N256; x++ ) {
      for( y=0; y<N256; y++ ) {
        word s = g( x, y );
        assert( gt_1y[x][s] == y );
      }
    }
  #endif
}

void init( byte key[N16], byte perm[N16] )
{
  File adf;
  adf.open( "zaes.adf", O_RDONLY | O_BINARY );
  assert( adf.filelength() == N256*N256 );
  adf.read( gt, N256*N256 );
  #ifndef NDEBUG
    word x, y;
    for( x=0; x<N256; x++ ) {
      word c = 0;
      for( y=0; y<N256; y++ ) if( g( x, y ) == x ) c++;
      assert( c == 1 );
    }
    for( y=0; y<N256; y++ ) {
      word c = 0;
      for( x=0; x<N256; x++ ) if( g( x, y ) == y ) c++;
      assert( c == 1 );
    }
    word cnt = 0;
    for( x=0; x<N256; x++ ) if( g( x, x ) == x ) cnt++;
    assert( cnt == 0 );
    int asso = 1;
    int comm = 1;
    for( x=0; x<N256; x++ ) {
      for( y=0; y<N256; y++ ) {
        if( g( x, y ) != g( y, x ) ) comm = 0;
        for( word z=0; z<N256; z++ ) {
          word g1 = g( g( x, y), z );
          word g2 = g( x, g( y, z ) );
          if( g1 != g2 ) asso = 0;
        }
        if( !asso && !comm ) break;
      }
      if( !asso && !comm ) break;
    }
    assert( !asso && !comm );
  #endif
  File pkey;
  pkey.open( "zaes.key", O_RDONLY | O_BINARY );
  if( pkey.filelength() < N16 ) {
    printf( "zaes.key must be 16 bytes long\n" );
    exit( 1 );
  }
  pkey.read( &key[0], N16 );
  for( byte k=0; k<N16; k++ ) perm[k] = k;
  for( byte j=16; j>=1; j-- ) {
    word k = key[16-j] % j;
    word j1 = j-1;
    byte temp = perm[j1];
    perm[j1] = perm[k];
    perm[k] = temp;
  }
  #ifndef NDEBUG
    byte nt[16];
    for( word s=0; s<N16; s++ ) nt[s] = 0;
    for( s=0; s<N16; s++ ) nt[perm[s]]++;
    for( s=0; s<N16; s++ ) assert( nt[s] == 1 );
  #endif
}

#ifdef VIEW
  void transform( byte b )
  {
    switch( b ) {
    case 0x00:   printf( "\\0" );   break;
    case 0x07:   printf( "\\a" );   break;
    case 0x08:   printf( "\\b" );   break;
    case 0x09:   printf( "\\t" );   break;
    case 0x0A:   printf( "\\n" );   break;
    case 0x0B:   printf( "\\v" );   break;
    case 0x0D:   printf( "\\r" );   break;
    case 0x1A:   printf( "\\z" );   break;
    case 0x20:   printf( "\\s" );   break;
    case 0x5C:   printf( "\\\\" );  break;
    case 0xFF:   printf( "\\f" );   break;
    default:  printf( " %c", b );
    }
  }

  void print( byte *blk )
  {
    word k;
    for( k=0; k<N16; k++ ) printf( "%2u", k%10 );
    printf( "\n" );
    for( k=0; k<N16; k++ ) printf( "%02X", blk[k] );
    printf( "\n" );
    for( k=0; k<N16; k++ ) transform( blk[k] );
    printf( "\n" );
  }

  void print_border()
  {
    printf( "---------------------------------------------------------------------------\n" );
  }

#endif

inline word fl( byte *blk, word pos )
{
  if( pos == 0 ) return LEMPTY;
  word sum = blk[0];
  for( word i=1; i<pos; i++ ) sum = g( sum, blk[i] );
  return sum;
}

inline word fr( byte *blk, word pos )
{
  if( pos == N16-1 ) return REMPTY;
  word sum = blk[N16-1];
  for( word i=N16-2; i>pos; i-- ) sum = g( blk[i], sum );
  return sum;
}

inline word f( byte *out, word pos )
{
  word sum0 = fl( out, pos );
  word sum = g( sum0, out[pos] );
  sum0 = fr( out, pos );
  sum = g( sum, sum0 );
  return sum;
}

void zaes_e( byte *in, byte *out, byte key[N16], byte perm[N16] )
{
  #ifdef VIEW
    print_border();
    printf( "   original plain:\n" );
    print( in );
  #endif
  for( word i=0; i<N16; i++ ) {
    word res = g( in[i], key[i] );
    assert( res < 256 );
    out[i] = byte( res );
  }
  #ifdef MID_VIEW
    printf( "   after 1st addition:\n" );
    print( out );
  #endif
  for( word k=0; k<N16; k++ ) {
    word pos = perm[k];
    word sum = f( out, pos );
    assert( sum < N256 );
    out[pos] = byte( sum );
    #ifdef PASS_VIEW
      printf( "\n" );
      printf( "   pos = %2u, sum = %02X\n", pos, sum );
      print();
    #endif
  }
  #ifdef MID_VIEW
    printf( "   after pass:\n" );
    print( out );
  #endif
  for( i=0; i<N16; i++ ) {
    word res = g( key[i], out[i] );
    assert( res < 256 );
    out[i] = byte( res );
  }
  #ifdef VIEW
    printf( "   after 2nd addition:\n" );
    print( out );
  #endif
}

void zaes_d( byte *in, byte *out, byte key[N16], byte perm[N16] )
{
  #ifdef VIEW
    print_border();
    printf( "   cipher:\n" );
    print( in );
  #endif
  for( word i=0; i<N16; i++ ) {
    word res = ung_y( key[i], in[i] );
    assert( res < 256 );
    out[i] = byte( res );
  }
  #ifdef MID_VIEW
    printf( "   after 2nd addition:\n" );
    print( out );
  #endif
  for( int k=N16-1; k>=0; k-- ) {
    word pos = perm[k];
    word sum0 = fr( out, pos );
    word sum = out[pos];
    sum = ung_x( sum0, sum );
    sum0 = fl( out, pos );
    sum = ung_y( sum0, sum );
    assert( sum < 256 );
    out[pos] = byte( sum );
    #ifdef PASS_VIEW
      printf( "\n" );
      printf( "   pos = %2u, sum = %02X\n", pos, sum );
      print( out );
    #endif
  }
  #ifdef MID_VIEW
    printf( "   after pass:\n" );
    print( out );
  #endif
  for( i=0; i<N16; i++ ) {
    word res = ung_x( key[i], out[i] );
    assert( res < 256 );
    out[i] = byte( res );
  }
  #ifdef VIEW
    printf( "   after 1st addition:\n" );
    print( out );
  #endif
}

class zaes_ecb {
public:
  void e( byte *in, byte *out, byte key[N16], byte perm[N16] ) {
    zaes_e( in, out, key, perm );
  }
  void d( byte *in, byte *out, byte key[N16], byte perm[N16] ) {
    zaes_d( in, out, key, perm );
  }
};

inline void xor_buf( byte *a, byte *b, word len )
{
  byte *lim = a+len;
  for( ; a<lim; a++, b++ ) *a ^= *b;
}

class zaes_cbc {
  byte prev[N16];
public:
  zaes_cbc( const byte iv[N16] ) { memcpy( prev, iv, N16 ); }
  void e( byte *p, byte *c, byte key[N16], byte perm[N16] ) {
    xor_buf( p, prev, N16 );
    zaes_e( p, c, key, perm );
    memcpy( prev, c, N16  );
  }
  void d( byte *c, byte *p, byte key[N16], byte perm[N16] ) {
    byte tmp[N16];
    memcpy( tmp, c, N16 );
    zaes_d( c, p, key, perm );
    xor_buf( p, prev, N16 );
    memcpy( prev, tmp, N16  );
  }
};

class zaes_pcbc {
  byte prev_c[N16];
  byte prev_p[N16];
  byte cnt;
public:
  zaes_pcbc( const byte iv[N16] ) { memcpy( prev_c, iv, N16 ); cnt = 0; }
  void e( byte *p, byte *c, byte key[N16], byte perm[N16] ) {
    byte tmp[N16];
    memcpy( tmp, p, N16 );
    xor_buf( p, prev_c, N16 );
    if( cnt ) xor_buf( p, prev_p, N16 );
    else cnt = 1;
    zaes_e( p, c, key, perm );
    memcpy( prev_c, c, N16  );
    memcpy( prev_p, tmp, N16  );
  }
  void d( byte *c, byte *p, byte key[N16], byte perm[N16] ) {
    byte tmp[N16];
    memcpy( tmp, c, N16 );
    zaes_d( c, p, key, perm );
    xor_buf( p, prev_c, N16 );
    if( cnt ) xor_buf( p, prev_p, N16 );
    else cnt = 1;
    memcpy( prev_p, p, N16  );
    memcpy( prev_c, tmp, N16  );
  }
};

class zaes_cfb_128 {
  byte i[N16];
public:
  zaes_cfb_128( const byte iv[N16] ) { memcpy( i, iv, N16 ); }
  void e( byte *p, byte *c, byte key[N16], byte perm[N16] ) {
    zaes_e( i, i, key, perm );
    xor_buf( i, p, N16 );
    memcpy( c, i, N16 );
  }
  void d( byte *c, byte *p, byte key[N16], byte perm[N16] ) {
    byte tmp[N16];
    memcpy( tmp, c, N16 );
    zaes_e( i, i, key, perm );
    xor_buf( i, c, N16 );
    memcpy( p, i, N16 );
    memcpy( i, tmp, N16  );
  }
};

class zaes_ofb_128 {
  byte i[N16];
public:
  zaes_ofb_128( const byte iv[N16] ) { memcpy( i, iv, N16 ); }
  void e( byte *p, byte *c, byte key[N16], byte perm[N16] ) {
    zaes_e( i, i, key, perm );
    xor_buf( p, i, N16 );
    memmove( c, p, N16 );
  }
  void d( byte *c, byte *p, byte key[N16], byte perm[N16] ) {
    e( c, p, key, perm );
  }
};

class mode_of_operation {
  zaes_ecb *zes0;
  zaes_cbc *zes1;
  zaes_pcbc *zes2;
  zaes_cfb_128 *zes3;
  zaes_ofb_128 *zes4;
  const byte *iv;
  moo_t moo;
public:
  mode_of_operation( moo_t moo0, const byte *iv0 ) : iv( iv0 ), moo( moo0 ) {
    switch( moo ) {
    case ZAES_ECB:
      zes0 = new zaes_ecb;
      printf( "ECB (Electronic Code Book)\n" );
      break;
    case ZAES_CBC:
      zes1 = new zaes_cbc( iv );
      printf( "CBC (Cipher Block Chaining)\n" );
      break;
    case ZAES_PCBC:
      zes2 = new zaes_pcbc( iv );
      printf( "PCBC (error-Propagating Cipher Block Chaining)\n" );
      break;
    case ZAES_CFB_128:
      zes3 = new zaes_cfb_128( iv );
      printf( "128-bit CFB (Cipher FeedBack)\n" );
      break;
    case ZAES_OFB_128:
      zes4 = new zaes_ofb_128( iv );
      printf( "128-bit OFB (Output FeedBack)\n" );
      break;
    }
  }
  void e( byte *a, byte *b, byte key[N16], byte perm[N16] ) {
    switch( moo ) {
    case ZAES_ECB:
      zes0->e( a, b, key, perm );
      break;
    case ZAES_CBC:
      zes1->e( a, b, key, perm );
      break;
    case ZAES_PCBC:
      zes2->e( a, b, key, perm );
      break;
    case ZAES_CFB_128:
      zes3->e( a, b, key, perm );
      break;
    case ZAES_OFB_128:
      zes4->e( a, b, key, perm );
      break;
    }
  }
  void d( byte *a, byte *b, byte key[N16], byte perm[N16] ) {
    switch( moo ) {
    case ZAES_ECB:
      zes0->d( a, b, key, perm );
      break;
    case ZAES_CBC:
      zes1->d( a, b, key, perm );
      break;
    case ZAES_PCBC:
      zes2->d( a, b, key, perm );
      break;
    case ZAES_CFB_128:
      zes3->d( a, b, key, perm );
      break;
    case ZAES_OFB_128:
      zes4->d( a, b, key, perm );
      break;
    }
  }
};

void encrypt( moo_t mo )
{
  word_32 flen = input_file.filelength();
  output_file.write( &flen, 4 );
  mode_of_operation moo( mo, pad );
  #ifndef VIEW
    ind_percent ind;
    word_32 read_bytes = 0;
  #endif
  byte perm[N16];
  byte key[N16];
  init( key, perm );
  for( ;; ) {
    int c = input_file.read( &buf[0], BUF_LEN );
    if( c == 0 ) break;
    word howmany = c / N16;
    word rem = c % N16;
    word curp = 0;
    for( word i=0; i<howmany; i++, curp+=N16 ) {
      byte *blk = &buf[curp];
      moo.e( blk, blk, key, perm );
    }
    word nb = howmany*N16;
    if( rem != 0 ) {
      nb += N16;
      for( word i=rem; i<N16; i++ ) buf[curp+i] = pad[i];
      byte *blk = &buf[curp];
      moo.e( blk, blk, key, perm );
    }
    output_file.write( &buf[0], nb );
    #ifndef VIEW
      read_bytes += c;
      ind.next( read_bytes, flen );
    #endif
  }
}

void decrypt( moo_t mo )
{
  word_32 flen;
  input_file.read( &flen, 4 );
  word_32 fl = flen;
  mode_of_operation moo( mo, pad );
  #ifndef VIEW
    ind_percent ind;
    word_32 read_bytes = 0;
  #endif
  byte perm[N16];
  byte key[N16];
  init( key, perm );
  init_d();
  for( ;; ) {
    int c = input_file.read( &buf[0], BUF_LEN );
    if( c == 0 ) break;
    word howmany = c / N16;
    if( (c % N16) != 0 ) {
      printf( "something's wrong with input_file\n" );
      exit( 1 );
    }
    word curp = 0;
    for( word i=0; i<howmany; i++, curp+=N16 ) {
      byte *blk = &buf[curp];
      moo.d( blk, blk, key, perm );
    }
    word nb = howmany*N16;
    if( fl < nb ) nb = word( fl );
    else fl -= nb;
    output_file.write( &buf[0], nb );
    #ifndef VIEW
      read_bytes += c;
      ind.next( read_bytes, flen );
    #endif
  }
  word_32 flen2 = output_file.filelength();
  if( flen != flen2 ) {
    printf( "something's wrong with input_file\n" );
    printf( "flen  = %10lu\n", flen );
    printf( "flen2 = %10lu\n", flen2 );
    exit( 1 );
  }
}

//#define TEST
#ifdef TEST
  #include <time.h>
  const word_32 KBYTES = 1024L;

  void main()
  {
    byte XX[16], YY[16], TT[16];
    clock_t start, end, end2, end3;
    long l;

    byte perm[N16];
    byte key[N16];
    init( key, perm );
    init_d();

    for( byte k=0; k<16; k++ ) XX[k] = k;

    const word_32 howmany = 32*KBYTES*N16;
    printf("\nEncrypting and decrypting %ld bytes\n", howmany );
    fflush( stdout );
    start = clock();
    zaes_e( XX, YY, key, perm );
    for( l=1; l < 32*KBYTES; l++ ) zaes_e( YY, YY, key, perm );
    end2 = clock() - start;
    l = end2 * 1000. / CLOCKS_PER_SEC + 1;
    word i = l / 1000;
    word j = l % 1000;
    l = KBYTES * 512. * CLOCKS_PER_SEC / end2;
    printf( "encrypting: %3d.%03d seconds = %ld bytes per second\n", i, j, l );
    zaes_d( YY, TT, key, perm );
    for (l = 1; l < 32*KBYTES; l++) zaes_d( TT, TT, key, perm );
    end = clock() - start;
    end3 = end - end2;
    l = end3 * 1000. / CLOCKS_PER_SEC + 1;
    i = l/1000;
    j = l%1000;
    l = KBYTES * 512. * CLOCKS_PER_SEC / end3;
    printf( "decrypting: %3d.%03d seconds = %ld bytes per second\n", i, j, l );
    l = end * 1000. / CLOCKS_PER_SEC + 1;
    i = l / 1000;
    j = l % 1000;
    l = KBYTES * 1024. * CLOCKS_PER_SEC / end;
    printf( "total:      %3d.%03d seconds = %ld bytes per second\n", i, j, l );

    for( k=0; k<16; k++ )
      if( TT[k] != XX[k] ) {
	printf("\nError!  Noninvertable encryption.\n");
        return;
      }
    printf("\nNormal exit.\n");
  }
#else
  void print_header()
  {                                         //April 13 1995
    printf( "ZAES  by  Timur Zambalaev        Jul 01 1995\n" );
  }

  void help()
  {
    print_header();
    printf( "Zambalaev Adding Encryption System\n"
            "Usage: ZAES <command> <mode_of_operation> input_file output_file\n"
            "Commands:\n"
            "  e         Encrypt\n"
            "  d         Decrypt\n"
            "Modes of operation:\n"
            "  e         ECB (Electronic Code Book)\n"
            "  cb        CBC (Cipher Block Chaining)\n"
            "  p         PCBC (error-Propagating Cipher Block Chaining)\n"
            "  cf        128-bit CFB (Cipher FeedBack)\n"
            "  o         128-bit OFB (Output FeedBack)\n" );
  }

  char *arg_str[2];

  int know_args( int argc, char *argv[] )
  {
    if( argc != 5 ) return -1;
    arg_str[0] = argv[3];
    arg_str[1] = argv[4];
    return 0;
  }

  int main( int argc, char *argv[] )
  {
    if( know_args( argc, argv ) < 0 ) {
      help();
      return BAD_PARAMS;
    }
    input_file.open( arg_str[0], O_RDONLY | O_BINARY );
    output_file.open( arg_str[1], O_WRONLY | O_CREAT | O_BINARY | O_TRUNC, S_IWRITE );
    moo_t moo;
    int ch = tolower( argv[2][0] );
    switch( ch ) {
    case 'e':
      moo = ZAES_ECB;
      break;
    case 'p':
      moo = ZAES_PCBC;
      break;
    case 'o':
      moo = ZAES_OFB_128;
      break;
    case 'c':
      ch = tolower( argv[2][1] );
      switch( ch ) {
      case 'b':
        moo = ZAES_CBC;
        break;
      case 'f':
        moo = ZAES_CFB_128;
        break;
      default:
        help();
        return BAD_PARAMS;
      }
      break;
    default:
      help();
      return BAD_PARAMS;
    }
    ch = tolower( argv[1][0] );
    switch( ch ) {
    case 'e':
      encrypt( moo );
      break;
    case 'd':
      decrypt( moo );
      break;
    default:
      help();
      return BAD_PARAMS;
    }
    printf( "\n" );
    return 0;
  }
#endif
