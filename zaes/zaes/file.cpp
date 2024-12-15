#include <ctype.h>

#include "typedef.h"
#include "file.h"

void upper_case( char *s )
{
  for( ; *s; s++ ) *s = char( toupper( *s ) );
}

void File::open( char *name0, int access0 )
{
  upper_case( name0 );
  name = name0;
  access = access0;
  handle = ::open( name, access );
  if( handle == -1 ) error_open();
}

void File::open( char *name0, int access0, unsigned mode )
{
  upper_case( name0 );
  name = name0;
  access = access0;
  handle = ::open( name, access, mode );
  if( handle == -1 ) error_open();
}

void File::perror()
{
  fflush( stdout );
  ::perror( " " );
}

void File::error_open()
{
  printf( "Can't open file %s", name );
  perror();
  exit( OPEN_ERROR );
}

int File::read( void *buf, unsigned len )
{
  int res = ::read( handle, buf, len );
  if( res == -1 ) {
    ::perror( "Can't read from file" );
    exit( READ_ERROR );
  }
  return res;
}

void File::write( void *buf, unsigned len )
{
  int res = ::write( handle, buf, len );
  if( res == -1 ) {
    ::perror( "Can't write to file" );
    exit( WRITE_ERROR );
  }
  if( res != len ) {
    printf( "Probably disk full\n" );
    exit( DISK_FULL );
  }
}

long File::lseek( long offset, int fromwhere )
{
  long res = ::lseek( handle, offset, fromwhere );
  assert( res != -1 );
  return res;
}

ind_percent::ind_percent()
{
  curr = 0;
  printf( "  0" );
  fflush( stdout );
}

void ind_percent::next( word_32 p1, word_32 p2 )
{
  word_32 p;
  if( p2 == 0 ) p = 100;
  else p = (100*p1)/p2;
  if( p > 100 ) p = 100;
  word perc = word( p );
  if( perc != curr ) {
    word p1 = perc / 10;
    word c1 = curr / 10;
    if( p1 == c1 ) {
      printf( "\b \b" );
      printf( "%1u", perc % 10 );
    }
    else {
      for( word k=0; k<3; k++ ) printf( "\b \b" );
      printf( "%3u", perc );
    }
    fflush( stdout );
    curr = perc;
  }
}

