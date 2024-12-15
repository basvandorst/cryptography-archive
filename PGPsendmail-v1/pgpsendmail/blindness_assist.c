/*  blindness_assist.h

    Programme to assist blind people (those who don't read documentation) for
    PGPsendmail.

    Copyright (C) 1994  Richard Gooch

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

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/
#include <stdio.h>
#include "pgpsendmail.h"

int main ()
{
    (void) fprintf (stderr, "\n***WARNING***\n");
    (void) fprintf (stderr, "PGPsendmail will expect that the real sendmail");
    (void) fprintf (stderr, " is: \"%s\"\n", SENDMAIL);
    (void) fprintf (stderr, "If this is not true, you will have problems\n\n");
    return (0);
}   /*  End Function main  */
