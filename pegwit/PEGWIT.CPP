#include "vlong.hpp"
#include "ncurve.hpp"
#include "stdio.h"
#include "string.h"
#include "time.h"

extern "C" 
{
  #include "square.h"
  #define WPB 4
  #define BPB 16
}

void put_vlong( FILE * f, vlong v )
{
    if ( v >= 256 )
      put_vlong( f, v / 256 );
    fprintf( f, "%02x", to_unsigned( v%256 ) );
}

vlong get_vlong( FILE * f )
{
      vlong v;
      unsigned u;
      while ( fscanf( f, "%2x", &u ) == 1 )
            v = u + ( v << 8 );
      return v;
}

class ec_crypt1 : public ec_crypt
{

  word32 seed[WPB], key[R+1][WPB];
  unsigned rand( unsigned range )
  {
    squareEncrypt( seed, key );
    return seed[0] % range;
  }
  public:
  void randomize( word32 data[WPB] )
  {
      for (unsigned j=0;j<WPB;j+=1)
          seed[j] += data[j];
      seed[0] += time(0);
  }
  ec_crypt1( full_curve_parameter & a, word32 Key[WPB] ) : ec_crypt( a )
  {
    word32 dummy[R+1][WPB];
    squareGenerateRoundkeys ( Key, key, dummy );
    for (unsigned j=0;j<WPB;j+=1) seed[j] = 0;
  }
};


void vlong_to_key( vlong v, word32 key[WPB] )
{
    vlong base = pow2(32)-1; // -1 to force use of all bits in v
    for (unsigned j=0;j<WPB;j+=1)
    {
      key[j] = to_unsigned(v % base);
      v = v / base;
    }
}

vlong key_to_vlong( word32 key[WPB] )
// not the inverse of vlong_to_key
{
    vlong base = pow2(32);
    vlong v = 0;
    for (unsigned j=0;j<WPB;j+=1)
    {
      v = v * base + key[j];
    }
    return v;
}

vlong str_to_vlong( char * s )
{
    // initialise key using password
    vlong v = 0;
    unsigned i = 0;
    while ( s[i] )
    {
        v = v * 256 + s[i];
        i += 1;
    }
    return v;

}

int read( FILE * f_inp, word32 a[WPB] )
{
    byte b[BPB];
    int n = fread( b, 1, BPB, f_inp );
    if ( n < BPB )
    {
        byte pad = BPB-n;
        memset( b+n, pad, pad );
    }
    unsigned i = BPB;
    do // On a little endian machine could use simple memcpy
    {
      word32 w = b[--i];
      w <<= 8; w += b[--i];
      w <<= 8; w += b[--i];
      w <<= 8; w += b[--i];
      a[i/4] = w;
    } while (i);
    return n<BPB;
}

void write( FILE * f_out, word32 a[WPB], int last )
{
    byte b[BPB];
    unsigned i = 0;
    do // On a little endian machine could use simple memcpy
    {
        word32 w=a[i/4];
        b[i++] = (byte) w; w>>=8;
        b[i++] = (byte) w; w>>=8;
        b[i++] = (byte) w; w>>=8;
        b[i++] = (byte) w;
    } while ( i < BPB );
    if (last) i -= b[BPB-1] % BPB;
    if (i) fwrite( b, 1, i,  f_out );
}

void compute_hash( FILE * f_inp, word32 key[WPB]  )
{
    word32 rkey[R+1][WPB], dummy[R+1][WPB];
    squareGenerateRoundkeys( key, rkey, dummy );

    word32 plain[WPB];
    int last = 0;
    while ( !last )
    {
        last = read( f_inp, plain );
        squareEncrypt( plain, rkey );
        for (unsigned j=0;j<WPB;j+=1)
            key[j] ^= plain[j];
    }
    fseek( f_inp, 0, SEEK_SET );
}

const curve_parameter cdata = { 13, 11, 2, 27, 773, 48, 1 };

void speed_up( full_curve_parameter & cp )
{
    // can be omitted, but makes initialisation faster
    cp.tm = 0x201 + 0x201 * pow2(117); // N.B. depends on curve parameter
}

void do_secret( char * passphrase, FILE * f_inp, FILE * f_out, int operation )
{
    word32 key[WPB];
    vlong ps = str_to_vlong(passphrase);
    vlong_to_key( ps, key );

    full_curve_parameter cp( cdata );
    speed_up( cp );

    ec_crypt1 c( cp, key );
    vlong pri = c.make_private_key();
 
    if (operation==0)
    {
        vlong pub = c.make_public_key( pri );
        put_vlong( stdout, pub );
        printf("\n");
    }
    else if ( operation==1)
    {
        pair msg;
        msg.s = get_vlong( f_inp );
        fscanf( f_inp, ":" );
        msg.r = get_vlong( f_inp );
        fscanf( f_inp, ":" );
        vlong secret = c.decode_secret( pri, msg );
        vlong_to_key( secret, key );

        word32 rkey[R+1][WPB], dummy[R+1][WPB];
        squareGenerateRoundkeys( key, dummy, rkey );
        
        word32 plain[WPB];
        word32 cipher[WPB];
        word32 old_cipher[WPB];
        int first = 1;
        unsigned j;
        for (j=0;j<WPB;j+=1) old_cipher[j]=0;
        while (1)
         {
            int last = read( f_inp, cipher );
            if (!first) 
                write(f_out, plain, last );
            if ( last )
                break;
            for (j=0;j<WPB;j+=1)
                plain[j] = cipher[j];
            squareDecrypt( plain, rkey );
            for (j=0;j<WPB;j+=1)
            {
                plain[j] ^= old_cipher[j];
                old_cipher[j] = cipher[j];
            }
            first = 0;
        }
        
    }
    else // operation == 2, signature
    {
        for (unsigned k=0;k<WPB;k+=1)
            key[k] = 0;
        compute_hash( f_inp, key );
        c.randomize( key );

        vlong h = key_to_vlong( key );
        pair sig = c.dsa_sign( h, pri );
        put_vlong( stdout, sig.s );
        printf( ":" );
        put_vlong( stdout, sig.r );
        printf("\n");
    }
}

