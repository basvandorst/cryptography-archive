/*
 *    MIRACL  C++ Header file crt.h
 *
 *    AUTHOR  : M. Scott
 *  
 *    PURPOSE : Definition of class Crt  (Chinese Remainder Thereom)
 *    NOTE    : Must be used in conjunction with big.cpp and crt.cpp
 *              Can be used with either Big or utype moduli
 *                
 *    Copyright (c) 1988-1993 Shamus Software Ltd.
 */

#ifndef CRT_H
#define CRT_H

#include <big.h>
#define BIGS   0
#define SMALLS 1

class Crt 
{ 
    Big *V,*C,*M;
    utype *v,*c,*m;
    int np,type;
public:
    Crt(int ,Big *);      /* two flavours */
    Crt(int ,utype *);

    Big eval(Big *);
    Big eval(utype *);

    ~Crt() 
     {  /* destructor */
        if (type==BIGS)
        {
            delete [] V;
            delete [] C;
            delete [] M;
        }
        if (type==SMALLS)
        {
            delete [] v;
            delete [] c;
            delete [] m;
        } 
    }
};

#endif

