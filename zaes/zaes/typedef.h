#ifndef TYPEDEF_H
  #define TYPEDEF_H

  #include <stdio.h>

  #define NDEBUG
  #include <assert.h>

  typedef unsigned char byte;
  typedef unsigned short ushort;
  typedef ushort word_16;
  typedef unsigned int word;
  typedef unsigned long word_32;

  const int FILE_EXISTS            =  3;
  const int OPEN_ERROR             =  4;
  const int BAD_PARAMS             =  5;
  const int NOT_ENOUGH_MEM         =  6;
  const int DISK_FULL              =  7;
  const int WRITE_ERROR            =  8;
  const int READ_ERROR             =  9;

#endif