int do_public ( FILE * fpub, FILE * f_inp, FILE * f_out, int operation )
{
    vlong pub = get_vlong( fpub );

    char s2[256];
    sprintf( s2, "%ld", time(0) );
    word32 key[WPB];
    vlong_to_key( str_to_vlong(s2), key );

    // Since the file we are encrypting is secret, use it to initialise random key
    compute_hash( f_inp, key );

    full_curve_parameter cp( cdata );
    speed_up( cp );
    ec_crypt1 c( cp, key );

    pair msg;

    if ( operation == 1 )
    {
        vlong secret = c.make_secret( pub, msg );
        put_vlong( f_out, msg.s );
        fprintf(f_out,":");
        put_vlong( f_out, msg.r );
        fprintf(f_out,":");

        vlong_to_key( secret, key );
        word32 rkey[R+1][WPB], dummy[R+1][WPB];
        squareGenerateRoundkeys( key, rkey, dummy );
        word32 plain[WPB], cipher[WPB];
        for (unsigned j=0;j<WPB;j+=1) cipher[j]=0;
        while (1)
        {           
           int last = read( f_inp, plain );
           for (j=0;j<WPB;j+=1)
               cipher[j] ^= plain[j];
           squareEncrypt( cipher, rkey );
           write( f_out, cipher, 0 );
           if ( last ) break;
        }
    }
    else // operation == 2, verify
    {
        for (unsigned k=0;k<WPB;k+=1)
            key[k] = 0;
        compute_hash( f_inp, key );
        vlong h = key_to_vlong( key );

        pair sig;
        sig.s = get_vlong( stdin );
        fscanf( stdin, ":" );
        sig.r = get_vlong( stdin );
        return c.dsa_verify( h, sig, pub );
    }
    return 0;
}

FILE * chkopen( char * s, char * mode )
{
    FILE * result = fopen(s,mode);
    if (!result) 
    { 
        fprintf(stderr,"Failed to open %s\n:",s); 
    }
    return result;
}

int main( unsigned argc, char * argv[] )
{
    if (argc<2) goto error;
    if ( strcmp(argv[1], "-i" ) == 0 )
    {
        if ( argc!=3 ) goto error;
        do_secret( argv[2], 0, 0, 0 );
    }
    else if ( strcmp(argv[1], "-d" ) == 0 )
    {
        if ( argc != 5 ) goto error;        
        FILE * f_inp = chkopen( argv[3], "rb" ); if (!f_inp) return 1;
        FILE * f_out = chkopen( argv[4], "wb" ); if (!f_out) return 1;
        do_secret( argv[2], f_inp, f_out, 1 );
    }
    else if ( strcmp(argv[1], "-e" ) == 0 )
    {
        if ( argc != 5 ) goto error;
        FILE * fpub = chkopen( argv[2], "rb" ); if (!fpub) return 1;
        FILE * f_inp = chkopen( argv[3], "rb" ); if (!f_inp) return 1;
        FILE * f_out = chkopen( argv[4], "wb" ); if (!f_out) return 1;
        do_public( fpub, f_inp, f_out, 1 );
        fclose( fpub );
    }
    else if ( strcmp(argv[1], "-s" ) == 0 )
    {
        if ( argc != 4 ) goto error;        
        FILE * f_inp = chkopen( argv[3], "rb" ); if (!f_inp) return 1;
        do_secret( argv[2], f_inp, 0, 2 );
    }
    else if ( strcmp(argv[1], "-v" ) == 0 )
    {
        if ( argc != 4 ) goto error;
        FILE * fpub = chkopen( argv[2], "rb" ); if (!fpub) return 1;
        FILE * f_inp = chkopen( argv[3], "rb" ); if (!f_inp) return 1;
        if ( !do_public( fpub, f_inp, 0, 2 ) )
        {
            fprintf( stderr, "signature did not verify!!!\n");
            return 1;
        }
    }
    else
        goto error;
    return 0;
error:
    fprintf( stderr,
      "Usage : (init/encrypt/decrypt/sign/validate) is: \n"
      "-i \"secret-phrase\" >public-key-file\n"
      "-e public-key-file plain-file cipher-file\n"
      "-d \"secret-phrase\" cipher-file plain-file\n" 
      "-s \"secret-phrase\" plain-file >signature-file\n"
      "-v public-key-file plain-file <signature-file\n" 
    );
    return 1;
}
