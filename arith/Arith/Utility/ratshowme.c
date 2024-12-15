/*
 * Arithmetic in Global Fields
 * Copyright (C) 1990 Kevin R. Coombes and David R. Grant
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*  @(#)ratshowme.c	1.1  3/29/90  */

#include <stdio.h>
#include <strings.h>
#include "normal.h"
#include "Qstruct.h"

extern void bailout();
extern void showme();

/* RATSHOWME()
 * displays an arbitrary precision rational number or function in two parts.
 */

void
ratshowme(outfile, msg, a, with_sign)
FILE *outfile;
char *msg;
ratl a;
bool with_sign;
{
  char buff[BUFSIZ];

  if (strcpy(buff, msg) != buff || strcat(buff,".num") != buff)
    bailout("Ratshowme: string handling error.");
  showme(outfile, buff, a.num, with_sign);
  if (strcpy(buff, msg) != buff || strcat(buff,".den") != buff)
    bailout("Ratshowme: string handling error.");
  showme(outfile, buff, a.den, with_sign);
  return;
}
