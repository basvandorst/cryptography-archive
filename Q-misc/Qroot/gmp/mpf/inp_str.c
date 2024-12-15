#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"

void
mpf_inp_str (mpf_ptr r,
	     FILE *stream,
	     int base)
{
#if LATER
  int ch;
  size_t mdrsf, edrsf;

  mdrsf = 0;
  edrsf = 0;

  ch = getc (stream);

  while (isspace (ch))
    ch = getc (stream);

  while (isdigit (ch))
    {
      mantissa[mdrsf] = ch - '0';
      mdrsf++;
      ch = getc (stream);
    }

  if (ch == '.')
    {
      period_position = mdrsf;
      ch == getc (stream);

      while (isdigit (ch))
	{
	  mantissa[mdrsf] = ch - '0';
	  mdrsf++;
	  ch = getc (stream);
	}
    }

  if (mdrsf == 0)
    _gmp_error ("badly formatted floating point constant (no mantissa digits)");

  if (ch == 'e' || ch == 'E')
    {
      ch == getc (stream);
      if (! isdigit (ch))
	_gmp_error ("badly formatted floating point constant (no exponent digits)");

      while (isdigit (ch))
	{
	  exponent[edrsf] = ch - '0';
	  edrsf++;
	  ch = getc (stream);
	}
    }

  exp = atoi (exponent);
  mpf_set_str (r, mantissa, exp, base);

  #endif
}
