// data.c -- class functions for class "Data".
//
// written by Marc Wallace, 30 Apr 93.
// last change: version 1.0 (14 May 93).


#include "data.h"


Data::Data(int m)
    {
    data = new unsigned char[m];
    size = 0;
    maxsize = m;
    };


Data::Data(int m, int s, unsigned char* d)
    {
    data = new unsigned char[m];
    for (int r = 0; r < s; r++)
        data[r] = d[r];
    size = s;
    maxsize = m;
    };


void
Data::reinit(int m)
    {
    delete data;
    Data(m);
    };


void
Data::reinit(int m, int s, unsigned char* d)
    {
    if (m > maxsize)
        {
        delete data;
        data = new unsigned char[m];
        };
    for (int r = 0; r < s; r++)
        data[r] = d[r];
    size = s;
    maxsize = m;
    };


