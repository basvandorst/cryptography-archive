/******   eliptic.h   *****/
/****************************************************************
*                                                               *
*       These are structures used to create elliptic curve      *
*  points and parameters.  "form" is a just a fast way to check *
*  if a2 == 0.                                                  *
*               form            equation                        *
*                                                               *
*                0              y^2 + xy = x^3 + a_6            *
*                1              y^2 + xy = x^3 + a_2*x^2 + a_6  *
*                                                               *
****************************************************************/

typedef struct {
        INDEX   form;
        BIGINT  a2;
        BIGINT  a6;
} CURVE;

/*  projective coordinates for a point  */

typedef struct {
        BIGINT  x;
        BIGINT  y;
        BIGINT  z;
} POINT;

/*  started getting tired of writing this */

#define SUMLOOP(i) for(i=STRTPOS; i<MAXLONG; i++)
