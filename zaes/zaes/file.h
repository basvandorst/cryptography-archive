#ifndef FILE_H
  #define FILE_H
  #include <io.h>
  #include <fcntl.h>
  #include <sys\stat.h>
  #include <stdlib.h>
  #include "typedef.h"

  class File {
    char *name;
    int access;
    int handle;
  public:
    File() { handle = -1; }
    ~File() { if( handle != -1 ) close( handle ); }
    void open( char *name0, int access0 );
    void open( char *name0, int access0, unsigned mode );
    void perror();
    void error_open();
    long filelength() { return ::filelength( handle ); }
    int read( void *buf, unsigned len );
    void write( void *buf, unsigned len );
    long lseek( long offset, int fromwhere );
  };

  class ind_percent {
    word curr;
  public:
    ind_percent();
    void next( word_32 p1, word_32 p2 );
  };

#endif
