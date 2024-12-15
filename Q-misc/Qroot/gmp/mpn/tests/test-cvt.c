/* Test mpn_set_str, mpn_get_str, mpn_random2, and mpn_cmp.
   Usage:  test-cvt [-r reps] [-s size] [-b base]
   The base argument can be in the range [2..256].

Copyright (C) 1992 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU MP Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU MP Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>

#include "gmp.h"
#include "gmp-impl.h"

char *prog_name;

void
usage(str)
     char *str;
{
  fprintf (stderr, "%s: %s\n", prog_name, str);
  fprintf (stderr, "usage: %s [-r reps] [-s size] [-b base]\n", prog_name);
  exit (2);
}

void
main (argc, argv)
     int argc;
     char **argv;
{
  int argi;

  int base = 0;
  mp_size size = 10;
  int reps = 10000;

  prog_name = argv[0];

  argi = 1;
  while (argi < argc)
    {
      char *arg = argv[argi];
      int flag;

      if (arg[0] != '-')
	break;

      flag = arg[1];

      if (arg[2] != 0)
	arg = arg + 2;
      else
	arg = argv[++argi];

      if (argi == argc)
	usage("numeric argument expected");

      if (flag == 'r')
	reps = strtol (arg, 0, 0);
      else if (flag == 's')
	size = strtol (arg, 0, 0);
      else if (flag == 'b')
	base = strtol (arg, 0, 0);
      else
	usage("no such command-line option");
      argi++;
    }

  if (argi != argc)
    usage ("junk after expected line end");

  {
    /* Allocate limb space plus 1 extra limb before and after
       for magic values.  */
    mp_ptr s_ptr = malloc ((size + 2) * BYTES_PER_MP_LIMB);
    mp_ptr sc_ptr = malloc (size * BYTES_PER_MP_LIMB);
    mp_ptr res_ptr = malloc ((size + 2) * BYTES_PER_MP_LIMB);
    mp_size res_size;
    size_t str_alloc;
    int i;
    unsigned char *str;
    mp_size this_size;
    int this_base = base;
    size_t str_len;

    /* Allocate string space.  We add 3 to the allocation; 1 to round the
       allocation up the next integral (it's a floating point value), 2 to
       make space for magic values to check that routines don't write
       outside of their assigned space.  */
    if (base == 0)
      /* If base is 0, we need space for base 2.  */
      str_alloc = size * BITS_PER_MP_LIMB + 3;
    else
      str_alloc = __mp_bases[base].chars_per_bit_exactly * size * BITS_PER_MP_LIMB + 3;
    str = malloc (str_alloc);

    /* Make the pointers point to the second limb/byte,
       after any magic value.  */
    s_ptr++;
    res_ptr++;
    str++;

    for (i = 0; i < reps; i++)
      {
	this_size = (unsigned) rand () % size + 1;

	if (base == 0)
	  this_base = (unsigned) rand () % 254 + 2;

	/* Insert magic value in S.  */
	s_ptr[-1] = i + 874;
	s_ptr[this_size] = i - 874;
	/* Get random limb vector.  */
	mpn_random2 (s_ptr, this_size);
	/* Check that magic values are not overwritten.  */
	if (s_ptr[-1] != i + 874 || s_ptr[this_size] != i - 874)
	  abort();
	/* Copy S to SC since mpn_get_str clobbers S.  We need the value
	   in mpn_cmp.  */
	memcpy (sc_ptr, s_ptr, this_size * BYTES_PER_MP_LIMB);

	/* Insert magic value in STR.  */
	str[-1] = (unsigned char) (i + 87);
	str[str_alloc - 2] = (unsigned char) (i - 87);
	/* Convert limb vector to string at STR in base THIS_BASE.  */
	str_len = mpn_get_str (str, this_base, s_ptr, this_size);
	/* Check that magic values are not overwritten.  */
	if (str[-1] != (unsigned char) (i + 87)
	    || str[str_alloc - 2] != (unsigned char) (i - 87))
	  abort ();

	/* Insert magic value in RES.  */
	res_ptr[-1] = i + 847;
	res_ptr[this_size] = i - 847;
	/* Convert string at STR in base THIS_BASE to limb vector RES.  */
	res_size = mpn_set_str (res_ptr, str, str_len, this_base);
	if (res_ptr[-1] != i + 847 || res_ptr[this_size] != i - 847)
	  abort ();

	if (res_size != this_size || mpn_cmp (res_ptr, sc_ptr, this_size) != 0)
	  abort();
      }
  }

  exit (0);
}
