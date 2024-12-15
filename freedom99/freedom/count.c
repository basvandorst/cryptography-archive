/****************************************************************************
    This file is part of the Freedom Remailer.  It is:
    Copyright (C) 1995  John B. Fleming (jfleming@indiana.edu)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
****************************************************************************/

#include <stdio.h>
#include "config.h"

int getcount(void)
{
  FILE *in;
  char countfile[255], string[50];

  sprintf(countfile, "%s/%s", REMAILER_DIR, COUNT_FILE);
  in = fopen(countfile, "r");
  if (in == NULL)
    return -1;
  fgets(string, 50, in);
  fclose(in);
  return atoi(string);
}

void makecount(void)
{
  FILE *out;
  char countfile[255];

  sprintf(countfile, "%s/%s", REMAILER_DIR, COUNT_FILE);
  out = fopen(countfile, "w");
  fprintf(out, "0");
  fclose(out);
}

void pluscount(void)
{
  FILE *out;
  char countfile[255];
  int count;

  sprintf(countfile, "%s/%s", REMAILER_DIR, COUNT_FILE);
  count = getcount();
  count++;
  
  out = fopen(countfile, "w");
  fprintf(out, "%d", count);
  fclose(out);
}
